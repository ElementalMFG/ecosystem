<!-- SPDX-License-Identifier: CC-BY-4.0 -->
<!-- Bench procedure: Lite sleep current + wake latency (S-03-003, NF-PWR-01) -->
<!-- Results pending the D-0013 hardware session. -->
# Bench Procedure — Lite Sleep Current & Wake Latency

Bench procedure to verify the Lite (Elecrow CrowPanel Advance 3.5″, ESP32-S3)
sleep-power and wake behaviour against **NF-PWR-01** ("Lite standby ≥ 24 h with
LoRa periodic"). It measures sleep current with the canonical wake set armed and
wake-to-responsive latency for each wake source.

**Status: results pending the D-0013 hardware session** (see
[`09_VENTURE_EXECUTION_MAP.md`](../portfolio/09_VENTURE_EXECUTION_MAP.md), VA-03).
The tables below are intentionally empty until measured on hardware.

## Acceptance targets

- **Sleep current ≤ 0.5 mA** with the canonical wake set armed.
- **Standby ≥ 24 h** with a LoRa periodic duty cycle (NF-PWR-01), implied by the
  current target and the battery capacity of the D-0013 fleet.
- **Wake-to-responsive latency** recorded (no hard ceiling this story; captured
  as a baseline) for each of: touch INT, LoRa DIO1, RTC timer.

## Canonical wake set (device under test)

Armed via `ss_power_wake_lite_defaults()` (see
[`ss_power_lite.h`](../../firmware/components/ss_power/include/ss_power_lite.h)):

| Source     | GPIO | Wake level     | Light sleep | Deep sleep |
|------------|------|----------------|-------------|------------|
| Touch INT  | 47   | 0 (LOW-active) | yes         | no (not RTC-capable) |
| LoRa DIO1  | 1    | 1 (HIGH-active)| yes         | yes        |
| RTC timer  | —    | —              | yes         | yes        |

The RTC timer is armed separately by the duty-cycle owner via
`ss_power_wake_timer_set()`.

## Equipment

- Lite board (D-0013 fleet unit), USB-Serial-JTAG console for logs.
- DC current meter (µA-resolution, e.g. Nordic PPK2 or a bench DMM with µA
  range) wired **in series** with the board supply.
- Bench PSU or characterised battery for the standby estimate.
- Stopwatch / logic analyser for wake-latency timing (timestamp the wake edge vs
  the first application-responsive log line).

## Procedure

### A. Light-sleep current

1. Flash a build that arms the canonical wake set and enters light sleep
   (`ss_power_enter(SS_PWR_STATE_LIGHT_SLEEP)`), no timer armed.
2. Let the board settle; record steady-state current from the series meter.
3. Record in the results table.

### B. Deep-sleep current

1. Flash a build that arms the canonical wake set and enters deep sleep
   (`ss_power_enter(SS_PWR_STATE_DEEP_SLEEP)`).
2. Let the board settle; record steady-state current.
3. Record in the results table.

### C. Wake-to-responsive latency (per source)

For each of touch INT (GPIO47, drive LOW), LoRa DIO1 (GPIO1, drive HIGH) and RTC
timer (arm a short duration):

1. Enter the applicable sleep state with the source armed.
2. Trigger the wake event; timestamp the trigger edge.
3. Timestamp the first application-responsive log line after wake.
4. Record the delta.

Note: touch INT (GPIO47) is **light-sleep only** — it will not wake the board
from deep sleep; that row applies to light sleep only.

## Results (fill at D-0013)

### Sleep current

| Measurement           | Target      | Measured | Pass/Fail |
|-----------------------|-------------|----------|-----------|
| Light-sleep current   | ≤ 0.5 mA    |          |           |
| Deep-sleep current    | ≤ 0.5 mA    |          |           |
| Standby estimate (h)  | ≥ 24 h      |          |           |

### Wake-to-responsive latency

| Source     | Sleep state | Measured latency | Notes |
|------------|-------------|------------------|-------|
| Touch INT  | light       |                  | GPIO47 low-active |
| LoRa DIO1  | light       |                  | GPIO1 high-active |
| LoRa DIO1  | deep        |                  | reboot-path wake |
| RTC timer  | light       |                  |       |
| RTC timer  | deep        |                  | reboot-path wake |

## Pass/fail criteria

- **PASS** iff light-sleep and deep-sleep current are each ≤ 0.5 mA and the
  standby estimate is ≥ 24 h with the LoRa periodic duty cycle.
- Wake-latency figures are recorded as a baseline; a regression against these
  numbers in a later session is a follow-up investigation, not a fail here.
