// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_wifi_prov.c — ESP-IDF glue for the S-03-015 captive-provisioning
// portal: soft-AP bring-up, DNS catch-all, HTTP portal, and teardown. Every
// security decision (passphrase generation, credential validation, session
// bounds, zeroization) lives in the pure host-tested core
// (ss_wifi_prov_core.c); this file owns only the ESP-IDF wiring.
//
// TRNG ORDERING: the ESP32-S3 hardware RNG is guaranteed truly random only
// while an RF subsystem is enabled. The AP, however, needs a WPA passphrase
// before esp_wifi_start(). So bring-up is two-phase: (1) start the AP with a
// bootstrap passphrase drawn from the boot-seeded RNG — it lives for
// milliseconds and is shown to no one; (2) with RF now running, restart the
// session so the core regenerates passphrase + verify code from the true
// TRNG, and push the final AP config. Only the phase-2 passphrase is ever
// displayed.
//
// SECRET HYGIENE: the home PSK appears in this file only inside the POST
// body buffer, the decoded field buffers, and the handoff ss_wifi_cfg_t —
// each is wiped immediately after use. No ESP_LOG call ever prints a
// passphrase, PSK, or SSID contents. esp_wifi itself is forced to
// WIFI_STORAGE_RAM in ss_wifi_init() (doc 05 §10.3: never plaintext at
// rest), so esp_wifi_set_config() here never touches flash.
//
// This is a well-structured first cut of the on-target path (it cannot run
// without hardware). It compiles under ESP-IDF v5.3.5.

#include "ss_wifi_prov.h"

#include <string.h>

#include "esp_event.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_netif.h"
#include "esp_random.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "lwip/sockets.h"

#include "ss_wifi_prov_core.h"

static const char* TAG = "ss_wifi_prov";

#define PROV_TICK_PERIOD_US (500 * 1000)
#define PROV_DNS_PORT 53u
#define PROV_DNS_MAX_PKT 512u
#define PROV_POST_MAX_LEN 256u
#define PROV_FIELD_MAX_LEN SS_WIFI_PROV_PSK_MAX_LEN // largest decoded form field

// Cohesive module state. `lock` serialises the core session between the
// httpd worker, the esp_timer task, the finisher task, and the public API.
typedef struct {
    bool active;                  // a session owns the Wi-Fi right now
    bool finishing;               // finisher task already spawned
    ss_wifi_prov_session_t sess;  // pure core session (all secrets live here)
    SemaphoreHandle_t lock;       // guards sess + active/finishing
    SemaphoreHandle_t dns_exited; // DNS task exit handshake
    volatile bool dns_stop;       // DNS task stop request
    int dns_sock;                 // DNS catch-all socket (-1 when closed)
    httpd_handle_t httpd;         // portal server
    esp_timer_handle_t tick_timer;
    esp_netif_t* ap_netif; // created once, reused across sessions
    uint32_t ap_ip;        // AP IPv4 (network order) for DNS answers
    char redirect_url[32]; // "http://a.b.c.d/"
    ss_wifi_prov_display_fn display_cb;
    ss_wifi_prov_done_fn done_cb;
} prov_ctx_t;

static prov_ctx_t s_prov = {.dns_sock = -1};

static void secure_wipe(void* p, size_t n)
{
    volatile uint8_t* v = (volatile uint8_t*)p;
    for (size_t i = 0; i < n; i++) { v[i] = 0u; }
}

static uint32_t now_ms(void)
{
    return (uint32_t)(esp_timer_get_time() / 1000);
}

// Core entropy hook. esp_fill_random() cannot report failure; the TRNG
// quality guarantee is handled by the two-phase bring-up above.
static bool prov_entropy(void* ctx, uint8_t* buf, size_t len)
{
    (void)ctx;
    esp_fill_random(buf, len);
    return true;
}

// ---------------------------------------------------------------- DNS ----

