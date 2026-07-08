// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_power_core.h — pure, host-testable decision core for the ss_power HAL.
//
// This layer holds NO ESP-IDF runtime dependency: it only reasons about the
// frozen contract types (ss_hal_power.h) so it can be exercised with host
// gtest. The IDF glue (ss_power.c) turns these decisions into sleep/wake calls.

#pragma once
#include "ss_hal_power.h"

// Concrete platform action derived from a requested power-state transition.
typedef enum {
    SS_PWR_ACTION_NONE, // stay awake (target == ON)
    SS_PWR_ACTION_LIGHT_SLEEP,
    SS_PWR_ACTION_DEEP_SLEEP,
    SS_PWR_ACTION_HIBERNATE,
    SS_PWR_ACTION_SHUTDOWN,
    SS_PWR_ACTION_INVALID, // current/target outside the valid enum range
} ss_power_action_t;

#define SS_PWR_MAX_WAKE_SOURCES 8

typedef struct {
    int gpio;
    int level;
} ss_power_wake_src_t;

typedef struct {
    ss_power_wake_src_t items[SS_PWR_MAX_WAKE_SOURCES];
    uint8_t count;
} ss_power_wake_table_t;

// Runtime record for the RTC-timer wake source (S-03-030). Pure state — the
// IDF glue mirrors it into esp_sleep_enable_timer_wakeup()/
// esp_sleep_disable_wakeup_source() calls.
typedef struct {
    bool armed;
    uint64_t us; // countdown from sleep entry; meaningful only when armed
} ss_power_timer_wake_t;

// Decide the platform action for a transition from `current` to `target`.
// Pre:  none (arguments are validated).
// Post: returns SS_PWR_ACTION_INVALID iff either argument is outside the five
//       valid ss_power_state_t values; SS_PWR_ACTION_NONE when target == ON;
//       otherwise the sleep action matching `target`.
ss_power_action_t ss_power_core_decide(ss_power_state_t current, ss_power_state_t target);

// Report whether `gpio` can serve as a deep-sleep / hibernate wake source.
// Target dependency: on the ESP32-S3 (the only Lite target) the deep-sleep
// wake domain (ext0/ext1) is driven from the RTC IO mux, which reaches only
// GPIO 0..21; higher pins have no RTC-domain path and cannot wake from deep
// sleep. This is the pure decision behind the light-vs-deep wake partition:
// e.g. the Lite touch INT on GPIO47 is light-sleep-only, while LoRa DIO1 on
// GPIO1 is deep-capable.
// Pre:  none (arguments are validated).
// Post: returns true iff 0 <= gpio <= 21; false otherwise (including negative).
bool ss_power_core_wake_is_deep_capable(int gpio);

// Append a wake source (gpio, level) to `t`.
// Pre:  none (arguments are validated).
// Post: returns false and leaves `t` unchanged when `t` is NULL, `level` is not
//       0/1, `gpio` is negative, the table is full, or `gpio` duplicates an
//       existing entry; otherwise appends the entry, increments count, returns
//       true.
bool ss_power_core_wake_add(ss_power_wake_table_t* t, int gpio, int level);

// Validate and record a timer-wake arming (pure state; no platform calls).
// Pre:  none (arguments are validated).
// Post: returns false and leaves `t` unchanged when `t` is NULL, `us` is 0, or
//       `us` exceeds SS_PWR_WAKE_TIMER_MAX_US; otherwise records armed == true
//       with the new duration (re-arming overwrites) and returns true.
bool ss_power_core_timer_set(ss_power_timer_wake_t* t, uint64_t us);

// Disarm a timer-wake record.
// Pre:  none (NULL is tolerated as a no-op).
// Post: when `t` != NULL: armed == false and us == 0. Idempotent.
void ss_power_core_timer_clear(ss_power_timer_wake_t* t);

// Fill `out` with the fixed "no fuel gauge" status for boards without a
// battery-sense line.
// Pre:  none (NULL is tolerated as a no-op).
// Post: when `out` != NULL it is fully zeroed and stamped with the
//       contract-mandated no-gauge values (battery_sense_valid == false, etc.).
void ss_power_core_fill_nogauge_status(ss_power_status_t* out);
