<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# 04 — SS-SP Delivery Blueprint

*Delivery ordering, RACI, milestone gates, risk register. No calendar dates.*

Prerequisite reading: `01_BRIEF.md`, `02_PRD.md`, `03_ARCHITECTURE.md`.

---

## 1. Milestone gates

Milestones are ORDERED, not dated. Progress from M0 to M9. Each milestone has explicit **entry criteria** (what must be true to start work toward it) and **exit criteria** (what must be true to declare it complete).

### M0 — Foundation complete
Entry: repo bootstrapped, constitution drafted.
Exit: Docs 00–08 exist and are signed off by wg-security + wg-legal. Portfolio (this dir) complete. Monorepo tree in place. Repo meta files in place. **Achieved in this repo.**

### M1 — Lite bring-up
Entry: M0 complete. Board_config.h for Lite pinned. HAL headers finalised.
Exit: Two Lite prototypes exchange an LXMF message over LoRa. LVGL renders a text screen. Buzzer beeps on incoming. CI green.

### M2 — Lite v1.0 ship
Entry: M1 complete.
Exit: All F-MSG-* and F-BR-01/02/03/07 functional. Companion mobile apps in public beta. Community OTA channel published with signed release. Reproducible build attested. FCC/CE certification passed for Lite. First 100 units shipped to seed users.

### M3 — Alpha bring-up
Entry: M2 complete. Alpha PCB rev-1 fabricated.
Exit: HaLow ↔ Wi-Fi ↔ LoRa demonstrated on Alpha. GNSS lock. IMU stream. Bezel LEDs animated.

### M4 — Alpha v1.5 ship
Entry: M3 complete.
Exit: All F-BR-* except cellular. Home Gateway Mode full. Fleet Console early access with first pilot customer. FCC/CE for Alpha.

### M5 — Cloud v2
Entry: M4 complete.
Exit: Fleet Console GA. Provisioning Service GA. Plugin Registry GA. First 5 paying fleet tenants.

### M6 — Omega bring-up
Entry: M4 complete. Omega PCB rev-1 fabricated.
Exit: Cellular LTE-M functional on Omega. Cellular billing controls verified.

### M7 — Omega v2.0 ship
Entry: M6 complete.
Exit: All F-BR-* functional. Certification portal open for third-party manufacturers.

### M8 — First third-party Certified device
Entry: M7 complete. Certification test suite v1.0 published.
Exit: One external manufacturer's device passes Certification and ships publicly bearing the badge.

### M9 — Foundation transition
Entry: M7 complete. Governance Phase 2 criteria met (per `06_GOVERNANCE.md` §2).
Exit: Trademarks transferred to the SS-SP Foundation. Infrastructure moved. Documentation updated. First Foundation board meeting held.

---

## 2. Workstream map

Twelve concurrent workstreams. Each maps to Working Groups (WGs) per `06_GOVERNANCE.md` §3 and to Epics in this portfolio.

| WS  | Workstream                          | WG owner       | Epics (primary) |
|-----|-------------------------------------|----------------|-----------------|
| WS01| Governance & Constitution           | wg-community   | EPIC-01 |
| WS02| Firmware Foundation                 | wg-firmware    | EPIC-02 |
| WS03| HAL — Lite / Alpha / Omega          | wg-firmware+hw | EPIC-03, 04, 05 |
| WS04| Cryptography & Identity             | wg-security    | EPIC-06, 07 |
| WS05| Secure Boot & OTA                   | wg-security    | EPIC-08, 09 |
| WS06| Networking & Protocol               | wg-protocol    | EPIC-10, 11, 12, 13 |
| WS07| Voice & Audio                       | wg-firmware    | EPIC-14 |
| WS08| UI & Application Layer              | wg-ui-ux       | EPIC-15, 16 |
| WS09| Home Gateway & Plugins              | wg-firmware+p  | EPIC-17, 18 |
| WS10| Companion Apps & SDKs               | wg-apps + sdk  | EPIC-19, 20 |
| WS11| Cloud Services                      | wg-cloud       | EPIC-21 |
| WS12| Testing, CI/CD, Compliance          | wg-ops + legal | EPIC-22, 23, 24 |

---

## 3. Dependency graph (epic-level)

