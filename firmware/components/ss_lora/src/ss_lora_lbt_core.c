// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_lora_lbt_core.c — pure LBT + EU868 duty-cycle policy. See ss_lora_lbt_core.h
// for scope, the ETSI sub-band model, the fail-safe rationale (ERR == 0), the
// bounded SOS override (C-08), and the monotonic-ms wrap note. No ESP-IDF or HAL
// dependency lives here; all math is integer (no libm).

#include "ss_lora_lbt_core.h"

// EU868 sub-band table (ETSI EN 300 220). Ranges are inclusive-lo / exclusive-hi
// in Hz; `permille` is 10 = 1 %, 1 = 0.1 %, 100 = 10 %.
typedef struct {
    uint32_t lo_hz;
    uint32_t hi_hz;
    uint16_t permille;
} ss_lora_subband_t;

static const ss_lora_subband_t k_eu868_subbands[] = {
    {863000000u, 868000000u, 10u},  // 863.0–868.0 MHz : 1 %
    {868000000u, 868600000u, 10u},  // 868.0–868.6 MHz : 1 %
    {868700000u, 869200000u, 1u},   // 868.7–869.2 MHz : 0.1 %
    {869400000u, 869650000u, 100u}, // 869.4–869.65 MHz : 10 %
    {869700000u, 870000000u, 10u},  // 869.7–870.0 MHz : 1 %
};

#define SS_LORA_SUBBAND_COUNT (sizeof(k_eu868_subbands) / sizeof(k_eu868_subbands[0]))

// 1-based index of the sub-band containing `freq_hz`, or 0 if none. Used to test
// whether two frequencies share the same duty budget.
static uint32_t band_index(uint32_t freq_hz)
{
    for (uint32_t i = 0; i < SS_LORA_SUBBAND_COUNT; i++) {
        if (freq_hz >= k_eu868_subbands[i].lo_hz && freq_hz < k_eu868_subbands[i].hi_hz) {
            return i + 1u;
        }
    }
    return 0u;
}

void ss_lora_duty_init(ss_lora_duty_state_t* st)
{
    if (st == NULL) { return; }
    st->count = 0u;
    st->head = 0u;
}

uint16_t ss_lora_duty_permille(uint32_t freq_hz)
{
    const uint32_t idx = band_index(freq_hz);
    if (idx == 0u) { return 0u; }
    return k_eu868_subbands[idx - 1u].permille;
}

uint32_t ss_lora_duty_budget_ms(uint32_t freq_hz)
{
    const uint16_t permille = ss_lora_duty_permille(freq_hz);
    return (uint32_t)((uint64_t)permille * (uint64_t)SS_LORA_DUTY_WINDOW_MS / 1000u);
}

// Sum airtime in the rolling window for the sub-band of `freq_hz`. When
// `sos_only` is true, only events granted under the SOS override are counted.
static uint32_t used_ms_impl(const ss_lora_duty_state_t* st, uint32_t freq_hz, uint32_t now_ms,
                             bool sos_only)
{
    if (st == NULL) { return 0u; }
    const uint32_t want = band_index(freq_hz);
    if (want == 0u) { return 0u; }

    uint32_t sum = 0u;
    for (uint32_t i = 0; i < st->count; i++) {
        const uint32_t idx = (st->head + i) % SS_LORA_DUTY_MAX_EVENTS;
        const ss_lora_tx_event_t* ev = &st->events[idx];
        if (sos_only && !ev->sos_override) { continue; }
        if (band_index(ev->freq_hz) != want) { continue; }
        // Unsigned wrap-safe window membership (window << 2^32).
        if ((now_ms - ev->ts_ms) <= SS_LORA_DUTY_WINDOW_MS) { sum += ev->airtime_ms; }
    }
    return sum;
}

uint32_t ss_lora_duty_used_ms(const ss_lora_duty_state_t* st, uint32_t freq_hz, uint32_t now_ms)
{
    return used_ms_impl(st, freq_hz, now_ms, false);
}

void ss_lora_duty_record(ss_lora_duty_state_t* st, uint32_t freq_hz, uint32_t airtime_ms,
                         uint32_t now_ms, bool sos_override)
{
    if (st == NULL) { return; }
    const uint32_t idx = (st->head + st->count) % SS_LORA_DUTY_MAX_EVENTS;
    st->events[idx].ts_ms = now_ms;
    st->events[idx].airtime_ms = airtime_ms;
    st->events[idx].freq_hz = freq_hz;
    st->events[idx].sos_override = sos_override;
    if (st->count < SS_LORA_DUTY_MAX_EVENTS) {
        st->count++;
    } else {
        // Ring full: we just overwrote the oldest slot; advance head past it.
        st->head = (st->head + 1u) % SS_LORA_DUTY_MAX_EVENTS;
    }
}

