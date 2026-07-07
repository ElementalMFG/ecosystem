// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// board_config.h — HOST TEST MOCK — never a real board.
//
// A pure-C consumer of the HAL reaches the board identity and capability mask
// through board_config.h (pulled in by ss_hal.h). The real board port
// (firmware/boards/lite/board_config.h) references ESP-IDF peripheral handle
// macros (SPI3_HOST, I2C_NUM_0, UART_NUM_1, I2S_NUM_0, ...) that do not exist
// off-target, so host tests substitute this fake header on the include path.
//
// The real, dependency-free HAL headers (ss_hal_caps.h, ss_hal_types.h — both
// include only <stdint.h>/<stdbool.h>/<stddef.h>) are used unmodified; only
// this board header is mocked. Values below are deliberately implausible
// sentinels so nothing can mistake them for a shipping board.

#pragma once

#include "ss_hal_caps.h"  // real, host-clean: SS_CAP_* flags
#include "ss_hal_types.h" // real, host-clean: SS_ORIENT_*, SS_PIXFMT_*

// ============================================================================
// Board identity (fake)
// ============================================================================
#define SS_BOARD_ID_STR "ss-sp-host-mock"
#define SS_BOARD_ID_INT 0xFFFF
#define SS_BOARD_HW_VARIANT "host_test_mock"
#define SS_BOARD_MCU "HOST-X86-64"
#define SS_BOARD_FLASH_MB 16
#define SS_BOARD_PSRAM_MB 8

// ============================================================================
// Capability mask (see ss_hal_caps.h)
//
// Mirrors the shape of a real board_config.h: a BASE set OR-ed with a radio.
// Chosen so tests can assert both a present cap (DISPLAY) and an absent one
// (PMIC), and that BASE and RADIO are disjoint.
// ============================================================================
#define SS_BOARD_CAPS_BASE (SS_CAP_DISPLAY | SS_CAP_TOUCH | SS_CAP_HW_RNG | SS_CAP_USB_CDC)

#define SS_BOARD_CAPS_RADIO SS_CAP_RADIO_LORA

#define SS_BOARD_CAPS (SS_BOARD_CAPS_BASE | SS_BOARD_CAPS_RADIO)

// ============================================================================
// A few geometry macros a pure-C consumer might touch (fake but well-formed)
// ============================================================================
#define SS_LCD_W_PX 480
#define SS_LCD_H_PX 320
#define SS_LCD_NATIVE_ORIENT SS_ORIENT_90
#define SS_LCD_FMT SS_PIXFMT_RGB565
#define SS_BATTERY_SENSE_PRESENT 0
