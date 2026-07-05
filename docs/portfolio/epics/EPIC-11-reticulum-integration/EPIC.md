<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# EPIC-11 — Reticulum (RNS) Integration

**Primary WG:** wg-protocol, wg-firmware · **Contributing:** wg-security
**Priority:** P0 · **SKU:** ★ · **Milestone:** M2

## Outcome
Reticulum Network Stack runs first-class on SS-SP devices as the cryptographic mesh substrate. Multiple RNS interfaces exist (LoRa, HaLow, Wi-Fi, cellular). Path-finding, resource caching, announces, and GET/PUT primitives all work. Devices interoperate with vanilla Python RNS nodes.

## Constitution
C-02 `02_PROTOCOL_STACK.md` §Reticulum; C-08 `08_UNIVERSAL_CONNECTIVITY.md` §bearers; C-04 `04_LICENSING_AND_FORK_STRATEGY.md` §third-party licences.

## Dependencies
EPIC-06, EPIC-10.

## Shards
- **S-11.A `ss_rns` component skeleton** — C port / bindings.
- **S-11.B RNS destination + identity mapping** — SS-SP identity ↔ RNS destination.
- **S-11.C Interface: `ss_rns_iface_lora`** — bridges to SS-Link LoRa bearer.
- **S-11.D Interface: `ss_rns_iface_halow`.**
- **S-11.E Interface: `ss_rns_iface_wifi`.**
- **S-11.F Interface: `ss_rns_iface_cellular`.**
- **S-11.G Announce & path-finding integration.**
- **S-11.H Resource caching layer** — flash-backed.
- **S-11.I Cross-implementation interop tests** — against Python reference RNS.
- **S-11.J License compliance review** — Reticulum licence terms (no-harm, no-AI-training).

## Exit criteria
1. Lite device announces on RNS mesh and is discoverable by a Python RNS node.
2. Resource GET from Python node to device succeeds.
3. Path re-computation on link failure < 30 s.
4. Interop with three RNS reference implementations.

## Risks
| # | Risk | Mitigation |
|---|------|-----------|
| R11-01 | Reticulum licence conditions constrain distribution | Legal review + separate module boundary |
| R11-02 | RNS protocol drift | Track upstream + versioned interface |
| R11-03 | Flash wear from resource cache | Cache size cap + TTL |
