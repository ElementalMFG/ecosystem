// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_hal_muxctl.h — Mux control for the shared mic/LoRa GPIO group on Lite.
//
// The CrowPanel Advance 3.5" HMI multiplexes microphone I2S (BCLK GPIO9,
// WS GPIO3, DIN GPIO10) against LoRa SX1262 SPI (SCK GPIO10, MISO GPIO9,
// MOSI GPIO3) through a single-bit selector on GPIO 45:
//   - GPIO 45 HIGH  → microphone path active   (SS_MUX_MODE_MIC)
//   - GPIO 45 LOW   → wireless/LoRa path active (SS_MUX_MODE_RADIO)
//
// The two subsystems CANNOT run simultaneously on the Lite board. Callers
// arbitrate through this API; boards without the mux (Alpha, Omega, headless)
// implement the same interface as a no-op that always succeeds.
//
// See 01_SS-SP_LITE_HARDWARE_REFERENCE.md §5 for pin-level detail.

#pragma once
#include "ss_hal_types.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"

/**
 * Acquire the mux for a given owner in the requested mode. Blocks up to
 * `timeout` ticks. Returns ESP_OK when the mux is switched and owned by
 * `owner`; ESP_ERR_TIMEOUT if another owner holds it; ESP_ERR_INVALID_STATE
 * if the board does not have a mux and the request is inconsistent.
 *
 * On boards without a mux (SS_CAP_MUX_MIC_RADIO absent), this returns ESP_OK
 * immediately.
 */
esp_err_t ss_mux_acquire(ss_mux_mode_t mode,
                         TickType_t     timeout,
                         ss_mux_owner_t owner);

/**
 * Release the mux. Must be called with the same owner value used to acquire.
 */
esp_err_t ss_mux_release(ss_mux_owner_t owner);

/**
 * Peek at the current mux mode without acquiring. For diagnostics only.
 */
ss_mux_mode_t ss_mux_current_mode(void);

/**
 * Peek at the current owner. Returns SS_MUX_OWNER_NONE if released.
 */
ss_mux_owner_t ss_mux_current_owner(void);

/**
 * Emergency force-release. USE ONLY FROM WATCHDOG / CRASH HANDLERS.
 * Bypasses ownership check. May leave a subsystem in an inconsistent state.
 */
void ss_mux_force_release(void);

/**
 * Initialize the mux control. Called by the board port during ss_hal_init.
 * Default mode on Lite is SS_MUX_MODE_RADIO.
 */
esp_err_t ss_mux_init(void);
