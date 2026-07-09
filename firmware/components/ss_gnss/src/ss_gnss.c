// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_gnss.c — ESP-IDF glue implementing the frozen ss_hal_gnss.h contract on
// UART1. All NMEA parse logic lives in the host-testable ss_gnss_core; this
// file owns the UART driver, pump task, fix mutex, NMEA tap and RX counters.
//
// Part-agnostic: the channel drives any NMEA-0183 module. ss_gnss_sleep is a
// documented no-op pending part confirmation at the D-0013 hardware session.

#include "ss_gnss.h"
#include "ss_gnss_core.h"

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "driver/uart.h"
#include "esp_err.h"
#include "esp_log.h"

#include "board_config.h" // canonical Lite pin map (via ss_hal include path)
#include "ss_hal.h"       // ss_hal_has_cap, SS_CAP_GNSS_L1

static const char* TAG = "ss.gnss";

// ss_input/ss_touch precedent: component tasks use a raw priority constant
// because ss_tasks.h (SS_PRIO_COMMS == 12) is main-only. 12 == GNSS byte pump.
#define SS_GNSS_PUMP_PRIO 12

// ---------------------------------------------------------------------------
// State (glue-owned; the core stays pure)
// ---------------------------------------------------------------------------
static ss_gnss_fix_t s_fix;
static bool s_fix_ever = false;
static SemaphoreHandle_t s_fix_lock = NULL;
static ss_gnss_nmea_tap_t s_nmea_tap = NULL;
static ss_gnss_stats_t s_stats;
static TaskHandle_t s_pump = NULL;

// ---------------------------------------------------------------------------
// Pump task — NMEA line framing on UART1
// ---------------------------------------------------------------------------
static void gnss_pump_task(void* arg)
{
    (void)arg;
    QueueHandle_t q;
    uart_config_t cfg = {
        .baud_rate = SS_UART_GNSS_BAUD,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    ESP_ERROR_CHECK(uart_driver_install(SS_UART_GNSS_PORT, SS_UART_GNSS_RX_BUF, 0, 16, &q, 0));
    ESP_ERROR_CHECK(uart_param_config(SS_UART_GNSS_PORT, &cfg));
    ESP_ERROR_CHECK(uart_set_pin(SS_UART_GNSS_PORT, SS_UART_GNSS_PIN_TX, SS_UART_GNSS_PIN_RX,
                                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    // Fire an event on '\n' so we wake exactly once per sentence.
    uart_enable_pattern_det_baud_intr(SS_UART_GNSS_PORT, '\n', 1, 9, 0, 0);
    uart_pattern_queue_reset(SS_UART_GNSS_PORT, 16);
    ESP_LOGI(TAG, "GNSS channel up: UART%d RX=%d @%d", (int)SS_UART_GNSS_PORT, SS_UART_GNSS_PIN_RX,
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
                (size_t)pos + 1 > sizeof(line) - 1 ? sizeof(line) - 1 : (size_t)pos + 1, 0);
            if (n <= 0) break;
            s_stats.rx_bytes += (uint32_t)n;
            line[n] = '\0';
            // Trim trailing CR/LF.
            size_t len = (size_t)n;
            while (len && (line[len - 1] == '\r' || line[len - 1] == '\n')) line[--len] = '\0';
            if (!len) break;

            s_stats.rx_frames++;
            ss_gnss_nmea_tap_t tap = s_nmea_tap;
            if (tap) tap(line, len);

            xSemaphoreTake(s_fix_lock, portMAX_DELAY);
            const ss_gnss_parse_result_t r = ss_gnss_core_parse(line, len, &s_fix);
            if (r == SS_GNSS_PARSE_RMC || r == SS_GNSS_PARSE_GGA) s_fix_ever = true;
            xSemaphoreGive(s_fix_lock);
            if (r == SS_GNSS_PARSE_CHECKSUM_FAIL) s_stats.rx_crc_errors++;
            break;
        }
        case UART_FIFO_OVF:
        case UART_BUFFER_FULL:
            s_stats.rx_overflows++;
            uart_flush_input(SS_UART_GNSS_PORT);
            xQueueReset(q);
            break;
        default:
            break;
        }
    }
}

// ---------------------------------------------------------------------------
// HAL contract (ss_hal_gnss.h)
// ---------------------------------------------------------------------------
esp_err_t ss_gnss_init(void)
{
    if (!s_fix_lock) {
        s_fix_lock = xSemaphoreCreateMutex();
        if (!s_fix_lock) return ESP_ERR_NO_MEM;
    }
    memset(&s_fix, 0, sizeof(s_fix));
    s_fix_ever = false;
    return ESP_OK;
}

esp_err_t ss_gnss_start(void)
{
#if CONFIG_SS_LITE_MOD_GNSS_BN880
    if (!ss_hal_has_cap(SS_CAP_GNSS_L1)) {
        ESP_LOGI(TAG, "no SS_CAP_GNSS_L1 on this board: GNSS idle");
        return ESP_OK;
    }
    if (s_pump) return ESP_OK; // already running (idempotent)
    // TWDT-EXEMPT (S-02-009): gnss_pump_task blocks on xQueueReceive(portMAX_
    // DELAY) waiting for UART events; when the GNSS is idle / absent it sleeps
    // indefinitely (>> the 5 s TWDT deadline), so it deliberately does NOT
    // subscribe to the task watchdog.
    if (xTaskCreate(gnss_pump_task, "ss_gnss_uart", 4096, NULL, SS_GNSS_PUMP_PRIO, &s_pump) !=
        pdPASS)
        return ESP_ERR_NO_MEM;
    return ESP_OK;
#else
    ESP_LOGI(TAG, "GNSS module disabled (CONFIG_SS_LITE_MOD_GNSS_BN880=n)");
    return ESP_OK;
#endif
}

esp_err_t ss_gnss_stop(void)
{
    if (s_pump) {
        vTaskDelete(s_pump);
        s_pump = NULL;
        uart_driver_delete(SS_UART_GNSS_PORT);
    }
    return ESP_OK;
}

esp_err_t ss_gnss_get(ss_gnss_fix_t* out)
{
    if (!out || !s_fix_lock) return ESP_ERR_INVALID_ARG;
    xSemaphoreTake(s_fix_lock, portMAX_DELAY);
    *out = s_fix;
    const bool ever = s_fix_ever;
    xSemaphoreGive(s_fix_lock);
    return ever ? ESP_OK : ESP_ERR_INVALID_STATE;
}

esp_err_t ss_gnss_sleep(bool on)
{
    // Part-agnostic no-op (S-03-027): the real standby/wake command is
    // module-specific and is deferred to part confirmation at the D-0013
    // hardware session. UART framing keeps idling at zero CPU when quiet.
    ESP_LOGI(TAG, "ss_gnss_sleep(%d): no-op pending D-0013 part confirmation", (int)on);
    return ESP_OK;
}

// ---------------------------------------------------------------------------
// Component extras (ss_gnss.h)
// ---------------------------------------------------------------------------
void ss_gnss_set_nmea_tap(ss_gnss_nmea_tap_t cb)
{
    s_nmea_tap = cb;
}

void ss_gnss_get_stats(ss_gnss_stats_t* out)
{
    if (out) *out = s_stats;
}
