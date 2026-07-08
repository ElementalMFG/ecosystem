// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_wifi_sta_core.h — pure, host-testable Wi-Fi STA policy core for S-03-014:
// scan-result ordering, WPA2/WPA3 auth-mode selection, and bounded reconnect
// backoff.
//
// SCOPE: this layer holds NO ESP-IDF and NO HAL dependency — it includes only
// <stdint.h>, <stdbool.h>, <stddef.h>. It reasons purely about scan-result
// records (SSID/RSSI/authmode/channel), the security policy of whether a given
// AP is acceptable to associate with, and caller-supplied retry-attempt counts,
// so it can be exercised with a plain gcc host harness under
// -fsanitize=address,undefined with no include hacks and no libc qsort. The
// on-target STA driver glue lives in ss_wifi.c and is compiled on-target only:
// it feeds esp_wifi scan records into ss_wifi_scan_sort(), consults
// ss_wifi_ap_acceptable() before ss_wifi_connect(), and drives its
// disconnect/reconnect loop with ss_wifi_reconnect_backoff_ms() /
// ss_wifi_reconnect_should_give_up().
//
// AUTH-MODE MODEL: Wi-Fi STA association must never silently downgrade security.
// The authmode enum is ordered by increasing security strength, with OPEN at 0
// and UNKNOWN as a distinct terminal value that is always rejected. A join is
// gated by three inputs: the AP's advertised authmode, whether the operator has
// configured a passphrase, and the minimum auth mode the caller requires. A
// secure network with no configured passphrase is rejected (we cannot join it);
// an AP weaker than the required minimum is rejected; an OPEN AP is rejected
// whenever any secured minimum is required. WPA2/WPA3 transition (mixed) mode is
// accepted for a WPA2 or WPA3 minimum because a WPA3-capable station negotiates
// SAE while remaining backward compatible.
//
// FAIL-SAFE DESIGN: every decision defaults to "do NOT join / do NOT continue".
// Unknown authmode, NULL/zero-length inputs, and the give-up boundary all resolve
// to the safe (reject / stop) side. ss_wifi_scan_sort() is a no-op on NULL or
// n == 0 so a caller that mishandles an empty scan cannot fault.
//
// STABLE ORDERING: ss_wifi_scan_sort() sorts strongest-RSSI-first and is STABLE
// for equal RSSI (APs with the same signal strength retain their scan-discovery
// order). This keeps UI listings deterministic across rescans and avoids a
// libc qsort dependency (which is not guaranteed stable) — a simple in-place
// insertion sort is used instead.
//
// BOUNDED RECONNECT: ss_wifi_reconnect_backoff_ms() is an exponential backoff
// (base << attempt) capped at a ceiling, with the shift guarded so a large
// attempt count can never overflow. ss_wifi_reconnect_should_give_up() bounds
// the number of retries so a permanently-unreachable AP does not spin the STA
// forever (the driver then surfaces a disconnected state to the app).

#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SS_WIFI_SSID_MAX_LEN 32u         // 802.11 SSID max octets (+1 for NUL in struct)
#define SS_WIFI_RECONNECT_BASE_MS 250u   // first reconnect backoff step
#define SS_WIFI_RECONNECT_MAX_MS 32000u  // backoff ceiling (32 s)
#define SS_WIFI_RECONNECT_MAX_RETRIES 8u // give up after this many attempts

// Advertised/required authentication mode, ORDERED by increasing security
// strength. UNKNOWN is a distinct terminal value that is always rejected.
// Fail-safe: an unrecognised on-air authmode maps to UNKNOWN, never to OPEN.
typedef enum {
    SS_WIFI_AUTH_OPEN = 0,      // no security (rejected whenever a secured min is required)
    SS_WIFI_AUTH_WPA2_PSK,      // WPA2-Personal (AES-CCMP)
    SS_WIFI_AUTH_WPA2_WPA3_PSK, // WPA2/WPA3 mixed transition mode
    SS_WIFI_AUTH_WPA3_SAE,      // WPA3-Personal (SAE)
    SS_WIFI_AUTH_UNKNOWN,       // unrecognised / unsupported -> always rejected
} ss_wifi_authmode_t;

// One scan result. `ssid` is always NUL-terminated (max 32 octets + NUL).
typedef struct {
    char ssid[SS_WIFI_SSID_MAX_LEN + 1u];
    int8_t rssi_dbm;  // received signal strength (dBm, typically negative)
    uint8_t authmode; // one of ss_wifi_authmode_t
    uint8_t channel;  // 802.11 channel number
} ss_wifi_ap_t;

/**
 * In-place STABLE descending sort of scan results by rssi_dbm (strongest first).
 *
 * Uses a simple insertion sort (no libc qsort): elements with equal rssi_dbm
 * retain their original relative order, so repeated scans of the same
 * environment produce a deterministic listing.
 *
 * Pre:  aps points to n valid records, or (aps == NULL) / (n == 0).
 * Post: on NULL/0 this is a no-op; otherwise aps[0..n) is ordered so that
 *       aps[i].rssi_dbm >= aps[i+1].rssi_dbm, ties keeping insertion order.
 */
void ss_wifi_scan_sort(ss_wifi_ap_t* aps, size_t n);

/**
 * Whether the station may associate with an AP under the security policy.
 *
 * Fail-safe: any doubt resolves to false (do NOT join).
 *
 * Pre:  none.
 * Post/return:
 *   - false if ap_authmode or min_required is SS_WIFI_AUTH_UNKNOWN (or any value
 *     outside the enum) — an unknown authmode is never joined.
 *   - false if min_required is a secured mode (> OPEN) but ap_authmode == OPEN —
 *     never downgrade to an open network when security is required.
 *   - false if the AP is a secured network (ap_authmode > OPEN) but have_pass is
 *     false — we cannot authenticate without a passphrase.
 *   - false if ap_authmode is strictly weaker than min_required (e.g. WPA2-only
 *     AP when WPA3-SAE is required). WPA2/WPA3 mixed satisfies a WPA2 or WPA3
 *     minimum because a WPA3 station negotiates SAE against a transition AP.
 *   - true  otherwise.
 */
bool ss_wifi_ap_acceptable(ss_wifi_authmode_t ap_authmode, bool have_pass,
                           ss_wifi_authmode_t min_required);

/**
 * Exponential reconnect backoff for a retry attempt.
 *
 * Pre:  none.
 * Post/return: SS_WIFI_RECONNECT_BASE_MS << attempt, capped at
 * SS_WIFI_RECONNECT_MAX_MS. Large `attempt` values saturate at the ceiling
 * (the shift is guarded so it can never overflow).
 */
uint32_t ss_wifi_reconnect_backoff_ms(uint32_t attempt);

/**
 * Whether reconnect retries are exhausted for `attempt`.
 *
 * Pre:  none.
 * Post/return: attempt >= SS_WIFI_RECONNECT_MAX_RETRIES.
 */
bool ss_wifi_reconnect_should_give_up(uint32_t attempt);

#ifdef __cplusplus
} // extern "C"
#endif
