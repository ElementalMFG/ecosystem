<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# EPIC-13 — Meshtastic Wire Compatibility (Clean-room)

**Primary WG:** wg-protocol, wg-firmware · **Contributing:** wg-legal, wg-security
**Priority:** P0 · **SKU:** ★ · **Milestone:** M2

## Outcome
SS-SP devices interoperate with the Meshtastic v2.x LoRa mesh at the wire level via a clean-room reimplementation (no derived code from GPL-3.0 Meshtastic firmware). Channels, PSKs, node info, positions, messages, and admin commands supported to the extent the constitution allows. No SS-SP crypto material ever crosses to Meshtastic side.

## Constitution
C-04 `04_LICENSING_AND_FORK_STRATEGY.md` §Meshtastic clean-room; C-02 `02_PROTOCOL_STACK.md`; C-08 `08_UNIVERSAL_CONNECTIVITY.md`.

## Dependencies
EPIC-06, EPIC-10 (LoRa bearer).

## Shards
- **S-13.A Wire spec extraction** — from public docs + observed traffic only, no source view.
- **S-13.B Clean-room protobuf regeneration** — from `.proto` files (Apache-2.0 licensed).
- **S-13.C Channel + PSK mapping.**
- **S-13.D Node-info + neighbours.**
- **S-13.E Text message send/receive.**
- **S-13.F Position + telemetry.**
- **S-13.G Store-and-forward (SFR).**
- **S-13.H Admin messages (documented subset).**
- **S-13.I Legal firewall audit** — proof of clean-room process.
- **S-13.J Capability advertisement** — SS-SP flag for feature negotiation.

## Exit criteria
1. SS-SP Lite exchanges text with commercial Meshtastic device on same channel.
2. Position report is visible on Meshtastic client.
3. No file in `firmware/components/ss_meshtastic_compat/` references GPL-3.0 code.
4. Legal review confirms clean-room process.

## Risks
| # | Risk | Mitigation |
|---|------|-----------|
| R13-01 | Meshtastic breaks wire on v3 | Version negotiation + feature gating |
| R13-02 | Copyright challenge | Documented clean-room process, dev logs |
| R13-03 | Feature parity expectations | Scope statement in docs |
