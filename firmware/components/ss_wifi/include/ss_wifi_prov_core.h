// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_wifi_prov_core.h — pure, host-testable soft-AP captive-provisioning
// session core for S-03-015: per-session AP-passphrase policy, credential
// validation, and the bounded provisioning-session state machine.
//
// SCOPE: this layer holds NO ESP-IDF and NO HAL dependency — it includes only
// <stdint.h>, <stdbool.h>, <stddef.h>. It reasons purely about the session
// lifecycle (begin/submit/tick/end), the secrets it generates (AP passphrase,
// user-verification code), and the credentials it receives (home SSID/PSK),
// so it can be exercised with a plain gcc host harness under
// -fsanitize=address,undefined. The on-target portal glue lives in
// ss_wifi_prov.c and is compiled on-target only: it owns the esp_wifi soft-AP,
// the DNS catch-all, and the HTTP portal, and routes every decision through
// this core.
//
// SECURITY CONTRACT (05_SECURITY_MODEL.md §10.3, standalone path; adversaries
// §1.2 T1 sniffer / T3 active spoofing). This is the contract of record for
// the no-companion first-boot credential handoff:
//
//  1. NEVER-OPEN AP. The provisioning soft-AP always runs WPA2/WPA3-PSK with
//     a passphrase generated per session from caller-supplied entropy
//     (SS_WIFI_PROV_PASS_LEN symbols of a 31-symbol alphabet ≈ 59 bits).
//     The passphrase is shown only on the device display: physical presence
//     is the authorization. A T1 sniffer sees only link-layer AEAD; a T3
//     attacker without the display cannot associate, and an offline attack
//     on a captured WPA2 handshake costs ≥ 2^59 PBKDF2-4096 guesses against
//     a secret that dies with the session.
//  2. FAIL-CLOSED ENTROPY. If the entropy source fails, the session never
//     starts (no fallback to weak or fixed passphrases).
//  3. BOUNDED SESSION. One provisioning window (default 10 min, clamped to
//     [1 min, 30 min]), at most SS_WIFI_PROV_MAX_BAD_SUBMITS rejected
//     submissions, single AP client (enforced by the glue's max_connection).
//     The session terminates on: accepted credentials (after a short grace
//     period so the portal's success page can flush), window expiry, caller
//     cancel, or the power restriction of 08_UNIVERSAL_CONNECTIVITY.md
//     (battery < 15 % ⇒ no AP).
//  4. ONE-SHOT HANDOFF, ALWAYS WIPED. Received credentials are released
//     exactly once via ss_wifi_prov_take_credentials(); every terminal
//     transition and ss_wifi_prov_end() zeroize all secrets held by the
//     session (AP passphrase, verification code, home SSID/PSK). This core
//     never logs and never persists credentials; sealed at-rest storage is
//     deferred to the FS_key store (EPIC-08, follow-up S-03-043).
//  5. EVIL-TWIN CHECK. Alongside the passphrase the core generates a short
//     user-verification code; the genuine portal displays it and the user
//     confirms it matches the device screen. A cloned AP/portal cannot know
//     it. (UX-level mitigation — the passphrase already blocks association
//     with the genuine AP by an attacker.)
//
// TIME MODEL: callers pass a monotonic millisecond clock (now_ms). Deadline
// comparison is wraparound-safe over the full uint32_t range provided the
// session outlives no more than ~24 days of wrap (windows are minutes).
//
// FAIL-SAFE DESIGN: every decision defaults to "do NOT accept / tear down".
// NULL or out-of-range inputs reject; unknown states ignore events; abort
// reasons are sticky so the glue can report why a session died.

#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SS_WIFI_PROV_SSID_MAX_LEN 32u // 802.11 SSID max octets (+1 for NUL in struct)
#define SS_WIFI_PROV_PSK_MAX_LEN 64u  // 63 printable chars, or exactly 64 hex digits
// Generated AP passphrase length: 12 symbols of the 31-char alphabet
// ~= 59.4 bits. Doc 05 §10.3 promises a >= 56-bit floor — do not shrink
// below 12 without amending doc 05.
#define SS_WIFI_PROV_PASS_LEN 12u
#define SS_WIFI_PROV_CODE_LEN 4u                           // user-verification code symbols
#define SS_WIFI_PROV_WINDOW_DEFAULT_MS (10u * 60u * 1000u) // 10 min session window
#define SS_WIFI_PROV_WINDOW_MIN_MS (60u * 1000u)           // clamp floor (1 min)
#define SS_WIFI_PROV_WINDOW_MAX_MS (30u * 60u * 1000u)     // clamp ceiling (30 min)
#define SS_WIFI_PROV_GRACE_MS (3u * 1000u)                 // success-page flush before teardown
#define SS_WIFI_PROV_MAX_BAD_SUBMITS 5u                    // rejected submissions before abort

