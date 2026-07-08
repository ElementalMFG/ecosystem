// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
#pragma once
#include "ss_hal_types.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

ss_mono_us_t ss_time_mono_us(void);
esp_err_t    ss_time_get_unix_ms(ss_unix_ms_t* out);
esp_err_t    ss_time_set_unix_ms(ss_unix_ms_t v);
esp_err_t    ss_time_rtc_init(void);

#ifdef __cplusplus
} // extern "C"
#endif
