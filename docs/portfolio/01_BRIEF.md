<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# 01 — SS-SP Program Brief

*Executive summary. Two-page read. Reader: anyone new to the program.*

---

## Problem

The world's communication infrastructure is centralised, fragile, surveilled, and monetised against its users. When cell towers fail (disaster, outage, deliberate takedown, wilderness), most people are cut off. Even when networks are up, users cannot verify their messages are private, unaltered, or free of surveillance. Existing off-grid alternatives — Meshtastic radios, satellite communicators, ham nets — are either hobbyist-grade, expensive, closed, or fragmented across incompatible ecosystems.

## Product

SS-SP (Seekie-Speakie Smart Pager) is a family of pocket-sized, rugged, sovereign mesh communicators that:

- Send and receive **text, voice, presence, location, SOS** over LoRa, Wi-Fi HaLow, Wi-Fi 2.4/5, Bluetooth LE, and (Omega tier) cellular.
- Form a **self-healing mesh** that works with zero infrastructure.
- Interoperate with **Meshtastic** networks out of the box (clean-room LoRa compat).
- Provide **end-to-end encryption** with forward secrecy on every bearer.
- Turn into a **Home Gateway** when docked — bridging LoRa/HaLow to the internet and acting as a Wi-Fi range extender.
- Ship with **open-source firmware and apps from day one** (Apache-2.0), no telemetry, no ads, no data sale.

## Product tiers

- **Lite** — ESP32-S3 + LoRa + Wi-Fi 2.4 + BLE. Entry SKU. Ships first.
- **Alpha** — ESP32-P4 + LoRa + Wi-Fi HaLow + Wi-Fi 5 + BLE + GNSS + IMU. Flagship.
- **Omega** — Alpha + LTE-M/NB-IoT + optional LEO satellite. Enterprise / expedition.

## Users

- **Off-grid citizens** — hikers, sailors, overlanders, rural residents.
- **First responders and humanitarian workers** — disaster zones, refugee support, wilderness rescue.
- **Sovereignty-conscious individuals** — journalists, activists, security-aware civilians.
- **Enterprises and fleets** — logistics, mining, energy, forestry, agriculture, security firms.
- **Public-safety agencies** — municipal emergency management, tribal networks, park services.
- **Hobbyists and mesh-network enthusiasts** — the Meshtastic-adjacent community.

## Non-users (out of scope)

- Consumers wanting a phone replacement with unlimited streaming.
- Users whose threat model requires anonymity from all state-level actors under all conditions (we defend against most classes; nation-state APT is out of full scope but partially mitigated by the security model).

## Outcomes (measurable)

1. Hundreds of thousands of devices shipped within a five-year window.
2. Three or more independent hardware manufacturers producing `SS-SP-Certified` devices.
3. Interoperability with 100 % of Meshtastic v2.x devices on LoRa.
4. Zero known backdoors, zero user-data commercialisation incidents.
5. Community edition firmware runs on 100 % of vendor-shipped hardware.
6. Trademark transfer to the SS-SP Foundation on the Phase-2 schedule.
7. Firmware Common Criteria EAL-4 or equivalent for Omega tier.

## Non-outcomes (explicitly not pursued)

- Vendor lock-in of any kind.
- Recurring subscription for essential connectivity.
- Cloud-mediated identity or messaging as a prerequisite.
- Retroactive licence changes on published code.

## Approach in one sentence

**Sell the hardware. Give away the software. Bridge every radio. Never rug-pull the community.**

## What makes it defensible

- **Hardware capital and pipeline** — the physical devices are non-trivial to design, source, certify (FCC/CE), and manufacture.
- **Brand and cryptographic identity** — `SS-SP-Certified` trademark plus `RelKey_A/B/C` signing keys we control.
- **Reticulum-native from day one** — the fastest mainstream device family to ship RNS + LXMF as a first-class transport.
- **Home Gateway Mode** — the marquee feature no competitor ships: your home becomes a neighbourhood mesh bridge.
- **Multi-bearer unified layer (SS-Link)** — one abstraction over LoRa + HaLow + Wi-Fi + BLE + cellular; competitors ship stacks per radio.

## What we bet on

- Wi-Fi HaLow (802.11ah) becomes the neighbourhood-scale mesh substrate this decade.
- Reticulum grows into the de-facto cryptographic mesh substrate for off-grid comms.
- Regulators (CRA, EU AI Act, Colorado AI Act) increasingly favour open, auditable, provenance-tracked stacks.
- User trust matures into a hard requirement, not a marketing preference.

## What we do NOT bet on

- Any single silicon vendor being available forever (HAL abstracts).
- Any single OS or app framework being available forever (SDK plurality).
- Cellular incumbents being friendly (we treat cellular as an optional bearer, not a dependency).

## Governance posture

Founded by a single commercial vendor entity, evolving through explicit governance phases to a neutral foundation (Linux Foundation / Apache / Eclipse / equivalent) by Phase 2. Governance rules, RFC process, and anti-rug-pull commitments are codified in `06_GOVERNANCE.md` and `governance/OPEN_ASSURANCE.md`.

## Legal posture

- Firmware and apps: Apache-2.0 forever.
- Specs: CC-BY-4.0.
- Cloud services: BSL 1.1 auto-converting to Apache-2.0 at 4 years.
- No fork of GPL-3.0 code (Meshtastic firmware and protobufs) — clean-room wire compat only.
- Reticulum and LXMF adopted under their custom licences, honouring no-harm and no-AI-training clauses.

## Team shape at launch

Small founding vendor team with defined Working Groups (per `06_GOVERNANCE.md`) that scale as external contributors and fleet customers arrive. Founding roles: firmware, hardware, protocol, security, apps, cloud, ops, community, docs.

## First deliverables (M0)

- Constitution complete: docs 00–08, OPEN_ASSURANCE, TRADEMARK. **Done in this repo.**
- Portfolio artifacts complete: brief, PRD, architecture, blueprint, 24 epics with stories. **This portfolio.**
- Monorepo skeleton, HAL contracts, Lite `board_config.h`, all repo meta files. **Done in this repo.**

## Success signal for M0 → M1 transition

The Lite reference unit boots the SS-SP firmware, joins a Reticulum mesh over LoRa, exchanges LXMF messages with another Lite, interoperates with an off-the-shelf Meshtastic device, and OTA-updates via a dual-signed community-channel manifest. Reproducible-build attestation verifies.

## Success signal for M1 → M2 transition

The Alpha reference unit adds HaLow, Wi-Fi 5, GNSS, IMU. Home Gateway Mode active. Companion mobile apps (iOS + Android) shipping in beta. Fleet Console usable by one paying pilot customer.

## Success signal for M2 → M3 transition

Omega reference unit adds cellular. Third-party SS-SP-Certified device from an independent manufacturer shipping. Foundation transition RFC in FINAL CALL.

---

*See `02_PRD.md` for requirements detail.*

*End of 01 — Brief.*
