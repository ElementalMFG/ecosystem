// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// test_ss_wifi_sta_core.c — host harness for the ss_wifi STA policy core.
//
// No Unity dependency: a tiny CHECK macro records failures. Built with
// -fsanitize=address,undefined so any out-of-bounds access to the scan array
// aborts the run. Only ss_wifi_sta_core.c is linked (ss_wifi.c needs the IDF).

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "ss_wifi_sta_core.h"

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

static ss_wifi_ap_t mk(const char* ssid, int8_t rssi, ss_wifi_authmode_t auth, uint8_t ch)
{
    ss_wifi_ap_t ap;
    memset(&ap, 0, sizeof(ap));
    strncpy(ap.ssid, ssid, SS_WIFI_SSID_MAX_LEN);
    ap.ssid[SS_WIFI_SSID_MAX_LEN] = '\0';
    ap.rssi_dbm = rssi;
    ap.authmode = (uint8_t)auth;
    ap.channel = ch;
    return ap;
}

int main(void)
{
    // 1. descending RSSI sort correctness.
    {
        ss_wifi_ap_t aps[4] = {
            mk("a", -80, SS_WIFI_AUTH_WPA2_PSK, 1),
            mk("b", -50, SS_WIFI_AUTH_WPA2_PSK, 6),
            mk("c", -90, SS_WIFI_AUTH_WPA2_PSK, 11),
            mk("d", -65, SS_WIFI_AUTH_WPA2_PSK, 3),
        };
        ss_wifi_scan_sort(aps, 4);
        CHECK(aps[0].rssi_dbm == -50);
        CHECK(aps[1].rssi_dbm == -65);
        CHECK(aps[2].rssi_dbm == -80);
        CHECK(aps[3].rssi_dbm == -90);
        CHECK(strcmp(aps[0].ssid, "b") == 0);
        printf("case 1  descending RSSI sort (strongest first)\n");
    }

    // 2. stability for equal RSSI: ties keep original discovery order.
    {
        ss_wifi_ap_t aps[5] = {
            mk("first", -70, SS_WIFI_AUTH_WPA2_PSK, 1),
            mk("second", -70, SS_WIFI_AUTH_WPA2_PSK, 2),
            mk("strong", -40, SS_WIFI_AUTH_WPA2_PSK, 3),
            mk("third", -70, SS_WIFI_AUTH_WPA2_PSK, 4),
            mk("fourth", -70, SS_WIFI_AUTH_WPA2_PSK, 5),
        };
        ss_wifi_scan_sort(aps, 5);
        CHECK(strcmp(aps[0].ssid, "strong") == 0); // strongest floats to top
        CHECK(strcmp(aps[1].ssid, "first") == 0);
        CHECK(strcmp(aps[2].ssid, "second") == 0);
        CHECK(strcmp(aps[3].ssid, "third") == 0);
        CHECK(strcmp(aps[4].ssid, "fourth") == 0);
        printf("case 2  stable ordering for equal RSSI ties\n");
    }

    // 3. NULL / empty / single-element no-op.
    {
        ss_wifi_scan_sort(NULL, 0); // must not fault
        ss_wifi_scan_sort(NULL, 5); // must not fault (NULL wins)
        ss_wifi_ap_t one[1] = {mk("solo", -55, SS_WIFI_AUTH_WPA3_SAE, 6)};
        ss_wifi_scan_sort(one, 0); // n == 0 no-op
        ss_wifi_scan_sort(one, 1); // single element unchanged
        CHECK(one[0].rssi_dbm == -55);
        CHECK(strcmp(one[0].ssid, "solo") == 0);
        printf("case 3  NULL/empty/single-element no-op\n");
    }

    // 4. auth-mode accept/reject matrix.
    {
        // OPEN AP when a secure minimum is required -> reject.
        CHECK(!ss_wifi_ap_acceptable(SS_WIFI_AUTH_OPEN, false, SS_WIFI_AUTH_WPA2_PSK));
        // OPEN AP with an OPEN minimum and no pass -> accept.
        CHECK(ss_wifi_ap_acceptable(SS_WIFI_AUTH_OPEN, false, SS_WIFI_AUTH_OPEN));
        // WPA2 AP with pass, WPA2 minimum -> accept.
        CHECK(ss_wifi_ap_acceptable(SS_WIFI_AUTH_WPA2_PSK, true, SS_WIFI_AUTH_WPA2_PSK));
        // WPA3 AP with pass, WPA2 minimum -> accept (stronger than required).
        CHECK(ss_wifi_ap_acceptable(SS_WIFI_AUTH_WPA3_SAE, true, SS_WIFI_AUTH_WPA2_PSK));
        // WPA3 AP with pass, WPA3 minimum -> accept.
        CHECK(ss_wifi_ap_acceptable(SS_WIFI_AUTH_WPA3_SAE, true, SS_WIFI_AUTH_WPA3_SAE));
        // WPA2/WPA3 mixed with pass satisfies both a WPA2 and a WPA3 minimum.
        CHECK(ss_wifi_ap_acceptable(SS_WIFI_AUTH_WPA2_WPA3_PSK, true, SS_WIFI_AUTH_WPA2_PSK));
        CHECK(ss_wifi_ap_acceptable(SS_WIFI_AUTH_WPA2_WPA3_PSK, true, SS_WIFI_AUTH_WPA3_SAE));
        // Secure AP with NO passphrase -> reject.
        CHECK(!ss_wifi_ap_acceptable(SS_WIFI_AUTH_WPA2_PSK, false, SS_WIFI_AUTH_WPA2_PSK));
        CHECK(!ss_wifi_ap_acceptable(SS_WIFI_AUTH_WPA3_SAE, false, SS_WIFI_AUTH_WPA2_PSK));
        // WPA2-only AP when WPA3-SAE is required -> reject (too weak).
        CHECK(!ss_wifi_ap_acceptable(SS_WIFI_AUTH_WPA2_PSK, true, SS_WIFI_AUTH_WPA3_SAE));
        // UNKNOWN authmode on either side -> reject.
        CHECK(!ss_wifi_ap_acceptable(SS_WIFI_AUTH_UNKNOWN, true, SS_WIFI_AUTH_WPA2_PSK));
        CHECK(!ss_wifi_ap_acceptable(SS_WIFI_AUTH_WPA2_PSK, true, SS_WIFI_AUTH_UNKNOWN));
        // Out-of-enum cast -> treated as UNKNOWN -> reject.
        CHECK(!ss_wifi_ap_acceptable((ss_wifi_authmode_t)99, true, SS_WIFI_AUTH_WPA2_PSK));
        printf("case 4  auth-mode accept/reject matrix\n");
    }

    // 5. reconnect backoff: exponential, capped, no overflow at large attempt.
    {
        CHECK(ss_wifi_reconnect_backoff_ms(0) == SS_WIFI_RECONNECT_BASE_MS); // 250
        CHECK(ss_wifi_reconnect_backoff_ms(1) == 500u);
        CHECK(ss_wifi_reconnect_backoff_ms(2) == 1000u);
        // 250 << 7 = 32000 == ceiling.
        CHECK(ss_wifi_reconnect_backoff_ms(7) == SS_WIFI_RECONNECT_MAX_MS);
        // Beyond that saturates at the ceiling.
        CHECK(ss_wifi_reconnect_backoff_ms(8) == SS_WIFI_RECONNECT_MAX_MS);
        CHECK(ss_wifi_reconnect_backoff_ms(31) == SS_WIFI_RECONNECT_MAX_MS);
        CHECK(ss_wifi_reconnect_backoff_ms(32) == SS_WIFI_RECONNECT_MAX_MS); // shift guard
        CHECK(ss_wifi_reconnect_backoff_ms(1000000u) == SS_WIFI_RECONNECT_MAX_MS);
        // Monotonic non-decreasing up to the cap.
        uint32_t prev = 0u;
        for (uint32_t a = 0; a < 40u; a++) {
            const uint32_t b = ss_wifi_reconnect_backoff_ms(a);
            CHECK(b >= prev || b == SS_WIFI_RECONNECT_MAX_MS);
            CHECK(b <= SS_WIFI_RECONNECT_MAX_MS);
            prev = b;
        }
        printf("case 5  reconnect backoff exponential+capped, no overflow\n");
    }

    // 6. give-up boundary.
    {
        CHECK(!ss_wifi_reconnect_should_give_up(0));
        CHECK(!ss_wifi_reconnect_should_give_up(SS_WIFI_RECONNECT_MAX_RETRIES - 1u));
        CHECK(ss_wifi_reconnect_should_give_up(SS_WIFI_RECONNECT_MAX_RETRIES));
        CHECK(ss_wifi_reconnect_should_give_up(SS_WIFI_RECONNECT_MAX_RETRIES + 100u));
        printf("case 6  reconnect give-up boundary\n");
    }

    printf("\n%d checks, %d failures\n", g_checks, g_fails);
    return (g_fails != 0) ? 1 : 0;
}