ss_lora_tx_decision_t ss_lora_tx_evaluate(const ss_lora_duty_state_t* st,
                                          const ss_lora_tx_req_t* req, int16_t rssi_dbm,
                                          int16_t lbt_threshold_dbm, uint32_t now_ms)
{
    // (a) fail-safe: bad args or a frequency outside any EU868 sub-band.
    if (st == NULL || req == NULL) { return SS_LORA_TX_ERR; }
    if (ss_lora_duty_permille(req->freq_hz) == 0u) { return SS_LORA_TX_ERR; }

    // (b) duty budget.
    const uint32_t budget = ss_lora_duty_budget_ms(req->freq_hz);
    const uint32_t used = ss_lora_duty_used_ms(st, req->freq_hz, now_ms);
    const bool fits = ((uint64_t)used + (uint64_t)req->airtime_ms) <= (uint64_t)budget;

    ss_lora_tx_decision_t candidate;
    if (fits) {
        candidate = SS_LORA_TX_ALLOW;
    } else if (req->is_sos) {
        const uint32_t sos_used = used_ms_impl(st, req->freq_hz, now_ms, true);
        if (((uint64_t)sos_used + (uint64_t)req->airtime_ms) <= (uint64_t)SS_LORA_SOS_OVERRIDE_MAX_MS) {
            candidate = SS_LORA_TX_ALLOW_SOS_OVERRIDE;
        } else {
            return SS_LORA_TX_BLOCK_DUTY;
        }
    } else {
        return SS_LORA_TX_BLOCK_DUTY;
    }

    // (c) LBT: a busy channel defers everything, including SOS (collision avoidance).
    if (rssi_dbm >= lbt_threshold_dbm) { return SS_LORA_TX_DEFER_BUSY; }

    // (d) clear on both counts.
    return candidate;
}

uint32_t ss_lora_lbt_backoff_ms(uint32_t attempt)
{
    // Cap the shift first so BASE << attempt can never overflow, then clamp.
    if (attempt >= 32u) { return SS_LORA_LBT_MAX_BACKOFF_MS; }
    const uint32_t backoff = SS_LORA_LBT_BASE_BACKOFF_MS << attempt;
    if (backoff > SS_LORA_LBT_MAX_BACKOFF_MS || backoff < SS_LORA_LBT_BASE_BACKOFF_MS) {
        return SS_LORA_LBT_MAX_BACKOFF_MS;
    }
    return backoff;
}

bool ss_lora_lbt_should_give_up(uint32_t attempt)
{
    return attempt >= SS_LORA_LBT_MAX_RETRIES;
}

// ceil(num / denom) for signed num and positive denom (C truncates toward zero).
static int64_t ceil_div_s(int64_t num, int64_t denom)
{
    if (num >= 0) { return (num + denom - 1) / denom; }
    return -((-num) / denom);
}

uint32_t ss_lora_lbt_toa_ms(uint8_t sf, uint16_t bw_khz, uint8_t cr, uint16_t preamble_len,
                            uint8_t payload_len, bool explicit_header, bool crc_on,
                            bool low_dr_optimize)
{
    const int64_t SF = (int64_t)sf;
    const int64_t PL = (int64_t)payload_len;
    const int64_t CR = (int64_t)cr - 4; // coding rate 1..4 (from cr 5..8)
    const int64_t IH = explicit_header ? 0 : 1;
    const int64_t DE = low_dr_optimize ? 1 : 0;
    const int64_t crc = crc_on ? 1 : 0;

    const int64_t num = 8 * PL - 4 * SF + 28 + 16 * crc - 20 * IH;
    const int64_t denom = 4 * (SF - 2 * DE);
    int64_t term = ceil_div_s(num, denom) * (CR + 4);
    if (term < 0) { term = 0; }
    const int64_t payload_symb_nb = 8 + term;

    // Symbol time Tsym = 2^SF / bw_khz (ms). Work in quarter-symbol units so the
    // 4.25-symbol preamble tail stays exact without floating point.
    //   preamble symbols  = preamble_len + 4.25  -> (4*preamble_len + 17) quarters
    //   payload  symbols  = payload_symb_nb      -> (4*payload_symb_nb) quarters
    const int64_t total_quarter = 4 * (int64_t)preamble_len + 17 + 4 * payload_symb_nb;
    // total_time_ms = total_quarter/4 * 2^SF / bw_khz = total_quarter * 2^SF / (4 * bw_khz)
    const uint64_t numerator = (uint64_t)total_quarter * ((uint64_t)1u << sf);
    const uint64_t divisor = 4u * (uint64_t)bw_khz;
    const uint64_t ms = (numerator + divisor - 1u) / divisor; // ceil
    return (uint32_t)ms;
}
