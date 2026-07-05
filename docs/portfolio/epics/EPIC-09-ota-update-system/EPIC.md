<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# EPIC-09 — OTA Update System

**Primary WG:** wg-firmware, wg-ops · **Contributing:** wg-security, wg-cloud
**Priority:** P0 · **SKU:** ★ · **Milestone:** M2

## Outcome
Devices update themselves safely over any bearer: dual-signed manifests, delta updates, staged rollout with cohorts, automatic rollback on failure, resumable across power loss, transport-agnostic (LoRa fallback for tiny critical updates), reproducible-build attestation verifiable from userspace.

## Constitution
C-05 `05_SECURITY_MODEL.md` §OTA signing; C-08 `08_UNIVERSAL_CONNECTIVITY.md` §OTA; C-OA `governance/OPEN_ASSURANCE.md` §reproducible builds.

## Dependencies
EPIC-06, EPIC-07, EPIC-08.

## Shards
- **S-09.A OTA manifest format** — JSON, signed by RelKey_B and/or RelKey_C, contains version, hash, delta info.
- **S-09.B Delta patcher** — bsdiff/hdiff on device.
- **S-09.C Dual-partition A/B switch** — safe boot after upgrade.
- **S-09.D Staged rollout** — cohort A (1 %), B (10 %), C (50 %), D (100 %).
- **S-09.E Rollback on failure** — first-boot success flag, watchdog on new firmware.
- **S-09.F Bearer-agnostic transport** — Wi-Fi/cellular by default, LoRa for critical patches < 32 KB.
- **S-09.G Reproducible-build attestation** — SLSA-3 provenance file verifiable via `ss-verify`.
- **S-09.H OTA server API** — REST + queue, cohort control.
- **S-09.I Force-update policy** — security-critical updates cannot be indefinitely postponed.

## Exit criteria
1. Lite unit updates itself over Wi-Fi and verifies signature.
2. Failed update rolls back automatically within one boot cycle.
3. Delta update reduces payload ≥ 60 % vs full image.
4. Community-channel manifest (RelKey_C) works when user opted in.
5. `ss-verify` reproduces same hash from source tree.

## Risks
| # | Risk | Mitigation |
|---|------|-----------|
| R09-01 | Bricked device on failed update | A/B partitions + rollback watchdog |
| R09-02 | Manifest replay attack | Anti-rollback counter + nonce |
| R09-03 | Cohort staging bug bricks fleet | Automated pause on error-rate spike |
