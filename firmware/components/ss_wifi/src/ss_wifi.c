// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_wifi.c — ESP-IDF glue implementing the frozen ss_hal_radio_wifi.h STA
// lifecycle (init/config/start/stop/sleep) against esp_wifi/esp_netif/esp_event.
//
// The security and retry policy is factored out into the pure, host-tested core
// (ss_wifi_sta_core.c): scan results are ordered with ss_wifi_scan_sort(), the
// candidate AP is gated by ss_wifi_ap_acceptable() before association, and the
// disconnect handler backs off with ss_wifi_reconnect_backoff_ms() and stops at
// ss_wifi_reconnect_should_give_up(). This file owns only the ESP-IDF wiring;
// all decisions live in the core so they can be exercised without hardware.
//
// This is a well-structured first cut of the on-target path (it cannot run
// without hardware). It compiles under ESP-IDF v5.3.5.

#include "ss_hal_radio_wifi.h"

#include <string.h>

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"

#include "ss_wifi_sta_core.h"

static const char* TAG = "ss_wifi";

#define SS_WIFI_SCAN_MAX_AP 16u

// Cohesive module state. All access is from the caller task and the default
// event loop, which are serialised on this component's public API.
typedef struct {
    bool inited;                     // esp_wifi_init + netif + handlers registered
    bool started;                    // esp_wifi_start succeeded
    bool have_cfg;                   // a valid ss_wifi_config() has been applied
    ss_wifi_cfg_t cfg;               // last applied configuration
    ss_wifi_authmode_t min_authmode; // minimum acceptable auth mode for joins
    uint32_t reconnect_attempt;      // consecutive failed (re)connect attempts
    esp_netif_t* sta_netif;          // default STA netif handle
} ss_wifi_ctx_t;

static ss_wifi_ctx_t s_ctx;

// Map an esp_wifi authmode to the core's fail-safe policy enum. Anything we do
// not explicitly support maps to UNKNOWN so the core rejects the join.
static ss_wifi_authmode_t map_authmode(wifi_auth_mode_t m)
{
    switch (m) {
    case WIFI_AUTH_OPEN:
        return SS_WIFI_AUTH_OPEN;
    case WIFI_AUTH_WPA2_PSK:
    case WIFI_AUTH_WPA2_ENTERPRISE:
        return SS_WIFI_AUTH_WPA2_PSK;
    case WIFI_AUTH_WPA2_WPA3_PSK:
        return SS_WIFI_AUTH_WPA2_WPA3_PSK;
    case WIFI_AUTH_WPA3_PSK:
        return SS_WIFI_AUTH_WPA3_SAE;
    default:
        return SS_WIFI_AUTH_UNKNOWN;
    }
}

static void handle_disconnect(void)
{
    if (ss_wifi_reconnect_should_give_up(s_ctx.reconnect_attempt)) {
        ESP_LOGW(TAG, "reconnect gave up after %u attempts", (unsigned)s_ctx.reconnect_attempt);
        return;
    }
    const uint32_t backoff_ms = ss_wifi_reconnect_backoff_ms(s_ctx.reconnect_attempt);
    s_ctx.reconnect_attempt++;
    ESP_LOGI(TAG, "reconnect attempt %u in %u ms", (unsigned)s_ctx.reconnect_attempt,
             (unsigned)backoff_ms);
    vTaskDelay(pdMS_TO_TICKS(backoff_ms));
    (void)esp_wifi_connect();
}

static void wifi_event_handler(void* arg, esp_event_base_t base, int32_t id, void* data)
{
    (void)arg;
    (void)data;
    if (base != WIFI_EVENT) { return; }

    switch (id) {
    case WIFI_EVENT_STA_START:
        (void)esp_wifi_connect();
        break;
    case WIFI_EVENT_STA_CONNECTED:
        s_ctx.reconnect_attempt = 0u; // success resets the backoff schedule
        break;
    case WIFI_EVENT_STA_DISCONNECTED:
        handle_disconnect();
        break;
    default:
        break;
    }
}

esp_err_t ss_wifi_init(void)
{
    if (s_ctx.inited) { return ESP_OK; }

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    if (err != ESP_OK) { return err; }

    err = esp_netif_init();
    if (err != ESP_OK) { return err; }

    err = esp_event_loop_create_default();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) { return err; }

    s_ctx.sta_netif = esp_netif_create_default_wifi_sta();
    if (s_ctx.sta_netif == NULL) { return ESP_FAIL; }

    wifi_init_config_t init_cfg = WIFI_INIT_CONFIG_DEFAULT();
    err = esp_wifi_init(&init_cfg);
    if (err != ESP_OK) { return err; }

    // Doc 05 §10.3: never store Wi-Fi PSKs plaintext. esp_wifi defaults to
    // WIFI_STORAGE_FLASH, which writes every set_config (STA creds AND the
    // provisioning AP passphrase) to plaintext NVS. Force RAM-only storage
    // before ANY esp_wifi_set_config call; persistence at rest is owned by
    // the sealed store (S-03-043, EPIC-08).
    err = esp_wifi_set_storage(WIFI_STORAGE_RAM);
    if (err != ESP_OK) { return err; }

    err = esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler,
                                              NULL, NULL);
    if (err != ESP_OK) { return err; }

    err = esp_wifi_set_mode(WIFI_MODE_STA);
    if (err != ESP_OK) { return err; }

    s_ctx.min_authmode = SS_WIFI_AUTH_WPA2_PSK; // secure-by-default minimum
    s_ctx.reconnect_attempt = 0u;
    s_ctx.inited = true;
    return ESP_OK;
}

