// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_power.c — ESP-IDF glue implementing the frozen ss_hal_power.h contract for
// the Lite board. All decision logic lives in the host-testable ss_power_core;
// this file only turns those decisions into sleep/wake/restart syscalls.

#include "ss_hal_power.h"
#include "ss_power_core.h"

#include <string.h>

#include "driver/gpio.h"
#include "driver/rtc_io.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_system.h"

static const char* TAG = "ss_power";

static ss_power_state_t s_current = SS_PWR_STATE_ON;
static ss_power_wake_table_t s_wake;

// Wake-source registration is sleep-mode-specific on ESP32-S3 (there is no
// esp_deep_sleep_enable_gpio_wakeup on Xtensa targets — that API is
// RISC-V-only). Best-effort throughout: a rejected GPIO is logged and the
// others still register.
//
// LIGHT sleep: any GPIO may wake via the GPIO wakeup path (this is how the
// Lite touch INT on GPIO47 wakes — 47 is not RTC-capable, see below).
static void register_wake_sources_light(void)
{
    bool any = false;
    for (uint8_t i = 0; i < s_wake.count; i++) {
        const gpio_num_t gpio = (gpio_num_t)s_wake.items[i].gpio;
        const int level = s_wake.items[i].level;
        const esp_err_t err =
            gpio_wakeup_enable(gpio, level ? GPIO_INTR_HIGH_LEVEL : GPIO_INTR_LOW_LEVEL);
        if (err != ESP_OK) {
            ESP_LOGW(TAG, "light wake gpio %d level %d rejected: 0x%x", (int)gpio, level, err);
            continue;
        }
        any = true;
    }
    if (any) {
        const esp_err_t err = esp_sleep_enable_gpio_wakeup();
        if (err != ESP_OK) { ESP_LOGW(TAG, "gpio light-sleep wakeup enable failed: 0x%x", err); }
    }
}

// DEEP sleep / hibernate: only RTC-capable GPIOs (0..21 on S3) can wake.
// High-level pins share one EXT1 mask (ANY_HIGH); one low-level pin may use
// EXT0. Unsupported combinations are logged, never silently dropped.
// Lite's deep-wake set (LoRa DIO1 = GPIO1, BOOT = GPIO0) is RTC-capable;
// touch INT (GPIO47) is deliberately light-sleep-only (C-01).
static void register_wake_sources_deep(void)
{
    uint64_t high_mask = 0;
    int low_gpio = -1;
    for (uint8_t i = 0; i < s_wake.count; i++) {
        const int gpio = s_wake.items[i].gpio;
        const int level = s_wake.items[i].level;
        if (!rtc_gpio_is_valid_gpio((gpio_num_t)gpio)) {
            ESP_LOGW(TAG, "deep wake gpio %d skipped: not RTC-capable on ESP32-S3 (0..21)", gpio);
            continue;
        }
        if (level) {
            high_mask |= 1ULL << (unsigned)gpio;
        } else if (low_gpio < 0) {
            low_gpio = gpio;
        } else {
            ESP_LOGW(TAG, "deep wake gpio %d skipped: only one low-level pin (EXT0) on S3", gpio);
        }
    }
    if (high_mask != 0) {
        const esp_err_t err = esp_sleep_enable_ext1_wakeup(high_mask, ESP_EXT1_WAKEUP_ANY_HIGH);
        if (err != ESP_OK) {
            ESP_LOGW(TAG, "ext1 high-mask 0x%llx rejected: 0x%x", (unsigned long long)high_mask,
                     err);
        }
    }
    if (low_gpio >= 0) {
        const esp_err_t err = esp_sleep_enable_ext0_wakeup((gpio_num_t)low_gpio, 0);
        if (err != ESP_OK) { ESP_LOGW(TAG, "ext0 low gpio %d rejected: 0x%x", low_gpio, err); }
    }
}

esp_err_t ss_power_init(void)
{
    s_current = SS_PWR_STATE_ON;
    memset(&s_wake, 0, sizeof(s_wake));
    // C-01 §Meshtastic-#7993: Lite exposes no battery-sense line, so power
    // status is reported as "unknown" (no fuel gauge) by design.
    ESP_LOGI(TAG, "ss_power init: no battery-sense line (C-01 Meshtastic-#7993), "
                  "status reported as unknown");
    return ESP_OK;
}

esp_err_t ss_power_status(ss_power_status_t* out)
{
    if (out == NULL) { return ESP_ERR_INVALID_ARG; }
    ss_power_core_fill_nogauge_status(out);
    return ESP_OK;
}

esp_err_t ss_power_enter(ss_power_state_t s)
{
    const ss_power_action_t action = ss_power_core_decide(s_current, s);
    switch (action) {
    case SS_PWR_ACTION_INVALID:
        return ESP_ERR_INVALID_ARG;

    case SS_PWR_ACTION_NONE:
        s_current = SS_PWR_STATE_ON;
        return ESP_OK;

    case SS_PWR_ACTION_LIGHT_SLEEP:
        register_wake_sources_light();
        s_current = SS_PWR_STATE_LIGHT_SLEEP;
        esp_light_sleep_start();
        // Resumed: the CPU keeps running after light sleep.
        s_current = SS_PWR_STATE_ON;
        return ESP_OK;

    case SS_PWR_ACTION_DEEP_SLEEP:
    case SS_PWR_ACTION_HIBERNATE:
        register_wake_sources_deep();
        s_current = s;
        esp_deep_sleep_start(); // does not return
        return ESP_OK;

    case SS_PWR_ACTION_SHUTDOWN:
        // Disable all wake sources so the device stays down until a manual
        // power cycle / reset.
        esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
        s_current = SS_PWR_STATE_SHUTDOWN;
        esp_deep_sleep_start(); // does not return
        return ESP_OK;
    }
    return ESP_ERR_INVALID_ARG;
}

esp_err_t ss_power_wake_source_add(int gpio, int level)
{
    if (ss_power_core_wake_add(&s_wake, gpio, level)) { return ESP_OK; }
    if (s_wake.count >= SS_PWR_MAX_WAKE_SOURCES) { return ESP_ERR_NO_MEM; }
    return ESP_ERR_INVALID_ARG;
}

esp_err_t ss_power_reboot(void)
{
    esp_restart(); // does not return
    return ESP_OK;
}

esp_err_t ss_power_shutdown(void)
{
    return ss_power_enter(SS_PWR_STATE_SHUTDOWN);
}