// Minimal, bounds-checked DNS responder: every standard A-shaped query is
// answered with the AP address so captive-portal probes resolve to us.
// Anything malformed, compressed-in-question, or non-query is dropped.
// This parser faces unauthenticated (post-association) input — keep every
// read bounds-checked.
static void dns_answer(int sock, const uint8_t* req, size_t len, const struct sockaddr_in* from)
{
    if (len < 12u || len > PROV_DNS_MAX_PKT) { return; }
    if ((req[2] & 0x80u) != 0u) { return; } // QR set: a response, not a query
    if ((req[2] & 0x78u) != 0u) { return; } // opcode != QUERY
    const uint16_t qdcount = (uint16_t)((req[4] << 8) | req[5]);
    if (qdcount == 0u) { return; }

    // Walk the first QNAME with explicit bounds.
    size_t off = 12u;
    for (;;) {
        if (off >= len) { return; }
        const uint8_t l = req[off];
        if (l == 0u) {
            off += 1u;
            break;
        }
        if ((l & 0xC0u) != 0u) { return; } // compression in a question: drop
        off += 1u + (size_t)l;
    }
    if (off + 4u > len) { return; } // QTYPE + QCLASS must be present
    const uint16_t qtype = (uint16_t)((req[off] << 8) | req[off + 1u]);
    const uint16_t qclass = (uint16_t)((req[off + 2u] << 8) | req[off + 3u]);
    if (qtype != 1u || qclass != 1u) { return; } // answer A/IN only; drop the rest

    uint8_t resp[PROV_DNS_MAX_PKT + 16u];
    const size_t qlen = off + 4u;
    memcpy(resp, req, qlen);
    resp[2] = 0x84u; // QR + AA
    resp[3] = 0x00u; // RA clear, RCODE 0
    resp[4] = 0x00u;
    resp[5] = 0x01u; // QDCOUNT = 1 (answer the first question only)
    resp[6] = 0x00u;
    resp[7] = 0x01u; // ANCOUNT = 1
    resp[8] = resp[9] = resp[10] = resp[11] = 0u;

    uint8_t* a = &resp[qlen];
    a[0] = 0xC0u; // pointer to QNAME at offset 12
    a[1] = 0x0Cu;
    a[2] = 0x00u; // TYPE A
    a[3] = 0x01u;
    a[4] = 0x00u; // CLASS IN
    a[5] = 0x01u;
    a[6] = a[7] = a[8] = 0u; // TTL
    a[9] = 60u;
    a[10] = 0x00u; // RDLENGTH 4
    a[11] = 0x04u;
    memcpy(&a[12], &s_prov.ap_ip, 4u);

    (void)sendto(sock, resp, qlen + 16u, 0, (const struct sockaddr*)from,
                 sizeof(struct sockaddr_in));
}

// The task owns its socket: the fd arrives as the task argument (a captured
// copy, never the mutable global) and the task closes it on exit. A leaked
// task from a should-not-happen stop timeout therefore can never race the
// next session's socket.
static void dns_task(void* arg)
{
    const int sock = (int)(intptr_t)arg;
    while (!s_prov.dns_stop) {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(sock, &rfds);
        struct timeval tv = {.tv_sec = 0, .tv_usec = 500 * 1000};
        const int r = select(sock + 1, &rfds, NULL, NULL, &tv);
        if (r <= 0) { continue; }

        uint8_t pkt[PROV_DNS_MAX_PKT];
        struct sockaddr_in from;
        socklen_t fromlen = sizeof(from);
        const ssize_t n = recvfrom(sock, pkt, sizeof(pkt), 0, (struct sockaddr*)&from, &fromlen);
        if (n > 0) { dns_answer(sock, pkt, (size_t)n, &from); }
    }
    close(sock);
    xSemaphoreGive(s_prov.dns_exited);
    vTaskDelete(NULL);
}

static esp_err_t dns_start(void)
{
    const int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) { return ESP_FAIL; }
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PROV_DNS_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        close(sock);
        return ESP_FAIL;
    }
    s_prov.dns_stop = false;
    (void)xSemaphoreTake(s_prov.dns_exited, 0); // drain any stale give
    if (xTaskCreate(dns_task, "prov_dns", 3072, (void*)(intptr_t)sock, tskIDLE_PRIORITY + 2,
                    NULL) != pdPASS) {
        close(sock);
        return ESP_FAIL;
    }
    s_prov.dns_sock = sock; // bookkeeping only; the task owns the fd
    return ESP_OK;
}

