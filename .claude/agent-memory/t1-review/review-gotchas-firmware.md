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
- extern "C" guard sweeps: sub-headers tend to be correct, the UMBRELLA header is where the open brace lands mid-include-list (found S-03-032, ss_hal.h) — always check the aggregator, and grep callers for now-stale "header has no guard" linkage comments (ss_display.cpp).
- Vacuous static_asserts pass silently: `(x & ~0xFFFFFFFFFFFFFFFFULL) == 0` is always true (mask is 0) — check assert expressions actually constrain (test_hal_mock.cpp:22).
- Standalone header syntax checks: `g++ -fsyntax-only header.h` false-fails with `-Werror` ("#pragma once in main file") — wrap in a one-line `#include` TU instead; host mocks dir (`firmware/test/host/mocks/`) supplies esp_err.h.
- Duplicated magic numbers across build/table/code (e.g. coredump partition size 0x10000 in partitions.csv AND ss_panic_guard.cpp) — flag drift risk; prefer esp_partition_find_first at runtime.
- ss_power glue mirrors platform sleep config in pure-core records — audit every mutation path (init/clear/shutdown) for record-vs-platform divergence. Do NOT prescribe unconditional esp_sleep_disable_wakeup_source: IDF v5.3 ESP_LOGEs + returns INVALID_STATE for a not-enabled source (sleep_modes.c CHECK_SOURCE requires the trigger bit). Validated pattern (S-03-030): prevent divergence at the sources — set() commits record only after platform accepts; init() disarms a live arming before memset; clear() gates on the record.
- Frozen HAL header post-conditions that enumerate an exact error set: grep the glue for propagated platform errors not in the list — "otherwise ESP_OK" claims are often false once esp_* return values are forwarded (found S-03-030 timer set/clear).
- esp_timer stop+delete from a different task can free the timer while its callback is in flight (stop does not quiesce; S3 is dual-core) — for session-scoped periodic timers demand create-once/stop-only reuse, mirroring the ap_netif pattern (found S-03-015 portal_teardown).
- User callbacks or blocking teardown (httpd_stop/dns_stop) invoked while holding a module mutex = self-deadlock / circular-wait hazard — copy data to locals, release the lock, then call out (found S-03-015 display_cb + error-path teardown under lock).
- ss_wifi host Makefile CFLAGS lack -Wshadow -Wconversion (repo standard has them) — always re-compile pure cores manually with the full flag set; harness passing proves less than it claims.
- Task-exit handshakes via binary semaphore with a take-timeout: timeout path closes resources under the live task AND leaves a stale give that voids the NEXT session's handshake (found S-03-015 dns_stop). Validated fix pattern: drain the semaphore before task creation; on timeout ABANDON (leak) the socket, never close it under a live task.
- esp_wifi defaults to WIFI_STORAGE_FLASH: every esp_wifi_set_config writes creds to plaintext NVS — any component calling set_config must have esp_wifi_set_storage(WIFI_STORAGE_RAM) upstream (found S-03-015 cross-review; fixed in ss_wifi_init).
- Lazy-init of module mutexes inside a public start() (`if (lock == NULL) lock = xSemaphoreCreateMutex()`) is itself a TOCTOU if two tasks race the first call — prefer create-at-init or note the single-caller assumption in the header (S-03-015 nit).
