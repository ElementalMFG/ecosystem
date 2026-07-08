// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// test_ss_lora_pa_core.c — host harness for the ss_lora region PA / EIRP core.
//
// No Unity dependency: a tiny CHECK macro records failures. Built with
// -fsanitize=address,undefined. Only ss_lora_pa_core.c is linked (ss_lora.c
// needs the IDF). The centerpiece is the TX-power sweep (case 1): for every
// region we sweep the requested power and assert the clamp NEVER exceeds that
// region's regulatory max EIRP — the acceptance criterion.

#include <stdint.h>
#include <stdio.h>

#include "ss_lora_pa_core.h"

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
    // Known regions to sweep (UNKNOWN handled separately as a fail-safe case).
    const ss_lora_region_t regions[] = {SS_LORA_REGION_US915, SS_LORA_REGION_EU868,
                                        SS_LORA_REGION_AU915, SS_LORA_REGION_AS923};
    const size_t n_regions = sizeof(regions) / sizeof(regions[0]);

    // 1. TX-POWER SWEEP: clamp NEVER exceeds the regional max EIRP nor the HW
    //    ceiling, and NEVER drops below the safe floor, for any requested power.
    {
        int sweep_points = 0;
        for (size_t r = 0; r < n_regions; r++) {
            const ss_lora_region_t region = regions[r];
            const int8_t reg_max = ss_lora_pa_limits(region).max_eirp_dbm;
            for (int req = -10; req <= 40; req++) {
                const int8_t got = ss_lora_pa_clamp_dbm(region, (int8_t)req);
                CHECK(got <= reg_max);                 // never exceed regional EIRP
                CHECK(got <= SS_LORA_PA_HW_MAX_DBM);   // never exceed HW ceiling
                CHECK(got >= SS_LORA_PA_SAFE_MIN_DBM); // never below safe floor
                sweep_points++;
            }
        }
        printf("case 1  TX-power sweep: %d clamp points, all <= regional EIRP "
               "AND <= %d dBm HW AND >= %d dBm floor\n",
               sweep_points, SS_LORA_PA_HW_MAX_DBM, SS_LORA_PA_SAFE_MIN_DBM);
    }

    // 2. Clamp exact values: HW ceiling (22) bites the 30 dBm regions; EIRP bites
    //    the 16 dBm regions; low requests floor at SAFE_MIN.
    {
        // US915/AU915 have 30 dBm regulatory but HW caps at 22.
        CHECK(ss_lora_pa_clamp_dbm(SS_LORA_REGION_US915, 40) == SS_LORA_PA_HW_MAX_DBM);
        CHECK(ss_lora_pa_clamp_dbm(SS_LORA_REGION_AU915, 30) == SS_LORA_PA_HW_MAX_DBM);
        CHECK(ss_lora_pa_clamp_dbm(SS_LORA_REGION_US915, 14) == 14);
        // EU868/AS923 regulatory 16 dBm < HW 22, so 16 is the ceiling.
        CHECK(ss_lora_pa_clamp_dbm(SS_LORA_REGION_EU868, 40) == 16);
        CHECK(ss_lora_pa_clamp_dbm(SS_LORA_REGION_AS923, 20) == 16);
        CHECK(ss_lora_pa_clamp_dbm(SS_LORA_REGION_EU868, 10) == 10);
        // Below-floor requests floor at SAFE_MIN for every known region.
        CHECK(ss_lora_pa_clamp_dbm(SS_LORA_REGION_US915, -10) == SS_LORA_PA_SAFE_MIN_DBM);
        CHECK(ss_lora_pa_clamp_dbm(SS_LORA_REGION_EU868, 0) == SS_LORA_PA_SAFE_MIN_DBM);
        printf("case 2  clamp exact: HW cap on 30 dBm regions, EIRP cap on 16 dBm, "
               "floor on low requests\n");
    }

    // 3. Fail-safe: UNKNOWN region clamps to SAFE_MIN regardless of request.
    {
        CHECK(ss_lora_pa_clamp_dbm(SS_LORA_REGION_UNKNOWN, 40) == SS_LORA_PA_SAFE_MIN_DBM);
        CHECK(ss_lora_pa_clamp_dbm(SS_LORA_REGION_UNKNOWN, 0) == SS_LORA_PA_SAFE_MIN_DBM);
        CHECK(ss_lora_pa_clamp_dbm(SS_LORA_REGION_UNKNOWN, -50) == SS_LORA_PA_SAFE_MIN_DBM);
        // Out-of-range enum value also collapses to the safe floor.
        CHECK(ss_lora_pa_clamp_dbm((ss_lora_region_t)99, 40) == SS_LORA_PA_SAFE_MIN_DBM);
        CHECK(SS_LORA_REGION_UNKNOWN == 0); // fail-safe invariant
        printf("case 3  fail-safe clamp: UNKNOWN/out-of-range -> SAFE_MIN (=%d dBm); "
               "UNKNOWN==0\n",
               SS_LORA_PA_SAFE_MIN_DBM);
    }

    // 4. limits table rows match the pinned regulatory table exactly.
    {
        const ss_lora_pa_limits_t us = ss_lora_pa_limits(SS_LORA_REGION_US915);
        CHECK(us.max_eirp_dbm == 30 && us.freq_min_hz == 902000000u &&
              us.freq_max_hz == 928000000u);
        const ss_lora_pa_limits_t eu = ss_lora_pa_limits(SS_LORA_REGION_EU868);
        CHECK(eu.max_eirp_dbm == 16 && eu.freq_min_hz == 863000000u &&
              eu.freq_max_hz == 870000000u);
        const ss_lora_pa_limits_t au = ss_lora_pa_limits(SS_LORA_REGION_AU915);
        CHECK(au.max_eirp_dbm == 30 && au.freq_min_hz == 915000000u &&
              au.freq_max_hz == 928000000u);
        const ss_lora_pa_limits_t as = ss_lora_pa_limits(SS_LORA_REGION_AS923);
        CHECK(as.max_eirp_dbm == 16 && as.freq_min_hz == 915000000u &&
              as.freq_max_hz == 928000000u);
        printf("case 4  limits rows match pinned table (US/EU/AU/AS)\n");
    }

    // 5. Fail-safe limits row for UNKNOWN/out-of-range: empty band rejects all.
    {
        const ss_lora_pa_limits_t unk = ss_lora_pa_limits(SS_LORA_REGION_UNKNOWN);
        CHECK(unk.max_eirp_dbm == SS_LORA_PA_SAFE_MIN_DBM);
        CHECK(unk.freq_min_hz == 1u && unk.freq_max_hz == 0u); // min > max => empty
        CHECK(unk.freq_min_hz > unk.freq_max_hz);
        const ss_lora_pa_limits_t oor = ss_lora_pa_limits((ss_lora_region_t)99);
        CHECK(oor.freq_min_hz > oor.freq_max_hz); // out-of-range folds to fail-safe row
        // No frequency is ever in-band for the fail-safe row.
        CHECK(!ss_lora_pa_freq_in_band(SS_LORA_REGION_UNKNOWN, 0u));
        CHECK(!ss_lora_pa_freq_in_band(SS_LORA_REGION_UNKNOWN, 915000000u));
        CHECK(!ss_lora_pa_freq_in_band(SS_LORA_REGION_UNKNOWN, 0xFFFFFFFFu));
        CHECK(!ss_lora_pa_freq_in_band((ss_lora_region_t)99, 915000000u));
        printf("case 5  fail-safe limits row: empty band (min>max) rejects all freqs\n");
    }

    // 6. freq_in_band per region: in-band true, band edges true, just-outside false.
    {
        // US915 902.0–928.0 MHz
        CHECK(ss_lora_pa_freq_in_band(SS_LORA_REGION_US915, 915000000u));
        CHECK(ss_lora_pa_freq_in_band(SS_LORA_REGION_US915, 902000000u));  // lower edge
        CHECK(ss_lora_pa_freq_in_band(SS_LORA_REGION_US915, 928000000u));  // upper edge
        CHECK(!ss_lora_pa_freq_in_band(SS_LORA_REGION_US915, 901999999u)); // just below
        CHECK(!ss_lora_pa_freq_in_band(SS_LORA_REGION_US915, 928000001u)); // just above
        // EU868 863.0–870.0 MHz
        CHECK(ss_lora_pa_freq_in_band(SS_LORA_REGION_EU868, 868000000u));
        CHECK(ss_lora_pa_freq_in_band(SS_LORA_REGION_EU868, 863000000u));
        CHECK(ss_lora_pa_freq_in_band(SS_LORA_REGION_EU868, 870000000u));
        CHECK(!ss_lora_pa_freq_in_band(SS_LORA_REGION_EU868, 862999999u));
        CHECK(!ss_lora_pa_freq_in_band(SS_LORA_REGION_EU868, 870000001u));
        CHECK(!ss_lora_pa_freq_in_band(SS_LORA_REGION_EU868, 915000000u)); // US band, not EU
        // AU915 915.0–928.0 MHz
        CHECK(ss_lora_pa_freq_in_band(SS_LORA_REGION_AU915, 915000000u));
        CHECK(ss_lora_pa_freq_in_band(SS_LORA_REGION_AU915, 928000000u));
        CHECK(!ss_lora_pa_freq_in_band(SS_LORA_REGION_AU915, 914999999u));
        CHECK(!ss_lora_pa_freq_in_band(SS_LORA_REGION_AU915, 902000000u)); // below AU lower
        // AS923 915.0–928.0 MHz
        CHECK(ss_lora_pa_freq_in_band(SS_LORA_REGION_AS923, 923000000u));
        CHECK(ss_lora_pa_freq_in_band(SS_LORA_REGION_AS923, 915000000u));
        CHECK(ss_lora_pa_freq_in_band(SS_LORA_REGION_AS923, 928000000u));
        CHECK(!ss_lora_pa_freq_in_band(SS_LORA_REGION_AS923, 914999999u));
        CHECK(!ss_lora_pa_freq_in_band(SS_LORA_REGION_AS923, 928000001u));
        printf("case 6  freq_in_band per region: in-band/edges true, just-outside false\n");
    }

    // 7. Region latch: init => UNKNOWN; first latch locks; re-latch same OK;
    //    different region while locked FAILS and leaves region unchanged.
    {
        ss_lora_region_ctx_t ctx;
        ss_lora_region_ctx_init(&ctx);
        CHECK(ss_lora_region_get(&ctx) == SS_LORA_REGION_UNKNOWN);
        CHECK(!ctx.locked);
        // First latch succeeds and locks.
        CHECK(ss_lora_region_latch(&ctx, SS_LORA_REGION_EU868));
        CHECK(ctx.locked);
        CHECK(ss_lora_region_get(&ctx) == SS_LORA_REGION_EU868);
        // Re-latch the SAME region: idempotent-true, no change.
        CHECK(ss_lora_region_latch(&ctx, SS_LORA_REGION_EU868));
        CHECK(ss_lora_region_get(&ctx) == SS_LORA_REGION_EU868);
        // Re-latch a DIFFERENT region while locked: FAILS, region unchanged.
        CHECK(!ss_lora_region_latch(&ctx, SS_LORA_REGION_US915));
        CHECK(ss_lora_region_get(&ctx) == SS_LORA_REGION_EU868);
        printf("case 7  latch: init UNKNOWN, first locks, same OK, different rejected\n");
    }

    // 8. Latch rejects UNKNOWN and is NULL-safe.
    {
        ss_lora_region_ctx_t ctx;
        ss_lora_region_ctx_init(&ctx);
        // Latching UNKNOWN is rejected and never locks.
        CHECK(!ss_lora_region_latch(&ctx, SS_LORA_REGION_UNKNOWN));
        CHECK(!ctx.locked);
        CHECK(ss_lora_region_get(&ctx) == SS_LORA_REGION_UNKNOWN);
        // Out-of-range region value is rejected too.
        CHECK(!ss_lora_region_latch(&ctx, (ss_lora_region_t)99));
        CHECK(!ctx.locked);
        // NULL args are safe.
        CHECK(!ss_lora_region_latch(NULL, SS_LORA_REGION_US915));
        CHECK(ss_lora_region_get(NULL) == SS_LORA_REGION_UNKNOWN);
        ss_lora_region_ctx_init(NULL); // no crash under ASan
        printf("case 8  latch rejects UNKNOWN/out-of-range; NULL-safe\n");
    }

    printf("\n%d checks, %d failures\n", g_checks, g_fails);
    return (g_fails != 0) ? 1 : 0;
}
