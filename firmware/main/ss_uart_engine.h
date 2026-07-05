// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_uart_engine.h — concurrent dual-UART data engine (directive goal B).
//
// Two independent, non-blocking, interrupt-driven ring-buffer channels:
//
//   UART1 (pins 18/17, "UART-IN")  → BN-880 GNSS, NMEA-0183 @ 9600.
//        Event-driven line framer → minimal RMC/GGA parser → shared fix.
//   UART2 (pins 44/43, "UART-OUT") → ESP32-C6/H2 mesh coprocessor @ 115200.
//        SLIP-style framing + CRC16-CCITT; discards the S3 boot-ROM preamble
//        that appears on pin 43 at reset (HW ref §3.8).
//
// Implementation note: the ESP32-S3 UART driver services the HW FIFO from
// ISR into a driver ring buffer and posts events to a FreeRTOS queue — the
// "DMA ring buffer" role in the directive. No task ever busy-waits; the UI
// thread is never blocked by serial I/O.

#pragma once
#include "esp_err.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "ss_hal_gnss.h"   // ss_gnss_fix_t (plain struct; no HAL impl required)

#ifdef __cplusplus
extern "C" {
#endif

// Install both UART drivers + spawn the two pump tasks. Non-blocking.
// Safe to call when a module is absent: the channel idles at zero cost.
esp_err_t ss_uart_engine_start(void);

// --- GNSS channel ------------------------------------------------------------

// Copy the most recent parsed fix. Returns true if a fix struct has ever
// been populated (check .has_fix for actual GNSS lock). Thread-safe.
bool ss_uart_gnss_last_fix(ss_gnss_fix_t* out);

// Raw NMEA sentence tap (e.g. network-GPS NMEA server, S-16-022). Called on
// the GNSS pump task; must not block. NULL to clear.
typedef void (*ss_nmea_sentence_cb_t)(const char* sentence, size_t len);
void ss_uart_gnss_set_tap(ss_nmea_sentence_cb_t cb);

// --- Mesh-coprocessor channel -------------------------------------------------

// Deframed, CRC-valid payload from the coprocessor. Called on the coproc
// pump task; must not block. NULL to clear.
typedef void (*ss_coproc_frame_cb_t)(const uint8_t* payload, size_t len);
void ss_uart_coproc_set_on_frame(ss_coproc_frame_cb_t cb);

// Frame + enqueue a payload to the coprocessor (SLIP + CRC16). Non-blocking;
// returns ESP_ERR_TIMEOUT if the TX ring is full.
esp_err_t ss_uart_coproc_send(const uint8_t* payload, size_t len);

// Per-channel counters for the diagnostics screen (S-14-014 style).
typedef struct {
    uint32_t rx_bytes, rx_frames, rx_crc_errors, rx_overflows;
    uint32_t tx_bytes, tx_frames;
} ss_uart_chan_stats_t;

void ss_uart_engine_stats(ss_uart_chan_stats_t* gnss, ss_uart_chan_stats_t* coproc);

#ifdef __cplusplus
}
#endif