esp_err_t ss_wifi_config(const ss_wifi_cfg_t* cfg)
{
    if (cfg == NULL) { return ESP_ERR_INVALID_ARG; }
    if (!s_ctx.inited) { return ESP_ERR_INVALID_STATE; }
    if (cfg->ssid[0] == '\0') { return ESP_ERR_INVALID_ARG; }

    wifi_config_t wc;
    memset(&wc, 0, sizeof(wc));
    // ssid/pass buffers in ss_wifi_cfg_t are NUL-terminated and no larger than
    // the esp_wifi fields, so a bounded copy is safe.
    strncpy((char*)wc.sta.ssid, cfg->ssid, sizeof(wc.sta.ssid));
    strncpy((char*)wc.sta.password, cfg->pass, sizeof(wc.sta.password));

    const bool have_pass = (cfg->pass[0] != '\0');
    // Require at least the module minimum; an open join is only allowed when no
    // passphrase is set (the core still rejects open when a secure min applies).
    wc.sta.threshold.authmode = have_pass ? WIFI_AUTH_WPA2_PSK : WIFI_AUTH_OPEN;
    wc.sta.pmf_cfg.capable = true; // enable PMF for WPA3/SAE negotiation
    wc.sta.pmf_cfg.required = false;

    esp_err_t err = esp_wifi_set_config(WIFI_IF_STA, &wc);
    if (err != ESP_OK) { return err; }

    s_ctx.cfg = *cfg;
    s_ctx.min_authmode = have_pass ? SS_WIFI_AUTH_WPA2_PSK : SS_WIFI_AUTH_OPEN;
    s_ctx.have_cfg = true;
    return ESP_OK;
}

// Perform a blocking scan, sort results strongest-first via the core, and log
// the acceptable candidates. Returns ESP_OK on a completed scan.
static esp_err_t ss_wifi_scan_and_rank(void)
{
    esp_err_t err = esp_wifi_scan_start(NULL, true);
    if (err != ESP_OK) { return err; }

    uint16_t found = SS_WIFI_SCAN_MAX_AP;
    wifi_ap_record_t records[SS_WIFI_SCAN_MAX_AP];
    err = esp_wifi_scan_get_ap_records(&found, records);
    if (err != ESP_OK) { return err; }

    ss_wifi_ap_t aps[SS_WIFI_SCAN_MAX_AP];
    const size_t n = (found < SS_WIFI_SCAN_MAX_AP) ? found : SS_WIFI_SCAN_MAX_AP;
    for (size_t i = 0; i < n; i++) {
        memset(&aps[i], 0, sizeof(aps[i]));
        strncpy(aps[i].ssid, (const char*)records[i].ssid, SS_WIFI_SSID_MAX_LEN);
        aps[i].ssid[SS_WIFI_SSID_MAX_LEN] = '\0';
        aps[i].rssi_dbm = records[i].rssi;
        aps[i].authmode = (uint8_t)map_authmode(records[i].authmode);
        aps[i].channel = records[i].primary;
    }

    ss_wifi_scan_sort(aps, n);

    const bool have_pass = s_ctx.have_cfg && (s_ctx.cfg.pass[0] != '\0');
    for (size_t i = 0; i < n; i++) {
        const bool ok = ss_wifi_ap_acceptable((ss_wifi_authmode_t)aps[i].authmode, have_pass,
                                              s_ctx.min_authmode);
        ESP_LOGI(TAG, "scan[%u] \"%s\" rssi=%d ch=%u auth=%u %s", (unsigned)i, aps[i].ssid,
                 (int)aps[i].rssi_dbm, (unsigned)aps[i].channel, (unsigned)aps[i].authmode,
                 ok ? "acceptable" : "rejected");
    }
    return ESP_OK;
}

esp_err_t ss_wifi_start(void)
{
    if (!s_ctx.inited || !s_ctx.have_cfg) { return ESP_ERR_INVALID_STATE; }
    if (s_ctx.started) { return ESP_OK; }

    esp_err_t err = esp_wifi_start();
    if (err != ESP_OK) { return err; }
    s_ctx.started = true;
    s_ctx.reconnect_attempt = 0u;

    // Best-effort ranked scan for diagnostics; a failure here does not fail start
    // (the STA_START event already drives the connect path).
    (void)ss_wifi_scan_and_rank();
    return ESP_OK;
}

esp_err_t ss_wifi_stop(void)
{
    if (!s_ctx.started) { return ESP_OK; }
    esp_err_t err = esp_wifi_stop();
    if (err != ESP_OK) { return err; }
    s_ctx.started = false;
    s_ctx.reconnect_attempt = 0u;
    return ESP_OK;
}

esp_err_t ss_wifi_sleep(bool on)
{
    if (!s_ctx.inited) { return ESP_ERR_INVALID_STATE; }
    return esp_wifi_set_ps(on ? WIFI_PS_MAX_MODEM : WIFI_PS_NONE);
}
