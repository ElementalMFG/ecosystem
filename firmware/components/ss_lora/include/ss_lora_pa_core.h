// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_lora_pa_core.h — pure, host-testable LoRa region power-amplifier (PA) table
// and EIRP-ceiling policy core for S-03-012.
//
// SCOPE: this layer holds NO ESP-IDF and NO HAL dependency — it includes only
// <stdint.h>, <stdbool.h>, <stddef.h>. It reasons purely about regions,
// regulatory EIRP ceilings, channel-plan band edges, and a caller-supplied
// requested TX power, so it can be exercised with a plain gcc host harness under
// -fsanitize=address,undefined with no include hacks and no libm. The on-target
// SX1262 driver (S-03-011) consumes the EIRP value returned by
// ss_lora_pa_clamp_dbm() and refines it downward with the specific antenna gain
// and conducted-power limits of the assembled board; this core only enforces the
// upper regulatory envelope. The driver glue lives in ss_lora.c and is compiled
// on-target only.
//
// FAIL-SAFE DESIGN: the region enum places the error value at 0
// (SS_LORA_REGION_UNKNOWN == 0). Any NULL argument, unknown/out-of-range region,
// or zeroed/uninitialised context therefore maps to the most conservative
// outcome: ss_lora_pa_clamp_dbm() returns SS_LORA_PA_SAFE_MIN_DBM (a low, widely
// legal floor), ss_lora_pa_limits() returns a row whose band is empty
// (freq_min_hz > freq_max_hz so NO frequency is ever in-band), and
// ss_lora_pa_freq_in_band() returns false. A caller that forgets to set the
// region, or reads uninitialised state, defaults to a minimal, in-band-nowhere
// posture rather than an illegal emission.
//
// REGULATORY BASIS: the pinned table encodes regulatory MAX-EIRP ceilings and
// LoRaWAN RP002 channel-plan band edges:
//   US915  30 dBm, 902.0–928.0 MHz (FCC 47 CFR §15.247 / LoRaWAN RP002)
//   EU868  16 dBm, 863.0–870.0 MHz (ETSI EN 300 220-2, 14 dBm ERP ≈ 16 dBm EIRP)
//   AU915  30 dBm, 915.0–928.0 MHz (LoRaWAN RP002)
//   AS923  16 dBm, 915.0–928.0 MHz (ARIB STD-T108 / AS923-1, LoRaWAN RP002)
//
// EIRP-CEILING SEMANTICS: max_eirp_dbm is the effective isotropic radiated power
// the region permits. The clamp is a NEVER-EXCEED bound: the returned power is
// the minimum of the requested power, the region's regulatory max EIRP, and the
// SX1262 conducted hardware ceiling (SS_LORA_PA_HW_MAX_DBM), floored at
// SS_LORA_PA_SAFE_MIN_DBM. The clamp result can never exceed the regional max
// EIRP for a known region, and is always SS_LORA_PA_SAFE_MIN_DBM for UNKNOWN.

#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// SX1262 conducted-power ceiling (dBm). No clamp result ever exceeds this.
#define SS_LORA_PA_HW_MAX_DBM 22
// Conservative floor (dBm) returned for UNKNOWN regions and as the never-below
// bound for every clamp. Low enough to be broadly legal, high enough to link.
#define SS_LORA_PA_SAFE_MIN_DBM 2

// LoRa region. Fail-safe: UNKNOWN == 0 so any bad/uninitialised path is the most
// conservative (never-transmit / minimal-power) outcome.
typedef enum {
    SS_LORA_REGION_UNKNOWN = 0, // fail-safe: unset region -> minimal power, empty band
    SS_LORA_REGION_US915,       // FCC 47 CFR §15.247 / LoRaWAN RP002
    SS_LORA_REGION_EU868,       // ETSI EN 300 220-2
    SS_LORA_REGION_AU915,       // LoRaWAN RP002
    SS_LORA_REGION_AS923,       // ARIB STD-T108 / AS923-1
} ss_lora_region_t;

