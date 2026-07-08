// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_power_lite.h — Lite-board convenience wiring for the ss_power HAL.
//
// This is NOT part of the portable HAL ABI (ss_hal_power.h is frozen and
// board-agnostic). It is a component-local helper that arms the canonical Lite
// GPIO wake set (C-01 §4.3 HAL map) via the portable ss_power_wake_source_add()
// entry points, so callers do not have to repeat board pin/polarity knowledge.

#pragma once
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// Register the canonical Lite GPIO wake set on the ss_power wake table:
//   - Touch INT (SS_TOUCH_PIN_INT, GPIO47), level 0 (LOW-active): the GT911
//     INT line idles HIGH and asserts LOW on a touch event, so we wake on the
//     low level. GPIO47 is NOT RTC-capable on the ESP32-S3 (deep-wake range is
//     0..21), so this source is honoured for LIGHT sleep only — it is filtered
//     out of the deep-sleep ext0/ext1 set by
//     ss_power_core_wake_is_deep_capable(). Light-only here is intended, not a
//     defect (C-01 §4.3).
//   - LoRa DIO1 (SS_LORA_PIN_DIO1, GPIO1), level 1 (HIGH-active): the SX1262
//     DIO1 IRQ line asserts HIGH on a radio event, so we wake on the high
//     level. GPIO1 is RTC-capable, so this source works for both LIGHT and
//     DEEP sleep.
// The RTC-timer wake (NF-PWR-01 periodic duty cycle) is NOT armed here: it is
// owned by the duty-cycle owner and armed separately via
// ss_power_wake_timer_set() / cleared via ss_power_wake_timer_clear().
//
// Intended to be called once, on a fresh wake table, after ss_power_init().
// It does not itself track duplicates: a second call re-attempts the adds and
// surfaces ss_power_wake_source_add()'s duplicate-GPIO rejection
// (ESP_ERR_INVALID_ARG), so callers should arm the defaults exactly once.
//
// Pre:  ss_power_init() has been called; the two canonical GPIOs are not yet in
//       the wake table.
// Post: returns ESP_OK when both canonical sources are registered; on failure
//       returns the first non-ESP_OK ss_power_wake_source_add() result (e.g.
//       ESP_ERR_NO_MEM when the table is full, ESP_ERR_INVALID_ARG on a
//       duplicate) without attempting the remaining source. Any source added
//       before the failure stays in the table.
esp_err_t ss_power_wake_lite_defaults(void);

#ifdef __cplusplus
} // extern "C"
#endif
