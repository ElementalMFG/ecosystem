// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
#pragma once
#include "ss_hal_types.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t ss_ble_init(void);
esp_err_t ss_ble_advertise(const char* name);
esp_err_t ss_ble_stop_advertise(void);
esp_err_t ss_ble_scan_start(void);
esp_err_t ss_ble_scan_stop(void);
esp_err_t ss_ble_sleep(bool on);

#ifdef __cplusplus
} // extern "C"
#endif
