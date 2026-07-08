// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
#pragma once
#include "ss_hal_types.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char ssid[33];
    char pass[65];
    bool sta_mode;
    bool ap_mode;
} ss_wifi_cfg_t;

esp_err_t ss_wifi_init(void);
esp_err_t ss_wifi_config(const ss_wifi_cfg_t* cfg);
esp_err_t ss_wifi_start(void);
esp_err_t ss_wifi_stop(void);
esp_err_t ss_wifi_sleep(bool on);

#ifdef __cplusplus
} // extern "C"
#endif
