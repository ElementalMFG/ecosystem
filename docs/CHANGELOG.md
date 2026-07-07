<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# Changelog

All notable changes to SS-SP. Format: [Keep a Changelog](https://keepachangelog.com/en/1.1.0/);
versioning: SemVer per released artifact once releases begin. Update this file
in every user-visible PR (CONTRIBUTING.md §3).

## [Unreleased]

### Added

- Working-group charters ratified (S-01-009 / D-0017): 12 charter files in
  `governance/wg/` (bootstrap staffing — founding maintainer chairs all);
  `wg-community` + `wg-legal` added to the doc 06 §3 table; S-01-012 recorded
  BLOCKED pending a second maintainer (D-0014) and org teams (D-0015).
- CLA gate live-verified on PR #2: bot blocked the unsigned PR, signature
  recorded on `cla-signatures` (`signatures/cla.json`), and the `cla` check
  promoted to a required status check on `main` per D-0016.
- Security-intake keys live (VA-04 / S-01-005): ed25519 PGP
  `10D6 DB6D 3C45 15F6 2437 C788 C314 E666 80DD 9FDD` + age recipient
  published in `SECURITY.md`; armored public key at
  `governance/ss-sp-security-intake.pub.asc`; private keys/passphrase/
  revocation cert in maintainer custody outside the repo.
- CLA per owner decision A5 / D-0016: normative text `docs/CLA.md`
  (grantee Elemental MFG), `contributor-assistant` bot workflow
  `.github/workflows/cla.yml` recording signatures on `cla-signatures`;
  CONTRIBUTING §5 links it (S-01-008 IN_PROGRESS).
- D-0015 (`governance/decisions.md`): `ElementalMFG` is a personal GitHub
  account, not an org — accepted for solo bootstrap; org conversion required
  before second maintainer / foundation transition (amends D-0011).
- D-0014 (`governance/decisions.md`): branch protection on `main` with
  required checks `dco`/`lint-docs`/`build (lite)` and founder bypass during
  solo bootstrap; private vulnerability reporting + Discussions enabled, wiki
  disabled. Closes VA-02 (owner decision A6).
- Owner decisions recorded as D-0011..D-0013 (`governance/decisions.md`):
  hosting at `github.com/ElementalMFG/ecosystem` (public), Elemental MFG
  (Idaho) as parent entity with interim `elementalmfg.com` contact addresses,
  and the HaLow-fitted 2-unit CrowPanel dev fleet as the Lite v1 spec-lock
  path (`docs/OWNER_DECISIONS.md` answered in place).
- PostToolUse clang-format hook (`tools/claude/format-c-hook.sh` +
  `.claude/settings.json`): auto-formats agent-edited C/C++ except the
  hand-aligned T1 trees `firmware/boards/**` and
  `firmware/components/ss_hal/**`.

- Constitution set docs 00–08, methodology + portfolio (24 epics / 533
  stories), model-allocation strategy (doc 10), venture execution map (doc 09).
- Governance baseline: CODE_OF_CONDUCT, SECURITY.md, CONTRIBUTING with DCO
  gate, RFC template + RFC-0001/0002, decisions log D-0001..D-0010.
- Firmware scaffold: digest-pinned ESP-IDF v5.3.5 container, `make
  lite|alpha|omega` board wrapper, Lite board config, `ss_hal` contracts,
  boot display / UART engine / compass / diag bring-up code.
- Tooling: `tools/lint-docs.py`, `tools/gen-stories-index.py` (now enforcing
  §2.7 story elaboration), Claude Code tier agents + rules + skills.
- Project governance files: `.clang-format`, CODEOWNERS, issue + PR
  templates, dependabot (GitHub Actions), this changelog.
- Panic post-mortem + crash-loop breaker (S-02-008, T1 double-reviewed):
  dedicated 64 KiB `coredump` partition (encrypted-flagged for EPIC-08),
  ELF core dumps on every panic path with a dedicated dump stack and
  DRAM capture pinned OFF (hygiene contract in `ss_panic_guard.h`);
  integrity-guarded RTC-noinit consecutive-panic counter with three-way
  reset classification (fail-safe on unknown reasons) gates boot into a
  minimal safe mode after 3 consecutive panics; 60 s stability window;
  `tools/xtensa-decode-crash.py` decode wrapper (format auto-detect,
  RISC-V-ready); duress wipe now explicitly covers the coredump
  partition (S-07-021 AC); early-boot loops tracked as S-02-022.
- Host-side gtest baseline (S-02-014): `firmware/test/host/` CMake
  project with SHA-pinned googletest (v1.15.2), mocked `board_config.h`
  + real host-clean HAL headers, the full ss_log redaction suite ported
  to gtest (11 tests / 39 assertions, sabotage-verified), and a
  `gtest-coverage` CI job producing a gcovr report (90.2% lines on the
  redaction core) with a Cobertura artifact; host-test docs added to
  BUILDING.md.
- Watchdog policy (S-02-009): explicit TWDT (5 s, panic-on-hang so
  hung tasks recover by reboot) and IWDT config in
  `sdkconfig.defaults`; `ss_task_wdt_register/feed/unregister` API in
  the ss_tasks policy layer with a documented register-vs-exempt rule;
  compass task registered (worst-case ~1 s loop), event-driven pumps
  and 30 s housekeeping exempted with in-code rationale; synthetic
  TWDT/IWDT hang tests staged for the S-02-014/015 runner.
- `ss_log` component (S-02-007): `SS_LOGE/W/I/D` macros with ANSI
  colorized console and runtime level filter; `%k` redaction token —
  consumes the pointer without ever dereferencing it and always emits
  a data-independent `[REDACTED]` at every level in every build; the
  formatter core is IDF-free and proven by a host test suite (39
  checks incl. differential matrix vs `vsnprintf`, ASan/UBSan, and a
  sabotage test) run by the new `host-tests` CI workflow.
- FreeRTOS baseline (S-02-006): named priority bands with an enforced
  application ceiling (`firmware/main/ss_tasks.h`, wrappers asserting
  1..SS_PRIO_CEILING), all four existing tasks migrated off numeric
  priorities, explicit stack-overflow canary in `sdkconfig.defaults`,
  new `tools/task-policy-check.py` CI gate rejecting raw
  `xTaskCreate*` calls and numeric-literal priorities, and a Unity
  test case staged for the S-02-014/015 test runner.
- Omega board port skeleton (S-02-005): `firmware/boards/omega/
  board_config.h` with full define-set parity (104), MCU honestly
  recorded as TBD (RISC-V/Linux SoM candidate per doc 00 §1.2), no
  radio capabilities claimed, 93 `TODO(models/CATALOG)` markers;
  `omega` joins the firmware-build CI matrix — all three board ports
  now compile-gated on every firmware PR.
- Alpha board port skeleton (S-02-004): `firmware/boards/alpha/
  board_config.h` with the full 104-define parity set from the Lite
  reference — locked facts only (ESP32-P4NRW32X, 32 MB PSRAM, 320×240
  display, 12× bezel LEDs), 89 `TODO(models/CATALOG)` markers for open
  hardware; new `tools/board-parity.py` gate + `board-parity` CI
  workflow; `alpha` added to the firmware-build CI matrix.
- RFC-0003 — protocol compatibility & deprecation policy (S-01-017,
  D-0018): per-surface supported-version windows and lead times
  (≥2 minor releases and ≥12 months), sunset via new
  `docs/DEPRECATIONS.md`, security fast-track for broken crypto suites;
  cited by the version-negotiation and SDK-policy stories, linked from
  `02_PROTOCOL_STACK.md` §11, and enforced by a new `wire-format-policy`
  CI check requiring `protocol/**` PRs to cite RFC-0003.
- Anti-rug-pull covenant test suite (S-01-015): published artifacts
  hash-anchored in append-only `governance/covenant-anchors.json`;
  `tools/covenant-check.py` fails on any retroactive license change,
  immutable-text change (LICENSE/NOTICE), or ledger rewrite; `covenant`
  CI workflow runs it on every PR and every merge to `main`.
- Typed PR templates for `feature`, `rfc`, `security`, and `docs` changes
  under `.github/PULL_REQUEST_TEMPLATE/`, each listing its required
  sign-offs (DCO, CLA, CODEOWNER/wg-security approvals per CONTRIBUTING.md
  §9); referenced from CONTRIBUTING.md §3 and the default PR template
  (S-01-011).

### Changed

- Interim contact pass (D-0012): `security@elementalmfg.com` and friends wired
  into SECURITY.md, README, docs 05/06, CODE_OF_CONDUCT, TRADEMARK, issue
  templates (+ `config.yml` contact link); CONTRIBUTING §12 channels now
  Matrix + GitHub Discussions; CODEOWNERS + DCO identity moved to
  `dylan@elementalmfg.com`.
- HaLow-on-Lite alignment (D-0013): docs 00/01/README/PRD now state the dev
  fleet is HaLow-fitted via the wireless header; "HaLow (Alpha only)" scope
  lines corrected to native-radio-only.
- One-time `clang-format` conformance pass over `firmware/main/` so the new
  format hook produces no spurious diffs (build-verified in the pinned
  container).

### Fixed

- Required check `build (lite)` never reported on docs-only PRs (GitHub
  collapses a matrix job skipped at job level to the bare job name `build`,
  so the required context stayed EXPECTED forever): `firmware-build.yml` now
  runs an always-on `build` gate matrix job that carries the required context
  and passes on firmware success or a legitimate docs-only skip — verified
  live on a probe PR (all six checks green, MERGEABLE, no admin bypass).
- HAL component shadowing (`hal` → `ss_hal`) that broke the mbedtls port on
  the first containerized build.
- Statusline effort parsing when the statusline JSON carries a plain-string
  `effort`.
