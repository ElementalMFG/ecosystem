<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# Deprecations & sunset list

The single announcement channel for every deprecation across all
versioned surfaces, required by `06_GOVERNANCE.md` §6.3 and governed by
[`rfcs/0003-compat-deprecation-policy.md`](../rfcs/0003-compat-deprecation-policy.md).
A deprecation is not in effect until it has a row here; release notes
must cross-link the rows they introduce.

| Surface | Deprecated version(s) | Announced in release | Earliest removal release | Reference |
|---|---|---|---|---|
| Firmware HAL C API (`ss_hal_watchdog.h`) | `ss_wdt_init` / `ss_wdt_subscribe` / `ss_wdt_unsubscribe` / `ss_wdt_feed` — original sketch, never implemented, never published | unreleased (`main`, 2026-07-08) | any pre-1.0 release — an internal C API is not one of RFC-0003's seven versioned surfaces and never shipped, so its windows (which bind from first *published* version) do not apply; recorded here per `06_GOVERNANCE.md` §6.3 (single sunset list) with the story standing in for the sunset-procedure governance issue. Tombstone declarations remain until a follow-up removal | S-03-039 (EPIC-03); replacement `ss_task_wdt_*` (S-02-009) |
