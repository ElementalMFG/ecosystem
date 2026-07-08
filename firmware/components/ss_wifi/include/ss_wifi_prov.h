// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_wifi_prov.h — on-target captive-provisioning portal (S-03-015).
//
// Public surface of the ESP-IDF glue around the pure session core
// (ss_wifi_prov_core.h — the security contract of record). The portal owns,
// for the lifetime of one session: the soft-AP (WPA2/WPA3-PSK, per-session
// passphrase, max 1 client), a DNS catch-all so phone captive-portal probes
// land on the portal, and the HTTP portal itself. On any terminal state the
// whole stack tears down and Wi-Fi returns to STA mode (AC: the soft-AP
// shuts down after provisioning completes).
//
// OWNERSHIP / PRECONDITIONS:
//   - ss_wifi_init() must have run; the STA must NOT be started
//     (first-boot flow — call ss_wifi_stop() first when re-provisioning).
//   - The caller enforces the 08_UNIVERSAL_CONNECTIVITY.md power rule:
//     do not start the portal below 15 % battery, and call
//     ss_wifi_prov_portal_power_restrict() if the restriction engages
//     mid-session (a dedicated ss_power event hook is future work).
//   - Credentials are delivered exactly once via the done callback and
//     wiped immediately after it returns. The callee applies them
//     (ss_wifi_config()/ss_wifi_start()); nothing is persisted here
//     (sealed at-rest storage is EPIC-08 / S-03-043).

#pragma once
#include <stdint.h>

#include "esp_err.h"
#include "ss_hal_radio_wifi.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Shows the provisioning secrets to the user (device display).
 * All strings are NUL-terminated and owned by the caller of the callback;
 * copy what must outlive the call. Never log or transmit ap_pass. Runs on
 * the task that called ss_wifi_prov_portal_start(); keep it short and do
 * not call back into ss_wifi_prov_portal_* from it.
 */
typedef void (*ss_wifi_prov_display_fn)(const char* ap_ssid, const char* ap_pass,
                                        const char* verify_code);

/**
 * Terminal-state callback. success == true: sta_cfg holds the handed-off
 * credentials (sta_mode set, ap_mode clear) and is valid ONLY during the
 * call — it is wiped when the callback returns. success == false: sta_cfg
 * is NULL (window expired, cancelled, power-restricted, or too many bad
 * submissions). Runs from an internal task; keep it short and do not call
 * back into ss_wifi_prov_portal_* from it.
 */
typedef void (*ss_wifi_prov_done_fn)(bool success, const ss_wifi_cfg_t* sta_cfg);

/**
 * Start one provisioning session: bring up the soft-AP + DNS + portal.
 *
 * window_ms == 0 selects the core default (10 min); values are clamped by
 * the core to [1 min, 30 min].
 *
 * Pre:  ss_wifi_init() done; STA not started; no session already active;
 *       display_cb and done_cb non-NULL. The very first call allocates the
 *       module's sync objects and must not race a concurrent first call
 *       from another task (the first-boot flow has a single owner).
 * Post/return: ESP_OK and the session is live (display_cb already invoked
 * with the AP SSID/passphrase/verify code), or an error and nothing is
 * running (fail closed — including entropy failure, ESP_FAIL).
 */
esp_err_t ss_wifi_prov_portal_start(ss_wifi_prov_display_fn display_cb,
                                    ss_wifi_prov_done_fn done_cb, uint32_t window_ms);

/**
 * Cancel the active session (user action). Teardown is asynchronous; the
 * done callback fires with success == false. ESP_ERR_INVALID_STATE when no
 * session is active.
 */
esp_err_t ss_wifi_prov_portal_cancel(void);

/**
 * Abort the active session because the doc-08 power restriction engaged
 * (battery < 15 % ⇒ no AP). Same teardown semantics as cancel.
 */
esp_err_t ss_wifi_prov_portal_power_restrict(void);

#ifdef __cplusplus
} // extern "C"
#endif
