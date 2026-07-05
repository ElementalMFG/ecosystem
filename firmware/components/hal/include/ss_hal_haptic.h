// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
#pragma once
#include "esp_err.h"
#include <stdint.h>

typedef enum {
    SS_HAPTIC_TICK,
    SS_HAPTIC_ACK,
    SS_HAPTIC_ERROR,
    SS_HAPTIC_INCOMING_MSG,
    SS_HAPTIC_INCOMING_ALERT,
    SS_HAPTIC_SOS,
    SS_HAPTIC_DIRECTION_TICK,   // for Seekie compass step
    SS_HAPTIC_LONG_PRESS_HINT,
} ss_haptic_pattern_t;

esp_err_t ss_haptic_init(void);
esp_err_t ss_haptic_play(ss_haptic_pattern_t p);
esp_err_t ss_haptic_custom(const uint8_t* env, size_t n, uint16_t period_ms);
esp_err_t ss_haptic_stop(void);
