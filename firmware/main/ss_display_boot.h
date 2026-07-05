// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_display_boot.h — first-light bring-up of the ILI9488 SPI panel.
//
// Boot-critical path (directive goal A): SPI bus @ 40 MHz on the verified
// CrowPanel Advance pins (SCLK 42, MOSI 39, DC 41, CS 40, BL 38 — see
// 01_SS-SP_LITE_HARDWARE_REFERENCE.md §3.1 and Appendix §12 for why these
// differ from the external directive's template pins).
//
// This module gives the boot screen + LVGL flush hook. It will migrate behind
// ss_hal_display.h (ss_display_init/flush/backlight) as EPIC-03 lands; the
// public surface here already mirrors that contract.

#pragma once
#include "esp_err.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Initialize SPI bus, panel IO, ILI9488 init sequence, backlight PWM.
// Must be the FIRST subsystem brought up after NVS (user feedback ASAP).
esp_err_t ss_display_boot_init(void);

// Fill the whole panel with an RGB565 color (converted to the panel's
// SPI-mode RGB666 wire format internally). Used for the boot test pattern.
esp_err_t ss_display_boot_fill(uint16_t rgb565);

// Draw the tricolor boot test pattern + backlight ramp 0→100 %.
esp_err_t ss_display_boot_test_pattern(void);

// Backlight, 0..100 %. LEDC PWM on GPIO 38.
esp_err_t ss_display_boot_backlight(uint8_t percent);

// Blit an RGB565 rect (LVGL flush_cb calls this; conversion to RGB666 is
// done here so upper layers stay in RGB565 per SS_LCD_FMT).
esp_err_t ss_display_boot_blit(int16_t x, int16_t y, int16_t w, int16_t h,
                               const uint16_t* px_rgb565);

#ifdef __cplusplus
}
#endif
