<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# EPIC-19 — Companion Apps (iOS, Android, Desktop, Web)

**Primary WG:** wg-apps · **Contributing:** wg-ui-ux, wg-security, wg-cloud
**Priority:** P0 · **SKU:** ★ · **Milestone:** M4

## Outcome
Cross-platform companion apps pair with a SS-SP device over BLE (or Wi-Fi when local), sync contacts and messages, back up keys, provision the device, act as a bigger keyboard/screen, and (optionally) mirror the LXMF inbox.

## Constitution
C-00 `00_MASTER_SOFTWARE_PLAN.md` §apps; C-05 `05_SECURITY_MODEL.md` §pairing & key backup.

## Dependencies
EPIC-03/04 (BLE), EPIC-12.

## Shards
- **S-19.A Shared TypeScript core** — protocol, crypto, storage, sync.
- **S-19.B iOS app** — SwiftUI shell over core (via Kotlin Multiplatform or WASM).
- **S-19.C Android app** — Jetpack Compose shell.
- **S-19.D macOS + Windows + Linux desktop** — Tauri.
- **S-19.E Web app** — Web-BLE / WebSerial for provisioning.
- **S-19.F BLE pairing UX + fingerprint verify.**
- **S-19.G Key backup / restore** — device seed + optional cloud (E2EE).
- **S-19.H Inbox mirroring policy.**
- **S-19.I Fleet-Console operator role.**

## Exit criteria
1. iOS + Android beta apps in TestFlight / Play Console.
2. Desktop app builds for macOS, Windows, Linux from single Tauri source.
3. Pairing + first message round-trip completes in ≤ 90 s from install.
4. E2EE cloud key backup restores on second device.

## Risks
| # | Risk | Mitigation |
|---|------|-----------|
| R19-01 | Store review rejection | Follow both stores' comms-device rules |
| R19-02 | Web-BLE browser support | Progressive enhancement |
| R19-03 | Cross-platform code drift | Shared core + coverage floor |
