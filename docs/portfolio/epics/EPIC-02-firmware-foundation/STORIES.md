<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# EPIC-02 — Stories

Format per `../../00_METHODOLOGY.md` §2.7. Meta lines are machine-parsed.

---

### S-02-001 — Pin ESP-IDF v5.x + provide reproducible build container
As a firmware engineer I want ESP-IDF v5.x pinned inside a reproducible build container so that every developer and CI job builds identical bits.
- AC: `.devcontainer/` and `Dockerfile` present; container image digest-pinned; `idf.py build` succeeds from a clean container
- Meta: Shard=A | Type=Feature | Size=M | Prio=P0 | Status=DONE | SKU=★ | PRD=NF-SUS-03 | Const=C-00
- Tasks: spec (pinning policy, RFC-0002) · design (digest-pinned base image) · impl (`ci/containers/firmware/Dockerfile` + `.devcontainer/` + `firmware-build.yml`) · test (clean `idf.py build` green inside the container) · docs (`BUILDING.md`).
- Deps: RFC-0002

### S-02-002 — Monorepo CMake wrapper
As a firmware engineer I want a monorepo CMake wrapper so that any board target builds with one command.
- AC: `firmware/CMakeLists.txt` selects board from `BOARD` env; `make lite|alpha|omega` targets work; unknown board value fails with a clear error
- Meta: Shard=A | Type=Feature | Size=M | Prio=P0 | Status=DONE | SKU=★ | PRD=— | Const=C-00
- Tasks: spec (board-selection contract) · design (`SS_BOARD` cache var + make targets) · impl (`firmware/CMakeLists.txt` + root `Makefile`) · test (`make lite` green; unknown board fails with FATAL_ERROR listing ports) · docs (`BUILDING.md`).
- Deps: S-02-001

### S-02-003 — `boards/lite/board_config.h` complete
As a firmware engineer I want a complete `boards/lite/board_config.h` so that all Lite HAL code binds to documented pins and settings.
- AC: all pins, radios, clocks, partition table, and security fuses documented in the header; header passes the board-config parity CI check; compiles under `make lite`
- Meta: Shard=B | Type=Feature | Size=S | Prio=P0 | Status=DONE | SKU=L | PRD=— | Const=C-00,C-01
- Tasks: spec (pin map from doc 01) · design (header layout + `ss_hal` export) · impl (`firmware/boards/lite/board_config.h`) · test (compiles under `make lite`) · docs (doc 01 lockstep).
- Deps: S-02-002

### S-02-004 — `boards/alpha/board_config.h` skeleton
As a firmware engineer I want a `boards/alpha/board_config.h` skeleton so that Alpha bring-up can start against the shared HAL contracts.
- AC: skeleton compiles under `make alpha`; open fields carry TODO markers referencing `models/CATALOG`; parity CI check passes for defined fields
- Meta: Shard=B | Type=Feature | Size=S | Prio=P0 | Status=DONE | SKU=A | PRD=— | Const=C-00
- Tasks: spec locked-vs-open Alpha fields from models/CATALOG + doc 00 · design define-set parity with lite, TODO(models/CATALOG) markers, parity checker + CI wiring · impl `firmware/boards/alpha/board_config.h`, `tools/board-parity.py`, board-parity workflow, alpha in the firmware-build matrix · test parity green locally, alpha container build green in CI · docs changelog
- Deps: S-02-002, S-02-003

### S-02-005 — `boards/omega/board_config.h` skeleton
As a firmware engineer I want a `boards/omega/board_config.h` skeleton so that Omega bring-up can start against the shared HAL contracts.
- AC: skeleton compiles under `make omega`; open fields carry TODO markers referencing `models/CATALOG`; parity CI check passes for defined fields
- Meta: Shard=B | Type=Feature | Size=S | Prio=P0 | Status=DONE | SKU=O | PRD=— | Const=C-00
- Tasks: spec locked-vs-open Omega fields from doc 00 + models/CATALOG · design mirror the S-02-004 skeleton pattern (define-set parity, TODO markers, base caps only) · impl `firmware/boards/omega/board_config.h` + omega in the firmware-build matrix · test parity green locally, omega container build green in CI · docs changelog
- Deps: S-02-002, S-02-003, S-02-004

