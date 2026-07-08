// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_lora_lbt_core.h — pure, host-testable Listen-Before-Talk (LBT) and EU868
// duty-cycle policy core for S-03-013.
//
// SCOPE: this layer holds NO ESP-IDF and NO HAL dependency — it includes only
// <stdint.h>, <stdbool.h>, <stddef.h>. It reasons purely about frequencies,
// airtimes, RSSI/CAD samples, and caller-supplied monotonic time, so it can be
// exercised with a plain gcc host harness under -fsanitize=address,undefined
// with no include hacks. The on-target LoRa driver (S-03-011, SX1262) will feed
// it RSSI/CAD samples and use ss_lora_tx_evaluate() to gate its ss_lora_tx()
// path; the driver glue lives in ss_lora.c and is compiled on-target only.
//
// ETSI EN 300 220 / EU868 SUB-BAND DUTY MODEL: transmitters in the 863–870 MHz
// SRD band are constrained to a per-sub-band duty cycle measured over a rolling
// one-hour window (SS_LORA_DUTY_WINDOW_MS). Each sub-band has its own budget
// expressed in permille of the window (see ss_lora_duty_permille). The core
// tracks transmitted airtime per sub-band in a bounded ring and refuses to hand
// out airtime that would exceed the budget.
//
// FAIL-SAFE DESIGN: the decision enum places the error value at 0
// (SS_LORA_TX_ERR == 0). Any NULL argument, unknown/out-of-band frequency, or
// zeroed/uninitialised state therefore maps to "do NOT transmit". A caller that
// forgets to check the result, or reads an uninitialised decision, defaults to
// silence rather than an illegal emission.
//
// SOS OVERRIDE (C-08): when the duty budget for a sub-band is exhausted, a
// safety (SOS) transmission may still be permitted, but only for a bounded
// amount of airtime per window (SS_LORA_SOS_OVERRIDE_MAX_MS). This implements
// C-08's "ignore duty cycle only where safety-legal": the override is capped so
// a stuck SOS path cannot monopolise the channel. LBT (collision avoidance)
// still applies to SOS traffic — a busy channel defers even an SOS.
//
// MONOTONIC-MS WRAP: all time is passed in as `now_ms` (a free-running monotonic
// millisecond counter, e.g. esp_timer/xTaskGetTickCount-derived). Window
// membership is tested with UNSIGNED subtraction `(now_ms - ev.ts_ms) <= WINDOW`.
// Because the window (3.6e6 ms ≈ 1 h) is vastly smaller than the uint32 period
// (2^32 ms ≈ 49.7 days), this comparison stays correct across a single wrap: a
// stale timestamp on the far side of the wrap yields a large unsigned delta and
// is treated as expired. No wall-clock is ever read inside the core.

#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SS_LORA_DUTY_WINDOW_MS 3600000u        // 1 h ETSI rolling window
#define SS_LORA_DUTY_MAX_EVENTS 64u            // bounded per-state ring capacity
#define SS_LORA_SOS_OVERRIDE_MAX_MS 5000u      // bounded SOS override airtime / window
#define SS_LORA_LBT_BASE_BACKOFF_MS 10u        // first LBT backoff step
#define SS_LORA_LBT_MAX_BACKOFF_MS 320u        // backoff ceiling
#define SS_LORA_LBT_MAX_RETRIES 5u             // give up after this many attempts
#define SS_LORA_LBT_DEFAULT_THRESHOLD_DBM (-90) // CAD/RSSI busy threshold (dBm)

// Transmit decision. Fail-safe: ERR == 0 so any bad/uninitialised path is "no TX".
typedef enum {
    SS_LORA_TX_ERR = 0,            // NULL args / unknown sub-band -> never transmit
    SS_LORA_TX_ALLOW,             // clear to transmit
    SS_LORA_TX_DEFER_BUSY,        // LBT sensed busy channel -> back off and retry
    SS_LORA_TX_BLOCK_DUTY,        // duty budget exhausted -> drop & log (C-08)
    SS_LORA_TX_ALLOW_SOS_OVERRIDE, // duty exhausted, bounded SOS override applies
} ss_lora_tx_decision_t;

// One recorded transmission for duty accounting.
typedef struct {
    uint32_t ts_ms;      // monotonic time the airtime was recorded
    uint32_t airtime_ms; // time-on-air consumed
    uint32_t freq_hz;    // centre frequency (selects the sub-band)
    bool sos_override;   // this airtime was granted under the SOS override
} ss_lora_tx_event_t;

// Bounded per-node duty-accounting state (fixed ring, no dynamic allocation).
typedef struct {
    ss_lora_tx_event_t events[SS_LORA_DUTY_MAX_EVENTS];
    uint32_t count; // number of valid events in the ring (<= MAX_EVENTS)
    uint32_t head;  // index of the oldest valid event
} ss_lora_duty_state_t;

// A pending transmit request evaluated against policy.
typedef struct {
    uint32_t freq_hz;    // centre frequency
    uint32_t airtime_ms; // estimated time-on-air (see ss_lora_lbt_toa_ms)
    bool is_sos;         // request is a safety transmission
} ss_lora_tx_req_t;

