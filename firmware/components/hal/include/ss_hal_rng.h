// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
#pragma once
#include "esp_err.h"
#include <stddef.h>
#include <stdint.h>

esp_err_t ss_rng_init(void);
esp_err_t ss_rng_bytes(uint8_t* out, size_t n);
esp_err_t ss_rng_healthcheck(void);   // NIST SP 800-90B startup/continuous health
