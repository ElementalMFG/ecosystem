// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
#pragma once
#include "ss_hal_types.h"
#include "esp_err.h"

typedef struct {
    uint32_t freq_hz;             // e.g. 915000000
    int8_t   tx_power_dbm;
    uint8_t  spreading_factor;    // 5..12
    uint8_t  bandwidth_khz;       // 125, 250, 500
    uint8_t  coding_rate;         // 5..8 (4/5..4/8)
    uint16_t preamble_len;
    bool     iq_inverted;
    uint32_t sync_word;
} ss_lora_cfg_t;

typedef void (*ss_lora_rx_cb_t)(const uint8_t* buf, size_t n,
                                int16_t rssi_dbm, int8_t snr_db,
                                void* user);

esp_err_t ss_lora_init(void);
esp_err_t ss_lora_config(const ss_lora_cfg_t* cfg);
esp_err_t ss_lora_tx(const uint8_t* buf, size_t n, uint32_t timeout_ms);
esp_err_t ss_lora_rx_start(ss_lora_rx_cb_t cb, void* user);
esp_err_t ss_lora_rx_stop(void);
esp_err_t ss_lora_sleep(bool on);

// Statistics
typedef struct {
    uint32_t tx_frames, rx_frames, tx_err, rx_err, tx_bytes, rx_bytes;
    int16_t  rssi_last_dbm; int8_t snr_last_db;
} ss_lora_stats_t;
esp_err_t ss_lora_stats(ss_lora_stats_t* out);
