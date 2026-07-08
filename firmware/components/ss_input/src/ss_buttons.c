// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_buttons.c — ESP-IDF glue implementing the frozen ss_hal_buttons.h
// contract for the Lite board's BOOT button (GPIO0). Debounce decision logic
// lives in the host-testable ss_debounce core; this file only samples the pin
// and dispatches derived events.
//
// RECOVERY PIN-SHARE ORDERING (critical — see firmware/main/ss_recovery.h):
// GPIO0 is the ESP32-S3 BOOT strapping pin and is ALSO SS_LORA_PIN_CS. For the
// first CONFIG_SS_RECOVERY_ENTRY_WINDOW_MS of every boot it is owned by
// ss_recovery_watch_start(), which samples it for the recovery hold gesture
// and RESETS the pin config when the window closes. This module MUST NOT claim
// or configure GPIO0 until AFTER that window elapses, or it would fight the
// recovery watcher for the strapping pin. ss_buttons_init() therefore only
// ARMS an esp_timer one-shot for CONFIG_SS_RECOVERY_ENTRY_WINDOW_MS; when it
// fires (window closed, watcher retired), acquire_button() configures GPIO0 as
// input + pull-up (BOOT is active-low) and starts a periodic sampling timer
// that feeds the debounce core. If the window is 0 (watcher disabled) the pin
// is acquired immediately.

#include "ss_hal_buttons.h"
#include "ss_hal.h"
#include "ss_input_core.h"

#include <string.h>

#include "sdkconfig.h"

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"

static const char* TAG = "ss_buttons";

#ifndef CONFIG_SS_RECOVERY_ENTRY_WINDOW_MS
#define CONFIG_SS_RECOVERY_ENTRY_WINDOW_MS 0
#endif

#define SS_BTN_GPIO           SS_LORA_PIN_CS // GPIO0 (BOOT), shared with LoRa CS
#define SS_BTN_SAMPLE_PERIOD_US 5000          // 5 ms sampling cadence

static ss_btn_cb_t s_cb;
static void* s_user;
static ss_debounce_t s_deb;
static esp_timer_handle_t s_defer_timer;
static esp_timer_handle_t s_sample_timer;
static ss_input_event_t s_latched; // last event, for poll_once
static volatile bool s_have_latched;
static bool s_acquired;

static uint32_t now_ms(void)
{
    return (uint32_t)(esp_timer_get_time() / 1000);
}

// Periodic sampler: read the debounced BOOT level and feed the core.
static void sample_cb(void* arg)
{
    (void)arg;
    // BOOT is active-low (pull-up + button to GND): pressed == level 0.
    const bool pressed = gpio_get_level((gpio_num_t)SS_BTN_GPIO) == 0;
    ss_input_event_t ev;
    if (ss_debounce_step(&s_deb, pressed, now_ms(), &ev)) {
        s_latched = ev;
        s_have_latched = true;
        if (s_cb != NULL) { s_cb(&ev, s_user); }
    }
}

// Acquire GPIO0 AFTER the recovery entry window (see doc-block).
static void acquire_button(void* arg)
{
    (void)arg;
    if (s_acquired) { return; }

    const gpio_config_t cfg = {
        .pin_bit_mask = 1ULL << SS_BTN_GPIO,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    if (gpio_config(&cfg) != ESP_OK) {
        ESP_LOGE(TAG, "GPIO%d config failed", (int)SS_BTN_GPIO);
        return;
    }
    ss_debounce_init(&s_deb);

    const esp_timer_create_args_t sargs = {
        .callback = sample_cb,
        .name = "ss_btn_sample",
    };
    if (esp_timer_create(&sargs, &s_sample_timer) != ESP_OK ||
        esp_timer_start_periodic(s_sample_timer, SS_BTN_SAMPLE_PERIOD_US) != ESP_OK) {
        ESP_LOGE(TAG, "sample timer start failed");
        return;
    }
    s_acquired = true;
    ESP_LOGI(TAG, "BOOT button (GPIO%d) acquired after recovery window", (int)SS_BTN_GPIO);
}

esp_err_t ss_buttons_init(void)
{
    ss_debounce_init(&s_deb);
    s_have_latched = false;

    const uint32_t window_ms = CONFIG_SS_RECOVERY_ENTRY_WINDOW_MS;
    if (window_ms == 0) {
        acquire_button(NULL); // watcher disabled: GPIO0 is free now
        return ESP_OK;
    }

    const esp_timer_create_args_t dargs = {
        .callback = acquire_button,
        .name = "ss_btn_defer",
    };
    esp_err_t err = esp_timer_create(&dargs, &s_defer_timer);
    if (err != ESP_OK) { return err; }
    // Defer acquisition until the recovery watcher has released GPIO0.
    return esp_timer_start_once(s_defer_timer, (uint64_t)window_ms * 1000u);
}

esp_err_t ss_buttons_register(ss_btn_cb_t cb, void* user)
{
    s_cb = cb;
    s_user = user;
    return ESP_OK;
}

esp_err_t ss_buttons_poll_once(ss_input_event_t* out)
{
    if (out == NULL) { return ESP_ERR_INVALID_ARG; }
    if (!s_have_latched) { return ESP_ERR_NOT_FOUND; }
    *out = s_latched;
    s_have_latched = false;
    return ESP_OK;
}
