// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
#pragma once
#include "esp_err.h"
#include <stddef.h>

esp_err_t ss_usb_cdc_init(void);
esp_err_t ss_usb_cdc_write(const void* buf, size_t n);
esp_err_t ss_usb_cdc_read(void* buf, size_t n, size_t* out, uint32_t timeout_ms);
