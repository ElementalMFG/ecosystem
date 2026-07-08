// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_power_core.c — pure decision logic for the ss_power HAL. No ESP-IDF
// runtime calls: this file must stay host-linkable for gtest.

#include "ss_power_core.h"

#include <string.h>

static bool state_is_valid(ss_power_state_t s)
{
    return s == SS_PWR_STATE_ON || s == SS_PWR_STATE_LIGHT_SLEEP || s == SS_PWR_STATE_DEEP_SLEEP ||
           s == SS_PWR_STATE_HIBERNATE || s == SS_PWR_STATE_SHUTDOWN;
}

ss_power_action_t ss_power_core_decide(ss_power_state_t current, ss_power_state_t target)
{
    if (!state_is_valid(current) || !state_is_valid(target)) { return SS_PWR_ACTION_INVALID; }
    switch (target) {
    case SS_PWR_STATE_ON:
        return SS_PWR_ACTION_NONE;
    case SS_PWR_STATE_LIGHT_SLEEP:
        return SS_PWR_ACTION_LIGHT_SLEEP;
    case SS_PWR_STATE_DEEP_SLEEP:
        return SS_PWR_ACTION_DEEP_SLEEP;
    case SS_PWR_STATE_HIBERNATE:
        return SS_PWR_ACTION_HIBERNATE;
    case SS_PWR_STATE_SHUTDOWN:
        return SS_PWR_ACTION_SHUTDOWN;
    default:
        return SS_PWR_ACTION_INVALID;
    }
}

bool ss_power_core_wake_is_deep_capable(int gpio)
{
    // ESP32-S3 RTC-capable GPIO range (see esp_deep_sleep wake domain): only
    // pins on the RTC IO mux, GPIO 0..21, can wake from deep sleep via
    // ext0/ext1. Pins outside this range are light-sleep-wake only.
    return gpio >= 0 && gpio <= 21;
}

bool ss_power_core_wake_add(ss_power_wake_table_t* t, int gpio, int level)
{
    if (t == NULL) { return false; }
    if (level != 0 && level != 1) { return false; }
    if (gpio < 0) { return false; }
    if (t->count >= SS_PWR_MAX_WAKE_SOURCES) { return false; }
    for (uint8_t i = 0; i < t->count; i++) {
        if (t->items[i].gpio == gpio) {
            return false; // duplicate
        }
    }
    t->items[t->count].gpio = gpio;
    t->items[t->count].level = level;
    t->count++;
    return true;
}

bool ss_power_core_timer_set(ss_power_timer_wake_t* t, uint64_t us)
{
    if (t == NULL) { return false; }
    if (us == 0 || us > SS_PWR_WAKE_TIMER_MAX_US) { return false; }
    t->armed = true;
    t->us = us;
    return true;
}

void ss_power_core_timer_clear(ss_power_timer_wake_t* t)
{
    if (t == NULL) { return; }
    t->armed = false;
    t->us = 0;
}

void ss_power_core_fill_nogauge_status(ss_power_status_t* out)
{
    if (out == NULL) { return; }
    // C-01 §Meshtastic-#7993: the Lite hardware exposes no battery-voltage
    // sense line (SS_BATTERY_SENSE_PRESENT == 0), so software cannot report a
    // real voltage, current, or state-of-charge. We report an honest "unknown"
    // rather than a fabricated reading.
    memset(out, 0, sizeof(*out));
    out->v_mv = 0;
    out->i_ma = 0;
    out->soc_percent = 0;
    out->temp_c_x10 = 0;
    out->charge_state = SS_CHG_UNKNOWN;
    out->on_external_power = false;
    out->usb_present = false;
    out->battery_sense_valid = false;
}
