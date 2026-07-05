# Security Policy

The SS-SP project takes security seriously. Devices are used in emergencies; regressions cost lives. Please help us keep the ecosystem safe.

## Reporting a vulnerability

**Do not** file public GitHub issues or forum posts for security matters.

Contact: **security@ss-sp.org** (PGP key fingerprint and full key will be published at `security.ss-sp.org`).

Please include:

- Affected component (firmware / protocol / companion / cloud / SDK).
- Affected version(s).
- Reproduction steps or proof-of-concept.
- Impact (CIA triad + safety consequences).
- Your preferred disclosure timeline (default 90 days).

## What to expect

1. **Acknowledgement** within 24 hours (business hours).
2. **Triage** within 72 hours: severity classified by CVSS 4.0.
3. **Fix development** on an embargoed branch. We may loop in you, upstream partners (Espressif, Morse Micro, etc.), and coordinating parties (Meshtastic for wire-format-relevant issues) as needed.
4. **Coordinated disclosure**: default 90-day embargo; earlier if a fix is trivial and low-risk, longer for complex hardware fixes.
5. **CVE assignment** via our CNA arrangement.
6. **Public advisory** at `security.ss-sp.org`, signed.
7. **Post-mortem** for P1/P2 issues within 30 days.

## Scope

In scope:

- Firmware for Lite, Alpha, Omega, and OEM board ports (`firmware/**`).
- Bootloader and secure-boot chain (`firmware/bootloader/**`).
- Protocol implementations and wire compat (`components/ss_lxmf`, `components/ss_meshtastic_compat`, `components/ss_rns`, `protocol/**`).
- Cryptography (`components/ss_crypto`).
- OTA (`components/ss_ota`).
- Companion apps.
- Cloud fleet console.
- SDKs.

Out of scope:

- Physical attacks that require sustained lab access to a stolen device (documented in the threat model at `05_SECURITY_MODEL.md` §1.2 T8).
- Social engineering of employees.
- Denial-of-service against RF bearers via unlimited jamming.
- Findings in third-party components already-known-fixed upstream.

## Safe harbor

We will not pursue legal action against researchers who:

- Report vulnerabilities in scope.
- Give us reasonable time to respond and fix.
- Do not exfiltrate user data.
- Do not attack customer devices in the field without written consent.
- Comply with applicable law.

## Bug bounty

A public bounty program will be operated when the project reaches Phase 1 governance (see `06_GOVERNANCE.md`). Program details will be published at `security.ss-sp.org/bounty`.

## Encryption keys

Two intake keys are published (both accepted; age preferred):

| Type | Fingerprint / recipient | Status |
|------|------------------------|--------|
| age  | `age1…` | *pending — generated on human-custodied hardware and published with the first tagged release (tracked as VA-04 in `docs/portfolio/09_VENTURE_EXECUTION_MAP.md`)* |
| PGP  | `XXXX XXXX XXXX XXXX XXXX  XXXX XXXX XXXX XXXX XXXX` | *pending — same event* |

Keys will also be served at `security.ss-sp.org/keys.txt` and via WKD. Revocation certificates are held offline. Until publication, unencrypted reports to `security@ss-sp.org` are accepted — omit exploit details and we will establish an encrypted channel within the acknowledgement window.

## Hall of fame

Researchers who report valid, in-scope vulnerabilities are credited here (with their consent) after coordinated disclosure completes:

- *No entries yet — be the first.*