static void dns_stop(void)
{
    if (s_prov.dns_sock < 0) { return; }
    s_prov.dns_stop = true;
    // The task re-checks the flag at least every 500 ms (select timeout) and
    // closes its own fd on exit. On the should-not-happen timeout the task
    // (and its fd) leak harmlessly — it holds a private copy of the fd, so
    // it can never touch the next session's socket.
    if (xSemaphoreTake(s_prov.dns_exited, pdMS_TO_TICKS(2000)) != pdTRUE &&
        xSemaphoreTake(s_prov.dns_exited, pdMS_TO_TICKS(2000)) != pdTRUE) {
        ESP_LOGE(TAG, "dns task did not exit; leaking it");
    }
    s_prov.dns_sock = -1;
}

// --------------------------------------------------------------- HTTP ----

static const char PORTAL_PAGE_FMT[] =
    "<!DOCTYPE html><html><head><meta charset=\"utf-8\">"
    "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
    "<title>SS-SP setup</title>"
    "<style>body{font-family:sans-serif;margin:2em;max-width:26em}"
    "code{font-size:1.6em;letter-spacing:.2em}</style></head><body>"
    "<h2>SS-SP provisioning</h2>"
    "<p>Confirm this code matches the device screen:</p><p><code>%s</code></p>"
    "<form method=\"POST\" action=\"/provision\">"
    "<label>Home Wi-Fi name<br><input name=\"ssid\" maxlength=\"32\" required></label><br><br>"
    "<label>Password (leave blank for an open network)<br>"
    "<input name=\"psk\" type=\"password\" maxlength=\"64\"></label><br><br>"
    "<button>Provision</button></form></body></html>";

static const char SUCCESS_PAGE[] =
    "<!DOCTYPE html><html><body><h2>Done</h2>"
    "<p>Credentials received — the device is connecting to your network. "
    "This setup network will now disappear.</p></body></html>";

static const char REJECT_PAGE[] =
    "<!DOCTYPE html><html><body><h2>Invalid credentials</h2>"
    "<p>Check the network name and password and try again.</p></body></html>";

static const char INACTIVE_PAGE[] = "<!DOCTYPE html><html><body><h2>Setup not active</h2>"
                                    "<p>This provisioning session has ended.</p></body></html>";

// Percent-decode src[0..slen) into out (cap includes the NUL). '+' becomes
// space. Returns decoded length, or -1 on malformed escapes / overflow.
static int url_decode(const char* src, size_t slen, char* out, size_t cap)
{
    size_t o = 0;
    for (size_t i = 0; i < slen; i++) {
        char c = src[i];
        if (c == '+') {
            c = ' ';
        } else if (c == '%') {
            if (i + 2u >= slen) { return -1; } // need two hex digits after '%'
            int hi = -1, lo = -1;
            const char h = src[i + 1u], l = src[i + 2u];
            hi = (h >= '0' && h <= '9')   ? h - '0'
                 : (h >= 'a' && h <= 'f') ? h - 'a' + 10
                 : (h >= 'A' && h <= 'F') ? h - 'A' + 10
                                          : -1;
            lo = (l >= '0' && l <= '9')   ? l - '0'
                 : (l >= 'a' && l <= 'f') ? l - 'a' + 10
                 : (l >= 'A' && l <= 'F') ? l - 'A' + 10
                                          : -1;
            if (hi < 0 || lo < 0) { return -1; }
            c = (char)((hi << 4) | lo);
            i += 2u;
        }
        if (o + 1u >= cap) { return -1; }
        out[o++] = c;
    }
    out[o] = '\0';
    return (int)o;
}