### S-02-006 — FreeRTOS config with tick=1 kHz, priority ceiling, stack hooks
As a firmware engineer I want the FreeRTOS baseline configured so that scheduling behaviour is deterministic across all boards.
- AC: tick rate 1 kHz; priority-ceiling policy documented and enforced; stack-overflow hooks installed and exercised by a unit test
- Meta: Shard=C | Type=Feature | Size=S | Prio=P0 | Status=IN_REVIEW | SKU=★ | PRD=— | Const=C-00
- Tasks: spec priority bands from existing task set (diag 5 / compass 8 / gnss 12 / coproc 14) · design `ss_tasks.h` named-priority bands + ceiling + `ss_task_create()` wrapper + grep-enforcement tool · impl header/wrapper, migrate all four call sites, explicit canary config, Unity test case (execution pending test framework) · test policy-check green, all-board CI builds green · docs policy section in header + changelog
- Deps: S-02-002, S-02-003; test execution needs S-02-014/S-02-015 (framework) — story parks at IN_REVIEW until then

### S-02-007 — `ss_log` component with levels, redaction, colorized console
As a firmware engineer I want the `ss_log` component with levels and redaction so that diagnostics never leak key material.
- AC: `SS_LOGI/W/E/D` macros available; `%k` (key) tokens redacted at every level; CI test proves redaction; colorized console output
- Meta: Shard=E | Type=Feature | Size=M | Prio=P0 | Status=DONE | SKU=★ | PRD=NF-SEC-03 | Const=C-00,C-05
- Tasks: spec redaction invariants (data-independent placeholder, pointer never dereferenced, all levels/builds) · design IDF-free formatter core + spec-parsing arg consumption + macro/sink layer (T2 contract) · impl `firmware/components/ss_log/` + host test harness + `host-tests` CI workflow · test host tests green locally + CI (ASan/UBSan), 3-board container build green · docs header policy block + changelog
- Deps: S-02-002

### S-02-008 — Panic handler dumps to flash + reboots
As a firmware engineer I want a panic handler that dumps to flash and reboots safely so that field crashes are diagnosable without a debugger.
- AC: crash record decodes with `xtensa-decode-crash.py`; no crash loop after 3 consecutive panics; safe-mode boot entered instead
- Meta: Shard=E | Type=Feature | Size=M | Prio=P0 | Status=IN_REVIEW | SKU=★ | PRD=— | Const=C-00
- Tasks: spec panic sources + dump-hygiene contract from doc 05 · design coredump partition + ELF dump config + RTC-noinit panic counter + safe-mode boundary vs S-02-016 · impl partitions.csv, sdkconfig, ss_panic_guard (pure core + IDF glue), main wiring, decode tool · test host gtests for the decision core, 3-board CI build; on-hardware decode + loop-breaker exercise pending flash access · docs contract block + changelog
- Deps: S-02-006, S-02-009, S-02-014; on-hardware verification needs S-02-015 + attached board — story parks at IN_REVIEW until then

### S-02-009 — Task-watchdog + interrupt-watchdog policy
As a firmware engineer I want task and interrupt watchdog policy so that hung tasks recover automatically.
- AC: TWDT default 5 s; per-task registration API available; synthetic hang tests trigger both TWDT and IWDT
- Meta: Shard=F | Type=Feature | Size=S | Prio=P0 | Status=IN_REVIEW | SKU=★ | PRD=— | Const=C-00
- Tasks: spec per-task feed periods vs 5 s deadline from existing loops · design explicit TWDT/IWDT config incl. panic-on-hang + `ss_task_wdt_*` API in the ss_tasks policy layer · impl config, API, register qualifying tasks, staged synthetic hang tests · test policy gates green, 3-board CI build green (hang-test execution needs S-02-014/S-02-015 runner) · docs policy block + changelog
- Deps: S-02-006; test execution needs S-02-014/S-02-015 — story parks at IN_REVIEW until then

