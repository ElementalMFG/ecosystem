<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# Omega/Alpha hardware baseline (D-0020, 2026-07-09)

Doc of record aligning the software portfolio to the **signed-off** PCB
designs in the `ss-pcb-design-engeneering` repo. Decision D-0020: software
claims follow the released board; deferred bearers/parts are preserved as
roadmap intent gated on a board revision — nothing is dropped.

## Round framing — round-1 `elecrow5` vs round-2 in-house v69 (D-0026, 2026-07-14)

D-0026 amends *ship order only*: the **round-1** market device line rides
off-the-shelf Elecrow CrowPanel Advance boards so the ecosystem can be built
and validated on purchasable hardware now; the **in-house PCBs are round-2**.
All v69 hardware-truth, capability ledgers, HAL work, and anti-rug-pull
preservation (D-0020/D-0021) below **stand unchanged** — only the round they
ship in moves. Name/brand/tier/board are decoupled (D-0026 §3): docs and build
targets key off **hardware target + tier role**, never a marketing name.

| Round | Tier role | Hardware target | Board | Status |
|---|---|---|---|---|
| Round-1 | Lite / Alpha | `elecrow35-s3` | Elecrow CrowPanel Advance 3.5″ ESP32-S3 (D-0023 variant matrix) | Purchasable / round-1 realization |
| Round-1 | **Omega (flagship)** | **`elecrow5`** | Elecrow CrowPanel Advance 5″ ESP32-P4 HMI | Purchasable / round-1 realization |
| Round-2 | Omega | `omega-v69` | In-house v69 (this doc, below) | Engineering board-of-record, fabbed later |
| Round-2 | Alpha | `alpha-v152` | In-house v152 (not release-verified) | Deferred to its own lock |

Round-1 Elecrow boards are **dev/functional-grade, non-IP-rated** (bare board
in a custom enclosure); IP65/66 ruggedization is a **round-2 / in-house**
property — round-1 docs/marketing must not claim rugged/IP ratings (D-0026 §5).

### `elecrow5` — Elecrow CrowPanel Advance 5″ ESP32-P4 HMI (round-1 Omega-tier)

Known config (Elecrow public listing; verify pins at bring-up):

| Subsystem | Part / config |
|---|---|
| Main SoC | ESP32-P4 |
| Wi-Fi 6 / BLE 5.3 | onboard ESP32-C6 (always present) |
| Display | 800×480, GT911 capacitive touch (public-listing read; the "MIPI-DSI" interface assumption is **SUPERSEDED by D-0027 → 16-bit parallel RGB565** — see schematic-validated table below) |
| Audio | NS4168 class-D speaker/mic path |
| Wireless-module slot | SPI slot — **HaLow (Elecrow MM6108) mandatory/always** (present on every V1 unit); optional LoRa is an *additive* sub-variant on a secondary bus (Crowtail/GPIO — bring-up feasibility TBD), never a swap for HaLow |
| Camera | optional MIPI-CSI (auto-detected) |
| GNSS | optional, UART (auto-detected) |
| Compass | optional 3-axis, I²C (auto-detected) |
| Expansion | UART / I²C Crowtail |
| Power | USB-C + battery / charging |
| Rugged | none — dev/functional-grade, non-IP-rated |

Capability default = **"everything optional except HaLow"**: Wi-Fi 6 + BLE
(C6) are always present; HaLow is the one mandatory radio; GPS/compass/speaker/
camera are boot-time auto-detected (D-0023 bearer-readiness + S-03-048 presence
probes) — absent peripherals compile/flag to zero.

**Bring-up UNKNOWNS to verify (D-0026 §7, non-blocking):**

- C6↔P4 link method (assume SDIO / ESP-Hosted until confirmed).
- HaLow-over-SPI throughput ceiling **and** the SPI/UART1 DIP-shared pins —
  route GPS to a free UART.
- ~~MIPI-DSI bridge IC (DSI↔RGB/parallel bridge, model TBD).~~ **RESOLVED (D-0027):** no DSI/bridge — panel is 16-bit parallel RGB565 direct off the P4.
- Camera sensor model (MIPI-CSI, TBD).
- Pinned ESP-IDF version for the P4 target.

**Schematic-validated `elecrow5` truth (D-0027, 2026-07-18):** netlist-level
reading of the board's own Eagle schematic (`ESP32-P4 Display 5.0 inch V1.0.sch`,
mirrored under `vendor/elecrow/`) corrects the D-0026 §(2) display/touch
assumptions (which came from generic P4 web specs) and confirms the rest.
Facts only — pin/net data, no reproduction of the copyrighted schematic:

