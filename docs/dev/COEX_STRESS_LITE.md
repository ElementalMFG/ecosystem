<!-- SPDX-License-Identifier: CC-BY-4.0 -->
<!-- Wi-Fi/BLE coexistence stress-test plan: single-antenna 2.4 GHz soak on Lite (S-03-025). -->
<!-- Results pending the D-0013 hardware session. -->
# Wi-Fi/BLE Coexistence Stress Test — Lite

Coexistence stress-test plan for the Lite board (Elecrow CrowPanel Advance
3.5″, ESP32-S3 + SX1262 + Wi-Fi/BLE). The Lite's ESP32-S3 shares **one**
2.4 GHz antenna between its Wi-Fi and BLE radios, so the two bearers must
time-share the RF front end under the ESP-IDF software coexistence scheduler.
This plan closes **EPIC-03** risk **R03-02** — "Wi-Fi/BLE coexistence on
2.4 GHz → ESP-IDF coex config + scheduling test"
([`EPIC-03/EPIC.md`](../portfolio/epics/EPIC-03-hal-lite/EPIC.md)) — by
soaking both bearers concurrently and proving the BLE link survives sustained
Wi-Fi load within defined packet-loss thresholds.

This document is a **peer of** the HIL rack plan and **reuses its rack** — it
does not stand up a second fixture. The self-hosted `[self-hosted, hil, lite]`
runner, the co-located Wi-Fi AP + BLE central peer, and the DUT wiring are all
defined in [`HIL_TEST_PLAN_LITE.md`](HIL_TEST_PLAN_LITE.md) (S-03-023) and
cross-linked here, not re-specified.

**Status: awaiting the D-0013 hardware session; results tables intentionally
empty until measured on hardware** (see
[`09_VENTURE_EXECUTION_MAP.md`](../portfolio/09_VENTURE_EXECUTION_MAP.md),
VA-03, and [`EPIC-03/EPIC.md`](../portfolio/epics/EPIC-03-hal-lite/EPIC.md)).
The D-0013 Lite dev-fleet has not been built yet, so this is a plan; the
threshold values below are **defined targets / design parameters**, and
**nothing below reports a measured figure**.

## 1. Why coexistence is a risk

Both mandatory 2.4 GHz bearers are native to the ESP32-S3 and share a single
antenna on the Lite:

- **F-BR-02** — Wi-Fi 2.4 GHz, mandatory on all SKUs
  ([`02_PRD.md`](../portfolio/02_PRD.md) §Bearers).
- **F-BR-03** — Bluetooth LE 5.x, mandatory on all SKUs
  ([`02_PRD.md`](../portfolio/02_PRD.md) §Bearers).