### S-02-010 — Boot-time budget instrumentation
As a release manager I want boot-time budget instrumentation so that boot regressions are caught in CI, not in the field.
- AC: milestone timestamps emitted in the log; CI asserts `app_main` < 400 ms on Lite; boot-time report archived per build
- Meta: Shard=D | Type=Feature | Size=S | Prio=P1 | Status=IN_REVIEW | SKU=★ | PRD=NF-PERF-05 | Const=C-00
- Tasks: spec milestone set from the main.cpp boot order · design `ss_bootmark` marks + machine-parseable boot-report line + log-parsing budget checker · impl `firmware/main/ss_bootmark.{h,cpp}`, main.cpp marks, `tools/boot-budget-check.py` + fixtures, checker self-test in CI · test checker self-test green (pass/fail fixtures), 3-board build green; live <400 ms assertion needs a boot log (hardware/QEMU) · docs changelog
- Deps: S-02-006; live measurement needs S-02-015 + board — story parks at IN_REVIEW until then

### S-02-011 — Heap tracker + stack watermark task
As a firmware engineer I want a heap tracker and stack-watermark task so that memory pressure is visible before it causes faults.
- AC: heap high-water and fragmentation stats logged periodically; per-task stack watermark reported; IDLE0/1 CPU load sampled
- Meta: Shard=G | Type=Feature | Size=S | Prio=P1 | Status=IN_REVIEW | SKU=★ | PRD=— | Const=C-00
- Tasks: spec metric set (free/min-free/largest-block per heap caps region, per-task stack high-water, IDLE0/1 load from run-time-stats deltas) + machine-parseable memwatch log line · design `ss_memwatch` low-prio periodic task (static stack, no heap on its own path) over `heap_caps_get_info` + `uxTaskGetSystemState`; enable `FREERTOS_USE_TRACE_FACILITY` + `FREERTOS_GENERATE_RUN_TIME_STATS` in sdkconfig.defaults · impl `firmware/main/ss_memwatch.{h,cpp}` with an IDF-free formatting/threshold core, wired in main.cpp · test host gtest suite for the core in `firmware/test/host`; firmware build green · docs changelog
- Deps: S-02-006 (IN_REVIEW — config in-tree, live evidence pending), S-02-007, S-02-014; live periodic-log capture needs a board — story parks at IN_REVIEW until then

### S-02-012 — Brown-out detector policy + save-state
As a firmware engineer I want a brown-out detector policy with save-state so that low-battery events do not corrupt user data.
- AC: brown-out IRQ triggers state save within the power-hold budget; NVS writes atomic across brown-out; recovery verified in a HIL brown-out test
- Meta: Shard=F | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=— | Const=C-00

### S-02-013 — Component template scaffold (`tools/new-component.sh`)
As a firmware engineer I want a component template scaffold so that new components start with correct structure, tests, and license headers.
- AC: `tools/new-component.sh` generates a buildable component; generated component includes a unit-test stub and SPDX header; semver metadata fields present
- Meta: Shard=H | Type=Feature | Size=S | Prio=P2 | Status=DRAFT | SKU=★ | PRD=— | Const=C-00

### S-02-014 — Host-side gtest baseline for pure-C code
As a firmware engineer I want a host-side gtest baseline so that pure-C logic is tested without hardware.
- AC: gtest target builds and runs on the host; mocked HAL headers available; coverage report wired into CI
- Meta: Shard=I | Type=Feature | Size=M | Prio=P0 | Status=DONE | SKU=★ | PRD=— | Const=C-00
- Tasks: spec host-testable surface (IDF-free cores + HAL-header consumers) · design CMake host project with SHA-pinned googletest FetchContent, mocks/ HAL header set, gcovr coverage in host-tests workflow · impl `firmware/test/host/` + port ss_log_format suite to gtest + mocked-HAL smoke test · test local build+run+coverage, CI green · docs BUILDING.md host-test section + changelog
- Deps: S-02-002, S-02-007

