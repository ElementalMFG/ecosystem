<!-- SPDX-License-Identifier: CC-BY-4.0 -->
<!-- Power budget roll-up: Lite per-state + per-bearer TX + standby model (S-03-024, NF-PWR-01). -->
<!-- Measured columns pending the D-0013 hardware session. -->
# Power Budget — Lite (roll-up)

Unified power budget for the Lite board (Elecrow CrowPanel Advance 3.5″,
ESP32-S3 + SX1262 + Wi-Fi/BLE) proving **NF-PWR-01** ("Lite standby ≥ 24 h with
LoRa periodic",
[`02_PRD.md`](../portfolio/02_PRD.md) §3.3). It is the roll-up that unifies the
whole Lite power model: every `ss_hal_power.h` power state, RX-idle, and the
per-bearer TX peak, plus the duty-cycle standby estimate that ties them to the
24 h target.

This document **complements** the two per-slice procedures and does not repeat
them:

- Sleep-current bench procedure (the sleep ≤ 0.5 mA slice) lives in
  [`BENCH_POWER_LITE.md`](BENCH_POWER_LITE.md) (S-03-003) — cross-referenced,
  not duplicated.
- The µA-meter-in-series fixture (Nordic PPK2 on the supply rail) is defined by
  the rack in [`HIL_TEST_PLAN_LITE.md`](HIL_TEST_PLAN_LITE.md) (S-03-023) —
  reused here, not re-specified.

**Status: results pending the D-0013 hardware session** (see
[`09_VENTURE_EXECUTION_MAP.md`](../portfolio/09_VENTURE_EXECUTION_MAP.md),
VA-03). The Measured columns and the standby-estimate arithmetic are
intentionally empty until measured on hardware; the datasheet-typical TX figures
below are documented now and confirmed on silicon at D-0013.

## 1. Acceptance targets

- **Sleep current ≤ 0.5 mA** with the canonical wake set armed (measured in
  [`BENCH_POWER_LITE.md`](BENCH_POWER_LITE.md)).
- **RX-idle ≤ 25 mA** with the radio in continuous-RX and the CPU idle.
- **TX peak documented per bearer** — LoRa, Wi-Fi, BLE (datasheet typical now,
  measured at D-0013; §3).
- **Standby ≥ 24 h** with a LoRa periodic duty cycle (NF-PWR-01), shown by the
  duty-cycle estimate in §5.

## 2. Power-state budget

One row per `ss_hal_power.h` `ss_power_state_t` state, plus RX-idle and the
per-bearer TX peaks. Targets are the acceptance ceilings where one exists,
otherwise the datasheet typical; the Measured column is filled from the PPK2
series trace at D-0013.

| State / mode | Target or datasheet | Measured (D-0013) | Pass/Fail |
|---|---|---|---|
| `SS_PWR_STATE_ON` (active, radios idle) | datasheet typical; no hard ceiling | | |
| `SS_PWR_STATE_LIGHT_SLEEP` | ≤ 0.5 mA (via [`BENCH_POWER_LITE.md`](BENCH_POWER_LITE.md)) | | |
| `SS_PWR_STATE_DEEP_SLEEP` | ≤ 0.5 mA (via [`BENCH_POWER_LITE.md`](BENCH_POWER_LITE.md)) | | |
| `SS_PWR_STATE_HIBERNATE` | ≤ 0.5 mA (maps to deep sleep today) | | |
| `SS_PWR_STATE_SHUTDOWN` | ≤ 0.5 mA (all wake sources disabled) | | |
| RX-idle (LoRa continuous-RX, CPU idle) | ≤ 25 mA (§4) | | |
| TX peak — LoRa (SX1262) | datasheet typical (§3) | | |
| TX peak — Wi-Fi 2.4 GHz | datasheet typical (§3) | | |
| TX peak — BLE 5 | datasheet typical (§3) | | |

Note: `SS_PWR_STATE_HIBERNATE` maps to `esp_deep_sleep_start()` today (see
[`ss_hal_power.h`](../../firmware/components/ss_hal/include/ss_hal_power.h)
HIBERNATE forward-note), so its current draw is expected to track deep sleep.

## 3. Per-bearer TX-peak budget

The AC word is **documented**: the datasheet-typical figures below satisfy it
now, and the Measured column is confirmed at D-0013. Every figure is a
**DATASHEET TYPICAL**, not a measured value — the actual draw depends on the
firmware TX-power setting and, for LoRa, the active **region PA table**
(`ss_lora_pa_core`, S-03-012, which clamps to the regional EIRP ceiling and the
SX1262 +22 dBm conducted limit).

