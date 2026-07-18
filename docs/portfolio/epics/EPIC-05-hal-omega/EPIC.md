<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# EPIC-05 — HAL for Omega (Alpha + Cellular + LEO Sat option)

**Primary WG:** wg-firmware · **Contributing:** wg-hardware, wg-legal
**Priority:** P0 · **SKU:** O · **Milestone:** M7

## Outcome
All Alpha HAL contracts plus Omega additions: LTE-M/NB-IoT modem (e.g. Nordic nRF9161 or Quectel BG95/BG770A), embedded SIM (eSIM/eUICC), optional LEO satellite modem (e.g. Iridium 9770 / Skylo NTN), enhanced power path for higher TX bursts, and enterprise-grade sensor set (baro, magnetometer).

## Hardware baseline (D-0020, 2026-07-09)

The signed-off Omega v1.0 board (PCB release v69) carries **none** of the Outcome's modem/sensor additions: no cellular, no satellite, no barometer, no secure element, no supercap power path, and no expansion interface (respin required). Shards S-05.A/B/C/D/E/F/I and the modem exit criteria are therefore **rev-2-gated**; their stories are `BLOCKED`, capability preserved. What v69 DOES add over the plan: BMM350 magnetometer (not MMC5983MA), MIA-M10Q I²C GNSS, DRV2625 haptics, ES8311 audio, 12× SK6805 LED ring. Authoritative: `docs/dev/OMEGA_HW_BASELINE.md`.

**D-0021 corrections (2026-07-09):** v69 also carries a GT911 touch controller (via the display FPC), a **routed** SKY66423 FEM including `pa_pdet` VSWR sense to a P4 ADC, an EA3059 3-channel PMIC, and a SPH0641LU4H PDM mic (clocked from i2s_bck) — the earlier "SKY66423 unrouted / gap G1" finding was a mis-read ("G1" is a GNSS pin name, not a gap ID). Core-platform HAL for these lands in new shards S-05.J/K/L (stories S-05-021…039). Ledger projection of record: `docs/dev/OMEGA_LEDGER_ALIGNMENT.md`.

**Omega rev-2 optimization batch:** ledger items HW-1…5 are small BOM tweaks (~$0.003/board total — charge-set resistor, rail trims, I²C pull-up values) with no firmware dependency; they are batched with the rev-2 respin decision and file no stories until that decision lands (HW-6 NTC-divider bypass is a documented design choice — FW-8 / S-05-028 owns the thermal envelope instead).

**BIT corrections (2026-07-09, HW readiness-audit second pass):** no `pa_pdet` net exists on v69 — S-05-029 rescopes to a composite VSWR proxy (Path A); the NS4150B mute pin is hard-strapped to v3v3 — S-05-023 mutes codec-side (ES8311 register mute + MCLK stop); BIT-3 hybrid RTC/backup-domain policy (GNSS→NTP-via-C6→Y2 freewheel) is added to S-05-037. **Rev-2 scope per owner direction 2026-07-09: LoRa is the rev-2 priority** (S-05-040, D-0023); cellular is UNSCHEDULED (D-0024, not in current product intent); satellite/SE/baro remain rev-2-preserved. Story elaboration must pull the HW readiness-audit §6 amendment row for each S-05-02x/03x story (per `docs/dev/OMEGA_LEDGER_ALIGNMENT.md` §7.5).

**Round-1 board realization (D-0026, 2026-07-14 — governs this epic):** EPIC-05's **round-1** Omega-tier board is the off-the-shelf **Elecrow CrowPanel Advance 5″ ESP32-P4 (`elecrow5`)** — ESP32-P4 + onboard C6 Wi-Fi 6/BLE, 800×480 MIPI-DSI GT911, NS4168 audio, an SPI wireless-module slot (**HaLow MM6108 mandatory/always**; optional LoRa is *additive* on a secondary bus, not a swap), optional MIPI-CSI camera + UART GNSS + I²C compass + speaker (all boot-time auto-detected). HaLow is present on every V1 unit; LoRa/GPS/compass are additive sub-variant options in any combination (D-0026 §4). The buildable-now Omega HAL work (shards J/K/L: display, touch, audio, power, USB, SD, inputs, HaLow/C6 plumbing, GNSS/compass) is realized on `elecrow5` for round-1 while the **v69-specific** stories (BMM350, ST7796S I8080, SDIO HaLow, EA3059 power tree, VSWR proxy, side-keys, etc.) remain the **round-2 / in-house** realization. Only ship-order changes; the v69 baseline, ledgers, and rev-2 preservation (D-0020/D-0021) stand. New `elecrow5` bring-up stories are added below in STORIES (round-1); existing per-story ACs are unchanged. Round-1 Elecrow boards are dev/functional-grade, non-IP-rated (D-0026 §5). See `docs/dev/OMEGA_HW_BASELINE.md` (Round framing).

