// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// main.cpp — SS-SP-Lite boot framework (Master Project Directive scaffolding).
//
// Boot order (directive priority: display FIRST for user feedback, then the
// dual-UART engine, then the sensor/diagnostic threads):
//
//   1. NVS + board banner (identity, capability mask)
//   2. ss_display_boot_init()      — SPI3 @40 MHz, ILI9488, test pattern (goal A)
//   3. ss_uart_engine_start()      — GNSS + coproc ring-buffer pumps  (goal B)
//   4. ss_compass_start()          — tilt-compensated compass thread  (goal C)
//   5. ss_diag_start()             — buzzer/speaker diag + power WD   (goal D)
//   6. heartbeat loop              — periodic status logging
//
// Everything after boot migrates behind the HAL seams as EPIC-03 lands:
//   TODO(EPIC-03): replace direct module calls with ss_hal_init() + drivers.
//   TODO(EPIC-15): LVGL 9 + ss_ui multi-page shell hooks ss_display_boot_blit
//                  as its flush callback, then owns the panel.
//   TODO(EPIC-05/06): ss_rns/ss_lxmf attach to the coproc frame callback.

#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_chip_info.h"

#include "board_config.h"
#include "ss_bootmark.h"
#include "ss_log.h"
#include "ss_memwatch.h"
#include "ss_panic_guard.h"
#include "ss_display_boot.h"
#include "ss_uart_engine.h"
#include "ss_compass.h"
#include "ss_diag.h"

static const char* TAG = "ss.main";

static void banner(void)
{
    esp_chip_info_t chip;
    esp_chip_info(&chip);
    ESP_LOGI(TAG, "==============================================");
    ESP_LOGI(TAG, " SS-SP firmware — board %s (%s)", SS_BOARD_ID_STR, SS_BOARD_MCU);
    ESP_LOGI(TAG, " cores=%d rev=%d flash=%dMB psram=%dMB", chip.cores, chip.revision,
             SS_BOARD_FLASH_MB, SS_BOARD_PSRAM_MB);
    ESP_LOGI(TAG, " caps mask = 0x%08" PRIx32, uint32_t(SS_BOARD_CAPS));
#if CONFIG_SS_LITE_MOD_HALOW
    ESP_LOGI(TAG, " wireless header: Wi-Fi HaLow module");
#else
    ESP_LOGI(TAG, " wireless header: SX1262 LoRa (stock)");
#endif
#if CONFIG_SS_LITE_MOD_GNSS_BN880
    ESP_LOGI(TAG, " GNSS: BN-880 on UART1 (%d/%d) + HMC5883L @0x1E", SS_UART_GNSS_PIN_RX,
             SS_UART_GNSS_PIN_TX);
#endif
#if CONFIG_SS_LITE_MOD_COPROC_C6
    ESP_LOGI(TAG, " coproc: ESP32-C6 on UART2 (%d/%d)", SS_UART_COPROC_PIN_RX,
             SS_UART_COPROC_PIN_TX);
#elif CONFIG_SS_LITE_MOD_COPROC_H2
    ESP_LOGI(TAG, " coproc: ESP32-H2 on UART2 (%d/%d)", SS_UART_COPROC_PIN_RX,
             SS_UART_COPROC_PIN_TX);
#endif
#if CONFIG_SS_LITE_MOD_IMU
    ESP_LOGI(TAG, " IMU: 6-axis @0x68 (tilt-compensated compass)");
#endif
    ESP_LOGI(TAG, "==============================================");
}

extern "C" void app_main(void)
{
    // --- 0. Panic boot gate FIRST (S-02-008): decide normal vs safe mode ---
    // Must run before any app task exists so a crash-looping subsystem can
    // never be re-entered on the boot that trips the loop breaker.
    ss_panic_guard_boot_gate();
    if (ss_panic_guard_in_safe_mode()) {
        ss_panic_guard_safe_mode_loop(); // never returns
    }
    ss_bootmark("gate");

    // --- 1. NVS (required by Wi-Fi/BLE later; cheap to do first) -----------
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    ss_bootmark("nvs");
    banner();
    SS_LOGI("boot", "ss_log online (redaction active)");

    // --- 2. Display FIRST (directive goal A: user feedback ASAP) -----------
    err = ss_display_boot_init();
    if (err == ESP_OK) {
        ss_display_boot_test_pattern(); // R/G/B + backlight ramp
    } else {
        ESP_LOGE(TAG, "display init failed: %s — continuing headless", esp_err_to_name(err));
    }
    ss_bootmark("display");

    // --- 3. Dual-UART engine SECOND (directive goal B) ----------------------
    ESP_ERROR_CHECK(ss_uart_engine_start());
    ss_bootmark("uart");

    // --- 4. Compass thread (directive goal C) -------------------------------
    ESP_ERROR_CHECK(ss_compass_start());
    ss_bootmark("compass");

    // --- 5. Diagnostics + power watchdog (directive goal D) -----------------
    ESP_ERROR_CHECK(ss_diag_start());
    ss_diag_beep(SS_DIAG_TONE_BOOT);
    ss_bootmark("diag");

    // --- 5b. Runtime resource telemetry (S-02-011): heap/stack/idle-load ----
    // Low-priority static-stack sampler; started after logging + core threads
    // so its first sample reflects the steady-state footprint.
    ss_memwatch_start();
    ss_bootmark("memwatch");

    ss_bootmark("app_ready");
    ESP_LOGI(TAG, "boot complete — entering heartbeat");
    ss_bootmark_report();

    // Boot survived bring-up: arm the 60 s stability window that clears the
    // consecutive-panic count (S-02-008).
    ss_panic_guard_arm_stability_timer();

    // --- 6. Heartbeat: 10 s status until ss_ui takes over (EPIC-15) ---------
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(10000));

        ss_uart_chan_stats_t gnss, coproc;
        ss_uart_engine_stats(&gnss, &coproc);

        ss_gnss_fix_t fix;
        const bool have_fix_struct = ss_uart_gnss_last_fix(&fix);

        ss_compass_reading_t hdg;
        const bool have_hdg = ss_compass_get(&hdg);

        ESP_LOGI(TAG,
                 "hb: gnss[rx=%" PRIu32 "B f=%" PRIu32 " crcE=%" PRIu32 " ovf=%" PRIu32 "]"
                 " coproc[rx=%" PRIu32 "B f=%" PRIu32 " crcE=%" PRIu32 " tx=%" PRIu32 "B]",
                 gnss.rx_bytes, gnss.rx_frames, gnss.rx_crc_errors, gnss.rx_overflows,
                 coproc.rx_bytes, coproc.rx_frames, coproc.rx_crc_errors, coproc.tx_bytes);

        if (have_fix_struct && fix.has_fix)
            ESP_LOGI(TAG, "hb: fix %.6f,%.6f alt=%.1fm sats=%u hdop=%.1f", fix.lat_deg, fix.lon_deg,
                     double(fix.alt_m), unsigned(fix.sats_used), double(fix.hdop));
        else
            ESP_LOGI(TAG, "hb: no GNSS fix yet");

        if (have_hdg && hdg.valid)
            ESP_LOGI(TAG, "hb: heading %.1f deg (src=%d pitch=%.1f roll=%.1f)",
                     double(hdg.heading_deg), int(hdg.src), double(hdg.pitch_deg),
                     double(hdg.roll_deg));
    }
}
