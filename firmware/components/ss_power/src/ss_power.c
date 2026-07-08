// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_power.c — ESP-IDF glue implementing the frozen ss_hal_power.h contract for
// the Lite board. All decision logic lives in the host-testable ss_power_core;
// this file only turns those decisions into sleep/wake/restart syscalls.

#include "ss_hal_power.h"
#include "ss_power_core.h"

#include <string.h>

#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_system.h"

static const char* TAG = "ss_power";

static ss_power_state_t s_current = SS_PWR_STATE_ON;
static ss_power_wake_table_t s_wake;

// Register every queued wake source with the sleep subsystem. Best-effort:
// return codes are logged but a single bad GPIO does not abort the others.
static void register_wake_sources(void)
{
    for (uint8_t i = 0; i < s_wake.count; i++) {
        const int gpio = s_wake.items[i].gpio;
        const int level = s_wake.items[i].level;
        const esp_err_t err = esp_deep_sleep_enable_gpio_wakeup(
            (1ULL << (unsigned)gpio), level ? ESP_GPIO_WAKEUP_GPIO_HIGH : ESP_GPIO_WAKEUP_GPIO_LOW);
        if (err != ESP_OK) {
            ESP_LOGW(TAG, "wake gpio %d level %d rejected: 0x%x", gpio, level, err);
        }
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
        register_wake_sources();
        s_current = SS_PWR_STATE_LIGHT_SLEEP;
        esp_light_sleep_start();
        // Resumed: the CPU keeps running after light sleep.
        s_current = SS_PWR_STATE_ON;
        return ESP_OK;

    case SS_PWR_ACTION_DEEP_SLEEP:
    case SS_PWR_ACTION_HIBERNATE:
        register_wake_sources();
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