// Session lifecycle. IDLE is the zero value so a zeroed struct is a valid
// idle session.
typedef enum {
    SS_WIFI_PROV_IDLE = 0, // no session (initial, and after ss_wifi_prov_end)
    SS_WIFI_PROV_ACTIVE,   // AP up, portal serving, waiting for credentials
    SS_WIFI_PROV_GRACE,    // credentials accepted; success page flushing
    SS_WIFI_PROV_DONE,     // grace elapsed; credentials ready to take
    SS_WIFI_PROV_ABORTED,  // terminal failure; see abort_reason
} ss_wifi_prov_state_t;

// Why an ABORTED session died. Sticky until ss_wifi_prov_end().
typedef enum {
    SS_WIFI_PROV_ABORT_NONE = 0,
    SS_WIFI_PROV_ABORT_ENTROPY,     // entropy source failed at begin (fail closed)
    SS_WIFI_PROV_ABORT_TIMEOUT,     // provisioning window expired
    SS_WIFI_PROV_ABORT_CANCEL,      // caller/user cancelled
    SS_WIFI_PROV_ABORT_POWER,       // doc-08 power restriction (battery < 15 %)
    SS_WIFI_PROV_ABORT_BAD_SUBMITS, // too many rejected credential submissions
} ss_wifi_prov_abort_t;

// Result of one credential submission.
typedef enum {
    SS_WIFI_PROV_SUBMIT_ACCEPTED = 0, // valid; session moves to GRACE
    SS_WIFI_PROV_SUBMIT_REJECTED,     // invalid credentials; counted and bounded
    SS_WIFI_PROV_SUBMIT_IGNORED,      // session not ACTIVE (or expired) — no effect
} ss_wifi_prov_submit_t;

/**
 * Caller-supplied entropy source (glue wires the SoC TRNG).
 *
 * Must fill buf[0..len) with cryptographically strong random bytes and return
 * true, or return false on failure (the session then fails closed).
 */
typedef bool (*ss_wifi_prov_entropy_fn)(void* ctx, uint8_t* buf, size_t len);

// Provisioning session. Treat as OPAQUE outside this core and its host tests:
// fields are exposed only so the session can live in static storage without a
// heap. Secrets (ap_pass, code, ssid, psk) are zeroized on every terminal
// transition and by ss_wifi_prov_end().
typedef struct {
    ss_wifi_prov_state_t state;
    ss_wifi_prov_abort_t abort_reason;
    char ap_pass[SS_WIFI_PROV_PASS_LEN + 1u];  // generated AP passphrase (NUL-terminated)
    char code[SS_WIFI_PROV_CODE_LEN + 1u];     // user-verification code (NUL-terminated)
    uint32_t deadline_ms;                      // window expiry (ACTIVE)
    uint32_t grace_deadline_ms;                // teardown time (GRACE)
    uint32_t bad_submits;                      // rejected submissions so far
    bool creds_taken;                          // credentials already released once
    char ssid[SS_WIFI_PROV_SSID_MAX_LEN + 1u]; // accepted home SSID
    char psk[SS_WIFI_PROV_PSK_MAX_LEN + 1u];   // accepted home PSK ("" = open)
} ss_wifi_prov_session_t;

/**
 * Start a provisioning session: generate the AP passphrase and verification
 * code and arm the window deadline.
 *
 * window_ms == 0 selects SS_WIFI_PROV_WINDOW_DEFAULT_MS; any other value is
 * clamped to [SS_WIFI_PROV_WINDOW_MIN_MS, SS_WIFI_PROV_WINDOW_MAX_MS] so the
 * AP lifetime is always bounded.
 *
 * Pre:  s non-NULL; entropy non-NULL; s in any state (a live session is
 *       wiped and restarted).
 * Post/return:
 *   - true: state == ACTIVE; ap_pass/code populated from entropy via
 *     rejection sampling (no modulo bias); deadline armed.
 *   - false: entropy failed or arguments were NULL — state == ABORTED with
 *     ABORT_ENTROPY (when s is non-NULL) and all secrets zeroized. The AP
 *     must NOT be started.
 */
