// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_spk_core.c — pure speaker-format validation, DMA planning, pop-free step
// sequencing, and Q15 software volume. See ss_spk_core.h for the format policy
// and per-function contracts. No ESP-IDF runtime dependency lives here.

#include "ss_spk_core.h"

// Clamp bounds for the DMA descriptor size (samples per descriptor).
#define SS_SPK_FRAME_MIN 64u
#define SS_SPK_FRAME_MAX 1024u

ss_spk_core_result_t ss_spk_core_validate_fmt(const ss_audio_fmt_t* fmt, bool has_spk_cap)
{
    if (fmt == NULL) { return SS_SPK_ERR_NULL; }
    if (!has_spk_cap) { return SS_SPK_ERR_NO_SPK_CAP; }
    if (fmt->sample_rate_hz != SS_SPK_CORE_RATE_HZ) { return SS_SPK_ERR_RATE; }
    if (fmt->channels != SS_SPK_CORE_CHANNELS) { return SS_SPK_ERR_CHANNELS; }
    if (fmt->bits_per_sample != SS_SPK_CORE_BITS) { return SS_SPK_ERR_BITS; }
    return SS_SPK_OK;
}

ss_spk_core_result_t ss_spk_core_plan_dma(const ss_audio_fmt_t* fmt, uint32_t target_latency_ms,
                                          ss_spk_dma_plan_t* out)
{
    if (out == NULL) { return SS_SPK_ERR_NULL; }
    // The plan is only meaningful for a speaker-capable board with a valid format.
    const ss_spk_core_result_t vr = ss_spk_core_validate_fmt(fmt, true);
    if (vr != SS_SPK_OK) { return vr; }

    // frame_num = samples that drain over target_latency_ms, clamped so a single
    // descriptor is neither wastefully large nor pathologically small.
    uint64_t frames = (uint64_t)fmt->sample_rate_hz * (uint64_t)target_latency_ms / 1000u;
    if (frames < SS_SPK_FRAME_MIN) { frames = SS_SPK_FRAME_MIN; }
    if (frames > SS_SPK_FRAME_MAX) { frames = SS_SPK_FRAME_MAX; }

    out->frame_num = (uint32_t)frames;
    out->desc_num = 2u; // double-buffer minimum
    out->bytes_per_write =
        out->frame_num * (uint32_t)(fmt->bits_per_sample / 8u) * (uint32_t)fmt->channels;
    out->max_latency_us = out->frame_num * 1000000u / fmt->sample_rate_hz;
    return SS_SPK_OK;
}

size_t ss_spk_core_open_seq(bool has_mute_gpio, ss_spk_step_t* out, size_t cap)
{
    if (out == NULL) { return 0; }
    if (has_mute_gpio) {
        // Clocks up, settle, then amp on — enabling the amp last avoids the pop.
        if (cap < 3u) { return 0; }
        out[0] = SS_SPK_STEP_I2S_ENABLE;
        out[1] = SS_SPK_STEP_SETTLE;
        out[2] = SS_SPK_STEP_AMP_ENABLE;
        return 3u;
    }
    if (cap < 1u) { return 0; }
    out[0] = SS_SPK_STEP_I2S_ENABLE;
    return 1u;
}

size_t ss_spk_core_close_seq(bool has_mute_gpio, ss_spk_step_t* out, size_t cap)
{
    if (out == NULL) { return 0; }
    if (has_mute_gpio) {
        // Amp off, settle, then clocks down — muting first avoids the pop.
        if (cap < 3u) { return 0; }
        out[0] = SS_SPK_STEP_AMP_DISABLE;
        out[1] = SS_SPK_STEP_SETTLE;
        out[2] = SS_SPK_STEP_I2S_DISABLE;
        return 3u;
    }
    if (cap < 1u) { return 0; }
    out[0] = SS_SPK_STEP_I2S_DISABLE;
    return 1u;
}

uint32_t ss_spk_core_settle_us(void)
{
    return SS_SPK_CORE_SETTLE_US;
}

int16_t ss_spk_core_gain_q15(uint8_t percent_0_100)
{
    uint8_t pct = percent_0_100;
    if (pct > 100u) { pct = 100u; }
    return (int16_t)((uint32_t)pct * 32767u / 100u);
}

int16_t ss_spk_core_apply_gain(int16_t sample, int16_t gain_q15)
{
    return (int16_t)(((int32_t)sample * (int32_t)gain_q15) >> 15);
}
