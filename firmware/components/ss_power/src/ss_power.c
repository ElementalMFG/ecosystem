// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_power.c — ESP-IDF glue implementing the frozen ss_hal_power.h contract for
// the Lite board. All decision logic lives in the host-testable ss_power_core;
// this file only turns those decisions into sleep/wake/restart syscalls.

#include "ss_hal_power.h"
#include "ss_power_core.h"
#include "ss_power_lite.h"

#include "board_config.h" // canonical Lite pin map (via ss_hal include path)

#include <string.h>

#include "driver/gpio.h"
#include "driver/rtc_io.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_system.h"

static const char* TAG = "ss_power";

static ss_power_state_t s_current = SS_PWR_STATE_ON;
static ss_power_wake_table_t s_wake;
// Timer wake (S-03-030): armed at set-time, not at sleep entry —
// esp_sleep_enable_timer_wakeup() records a duration that IDF measures from
// each sleep start, so set-time arming already gives the contract's
// "countdown from sleep entry" + sticky-until-clear semantics.
static ss_power_timer_wake_t s_timer;

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
        // Pure predicate is the decision: ss_power_core_wake_is_deep_capable()
        // encodes the S3 RTC-capable range (0..21) and is host-tested.
        // rtc_gpio_is_valid_gpio() stays as a defense-in-depth secondary check
        // in case a target's RTC IO map ever diverges from the 0..21 range.
        if (!ss_power_core_wake_is_deep_capable(gpio) ||
            !rtc_gpio_is_valid_gpio((gpio_num_t)gpio)) {
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
    // Re-init while an arming is live: disarm through the platform BEFORE
    // dropping the record, so a platform timer can never outlive the record
    // that armed it. Skipped when not armed — IDF v5.3.5
    // esp_sleep_disable_wakeup_source() ESP_LOGEs and returns
    // ESP_ERR_INVALID_STATE when the source is not currently enabled, which
    // would put an error line in every clean boot.
    if (s_timer.armed) { (void)esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER); }
    memset(&s_timer, 0, sizeof(s_timer));
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

esp_err_t ss_power_wake_timer_set(uint64_t us)
{
    // Deliberate copy: core_timer_set overwrites every field on success, but
    // seeding from the live record keeps the reject-path "prior arming
    // unchanged" semantics obvious and survives future partial-update fields.
    ss_power_timer_wake_t staged = s_timer;
    if (!ss_power_core_timer_set(&staged, us)) { return ESP_ERR_INVALID_ARG; }
    const esp_err_t err = esp_sleep_enable_timer_wakeup(us);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "timer wake %llu us rejected by platform: 0x%x", (unsigned long long)us, err);
        return err;
    }
    s_timer = staged;
    return ESP_OK;
}

esp_err_t ss_power_wake_timer_clear(void)
{
    // Gated on the record: IDF v5.3.5 esp_sleep_disable_wakeup_source()
    // ESP_LOGEs and returns ESP_ERR_INVALID_STATE for a source that is not
    // currently enabled (verified in sleep_modes.c: CHECK_SOURCE requires the
    // trigger bit), so an unconditional call would log an error on every
    // idempotent not-armed clear. Record-vs-platform divergence is prevented
    // at the sources instead: set() commits the record only after the platform
    // accepts, and init() disarms a live arming before dropping the record.
    if (s_timer.armed) {
        const esp_err_t err = esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);
        // Absorb INVALID_STATE as defense-in-depth: if the platform already
        // has no timer armed, "disarmed" is exactly this call's
        // post-condition, not a failure of an idempotent operation.
        if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
            ESP_LOGW(TAG, "timer wake disable rejected: 0x%x", err);
            return err;
        }
    }
    ss_power_core_timer_clear(&s_timer);
    return ESP_OK;
}

esp_err_t ss_power_wake_lite_defaults(void)
{
    // Touch INT (GPIO47): GT911 INT idles HIGH, asserts LOW on touch -> wake on
    // level 0. Not RTC-capable on S3, so light-sleep-only (filtered from the
    // deep-wake ext0/ext1 set by ss_power_core_wake_is_deep_capable). C-01 §4.3.
    esp_err_t err = ss_power_wake_source_add(SS_TOUCH_PIN_INT, 0);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "lite touch INT (gpio %d) wake add failed: 0x%x", SS_TOUCH_PIN_INT, err);
        return err;
    }
    // LoRa DIO1 (GPIO1): SX1262 DIO1 IRQ asserts HIGH -> wake on level 1.
    // RTC-capable, so this serves both light and deep sleep.
    err = ss_power_wake_source_add(SS_LORA_PIN_DIO1, 1);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "lite LoRa DIO1 (gpio %d) wake add failed: 0x%x", SS_LORA_PIN_DIO1, err);
        return err;
    }
    // NF-PWR-01 periodic RTC-timer wake is armed separately by the duty-cycle
    // owner via ss_power_wake_timer_set(); deliberately not armed here.
    return ESP_OK;
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