bool ss_wifi_prov_begin(ss_wifi_prov_session_t* s, ss_wifi_prov_entropy_fn entropy,
                        void* entropy_ctx, uint32_t now_ms, uint32_t window_ms);

/**
 * Validate one candidate home-network credential pair.
 *
 * Rules (802.11 / WPA2-Personal):
 *   - SSID: 1..32 octets, no control bytes (0x00..0x1F, 0x7F); bytes >= 0x80
 *     are allowed (UTF-8 SSIDs exist in the wild).
 *   - PSK: empty (open home network), OR 8..63 printable ASCII (0x20..0x7E),
 *     OR exactly 64 hex digits (raw PSK). A 64-char non-hex string is
 *     invalid — the hex rule takes precedence at that length.
 *   - NULL ssid always invalid; NULL psk valid only with psk_len == 0.
 *
 * Pre:  none.
 * Post/return: true iff both fields satisfy the rules above.
 */
bool ss_wifi_prov_creds_valid(const char* ssid, size_t ssid_len, const char* psk, size_t psk_len);

/**
 * Submit credentials received by the portal.
 *
 * Pre:  s non-NULL.
 * Post/return:
 *   - IGNORED if state != ACTIVE, or the window expired at now_ms (the
 *     session then aborts with ABORT_TIMEOUT).
 *   - REJECTED if ss_wifi_prov_creds_valid() fails; after
 *     SS_WIFI_PROV_MAX_BAD_SUBMITS rejections the session aborts with
 *     ABORT_BAD_SUBMITS (secrets zeroized).
 *   - ACCEPTED otherwise: credentials are copied into the session, state
 *     moves to GRACE, and teardown is due at now_ms + SS_WIFI_PROV_GRACE_MS.
 */
ss_wifi_prov_submit_t ss_wifi_prov_submit(ss_wifi_prov_session_t* s, const char* ssid,
                                          size_t ssid_len, const char* psk, size_t psk_len,
                                          uint32_t now_ms);

/**
 * Advance session time. Call periodically (the glue ticks every ~500 ms).
 *
 * Pre:  s non-NULL.
 * Post/return: the state after applying deadlines at now_ms —
 *   - ACTIVE at/past deadline_ms  -> ABORTED (ABORT_TIMEOUT, secrets zeroized).
 *   - GRACE at/past grace_deadline_ms -> DONE (AP passphrase/code zeroized;
 *     accepted credentials retained for ss_wifi_prov_take_credentials()).
 *   - all other states are returned unchanged.
 */
ss_wifi_prov_state_t ss_wifi_prov_tick(ss_wifi_prov_session_t* s, uint32_t now_ms);

/**
 * Cancel a live session (user action). No effect unless state is
 * ACTIVE/GRACE. A GRACE cancel discards the accepted credentials — the
 * handoff has not happened yet.
 *
 * Pre:  s non-NULL.
 * Post: state == ABORTED (ABORT_CANCEL), all secrets zeroized.
 */
void ss_wifi_prov_cancel(ss_wifi_prov_session_t* s);

/**
 * Abort a live session because the doc-08 power restriction engaged
 * (battery < 15 % ⇒ no AP). Semantics as ss_wifi_prov_cancel() with
 * ABORT_POWER.
 */
void ss_wifi_prov_power_restrict(ss_wifi_prov_session_t* s);

/**
 * Release the accepted credentials — exactly once.
 *
 * Pre:  s non-NULL; ssid_out capacity >= SS_WIFI_PROV_SSID_MAX_LEN + 1;
 *       psk_out capacity >= SS_WIFI_PROV_PSK_MAX_LEN + 1 (the ss_wifi_cfg_t
 *       ssid[33]/pass[65] fields satisfy this).
 * Post/return:
 *   - true only when state == DONE and credentials were not yet taken:
 *     NUL-terminated SSID/PSK copied out (PSK "" = open network) and the
 *     session's internal copies zeroized (single release).
 *   - false otherwise; output buffers are zeroed either way when non-NULL
 *     with the required capacity.
 */
bool ss_wifi_prov_take_credentials(ss_wifi_prov_session_t* s, char* ssid_out, size_t ssid_cap,
                                   char* psk_out, size_t psk_cap);

/**
 * End a session in any state: zeroize every secret and return to IDLE.
 * Always safe to call (NULL is a no-op).
 */
void ss_wifi_prov_end(ss_wifi_prov_session_t* s);

#ifdef __cplusplus
} // extern "C"
#endif
