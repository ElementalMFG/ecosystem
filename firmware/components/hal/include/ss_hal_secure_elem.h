// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
#pragma once
#include "esp_err.h"
#include <stdint.h>
#include <stddef.h>

esp_err_t ss_se_init(void);
esp_err_t ss_se_read_serial(uint8_t out[16]);
esp_err_t ss_se_sign_ed25519(const uint8_t* msg, size_t msg_len,
                             uint8_t sig[64]);
esp_err_t ss_se_x25519_derive(const uint8_t peer_pub[32],
                              uint8_t shared[32]);
esp_err_t ss_se_random(uint8_t* out, size_t n);
esp_err_t ss_se_lock(void);