// Extract one form field ("name=value") from an x-www-form-urlencoded body.
// Returns decoded length (0 for present-but-empty or absent), -1 on
// malformed encoding or overflow.
static int form_field(const char* body, size_t blen, const char* name, char* out, size_t cap)
{
    const size_t nlen = strlen(name);
    size_t i = 0;
    out[0] = '\0';
    while (i < blen) {
        size_t j = i;
        while (j < blen && body[j] != '&') { j++; }
        const size_t flen = j - i;
        if (flen > nlen && memcmp(&body[i], name, nlen) == 0 && body[i + nlen] == '=') {
            return url_decode(&body[i + nlen + 1u], flen - nlen - 1u, out, cap);
        }
        i = j + 1u;
    }
    return 0;
}

static esp_err_t http_root_get(httpd_req_t* req)
{
    char page[sizeof(PORTAL_PAGE_FMT) + SS_WIFI_PROV_CODE_LEN];
    char code[SS_WIFI_PROV_CODE_LEN + 1u] = {0};

    xSemaphoreTake(s_prov.lock, portMAX_DELAY);
    memcpy(code, s_prov.sess.code, sizeof(code));
    xSemaphoreGive(s_prov.lock);

    if (code[0] == '\0') { // session no longer ACTIVE (code wiped)
        httpd_resp_set_type(req, "text/html");
        httpd_resp_set_status(req, "410 Gone");
        return httpd_resp_send(req, INACTIVE_PAGE, sizeof(INACTIVE_PAGE) - 1u);
    }

    const int n = snprintf(page, sizeof(page), PORTAL_PAGE_FMT, code);
    if (n < 0 || (size_t)n >= sizeof(page)) { return ESP_FAIL; }
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, page, n);
}

static esp_err_t http_provision_post(httpd_req_t* req)
{
    if (req->content_len > PROV_POST_MAX_LEN) {
        httpd_resp_set_status(req, "413 Payload Too Large");
        (void)httpd_resp_send(req, NULL, 0);
        return ESP_FAIL; // close the connection: the oversized body was never drained
    }

    char body[PROV_POST_MAX_LEN + 1u];
    size_t got = 0;
    while (got < req->content_len) {
        const int r = httpd_req_recv(req, &body[got], req->content_len - got);
        if (r <= 0) {
            secure_wipe(body, sizeof(body));
            return ESP_FAIL;
        }
        got += (size_t)r;
    }
    body[got] = '\0';

    char ssid[PROV_FIELD_MAX_LEN + 1u];
    char psk[PROV_FIELD_MAX_LEN + 1u];
    const int ssid_len = form_field(body, got, "ssid", ssid, sizeof(ssid));
    const int psk_len = form_field(body, got, "psk", psk, sizeof(psk));
    secure_wipe(body, sizeof(body));

    ss_wifi_prov_submit_t res;
    xSemaphoreTake(s_prov.lock, portMAX_DELAY);
    if (ssid_len >= 0 && psk_len >= 0) {
        res = ss_wifi_prov_submit(&s_prov.sess, ssid, (size_t)ssid_len, psk, (size_t)psk_len,
                                  now_ms());
    } else {
        // Malformed encoding still charges the failed-submit budget
        // (contract §3): submit a deliberately-invalid pair.
        res = ss_wifi_prov_submit(&s_prov.sess, NULL, 0u, NULL, 0u, now_ms());
    }
    xSemaphoreGive(s_prov.lock);
    secure_wipe(ssid, sizeof(ssid));
    secure_wipe(psk, sizeof(psk));

    httpd_resp_set_type(req, "text/html");
    switch (res) {
    case SS_WIFI_PROV_SUBMIT_ACCEPTED:
        ESP_LOGI(TAG, "credentials accepted");
        return httpd_resp_send(req, SUCCESS_PAGE, sizeof(SUCCESS_PAGE) - 1u);
    case SS_WIFI_PROV_SUBMIT_REJECTED:
        ESP_LOGW(TAG, "credentials rejected");
        httpd_resp_set_status(req, "400 Bad Request");
        return httpd_resp_send(req, REJECT_PAGE, sizeof(REJECT_PAGE) - 1u);
    case SS_WIFI_PROV_SUBMIT_IGNORED:
    default:
        httpd_resp_set_status(req, "410 Gone");
        return httpd_resp_send(req, NULL, 0);
    }
}

