<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# EPIC-04 — HAL for Alpha (ESP32-P4 + HaLow + Wi-Fi 5 + GNSS + IMU)

**Primary WG:** wg-firmware · **Contributing:** wg-hardware
**Priority:** P0 · **SKU:** A · **Milestone:** M4

## Outcome
All Lite HAL contracts plus the Alpha-specific ones (Wi-Fi HaLow MM8108 + SKY66423 FEM, Wi-Fi 5 client chip, u-blox GNSS, LSM6DSV IMU, larger DSI display) are implemented, tested, and pass conformance vectors.

## Constitution
C-00 `00_MASTER_SOFTWARE_PLAN.md` §HAL contracts, §tiers + `models/CATALOG` (Alpha hardware); C-08 `08_UNIVERSAL_CONNECTIVITY.md` §bearers (Wi-Fi HaLow, Wi-Fi 5).

## Dependencies
EPIC-02, EPIC-03 (HAL contracts frozen).

## Shards
- **S-04.A ESP32-P4 bring-up** — DDR PSRAM, dual-core RV32, cache config.
- **S-04.B Wi-Fi HaLow driver (MM8108)** — SDIO interface, Morse OSAL glue, WPA3-SAE.
- **S-04.C SKY66423 FEM** — TX gain, RX LNA, T/R switch timing.
- **S-04.D Wi-Fi 5 client chip** — ESP32-P4 uses external, e.g. RTL8852BE, PCIe/SDIO.
- **S-04.E GNSS** — u-blox ZOE-M8Q UART, NMEA/UBX parser, cold/warm/hot fix.
- **S-04.F IMU** — LSM6DSV I²C, motion wake, tilt/orient.
- **S-04.G Large DSI display** — 320×480 or 480×640 MIPI-DSI.
- **S-04.H HaLow region PA table & channel plan** — US, EU, JP, KR, IN, AU.
- **S-04.I DSP acceleration** — ESP32-P4 AI accelerator for wake-word.

## Exit criteria
1. All Alpha HAL headers implemented and pass conformance.
2. HaLow single-hop range ≥ 500 m LoS with 8 dBi antenna.
3. GNSS cold-fix < 60 s in open sky.
4. Wi-Fi 5 STA achieves ≥ 100 Mbps to laptop AP.
5. IMU wake latency < 200 ms.

## Risks
| # | Risk | Mitigation |
|---|------|-----------|
| R04-01 | MM8108 driver stability | Escalation MoU with Morse Micro |
| R04-02 | ESP32-P4 silicon errata | Track vendor advisories, workaround registry |
| R04-03 | HaLow regulatory table drift | Auto-fetch from wg-legal repo |
