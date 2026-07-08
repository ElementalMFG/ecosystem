// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_spk_core.h — pure, host-testable decision core for the speaker half of the
// ss_hal_audio.h contract (ss_spk_open/write/close/mute/volume).
//
// This layer holds NO ESP-IDF runtime dependency: it reasons only about the
// frozen contract types (ss_hal_types.h, ss_hal_audio.h) so it can be exercised
// with a host harness. The IDF glue (ss_spk.c) turns these decisions into I2S
// std-driver calls and class-D amp enable toggles.
//
// Scope: speaker only. The mic core (ss_audio_core.*) is a separate story
// (S-03-009) and is NOT modelled here.
//
// AMP: the Lite class-D amp (MAX98357A-class) has an active-high SD/enable pin
// wired to SS_SPK_PIN_MUTE. It has no gain register, so volume is applied in
// software as a Q15 multiply on the PCM stream (ss_spk_core_apply_gain).
//
// POP-FREE ORDER: an amp that is enabled while its I2S input floats (or is
// disabled after the clocks stop) thumps the driver. The open/close step
// sequences therefore bring the I2S clocks up BEFORE enabling the amp and shut
// the amp down BEFORE stopping the clocks, with a settle delay in between.
//
// MUX: the speaker takes NO mux (it is mux-independent per ss_hal_audio.h). That
// is deliberate — it leaves the sidetone path open while the mic holds the mux
// during capture.
//
// Format policy (Lite / MAX98357A-class I2S amp): the playback format is locked
// to 16000 Hz, 1 channel (mono), 16 bits/sample. Any other request is rejected
// by ss_spk_core_validate_fmt so the glue never programs an unsupported clock.

#pragma once
#include "ss_hal_types.h"
#include "ss_hal_audio.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SS_SPK_OK = 0,         // format accepted / plan produced
    SS_SPK_ERR_NULL,       // a required pointer argument was NULL
    SS_SPK_ERR_NO_SPK_CAP, // board has no speaker capability
    SS_SPK_ERR_RATE,       // sample_rate_hz not supported
    SS_SPK_ERR_CHANNELS,   // channel count not supported
    SS_SPK_ERR_BITS,       // bits_per_sample not supported
} ss_spk_core_result_t;

// Locked Lite playback format (MAX98357A-class mono I2S amp).
#define SS_SPK_CORE_RATE_HZ 16000u
#define SS_SPK_CORE_CHANNELS 1u
#define SS_SPK_CORE_BITS 16u

// Amp settle time between the I2S clocks starting and the amp being enabled (and
// symmetrically on close). 5 ms lets the amp's charge pump stabilise pop-free.
#define SS_SPK_CORE_SETTLE_US 5000u

// DMA buffering plan for the I2S TX channel. Field units:
//   frame_num      — samples per DMA descriptor (dma_frame_num)
//   desc_num       — number of DMA descriptors (dma_desc_num), >= 2 double-buffer
//   bytes_per_write— natural write granularity = frame_num * (bits/8) * channels
//   max_latency_us — worst-case time to drain one descriptor (per-write latency)
typedef struct {
    uint32_t frame_num;
    uint32_t desc_num;
    uint32_t bytes_per_write;
    uint32_t max_latency_us;
} ss_spk_dma_plan_t;

// Ordered pop-free amp/I2S steps the glue executes on open/close.
typedef enum {
    SS_SPK_STEP_AMP_DISABLE = 0, // drive SD/mute pin low (amp shutdown)
    SS_SPK_STEP_I2S_ENABLE,      // start I2S clocks
    SS_SPK_STEP_SETTLE,          // settle delay (ss_spk_core_settle_us)
    SS_SPK_STEP_AMP_ENABLE,      // drive SD/mute pin high (amp active)
    SS_SPK_STEP_I2S_DISABLE,     // stop + delete I2S channel
} ss_spk_step_t;

