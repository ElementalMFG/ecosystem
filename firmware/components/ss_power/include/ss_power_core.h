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

// Decide the platform action for a transition from `current` to `target`.
// Pre:  none (arguments are validated).
// Post: returns SS_PWR_ACTION_INVALID iff either argument is outside the five
//       valid ss_power_state_t values; SS_PWR_ACTION_NONE when target == ON;
//       otherwise the sleep action matching `target`.
ss_power_action_t ss_power_core_decide(ss_power_state_t current, ss_power_state_t target);

// Append a wake source (gpio, level) to `t`.
// Pre:  none (arguments are validated).
// Post: returns false and leaves `t` unchanged when `t` is NULL, `level` is not
//       0/1, `gpio` is negative, the table is full, or `gpio` duplicates an
//       existing entry; otherwise appends the entry, increments count, returns
//       true.
bool ss_power_core_wake_add(ss_power_wake_table_t* t, int gpio, int level);

// Fill `out` with the fixed "no fuel gauge" status for boards without a
// battery-sense line.
// Pre:  none (NULL is tolerated as a no-op).
// Post: when `out` != NULL it is fully zeroed and stamped with the
//       contract-mandated no-gauge values (battery_sense_valid == false, etc.).
void ss_power_core_fill_nogauge_status(ss_power_status_t* out);