/**
 * Zero-initialise a duty-accounting state (empty ring).
 *
 * Pre:  st != NULL.
 * Post: st->count == 0, st->head == 0. No-op if st == NULL.
 */
void ss_lora_duty_init(ss_lora_duty_state_t* st);

/**
 * Duty-cycle budget of the EU868 sub-band containing `freq_hz`, in permille
 * (10 = 1 %, 1 = 0.1 %, 100 = 10 %).
 *
 * Pre:  none.
 * Post/return: the sub-band permille per ETSI EN 300 220, or 0 if `freq_hz` is
 * not inside any EU868 sub-band (which forces a fail-safe "no TX").
 */
uint16_t ss_lora_duty_permille(uint32_t freq_hz);

/**
 * Absolute airtime budget for the sub-band containing `freq_hz` over one window.
 *
 * Pre:  none.
 * Post/return: permille(freq_hz) * SS_LORA_DUTY_WINDOW_MS / 1000, or 0 if the
 * frequency is not an EU868 sub-band.
 */
uint32_t ss_lora_duty_budget_ms(uint32_t freq_hz);

/**
 * Airtime already consumed within the rolling window for the sub-band that
 * contains `freq_hz`.
 *
 * Pre:  none (st may be NULL -> returns 0).
 * Post/return: sum of `airtime_ms` over events whose frequency maps to the SAME
 * sub-band as `freq_hz` and whose timestamp is within SS_LORA_DUTY_WINDOW_MS of
 * `now_ms` (unsigned wrap-safe). Stale events are ignored (compute-only; the
 * const state is never mutated).
 */
uint32_t ss_lora_duty_used_ms(const ss_lora_duty_state_t* st, uint32_t freq_hz, uint32_t now_ms);

/**
 * Record a completed transmission into the ring.
 *
 * Pre:  st != NULL.
 * Post: appends {now_ms, airtime_ms, freq_hz, sos_override}; when the ring is
 * full the oldest event is overwritten (head advances). No-op if st == NULL.
 */
void ss_lora_duty_record(ss_lora_duty_state_t* st, uint32_t freq_hz, uint32_t airtime_ms,
                         uint32_t now_ms, bool sos_override);

/**
 * Evaluate a transmit request against duty policy and LBT.
 *
 * Pre:  none (st/req may be NULL).
 * Post/return (evaluated in order):
 *   1. SS_LORA_TX_ERR              if st == NULL, req == NULL, or
 *                                  ss_lora_duty_permille(req->freq_hz) == 0.
 *   2. duty: fits = used + req->airtime_ms <= budget. If it does not fit:
 *        - is_sos AND (sos-override airtime used + req->airtime_ms
 *          <= SS_LORA_SOS_OVERRIDE_MAX_MS) -> candidate = ALLOW_SOS_OVERRIDE;
 *        - otherwise return SS_LORA_TX_BLOCK_DUTY.
 *      If it fits -> candidate = ALLOW.
 *   3. LBT: rssi_dbm >= lbt_threshold_dbm -> return SS_LORA_TX_DEFER_BUSY
 *      (applies to SOS too — collision avoidance, bounded by backoff/give-up).
 *   4. return candidate.
 */
ss_lora_tx_decision_t ss_lora_tx_evaluate(const ss_lora_duty_state_t* st,
                                          const ss_lora_tx_req_t* req, int16_t rssi_dbm,
                                          int16_t lbt_threshold_dbm, uint32_t now_ms);

/**
 * Exponential LBT backoff for a retry attempt.
 *
 * Pre:  none.
 * Post/return: SS_LORA_LBT_BASE_BACKOFF_MS << attempt, capped at
 * SS_LORA_LBT_MAX_BACKOFF_MS. Large `attempt` values saturate at the ceiling
 * (no shift overflow).
 */
uint32_t ss_lora_lbt_backoff_ms(uint32_t attempt);

/**
 * Whether LBT retries are exhausted for `attempt`.
 *
 * Pre:  none.
 * Post/return: attempt >= SS_LORA_LBT_MAX_RETRIES.
 */
bool ss_lora_lbt_should_give_up(uint32_t attempt);

/**
 * Semtech LoRa time-on-air, rounded UP to whole milliseconds.
 *
 * Computes Tsym = 2^SF / BW; preamble = (preamble_len + 4.25) * Tsym; and
 * payloadSymbNb = 8 + max(ceil((8*PL - 4*SF + 28 + 16*crc - 20*IH) /
 * (4*(SF - 2*DE))) * (CR + 4), 0), where CR = cr - 4 (coding rate 4/5..4/8 for
 * cr 5..8), IH = !explicit_header, DE = low_dr_optimize, crc = crc_on. All math
 * is integer (no libm); the result is ceil of the total on-air time in ms.
 *
 * Pre:  sf in [6,12], bw_khz > 0, cr in [5,8].
 * Post/return: ceil(time-on-air) in ms.
 */
uint32_t ss_lora_lbt_toa_ms(uint8_t sf, uint16_t bw_khz, uint8_t cr, uint16_t preamble_len,
                            uint8_t payload_len, bool explicit_header, bool crc_on,
                            bool low_dr_optimize);

#ifdef __cplusplus
} // extern "C"
#endif
