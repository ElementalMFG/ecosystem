<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# EPIC-16 — On-device Application Layer

**Primary WG:** wg-firmware, wg-ui-ux · **Contributing:** wg-protocol
**Priority:** P0 · **SKU:** ★ · **Milestone:** M2

## Outcome
End-user apps that live on the device: Messages, Contacts, Presence, Location, SOS, Settings, About/Diagnostics. Each app is a screen tree over `ss_ui`, backed by LXMF/RNS calls. First-run onboarding, backup, and restore included.

## Constitution
C-00 `00_MASTER_SOFTWARE_PLAN.md` §applications; C-03 `03_UI_LAYOUT_SPEC.md`.

## Dependencies
EPIC-12, EPIC-15.

## Shards
- **S-16.A Messages app** — thread list, thread view, compose, priority selector.
- **S-16.B Contacts app** — add/edit, address book, key fingerprint verify.
- **S-16.C Presence** — status broadcast, hearts/away/busy.
- **S-16.D Location** — periodic + on-demand share, privacy toggle.
- **S-16.E SOS** — hold-to-arm, cancel window, escalate broadcast.
- **S-16.F Settings** — radios, security, display, updates, about.
- **S-16.G About / Diagnostics** — versions, signals, signed identity.
- **S-16.H First-run onboarding** — language, region, key backup, contacts import.
- **S-16.I Backup / restore** — encrypted backup blob, seed passphrase.
- **S-16.J Notifications routing.**

## Exit criteria
1. All apps navigable on Lite in < 300 ms per screen transition.
2. SOS delivers to all mesh peers with priority.
3. Backup restores on a fresh device.
4. Onboarding completes in ≤ 3 min for a first-time user.

## Risks
| # | Risk | Mitigation |
|---|------|-----------|
| R16-01 | SOS misfire | 3-sec arm, undo window, clear haptic |
| R16-02 | Backup UX confusion | Guided flow + printed seed-card option |
| R16-03 | Locale coverage | Community translation portal (M3+) |