| Subsystem | Schematic-validated fact (D-0027) |
|---|---|
| Display | **16-bit parallel RGB565, NOT MIPI-DSI** — 40-pin FPC JP1; PCLK=GPIO3, DE=GPIO2, HSYNC=GPIO40, VSYNC=GPIO41, R/G/B on GPIO4–19 → `esp_lcd` **RGB** panel path |
| Touch | on-panel, **I²C1** (SCL=GPIO46, SDA=GPIO45, INT=GPIO42, RST via GPIO36 + STC8); controller on the display flex, **not in board BOM — "GT911" UNVERIFIED** (likely @ 0x5D/0x14; confirm at bring-up) |
| C6↔P4 bus | **SDIO 4-bit CONFIRMED** — CLK/CMD/D0–D3 = GPIO53/54/52/51/50/49 (C6 as SDIO radio slave; UART0 only on flashing test-pads); ESP-IDF ≥ v5.3 + `esp_hosted`/`esp_wifi_remote` |
| Module (HaLow/LoRa) slot | **SPI2** — SCK=GPIO26, MOSI=GPIO48, MISO=GPIO47 (headers J9/J11); + GPIO29/30 (reset/BUSY, SX126x-style LoRa path) + UART2 + I²C1 + 3V3/5V. **MOSI/MISO shared with UART1→Crowtail J2 via an SGM3005 analog mux (slide-switch S1 + STC8) — firmware must set the mux before slot use.** HaLow driver basis = MorseMicro `esp-halow` (Apache-2.0) |
| GNSS / mag / fuel gauge | **none on-board — all external** (Crowtail UART/I²C). No fuel gauge: battery (JST-PH2.0, J3) is sensed by the **STC8 co-MCU ADC**, read by the P4 over I²C1 |
| STC8H1K08 co-MCU (U14) | power/peripheral co-MCU on **I²C1** — gates backlight-EN, SPI/UART mux select, touch-RST, camera-RST, audio-shutdown, charge LEDs, and VBAT/charge-status sense. First-class bring-up dependency → **new story S-05-048** |
| Camera | **external only** — 2-lane MIPI-CSI header (FPC3), SCCB on I²C2 (GPIO34/33), no on-board sensor (OV5647/SC2336 is a module question) |
| Audio | **2× NS4168** I²S amps (shared BCLK=GPIO22) + **PDM MEMS mic** — always present |
| Flash / USB | **W25Q128JV 16 MB** flash; dual USB-C (**CH340K** UART-boot + P4-native USB 2.0) |

Residual bring-up UNKNOWNS (D-0027): exact touch controller/addr, TLV62569 rail
voltages, module-slot LoRa module pinout, PSRAM size (P4NRW32 implies integrated).
Provenance: `vendor/elecrow/` (README + `_MANIFEST.md`).

**Confirmed from Elecrow example code + ESP32-P4 datasheet (2026-07-18):** the
following upgrade the D-0027 residual "UNVERIFIED/likely/TBD" items to CONFIRMED
(evidence in `vendor/elecrow/`; part-number nomenclature per the P4 datasheet):

- **Touch controller = GT911 (Goodix)**, I²C address **0x5D (default) / 0x14
  (alt, INT-strap-selected at reset)** on the GPIO46(SCL)/GPIO45(SDA) I²C bus
  (port is a firmware assignment — Elecrow's example uses I2C0; the D-0027 pin
  facts stand), RST=GPIO36, INT=GPIO42, 400 kHz. Evidence: Elecrow `Lesson05`
  `esp_panel_board_custom_conf.h` `ESP_PANEL_BOARD_TOUCH_CONTROLLER GT911`;
  `gt911_for_crowpanel/TAMC_GT911.h` `GT911_ADDR1 0x5D` / `ADDR2 0x14`; board
  wiki. Residual: which strap (0x5D vs 0x14) on a physical unit — confirm by
  I²C scan at bring-up.
- **PSRAM = 32 MB, in-package 16-line (HEX / OPI), 1.8 V, ~200 MHz** — from
  part **ESP32-P4NRW32** (R=has-PSRAM, W=16-line-1.8V, 32=32 MB) + repo
  `sdkconfig` `CONFIG_SPIRAM_MODE_HEX=y`, `SPEED_200M`.
- **Flash = 16 MB Winbond W25Q128JVSIQ** (confirms the D-0027 W25Q128JV note).
- **Module-slot LoRa sub-variant = SX1262**; slot pinout NSS=GPIO30,
  BUSY=GPIO29, IRQ/DIO=GPIO31, NRST=GPIO32, SCK=GPIO26, MISO=GPIO47,
  MOSI=GPIO48, 3.3 V (product-page option + wiki BSP `RADIO_GPIO_*`).
- **Camera** optional 2 MP module sensor **LIKELY SC2336** (full DS gated —
  LIKELY, not confirmed).

Firmware policy unchanged (D-0026 §6): ESP32-family (P4/S3 in-family) stays the
first-party firmware target; non-ESP32 MCU firmware stays out of scope.

## Omega v1.0 — RELEASED (authoritative)

*Round classification (D-0026): this v69 board is the **round-2 / in-house**
Omega board-of-record; the round-1 Omega-tier device ships on `elecrow5` (see
Round framing above). "RELEASED" here means design-released/signed-off — ship
order is round-2. Everything below stands unchanged.*

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
| Audio | ES8311 codec + NS4150B class-D + SPH0641LU4H **PDM** mic (clocked from i2s_bck). NS4150B U10 CTRL/SD is **hard-strapped to v3v3** (BIT-2) — no runtime mute GPIO; muting is codec-side only (ES8311 registers + MCLK stop) | U9/U10/U22 |
| Status LEDs | 12× SK6805-EC15 addressable ring | U24–U35 |
| Display | ER-TFT3.92-1 3.92″ IPS 480×480, ST7796S, 8-bit parallel I8080, 40-pin FPC | J6 |
| Touch | **GT911** (via display FPC) — D-27: firmware must strap to I²C 0x5D at reset (tp_int LOW during touch_rst release) to avoid 0x14 collision with BMM350 | — |
| HaLow FEM | SKY66423-11 — routed (CTX/CSD/CPS control paths). **No `pa_pdet`/VSWR sense net exists** (BIT-1): VSWR protection is a composite proxy (die-temp + v3v6_rf current + MM8108 PHY telemetry, Path A per owner decision #4), EVT-validated | U3 |
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