// Captive-portal catch-all: every other GET (connectivity probes included)
// is redirected to the portal root.
static esp_err_t http_redirect_get(httpd_req_t* req)
{
    httpd_resp_set_status(req, "302 Found");
    httpd_resp_set_hdr(req, "Location", s_prov.redirect_url);
    return httpd_resp_send(req, NULL, 0);
}

static esp_err_t http_start(void)
{
    httpd_config_t cfg = HTTPD_DEFAULT_CONFIG();
    cfg.uri_match_fn = httpd_uri_match_wildcard;
    cfg.max_open_sockets = 4;
    cfg.lru_purge_enable = true;
    if (httpd_start(&s_prov.httpd, &cfg) != ESP_OK) { return ESP_FAIL; }

    const httpd_uri_t root = {.uri = "/", .method = HTTP_GET, .handler = http_root_get};
    const httpd_uri_t prov = {
        .uri = "/provision", .method = HTTP_POST, .handler = http_provision_post};
    const httpd_uri_t any = {.uri = "/*", .method = HTTP_GET, .handler = http_redirect_get};
    esp_err_t err = httpd_register_uri_handler(s_prov.httpd, &root);
    if (err == ESP_OK) { err = httpd_register_uri_handler(s_prov.httpd, &prov); }
    if (err == ESP_OK) { err = httpd_register_uri_handler(s_prov.httpd, &any); }
    if (err != ESP_OK) {
        httpd_stop(s_prov.httpd);
        s_prov.httpd = NULL;
    }
    return err;
}

// ----------------------------------------------------------- lifecycle ----

static void build_ap_ssid(char* out, size_t cap)
{
    uint8_t mac[6] = {0};
    (void)esp_read_mac(mac, ESP_MAC_WIFI_SOFTAP);
    (void)snprintf(out, cap, "SS-SP-%02X%02X", mac[4], mac[5]);
}

// Push the session passphrase into the AP config. Never open: WPA2/WPA3
// transition, PMF capable, exactly one client. The no-plaintext-at-rest
// guarantee relies on ss_wifi_init() having set WIFI_STORAGE_RAM (a hard
// precondition of portal_start); this call does not set storage itself.
static esp_err_t ap_apply_config(const char* pass)
{
    wifi_config_t wc;
    memset(&wc, 0, sizeof(wc));
    build_ap_ssid((char*)wc.ap.ssid, sizeof(wc.ap.ssid));
    wc.ap.ssid_len = (uint8_t)strlen((const char*)wc.ap.ssid);
    strncpy((char*)wc.ap.password, pass, sizeof(wc.ap.password) - 1u);
    wc.ap.channel = 1;
    wc.ap.max_connection = 1; // single provisioning client (contract §3)
    wc.ap.authmode = WIFI_AUTH_WPA2_WPA3_PSK;
    wc.ap.pmf_cfg.capable = true;
    wc.ap.pmf_cfg.required = false;
    const esp_err_t err = esp_wifi_set_config(WIFI_IF_AP, &wc);
    secure_wipe(&wc, sizeof(wc));
    return err;
}

// Tear the whole portal down and hand Wi-Fi back to STA mode. Called only
// from the finisher task (or a failed start on the caller task).
static void portal_teardown(void)
{
    if (s_prov.tick_timer != NULL) {
        // Stop only — the timer object is created once and reused across
        // sessions (like ap_netif). Deleting here could free it under an
        // in-flight tick_cb on the other core; a stray post-stop tick is
        // harmless (it takes the lock, sees active == false, does nothing).
        (void)esp_timer_stop(s_prov.tick_timer);
    }
    if (s_prov.httpd != NULL) {
        (void)httpd_stop(s_prov.httpd);
        s_prov.httpd = NULL;
    }
    dns_stop();
    (void)esp_wifi_stop();
    (void)esp_wifi_set_mode(WIFI_MODE_STA); // AC: soft-AP is gone after the session
}

