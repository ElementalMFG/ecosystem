<!-- SPDX-License-Identifier: Apache-2.0 -->
---
name: reference-idf-source-fetch
description: Pinned ESP-IDF v5.3.5 source is fetchable in this env for verifying API semantics — use it, don't assert from memory
metadata:
  type: reference
---

The sandbox HAS outbound network to `raw.githubusercontent.com`. Pinned ESP-IDF source can be fetched to verify IDF-version-specific API semantics (return codes, macro bodies, validation order):

`curl -sSL "https://raw.githubusercontent.com/espressif/esp-idf/v5.3.5/components/<path>"`

e.g. `components/esp_hw_support/sleep_modes.c` for `esp_sleep_*`.

**Why:** On S-03-030 I flagged a "comment vs IDF-semantics" defect asserting `esp_sleep_disable_wakeup_source` always returns ESP_OK — asserted from memory, hedged only with "verify against pinned source." It was WRONG (it returns ESP_ERR_INVALID_STATE for a not-armed source), and my remediation (delete the defensive absorption) would have broken idempotency. The coordinator fetched the raw file and corrected me.

**How to apply:** before flagging any IDF return-code / enum / macro / validation-order claim — especially one whose remediation is "remove defensive code" — FETCH the pinned v5.3.5 source and read it. Do not downgrade this to a "please confirm" hedge and still assign the finding; verify first, or state the claim as unverified and do NOT recommend deleting code on its basis. See [[gotchas-ss-power]].
