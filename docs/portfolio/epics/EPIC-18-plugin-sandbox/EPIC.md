<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# EPIC-18 — Plugin Sandbox (WASM)

**Primary WG:** wg-firmware, wg-security · **Contributing:** wg-sdk
**Priority:** P1 · **SKU:** A, O · **Milestone:** M6

## Outcome
Third-party mini-apps run in a WASM sandbox on-device with capability-based permissions (send/receive messages, read location, access ss_ui). A plugin manifest declares required caps; the review pipeline signs approved plugins; the on-device store enforces quotas.

## Constitution
C-00 `00_MASTER_SOFTWARE_PLAN.md` §plugins; C-05 `05_SECURITY_MODEL.md`; C-SEC `SECURITY.md`.

## Dependencies
EPIC-15, EPIC-16.

## Shards
- **S-18.A WASM runtime** — WAMR or Wasm3 evaluation & integration.
- **S-18.B Plugin manifest v1** — id, ver, caps, publisher, signature.
- **S-18.C Capability system** — messages, location, ui, storage, network.
- **S-18.D Quota enforcement** — CPU/mem/net per plugin.
- **S-18.E Plugin lifecycle** — install, run, suspend, uninstall.
- **S-18.F Signing pipeline** — plugin registry cloud-side.
- **S-18.G Plugin review policy** — SS-SP-Certified, community-approved, sideload.
- **S-18.H Sandbox host functions (syscalls)** — narrow, audited.
- **S-18.I Reference plugins** — hello-world, RSS reader, weather.

## Exit criteria
1. A signed plugin runs and can send an LXMF message under caps.
2. An unsigned plugin refuses to load unless sideload mode enabled.
3. Quota breach terminates plugin cleanly.
4. Uninstall wipes plugin data.

## Risks
| # | Risk | Mitigation |
|---|------|-----------|
| R18-01 | Sandbox escape | External audit + minimal host surface |
| R18-02 | Fragmented plugin ecosystem | Reference SDK + docs |
| R18-03 | Plugin cert supply chain | Registry HSM |
