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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_check.h"

#include "board_config.h"
#include "ss_tasks.h"
#include "ss_gnss.h" // GNSS channel now lives in the ss_gnss HAL component

static const char* TAG = "ss.uart";

// ---------------------------------------------------------------------------
// Shared state (coprocessor channel only; GNSS state moved to ss_gnss)
// ---------------------------------------------------------------------------
static ss_coproc_frame_cb_t s_coproc_cb = nullptr;
static ss_uart_chan_stats_t s_coproc_stats;

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
    // GNSS channel now lives in the ss_gnss HAL component (S-03-027); both calls
    // are cap/CONFIG-gated internally and idle at zero cost when absent.
    esp_err_t gerr = ss_gnss_init();
    if (gerr != ESP_OK) return gerr;
    gerr = ss_gnss_start();
    if (gerr != ESP_OK) return gerr;

#if CONFIG_SS_LITE_MOD_COPROC_C6 || CONFIG_SS_LITE_MOD_COPROC_H2
    // TWDT-EXEMPT (S-02-009): coproc_pump_task blocks on xQueueReceive(portMAX_
    // DELAY) waiting for UART events; when the coprocessor is quiet it sleeps
    // indefinitely (>> the 5 s TWDT deadline), so it deliberately does NOT
    // subscribe to the task watchdog.
    if (ss_task_create(coproc_pump_task, "ss_coproc_uart", 4096, nullptr, SS_PRIO_COMMS_CRITICAL,
                       nullptr) != pdPASS)
        return ESP_ERR_NO_MEM;
#else
    ESP_LOGI(TAG, "mesh coprocessor disabled (CONFIG_SS_LITE_MOD_COPROC_*=n)");
#endif
    return ESP_OK;
}

bool ss_uart_gnss_last_fix(ss_gnss_fix_t* out)
{
    return ss_gnss_get(out) == ESP_OK;
}

void ss_uart_gnss_set_tap(ss_nmea_sentence_cb_t cb)
{
    // ss_nmea_sentence_cb_t and ss_gnss_nmea_tap_t are identical signatures.
    ss_gnss_set_nmea_tap((ss_gnss_nmea_tap_t)cb);
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
    if (gnss) {
        ss_gnss_stats_t g = {};
        ss_gnss_get_stats(&g);
        *gnss = ss_uart_chan_stats_t{};
        gnss->rx_bytes = g.rx_bytes;
        gnss->rx_frames = g.rx_frames;
        gnss->rx_crc_errors = g.rx_crc_errors;
        gnss->rx_overflows = g.rx_overflows;
    }
    if (coproc) *coproc = s_coproc_stats;
}
