---
# SPDX-License-Identifier: Apache-2.0
name: gotchas_ss_wifi
description: ss_wifi component (STA S-03-014 + soft-AP provisioning S-03-015) per-component review gotchas
metadata:
  type: project
---

ss_wifi component gotchas (verified 2026-07 on S-03-015 review):

- **Default flash persistence — FIXED (S-03-015 rework, verified 2026-07-08):**
  ss_wifi_init (ss_wifi.c:133) now calls `esp_wifi_set_storage(WIFI_STORAGE_RAM)` right
  after esp_wifi_init, before any set_config. Driver-global, persists for wifi lifetime;
  covers STA (ss_wifi_config) AND AP (ap_apply_config) AND the done-cb STA apply. No
  esp_wifi_restore/deinit anywhere to reset it. Re-init early-returns (set once = fine).
  NOTE the prov plaintext guarantee is TRANSITIVELY dependent on ss_wifi_init running
  first — ap_apply_config does not set storage itself. See [[defects-esp-wifi-flash-persist]].
- **Host harness scope:** only the pure cores (ss_wifi_sta_core.c, ss_wifi_prov_core.c)
  are host-built under ASan/UBSan; the IDF glue (ss_wifi.c, ss_wifi_prov.c) is NEVER
  host-compiled and NOT under -Wconversion locally — its only conversion gate is
  `make lite` in CI. A green host suite proves nothing about the glue.
- **prov core is genuinely solid:** rejection sampling unbiased (REJECT_BOUND=248=8*31),
  12 sym * log2(31) = 59.4 bits > doc-05 floor of 56; wrap-safe deadlines; fail-closed
  entropy; DNS + form/url parsers are bounds-tight. Don't waste a pass re-deriving these.
- **prov core -Wconversion/-Wsign-conversion CLEAN** (verified with gcc directly); host
  Makefile itself omits -Wconversion so re-check the core manually if it changes.
- **Two-phase TRNG bring-up** (bootstrap pass pre-RF, regenerate post esp_wifi_start) is
  intentional, not a bug — RNG quality guarantee needs RF on.
- **esp_timer teardown race — FIXED (rework):** tick_timer created once (portal_start
  guards on NULL), reused; portal_teardown only stops, never deletes. Stray post-stop tick
  takes lock, sees active==false, no-ops. finishing-guard blocks double finisher during
  teardown (finishing cleared only AFTER timer stopped). Verified no double-teardown.
- **DNS stop abandon-path residual (LOW, still open):** dns_stop (ss_wifi_prov.c:210) on
  the should-not-happen 4s timeout sets dns_sock=-1 WITHOUT stopping the still-alive task.
  dns_task reads the GLOBAL s_prov.dns_sock, so a next session's dns_start reassigns it and
  the leaked task races the new socket + a late give corrupts the next dns_exited handshake;
  FD_SET(-1) is also UB. Unreachable in practice (500ms select, no blocking op) → LOW. Clean
  fix = pass fd as task arg (captured copy), task owns close-on-exit.
