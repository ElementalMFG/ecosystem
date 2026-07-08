// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_wifi_prov_core.c — pure soft-AP captive-provisioning session core
// (S-03-015). See the security contract in ss_wifi_prov_core.h; this file
// holds no ESP-IDF and no HAL dependency.

#include "ss_wifi_prov_core.h"

// 31 unambiguous symbols (no 0/o, 1/l/i): 12 symbols ~= 59.4 bits.
static const char ALPHABET[] = "23456789abcdefghjkmnpqrstuvwxyz";
#define ALPHABET_LEN 31u
_Static_assert(sizeof(ALPHABET) - 1u == ALPHABET_LEN,
               "alphabet length drives the entropy claim and REJECT_BOUND");
// Rejection-sampling acceptance bound: largest multiple of 31 <= 256, so
// (byte % 31) is unbiased for accepted bytes.
#define REJECT_BOUND 248u
// Refill cap, per draw_symbols() call: the longest draw (the 12-symbol
// passphrase) expects ~13 bytes at the 1/32 rejection rate; 8 x 16-byte
// blocks makes exhaustion astronomically unlikely while bounding the loop.
#define ENTROPY_MAX_REFILLS 8u
#define ENTROPY_BLOCK_LEN 16u

// memset can be elided on objects the compiler proves dead; a volatile
// write loop cannot. All secret wipes go through here.
static void secure_wipe(void* p, size_t n)
{
    volatile uint8_t* v = (volatile uint8_t*)p;
    for (size_t i = 0; i < n; i++) { v[i] = 0u; }
}

// Wraparound-safe "now has reached deadline" over uint32_t millisecond
// clocks (valid while |now - deadline| < 2^31 ms ~ 24.8 days; windows are
// minutes).
static bool time_reached(uint32_t now_ms, uint32_t deadline_ms)
{
    return (uint32_t)(now_ms - deadline_ms) < 0x80000000u;
}

static void wipe_secrets(ss_wifi_prov_session_t* s)
{
    secure_wipe(s->ap_pass, sizeof(s->ap_pass));
    secure_wipe(s->code, sizeof(s->code));
    secure_wipe(s->ssid, sizeof(s->ssid));
    secure_wipe(s->psk, sizeof(s->psk));
}

static void abort_session(ss_wifi_prov_session_t* s, ss_wifi_prov_abort_t reason)
{
    wipe_secrets(s);
    s->state = SS_WIFI_PROV_ABORTED;
    s->abort_reason = reason;
}

// Draw n_sym alphabet symbols into out (out capacity must be n_sym + 1) via
// rejection sampling. Fails closed: false on entropy failure or refill
// exhaustion, with out wiped.
static bool draw_symbols(ss_wifi_prov_entropy_fn entropy, void* ectx, char* out, size_t n_sym)
{
    size_t filled = 0;
    for (unsigned refill = 0; refill < ENTROPY_MAX_REFILLS && filled < n_sym; refill++) {
        uint8_t block[ENTROPY_BLOCK_LEN];
        if (!entropy(ectx, block, sizeof(block))) {
            secure_wipe(block, sizeof(block));
            secure_wipe(out, n_sym + 1u);
            return false;
        }
        for (size_t i = 0; i < sizeof(block) && filled < n_sym; i++) {
            if (block[i] < REJECT_BOUND) { out[filled++] = ALPHABET[block[i] % ALPHABET_LEN]; }
        }
        secure_wipe(block, sizeof(block));
    }
    if (filled < n_sym) {
        secure_wipe(out, n_sym + 1u);
        return false;
    }
    out[n_sym] = '\0';
    return true;
}

bool ss_wifi_prov_begin(ss_wifi_prov_session_t* s, ss_wifi_prov_entropy_fn entropy,
                        void* entropy_ctx, uint32_t now_ms, uint32_t window_ms)
{
    if (s == NULL) { return false; }
    // Restarting wipes any prior session state first.
    wipe_secrets(s);
    s->bad_submits = 0u;
    s->creds_taken = false;
    s->abort_reason = SS_WIFI_PROV_ABORT_NONE;

    if (entropy == NULL) {
        abort_session(s, SS_WIFI_PROV_ABORT_ENTROPY);
        return false;
    }
    if (!draw_symbols(entropy, entropy_ctx, s->ap_pass, SS_WIFI_PROV_PASS_LEN) ||
        !draw_symbols(entropy, entropy_ctx, s->code, SS_WIFI_PROV_CODE_LEN)) {
        abort_session(s, SS_WIFI_PROV_ABORT_ENTROPY);
        return false;
    }

    uint32_t w = (window_ms == 0u) ? SS_WIFI_PROV_WINDOW_DEFAULT_MS : window_ms;
    if (w < SS_WIFI_PROV_WINDOW_MIN_MS) { w = SS_WIFI_PROV_WINDOW_MIN_MS; }
    if (w > SS_WIFI_PROV_WINDOW_MAX_MS) { w = SS_WIFI_PROV_WINDOW_MAX_MS; }
    s->deadline_ms = now_ms + w;
    s->grace_deadline_ms = 0u;
    s->state = SS_WIFI_PROV_ACTIVE;
    return true;
}

static bool psk_is_hex64(const char* psk)
{
    for (size_t i = 0; i < 64u; i++) {
        const char c = psk[i];
        const bool hex = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
        if (!hex) { return false; }
    }
    return true;
}

