// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_wifi_sta_core.c — pure Wi-Fi STA policy: scan-result ordering, auth-mode
// selection, and bounded reconnect backoff. See ss_wifi_sta_core.h for scope,
// the auth-mode model, the fail-safe rationale, the stable-sort note, and the
// bounded-reconnect design. No ESP-IDF or HAL dependency lives here; all math is
// integer (no libm) and no libc qsort is used.

#include "ss_wifi_sta_core.h"

void ss_wifi_scan_sort(ss_wifi_ap_t* aps, size_t n)
{
    if (aps == NULL || n < 2u) { return; }

    // Insertion sort: stable and in-place. Descending by rssi_dbm. An element is
    // shifted left only while its predecessor is STRICTLY weaker, so equal-RSSI
    // records keep their original relative order (stability).
    for (size_t i = 1u; i < n; i++) {
        const ss_wifi_ap_t key = aps[i];
        size_t j = i;
        while (j > 0u && aps[j - 1u].rssi_dbm < key.rssi_dbm) {
            aps[j] = aps[j - 1u];
            j--;
        }
        aps[j] = key;
    }
}

// True if `m` is a value the enum defines (guards against out-of-range casts).
static bool authmode_known(ss_wifi_authmode_t m)
{
    switch (m) {
    case SS_WIFI_AUTH_OPEN:
    case SS_WIFI_AUTH_WPA2_PSK:
    case SS_WIFI_AUTH_WPA2_WPA3_PSK:
    case SS_WIFI_AUTH_WPA3_SAE:
        return true;
    case SS_WIFI_AUTH_UNKNOWN:
    default:
        return false;
    }
}

// Security rank used for the "at least as strong as the required minimum" test.
// The enum's ordinal order is NOT usable directly because WPA2/WPA3 mixed sits
// between WPA2 and WPA3 numerically yet, for a WPA3-capable station, is as strong
// as WPA3-SAE (the station negotiates SAE against a transition AP). Mixed is
// therefore ranked equal to WPA3-SAE. Assumes `m` is a known authmode.
static uint8_t authmode_rank(ss_wifi_authmode_t m)
{
    switch (m) {
    case SS_WIFI_AUTH_OPEN:
        return 0u;
    case SS_WIFI_AUTH_WPA2_PSK:
        return 1u;
    case SS_WIFI_AUTH_WPA2_WPA3_PSK:
    case SS_WIFI_AUTH_WPA3_SAE:
        return 2u;
    case SS_WIFI_AUTH_UNKNOWN:
    default:
        return 0u; // unreachable: callers gate on authmode_known first
    }
}

bool ss_wifi_ap_acceptable(ss_wifi_authmode_t ap_authmode, bool have_pass,
                           ss_wifi_authmode_t min_required)
{
    // (a) fail-safe: an unknown/unsupported authmode on either side is never joined.
    if (!authmode_known(ap_authmode) || !authmode_known(min_required)) { return false; }

    // (b) never downgrade: a secured minimum forbids an OPEN AP.
    if (min_required > SS_WIFI_AUTH_OPEN && ap_authmode == SS_WIFI_AUTH_OPEN) { return false; }

    // (c) a secured AP requires a configured passphrase to authenticate.
    if (ap_authmode > SS_WIFI_AUTH_OPEN && !have_pass) { return false; }

    // (d) the AP must be at least as strong as the required minimum (by security
    // rank, not raw enum order). WPA2/WPA3 mixed ranks equal to WPA3-SAE, so it
    // satisfies both a WPA2 and a WPA3 minimum: a WPA3-capable station negotiates
    // SAE against a transition AP.
    if (authmode_rank(ap_authmode) < authmode_rank(min_required)) { return false; }

    return true;
}

uint32_t ss_wifi_reconnect_backoff_ms(uint32_t attempt)
{
    // Guard the shift first so BASE << attempt can never overflow, then clamp.
    if (attempt >= 32u) { return SS_WIFI_RECONNECT_MAX_MS; }
    const uint32_t backoff = SS_WIFI_RECONNECT_BASE_MS << attempt;
    if (backoff > SS_WIFI_RECONNECT_MAX_MS || backoff < SS_WIFI_RECONNECT_BASE_MS) {
        return SS_WIFI_RECONNECT_MAX_MS;
    }
    return backoff;
}

bool ss_wifi_reconnect_should_give_up(uint32_t attempt)
{
    return attempt >= SS_WIFI_RECONNECT_MAX_RETRIES;
}
