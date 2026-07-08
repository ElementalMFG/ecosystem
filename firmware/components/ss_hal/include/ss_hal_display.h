// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
#pragma once
#include "ss_hal_types.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t     w_px, h_px;         // native panel resolution
    uint16_t     dpi;                // approx dpi
    ss_shape_t   shape;
    ss_orient_t  native_orient;
    ss_pixfmt_t  fmt;
    bool         has_backlight_pwm;
} ss_display_info_t;

esp_err_t ss_display_init(void);
esp_err_t ss_display_info(ss_display_info_t* out);
esp_err_t ss_display_flush(const ss_rect_t* r, const void* px);
esp_err_t ss_display_backlight(uint8_t percent_0_100);
esp_err_t ss_display_sleep(bool on);
esp_err_t ss_display_set_orient(ss_orient_t o);

#ifdef __cplusplus
} // extern "C"
#endif