| Bearer | IC | Datasheet TX-peak (typical) | Measured (D-0013) | Notes |
|---|---|---|---|---|
| LoRa | Semtech SX1262 | ~118 mA at +22 dBm (high-power PA) — DATASHEET TYPICAL | | Actual draw set by the region PA table / clamped TX power (S-03-012); +22 dBm is the SX1262 conducted ceiling. |
| Wi-Fi 2.4 GHz | ESP32-S3 (native) | ~355 mA peak, 802.11b @ ~+20.5 dBm — DATASHEET TYPICAL | | Actual draw depends on the PHY rate and TX-power setting; OFDM/HT rates draw less than 802.11b DSSS peak. |
| BLE 5 | ESP32-S3 (native) | ~30 mA at 0 dBm — DATASHEET TYPICAL | | Actual draw scales with the configured BLE TX power. |

Datasheet typicals are per the Semtech SX1262 and Espressif ESP32-S3 datasheets;
they are **not** measured on the Lite board. Firmware region/TX-power
configuration drives the real figure, so the Measured column is the figure of
record once the D-0013 session runs.

## 4. RX-idle measurement procedure

Target: **RX-idle ≤ 25 mA**. Reuses the Nordic PPK2 in series with the board
supply, per [`HIL_TEST_PLAN_LITE.md`](HIL_TEST_PLAN_LITE.md) §2–3 and
[`BENCH_POWER_LITE.md`](BENCH_POWER_LITE.md). The sleep-current procedure is
**not** repeated here — see [`BENCH_POWER_LITE.md`](BENCH_POWER_LITE.md).

1. Wire the PPK2 (or a bench DMM with a mA range) **in series** with the board
   supply; power the Lite unit from the PPK2 source or a characterised 3.7 V
   rail.
2. Flash a build that brings the radio up into **continuous-RX** with no TX and
   leaves the CPU idle (no display refresh, no active tasks beyond the idle
   loop).
3. Let the board settle; record the steady-state current from the series meter.
4. Record it in the §2 RX-idle row. **PASS** iff steady-state RX-idle ≤ 25 mA.

## 5. Standby-hours budget estimate

Duty-cycle model proving NF-PWR-01 (≥ 24 h). The Lite has **no battery gauge**
(C-01 §3.12), so the capacity is the user-supplied 3.7 V LiPo — stated here as
an assumption, not read from hardware.

**Assumptions (filled at D-0013):**

- `C_batt` — usable capacity of the user-supplied 3.7 V LiPo, in mAh.
- `I_sleep` — sleep baseline current (mA), from
  [`BENCH_POWER_LITE.md`](BENCH_POWER_LITE.md) (target ≤ 0.5 mA).
- `I_wake` — average current while awake for a periodic LoRa cycle (mA), from
  the RX-idle (§4) and TX-peak (§3) measurements.
- `t_wake` — awake window per periodic wake (s).
- `T` — wake period (s), armed via `ss_power_wake_timer_set()`
  ([`ss_hal_power.h`](../../firmware/components/ss_hal/include/ss_hal_power.h)).

**Arithmetic (symbolic):**

```text
duty      = t_wake / T                       (fraction of each period awake)
I_avg     = I_sleep + duty * (I_wake - I_sleep)   (mA, average current)
standby_h = C_batt / I_avg                    (hours)
```

**PASS** iff `standby_h ≥ 24` for the assumed `C_batt` and the chosen LoRa
periodic duty cycle. The numeric fill lands at D-0013, where measured
`I_sleep` (§2 / [`BENCH_POWER_LITE.md`](BENCH_POWER_LITE.md)), `I_wake`
(§3–4), and the assumed capacity feed the model.

## 6. Pass/fail criteria

**PASS** iff all of:

- Sleep current ≤ 0.5 mA (all sleep/hibernate/shutdown states, §2 /
  [`BENCH_POWER_LITE.md`](BENCH_POWER_LITE.md)), **and**
- RX-idle ≤ 25 mA (§4), **and**
- Per-bearer TX peak documented — datasheet typical now, measured at D-0013
  (§3), **and**
- Standby estimate ≥ 24 h under the LoRa periodic duty cycle (§5).

Any single breach is a fail, with the offending row named. Until the D-0013
fleet exists, this budget is documented (datasheet + procedure) and parks its
measured confirmation on the same hardware session as
[`BENCH_POWER_LITE.md`](BENCH_POWER_LITE.md) (S-03-003).