// Finisher: runs teardown and the done callback outside the timer task.
static void finish_task(void* arg)
{
    const bool success = (arg != NULL);
    ss_wifi_cfg_t cfg;
    memset(&cfg, 0, sizeof(cfg));
    bool have_creds = false;

    if (success) {
        xSemaphoreTake(s_prov.lock, portMAX_DELAY);
        have_creds = ss_wifi_prov_take_credentials(&s_prov.sess, cfg.ssid, sizeof(cfg.ssid),
                                                   cfg.pass, sizeof(cfg.pass));
        xSemaphoreGive(s_prov.lock);
        cfg.sta_mode = true;
        cfg.ap_mode = false;
    }

    portal_teardown();

    xSemaphoreTake(s_prov.lock, portMAX_DELAY);
    const ss_wifi_prov_abort_t reason = s_prov.sess.abort_reason;
    ss_wifi_prov_end(&s_prov.sess);
    s_prov.active = false;
    s_prov.finishing = false;
    ss_wifi_prov_done_fn done = s_prov.done_cb;
    xSemaphoreGive(s_prov.lock);

    if (done != NULL) {
        if (have_creds) {
            done(true, &cfg);
        } else {
            ESP_LOGW(TAG, "session ended without handoff (reason %d)", (int)reason);
            done(false, NULL);
        }
    }
    secure_wipe(&cfg, sizeof(cfg));
    vTaskDelete(NULL);
}

static void spawn_finisher(bool success)
{
    // Caller holds s_prov.lock.
    if (s_prov.finishing) { return; }
    s_prov.finishing = true;
    if (xTaskCreate(finish_task, "prov_fin", 4096, success ? (void*)1 : NULL, tskIDLE_PRIORITY + 2,
                    NULL) != pdPASS) {
        s_prov.finishing = false; // next tick retries
    }
}

static void tick_cb(void* arg)
{
    (void)arg;
    xSemaphoreTake(s_prov.lock, portMAX_DELAY);
    if (s_prov.active) {
        const ss_wifi_prov_state_t st = ss_wifi_prov_tick(&s_prov.sess, now_ms());
        if (st == SS_WIFI_PROV_DONE) {
            spawn_finisher(true);
        } else if (st == SS_WIFI_PROV_ABORTED) {
            spawn_finisher(false);
        }
    }
    xSemaphoreGive(s_prov.lock);
}

