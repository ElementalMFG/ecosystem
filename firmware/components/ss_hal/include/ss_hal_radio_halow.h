// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// Wi-Fi HaLow (802.11ah) via Morse Micro MM8108. Present on Alpha/Omega,
// absent on Lite. All functions return ESP_ERR_NOT_SUPPORTED if the board
// lacks SS_CAP_RADIO_HALOW.

#pragma once
#include "ss_hal_types.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t center_freq_hz;      // regional
    uint8_t  bandwidth_mhz;       // 1, 2, 4, 8, 16
    int8_t   tx_power_dbm;
    uint8_t  mcs;                 // 0..10 (MM8108 supports up to MCS 10)
    bool     mesh_enabled;        // 802.11s S-mesh
    char     ssid[33];
    uint8_t  bssid[6];
    uint8_t  region_code[4];      // e.g. "US","EU","JP","AU"
} ss_halow_cfg_t;

typedef void (*ss_halow_rx_cb_t)(const uint8_t* buf, size_t n, void* user);

esp_err_t ss_halow_init(void);
esp_err_t ss_halow_config(const ss_halow_cfg_t* cfg);
esp_err_t ss_halow_start(void);
esp_err_t ss_halow_stop(void);
esp_err_t ss_halow_tx(const uint8_t* buf, size_t n, uint32_t timeout_ms);
esp_err_t ss_halow_rx_start(ss_halow_rx_cb_t cb, void* user);

typedef struct {
    uint32_t neighbors;
    uint32_t tx_frames, rx_frames, tx_err, rx_err;
    int16_t  rssi_avg_dbm;
    uint32_t mbps_current;
} ss_halow_stats_t;
esp_err_t ss_halow_stats(ss_halow_stats_t* out);

#ifdef __cplusplus
} // extern "C"
#endif
