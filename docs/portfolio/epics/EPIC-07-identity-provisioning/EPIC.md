<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# EPIC-07 — Identity & Provisioning

**Primary WG:** wg-security · **Contributing:** wg-firmware, wg-ops, wg-legal
**Priority:** P0 · **SKU:** ★ · **Milestone:** M1

## Outcome
Every SS-SP device leaves the factory with a unique keypair, a signed device certificate, provisioning metadata, and a chain of trust rooted in RelKey_A (root), RelKey_B (release signer), RelKey_C (community-channel signer). The provisioning line runs on the factory floor, offline-capable, HSM-backed, audit-logged.

## Constitution
C-05 `05_SECURITY_MODEL.md` §identity & key hierarchy; C-OA `governance/OPEN_ASSURANCE.md` §keys.

## Dependencies
EPIC-06 (crypto core).

## Shards
- **S-07.A Root of trust ceremony** — RelKey_A generation in air-gapped enclave, M-of-N shard split.
- **S-07.B Release signer key** — RelKey_B on HSM, rotation cadence, revocation plan.
- **S-07.C Community-channel key** — RelKey_C in escrow, transfer clause to foundation.
- **S-07.D Per-device keypair** — Ed25519 device identity, X25519 KEM, sealed to secure element.
- **S-07.E Device certificate** — SS-SP DevCert format, chain to RelKey_A.
- **S-07.F Provisioning line tool** — `tools/provisioning-line/`, USB/JTAG interface.
- **S-07.G Manufacturing HSM integration** — YubiHSM or NitroKey HSM.
- **S-07.H Provisioning audit log** — signed line entries, offline verifier.
- **S-07.I Reprovisioning / factory reset flow** — user-initiated, cryptographically clean.

## Exit criteria
1. RelKey_A ceremony transcript published (with public part).
2. RelKey_B/C usable via HSM in signing pipeline.
3. Provisioning line provisions ≥ 100 devices/hour on a test rig.
4. DevCert verifies on-device against embedded root.
5. Factory reset wipes all keys and re-attests to first-boot state.

## Risks
| # | Risk | Mitigation |
|---|------|-----------|
| R07-01 | Root-key compromise | Air-gap + M-of-N + geographic shard split |
| R07-02 | Provisioning-line supply-chain attack | Signed provisioning firmware, checksum on line |
| R07-03 | Escrow custodian failure | Named foundation escrow contract |
