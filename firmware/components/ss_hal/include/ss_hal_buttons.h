// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
#pragma once
#include "ss_hal_types.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*ss_btn_cb_t)(const ss_input_event_t* ev, void* user);

esp_err_t ss_buttons_init(void);
esp_err_t ss_buttons_register(ss_btn_cb_t cb, void* user);
esp_err_t ss_buttons_poll_once(ss_input_event_t* out);

#ifdef __cplusplus
} // extern "C"
#endif
