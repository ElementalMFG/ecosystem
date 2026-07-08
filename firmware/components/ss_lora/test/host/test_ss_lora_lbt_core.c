// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// test_ss_lora_lbt_core.c — host harness for the ss_lora LBT + duty-cycle core.
//
// No Unity dependency: a tiny CHECK macro records failures. Built with
// -fsanitize=address,undefined so any out-of-bounds access to the event ring
// aborts the run. Only ss_lora_lbt_core.c is linked (ss_lora.c needs the IDF).

#include <stdint.h>
#include <stdio.h>

#include "ss_lora_lbt_core.h"

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
    // 1. permille table: one representative frequency per sub-band + out-of-band.
    {
        CHECK(ss_lora_duty_permille(865000000u) == 10u);  // 863.0–868.0 : 1 %
        CHECK(ss_lora_duty_permille(868300000u) == 10u);  // 868.0–868.6 : 1 %
        CHECK(ss_lora_duty_permille(868900000u) == 1u);   // 868.7–869.2 : 0.1 %
        CHECK(ss_lora_duty_permille(869525000u) == 100u); // 869.4–869.65 : 10 %
        CHECK(ss_lora_duty_permille(869800000u) == 10u);  // 869.7–870.0 : 1 %
        // Out-of-band and gap frequencies map to 0 (fail-safe).
        CHECK(ss_lora_duty_permille(862000000u) == 0u);   // below band
        CHECK(ss_lora_duty_permille(868650000u) == 0u);   // gap 868.6–868.7
        CHECK(ss_lora_duty_permille(871000000u) == 0u);   // above band
        CHECK(ss_lora_duty_permille(915000000u) == 0u);   // US band
        printf("case 1  permille table (each sub-band + out-of-band=0)\n");
    }

    // 2. budget_ms = permille * WINDOW / 1000.
    {
        CHECK(ss_lora_duty_budget_ms(865000000u) == 36000u);  // 1 %
        CHECK(ss_lora_duty_budget_ms(868900000u) == 3600u);   // 0.1 %
        CHECK(ss_lora_duty_budget_ms(869525000u) == 360000u); // 10 %
        CHECK(ss_lora_duty_budget_ms(862000000u) == 0u);      // out-of-band
        printf("case 2  budget_ms math (36000 / 3600 / 360000 / 0)\n");
    }

    // 3. toa_ms: sanity band + monotonicity in SF and payload.
    {
        // SF7/BW125/CR4:5/8-byte, preamble 8, explicit header, CRC on, no low-DR.
        const uint32_t base = ss_lora_lbt_toa_ms(7, 125, 5, 8, 8, true, true, false);
        CHECK(base >= 20u && base <= 80u); // plausible ms range
        // Higher SF -> larger ToA.
        const uint32_t sf9 = ss_lora_lbt_toa_ms(9, 125, 5, 8, 8, true, true, false);
        const uint32_t sf12 = ss_lora_lbt_toa_ms(12, 125, 5, 8, 8, true, true, false);
        CHECK(sf9 > base);
        CHECK(sf12 > sf9);
        // Larger payload -> larger ToA.
        const uint32_t big = ss_lora_lbt_toa_ms(7, 125, 5, 8, 64, true, true, false);
        CHECK(big > base);
        printf("case 3  toa_ms base=%ums sf9=%ums sf12=%ums big=%ums (monotonic)\n",
               (unsigned)base, (unsigned)sf9, (unsigned)sf12, (unsigned)big);
    }

    // 4. used_ms windowing: records accumulate, then evict once past the window.
    {
        ss_lora_duty_state_t st;
        ss_lora_duty_init(&st);
        const uint32_t f = 865000000u;
        ss_lora_duty_record(&st, f, 100u, 1000u, false);
        ss_lora_duty_record(&st, f, 150u, 2000u, false);
        CHECK(ss_lora_duty_used_ms(&st, f, 2000u) == 250u);
        // A different sub-band does not share the budget.
        CHECK(ss_lora_duty_used_ms(&st, 869525000u, 2000u) == 0u);
        // Advance now_ms past the window from the first event: it evicts.
        CHECK(ss_lora_duty_used_ms(&st, f, 1000u + SS_LORA_DUTY_WINDOW_MS + 1u) == 150u);
        // Advance past both: fully evicted.
        CHECK(ss_lora_duty_used_ms(&st, f, 2000u + SS_LORA_DUTY_WINDOW_MS + 1u) == 0u);
        printf("case 4  used_ms windowing + per-sub-band isolation + eviction\n");
    }

    // 5. evaluate: ALLOW when under budget and channel clear.
    {
        ss_lora_duty_state_t st;
        ss_lora_duty_init(&st);
        const ss_lora_tx_req_t req = {.freq_hz = 865000000u, .airtime_ms = 100u, .is_sos = false};
        CHECK(ss_lora_tx_evaluate(&st, &req, -110, SS_LORA_LBT_DEFAULT_THRESHOLD_DBM, 5000u) ==
              SS_LORA_TX_ALLOW);
        printf("case 5  evaluate ALLOW (under budget, channel clear)\n");
    }

    // 6. evaluate: DEFER_BUSY when RSSI >= threshold (channel busy).
    {
        ss_lora_duty_state_t st;
        ss_lora_duty_init(&st);
        const ss_lora_tx_req_t req = {.freq_hz = 865000000u, .airtime_ms = 100u, .is_sos = false};
        CHECK(ss_lora_tx_evaluate(&st, &req, -80, SS_LORA_LBT_DEFAULT_THRESHOLD_DBM, 5000u) ==
              SS_LORA_TX_DEFER_BUSY);
        // Exactly at threshold is busy too.
        CHECK(ss_lora_tx_evaluate(&st, &req, SS_LORA_LBT_DEFAULT_THRESHOLD_DBM,
                                  SS_LORA_LBT_DEFAULT_THRESHOLD_DBM, 5000u) == SS_LORA_TX_DEFER_BUSY);
        printf("case 6  evaluate DEFER_BUSY (rssi >= threshold)\n");
    }

    // 7. evaluate: BLOCK_DUTY when over budget and not an SOS.
    {
        ss_lora_duty_state_t st;
        ss_lora_duty_init(&st);
        const uint32_t f = 868900000u; // 0.1 % -> budget 3600 ms
        // Fill the budget.
        ss_lora_duty_record(&st, f, 3600u, 1000u, false);
        const ss_lora_tx_req_t req = {.freq_hz = f, .airtime_ms = 100u, .is_sos = false};
        CHECK(ss_lora_tx_evaluate(&st, &req, -110, SS_LORA_LBT_DEFAULT_THRESHOLD_DBM, 2000u) ==
              SS_LORA_TX_BLOCK_DUTY);
        printf("case 7  evaluate BLOCK_DUTY (over budget, not SOS)\n");
    }

    // 8. evaluate: ALLOW_SOS_OVERRIDE when over budget but SOS within override cap.
    {
        ss_lora_duty_state_t st;
        ss_lora_duty_init(&st);
        const uint32_t f = 868900000u; // budget 3600 ms
        ss_lora_duty_record(&st, f, 3600u, 1000u, false);
        const ss_lora_tx_req_t req = {.freq_hz = f, .airtime_ms = 1000u, .is_sos = true};
        CHECK(ss_lora_tx_evaluate(&st, &req, -110, SS_LORA_LBT_DEFAULT_THRESHOLD_DBM, 2000u) ==
              SS_LORA_TX_ALLOW_SOS_OVERRIDE);
        // LBT still applies to SOS: busy channel defers even the override.
        CHECK(ss_lora_tx_evaluate(&st, &req, -70, SS_LORA_LBT_DEFAULT_THRESHOLD_DBM, 2000u) ==
              SS_LORA_TX_DEFER_BUSY);
        printf("case 8  evaluate ALLOW_SOS_OVERRIDE (+LBT still defers SOS)\n");
    }

    // 9. evaluate: BLOCK_DUTY when the SOS override cap itself is exhausted.
    {
        ss_lora_duty_state_t st;
        ss_lora_duty_init(&st);
        const uint32_t f = 868900000u; // budget 3600 ms
        ss_lora_duty_record(&st, f, 3600u, 1000u, false);          // duty exhausted
        ss_lora_duty_record(&st, f, 4500u, 1500u, true);           // SOS override used
        // 4500 used + 1000 requested = 5500 > 5000 cap -> blocked.
        const ss_lora_tx_req_t req = {.freq_hz = f, .airtime_ms = 1000u, .is_sos = true};
        CHECK(ss_lora_tx_evaluate(&st, &req, -110, SS_LORA_LBT_DEFAULT_THRESHOLD_DBM, 2000u) ==
              SS_LORA_TX_BLOCK_DUTY);
        printf("case 9  evaluate BLOCK_DUTY (SOS override cap exhausted)\n");
    }

    // 10. evaluate: ERR on NULL args and on an unknown/out-of-band frequency.
    {
        ss_lora_duty_state_t st;
        ss_lora_duty_init(&st);
        const ss_lora_tx_req_t req = {.freq_hz = 865000000u, .airtime_ms = 100u, .is_sos = false};
        CHECK(ss_lora_tx_evaluate(NULL, &req, -110, -90, 0u) == SS_LORA_TX_ERR);
        CHECK(ss_lora_tx_evaluate(&st, NULL, -110, -90, 0u) == SS_LORA_TX_ERR);
        const ss_lora_tx_req_t oob = {.freq_hz = 915000000u, .airtime_ms = 100u, .is_sos = false};
        CHECK(ss_lora_tx_evaluate(&st, &oob, -110, -90, 0u) == SS_LORA_TX_ERR);
        CHECK(SS_LORA_TX_ERR == 0); // fail-safe invariant
        printf("case 10 evaluate ERR (NULL st/req, unknown band; ERR==0)\n");
    }

    // 11. backoff cap + should_give_up boundary.
    {
        CHECK(ss_lora_lbt_backoff_ms(0) == SS_LORA_LBT_BASE_BACKOFF_MS);       // 10
        CHECK(ss_lora_lbt_backoff_ms(1) == 20u);
        CHECK(ss_lora_lbt_backoff_ms(5) == SS_LORA_LBT_MAX_BACKOFF_MS);        // 320
        CHECK(ss_lora_lbt_backoff_ms(100) == SS_LORA_LBT_MAX_BACKOFF_MS);      // saturates
        CHECK(!ss_lora_lbt_should_give_up(SS_LORA_LBT_MAX_RETRIES - 1u));
        CHECK(ss_lora_lbt_should_give_up(SS_LORA_LBT_MAX_RETRIES));
        CHECK(ss_lora_lbt_should_give_up(SS_LORA_LBT_MAX_RETRIES + 10u));
        printf("case 11 backoff cap + should_give_up boundary\n");
    }

    // 12. ring overwrite is bounded (no OOB under ASan when > MAX_EVENTS recorded).
    {
        ss_lora_duty_state_t st;
        ss_lora_duty_init(&st);
        const uint32_t f = 865000000u;
        for (uint32_t i = 0; i < SS_LORA_DUTY_MAX_EVENTS + 10u; i++) {
            ss_lora_duty_record(&st, f, 1u, 1000u + i, false);
        }
        CHECK(st.count == SS_LORA_DUTY_MAX_EVENTS);
        // Only the retained (bounded) events count toward used airtime.
        CHECK(ss_lora_duty_used_ms(&st, f, 1000u + SS_LORA_DUTY_MAX_EVENTS + 10u) ==
              SS_LORA_DUTY_MAX_EVENTS);
        printf("case 12 ring overwrite bounded to %u events\n",
               (unsigned)SS_LORA_DUTY_MAX_EVENTS);
    }

    printf("\n%d checks, %d failures\n", g_checks, g_fails);
    return (g_fails != 0) ? 1 : 0;
}
