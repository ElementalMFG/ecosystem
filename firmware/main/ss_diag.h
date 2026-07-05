// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_diag.h — audible diagnostics + power watchdog (directive goal D).
//
//   Buzzer:  GPIO 8, LEDC ch1/timer1 — boot/ack/error/incoming/SOS patterns.
//   Speaker: I2S1 (BCLK 13 / WS 11 / DOUT 12, mute GPIO 21) — sine test tone.
//            (The mic path is NOT touched here: it shares pins with the LoRa
//             SPI behind the GPIO 45 mux and belongs to ss_hal_muxctl.)
//   Power:   directive asked for ADC battery monitoring + safe power-down.
//            AMENDED (HW ref §12): the Lite has no battery-sense line
//            (SS_BATTERY_SENSE_PRESENT == 0, Meshtastic #7993). The watchdog
//            therefore monitors what exists — brownout + charger/USB hints —
//            and the full policy lands with an optional fuel-gauge module.

#pragma once
#include "esp_err.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SS_DIAG_TONE_BOOT = 0, // rising two-note "hello"
    SS_DIAG_TONE_ACK,      // single short mid beep
    SS_DIAG_TONE_ERROR,    // low double buzz
    SS_DIAG_TONE_INCOMING, // triple chirp (message received)
    SS_DIAG_TONE_SOS,      // ... --- ... pattern (blocking, emergency only)
} ss_diag_tone_t;

// Init buzzer LEDC + spawn the power-watchdog task. Non-blocking.
esp_err_t ss_diag_start(void);

// Play a buzzer pattern. Queued/fire-and-forget except SOS (blocking).
esp_err_t ss_diag_beep(ss_diag_tone_t tone);

// 1 kHz sine on the I2S speaker for `ms` milliseconds (audio path check).
// Blocking for the duration; scaffolding-grade (real audio = EPIC-03 hal_audio).
esp_err_t ss_diag_speaker_test(uint32_t ms);

#ifdef __cplusplus
}
#endif