**D-0027 + example-code corrections (2026-07-18, schematic-validated — supersede the D-0026 web-spec assumptions above):** netlist reading of the board's own Eagle schematic (bundled in `vendor/elecrow/`) plus the Elecrow example driver confirm: the `elecrow5` display is **16-bit parallel RGB565 800×480 via the `esp_lcd` RGB panel path — NOT MIPI-DSI**; touch is on-panel over I²C1, controller **CONFIRMED = GT911 (Goodix) @ 0x5D/0x14**; C6↔P4 bus **CONFIRMED = SDIO 4-bit** (`esp_hosted`); PSRAM **CONFIRMED = 32 MB in-package 16-line HEX, 1.8 V ~200 MHz (ESP32-P4NRW32)** + 16 MB W25Q128JV flash; the module-slot LoRa sub-variant **= SX1262**; audio = 2× NS4168 + PDM mic (always present); there is **no on-board GNSS/mag/fuel-gauge/camera** (external-only; battery sensed via the STC8 co-MCU on I²C1). A new bring-up story **S-05-048** (STC8 co-MCU) is added. Story ACs already reflect these facts; the "MIPI-DSI GT911" phrase in the D-0026 paragraph above is the superseded web-spec reading, retained for provenance. Authoritative: `docs/dev/OMEGA_HW_BASELINE.md`.

## Constitution
C-00 `00_MASTER_SOFTWARE_PLAN.md` §HAL contracts, §tiers + `models/CATALOG` (Omega hardware); C-05 `05_SECURITY_MODEL.md` §secure element, §tamper response; C-08 `08_UNIVERSAL_CONNECTIVITY.md` §bearers (cellular LTE-M/NB-IoT, LEO satellite).

## Dependencies
**EPIC-04 (Alpha) is NOT a dependency.** Under the D-0021 inversion, Omega v69 is the released board and ships first; the shared-platform HAL components (display, touch, audio, power, USB, SD, inputs, haptics, HaLow/C6 plumbing) are authored here and later reused by Alpha when an Alpha release package exists. EPIC-06 (crypto core) gates the T1 sub-tasks (HaLow region-blob signing, BLE key boundary, JTAG/ROM eFuse lockdown). See `docs/dev/OMEGA_LEDGER_ALIGNMENT.md` §1.

## Shards
- **S-05.A Cellular modem UART/USB driver** — AT+MUX + IP passthrough.
- **S-05.B eSIM/eUICC profile management** — SGP.22 (consumer) or SGP.32 (IoT).
- **S-05.C PSM/eDRX low-power modes.**
- **S-05.D NB-IoT band table** — global roaming profile.
- **S-05.E LEO satellite modem** — Iridium SBD or Skylo NTN (3GPP Release 17).
- **S-05.F Power path for TX bursts** — supercap / boost, brownout tolerance.
- **S-05.G Baro (BMP390) + mag (MMC5983MA).**
- **S-05.H Enterprise LED bar** (multi-LED status ring for radio state).
- **S-05.I Physical security hardware** — tamper switch, secure element (ATECC608 or SE050).
- **S-05.J Core platform HAL bring-up** — display, touch, audio, USB, SD, inputs, haptics.
- **S-05.K Power & thermal** — battery policy, power tree, IEC 62368-1 TX duty cap.
- **S-05.L Radio & comms plumbing** — HaLow SDIO, ESP-Hosted C6 bridge, PA VSWR watchdog, BLE key boundary.

## Exit criteria

**Omega rev-2-gated (D-0020 — hardware absent from v69):**
1. LTE-M attach + IP data path within 30 s in cell coverage. *(rev-2-gated)*
2. PSM sleep current ≤ 15 µA. *(rev-2-gated)*
3. eSIM profile install / delete works over BIP. *(rev-2-gated)*
4. Satellite SBD round-trip succeeds in field trial. *(rev-2-gated)*
5. Tamper switch triggers key-wipe when armed. *(rev-2-gated)*

**Omega v1.0 (v69 board):**
6. Display + touch + audio + HaLow bring-up on v69.
7. IEC 62368-1 TX duty cap active (rolling-60 s ≤ 40 %).
8. USB↔LCD arbitration passes the 100-cycle plug soak.
9. ESP-Hosted Wi-Fi/BLE functional through the C6 bridge.

## Risks
| # | Risk | Mitigation |
|---|------|-----------|
| R05-01 | Modem certification cost (PTCRB, GCF) | Reuse modem-vendor cert where possible |
| R05-02 | Satellite airtime cost | Airtime-included SKU + BYOA plan |
| R05-03 | eSIM MNO deal complexity | Aggregator partnership (1NCE / Emnify) |