// Regulatory envelope for a region: max EIRP and channel-plan band edges.
typedef struct {
    int8_t max_eirp_dbm;  // regulatory max EIRP (dBm)
    uint32_t freq_min_hz; // inclusive lower band edge (Hz)
    uint32_t freq_max_hz; // inclusive upper band edge (Hz)
} ss_lora_pa_limits_t;

// One-shot region latch: once locked, the region is fixed for the node's life.
typedef struct {
    ss_lora_region_t region; // latched region (UNKNOWN until first latch)
    bool locked;             // true once a valid region has been latched
} ss_lora_region_ctx_t;

/**
 * Zero-initialise a region context to UNKNOWN + unlocked.
 *
 * Pre:  ctx may be NULL.
 * Post: ctx->region == SS_LORA_REGION_UNKNOWN, ctx->locked == false.
 *       No-op if ctx == NULL.
 */
void ss_lora_region_ctx_init(ss_lora_region_ctx_t* ctx);

/**
 * Regulatory envelope (max EIRP + band edges) for `region`.
 *
 * Pre:  none.
 * Post/return: the pinned table row for a known region. For
 * SS_LORA_REGION_UNKNOWN or any out-of-range value, returns the fail-safe row
 * {max_eirp_dbm = SS_LORA_PA_SAFE_MIN_DBM, freq_min_hz = 1, freq_max_hz = 0};
 * because freq_min_hz > freq_max_hz, NO frequency is ever in-band for that row.
 */
ss_lora_pa_limits_t ss_lora_pa_limits(ss_lora_region_t region);

/**
 * Clamp a requested TX power to the region's never-exceed EIRP envelope.
 *
 * Pre:  none.
 * Post/return: min(requested_dbm, region max_eirp_dbm, SS_LORA_PA_HW_MAX_DBM),
 * floored at SS_LORA_PA_SAFE_MIN_DBM. For SS_LORA_REGION_UNKNOWN or any
 * out-of-range region, returns SS_LORA_PA_SAFE_MIN_DBM. The result NEVER exceeds
 * the region's regulatory max EIRP and NEVER exceeds SS_LORA_PA_HW_MAX_DBM — this
 * is the never-exceed guarantee behind the acceptance criteria.
 */
int8_t ss_lora_pa_clamp_dbm(ss_lora_region_t region, int8_t requested_dbm);

/**
 * Whether `freq_hz` lies within the region's channel-plan band.
 *
 * Pre:  none.
 * Post/return: true iff freq_min_hz <= freq_hz <= freq_max_hz for the region's
 * table row. UNKNOWN/out-of-range regions have an empty band (min > max), so this
 * returns false for every frequency; out-of-band frequencies return false.
 */
bool ss_lora_pa_freq_in_band(ss_lora_region_t region, uint32_t freq_hz);

/**
 * One-shot region latch.
 *
 * Semantics (exact):
 *   - ctx == NULL              -> return false, no state change.
 *   - region == UNKNOWN        -> return false, no state change (never latch the
 *                                 fail-safe value).
 *   - not yet locked, valid    -> set ctx->region = region, ctx->locked = true,
 *                                 return true (the latching set).
 *   - locked, SAME region      -> return true, no state change (idempotent).
 *   - locked, DIFFERENT region -> return false, no state change (region stays as
 *                                 first latched — a mismatched re-latch is
 *                                 rejected, not silently overwritten).
 *
 * Pre:  ctx may be NULL.
 * Post: as above.
 */
bool ss_lora_region_latch(ss_lora_region_ctx_t* ctx, ss_lora_region_t region);

/**
 * Read the latched region.
 *
 * Pre:  ctx may be NULL.
 * Post/return: ctx->region, or SS_LORA_REGION_UNKNOWN if ctx == NULL.
 */
ss_lora_region_t ss_lora_region_get(const ss_lora_region_ctx_t* ctx);

#ifdef __cplusplus
} // extern "C"
#endif
