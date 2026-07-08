// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
#pragma once
#include "esp_err.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float ax, ay, az;   // g
    float gx, gy, gz;   // dps
    float mx, my, mz;   // uT (0 if no mag)
    float temp_c;
    uint64_t at_us;
} ss_imu_sample_t;

esp_err_t ss_imu_init(void);
esp_err_t ss_imu_read(ss_imu_sample_t* out);
esp_err_t ss_imu_heading_deg(float* out_heading_deg);   // fused compass
esp_err_t ss_imu_sleep(bool on);

#ifdef __cplusplus
} // extern "C"
#endif
