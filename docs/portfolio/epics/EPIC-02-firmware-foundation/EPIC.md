<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# EPIC-02 — Firmware Foundation

**Primary WG:** wg-firmware · **Contributing:** wg-ops, wg-security
**Priority:** P0 · **SKU:** ★ · **Milestone:** M0

## Outcome
A reusable firmware baseline exists in `firmware/` that any board target can consume: ESP-IDF as the SDK, FreeRTOS as the RTOS, monorepo build via idf.py + CMake wrappers, logging, panic capture, watchdog, memory instrumentation, and a `main/` boot sequence that brings up HAL and hands off to application layer.

## Constitution
C-00 `00_MASTER_SOFTWARE_PLAN.md` §firmware baseline (SDK, RTOS, build, logging, test strategy).

## Dependencies
EPIC-01 (governance to gate merges).

## Shards
- **S-02.A Toolchain & build** — pinned ESP-IDF v5.x, xtensa/riscv toolchains, reproducible container.
- **S-02.B Board abstraction glue** — `boards/<sku>/board_config.h`, KConfig fragments, per-board partitions.
- **S-02.C RTOS baseline** — FreeRTOS config, tick, heap policy, stack overflow hooks.
- **S-02.D Boot sequence** — bootloader → app_main → hal_init → services → applications, with hooks for secure boot.
- **S-02.E Logging & panic** — `ss_log` levels, redaction rules, panic dump to flash, crash-loop guard.
- **S-02.F Watchdog & health** — TWDT + IWDT, task health beacons, brown-out policy.
- **S-02.G Memory instrumentation** — heap tracker, task stack watermark, IDLE0/1 CPU sampler.
- **S-02.H Component registry** — component template, versioning, semver, dependency graph.
- **S-02.I Firmware unit-test harness** — host-side gtest + on-target unity, mocked HAL.

## Exit criteria
1. `idf.py build` succeeds on all three board targets from a clean container.
2. Boot to `app_main` under 400 ms on Lite hardware.
3. Panic on any task dumps a decode-able crash record to flash and reboots without loop.
4. Watchdogs fire in synthetic hang tests.
5. Unit tests ≥ 80 % line coverage on baseline code.

## RACI
- R: wg-firmware chair · A: wg-firmware chair · C: wg-security, wg-ops · I: all firmware devs.

## Risks
| # | Risk | Mitigation |
|---|------|-----------|
| R02-01 | ESP-IDF breaking-change churn | Pin minor version, quarterly upgrade RFC |
| R02-02 | Board configs diverge silently | Shared header parity CI check |
| R02-03 | Heap fragmentation over long-run | Long-run soak test in EPIC-22 |
