<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# EPIC_INDEX — All 24 SS-SP Program Epics

*Master catalog. Every epic here has its own directory under `epics/EPIC-NN-*`, an `EPIC.md` describing shards and exit criteria, and a `STORIES.md` listing every story that belongs to it.*

Legend:
- **WG** = primary Working Group per `06_GOVERNANCE.md` §3.
- **SKU** = which product tier the epic is required for at first ship (L = Lite, A = Alpha, O = Omega, ★ = all).
- **Priority** = program-level P0..P3.
- **Milestone** = earliest milestone in which the epic must be substantially complete (see `04_BLUEPRINT.md` §1).

---

| # | Epic | Outcome (one line) | Primary WG | SKU | Priority | Milestone |
|---|------|--------------------|------------|-----|----------|-----------|
| 01 | governance-constitution | Founding docs, RFC process, DCO/CLA, decisions log, working-group charters operational | wg-community | ★ | P0 | M0 |
| 02 | firmware-foundation | RTOS baseline, monorepo build, board bring-up scaffolding, logging, panic handler, watchdog | wg-firmware | ★ | P0 | M0 |
| 03 | hal-lite | Full HAL implementation for Lite (ESP32-S3 + SX1262 + ESP32 Wi-Fi/BLE) | wg-firmware | L | P0 | M1 |
| 04 | hal-alpha | Full HAL implementation for Alpha (ESP32-P4 + MM8108 HaLow + SKY66423 FEM + GNSS + IMU) | wg-firmware | A | P0 | M4 |
| 05 | hal-omega | Full HAL implementation for Omega (Alpha + LTE-M/NB-IoT + optional LEO sat) | wg-firmware | O | P0 | M7 |
| 06 | crypto-core | `ss_crypto` primitives, Double Ratchet, sender-keys, hybrid PQ, KAT tests, side-channel review | wg-security | ★ | P0 | M1 |
| 07 | identity-provisioning | RelKey_A/B/C hierarchy, per-device keypair generation, provisioning line, key ceremony docs | wg-security | ★ | P0 | M1 |
| 08 | secure-boot-flash-enc | Secure Boot v2, flash encryption, anti-rollback, verified boot chain, HSM signing | wg-firmware, wg-security | ★ | P0 | M1 |
| 09 | ota-update-system | Dual-signed manifests, delta OTA, staged rollout, reproducible build attestation | wg-firmware, wg-ops | ★ | P0 | M2 |
| 10 | ss-link-bearer | Unified multi-bearer transport, bearer plugins for LoRa/HaLow/Wi-Fi/BLE/cellular, QoS, scheduler | wg-firmware, wg-protocol | ★ | P0 | M2 |
| 11 | reticulum-integration | RNS transport, per-bearer interfaces, resource caching, path finding, GET/PUT primitives | wg-protocol, wg-firmware | ★ | P0 | M2 |
| 12 | lxmf-messaging | LXMF store-and-forward, delivery receipts, retry, priority, propagation nodes | wg-protocol | ★ | P0 | M2 |
| 13 | meshtastic-compat | Clean-room LoRa wire compat (v2.x), channel/PSK translation, capability advertisement | wg-protocol, wg-firmware | ★ | P0 | M2 |
| 14 | voice-subsystem | Opus/Codec2 encode, PTT UX, half-duplex bearer arbitration, jitter/echo suppression | wg-firmware, wg-ui-ux | A, O | P1 | M5 |
| 15 | ui-framework | `ss_ui` toolkit, screen graph, input, notification, theming, localisation, accessibility | wg-ui-ux, wg-firmware | ★ | P0 | M1 |
| 16 | application-layer | Native apps on device: Messages, Contacts, Presence, Location, SOS, Settings, About | wg-firmware, wg-ui-ux | ★ | P0 | M2 |
| 17 | home-gateway | Docked HGW mode: HaLow ↔ LoRa ↔ Wi-Fi ↔ Internet bridge, Wi-Fi extender/AP, captive setup | wg-firmware, wg-cloud | A, O | P0 | M5 |
| 18 | plugin-sandbox | WASM runtime, permission model, plugin manifest, review pipeline, quota enforcement | wg-firmware, wg-security | A, O | P1 | M6 |
| 19 | companion-apps | iOS, Android, macOS, Windows, Linux, Web apps — BLE pairing, contact sync, key backup | wg-apps | ★ | P0 | M4 |
| 20 | sdks | C, Rust, Python, TypeScript, Dart SDKs for RNS/LXMF/plugin dev + wire testkits | wg-sdk | ★ | P1 | M6 |
| 21 | cloud-services | Fleet Console, Relay, Provisioning Service, Plugin Registry, BSL 1.1 | wg-cloud | ★ | P1 | M6 |
| 22 | testing-simulation | Unit, integration, HIL, protocol fuzzer, mesh simulator, chaos, coverage gates | wg-ops, wg-firmware | ★ | P0 | M0 |
| 23 | ci-cd-supply-chain | GitHub Actions matrix, reproducible builds, SBOM, SLSA-3 provenance, signed artifacts | wg-ops | ★ | P0 | M0 |
| 24 | compliance-business-support | FCC/CE/CRA/GDPR filings, warranty, RMA, support portal, marketing/sales, foundation transfer | wg-legal, wg-community | ★ | P0 | M3 |

---

## Cross-cutting dependencies

- **02** blocks 03/04/05.
- **06** and **07** and **08** block **09**.
- **10** blocks **11**, which blocks **12** and **13**.
- **15** blocks **16**.
- **11**+**16**+**17** together deliver M2 launch.
- **19** blocks **21** on the Fleet Console side.
- **22** and **23** are cross-cutting and must be productive before any of 03..21 can ship code.
- **24** is continuous and gates certification/ship milestones.

---

## Epic-to-constitution mapping (clause keys per `00_METHODOLOGY.md` §2.9; per-story mapping in `STORIES_INDEX.md`)

| Epic | Constitution clause keys |
|---|---|
| 01 | C-04, C-06, C-07, C-OA, C-TM, C-CoC |
| 02 | C-00 |
| 03..05 | C-00, C-01, C-08 |
| 06 | C-05, C-SEC |
| 07 | C-05, C-OA |
| 08 | C-05, C-SEC |
| 09 | C-00, C-05, C-OA |
| 10 | C-02, C-08 |
| 11 | C-02, C-08 |
| 12 | C-02, C-08 |
| 13 | C-02, C-04, C-08 |
| 14 | C-00, C-08 |
| 15 | C-03 |
| 16 | C-00, C-03 |
| 17 | C-07, C-08 |
| 18 | C-00, C-05, C-SEC |
| 19 | C-00, C-05 |
| 20 | C-00, C-02 |
| 21 | C-04, C-07 |
| 22 | C-00 |
| 23 | C-06, C-OA |
| 24 | C-06, C-07, C-TM, C-SEC |

---

## Reading order for a new engineer

1. `00_METHODOLOGY.md`
2. `01_BRIEF.md`
3. `02_PRD.md`
4. `03_ARCHITECTURE.md`
5. `04_BLUEPRINT.md`
6. This index.
7. The epics for your Working Group.
8. `STORIES_INDEX.md` when you need to trace a requirement.

---

*End of EPIC_INDEX.*
