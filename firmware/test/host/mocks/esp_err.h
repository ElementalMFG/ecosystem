// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// Minimal host mock of ESP-IDF's <esp_err.h> so pure cores that include
// contract headers (which pull in esp_err.h) can compile on the host toolchain.

#pragma once

typedef int esp_err_t;

#define ESP_OK 0
#define ESP_FAIL -1
#define ESP_ERR_INVALID_ARG 0x102
#define ESP_ERR_NO_MEM 0x101