esp_err_t ss_wifi_prov_portal_start(ss_wifi_prov_display_fn display_cb,
                                    ss_wifi_prov_done_fn done_cb, uint32_t window_ms)
{
    if (display_cb == NULL || done_cb == NULL) { return ESP_ERR_INVALID_ARG; }

    wifi_mode_t mode;
    if (esp_wifi_get_mode(&mode) != ESP_OK) { return ESP_ERR_INVALID_STATE; } // ss_wifi_init first

    if (s_prov.lock == NULL) {
        s_prov.lock = xSemaphoreCreateMutex();
        s_prov.dns_exited = xSemaphoreCreateBinary();
        if (s_prov.lock == NULL || s_prov.dns_exited == NULL) { return ESP_ERR_NO_MEM; }
    }

    xSemaphoreTake(s_prov.lock, portMAX_DELAY);
    if (s_prov.active) {
        xSemaphoreGive(s_prov.lock);
        return ESP_ERR_INVALID_STATE;
    }

    // Phase 1: bootstrap session so the AP can start with a non-open,
    // unguessable config while RF (and thus the true TRNG) is still off.
    if (!ss_wifi_prov_begin(&s_prov.sess, prov_entropy, NULL, now_ms(), window_ms)) {
        xSemaphoreGive(s_prov.lock);
        return ESP_FAIL; // fail closed: no AP without a passphrase
    }

    esp_err_t err = ESP_OK;
    if (s_prov.ap_netif == NULL) {
        s_prov.ap_netif = esp_netif_create_default_wifi_ap();
        if (s_prov.ap_netif == NULL) { err = ESP_FAIL; }
    }
    if (err == ESP_OK) { err = esp_wifi_set_mode(WIFI_MODE_AP); }
    if (err == ESP_OK) { err = ap_apply_config(s_prov.sess.ap_pass); }
    if (err == ESP_OK) { err = esp_wifi_start(); }

    // Phase 2: RF is up — regenerate the real secrets from the true TRNG and
    // push the final AP passphrase.
    if (err == ESP_OK &&
        !ss_wifi_prov_begin(&s_prov.sess, prov_entropy, NULL, now_ms(), window_ms)) {
        err = ESP_FAIL;
    }
    if (err == ESP_OK) { err = ap_apply_config(s_prov.sess.ap_pass); }

    if (err == ESP_OK) {
        esp_netif_ip_info_t ip = {0};
        err = esp_netif_get_ip_info(s_prov.ap_netif, &ip);
        if (err == ESP_OK) {
            s_prov.ap_ip = ip.ip.addr;
            (void)snprintf(s_prov.redirect_url, sizeof(s_prov.redirect_url), "http://" IPSTR "/",
                           IP2STR(&ip.ip));
        }
    }

    if (err == ESP_OK) { err = dns_start(); }
    if (err == ESP_OK) { err = http_start(); }
    if (err == ESP_OK && s_prov.tick_timer == NULL) {
        // Created once and reused across sessions; see portal_teardown().
        const esp_timer_create_args_t targs = {.callback = tick_cb, .name = "prov_tick"};
        err = esp_timer_create(&targs, &s_prov.tick_timer);
    }
    if (err == ESP_OK) { err = esp_timer_start_periodic(s_prov.tick_timer, PROV_TICK_PERIOD_US); }

    if (err != ESP_OK) {
        // Wipe the session under the lock, then run the blocking teardown
        // outside it (httpd_stop/dns_stop must never run under the mutex).
        // No finisher exists (active is still false), so this task owns the
        // portal resources; a late portal handler just sees an IDLE session.
        ss_wifi_prov_end(&s_prov.sess);
        xSemaphoreGive(s_prov.lock);
        portal_teardown();
        ESP_LOGE(TAG, "portal start failed: %s", esp_err_to_name(err));
        return err;
    }

    s_prov.display_cb = display_cb;
    s_prov.done_cb = done_cb;
    s_prov.active = true;
    s_prov.finishing = false;

    // Snapshot the secrets and invoke the display callback OUTSIDE the lock:
    // a blocking display must not stall the tick/portal, and a callback that
    // (against the header contract) re-enters portal_* must not self-deadlock.
    char ap_ssid[sizeof(((wifi_ap_config_t*)0)->ssid) + 1u] = {0};
    char ap_pass[SS_WIFI_PROV_PASS_LEN + 1u];
    char code[SS_WIFI_PROV_CODE_LEN + 1u];
    build_ap_ssid(ap_ssid, sizeof(ap_ssid));
    memcpy(ap_pass, s_prov.sess.ap_pass, sizeof(ap_pass));
    memcpy(code, s_prov.sess.code, sizeof(code));
    xSemaphoreGive(s_prov.lock);

    display_cb(ap_ssid, ap_pass, code);
    secure_wipe(ap_pass, sizeof(ap_pass));
    secure_wipe(code, sizeof(code));

    ESP_LOGI(TAG, "provisioning portal up (single client, bounded window)");
    return ESP_OK;
}

static esp_err_t portal_abort(bool power)
{
    if (s_prov.lock == NULL) { return ESP_ERR_INVALID_STATE; }
    xSemaphoreTake(s_prov.lock, portMAX_DELAY);
    if (!s_prov.active) {
        xSemaphoreGive(s_prov.lock);
        return ESP_ERR_INVALID_STATE;
    }
    if (power) {
        ss_wifi_prov_power_restrict(&s_prov.sess);
    } else {
        ss_wifi_prov_cancel(&s_prov.sess);
    }
    spawn_finisher(false);
    xSemaphoreGive(s_prov.lock);
    return ESP_OK;
}

esp_err_t ss_wifi_prov_portal_cancel(void)
{
    return portal_abort(false);
}

esp_err_t ss_wifi_prov_portal_power_restrict(void)
{
    return portal_abort(true);
}