bool ss_wifi_prov_creds_valid(const char* ssid, size_t ssid_len, const char* psk, size_t psk_len)
{
    if (ssid == NULL || ssid_len == 0u || ssid_len > SS_WIFI_PROV_SSID_MAX_LEN) { return false; }
    for (size_t i = 0; i < ssid_len; i++) {
        const uint8_t b = (uint8_t)ssid[i];
        // Reject control bytes (incl. NUL — the handoff struct is
        // NUL-terminated) and DEL; allow >= 0x80 for UTF-8 SSIDs.
        if (b < 0x20u || b == 0x7Fu) { return false; }
    }

    if (psk_len == 0u) { return true; } // open home network
    if (psk == NULL) { return false; }
    if (psk_len == 64u) { return psk_is_hex64(psk); } // raw-PSK hex form only
    if (psk_len < 8u || psk_len > 63u) { return false; }
    for (size_t i = 0; i < psk_len; i++) {
        const uint8_t b = (uint8_t)psk[i];
        if (b < 0x20u || b > 0x7Eu) { return false; } // 802.11 passphrase: printable ASCII
    }
    return true;
}

ss_wifi_prov_submit_t ss_wifi_prov_submit(ss_wifi_prov_session_t* s, const char* ssid,
                                          size_t ssid_len, const char* psk, size_t psk_len,
                                          uint32_t now_ms)
{
    if (s == NULL || s->state != SS_WIFI_PROV_ACTIVE) { return SS_WIFI_PROV_SUBMIT_IGNORED; }
    if (time_reached(now_ms, s->deadline_ms)) {
        abort_session(s, SS_WIFI_PROV_ABORT_TIMEOUT);
        return SS_WIFI_PROV_SUBMIT_IGNORED;
    }

    if (!ss_wifi_prov_creds_valid(ssid, ssid_len, psk, psk_len)) {
        s->bad_submits++;
        if (s->bad_submits >= SS_WIFI_PROV_MAX_BAD_SUBMITS) {
            abort_session(s, SS_WIFI_PROV_ABORT_BAD_SUBMITS);
        }
        return SS_WIFI_PROV_SUBMIT_REJECTED;
    }

    // Validated lengths are within the fixed buffers; copy and terminate.
    for (size_t i = 0; i < ssid_len; i++) { s->ssid[i] = ssid[i]; }
    s->ssid[ssid_len] = '\0';
    for (size_t i = 0; i < psk_len; i++) { s->psk[i] = psk[i]; }
    s->psk[psk_len] = '\0';

    s->grace_deadline_ms = now_ms + SS_WIFI_PROV_GRACE_MS;
    s->state = SS_WIFI_PROV_GRACE;
    return SS_WIFI_PROV_SUBMIT_ACCEPTED;
}

ss_wifi_prov_state_t ss_wifi_prov_tick(ss_wifi_prov_session_t* s, uint32_t now_ms)
{
    if (s == NULL) { return SS_WIFI_PROV_IDLE; }
    if (s->state == SS_WIFI_PROV_ACTIVE && time_reached(now_ms, s->deadline_ms)) {
        abort_session(s, SS_WIFI_PROV_ABORT_TIMEOUT);
    } else if (s->state == SS_WIFI_PROV_GRACE && time_reached(now_ms, s->grace_deadline_ms)) {
        // The AP secret is done working; only the accepted credentials
        // survive into DONE, for the single take_credentials() release.
        secure_wipe(s->ap_pass, sizeof(s->ap_pass));
        secure_wipe(s->code, sizeof(s->code));
        s->state = SS_WIFI_PROV_DONE;
    }
    return s->state;
}

void ss_wifi_prov_cancel(ss_wifi_prov_session_t* s)
{
    if (s == NULL) { return; }
    if (s->state == SS_WIFI_PROV_ACTIVE || s->state == SS_WIFI_PROV_GRACE) {
        abort_session(s, SS_WIFI_PROV_ABORT_CANCEL);
    }
}

void ss_wifi_prov_power_restrict(ss_wifi_prov_session_t* s)
{
    if (s == NULL) { return; }
    if (s->state == SS_WIFI_PROV_ACTIVE || s->state == SS_WIFI_PROV_GRACE) {
        abort_session(s, SS_WIFI_PROV_ABORT_POWER);
    }
}

bool ss_wifi_prov_take_credentials(ss_wifi_prov_session_t* s, char* ssid_out, size_t ssid_cap,
                                   char* psk_out, size_t psk_cap)
{
    const bool caps_ok = (ssid_out != NULL) && (ssid_cap >= SS_WIFI_PROV_SSID_MAX_LEN + 1u) &&
                         (psk_out != NULL) && (psk_cap >= SS_WIFI_PROV_PSK_MAX_LEN + 1u);
    if (caps_ok) {
        secure_wipe(ssid_out, ssid_cap);
        secure_wipe(psk_out, psk_cap);
    }
    if (s == NULL || !caps_ok) { return false; }
    if (s->state != SS_WIFI_PROV_DONE || s->creds_taken) { return false; }

    for (size_t i = 0; i < sizeof(s->ssid); i++) { ssid_out[i] = s->ssid[i]; }
    for (size_t i = 0; i < sizeof(s->psk); i++) { psk_out[i] = s->psk[i]; }
    secure_wipe(s->ssid, sizeof(s->ssid));
    secure_wipe(s->psk, sizeof(s->psk));
    s->creds_taken = true;
    return true;
}

void ss_wifi_prov_end(ss_wifi_prov_session_t* s)
{
    if (s == NULL) { return; }
    wipe_secrets(s);
    s->deadline_ms = 0u;
    s->grace_deadline_ms = 0u;
    s->bad_submits = 0u;
    s->creds_taken = false;
    s->abort_reason = SS_WIFI_PROV_ABORT_NONE;
    s->state = SS_WIFI_PROV_IDLE;
}
