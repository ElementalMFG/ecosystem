// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// test_ss_wifi_prov_core.c — host harness for the ss_wifi provisioning
// session core (S-03-015).
//
// No Unity dependency: a tiny CHECK macro records failures. Built with
// -fsanitize=address,undefined. Only ss_wifi_prov_core.c is linked
// (ss_wifi_prov.c needs the IDF).

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "ss_wifi_prov_core.h"

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

// --- fake entropy sources -------------------------------------------------

// Deterministic counter bytes (all < 248, so nothing is rejected).
static bool ent_counter(void* ctx, uint8_t* buf, size_t len)
{
    uint8_t* c = (uint8_t*)ctx;
    for (size_t i = 0; i < len; i++) { buf[i] = (*c)++ % 200u; }
    return true;
}

// Always fails.
static bool ent_fail(void* ctx, uint8_t* buf, size_t len)
{
    (void)ctx;
    (void)buf;
    (void)len;
    return false;
}

// Every byte is in the rejection band (>= 248): forces refill exhaustion.
static bool ent_all_rejected(void* ctx, uint8_t* buf, size_t len)
{
    (void)ctx;
    for (size_t i = 0; i < len; i++) { buf[i] = 0xFFu; }
    return true;
}

// First N bytes are rejected, the rest accepted: exercises the resampling
// path without exhausting it.
typedef struct {
    size_t reject_first;
    size_t served;
} ent_mixed_t;

static bool ent_mixed(void* ctx, uint8_t* buf, size_t len)
{
    ent_mixed_t* m = (ent_mixed_t*)ctx;
    for (size_t i = 0; i < len; i++) {
        buf[i] = (m->served < m->reject_first) ? 0xF8u : (uint8_t)(m->served % 31u);
        m->served++;
    }
    return true;
}

static const char* ALPHABET = "23456789abcdefghjkmnpqrstuvwxyz";

static bool in_alphabet(char c)
{
    return strchr(ALPHABET, c) != NULL && c != '\0';
}

static bool all_zero(const void* p, size_t n)
{
    const uint8_t* b = (const uint8_t*)p;
    for (size_t i = 0; i < n; i++) {
        if (b[i] != 0u) { return false; }
    }
    return true;
}

static ss_wifi_prov_session_t begin_active(uint32_t now, uint32_t window)
{
    ss_wifi_prov_session_t s;
    memset(&s, 0, sizeof(s));
    uint8_t ctr = 0;
    const bool ok = ss_wifi_prov_begin(&s, ent_counter, &ctr, now, window);
    CHECK(ok);
    CHECK(s.state == SS_WIFI_PROV_ACTIVE);
    return s;
}