```
EPIC-01 (Governance) ─── prerequisite of EVERYTHING
   │
   ▼
EPIC-02 (Firmware Foundation)
   │
   ├──▶ EPIC-03 (HAL Lite) ──▶ M1
   │         │
   │         ├──▶ EPIC-04 (HAL Alpha) ──▶ M3
   │         │         │
   │         │         └──▶ EPIC-05 (HAL Omega) ──▶ M6
   │
   ├──▶ EPIC-06 (Crypto Core) ──▶ EPIC-07 (Identity) ──▶ EPIC-08 (Secure Boot) ──▶ EPIC-09 (OTA)
   │
   ├──▶ EPIC-10 (SS-Link) ──▶ EPIC-11 (Reticulum) ──▶ EPIC-12 (LXMF)
   │         │
   │         └──▶ EPIC-13 (Meshtastic compat)
   │
   ├──▶ EPIC-14 (Voice) — requires HAL audio + SS-Link
   │
   ├──▶ EPIC-15 (UI) ──▶ EPIC-16 (App layer) — requires HAL display + SS-Link
   │
   ├──▶ EPIC-17 (Home Gateway) — requires SS-Link + Wi-Fi + HaLow + Meshtastic-compat
   │
   ├──▶ EPIC-18 (Plugin Sandbox) — requires Crypto + Storage
   │
   ├──▶ EPIC-19 (Companion apps) — requires SDK + Pairing
   │
   ├──▶ EPIC-20 (SDKs) — requires stable protocol
   │
   ├──▶ EPIC-21 (Cloud) — requires Provisioning + OTA + Plugin Registry
   │
   ├──▶ EPIC-22 (Testing) — cross-cuts all
   │
   ├──▶ EPIC-23 (CI/CD supply-chain) — cross-cuts all
   │
   └──▶ EPIC-24 (Compliance / Business / Support) — cross-cuts all
```

Critical path for M2 (Lite v1.0):
`EPIC-01 → EPIC-02 → EPIC-03 → EPIC-06 → EPIC-07 → EPIC-08 → EPIC-09 → EPIC-10 → EPIC-11 → EPIC-12 → EPIC-13 → EPIC-14 → EPIC-15 → EPIC-16 → EPIC-19 → EPIC-22 → EPIC-23 → EPIC-24`

Critical path for M4 (Alpha v1.5): add `EPIC-04 → EPIC-17 → EPIC-18 → EPIC-21`.

Critical path for M7 (Omega v2.0): add `EPIC-05`.

---

## 4. RACI matrix (workstream × role)

Roles: **R** = Responsible (does the work), **A** = Accountable (single point owner), **C** = Consulted, **I** = Informed.

|            | Firmware | Hardware | Protocol | Security | UI/UX | Apps | Cloud | SDK | Legal | Docs | Ops | Community |
|------------|----------|----------|----------|----------|-------|------|-------|-----|-------|------|-----|-----------|
| WS01 Gov   | I        | I        | C        | C        | I     | I    | I     | I   | R     | R    | I   | **A**     |
| WS02 FW    | **A** R  | C        | C        | C        | I     | I    | I     | I   | I     | C    | C   | I         |
| WS03 HAL   | R        | **A** R  | I        | C        | I     | I    | I     | I   | I     | C    | C   | I         |
| WS04 Crypto| C        | I        | C        | **A** R  | I     | I    | I     | C   | C     | C    | I   | I         |
| WS05 Boot/OTA| R      | C        | I        | **A** R  | I     | I    | C     | I   | C     | C    | R   | I         |
| WS06 Net   | R        | C        | **A** R  | R        | I     | I    | C     | R   | I     | C    | I   | I         |
| WS07 Voice | **A** R  | C        | I        | C        | R     | I    | I     | I   | C     | C    | I   | I         |
| WS08 UI/App| R        | I        | C        | C        | **A** R| R   | I     | R   | I     | R    | I   | I         |
| WS09 HGW+Plug| **A** R| C        | R        | R        | I     | I    | C     | I   | C     | C    | I   | I         |
| WS10 Apps+SDK| I     | I        | C        | C        | R     | **A** R| I   | R   | I     | R    | I   | I         |
| WS11 Cloud | I        | I        | C        | R        | I     | I    | **A** R| C   | C     | C    | R   | I         |
| WS12 Test/CI/Comp| R  | R        | R        | R        | R     | R    | R     | R   | R     | R    | **A** R| C      |

Every workstream has one Accountable owner. That role signs off on epic completion.

---

## 5. Rollout gates (per SKU)

Each SKU (Lite, Alpha, Omega) must pass **all** gates before commercial shipment.

- **G1 — Hardware validation:** DVT complete, EMC pre-scan clean, environmental tests passed.
- **G2 — Firmware GA:** all in-scope PRD requirements green in CI; reproducible build attested.
- **G3 — Security review:** wg-security sign-off; third-party pentest report reviewed.
- **G4 — Regulatory:** FCC/CE (and applicable regional) certifications granted.
- **G5 — Compliance:** GDPR readiness, CRA SBOM, VDP live.
- **G6 — Provisioning:** factory line qualified with HSM ceremony validated.
- **G7 — Support:** RMA process live; docs published; support team trained.
- **G8 — Companion:** iOS + Android apps in respective stores or public TestFlight/Play Beta.
- **G9 — OTA:** community OTA channel serving signed releases; rollback tested.
- **G10 — Launch:** marketing materials factual; Open Assurance countersigned in transparency log.

Gate failure = ship deferred. No partial-ship exceptions.

---

## 6. Risk register (top 20)

