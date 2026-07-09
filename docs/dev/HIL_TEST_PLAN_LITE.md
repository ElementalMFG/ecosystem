<!-- SPDX-License-Identifier: CC-BY-4.0 -->
<!-- HIL rack test-plan: Lite conformance on real hardware (S-03-023). -->
<!-- Results pending the D-0013 hardware session. -->
# HIL Rack Test Plan — Lite

Hardware-in-the-loop (HIL) rack test plan for the Lite board (Elecrow
CrowPanel Advance 3.5″, ESP32-S3 + SX1262 + Wi-Fi/BLE). The rack exists to
close **EPIC-03** exit criterion 2 — "hardware-in-loop rack passes
power/audio/lora/wifi/ble conformance vectors" — and to cross-check the
power (crit. 4) and duty-cycle (crit. 3) budgets on real silicon.

**Status: awaiting the D-0013 hardware session; results tables intentionally
empty until measured on hardware** (see
[`09_VENTURE_EXECUTION_MAP.md`](../portfolio/09_VENTURE_EXECUTION_MAP.md),
VA-03, and [`EPIC-03/EPIC.md`](../portfolio/epics/EPIC-03-hal-lite/EPIC.md)).
The D-0013 Lite dev-fleet has not been built yet, so this is a plan; nothing
below reports a measured figure.

## 1. Design point — reuse the conformance vector core

The rack does **not** author a second test suite. It runs the *same*
`ss_hal_conformance_core.c` + `ss_hal_conformance_vectors.c` that already run
on the host against mocks
([`test_hal_conformance.cpp`](../../firmware/test/host/tests/test_hal_conformance.cpp)
via [`ss_hal_mock_env.c`](../../firmware/test/host/mocks/ss_hal_mock_env.c)),
except on-target the `ss_conf_env_t` `exec`/`reset`/`emit` hooks bind to the
real Lite HAL drivers instead of the mock environment (the target adapter is
S-02-015, deferred until a board exists). See
[`CONFORMANCE_SPEC.md`](../../firmware/components/ss_hal/test/conformance/CONFORMANCE_SPEC.md)
§5 for the host/target reuse contract.

Because the core and the vector tables are byte-identical between host and
target, the rack's pass condition is simply: the frozen actionable-diff
([`CONFORMANCE_SPEC.md`](../../firmware/components/ss_hal/test/conformance/CONFORMANCE_SPEC.md)
§4) emitted on-target is **empty** for every domain. Any `CONF-FAIL …` line
is a real hardware/driver divergence from the mock-verified semantics.

## 2. Rack topology

```text
                 ┌─────────────────────────────────────────────┐
                 │  Self-hosted CI runner (host controller)     │
                 │  labels: [self-hosted, hil, lite]            │
                 └───┬───────────┬──────────────┬───────────────┘
                     │ USB-JTAG  │ USB (PPK2)   │ LAN / USB
        flash + console          │ current      │ radio peers
                     │           │ traces       │
                     ▼           │              ▼
              ┌────────────┐     │        ┌──────────────┐
              │  Lite DUT  │◀────┘        │ Wi-Fi AP +   │
              │ ESP32-S3   │  power rail  │ BLE central  │
              │ +SX1262    │─────────────▶│ (radio peer) │
              └─────┬──────┘   in series  └──────▲───────┘
                    │ LoRa RF (conducted)        │
                    │                     Wi-Fi / BLE (air)
                    ▼                            │
          ┌───────────────────┐                 │
          │ Attenuator +      │                 │
          │ shielded enclosure│─────────────────┘
          └────────┬──────────┘
                   │ conducted RF
                   ▼
          ┌───────────────────────┐
          │ Spectrum analyzer OR  │
          │ reference SX1262 peer │  (duty-cycle / TX capture)
          └───────────────────────┘
```

- Host controller flashes and drives the DUT over USB-Serial-JTAG and
  captures the console log (where the conformance diff and duty-cycle
  assertions are emitted).
- The LoRa RF path is **conducted** through a fixed attenuator inside a
  shielded enclosure into either a spectrum analyzer or a reference SX1262
  peer, so TX capture is repeatable and does not radiate.
- The DUT supply rail passes **in series** through a Nordic PPK2 for
  µA-resolution current traces (idle/sleep), per
  [`BENCH_POWER_LITE.md`](BENCH_POWER_LITE.md).
- Wi-Fi/BLE domains exercise a co-located AP + BLE central acting as the
  peer for `advertise`/`scan`/`config`/`start` round-trips.

## 3. Fixture inventory

| Fixture | Purpose | Serves |
|---|---|---|
| Lite DUT (D-0013 unit) | Device under test; runs on-target conformance core + real HAL | all criteria / all domains |
| Self-hosted CI runner | Host controller: flashes DUT, drives vectors, captures logs, adjudicates | crit. 1–4 |
| USB-Serial-JTAG console | Flash + console log capture (diff lines, assertions) | crit. 1; all domains |
| Attenuator + shielded enclosure | Conducted, non-radiating LoRa RF path | lora; crit. 3 |
| Spectrum analyzer or reference SX1262 peer | RF/duty-cycle capture cross-check of LoRa TX | crit. 3; lora |
| Nordic PPK2 (µA meter, in series) | Idle/sleep current traces per [`BENCH_POWER_LITE.md`](BENCH_POWER_LITE.md) | crit. 4; power |
| Wi-Fi AP + BLE central peer | Association/advertise/scan round-trips for radio domains | wifi, ble |

