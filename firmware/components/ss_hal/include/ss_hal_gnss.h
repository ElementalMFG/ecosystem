// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
#pragma once
#include "ss_hal_types.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    ss_unix_ms_t fix_time_ms;
    double       lat_deg;
    double       lon_deg;
    float        alt_m;
    float        hdop, pdop;
    float        speed_mps;
    float        course_deg;
    uint8_t      sats_used;
    bool         has_fix;
} ss_gnss_fix_t;

esp_err_t ss_gnss_init(void);
esp_err_t ss_gnss_start(void);
esp_err_t ss_gnss_stop(void);
esp_err_t ss_gnss_get(ss_gnss_fix_t* out);
esp_err_t ss_gnss_sleep(bool on);

#ifdef __cplusplus
} // extern "C"
#endif
