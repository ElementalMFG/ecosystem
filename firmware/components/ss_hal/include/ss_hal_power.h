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

esp_err_t ss_power_init(void);
esp_err_t ss_power_status(ss_power_status_t* out);
esp_err_t ss_power_enter(ss_power_state_t s);
esp_err_t ss_power_wake_source_add(int gpio, int level);
esp_err_t ss_power_reboot(void);
esp_err_t ss_power_shutdown(void);

#ifdef __cplusplus
} // extern "C"
#endif
