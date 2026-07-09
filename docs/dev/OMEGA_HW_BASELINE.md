<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# Omega/Alpha hardware baseline (D-0020, 2026-07-09)

Doc of record aligning the software portfolio to the **signed-off** PCB
designs in the `ss-pcb-design-engeneering` repo. Decision D-0020: software
claims follow the released board; deferred bearers/parts are preserved as
roadmap intent gated on a board revision — nothing is dropped.

## Omega v1.0 — RELEASED (authoritative)

Release: `release_v69/` (2026-07-08) — TVF 69/69 PASS, DRC 0 errors,
gerbers + assembly + mechanical cut, SHA 054eaa8b…; supersedes v67/v68.
Source board: `closure_work/v67_route_work.kicad_pcb` (v69 is its release).
**The board is final — software aligns to it, never the reverse.**

### Present (v69 BOM, `release_v69/assembly/v69-BOM-raw.csv`)

| Subsystem | Part | Ref |
|---|---|---|
| Main SoC | ESP32-P4NRW32X SoM | U1 |
| Wi-Fi/BLE bridge | ESP32-C6-MINI-1 | U12 |
| HaLow (802.11ah) | Morse Micro MM8108-MF15457 | U2 |
| GNSS | u-blox MIA-M10Q (I²C) | U78 |
| Magnetometer | Bosch BMM350 (**not** MMC5983MA) | U79 |
| Haptics | TI DRV2625 + LRA (JST J8) | U80 |
| Audio | ES8311 codec + NS4150B class-D + SPH0641LU4H **PDM** mic (clocked from i2s_bck) | U9/U10/U22 |
| Status LEDs | 12× SK6805-EC15 addressable ring | U24–U35 |
| Display | ER-TFT3.92-1 3.92″ IPS 480×480, ST7796S, 8-bit parallel I8080, 40-pin FPC | J6 |
| Touch | **GT911** (via display FPC) — D-27: firmware must strap to I²C 0x5D at reset (tp_int LOW during touch_rst release) to avoid 0x14 collision with BMM350 | — |
| HaLow FEM | SKY66423-11 — **routed**, incl. `pa_pdet` (VSWR sense) to a P4 ADC | U3 |
| Buttons | 4 side keys: power / vol+ / vol− / reset (SKRTLAE010); **no D-pad, no rocker** | U18–U21 |
| Power | EA3059QDR 3-ch PMIC (v3v3/v1v8/v3v6_rf) + TP4056 charger + MAX17048 fuel gauge; SY7201ABC boost (backlight only) | U6/U4/U5/U8 |
| USB | USB-C 5 V sink only, **no PD IC**, D+/D− routed FS-only (12 Mbps); SoC-level pin mux with LCD CSX/DCX (GPIO26/27) — firmware VBUS-detect arbitration required | U13 |
| Storage | microSD, SDIO | U15 |

### Absent (confirmed against the BOM — GHOSTWARE if software claims them)

- **LoRa (SX1262 or any):** never designed in.
- **Cellular modem (LTE-M/NB-IoT), eSIM/SIM slot:** absent.
- **Satellite modem (Iridium/Skylo NTN):** absent.
- **BMP390 / any barometer:** absent.
- **Secure element (ATECC608/SE050):** absent.
- **Supercap / TX-burst power path:** absent (boost converter is backlight supply).
- **IMU / accelerometer:** absent (no wake-on-motion source); magnetometer
  BMM350 is present.
- **Ambient-light sensor:** absent (backlight is fixed/PWM, no auto-dim input).
- **NTC on battery pack:** RT1/RT2 jumpered — firmware owns the full thermal
  envelope (case-temp model + TX duty cap).
- **Any expansion interface:** no headers, M.2, mikroBUS, USB host, or spare
  footprints. Deferred capability **requires a board respin** — it cannot be
  added as an external module.

Correction (2026-07-09, D-0021): the earlier "SKY66423 unrouted / gap G1"
finding was wrong — "G1" is a GNSS pin name, not a gap ID. The FEM **is**
routed (see Present table); Omega v1.0 has the full PA path.

### Portfolio consequences (applied 2026-07-09)

- EPIC-05 cellular/satellite/baro/SE/tamper/power-path stories → `BLOCKED`
  on the Omega rev-2 respin (capability preserved, not dropped).
- S-05-011 retargeted MMC5983MA → BMM350; S-05-012 part confirmed (SK6805×12).
- S-05-020 (Omega spec-lock) takes this document + v69 release as its input
  of record; `firmware/boards/omega/board_config.h` claims flow from it.
- doc 00 §1.2/§11-Ph4 and doc 08 bearer-table roadmap claims carry baseline
  callouts pointing here (additive — aspiration text retained).

## Alpha 1.0 — NOT LOCKED (do not align software to it yet)

- Latest source: `SS-SP-Alpha-1-0_production_v152_v15_FINAL.kicad_pcb`
  (2026-06-19); **no release package, no post-fix verification report**.
- `FUNCTIONAL_VERIFICATION_REPORT_v152_v14.md` verdict: **"FUNCTIONALLY DEAD
  — DO NOT FABRICATE"** (U1 P4 footprint fictional, MM8108 pins wrong, power
  tree dead-latch, audio mis-pinned). v15 fixes are unverified.
- Comms intent: MM8108 HaLow (+P4/C6, ER-TFT024IPS-3 2.4″ panel).
  **No LoRa** (per `firmware/boards/alpha/board_config.h`).
- Consequence: EPIC-04 stories stay as written; hardware-empirical
  elaboration waits for an Alpha release package equivalent to v69.

## Naming & companion truth sources (2026-07-09)

- `ss-pcb-design-engeneering/NAMING.md` is the cross-repo naming source of
  truth: the v67/v69 board **is the Seekie-Speakie ss-sp omega**; legacy
  labels "S SP2 / SP2 Alpha / v67 Alpha" in HW-repo artifacts refer to this
  same Omega board. Any HW-repo mapping of its concerns onto "EPIC-04/Alpha"
  must be re-projected onto EPIC-05/Omega (D-0021).
- Deeper truth artifacts (component/pin adjudication): `verification/
  pinmap_truth.json`, `verification/manifest.yaml`, `release_v69/assembly/`
  BOM, `closure_work/UNIFIED_CONCERNS_LEDGER.md` (~88 SW-side concerns),
  `closure_work/HW_SW_ALIGNMENT_AUDIT.md`, `closure_work/
  OPTIMIZATION_DECISIONS.md`. Portfolio-side projection of record:
  `docs/dev/OMEGA_LEDGER_ALIGNMENT.md`.

Maintenance: update this file only from a formally released PCB package
(release notes + gerbers + BOM); cite the release SHA.
