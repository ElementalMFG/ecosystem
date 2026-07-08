// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_display_core.h — pure, host-testable core for the ss_display HAL driver.
//
// This layer holds NO ESP-IDF runtime dependency and reads NO board_config
// macro: every geometry/timing input is passed as an argument so the logic can
// be exercised with host gtest. The IDF glue (ss_display.cpp) supplies the
// SS_LCD_* values and turns these decisions into esp_lcd transactions.
//
// Covers the arithmetic that would otherwise be hard to prove on-target:
//   - rect clipping to the panel window,
//   - RGB565 -> RGB666 wire-byte conversion (ILI9488 4-wire SPI, COLMOD 0x66),
//   - the SPI transfer-time budget behind the 60 FPS partial-redraw AC,
//   - the MADCTL byte per display orientation.

#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "ss_hal_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Clip rect `in` to the panel bounds [0,w_px) x [0,h_px).
// Pre:  none (NULL `out` is tolerated and reported as not-visible).
// Post: returns false and (when `out` != NULL) zeroes `*out` when the rect is
//       NULL-target, empty, or falls entirely outside the panel; otherwise
//       writes the clipped rect to `*out` and returns true. `in.w`/`in.h` are
//       treated as extents (exclusive right/bottom edge = x+w / y+h).
bool ss_display_core_clip(ss_rect_t in, uint16_t w_px, uint16_t h_px, ss_rect_t* out);

// Convert one RGB565 pixel to its 3-byte ILI9488 RGB666 wire representation
// (6 significant bits per channel, left-justified in each byte).
// Pre:  `out` points to at least 3 writable bytes.
// Post: out[0]=R', out[1]=G', out[2]=B' using the exact shifts the verified
//       bring-up path used (R5<<3-aligned, G6, B5<<3-aligned).
void ss_display_core_px565_to_666(uint16_t c, uint8_t out[3]);

// Analytical SPI transfer time, in microseconds (rounded up), for a `w` x `h`
// tile at `wire_bpp` bytes/pixel over a `freq_hz` clock:
//   us = ceil( w * h * wire_bpp * 8 bits / freq_hz * 1e6 ).
// Pre:  none (freq_hz == 0 returns UINT32_MAX to denote "never fits").
// Post: returns the ceiling of the transfer time in microseconds. Used to prove
//       a partial-redraw tile fits inside a 16.67 ms (60 FPS) frame budget.
uint32_t ss_display_core_tile_xfer_us(uint16_t w, uint16_t h, uint8_t wire_bpp, uint32_t freq_hz);

// MADCTL (0x36) byte for the requested logical orientation.
// Pre:  none (an out-of-range enum falls back to the native landscape byte).
// Post: returns a distinct, BGR-tagged MADCTL byte per orientation; SS_ORIENT_90
//       is the panel-native landscape value 0xE8. 0/180 are the portrait pair
//       (MV clear); 90/270 the landscape pair (MV set); the 180-flip of each
//       pair toggles both MX and MY.
uint8_t ss_display_core_madctl(ss_orient_t o);

#ifdef __cplusplus
}
#endif
