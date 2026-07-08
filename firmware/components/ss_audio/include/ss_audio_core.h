// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_audio_core.h — pure, host-testable decision core for the mic half of the
// ss_hal_audio.h contract (ss_mic_open/read/close).
//
// This layer holds NO ESP-IDF runtime dependency: it reasons only about the
// frozen contract types (ss_hal_types.h, ss_hal_audio.h) so it can be exercised
// with a host harness. The IDF glue (ss_mic.c) turns these decisions into I2S
// std-driver calls and mux acquire/release.
//
// Scope: mic only. Speaker (ss_spk_*) and buzzer (ss_buzzer_*) are separate
// stories (S-03-010 and later) and are NOT modelled here.
//
// Format policy (Lite / INMP441-class I2S mic): the capture format is locked to
// 16000 Hz, 1 channel (mono), 16 bits/sample. Any other request is rejected by
// ss_audio_core_validate_mic_fmt so the glue never programs an unsupported clock.

#pragma once
#include "ss_hal_types.h"
#include "ss_hal_audio.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SS_AUDIO_OK = 0,         // format accepted / plan produced
    SS_AUDIO_ERR_NULL,       // a required pointer argument was NULL
    SS_AUDIO_ERR_NO_MIC_CAP, // board has no microphone capability
    SS_AUDIO_ERR_RATE,       // sample_rate_hz not supported
    SS_AUDIO_ERR_CHANNELS,   // channel count not supported
    SS_AUDIO_ERR_BITS,       // bits_per_sample not supported
} ss_audio_core_result_t;

// Locked Lite mic capture format (INMP441-class mono I2S mic).
#define SS_AUDIO_MIC_RATE_HZ 16000u
#define SS_AUDIO_MIC_CHANNELS 1u
#define SS_AUDIO_MIC_BITS 16u

// DMA buffering plan for the I2S RX channel. Field units:
//   frame_num     — samples per DMA descriptor (dma_frame_num)
//   desc_num      — number of DMA descriptors (dma_desc_num), >= 2 to double-buffer
//   bytes_per_read— natural read granularity = frame_num * (bits/8) * channels
//   max_jitter_us — worst-case time to fill one descriptor (per-read jitter bound)
typedef struct {
    uint32_t frame_num;
    uint32_t desc_num;
    uint32_t bytes_per_read;
    uint32_t max_jitter_us;
} ss_mic_dma_plan_t;

/**
 * Validate a requested mic capture format against the locked Lite policy.
 *
 * Pre:  none (fmt may be NULL).
 * Post/return (checks in this order):
 *   - SS_AUDIO_ERR_NULL       if fmt == NULL.
 *   - SS_AUDIO_ERR_NO_MIC_CAP if !has_mic_cap.
 *   - SS_AUDIO_ERR_RATE       if sample_rate_hz != 16000.
 *   - SS_AUDIO_ERR_CHANNELS   if channels != 1.
 *   - SS_AUDIO_ERR_BITS       if bits_per_sample != 16.
 *   - SS_AUDIO_OK             otherwise.
 */
ss_audio_core_result_t ss_audio_core_validate_mic_fmt(const ss_audio_fmt_t* fmt, bool has_mic_cap);

/**
 * Compute a DMA buffering plan that bounds per-read jitter to one descriptor
 * period while targeting `target_latency_ms` of buffering.
 *
 * frame_num  = sample_rate * target_latency_ms / 1000, clamped to [64, 1024].
 * desc_num   = 2 (minimum double-buffer).
 * The resulting max_jitter_us == frame_num * 1e6 / sample_rate stays <=
 * target_latency_ms * 1000 whenever the clamp does not raise frame_num above
 * the target (true for target_latency_ms >= ~4 ms at 16 kHz).
 *
 * Pre:  none (fmt/out may be NULL).
 * Post/return:
 *   - SS_AUDIO_ERR_NULL if out == NULL.
 *   - the validate result (assuming mic present) if fmt is NULL/invalid;
 *     *out is left unmodified in that case.
 *   - SS_AUDIO_OK otherwise, with *out fully populated.
 */
ss_audio_core_result_t ss_audio_core_plan_dma(const ss_audio_fmt_t* fmt, uint32_t target_latency_ms,
                                              ss_mic_dma_plan_t* out);

/**
 * The mux mode the mic path must hold while capturing.
 * Pre/post: pure; always returns SS_MUX_MODE_MIC.
 */
ss_mux_mode_t ss_audio_core_mic_mux_mode(void);

/**
 * The mux owner identity the mic path acquires/releases under.
 * Pre/post: pure; always returns SS_MUX_OWNER_AUDIO_MIC.
 */
ss_mux_owner_t ss_audio_core_mic_mux_owner(void);

#ifdef __cplusplus
} // extern "C"
#endif