Codes: **P** = Probability (H/M/L), **I** = Impact (H/M/L).

| # | Risk | P | I | Owner | Mitigation |
|---|------|---|---|-------|------------|
| R01 | HaLow ecosystem doesn't mature | M | M | wg-hw | LoRa carries UX; HaLow is upside |
| R02 | FCC/CE certification delay | M | H | wg-legal | Parallel pre-scan at DVT; buffer builds |
| R03 | Reticulum backwards-incompat | L | H | wg-protocol | Pin version; contribute upstream; LTS fork last resort |
| R04 | Meshtastic community perception | M | M | wg-community | Neutral, factual comms; contribute test vectors |
| R05 | Cellular billing runaway | M | H | wg-firmware | Hard user cap; Fleet policy |
| R06 | Duress PIN false trigger | L | H | wg-security | 500 ms hold + pattern; false-positive telemetry opt-in |
| R07 | Plugin marketplace attack surface | H | H | wg-security | Sandbox + signed + review + revocation |
| R08 | BSL → Apache clock-reset perception | L | M | wg-legal | Immutable Change Date; annual audit |
| R09 | Single-silicon dependence (ESP32) | M | H | wg-hw | HAL abstracts; second-source SoC evaluated M4 |
| R10 | Founder-vendor bankruptcy | L | H | wg-legal | Full escrow; devices self-sufficient |
| R11 | Fleet Console adoption stalls | M | M | wg-cloud | Personal-tier free; humanitarian outreach |
| R12 | Duplicate/clone hardware confuses buyers | M | M | wg-community | Certification badge; transparency log |
| R13 | Supply chain compromise of dep | M | H | wg-security | SBOM + Sigstore + reproducible builds |
| R14 | On-device LLM privacy incident | L | H | wg-security | Omega-only; explicit user opt-in |
| R15 | LoRa airtime regulator changes | L | M | wg-protocol | Duty-cycle-honouring code path; regional profile |
| R16 | Companion app store rejection | M | M | wg-apps | App Store liaison; alt distribution channel |
| R17 | Insufficient contributor base | M | M | wg-community | Foundation transition roadmap; grants |
| R18 | Escrow instrument legal defect | L | H | wg-legal | External counsel; independent trustee |
| R19 | Voice codec (Codec2 LGPL) contamination | L | M | wg-legal | Loadable-module isolation; Opus default |
| R20 | Firmware sizes exceed flash | M | M | wg-firmware | Size CI gate; feature flags per SKU |

Full risk register maintained in `docs/dev/risk_register.md` and reviewed at every SC meeting.

---

## 7. Release cadence and channels

- **Community OTA:** monthly minor + patch as needed. LTS branches per SKU with 5-year security fix commitment.
- **Fleet channel:** staged rollouts 5 %/25 %/100 % with automatic rollback triggers.
- **Companion apps:** aligned to firmware minor releases; app stores' own beta programs used.
- **Cloud services:** continuous deployment; feature flags; blue/green.

Each release has:
- Semver bump.
- Signed manifest.
- Signed release notes.
- Sigstore attestation.
- Rebuild reproducibility receipt.
- SBOM (CycloneDX).
- Warrant canary refresh.

---

## 8. Launch criteria (Lite v1.0 illustrative)

Concrete gate list to enter shipping:

- All P0 stories in EPIC-01..EPIC-03, EPIC-06..EPIC-16, EPIC-19, EPIC-22..EPIC-24 marked DONE.
- All P1 stories on the M2 critical path marked DONE.
- Battery-life test result meets NF-PWR-01.
- OTA update test on 100-unit fleet: 100 % success or documented mitigation.
- FCC ID granted; CE DoC signed.
- GDPR Data Protection Impact Assessment signed.
- SBOM published.
- Reproducible-build attestation verified by two independent parties.
- Third-party pentest scoped and scheduled (annual thereafter).
- Support runbooks published in `docs/user/`.
- Warranty terms published.
- Open Assurance countersigned in Sigstore Rekor.

---

## 9. Deferred / backlog capabilities

Explicitly deferred, tracked in stories but scheduled after M7:

- LEO satellite bearer.
- Ku-band point-to-point.
- On-device LLM assist (Omega).
- Marine AIS gateway plugin.
- Aviation ADS-B receive plugin.
- Multi-device account sync.
- Web-of-trust key discovery beyond QR handshake.
- Formal verification of ratchet code paths.

---

## 10. Escalation and decisions

- **Working-Group-level:** WG chair decides; recorded in `governance/decisions.md`.
- **Cross-WG:** Steering Committee vote.
- **Constitutional:** RFC + SC supermajority.
- **Security emergency:** Security Response Team, per `05_SECURITY_MODEL.md` §14.

Every decision cited in this Blueprint has (or will have) a `governance/decisions.md` entry.

---

*See `EPIC_INDEX.md` for the epic catalogue.*

*End of 04 — Blueprint.*
