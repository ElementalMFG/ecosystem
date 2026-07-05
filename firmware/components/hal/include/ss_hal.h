// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_hal.h — SS-SP Hardware Abstraction Layer, umbrella header.
//
// This file is the single #include that upper layers use to reach any HAL
// subsystem. Concrete board configurations (Lite / Alpha / Omega / OEM) are
// pulled in by board_config.h, which is provided by the board's port under
// firmware/boards/<board>/board_config.h and selected at build time.
//
// The HAL is intentionally minimal, capability-flagged, and portable across
// ESP32-S3 (Lite), ESP32-P4 (Alpha), and future silicon. Every function returns
// esp_err_t (or an ss-namespaced equivalent when we later go silicon-agnostic).

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "esp_err.h"

#include "board_config.h"        // provided by the board port
#include "ss_hal_caps.h"         // capability flags
#include "ss_hal_types.h"        // shared types (colors, pixel formats, mux modes, etc.)

#include "ss_hal_display.h"      // framebuffer, refresh, backlight
#include "ss_hal_touch.h"        // touch, gesture normalization
#include "ss_hal_buttons.h"      // hard buttons, PTT, rotary
#include "ss_hal_leds.h"         // bezel LEDs (Alpha) / single indicator (Lite)
#include "ss_hal_haptic.h"       // vibration / rumble
#include "ss_hal_audio.h"        // mic in + speaker out
#include "ss_hal_storage.h"      // SD, internal FS
#include "ss_hal_power.h"        // battery, charging, PMIC, thermals
#include "ss_hal_radio_lora.h"   // SX126x path
#include "ss_hal_radio_halow.h"  // MM8108 path (Alpha+)
#include "ss_hal_radio_ble.h"    // BLE
#include "ss_hal_radio_wifi.h"   // Wi-Fi 4/6
#include "ss_hal_gnss.h"         // GNSS receiver (Alpha+)
#include "ss_hal_imu.h"          // IMU/mag (Alpha+)
#include "ss_hal_muxctl.h"       // Lite mic-vs-radio mux; portable no-op elsewhere
#include "ss_hal_secure_elem.h"  // ATECC608 / on-chip HW crypto (variant-selected)
#include "ss_hal_rng.h"          // hardware RNG
#include "ss_hal_time.h"         // RTC + monotonic
#include "ss_hal_usb.h"          // USB-CDC, USB-serial-JTAG on dev units
#include "ss_hal_watchdog.h"     // watchdog
#include "ss_hal_ota.h"          // partition + rollback plumbing

// ============================================================================
// Lifecycle
// ============================================================================

/**
 * Bring the board up. Sequenced initialization of every enabled subsystem.
 * Order is defined by the board port; see 01_SS-SP_LITE_HARDWARE_REFERENCE.md
 * §7 for the Lite sequence.
 */
esp_err_t ss_hal_init(void);

/**
 * Shutdown / prepare for deep-sleep.
 */
esp_err_t ss_hal_shutdown(void);

/**
 * Query the compile-time board identity string (e.g. "ss-sp-lite-v1").
 */
const char* ss_hal_board_id(void);

/**
 * Runtime capability query. See ss_hal_caps.h for SS_CAP_* flags.
 */
bool ss_hal_has_cap(uint32_t cap_flag);
uint64_t ss_hal_caps_mask(void);
