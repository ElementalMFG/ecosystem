---
# SPDX-License-Identifier: Apache-2.0
name: defects-esp-wifi-flash-persist
description: esp_wifi defaults to WIFI_STORAGE_FLASH — set_config silently persists SSID/PSK/AP-passphrase to NVS unless RAM storage is set
metadata:
  type: project
---

esp_wifi's default storage is `WIFI_STORAGE_FLASH` (confirmed IDF v5.3.5 esp_wifi.h:1040).
Once NVS is initialized, EVERY `esp_wifi_set_config()` writes the config (incl. AP
passphrase and STA PSK) to the `nvs.net80211` namespace in plaintext, surviving
teardown and reboot.

**Why:** This silently defeats "credentials always wiped / nothing persisted" contracts.
RAM secure_wipe() only clears the app copy, not the NVS copy. Generating models
routinely miss this because the persistence is an implicit IDF default, not a visible
write call.

**How to apply:** Whenever reviewing code that calls `esp_wifi_set_config` for AP or STA
in a security-sensitive/provisioning path, check for a preceding
`esp_wifi_set_storage(WIFI_STORAGE_RAM)`. Its absence contradicts any "no plaintext PSK on
flash" / "always wiped" claim (doc 05 §10.3). Same trap class likely applies to other
esp_* subsystems with implicit NVS persistence. See [[gotchas_ss_wifi]].
