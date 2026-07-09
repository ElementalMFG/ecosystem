<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# IN_REVIEW evidence runsheet

**What IN_REVIEW means in this portfolio:** the story's code/docs are merged,
gate-verified, and tier-review-complete (T1 stories carry both review verdicts
in their commit body). What is *pending* is **AC evidence** — a clause that can
only be proven by hardware, a fleet, a release event, or a dependent driver.
This runsheet is the single place those pending clauses live, grouped by the
fixture that discharges them.

**Exit rule (binding):** a story flips `IN_REVIEW → DONE` only when every
parked clause has linked evidence — a filled results-table row (value +
timestamp + firmware rev) in the named procedure doc, a CI run ID, or a release
URL. Flips are per-story `Story:` trailer commits (or one sweep commit per
fixture session) followed by index + allocation regeneration. Never flip on
"probably fine"; a failed clause blocks the flip and files a fix story.

Machine guard: `gen-stories-index.py --check` rejects any `IN_REVIEW`/`BLOCKED`
story whose `- Deps:` line records no park reason (added 2026-07-09).

## A — Software-dischargeable now (no hardware)

| Story | Pending clause | Discharged by |
|---|---|---|
| S-03-012 | on-target RF sweep applies clamped EIRP | S-03-011 (SX1262 driver — headless-eligible now) + D-0013 units |
| S-03-013 | on-air LBT/duty integration | S-03-011 + D-0013 units |
| S-02-019 | SBOM signed by CI + published with release | first tagged release exercising `release-sbom.yml` |

## B — Any single attached Lite board (bench session)

| Story | Pending clause | Procedure |
|---|---|---|
| S-02-006/009 | on-target FreeRTOS/WDT test execution | S-02-015 Unity harness, then re-run |
| S-02-008 | panic dump + crash-loop guard on hardware | Unity + manual panic trigger |
| S-02-010 | boot-time budget live measurement | boot instrumentation log capture |
| S-02-011 | heap/stack watermark periodic log | live capture over serial |
| S-02-015 | flash + serial-to-CI + smoke pass | first board = unblocks the whole B column |
| S-02-016 | recovery-window entry on hardware (partial — anti-rollback stays gated on EPIC-08/09) | manual BOOT-hold test |
| S-03-003 | sleep ≤ 0.5 mA, wake latency | `BENCH_POWER_LITE.md` |
| S-03-004 | GT911 gestures + PTT < 25 ms | on-board input test |
| S-03-006 | 60 FPS partial redraw, tear-free DMA | on-board display test |
| S-03-009/010 | I²S capture/playback on hardware | mic/speaker bench |
| S-03-019 | power-cut torture test | S-02-015 harness + board |
| S-03-022 | vectors runnable on target | Unity harness run |
| S-03-027 | live NMEA fix via UART1 | GNSS module attached |

## C — Two-unit fleet / HIL rack (D-0013 session proper)

| Story | Pending clause | Procedure |
|---|---|---|
| S-03-014/015 | STA connect + soft-AP portal against real phone/AP | Wi-Fi HIL rack (`HIL_TEST_PLAN_LITE.md`) |
| S-03-024 | measured sleep/RX-idle cells | `POWER_BUDGET_LITE.md` §4 |
| S-03-025 | coex soak thresholds met | `COEX_STRESS_LITE.md` (results table) |
| — | HIL matrix execution | S-03-045 (fills `HIL_TEST_PLAN_LITE.md` §4.1) |

## D — Later-epic-gated (do NOT wait for these to exit EPIC-02/03)

| Story | Gated clause | Gate |
|---|---|---|
| S-02-016 | full anti-rollback verification | EPIC-08 eFuse + EPIC-09 OTA images |

Maintenance: when a story parks at IN_REVIEW, add its row here in the same
commit; when evidence lands, strike the row in the flip commit.
