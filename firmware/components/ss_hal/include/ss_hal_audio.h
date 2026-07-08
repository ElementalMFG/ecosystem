// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
#pragma once
#include "ss_hal_types.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t sample_rate_hz;
    uint8_t  channels;          // 1=mono, 2=stereo
    uint8_t  bits_per_sample;   // 16 (default) or 32
} ss_audio_fmt_t;

// Mic input. On Lite, this implicitly requires SS_MUX_MODE_MIC.
esp_err_t ss_mic_open(const ss_audio_fmt_t* fmt);
esp_err_t ss_mic_read(void* buf, size_t bytes, size_t* out_bytes, uint32_t timeout_ms);
esp_err_t ss_mic_close(void);

// Speaker output. Independent of mux on Lite (speaker on GPIO 11/12/13).
esp_err_t ss_spk_open(const ss_audio_fmt_t* fmt);
esp_err_t ss_spk_write(const void* buf, size_t bytes);
esp_err_t ss_spk_close(void);
esp_err_t ss_spk_mute(bool on);
esp_err_t ss_spk_volume(uint8_t percent_0_100);

// Simple buzzer beep (Lite has GPIO 8 buzzer)
esp_err_t ss_buzzer_beep(uint16_t freq_hz, uint16_t duration_ms);

#ifdef __cplusplus
} // extern "C"
#endif
