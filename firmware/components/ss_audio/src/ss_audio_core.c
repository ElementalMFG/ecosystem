// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_audio_core.c — pure mic-format validation + DMA planning. See
// ss_audio_core.h for the format policy and per-function contracts. No ESP-IDF
// runtime dependency lives here.

#include "ss_audio_core.h"

// Clamp bounds for the DMA descriptor size (samples per descriptor).
#define SS_AUDIO_FRAME_MIN 64u
#define SS_AUDIO_FRAME_MAX 1024u

ss_audio_core_result_t ss_audio_core_validate_mic_fmt(const ss_audio_fmt_t* fmt, bool has_mic_cap)
{
    if (fmt == NULL) { return SS_AUDIO_ERR_NULL; }
    if (!has_mic_cap) { return SS_AUDIO_ERR_NO_MIC_CAP; }
    if (fmt->sample_rate_hz != SS_AUDIO_MIC_RATE_HZ) { return SS_AUDIO_ERR_RATE; }
    if (fmt->channels != SS_AUDIO_MIC_CHANNELS) { return SS_AUDIO_ERR_CHANNELS; }
    if (fmt->bits_per_sample != SS_AUDIO_MIC_BITS) { return SS_AUDIO_ERR_BITS; }
    return SS_AUDIO_OK;
}

ss_audio_core_result_t ss_audio_core_plan_dma(const ss_audio_fmt_t* fmt, uint32_t target_latency_ms,
                                              ss_mic_dma_plan_t* out)
{
    if (out == NULL) { return SS_AUDIO_ERR_NULL; }
    // The plan is only meaningful for a mic-capable board with a valid format.
    const ss_audio_core_result_t vr = ss_audio_core_validate_mic_fmt(fmt, true);
    if (vr != SS_AUDIO_OK) { return vr; }

    // frame_num = samples that accumulate over target_latency_ms, clamped so a
    // single descriptor is neither wastefully large nor pathologically small.
    uint64_t frames = (uint64_t)fmt->sample_rate_hz * (uint64_t)target_latency_ms / 1000u;
    if (frames < SS_AUDIO_FRAME_MIN) { frames = SS_AUDIO_FRAME_MIN; }
    if (frames > SS_AUDIO_FRAME_MAX) { frames = SS_AUDIO_FRAME_MAX; }

    out->frame_num = (uint32_t)frames;
    out->desc_num = 2u; // double-buffer minimum
    out->bytes_per_read =
        out->frame_num * (uint32_t)(fmt->bits_per_sample / 8u) * (uint32_t)fmt->channels;
    out->max_jitter_us = out->frame_num * 1000000u / fmt->sample_rate_hz;
    return SS_AUDIO_OK;
}

ss_mux_mode_t ss_audio_core_mic_mux_mode(void)
{
    return SS_MUX_MODE_MIC;
}

ss_mux_owner_t ss_audio_core_mic_mux_owner(void)
{
    return SS_MUX_OWNER_AUDIO_MIC;
}
