// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
#pragma once
#include "esp_err.h"
#include <stdint.h>

esp_err_t ss_wdt_init(uint32_t timeout_ms);
esp_err_t ss_wdt_subscribe(void);      // current task
esp_err_t ss_wdt_unsubscribe(void);
esp_err_t ss_wdt_feed(void);
