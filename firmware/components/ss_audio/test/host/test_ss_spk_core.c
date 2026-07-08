// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// test_ss_spk_core.c — host harness for the ss_audio speaker decision core.
//
// No Unity dependency: a tiny CHECK macro records failures. Built with
// -fsanitize=address,undefined so any out-of-bounds write to the plan or step
// buffers aborts the run. Only ss_spk_core.c is linked (ss_spk.c needs the IDF).

#include <stdint.h>
#include <stdio.h>

#include "ss_spk_core.h"

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
        CHECK(ss_spk_core_validate_fmt(&ok, true) == SS_SPK_OK);
        printf("case 1  valid 16k/mono/16 -> OK\n");
    }

    // 2. NULL fmt is reported before any field is read.
    {
        CHECK(ss_spk_core_validate_fmt(NULL, true) == SS_SPK_ERR_NULL);
        printf("case 2  NULL -> ERR_NULL\n");
    }

    // 3. Missing speaker capability is reported before format checks.
    {
        const ss_audio_fmt_t ok = {.sample_rate_hz = 16000, .channels = 1, .bits_per_sample = 16};
        CHECK(ss_spk_core_validate_fmt(&ok, false) == SS_SPK_ERR_NO_SPK_CAP);
        // Even a bad format still reports NO_SPK_CAP first when cap is absent.
        const ss_audio_fmt_t bad = {.sample_rate_hz = 8000, .channels = 2, .bits_per_sample = 32};
        CHECK(ss_spk_core_validate_fmt(&bad, false) == SS_SPK_ERR_NO_SPK_CAP);
        printf("case 3  !has_spk_cap -> ERR_NO_SPK_CAP (precedes fmt checks)\n");
    }

    // 4. Each format field yields its own distinct error, checked in order.
    {
        const ss_audio_fmt_t rate = {.sample_rate_hz = 44100, .channels = 1, .bits_per_sample = 16};
        CHECK(ss_spk_core_validate_fmt(&rate, true) == SS_SPK_ERR_RATE);
        const ss_audio_fmt_t chans = {
            .sample_rate_hz = 16000, .channels = 2, .bits_per_sample = 16};
        CHECK(ss_spk_core_validate_fmt(&chans, true) == SS_SPK_ERR_CHANNELS);
        const ss_audio_fmt_t bits = {.sample_rate_hz = 16000, .channels = 1, .bits_per_sample = 32};
        CHECK(ss_spk_core_validate_fmt(&bits, true) == SS_SPK_ERR_BITS);
        printf("case 4  rate/channels/bits -> distinct errors\n");
    }

    // 5. DMA plan bounds latency within target and sizes the write correctly.
    {
        const ss_audio_fmt_t ok = {.sample_rate_hz = 16000, .channels = 1, .bits_per_sample = 16};
        ss_spk_dma_plan_t plan = {0};
        CHECK(ss_spk_core_plan_dma(&ok, 10, &plan) == SS_SPK_OK);
        // 16000 * 10 / 1000 = 160 samples per descriptor.
        CHECK(plan.frame_num == 160);
        CHECK(plan.desc_num >= 2);
        CHECK(plan.bytes_per_write == plan.frame_num * 2); // 16-bit mono
        // per-write latency must stay within the 10 ms target (10 ms == 10000 us).
        CHECK(plan.max_latency_us <= 10u * 1000u);
        CHECK(plan.max_latency_us == plan.frame_num * 1000000u / 16000u);
        printf("case 5  plan: frame=%u desc=%u bytes=%u latency=%uus\n", (unsigned)plan.frame_num,
               (unsigned)plan.desc_num, (unsigned)plan.bytes_per_write,
               (unsigned)plan.max_latency_us);
    }

    // 6. Plan clamps a tiny target up to the floor and stays double-buffered.
    {
        const ss_audio_fmt_t ok = {.sample_rate_hz = 16000, .channels = 1, .bits_per_sample = 16};
        ss_spk_dma_plan_t plan = {0};
        CHECK(ss_spk_core_plan_dma(&ok, 1, &plan) == SS_SPK_OK);
        CHECK(plan.frame_num == 64); // clamped to floor
        CHECK(plan.desc_num >= 2);
        printf("case 6  tiny target clamps frame_num to 64\n");
    }

    // 7. Plan rejects NULL out / bad fmt.
    {
        const ss_audio_fmt_t ok = {.sample_rate_hz = 16000, .channels = 1, .bits_per_sample = 16};
        CHECK(ss_spk_core_plan_dma(&ok, 10, NULL) == SS_SPK_ERR_NULL);
        CHECK(ss_spk_core_plan_dma(NULL, 10, &(ss_spk_dma_plan_t){0}) == SS_SPK_ERR_NULL);
        const ss_audio_fmt_t rate = {.sample_rate_hz = 8000, .channels = 1, .bits_per_sample = 16};
        CHECK(ss_spk_core_plan_dma(&rate, 10, &(ss_spk_dma_plan_t){0}) == SS_SPK_ERR_RATE);
        printf("case 7  plan rejects NULL out / NULL fmt / bad rate\n");
    }

    // 8. Pop-free OPEN sequence: clocks up before amp on.
    {
        ss_spk_step_t steps[3] = {0};
        const size_t n = ss_spk_core_open_seq(true, steps, 3);
        CHECK(n == 3);
        CHECK(steps[0] == SS_SPK_STEP_I2S_ENABLE);
        CHECK(steps[1] == SS_SPK_STEP_SETTLE);
        CHECK(steps[2] == SS_SPK_STEP_AMP_ENABLE);
        // Locate indices and assert I2S enable precedes amp enable (pop-free).
        int i2s_idx = -1, amp_idx = -1;
        for (int i = 0; i < (int)n; i++) {
            if (steps[i] == SS_SPK_STEP_I2S_ENABLE) { i2s_idx = i; }
            if (steps[i] == SS_SPK_STEP_AMP_ENABLE) { amp_idx = i; }
        }
        CHECK(i2s_idx >= 0 && amp_idx >= 0 && i2s_idx < amp_idx);

        ss_spk_step_t one[1] = {0};
        const size_t m = ss_spk_core_open_seq(false, one, 1);
        CHECK(m == 1);
        CHECK(one[0] == SS_SPK_STEP_I2S_ENABLE);

        // cap too small for the mute-gpio sequence returns 0.
        ss_spk_step_t small[1] = {0};
        CHECK(ss_spk_core_open_seq(true, small, 1) == 0);
        CHECK(ss_spk_core_open_seq(true, NULL, 3) == 0);
        printf("case 8  open_seq clocks-before-amp (pop-free)\n");
    }

    // 9. Pop-free CLOSE sequence: amp off before clocks stop.
    {
        ss_spk_step_t steps[3] = {0};
        const size_t n = ss_spk_core_close_seq(true, steps, 3);
        CHECK(n == 3);
        CHECK(steps[0] == SS_SPK_STEP_AMP_DISABLE);
        CHECK(steps[1] == SS_SPK_STEP_SETTLE);
        CHECK(steps[2] == SS_SPK_STEP_I2S_DISABLE);
        int amp_idx = -1, i2s_idx = -1;
        for (int i = 0; i < (int)n; i++) {
            if (steps[i] == SS_SPK_STEP_AMP_DISABLE) { amp_idx = i; }
            if (steps[i] == SS_SPK_STEP_I2S_DISABLE) { i2s_idx = i; }
        }
        CHECK(amp_idx >= 0 && i2s_idx >= 0 && amp_idx < i2s_idx);

        ss_spk_step_t one[1] = {0};
        const size_t m = ss_spk_core_close_seq(false, one, 1);
        CHECK(m == 1);
        CHECK(one[0] == SS_SPK_STEP_I2S_DISABLE);
        CHECK(ss_spk_core_close_seq(true, one, 1) == 0);
        CHECK(ss_spk_core_close_seq(true, NULL, 3) == 0);
        printf("case 9  close_seq amp-before-clocks (pop-free)\n");
    }

    // 10. Q15 volume: gain table and overflow-free application.
    {
        CHECK(ss_spk_core_gain_q15(0) == 0);
        CHECK(ss_spk_core_gain_q15(100) == 32767);
        CHECK(ss_spk_core_gain_q15(50) == 16383);
        CHECK(ss_spk_core_gain_q15(200) == 32767); // clamped
        CHECK(ss_spk_core_apply_gain(1000, 32767) >= 999);
        CHECK(ss_spk_core_apply_gain(1000, 0) == 0);
        CHECK(ss_spk_core_apply_gain(-32768, 32767) >= -32768); // no overflow
        printf("case 10 gain_q15 table + apply_gain no overflow\n");
    }

    printf("\n%d checks, %d failures\n", g_checks, g_fails);
    return (g_fails == 0) ? 0 : 1;
}
