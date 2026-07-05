// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_uart_engine.cpp — dual-UART ring-buffer engine (GNSS + mesh coprocessor).
//
// Channel map (HW ref §3.8; console is on native USB-Serial-JTAG so both
// UART peripherals are free):
//   UART1  RX=18 TX=17  BN-880 GNSS      9600 8N1   NMEA-0183 line framing
//   UART2  RX=44 TX=43  ESP32-C6/H2    115200 8N1   SLIP + CRC16-CCITT frames
//
// Both channels: uart_driver_install() with an event queue; the ISR drains
// the HW FIFO into the driver ring buffer; a pump task blocks on the event
// queue (zero CPU when idle) and never busy-waits. UI is never blocked.

#include "ss_uart_engine.h"

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_check.h"

#include "board_config.h"

static const char* TAG = "ss.uart";

// ---------------------------------------------------------------------------
// Shared state
// ---------------------------------------------------------------------------
static ss_gnss_fix_t s_fix;
static bool s_fix_ever = false;
static SemaphoreHandle_t s_fix_lock;
static ss_nmea_sentence_cb_t s_nmea_tap = nullptr;
static ss_coproc_frame_cb_t s_coproc_cb = nullptr;
static ss_uart_chan_stats_t s_gnss_stats, s_coproc_stats;

// SLIP special bytes (RFC 1055)
static constexpr uint8_t SLIP_END = 0xC0, SLIP_ESC = 0xDB, SLIP_ESC_END = 0xDC, SLIP_ESC_ESC = 0xDD;
static constexpr size_t COPROC_MAX_FRAME = 1024;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
static uint16_t crc16_ccitt(const uint8_t* d, size_t n)
{
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < n; ++i) {
        crc ^= uint16_t(d[i]) << 8;
        for (int b = 0; b < 8; ++b)
            crc = (crc & 0x8000) ? uint16_t((crc << 1) ^ 0x1021) : uint16_t(crc << 1);
    }
    return crc;
}

// Verify "$....*hh" checksum; NMEA XOR between '$' and '*'.
static bool nmea_checksum_ok(const char* s, size_t len)
{
    if (len < 9 || s[0] != '$') return false;
    const char* star = static_cast<const char*>(memchr(s, '*', len));
    if (!star || size_t(star - s) + 3 > len) return false;
    uint8_t x = 0;
    for (const char* p = s + 1; p < star; ++p) x ^= uint8_t(*p);
    return strtoul(star + 1, nullptr, 16) == x;
}

// ddmm.mmmm(,dir) → signed decimal degrees.
static double nmea_coord(const char* f, char dir)
{
    if (!f || !*f) return 0.0;
    const double v = atof(f);
    const double deg = floor(v / 100.0);
    double out = deg + (v - deg * 100.0) / 60.0;
    if (dir == 'S' || dir == 'W') out = -out;
    return out;
}

// Split a sentence into comma fields in place. Returns field count.
static size_t split_fields(char* s, const char* fields[], size_t max)
{
    size_t n = 0;
    for (char* p = s; p && n < max;) {
        fields[n++] = p;
        p = strchr(p, ',');
        if (p) *p++ = '\0';
    }
    return n;
}

// ---------------------------------------------------------------------------
// GNSS channel — NMEA line framing + minimal RMC/GGA parse
// ---------------------------------------------------------------------------
static void gnss_handle_sentence(char* line, size_t len)
{
    s_gnss_stats.rx_frames++;
    if (s_nmea_tap) s_nmea_tap(line, len);
    if (!nmea_checksum_ok(line, len)) {
        s_gnss_stats.rx_crc_errors++;
        return;
    }

    // Strip "*hh" so field splitting is clean.
    char* star = strchr(line, '*');
    if (star) *star = '\0';

    const char* f[24];
    const size_t n = split_fields(line, f, 24);
    if (n < 2) return;
    const char* type = f[0] + 3; // skip "$GP"/"$GN"/…

    xSemaphoreTake(s_fix_lock, portMAX_DELAY);
    if (strncmp(type, "RMC", 3) == 0 && n >= 10) {
        // $..RMC,time,status,lat,NS,lon,EW,sog_kn,cog,date,...
        s_fix.has_fix = (f[2][0] == 'A');
        s_fix.lat_deg = nmea_coord(f[3], f[4][0]);
        s_fix.lon_deg = nmea_coord(f[5], f[6][0]);
        s_fix.speed_mps = float(atof(f[7]) * 0.514444);
        s_fix.course_deg = float(atof(f[8]));
        s_fix_ever = true;
    } else if (strncmp(type, "GGA", 3) == 0 && n >= 10) {
        // $..GGA,time,lat,NS,lon,EW,quality,sats,hdop,alt,...
        s_fix.sats_used = uint8_t(atoi(f[7]));
        s_fix.hdop = float(atof(f[8]));
        s_fix.alt_m = float(atof(f[9]));
        s_fix_ever = true;
    }
    xSemaphoreGive(s_fix_lock);
}

