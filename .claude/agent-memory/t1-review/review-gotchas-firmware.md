<!-- SPDX-License-Identifier: Apache-2.0 -->
---
name: review-gotchas-firmware
description: Recurring defect classes and per-component gotchas found in T1 firmware reviews (EPIC-02 area)
metadata:
  type: project
---

- Traceability claims in security-contract header comments ("tracked in story X / epic Y") are frequently wrong or unbacked — always grep the actual STORIES.md for the claimed requirement before accepting them (found in S-02-008: duress/coredump-erase claim pointed at EPIC-08; real duress story is S-07-021 in EPIC-07 and carried no such AC).
- `firmware/main/` code is board-generic but authors reason Lite-only: check `esp_reset_reason_t` handling (e.g. `ESP_RST_CPU_LOCKUP` unreachable on ESP32-S3 but real on Alpha's ESP32-P4/RISC-V) and similar S3-vs-P4 divergences.
- Host test harness: `cmake -S firmware/test/host -B <build> && cmake --build <build> -j && ctest --test-dir <build>` works standalone; pure cores also compile standalone with `gcc -std=c11 -Wall -Wextra -Wshadow -Wconversion -Werror -I firmware/main` — cheap warning-flag check without the IDF container.
- Flash-encryption interplay is a recurring blind spot: `encrypted`-flag partitions become unreadable via esptool/serial raw reads once EPIC-08 lands — any tool that reads flash over `--port` (e.g. tools/xtensa-decode-crash.py) silently breaks on release units.
- `ss_log` (SS_LOGx) needs no init and writes to stdout — safe to call pre-NVS/pre-scheduler in boot-gate code; don't flag early logging as a defect.
- partitions.csv arithmetic to re-verify every time: 16 MB total (0x1000000); current layout ends exactly at 0x1000000 with coredump 0xff0000+0x10000.
- Review artifacts often arrive STAGED: bare `git diff` shows nothing — use `git diff --cached` (git status column 1 = staged).
- esp_reset_reason_t in IDF v5.3 has exactly 16 members (UNKNOWN..ESP_RST_CPU_LOCKUP) — check classify functions for full coverage + safe default arm.
- espcoredump in IDF v5.3 (esp-coredump pkg): `--core-format` accepts and defaults to `auto`; `--chip`/`--port` are global opts before the `info_corefile` subcommand.
- Duplicated magic numbers across build/table/code (e.g. coredump partition size 0x10000 in partitions.csv AND ss_panic_guard.cpp) — flag drift risk; prefer esp_partition_find_first at runtime.