## 4. Test matrix (maps EPIC-03 exit criteria × conformance domains)

Every EPIC-03 exit criterion and every one of the five conformance domains
(power, audio, lora, wifi, ble) maps to a concrete rack action below.

| Exit criterion | Domain(s) | Rack action / vector | Instrument / fixture | Pass condition |
|---|---|---|---|---|
| 1. All HAL headers have a Lite `.c` impl | (build) | Build `make lite`, flash, boot, `ss_hal_init` smoke; enumerate `ss_hal_has_cap` | CI runner + USB-JTAG console | Clean build + boot; `ss_hal_init` returns OK; expected caps present |
| 2. HIL passes conformance vectors | power | Run on-target `ss_hal_conformance_core` power vectors (`lifecycle-status`, `wake-timer-arg-validation`, `wake-timer-clear-idempotent`, `shutdown-disarms-wake`) | CI runner + USB-JTAG console | Frozen actionable-diff **empty** for power |
| 2. HIL passes conformance vectors | audio | Run on-target audio vectors (`mic-lifecycle`, `spk-volume-mute`, `use-before-open`, `buzzer-beep`) | CI runner + USB-JTAG console | Frozen diff **empty** for audio |
| 2. HIL passes conformance vectors | lora | Run on-target lora vectors (`init-config-tx`, `rx-start-stop`, `use-before-init`, `sleep-wake`) | CI runner + shielded RF path | Frozen diff **empty** for lora |
| 2. HIL passes conformance vectors | wifi | Run on-target wifi vectors (`lifecycle`, `config-null`, `sleep-toggle`) | CI runner + Wi-Fi AP peer | Frozen diff **empty** for wifi |
| 2. HIL passes conformance vectors | ble | Run on-target ble vectors (`advertise-stop`, `scan-lifecycle`, `advertise-null-name`, `use-before-init`) | CI runner + BLE central peer | Frozen diff **empty** for ble |
| 3. LoRa TX meets FCC 15.247 duty cycle | lora | Software duty-cycle assertion in TX path + RF capture cross-check of on-air time | Spectrum analyzer / reference SX1262 peer | Software assertion holds AND captured duty cycle within FCC 15.247 limit |
| 4. Idle current ≤ 25 mA @ 3.7 V | power | Steady-state idle current on 3.7 V rail | PPK2 in series ([`BENCH_POWER_LITE.md`](BENCH_POWER_LITE.md)) | Idle ≤ 25 mA @ 3.7 V |
| 4. Sleep current ≤ 0.5 mA | power | Sleep current with canonical wake set armed | PPK2 in series ([`BENCH_POWER_LITE.md`](BENCH_POWER_LITE.md)) | Sleep ≤ 0.5 mA |

### 4.1 Results (fill at D-0013)

| Row | Target | Measured | Pass/Fail |
|---|---|---|---|
| HAL impl completeness (build+flash+smoke) | clean boot | | |
| Conformance — power | empty diff | | |
| Conformance — audio | empty diff | | |
| Conformance — lora | empty diff | | |
| Conformance — wifi | empty diff | | |
| Conformance — ble | empty diff | | |
| FCC 15.247 duty cycle | within limit | | |
| Idle current | ≤ 25 mA @ 3.7 V | | |
| Sleep current | ≤ 0.5 mA | | |

## 5. CI trigger and result reporting

**Trigger.** The rack job runs on the self-hosted runner selected by
`runs-on: [self-hosted, hil, lite]`. Because the D-0013 fleet does not exist
yet, the job is currently **`workflow_dispatch`-only and non-blocking**. When
the fleet lands, it is promoted to a required merge gate on `firmware/**`
changes (path-filtered, alongside the existing `firmware-build.yml` matrix).
Until then it may not block merges — there is no runner online to answer.

**Execution.** The runner builds `make lite`, flashes the DUT over USB-JTAG,
and runs the on-target Unity runner that links the identical conformance core
+ vectors (S-02-015 adapter). Each domain's `emit` routes `CONF-FAIL …` lines
to the console; the duty-cycle and current fixtures run alongside.

**Result reporting.** The rack wrapper parses the frozen actionable-diff and
converts per-domain pass/fail into **JUnit XML** (one test case per domain
plus duty-cycle and current), surfaced as a PR check with per-domain
PASS/FAIL. Raw evidence — console logs, RF captures, and PPK2 current traces
— is uploaded as a build artifact for audit.

**Green condition.** The check is green iff the actionable-diff is **empty
across all five domains** *and* the idle/sleep current and LoRa duty cycle are
each within budget. Any non-empty diff line, or any budget breach, is a fail
with the offending domain named in the check summary.