### S-02-015 — On-target Unity test framework wired in
As a firmware engineer I want the on-target Unity test framework wired in so that hardware-dependent code is testable on real boards.
- AC: Unity test app flashes and runs on target; results reported over serial to CI; at least one HAL smoke test passes on Lite
- Meta: Shard=I | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-00

### S-02-016 — Safe-mode / recovery boot path
As a device owner I want a safe-mode recovery boot path so that a bad update or corrupt state never bricks my device.
- AC: holding BOOT for 3 s enters recovery; recovery offers OTA rollback and factory reset; rollback preserves anti-rollback security guarantees
- Meta: Shard=D | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=NF-REL-03 | Const=C-00,C-05

### S-02-017 — NVS namespace scheme + versioning
As a firmware engineer I want a versioned NVS namespace scheme so that persisted settings survive firmware upgrades.
- AC: `ss_nvs_get/set(namespace, key, ver)` API implemented; migration hook invoked on version mismatch; migrations covered by unit tests
- Meta: Shard=B | Type=Feature | Size=S | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-00

### S-02-018 — RTC & wall clock source (GNSS/NTP fallback)
As a firmware engineer I want an RTC and wall-clock source with GNSS/NTP fallback so that timestamps stay trustworthy on and off grid.
- AC: monotonic and wall clocks exposed via one API; source priority GNSS → NTP → RTC documented and tested; clock survives sleep/wake cycles
- Meta: Shard=D | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=— | Const=C-00

### S-02-019 — Build attestation SBOM emitted per firmware artifact
As a release manager I want an SBOM attestation emitted per firmware artifact so that releases meet CRA disclosure duties.
- AC: CycloneDX JSON emitted per artifact; SBOM signed by CI; SBOM published alongside the release
- Meta: Shard=A | Type=Ops | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=NF-SEC-05,NF-REG-03 | Const=C-00,C-OA

### S-02-020 — Firmware version resource (git SHA + tag + build id)
As a support engineer I want a firmware version resource so that any device can report exactly what it runs.
- AC: git SHA, tag, and build id compiled into the image; values exposed via API and printed at boot; values match the release artifact metadata
- Meta: Shard=A | Type=Feature | Size=XS | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-00

### S-02-021 — Fixed-pool allocator contract for frame/message hot paths
As a firmware engineer I want a fixed-size pool allocator with a frozen contract for RNS frames, LXMF payloads, and voice frames so that hot paths never touch the general heap and exhaustion behaviour is defined instead of accidental.
- AC: a `ss_pool` API (create/alloc/free/stats) is specified in a header with documented pool sizes per subsystem derived from the RAM budget in `01_SS-SP_LITE_HARDWARE_REFERENCE.md`; exhaustion behaviour is explicit per pool (fail-fast with counter for bulk, pre-emption of lowest-priority buffers for VOICE/ALERT QoS classes) and unit-tested; zero general-heap allocations occur on the frame RX/TX path under load test, verified with the S-02-011 heap tracker; pool high-water marks are exposed via diagnostics
- Meta: Shard=— | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=NF-PERF-03 | Const=C-00

### S-02-022 — Bootloader-stage early-boot crash counter
As a firmware engineer I want a crash-loop breaker that also covers failures before `app_main` so that bootloader-stage, init-hook, or global-constructor crash loops cannot reboot-cycle forever below the S-02-008 gate.
- AC: a counter mechanism owned by the 2nd-stage bootloader (or verified equivalent) detects N consecutive resets that never reach the S-02-008 boot gate and diverts to a recoverable state; interplay with secure-boot and OTA rollback (EPIC-08/09) is specified before implementation; the S-02-008 gate's scope note is updated to reference the delivered mechanism
- Meta: Shard=E | Type=Feature | Size=M | Prio=P2 | Status=DRAFT | SKU=★ | PRD=— | Const=C-00
