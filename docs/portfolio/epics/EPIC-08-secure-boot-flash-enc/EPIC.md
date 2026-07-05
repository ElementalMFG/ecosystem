<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# EPIC-08 — Secure Boot & Flash Encryption

**Primary WG:** wg-firmware, wg-security · **Contributing:** wg-ops
**Priority:** P0 · **SKU:** ★ · **Milestone:** M1

## Outcome
Every SS-SP device boots only firmware signed by RelKey_B (release) or RelKey_C (community channel — user-enabled), verifies the entire chain (bootloader → app), enforces anti-rollback via eFuse counters, encrypts all flash with device-unique AES-256-XTS key, and refuses to boot on tamper.

## Constitution
C-05 `05_SECURITY_MODEL.md` §secure boot & flash encryption; C-SEC `SECURITY.md`.

## Dependencies
EPIC-06, EPIC-07.

## Shards
- **S-08.A Secure Boot v2 setup** — RSA-3072-PSS pubkey burned to eFuse.
- **S-08.B Flash encryption enablement** — device-unique key from eFuse.
- **S-08.C Bootloader signing pipeline** — CI signs with HSM.
- **S-08.D Anti-rollback via eFuse counter.**
- **S-08.E Community-channel opt-in** — user consent flow, warns, sets alt-key.
- **S-08.F Boot chain verifier component** — runtime verification.
- **S-08.G Tamper response** — brownout, chip-erase policy.
- **S-08.H Debug lockdown** — JTAG disable via eFuse, unlock ceremony documented.
- **S-08.I Attestation service on-device** — remote attestation for cloud enrollment.

## Exit criteria
1. Modified firmware refuses to boot (verified on rack).
2. Rollback to older firmware refused after eFuse bump.
3. Flash reads without device key produce ciphertext.
4. Community-channel key enable requires physical button + user typed confirmation.
5. JTAG disabled in production units.

## Risks
| # | Risk | Mitigation |
|---|------|-----------|
| R08-01 | User bricks device with wrong RelKey_C | Recovery-mode requires signed bootloader too |
| R08-02 | eFuse write bug | Line-side dry-run test |
| R08-03 | Debug back-doors leak | External audit of debug paths |
