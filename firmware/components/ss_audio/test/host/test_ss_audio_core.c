// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// test_ss_audio_core.c — host harness for the ss_audio mic decision core.
//
// No Unity dependency: a tiny CHECK macro records failures. Built with
// -fsanitize=address,undefined so any out-of-bounds write to the plan struct
// aborts the run. Only ss_audio_core.c is linked (ss_mic.c needs the IDF).

#include <stdint.h>
#include <stdio.h>

#include "ss_audio_core.h"

static int g_checks = 0;
static int g_fails = 0;

#define CHECK(cond)                                                                                \
    do {                                                                                           \
        g_checks++;                                                                                \
        if (!(cond)) {                                                                             \
            g_fails++;                                                                             \
            printf("  FAIL: %s (line %d)\n", #cond, __LINE__);                                     \
        }                                                                                          \
    } while (0)

int main(void)
{
    // 1. Valid Lite format is accepted.
    {
        const ss_audio_fmt_t ok = {.sample_rate_hz = 16000, .channels = 1, .bits_per_sample = 16};
        CHECK(ss_audio_core_validate_mic_fmt(&ok, true) == SS_AUDIO_OK);
        printf("case 1  valid 16k/mono/16 -> OK\n");
    }

    // 2. NULL fmt is reported before any field is read.
    {
        CHECK(ss_audio_core_validate_mic_fmt(NULL, true) == SS_AUDIO_ERR_NULL);
        printf("case 2  NULL -> ERR_NULL\n");
    }

    // 3. Missing mic capability is reported before format checks.
    {
        const ss_audio_fmt_t ok = {.sample_rate_hz = 16000, .channels = 1, .bits_per_sample = 16};
        CHECK(ss_audio_core_validate_mic_fmt(&ok, false) == SS_AUDIO_ERR_NO_MIC_CAP);
        // Even a bad format still reports NO_MIC_CAP first when cap is absent.
        const ss_audio_fmt_t bad = {.sample_rate_hz = 8000, .channels = 2, .bits_per_sample = 32};
        CHECK(ss_audio_core_validate_mic_fmt(&bad, false) == SS_AUDIO_ERR_NO_MIC_CAP);
        printf("case 3  !has_mic_cap -> ERR_NO_MIC_CAP (precedes fmt checks)\n");
    }

    // 4. Each format field yields its own distinct error, checked in order.
    {
        const ss_audio_fmt_t rate = {.sample_rate_hz = 44100, .channels = 1, .bits_per_sample = 16};
        CHECK(ss_audio_core_validate_mic_fmt(&rate, true) == SS_AUDIO_ERR_RATE);
        const ss_audio_fmt_t chans = {
            .sample_rate_hz = 16000, .channels = 2, .bits_per_sample = 16};
        CHECK(ss_audio_core_validate_mic_fmt(&chans, true) == SS_AUDIO_ERR_CHANNELS);
        const ss_audio_fmt_t bits = {.sample_rate_hz = 16000, .channels = 1, .bits_per_sample = 32};
        CHECK(ss_audio_core_validate_mic_fmt(&bits, true) == SS_AUDIO_ERR_BITS);
        printf("case 4  rate/channels/bits -> distinct errors\n");
    }

    // 5. DMA plan bounds jitter within target and sizes the read correctly.
    {
        const ss_audio_fmt_t ok = {.sample_rate_hz = 16000, .channels = 1, .bits_per_sample = 16};
        ss_mic_dma_plan_t plan = {0};
        CHECK(ss_audio_core_plan_dma(&ok, 10, &plan) == SS_AUDIO_OK);
        // 16000 * 10 / 1000 = 160 samples per descriptor.
        CHECK(plan.frame_num == 160);
        CHECK(plan.desc_num >= 2);
        CHECK(plan.bytes_per_read == plan.frame_num * 2); // 16-bit mono
        // per-read jitter must stay within the 10 ms target (10 ms == 10000 us).
        CHECK(plan.max_jitter_us <= 10u * 1000u);
        CHECK(plan.max_jitter_us == plan.frame_num * 1000000u / 16000u);
        printf("case 5  plan: frame=%u desc=%u bytes=%u jitter=%uus\n", (unsigned)plan.frame_num,
               (unsigned)plan.desc_num, (unsigned)plan.bytes_per_read,
               (unsigned)plan.max_jitter_us);
    }

    // 6. Plan clamps a tiny target up to the floor and stays double-buffered.
    {
        const ss_audio_fmt_t ok = {.sample_rate_hz = 16000, .channels = 1, .bits_per_sample = 16};
        ss_mic_dma_plan_t plan = {0};
        CHECK(ss_audio_core_plan_dma(&ok, 1, &plan) == SS_AUDIO_OK);
        CHECK(plan.frame_num == 64); // clamped to floor
        CHECK(plan.desc_num >= 2);
        printf("case 6  tiny target clamps frame_num to 64\n");
    }

    // 7. Plan rejects NULL out / bad fmt.
    {
        const ss_audio_fmt_t ok = {.sample_rate_hz = 16000, .channels = 1, .bits_per_sample = 16};
        CHECK(ss_audio_core_plan_dma(&ok, 10, NULL) == SS_AUDIO_ERR_NULL);
        CHECK(ss_audio_core_plan_dma(NULL, 10, &(ss_mic_dma_plan_t){0}) == SS_AUDIO_ERR_NULL);
        const ss_audio_fmt_t rate = {.sample_rate_hz = 8000, .channels = 1, .bits_per_sample = 16};
        CHECK(ss_audio_core_plan_dma(&rate, 10, &(ss_mic_dma_plan_t){0}) == SS_AUDIO_ERR_RATE);
        printf("case 7  plan rejects NULL out / NULL fmt / bad rate\n");
    }

    // 8. Mux plan is host-asserted: mode and owner match the contract.
    {
        CHECK(ss_audio_core_mic_mux_mode() == SS_MUX_MODE_MIC);
        CHECK(ss_audio_core_mic_mux_owner() == SS_MUX_OWNER_AUDIO_MIC);
        printf("case 8  mux mode==MIC owner==AUDIO_MIC\n");
    }

    printf("\n%d checks, %d failures\n", g_checks, g_fails);
    return (g_fails == 0) ? 0 : 1;
}
