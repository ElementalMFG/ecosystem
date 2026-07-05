// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
#pragma once
#include "ss_hal_types.h"
#include "esp_err.h"

typedef struct { uint8_t r, g, b; } ss_led_rgb_t;

esp_err_t ss_leds_init(void);
size_t    ss_leds_count(void);
esp_err_t ss_leds_set(size_t idx, ss_led_rgb_t c);
esp_err_t ss_leds_fill(ss_led_rgb_t c);
esp_err_t ss_leds_flush(void);
esp_err_t ss_leds_brightness(uint8_t percent_0_100);
esp_err_t ss_leds_sleep(bool on);
