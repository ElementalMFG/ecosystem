// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_lora_pa_core.c — pure LoRa region PA table + EIRP-ceiling policy. See
// ss_lora_pa_core.h for scope, the pinned regulatory table, the fail-safe
// rationale (UNKNOWN == 0), and the never-exceed clamp guarantee. No ESP-IDF or
// HAL dependency lives here; all math is integer (no libm), no dynamic alloc,
// no mutable globals (the table is const).

#include "ss_lora_pa_core.h"

// Pinned regulatory table: max EIRP (dBm) + LoRaWAN RP002 channel-plan band
// edges (inclusive, Hz), indexed by ss_lora_region_t. Row 0 (UNKNOWN) is the
// fail-safe row: SAFE_MIN power and an EMPTY band (min > max) so no frequency is
// ever in-band and clamp collapses to SAFE_MIN.
static const ss_lora_pa_limits_t k_region_limits[] = {
    [SS_LORA_REGION_UNKNOWN] = {SS_LORA_PA_SAFE_MIN_DBM, 1u, 0u},
    [SS_LORA_REGION_US915] = {30, 902000000u, 928000000u},
    [SS_LORA_REGION_EU868] = {16, 863000000u, 870000000u},
    [SS_LORA_REGION_AU915] = {30, 915000000u, 928000000u},
    [SS_LORA_REGION_AS923] = {16, 915000000u, 928000000u},
};

#define SS_LORA_REGION_COUNT (sizeof(k_region_limits) / sizeof(k_region_limits[0]))

void ss_lora_region_ctx_init(ss_lora_region_ctx_t* ctx)
{
    if (ctx == NULL) { return; }
    ctx->region = SS_LORA_REGION_UNKNOWN;
    ctx->locked = false;
}

ss_lora_pa_limits_t ss_lora_pa_limits(ss_lora_region_t region)
{
    if ((uint32_t)region >= SS_LORA_REGION_COUNT) {
        return k_region_limits[SS_LORA_REGION_UNKNOWN];
    }
    return k_region_limits[region];
}

int8_t ss_lora_pa_clamp_dbm(ss_lora_region_t region, int8_t requested_dbm)
{
    // Fail-safe: any unknown/out-of-range region collapses to the safe floor.
    if (region == SS_LORA_REGION_UNKNOWN || (uint32_t)region >= SS_LORA_REGION_COUNT) {
        return SS_LORA_PA_SAFE_MIN_DBM;
    }

    int8_t limit = k_region_limits[region].max_eirp_dbm;
    // Never exceed the SX1262 conducted hardware ceiling.
    if (limit > SS_LORA_PA_HW_MAX_DBM) { limit = (int8_t)SS_LORA_PA_HW_MAX_DBM; }

    // Never exceed the (region- and hw-bounded) ceiling.
    int8_t out = (requested_dbm < limit) ? requested_dbm : limit;
    // Never below the conservative floor.
    if (out < SS_LORA_PA_SAFE_MIN_DBM) { out = (int8_t)SS_LORA_PA_SAFE_MIN_DBM; }
    return out;
}

bool ss_lora_pa_freq_in_band(ss_lora_region_t region, uint32_t freq_hz)
{
    if ((uint32_t)region >= SS_LORA_REGION_COUNT) { return false; }
    const ss_lora_pa_limits_t lim = k_region_limits[region];
    // UNKNOWN row has min (1) > max (0), so the empty band rejects every freq.
    return (freq_hz >= lim.freq_min_hz) && (freq_hz <= lim.freq_max_hz);
}

bool ss_lora_region_latch(ss_lora_region_ctx_t* ctx, ss_lora_region_t region)
{
    if (ctx == NULL) { return false; }
    if (region == SS_LORA_REGION_UNKNOWN || (uint32_t)region >= SS_LORA_REGION_COUNT) {
        return false;
    }
    if (ctx->locked) {
        // Idempotent-true only when re-latching the SAME region; a mismatched
        // re-latch is rejected and leaves the first-latched region untouched.
        return ctx->region == region;
    }
    ctx->region = region;
    ctx->locked = true;
    return true;
}

ss_lora_region_t ss_lora_region_get(const ss_lora_region_ctx_t* ctx)
{
    if (ctx == NULL) { return SS_LORA_REGION_UNKNOWN; }
    return ctx->region;
}
