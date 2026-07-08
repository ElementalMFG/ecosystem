// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
#pragma once
#include "ss_hal_types.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*ss_touch_cb_t)(const ss_input_event_t* ev, void* user);

esp_err_t ss_touch_init(void);
esp_err_t ss_touch_register(ss_touch_cb_t cb, void* user);
esp_err_t ss_touch_poll_once(ss_input_event_t* out);   // returns ESP_ERR_NOT_FOUND if no event
esp_err_t ss_touch_sleep(bool on);

#ifdef __cplusplus
} // extern "C"
#endif
