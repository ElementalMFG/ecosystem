<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# EPIC-05 — HAL for Omega (Alpha + Cellular + LEO Sat option)

**Primary WG:** wg-firmware · **Contributing:** wg-hardware, wg-legal
**Priority:** P0 · **SKU:** O · **Milestone:** M7

## Outcome
All Alpha HAL contracts plus Omega additions: LTE-M/NB-IoT modem (e.g. Nordic nRF9161 or Quectel BG95/BG770A), embedded SIM (eSIM/eUICC), optional LEO satellite modem (e.g. Iridium 9770 / Skylo NTN), enhanced power path for higher TX bursts, and enterprise-grade sensor set (baro, magnetometer).

## Constitution
C-00 `00_MASTER_SOFTWARE_PLAN.md` §HAL contracts, §tiers + `models/CATALOG` (Omega hardware); C-05 `05_SECURITY_MODEL.md` §secure element, §tamper response; C-08 `08_UNIVERSAL_CONNECTIVITY.md` §bearers (cellular LTE-M/NB-IoT, LEO satellite).

## Dependencies
EPIC-04 (Alpha HAL frozen).

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

## Exit criteria
1. LTE-M attach + IP data path within 30 s in cell coverage.
2. PSM sleep current ≤ 15 µA.
3. eSIM profile install / delete works over BIP.
4. Satellite SBD round-trip succeeds in field trial.
5. Tamper switch triggers key-wipe when armed.

## Risks
| # | Risk | Mitigation |
|---|------|-----------|
| R05-01 | Modem certification cost (PTCRB, GCF) | Reuse modem-vendor cert where possible |
| R05-02 | Satellite airtime cost | Airtime-included SKU + BYOA plan |
| R05-03 | eSIM MNO deal complexity | Aggregator partnership (1NCE / Emnify) |