static void gnss_pump_task(void*)
{
    QueueHandle_t q;
    uart_config_t cfg = {};
    cfg.baud_rate = SS_UART_GNSS_BAUD;
    cfg.data_bits = UART_DATA_8_BITS;
    cfg.parity = UART_PARITY_DISABLE;
    cfg.stop_bits = UART_STOP_BITS_1;
    cfg.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
    cfg.source_clk = UART_SCLK_DEFAULT;

    ESP_ERROR_CHECK(uart_driver_install(SS_UART_GNSS_PORT, SS_UART_GNSS_RX_BUF, 0, 16, &q, 0));
    ESP_ERROR_CHECK(uart_param_config(SS_UART_GNSS_PORT, &cfg));
    ESP_ERROR_CHECK(uart_set_pin(SS_UART_GNSS_PORT, SS_UART_GNSS_PIN_TX, SS_UART_GNSS_PIN_RX,
                                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    // Fire an event on '\n' so we wake exactly once per sentence.
    uart_enable_pattern_det_baud_intr(SS_UART_GNSS_PORT, '\n', 1, 9, 0, 0);
    uart_pattern_queue_reset(SS_UART_GNSS_PORT, 16);
    ESP_LOGI(TAG, "GNSS channel up: UART%d RX=%d @%d", int(SS_UART_GNSS_PORT), SS_UART_GNSS_PIN_RX,
             SS_UART_GNSS_BAUD);

    static char line[128];
    uart_event_t ev;
    for (;;) {
        if (xQueueReceive(q, &ev, portMAX_DELAY) != pdTRUE) continue;
        switch (ev.type) {
        case UART_PATTERN_DET: {
            const int pos = uart_pattern_pop_pos(SS_UART_GNSS_PORT);
            if (pos < 0) {
                uart_flush_input(SS_UART_GNSS_PORT);
                break;
            }
            const int n = uart_read_bytes(
                SS_UART_GNSS_PORT, line,
                size_t(pos) + 1 > sizeof(line) - 1 ? sizeof(line) - 1 : size_t(pos) + 1, 0);
            if (n <= 0) break;
            s_gnss_stats.rx_bytes += uint32_t(n);
            line[n] = '\0';
            // Trim trailing CR/LF.
            size_t len = size_t(n);
            while (len && (line[len - 1] == '\r' || line[len - 1] == '\n')) line[--len] = '\0';
            if (len) gnss_handle_sentence(line, len);
            break;
        }
        case UART_FIFO_OVF:
        case UART_BUFFER_FULL:
            s_gnss_stats.rx_overflows++;
            uart_flush_input(SS_UART_GNSS_PORT);
            xQueueReset(q);
            break;
        default:
            break;
        }
    }
}

// ---------------------------------------------------------------------------
// Coprocessor channel — SLIP deframe + CRC16, boot-preamble tolerant
// ---------------------------------------------------------------------------
static void coproc_feed(const uint8_t* buf, size_t n)
{
    // Bytes outside END…END are the S3 boot-ROM log / line noise: discarded
    // by construction because the accumulator only opens on SLIP_END.
    static uint8_t frame[COPROC_MAX_FRAME];
    static size_t flen = 0;
    static bool in_frame = false, esc = false;

    for (size_t i = 0; i < n; ++i) {
        const uint8_t b = buf[i];
        if (b == SLIP_END) {
            if (in_frame && flen >= 2) {
                const uint16_t rx_crc = uint16_t(frame[flen - 2]) << 8 | frame[flen - 1];
                if (crc16_ccitt(frame, flen - 2) == rx_crc) {
                    s_coproc_stats.rx_frames++;
                    if (s_coproc_cb) s_coproc_cb(frame, flen - 2);
                } else {
                    s_coproc_stats.rx_crc_errors++;
                }
            }
            in_frame = true;
            flen = 0;
            esc = false;
            continue;
        }
        if (!in_frame) continue;
        uint8_t out = b;
        if (esc) {
            out = (b == SLIP_ESC_END) ? SLIP_END : (b == SLIP_ESC_ESC) ? SLIP_ESC : b;
            esc = false;
        } else if (b == SLIP_ESC) {
            esc = true;
            continue;
        }
        if (flen < COPROC_MAX_FRAME)
            frame[flen++] = out;
        else {
            in_frame = false;
            flen = 0;
        } // oversize → resync
    }
}

static void coproc_pump_task(void*)
{
    QueueHandle_t q;
    uart_config_t cfg = {};
    cfg.baud_rate = SS_UART_COPROC_BAUD;
    cfg.data_bits = UART_DATA_8_BITS;
    cfg.parity = UART_PARITY_DISABLE;
    cfg.stop_bits = UART_STOP_BITS_1;
    cfg.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
    cfg.source_clk = UART_SCLK_DEFAULT;

    ESP_ERROR_CHECK(uart_driver_install(SS_UART_COPROC_PORT, SS_UART_COPROC_RX_BUF,
                                        SS_UART_COPROC_RX_BUF, 16, &q, 0));
    ESP_ERROR_CHECK(uart_param_config(SS_UART_COPROC_PORT, &cfg));
    ESP_ERROR_CHECK(uart_set_pin(SS_UART_COPROC_PORT, SS_UART_COPROC_PIN_TX, SS_UART_COPROC_PIN_RX,
                                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_LOGI(TAG, "coproc channel up: UART%d RX=%d TX=%d @%d", int(SS_UART_COPROC_PORT),
             SS_UART_COPROC_PIN_RX, SS_UART_COPROC_PIN_TX, SS_UART_COPROC_BAUD);

    static uint8_t buf[256];
    uart_event_t ev;
    for (;;) {
        if (xQueueReceive(q, &ev, portMAX_DELAY) != pdTRUE) continue;
        switch (ev.type) {
        case UART_DATA: {
            size_t left = ev.size;
            while (left) {
                const int n = uart_read_bytes(SS_UART_COPROC_PORT, buf,
                                              left < sizeof(buf) ? left : sizeof(buf), 0);
                if (n <= 0) break;
                s_coproc_stats.rx_bytes += uint32_t(n);
                coproc_feed(buf, size_t(n));
                left -= size_t(n);
            }
            break;
        }
        case UART_FIFO_OVF:
        case UART_BUFFER_FULL:
            s_coproc_stats.rx_overflows++;
            uart_flush_input(SS_UART_COPROC_PORT);
            xQueueReset(q);
            break;
        default:
            break;
        }
    }
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------
esp_err_t ss_uart_engine_start(void)
{
    s_fix_lock = xSemaphoreCreateMutex();
    if (!s_fix_lock) return ESP_ERR_NO_MEM;
    memset(&s_fix, 0, sizeof(s_fix));

#if CONFIG_SS_LITE_MOD_GNSS_BN880
    if (xTaskCreate(gnss_pump_task, "ss_gnss_uart", 4096, nullptr, 12, nullptr) != pdPASS)
        return ESP_ERR_NO_MEM;
#else
    ESP_LOGI(TAG, "GNSS module disabled (CONFIG_SS_LITE_MOD_GNSS_BN880=n)");
#endif

#if CONFIG_SS_LITE_MOD_COPROC_C6 || CONFIG_SS_LITE_MOD_COPROC_H2
    if (xTaskCreate(coproc_pump_task, "ss_coproc_uart", 4096, nullptr, 14, nullptr) != pdPASS)
        return ESP_ERR_NO_MEM;
#else
    ESP_LOGI(TAG, "mesh coprocessor disabled (CONFIG_SS_LITE_MOD_COPROC_*=n)");
#endif
    return ESP_OK;
}

bool ss_uart_gnss_last_fix(ss_gnss_fix_t* out)
{
    if (!out || !s_fix_lock) return false;
    xSemaphoreTake(s_fix_lock, portMAX_DELAY);
    *out = s_fix;
    const bool ever = s_fix_ever;
    xSemaphoreGive(s_fix_lock);
    return ever;
}

void ss_uart_gnss_set_tap(ss_nmea_sentence_cb_t cb)
{
    s_nmea_tap = cb;
}
void ss_uart_coproc_set_on_frame(ss_coproc_frame_cb_t cb)
{
    s_coproc_cb = cb;
}

esp_err_t ss_uart_coproc_send(const uint8_t* payload, size_t len)
{
#if CONFIG_SS_LITE_MOD_COPROC_C6 || CONFIG_SS_LITE_MOD_COPROC_H2
    if (!payload || len == 0 || len > COPROC_MAX_FRAME - 2) return ESP_ERR_INVALID_ARG;

    // Worst case: every byte escaped + CRC + 2 END markers.
    uint8_t* wire = static_cast<uint8_t*>(malloc(len * 2 + 8));
    if (!wire) return ESP_ERR_NO_MEM;

    size_t w = 0;
    wire[w++] = SLIP_END;
    auto put = [&](uint8_t b) {
        if (b == SLIP_END) {
            wire[w++] = SLIP_ESC;
            wire[w++] = SLIP_ESC_END;
        } else if (b == SLIP_ESC) {
            wire[w++] = SLIP_ESC;
            wire[w++] = SLIP_ESC_ESC;
        } else {
            wire[w++] = b;
        }
    };
    for (size_t i = 0; i < len; ++i) put(payload[i]);
    const uint16_t crc = crc16_ccitt(payload, len);
    put(uint8_t(crc >> 8));
    put(uint8_t(crc & 0xFF));
    wire[w++] = SLIP_END;

    const int sent = uart_write_bytes(SS_UART_COPROC_PORT, wire, w);
    free(wire);
    if (sent != int(w)) return ESP_ERR_TIMEOUT;
    s_coproc_stats.tx_bytes += uint32_t(w);
    s_coproc_stats.tx_frames++;
    return ESP_OK;
#else
    (void)payload;
    (void)len;
    return ESP_ERR_NOT_SUPPORTED;
#endif
}

void ss_uart_engine_stats(ss_uart_chan_stats_t* gnss, ss_uart_chan_stats_t* coproc)
{
    if (gnss) *gnss = s_gnss_stats;
    if (coproc) *coproc = s_coproc_stats;
}
