// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_gnss.h — component surface beyond the frozen ss_hal_gnss.h contract.
//
// The HAL contract (ss_gnss_init/start/stop/get/sleep) lives in ss_hal_gnss.h.
// This header adds the two extras the main-side UART engine forwards to: a raw
// NMEA sentence tap and per-channel RX counters.

#pragma once
#include <stddef.h>
#include <stdint.h>

#include "ss_hal_gnss.h" // ss_gnss_* HAL contract + ss_gnss_fix_t

#ifdef __cplusplus
extern "C" {
#endif

// Raw NMEA sentence tap. Called on the GNSS pump task; must not block. NULL to
// clear. Set before/after ss_gnss_start; the pointer is read on each sentence.
typedef void (*ss_gnss_nmea_tap_t)(const char* sentence, size_t len);
void ss_gnss_set_nmea_tap(ss_gnss_nmea_tap_t cb);

// Per-channel RX counters for the diagnostics screen.
typedef struct {
    uint32_t rx_bytes, rx_frames, rx_crc_errors, rx_overflows;
} ss_gnss_stats_t;

// Copy the current counters into *out (no-op if out is NULL).
void ss_gnss_get_stats(ss_gnss_stats_t* out);

#ifdef __cplusplus
} // extern "C"
#endif
