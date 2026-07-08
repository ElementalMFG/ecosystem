// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
#pragma once
#include "ss_hal_types.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SS_PWR_STATE_ON,
    SS_PWR_STATE_LIGHT_SLEEP,
    SS_PWR_STATE_DEEP_SLEEP,
    SS_PWR_STATE_HIBERNATE,
    SS_PWR_STATE_SHUTDOWN,
} ss_power_state_t;

typedef enum {
    SS_CHG_UNKNOWN,
    SS_CHG_ABSENT,
    SS_CHG_TRICKLE,
    SS_CHG_CC,
    SS_CHG_CV,
    SS_CHG_DONE,
    SS_CHG_FAULT,
} ss_charge_state_t;

typedef struct {
    uint16_t          v_mv;              // battery voltage mV (Lite: N/A → 0)
    int16_t           i_ma;              // current mA (positive = charging)
    uint8_t           soc_percent;       // 0..100
    int16_t           temp_c_x10;        // °C * 10
    ss_charge_state_t charge_state;
    bool              on_external_power;
    bool              usb_present;
    bool              battery_sense_valid;
} ss_power_status_t;

// Concurrency: the ss_power API is not thread-safe; call it from the single
// power-owner task.
esp_err_t ss_power_init(void);
esp_err_t ss_power_status(ss_power_status_t* out);
esp_err_t ss_power_enter(ss_power_state_t s);
esp_err_t ss_power_wake_source_add(int gpio, int level);

// ---- Timer / periodic wake (S-03-030, NF-PWR-01) ---------------------------

// Upper bound accepted by ss_power_wake_timer_set(). Anything above this is
// near-certainly a unit-arithmetic mistake (seconds or milliseconds multiplied
// into the microsecond argument), not a real duty cycle: NF-PWR-01 periodic
// wakes are seconds to minutes. Raising the cap is a backward-compatible
// contract change; lowering it is not.
#define SS_PWR_WAKE_TIMER_MAX_US (30ULL * 24ULL * 60ULL * 60ULL * 1000000ULL) // 30 days

// Arm (or re-arm) the RTC-timer wake source with a duration of `us`
// microseconds counted from each subsequent sleep entry — NOT from this call.
// Semantics, light vs deep:
//   - Applies to LIGHT_SLEEP, DEEP_SLEEP and HIBERNATE entered via
//     ss_power_enter(); whichever armed wake source (timer or a GPIO added via
//     ss_power_wake_source_add) fires first wakes the device.
//   - Sticky: stays armed across light-sleep wake cycles until
//     ss_power_wake_timer_clear(); every sleep entry restarts the full `us`
//     countdown. Light-sleep wake resumes execution after ss_power_enter();
//     deep-sleep/hibernate wake is a reboot, which resets this (and all) wake
//     configuration.
//   - SHUTDOWN disables every wake source, including an armed timer.
// Pre:  none (arguments are validated).
// Post: returns ESP_ERR_INVALID_ARG when us == 0 or
//       us > SS_PWR_WAKE_TIMER_MAX_US, leaving any prior arming unchanged;
//       otherwise the timer is armed with `us` (a prior arming is overwritten)
//       and ESP_OK is returned. Any other error is propagated from the
//       platform, leaving the prior arming unchanged.
esp_err_t ss_power_wake_timer_set(uint64_t us);

// Disarm the RTC-timer wake source. GPIO wake sources are unaffected.
// Pre:  none.
// Post: the timer is disarmed; idempotent — returns ESP_OK whether or not a
//       timer was armed. Any other error is propagated from the platform,
//       leaving the armed state unchanged.
esp_err_t ss_power_wake_timer_clear(void);

esp_err_t ss_power_reboot(void);
esp_err_t ss_power_shutdown(void);

#ifdef __cplusplus
} // extern "C"
#endif