/**
 * Validate a requested speaker playback format against the locked Lite policy.
 *
 * Pre:  none (fmt may be NULL).
 * Post/return (checks in this order):
 *   - SS_SPK_ERR_NULL       if fmt == NULL.
 *   - SS_SPK_ERR_NO_SPK_CAP if !has_spk_cap.
 *   - SS_SPK_ERR_RATE       if sample_rate_hz != 16000.
 *   - SS_SPK_ERR_CHANNELS   if channels != 1.
 *   - SS_SPK_ERR_BITS       if bits_per_sample != 16.
 *   - SS_SPK_OK             otherwise.
 */
ss_spk_core_result_t ss_spk_core_validate_fmt(const ss_audio_fmt_t* fmt, bool has_spk_cap);

/**
 * Compute a DMA buffering plan that bounds per-write latency to one descriptor
 * period while targeting `target_latency_ms` of buffering.
 *
 * frame_num  = sample_rate * target_latency_ms / 1000, clamped to [64, 1024].
 * desc_num   = 2 (minimum double-buffer).
 * The resulting max_latency_us == frame_num * 1e6 / sample_rate stays <=
 * target_latency_ms * 1000 whenever the clamp does not raise frame_num above
 * the target (true for target_latency_ms >= ~4 ms at 16 kHz).
 *
 * Pre:  none (fmt/out may be NULL).
 * Post/return:
 *   - SS_SPK_ERR_NULL if out == NULL.
 *   - the validate result (assuming speaker present) if fmt is NULL/invalid;
 *     *out is left unmodified in that case.
 *   - SS_SPK_OK otherwise, with *out fully populated.
 */
ss_spk_core_result_t ss_spk_core_plan_dma(const ss_audio_fmt_t* fmt, uint32_t target_latency_ms,
                                          ss_spk_dma_plan_t* out);

/**
 * Write the pop-free OPEN step order and return the step count.
 *
 * Pre:  none (out may be NULL).
 * Post/return:
 *   - with a mute GPIO: {I2S_ENABLE, SETTLE, AMP_ENABLE} (3) — clocks before amp.
 *   - without a mute GPIO: {I2S_ENABLE} (1).
 *   - 0 if out == NULL or cap < the required count (*out left unmodified).
 */
size_t ss_spk_core_open_seq(bool has_mute_gpio, ss_spk_step_t* out, size_t cap);

/**
 * Write the pop-free CLOSE step order and return the step count.
 *
 * Pre:  none (out may be NULL).
 * Post/return:
 *   - with a mute GPIO: {AMP_DISABLE, SETTLE, I2S_DISABLE} (3) — amp before clocks.
 *   - without a mute GPIO: {I2S_DISABLE} (1).
 *   - 0 if out == NULL or cap < the required count (*out left unmodified).
 */
size_t ss_spk_core_close_seq(bool has_mute_gpio, ss_spk_step_t* out, size_t cap);

/**
 * The amp settle delay used between the SETTLE steps of the sequences.
 * Pre/post: pure; always returns SS_SPK_CORE_SETTLE_US.
 */
uint32_t ss_spk_core_settle_us(void);

/**
 * Convert a 0..100 volume percentage into a Q15 gain multiplier.
 *
 * Pre:  none (percent > 100 is clamped to 100).
 * Post/return: (int16_t)(pct * 32767 / 100) — 0 -> 0, 100 -> 32767.
 */
int16_t ss_spk_core_gain_q15(uint8_t percent_0_100);

/**
 * Apply a Q15 gain to one 16-bit PCM sample.
 *
 * Pre:  |gain_q15| <= 32767 (as produced by ss_spk_core_gain_q15).
 * Post/return: (int16_t)(((int32_t)sample * gain_q15) >> 15). Cannot overflow
 * int16 because the gain magnitude never exceeds unity.
 */
int16_t ss_spk_core_apply_gain(int16_t sample, int16_t gain_q15);

#ifdef __cplusplus
} // extern "C"
#endif