Constitution **C-08** treats both as first-class SS-Link bearers that must run
identically to the other radios
([`08_UNIVERSAL_CONNECTIVITY.md`](../../08_UNIVERSAL_CONNECTIVITY.md#1-bearer-inventory-and-role)).
Because there is only one radio and one antenna, Wi-Fi TX/RX and BLE
advertising/connection events cannot be on-air simultaneously; the ESP-IDF
software coexistence scheduler must interleave them. The failure mode this
plan guards against is a **BLE link starved or dropped** while Wi-Fi runs at
full throughput, or Wi-Fi throughput collapsing when BLE holds the radio. That
is exactly the R03-02 risk, and the mitigation named there is "ESP-IDF coex
config + scheduling test" — this document is that test plan.

## 2. Software-coex configuration

The single antenna requires the **software coexistence** scheduler, which
time-shares the radio between the two protocol stacks rather than assuming
independent front ends:

- `CONFIG_ESP_COEX_SW_COEXIST_ENABLE=y` — enable the ESP-IDF software
  coexistence arbiter (required whenever Wi-Fi and BLE run at once on a shared
  radio).
- Coexistence **preference / priority** — the arbiter is run in one of three
  policies, exercised as a plan parameter:
  - **balance** — default fair time-share between the two stacks;
  - **Wi-Fi-priority** — Wi-Fi windows win contention (throughput-favoured);
  - **BLE-priority** — BLE connection events win contention (link-stability
    favoured).

The soak runs the **balance** policy as the baseline and re-runs the two
priority skews to bound the trade-off. This scheduler is the software
component under test; the radios it arbitrates are brought up through the HAL:

- **Wi-Fi** via
  [`ss_hal_radio_wifi.h`](../../firmware/components/ss_hal/include/ss_hal_radio_wifi.h)
  — `ss_wifi_init` / `ss_wifi_config` / `ss_wifi_start`.
- **BLE** via
  [`ss_hal_radio_ble.h`](../../firmware/components/ss_hal/include/ss_hal_radio_ble.h)
  — `ss_ble_advertise`, `ss_ble_scan_start`.

App code selects these bearers through `ss_hal_has_cap()`; this plan only
verifies that the two HAL surfaces coexist on one antenna under the coex
scheduler, not the bearer-selection policy above them.

## 3. Soak topology (reuses the HIL rack)

The topology **reuses the co-located Wi-Fi AP + BLE central peer** and the
self-hosted runner already drawn in
[`HIL_TEST_PLAN_LITE.md`](HIL_TEST_PLAN_LITE.md#2-rack-topology) §2 — see that
rack drawing rather than a second one here. The coex-specific overlay is
minimal: the DUT runs a **Wi-Fi STA carrying iperf traffic** *concurrently*
with a **BLE connection to the central peer** maintaining a GATT link with
periodic notifications.

```text
        ┌──────────────────────────┐   Wi-Fi (iperf, air)   ┌──────────────┐
        │  Lite DUT (ESP32-S3)     │◀──────────────────────▶│ Wi-Fi AP +   │
        │  one 2.4 GHz antenna     │                        │ BLE central  │
        │  Wi-Fi STA + BLE conn.   │◀── BLE conn (air) ─────▶│ (radio peer) │
        └──────────────────────────┘   GATT notifications   └──────────────┘
                shared radio, time-shared by SW coex scheduler
```

- The DUT associates as a Wi-Fi **STA** to the AP peer and holds a **BLE
  connection** to the same peer's central role for the whole soak.
- Both links are on the single DUT antenna; the coex scheduler (§2)
  interleaves them.
- Console, flashing, and artifact capture are the runner's job exactly as in
  [`HIL_TEST_PLAN_LITE.md`](HIL_TEST_PLAN_LITE.md#2-rack-topology).

## 4. Load profile

Stated as **plan parameters**, not results:

- **Wi-Fi (iperf):**
  - **TCP soak** and a separate **UDP soak** (UDP fixes the offered rate so
    packet loss is measurable).
  - **Direction:** run **uplink**, **downlink**, and **bidirectional** passes.
  - **Duration:** each direction soaks for **≥ 1 h continuous** (a **12 h**
    overnight bidirectional run is the endurance parameter).
- **BLE keep-alive (must survive the Wi-Fi load):**
  - **Connection interval:** target **30 ms** (parameter; within the 7.5 ms–
    4 s BLE range).
  - **Notification cadence:** peer-driven GATT notification every **1 s** as a
    liveness keep-alive; each notification carries a monotonic counter so the
    peer can detect gaps.

The BLE connection is established **before** the Wi-Fi soak starts and must be
continuously maintained across the full soak duration.

## 5. Coexistence thresholds (defined targets)

These are the **defined packet-loss thresholds** the AC calls for. Every value
here is a **design target / parameter**, not a measurement; Measured and
Pass/Fail columns are filled at D-0013 (§6).

| # | Metric | Target (design parameter) | Measured (D-0013) | Pass/Fail |
|---|---|---|---|---|
| C1 | BLE unexpected-disconnect count over the full soak | **target 0** | | |
| C2 | BLE notification loss | **target ≤ 0.5 %** of expected notifications | | |
| C3 | Wi-Fi throughput floor under concurrent BLE (TCP) | **target ≥ 10 Mbps** floor | | |
| C4 | Wi-Fi UDP packet loss under concurrent BLE | **target ≤ 1 %** | | |
| C5 | Bidirectional degradation (aggregate UL+DL throughput vs. Wi-Fi-only baseline) | **target ≤ 20 %** drop | | |

Notes:

- **C1** is the hard AC clause "BLE connection maintained under sustained
  Wi-Fi load" — any unexpected disconnect is a fail.
- **C3** the throughput floor is a **defined target**, not a measured rate; the
  Wi-Fi-only baseline it compares against is captured in the same session with
  BLE idle.
- Targets follow the datasheet-typical labelling convention of
  [`POWER_BUDGET_LITE.md`](POWER_BUDGET_LITE.md) §3 — documented now, confirmed
  on silicon at D-0013.

## 6. Results (fill at D-0013)

| Row | Target | Measured | Pass/Fail |
|---|---|---|---|
| C1 BLE unexpected disconnects (balance) | 0 | | |
| C2 BLE notification loss (balance) | ≤ 0.5 % | | |
| C3 Wi-Fi TCP throughput floor (balance) | ≥ 10 Mbps | | |
| C4 Wi-Fi UDP packet loss (balance) | ≤ 1 % | | |
| C5 Bidirectional degradation (balance) | ≤ 20 % | | |
| C1 BLE unexpected disconnects (BLE-priority) | 0 | | |
| C3 Wi-Fi TCP throughput floor (Wi-Fi-priority) | ≥ 10 Mbps | | |
| Endurance — 12 h bidirectional soak | no disconnect; thresholds held | | |

## 7. CI trigger and result reporting

**Trigger.** The coex soak runs as an **additional job on the same
self-hosted HIL runner** (`runs-on: [self-hosted, hil, lite]`), alongside the
conformance rack of [`HIL_TEST_PLAN_LITE.md`](HIL_TEST_PLAN_LITE.md#5-ci-trigger-and-result-reporting)
§5 — it does not add a second runner label. Because the D-0013 fleet does not
exist yet, the job is **`workflow_dispatch`-only and non-blocking**; when the
fleet lands it is promoted to a merge gate on `firmware/**` changes (path-
filtered), mirroring the HIL rack promotion. Until then no runner is online to
answer, so it may not block merges.

**Execution.** The runner builds `make lite`, flashes the DUT, brings up the
Wi-Fi STA + BLE connection under the coex config (§2), and drives the iperf and
BLE keep-alive load profile (§4) against the co-located AP + BLE central peer.

**Result reporting.** The wrapper converts **per-radio** outcomes into **JUnit
XML** — one test case per threshold row (C1–C5) per coex policy — surfaced as a
PR check with per-radio PASS/FAIL, exactly as the HIL rack surfaces per-domain
results. Raw **iperf logs** and **BLE link traces** (connection events,
notification counter gaps, any disconnect reasons) are uploaded as build
artifacts for audit.

**Green condition.** The check is green iff, across the balance baseline and
both priority skews, C1 = 0 disconnects **and** every threshold in §5 holds.
Any disconnect, or any breached threshold, is a fail with the offending metric
and coex policy named in the check summary.