int main(void)
{
    static const char* const kValidSsid = "HomeNet";
    static const char* const kValidPsk = "hunter2hunter2";

    // 1. begin: generated secrets have the right shape.
    {
        ss_wifi_prov_session_t s = begin_active(1000u, 0u);
        CHECK(strlen(s.ap_pass) == SS_WIFI_PROV_PASS_LEN);
        CHECK(strlen(s.code) == SS_WIFI_PROV_CODE_LEN);
        for (size_t i = 0; i < SS_WIFI_PROV_PASS_LEN; i++) { CHECK(in_alphabet(s.ap_pass[i])); }
        for (size_t i = 0; i < SS_WIFI_PROV_CODE_LEN; i++) { CHECK(in_alphabet(s.code[i])); }
        CHECK(s.deadline_ms == 1000u + SS_WIFI_PROV_WINDOW_DEFAULT_MS);
        ss_wifi_prov_end(&s);
        printf("case 1  begin generates well-formed secrets\n");
    }

    // 2. begin is deterministic for identical entropy streams.
    {
        ss_wifi_prov_session_t a, b;
        memset(&a, 0, sizeof(a));
        memset(&b, 0, sizeof(b));
        uint8_t c1 = 7u, c2 = 7u;
        CHECK(ss_wifi_prov_begin(&a, ent_counter, &c1, 0u, 0u));
        CHECK(ss_wifi_prov_begin(&b, ent_counter, &c2, 0u, 0u));
        CHECK(strcmp(a.ap_pass, b.ap_pass) == 0);
        CHECK(strcmp(a.code, b.code) == 0);
        ss_wifi_prov_end(&a);
        ss_wifi_prov_end(&b);
        printf("case 2  deterministic under identical entropy\n");
    }

    // 3. entropy failure fails closed with no secret residue.
    {
        ss_wifi_prov_session_t s;
        memset(&s, 0, sizeof(s));
        CHECK(!ss_wifi_prov_begin(&s, ent_fail, NULL, 0u, 0u));
        CHECK(s.state == SS_WIFI_PROV_ABORTED);
        CHECK(s.abort_reason == SS_WIFI_PROV_ABORT_ENTROPY);
        CHECK(all_zero(s.ap_pass, sizeof(s.ap_pass)));
        CHECK(all_zero(s.code, sizeof(s.code)));
        CHECK(!ss_wifi_prov_begin(&s, NULL, NULL, 0u, 0u)); // NULL entropy also fails closed
        CHECK(s.abort_reason == SS_WIFI_PROV_ABORT_ENTROPY);
        CHECK(!ss_wifi_prov_begin(NULL, ent_fail, NULL, 0u, 0u));
        printf("case 3  entropy failure fails closed\n");
    }

    // 4. rejection sampling: exhaustion fails closed; partial rejection works.
    {
        ss_wifi_prov_session_t s;
        memset(&s, 0, sizeof(s));
        CHECK(!ss_wifi_prov_begin(&s, ent_all_rejected, NULL, 0u, 0u));
        CHECK(s.state == SS_WIFI_PROV_ABORTED);
        CHECK(s.abort_reason == SS_WIFI_PROV_ABORT_ENTROPY);

        ent_mixed_t m = {.reject_first = 20u, .served = 0u};
        CHECK(ss_wifi_prov_begin(&s, ent_mixed, &m, 0u, 0u));
        CHECK(s.state == SS_WIFI_PROV_ACTIVE);
        CHECK(strlen(s.ap_pass) == SS_WIFI_PROV_PASS_LEN);
        ss_wifi_prov_end(&s);
        printf("case 4  rejection sampling bounded and correct\n");
    }

    // 5. window clamping.
    {
        ss_wifi_prov_session_t s = begin_active(0u, 1u); // below the floor
        CHECK(s.deadline_ms == SS_WIFI_PROV_WINDOW_MIN_MS);
        ss_wifi_prov_end(&s);
        s = begin_active(0u, 0xFFFFFFFFu); // above the ceiling
        CHECK(s.deadline_ms == SS_WIFI_PROV_WINDOW_MAX_MS);
        ss_wifi_prov_end(&s);
        printf("case 5  window clamped to [min, max]\n");
    }

    // 6. credential validation matrix.
    {
        // SSID rules.
        CHECK(!ss_wifi_prov_creds_valid(NULL, 5u, "password", 8u));
        CHECK(!ss_wifi_prov_creds_valid("x", 0u, "password", 8u));
        CHECK(!ss_wifi_prov_creds_valid("123456789012345678901234567890123", 33u, "password", 8u));
        CHECK(ss_wifi_prov_creds_valid("12345678901234567890123456789012", 32u, "password", 8u));
        CHECK(!ss_wifi_prov_creds_valid("bad\nssid", 8u, "password", 8u));   // control byte
        CHECK(!ss_wifi_prov_creds_valid("bad\x7fssid", 8u, "password", 8u)); // DEL
        CHECK(ss_wifi_prov_creds_valid("caf\xc3\xa9", 5u, "password", 8u));  // UTF-8 ok

        // PSK rules.
        CHECK(ss_wifi_prov_creds_valid("open-net", 8u, NULL, 0u));  // open network
        CHECK(ss_wifi_prov_creds_valid("open-net", 8u, "", 0u));    // open network
        CHECK(!ss_wifi_prov_creds_valid("net", 3u, NULL, 8u));      // NULL with length
        CHECK(!ss_wifi_prov_creds_valid("net", 3u, "seven77", 7u)); // too short
        CHECK(ss_wifi_prov_creds_valid("net", 3u, "eight888", 8u));
        char psk63[64];
        memset(psk63, 'a', 63u);
        psk63[63] = '\0';
        CHECK(ss_wifi_prov_creds_valid("net", 3u, psk63, 63u));
        CHECK(!ss_wifi_prov_creds_valid("net", 3u, "bad\tpsk!", 8u)); // control byte
        char hex64[65];
        memset(hex64, 'A', 64u);
        hex64[64] = '\0';
        CHECK(ss_wifi_prov_creds_valid("net", 3u, hex64, 64u)); // raw PSK hex
        hex64[10] = 'g';                                        // not hex
        CHECK(!ss_wifi_prov_creds_valid("net", 3u, hex64, 64u));
        printf("case 6  credential validation matrix\n");
    }

    // 7. happy path: submit -> GRACE -> DONE -> single take -> end.
    {
        ss_wifi_prov_session_t s = begin_active(1000u, 0u);
        CHECK(ss_wifi_prov_submit(&s, kValidSsid, strlen(kValidSsid), kValidPsk, strlen(kValidPsk),
                                  2000u) == SS_WIFI_PROV_SUBMIT_ACCEPTED);
        CHECK(s.state == SS_WIFI_PROV_GRACE);
        CHECK(ss_wifi_prov_tick(&s, 2000u + SS_WIFI_PROV_GRACE_MS - 1u) == SS_WIFI_PROV_GRACE);
        CHECK(ss_wifi_prov_tick(&s, 2000u + SS_WIFI_PROV_GRACE_MS) == SS_WIFI_PROV_DONE);
        // AP secrets are wiped at DONE; credentials remain.
        CHECK(all_zero(s.ap_pass, sizeof(s.ap_pass)));
        CHECK(all_zero(s.code, sizeof(s.code)));

        char ssid[SS_WIFI_PROV_SSID_MAX_LEN + 1u];
        char psk[SS_WIFI_PROV_PSK_MAX_LEN + 1u];
        CHECK(ss_wifi_prov_take_credentials(&s, ssid, sizeof(ssid), psk, sizeof(psk)));
        CHECK(strcmp(ssid, kValidSsid) == 0);
        CHECK(strcmp(psk, kValidPsk) == 0);
        // Internal copies wiped; a second take must fail.
        CHECK(all_zero(s.ssid, sizeof(s.ssid)));
        CHECK(all_zero(s.psk, sizeof(s.psk)));
        CHECK(!ss_wifi_prov_take_credentials(&s, ssid, sizeof(ssid), psk, sizeof(psk)));
        CHECK(all_zero(ssid, sizeof(ssid))); // failed take zeroes the outputs
        ss_wifi_prov_end(&s);
        CHECK(s.state == SS_WIFI_PROV_IDLE);
        printf("case 7  happy path with one-shot handoff\n");
    }

    // 8. take preconditions: wrong state / short buffers.
    {
        ss_wifi_prov_session_t s = begin_active(0u, 0u);
        char ssid[SS_WIFI_PROV_SSID_MAX_LEN + 1u];
        char psk[SS_WIFI_PROV_PSK_MAX_LEN + 1u];
        CHECK(!ss_wifi_prov_take_credentials(&s, ssid, sizeof(ssid), psk, sizeof(psk))); // ACTIVE
        CHECK(ss_wifi_prov_submit(&s, kValidSsid, strlen(kValidSsid), kValidPsk, strlen(kValidPsk),
                                  1u) == SS_WIFI_PROV_SUBMIT_ACCEPTED);
        (void)ss_wifi_prov_tick(&s, 1u + SS_WIFI_PROV_GRACE_MS);
        CHECK(!ss_wifi_prov_take_credentials(&s, ssid, sizeof(ssid) - 1u, psk, sizeof(psk)));
        CHECK(!ss_wifi_prov_take_credentials(&s, NULL, 0u, psk, sizeof(psk)));
        CHECK(!ss_wifi_prov_take_credentials(NULL, ssid, sizeof(ssid), psk, sizeof(psk)));
        CHECK(ss_wifi_prov_take_credentials(&s, ssid, sizeof(ssid), psk, sizeof(psk)));
        ss_wifi_prov_end(&s);
        printf("case 8  take_credentials preconditions\n");
    }

    // 9. rejected submissions are bounded.
    {
        ss_wifi_prov_session_t s = begin_active(0u, 0u);
        for (uint32_t i = 0; i + 1u < SS_WIFI_PROV_MAX_BAD_SUBMITS; i++) {
            CHECK(ss_wifi_prov_submit(&s, "", 0u, "x", 1u, 10u) == SS_WIFI_PROV_SUBMIT_REJECTED);
            CHECK(s.state == SS_WIFI_PROV_ACTIVE);
        }
        CHECK(ss_wifi_prov_submit(&s, "", 0u, "x", 1u, 10u) == SS_WIFI_PROV_SUBMIT_REJECTED);
        CHECK(s.state == SS_WIFI_PROV_ABORTED);
        CHECK(s.abort_reason == SS_WIFI_PROV_ABORT_BAD_SUBMITS);
        CHECK(all_zero(s.ap_pass, sizeof(s.ap_pass)));
        // Post-abort submissions are ignored.
        CHECK(ss_wifi_prov_submit(&s, kValidSsid, strlen(kValidSsid), kValidPsk, strlen(kValidPsk),
                                  11u) == SS_WIFI_PROV_SUBMIT_IGNORED);
        ss_wifi_prov_end(&s);
        printf("case 9  bad submissions bounded then ignored\n");
    }

    // 10. window timeout via tick and via submit.
    {
        ss_wifi_prov_session_t s = begin_active(1000u, 0u);
        const uint32_t deadline = s.deadline_ms;
        CHECK(ss_wifi_prov_tick(&s, deadline - 1u) == SS_WIFI_PROV_ACTIVE);
        CHECK(ss_wifi_prov_tick(&s, deadline) == SS_WIFI_PROV_ABORTED);
        CHECK(s.abort_reason == SS_WIFI_PROV_ABORT_TIMEOUT);
        CHECK(all_zero(s.ap_pass, sizeof(s.ap_pass)));

        s = begin_active(1000u, 0u);
        CHECK(ss_wifi_prov_submit(&s, kValidSsid, strlen(kValidSsid), kValidPsk, strlen(kValidPsk),
                                  s.deadline_ms) == SS_WIFI_PROV_SUBMIT_IGNORED);
        CHECK(s.state == SS_WIFI_PROV_ABORTED);
        CHECK(s.abort_reason == SS_WIFI_PROV_ABORT_TIMEOUT);
        ss_wifi_prov_end(&s);
        printf("case 10 window timeout paths\n");
    }

    // 11. clock wraparound: deadline past 2^32 still fires correctly.
    {
        ss_wifi_prov_session_t s = begin_active(0xFFFFFF00u, 0u);         // deadline wraps
        CHECK(ss_wifi_prov_tick(&s, 0xFFFFFFF0u) == SS_WIFI_PROV_ACTIVE); // before wrap
        CHECK(ss_wifi_prov_tick(&s, 5u) == SS_WIFI_PROV_ACTIVE); // after wrap, pre-deadline
        CHECK(ss_wifi_prov_tick(&s, s.deadline_ms + 1u) == SS_WIFI_PROV_ABORTED);
        ss_wifi_prov_end(&s);
        printf("case 11 wrap-safe deadline comparison\n");
    }

    // 12. cancel and power-restrict abort live sessions and wipe secrets.
    {
        ss_wifi_prov_session_t s = begin_active(0u, 0u);
        ss_wifi_prov_cancel(&s);
        CHECK(s.state == SS_WIFI_PROV_ABORTED);
        CHECK(s.abort_reason == SS_WIFI_PROV_ABORT_CANCEL);
        CHECK(all_zero(s.ap_pass, sizeof(s.ap_pass)));

        s = begin_active(0u, 0u);
        CHECK(ss_wifi_prov_submit(&s, kValidSsid, strlen(kValidSsid), kValidPsk, strlen(kValidPsk),
                                  1u) == SS_WIFI_PROV_SUBMIT_ACCEPTED);
        ss_wifi_prov_power_restrict(&s); // GRACE abort discards accepted creds
        CHECK(s.state == SS_WIFI_PROV_ABORTED);
        CHECK(s.abort_reason == SS_WIFI_PROV_ABORT_POWER);
        CHECK(all_zero(s.ssid, sizeof(s.ssid)));
        CHECK(all_zero(s.psk, sizeof(s.psk)));

        // Terminal states are unaffected by further cancels.
        ss_wifi_prov_cancel(&s);
        CHECK(s.abort_reason == SS_WIFI_PROV_ABORT_POWER);
        ss_wifi_prov_cancel(NULL);
        ss_wifi_prov_power_restrict(NULL);
        ss_wifi_prov_end(&s);
        printf("case 12 cancel / power-restrict semantics\n");
    }

    // 13. end() from any state leaves a fully zeroed struct.
    {
        ss_wifi_prov_session_t s = begin_active(0u, 0u);
        CHECK(ss_wifi_prov_submit(&s, kValidSsid, strlen(kValidSsid), kValidPsk, strlen(kValidPsk),
                                  1u) == SS_WIFI_PROV_SUBMIT_ACCEPTED);
        ss_wifi_prov_end(&s);
        CHECK(all_zero(&s, sizeof(s)));
        ss_wifi_prov_end(NULL); // safe no-op
        printf("case 13 end() zeroizes everything\n");
    }

    // 14. submit is inert while IDLE.
    {
        ss_wifi_prov_session_t s;
        memset(&s, 0, sizeof(s));
        CHECK(ss_wifi_prov_submit(&s, kValidSsid, strlen(kValidSsid), kValidPsk, strlen(kValidPsk),
                                  1u) == SS_WIFI_PROV_SUBMIT_IGNORED);
        CHECK(ss_wifi_prov_submit(NULL, kValidSsid, strlen(kValidSsid), kValidPsk,
                                  strlen(kValidPsk), 1u) == SS_WIFI_PROV_SUBMIT_IGNORED);
        CHECK(s.state == SS_WIFI_PROV_IDLE);
        printf("case 14 submit inert outside ACTIVE\n");
    }

    printf("\n%d checks, %d failures\n", g_checks, g_fails);
    return (g_fails == 0) ? 0 : 1;
}
