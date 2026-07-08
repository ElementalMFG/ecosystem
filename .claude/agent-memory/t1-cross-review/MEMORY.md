<!-- SPDX-License-Identifier: Apache-2.0 -->
# T1 cross-review memory index

Consult these at the start of every review; append one line per durable finding.

## Defect classes (recurring)
- [Coredump / panic-guard gotchas](defects_coredump_panic.md) — espcoredump format default, reset-reason enum gaps, DRAM-capture pinning, app-level breaker scope.
- [Scripted guard placement](defects_scripted_guard_placement.md) — bulk extern "C"/ifdef inserts land before late #includes / double-wrap; still compiles, so read the diff not just the build.
- [esp_wifi default flash persistence](defects_esp_wifi_flash_persist.md) — set_config silently writes SSID/PSK/AP-pass to NVS plaintext unless esp_wifi_set_storage(RAM); breaks "always wiped" contracts.

## Per-component / repo gotchas
- [Repo doc & config gotchas](gotchas_repo.md) — 05_SECURITY_MODEL.md is cited everywhere but does not exist yet; sdkconfig self-documents defaults selectively.
- [ss_hal gotchas](gotchas_ss_hal.md) — umbrella extern "C" before 6 includes; host suite omits ss_hal.c (109/109 proves nothing about accessors); haptic/usb headers not self-contained; caps are all-bits-AND & 64-bit safe.
- [ss_power gotchas](gotchas_ss_power.md) — only core.c is host-built (glue untested); esp_sleep_disable_wakeup_source never returns ESP_ERR_INVALID_STATE; timer stickiness reasoning; HIBERNATE==DEEP_SLEEP.
- [ss_wifi gotchas](gotchas_ss_wifi.md) — default WIFI_STORAGE_FLASH leaks PSK/AP-pass to NVS; glue never host-built (only -Wconversion gate is make lite); prov core is solid + conversion-clean; esp_timer teardown UAF race.

## Workflow
- [Cross-review workflow](workflow_cross_review.md) — host tests are runnable/expected green; hardware AC parts park at IN_REVIEW by design.
- [IDF source is fetchable](reference_idf_source_fetch.md) — verify IDF v5.3.5 API semantics via raw.githubusercontent BEFORE flagging return-code claims / recommending deleting defensive code.
