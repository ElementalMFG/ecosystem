// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_display.h — component surface for the Lite ILI9488 display driver.
//
// The frozen HAL contract (ss_display_init/info/flush/backlight/sleep/
// set_orient) lives in ss_hal_display.h and is pulled in here so a single
// include reaches the whole driver. This header adds only the NON-contract
// boot self-test used by main.cpp for "display-first" boot feedback
// (directive goal A).

#pragma once
#include "ss_hal_display.h" // frozen contract: init/info/flush/backlight/sleep/set_orient
#include "esp_err.h"

// NOTE: no extern "C" guard. The frozen ss_hal_display.h contract carries none,
// so its symbols take C++ linkage in a C++ TU; the ss_display.cpp glue and its
// C++ callers (main.cpp, future ss_ui) match. Keep this header C++-linkage too.

// Boot self-test: tricolor R/G/B fills followed by a 0->100 % backlight ramp.
// Pre:  ss_display_init() has returned ESP_OK.
// Post: draws the pattern and leaves the backlight at 100 %; returns the first
//       failing esp_err_t, or ESP_OK. ESP_ERR_INVALID_STATE if not initialized.
esp_err_t ss_display_selftest(void);
