<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# 09 — Venture Execution Map

*Decision record: how the entire venture — firmware, apps, cloud, ecosystem, and business — is carried from the PASS-certified portfolio (533 stories, `07_FINAL_READINESS_TRIAGE.md` §7) into a shipping company. Companion to `04_BLUEPRINT.md` (which orders the buildable work); this document orders the **non-buildable** work.*

---

## 0. Purpose and the two-class model

Everything that must be done to realize SS-SP falls into exactly two classes:

| Class | What it is | Where it lives | Who executes |
|---|---|---|---|
| **Class 1 — Buildable work** | Code, specs, tests, docs, configs, runbooks, policies | The 533 stories in `epics/` | Engineering (incl. AI-assisted sessions) under the 12-gate DoD (`00_METHODOLOGY.md` §5) |
| **Class 2 — Venture actions (VA)** | Accounts, money, legal entity, hardware purchases, labs, external parties | **This document, §2** | A human with signing/spending authority |

The portfolio was audited (T-01..T-20) as complete for Class 1. This document closes the last structural gap: **Class 2 was implied by stories but never registered as first-class, tracked work.** Every VA item below is the *human* half of one or more stories; the stories themselves remain the sole source for the buildable half.

**Maintenance rule:** any newly discovered real-world action is appended to §2 with the next `VA-nn` number, its gating milestone, and the stories it unblocks — mirroring the `T-nn` triage ledger convention.

**Non-timeline rule applies:** this register orders actions by **gating milestone** (M0..M9 per `04_BLUEPRINT.md` §1), never by calendar date. Cost bands are order-of-magnitude planning aids, not quotes.

---

## 1. How the two streams interleave

Class 1 and Class 2 run **in parallel**. Class 1 proceeds story-by-story down the dependency DAG (`04_BLUEPRINT.md` §3); Class 2 items are executed **just-in-time, ahead of the milestone gate they unblock**. The standing rule:

> Before declaring milestone M*n* entry, every VA item whose *Gate* column reads M*n* must be `DONE`.

Nothing in Class 2 blocks *starting* Class 1: firmware, apps, cloud, and SDK code can all be written and CI-tested with only VA-01/VA-02 complete.

```
Milestone   Class 1 (stories)                              Class 2 (VA items)
─────────   ─────────────────────────────────────────────  ─────────────────────────────
M0 DONE     Portfolio certified (533 stories, PASS)        — (achieved)
M1          EPIC-01 READY → EPIC-02 → EPIC-03 Lite HAL     VA-01..05  (toolchain, repo host,
            → two-device LXMF-over-LoRa demo                boards, security keys, domains)
M2          Crypto→OTA→mesh→UI→apps; iOS/Android beta      VA-06..15  (entity, insurance, labs,
            (EPIC-06..16, 19); cert test plans (EPIC-24)    CM, app stores, payments, CLA, CNA)
M3–M4       Alpha HAL, HaLow, Home Gateway, Fleet early    VA-16..20  (cloud accts, KMS/HSM, WFA,
            (EPIC-04, 17; EPIC-21 early access)             Alpha PCB, pilot, HIL bench)
M5          Cloud GA: billing, marketplace, relays         VA-13/16 live; first 5 paying tenants
M6–M7       Omega, cellular, certification test suite      VA-21..23  (carrier/MVNO, NTN, bounty)
M8–M9       Third-party certified device; foundation       VA-24..25  (counsel, escrow, transfer)
Continuous  EPIC-22/23/24 cross-cut all milestones         VA-26..28  (compliance, community, ops)
```

---

## 2. The Venture-Action register

Status vocabulary: `TODO` → `IN-PROGRESS` → `DONE`. Cost bands: `$` <500, `$$` 500–5 k, `$$$` 5–50 k, `$$$$` >50 k (USD, order of magnitude).

### 2.1 Gate M1 — Lite bring-up

| VA | Action | Unblocks | Cost | Status |
|---|---|---|---|---|
| VA-01 | Provide the pinned firmware toolchain: pull the digest-pinned `espressif/idf` v5.3.5 container (see `ci/containers/firmware/Dockerfile`) **or** install ESP-IDF v5.3.5 natively. Verification = `idf.py build` green from a clean container. | S-02-001 final gate; every firmware story | $ (free) | DONE — digest-pinned container pulled; clean containerized `idf.py build` green for Lite (`ss_sp_firmware.bin`, 292 KB); unknown-board and `BOARD` env paths verified |
| VA-02 | Create the GitHub organization; push this repo; enable branch protection on `main` (required checks: `firmware-build`, `dco`), CODEOWNERS mapping paths→WGs, two-approver rule; install/enable DCO enforcement (workflow present at `.github/workflows/dco.yml` — mark it a required check). | S-01-007 enforcement, S-01-012, S-02-001 CI, all merges | $ (free) | DONE — org+repo live, `main` pushed to `github.com/ElementalMFG/ecosystem` (D-0011, public); branch protection with required checks `dco`/`lint-docs`/`build (lite)`, founder bypass during solo bootstrap (D-0014); two-approver rule deferred to second-maintainer supersession per D-0014 |
| VA-03 | Purchase M1 demo hardware: 2× Elecrow CrowPanel Advance 3.5″ (ESP32-S3) + 2× LoRa modules per `01_SS-SP_LITE_HARDWARE_REFERENCE.md`, plus cables/antennas. | M1 exit demo; EPIC-03 HIL; S-22 on-target tests | $ | DONE — 2× units in hand, both HaLow-fitted; C6 + GNSS + compass modules staged (D-0013) |
| VA-04 | Generate the security-intake keypair(s) — age **and** PGP — on human-custodied hardware (ideally a hardware token; never inside a dev container or CI). Publish fingerprints in `SECURITY.md` §PGP key and at `security.ss-sp.org`. Store revocation certificates offline. | S-01-005 final gate; vulnerability intake | $ | TODO |
| VA-05 | Register domains (`ss-sp.org` and defensive variants), set up DNS, and mail routing for `security@`, `conduct@`, `dev@lists.` addresses already referenced by SECURITY.md / CODE_OF_CONDUCT.md / CONTRIBUTING.md. | S-01-004/005 addresses live; S-24-017 portal later | $–$$ /yr | DEFERRED — interim contact runs on `elementalmfg.com` (D-0012); brand domain when product naming settles |

### 2.2 Gate M2 — Lite v1.0 ship (first revenue)

| VA | Action | Unblocks | Cost | Status |
|---|---|---|---|---|
| VA-06 | Form the legal entity (LLC or C-corp per counsel), obtain EIN, open business bank account, engage bookkeeping/accounting. All contracts below hang off this. | Every contract, sale, and filing below | $$ | TODO |
| VA-07 | Obtain general liability + **product liability** insurance (hardware sold for emergency use raises the bar — disclose intended-use wording per `02_PRD.md` safety disclaimers). | Selling the first 100 units (M2 exit) | $$–$$$ /yr | TODO |
| VA-08 | File trademark applications (word + logo) per `docs/TRADEMARK.md`; USPTO first, Madrid Protocol extensions when international sales begin. | S-24-024 policing has something to police; M9 transfer asset | $$–$$$ | TODO |
| VA-09 | Engage an accredited RF test lab for FCC Part 15.247 + CE/ETSI EN 300 220 (+ EN 300 328 for Wi-Fi) on Lite. Stories S-24-001/002/003 produce the test *plans*; the lab executes them. | M2 exit criterion "FCC/CE passed for Lite" | $$$–$$$$ | TODO |
| VA-10 | Engage safety/battery labs: UL/IEC 62368-1 (S-24-007) and UN 38.3 lithium transport (S-24-035); confirm shipping-compliance paperwork with the fulfilment partner. | Legally shipping battery devices | $$$ | TODO |
| VA-11 | Select contract manufacturer; run DFM review against the Lite BOM; contract fulfilment/3PL for the first 100-unit run (M2 exit). Feed actuals into S-24-031 BOM/margin tracking. | M2 exit "first 100 units shipped" | $$$–$$$$ | TODO |
| VA-12 | Open Apple Developer Program and Google Play Console accounts; complete D-U-N-S/entity verification (needs VA-06); review store policies against S-24-030 findings. | S-19-017 TestFlight/Play betas; M2 exit "apps in public beta" | $ /yr | TODO |
| VA-13 | Open payment-processor account (e.g. Stripe) in live mode + sales-tax/VAT registration strategy (e.g. merchant-of-record vs direct). | Device sales; later S-21-022 billing, S-21-023 payouts | $ (rev-share) | TODO |
| VA-14 | Stand up CLA management (CLA-assistant bot or equivalent) wired to the CONTRIBUTING.md §5 CLA text; store signed CLAs durably (foundation transfer depends on them, S-24-023). | First external PR; M9 legal package | $ | TODO |
| VA-15 | Establish CVE Numbering Authority arrangement (partner CNA or MITRE direct) referenced by `SECURITY.md` §What-to-expect. | Advisory publication; S-24-013 CRA vuln SLA | $ | TODO |

### 2.3 Gates M3–M5 — Alpha, Home Gateway, Cloud GA

| VA | Action | Unblocks | Cost | Status |
|---|---|---|---|---|
| VA-16 | Open production cloud account(s) (provider per S-21-028 ADR outcome), plus KMS/HSM for release signing and plugin-registry signing (S-21-012, EPIC-23 HSM signing). Configure org-level security (SSO, MFA, audit logs). | EPIC-21 deploys; S-23 signing chain | $$–$$$ /yr | TODO |
| VA-17 | Join the Wi-Fi Alliance and schedule Wi-Fi CERTIFIED HaLow testing per HaLow SKU (S-24-037; dossier `08_HALOW_TECHNOLOGY_DOSSIER.md` D-HALOW-05). | Alpha/Omega HaLow marketing claims; S-22-026 bench overlaps | $$$–$$$$ | TODO |
| VA-18 | Contract Alpha PCB fabrication + assembly (rev-1) per M3 entry criterion. | M3 entry; EPIC-04 HIL | $$$ | TODO |
| VA-19 | Sign the first pilot fleet customer (design-partner terms) and execute the S-21-020 onboarding runbook with them. | M4 exit "Fleet Console early access with first pilot" | — (revenue) | TODO |
| VA-20 | Purchase the interop/HIL bench: HaLowLink 2 router, NRC7394 reference AP, programmable PSU + power analyzer (for S-04-024 current measurements), RF attenuators/shield box. | S-22-026 cross-vendor gate; S-04-023/024 measurements | $$–$$$ | TODO |

### 2.4 Gates M6–M9 — Omega, ecosystem, foundation

| VA | Action | Unblocks | Cost | Status |
|---|---|---|---|---|
| VA-21 | Negotiate cellular connectivity: LTE-M MVNO/carrier data agreements + eSIM/SIM logistics for Omega; SMS/PSTN gateway partner (e.g. Twilio) for S-21-027. | EPIC-05 cellular; M6 exit "billing controls verified" | $$–$$$ /yr | TODO |
| VA-22 | (Optional, per S-24-006) Iridium/NTN service-provider agreement for satellite SKU option. | Omega satellite option | $$$ | TODO |
| VA-23 | Fund and launch the public bug bounty (HackerOne or self-host per S-24-027/028) once Phase-1 governance is reached. | SECURITY.md §Bug-bounty promise | $$$ /yr | TODO |
| VA-24 | Retain foundation-formation counsel; execute IP escrow (S-24-022) and prepare the foundation transfer legal package (S-24-023) per RFC-0002 shortlist (S-24-021). | M9 entry | $$$ | TODO |
| VA-25 | Execute the M9 transfer: assign trademarks, move infrastructure accounts (GitHub org, domains, cloud, signing keys) to the Foundation; hold first board meeting. | M9 exit | $$ | TODO |

### 2.5 Continuous (never "done")

| VA | Action | Unblocks | Cost | Status |
|---|---|---|---|---|
| VA-26 | Regional compliance operations: WEEE registrations per market (S-24-009), recertification on hardware/radio changes (triggers per S-24-037 AC), export-control review on crypto changes (S-24-034). | Ongoing legal sale per region | $$ /yr/region | TODO |
| VA-27 | Community infrastructure operations: Matrix room, Discourse forum, mailing lists (`CONTRIBUTING.md` §12), docs/support hosting (S-24-017), moderation roster staffing (CODE_OF_CONDUCT.md §Enforcement-roster). | Community formation; Phase-1 governance criteria | $–$$ /yr | TODO |
| VA-28 | Business operations cadence: accounting/tax filings, insurance renewals, BOM/margin review (S-24-031), quarterly governance review (`06_GOVERNANCE.md`). | Staying solvent and governed | $$ /yr | TODO |

---

## 3. Coverage check — every venture dimension mapped

| Dimension | Class 1 (stories) | Class 2 (VA) |
|---|---|---|
| Device firmware (3 SKUs, all radios/sensors) | EPIC-02..18 | VA-01, 03, 18, 20 |
| Android / iOS / desktop / web / watch / vehicle apps | EPIC-19 (30 stories incl. app-as-node, Auto/CarPlay, Connect IQ, OpenWrt) | VA-12 |
| SDKs (5 languages) + third-party ecosystem | EPIC-20; S-24-025 certified-partner program | VA-14 |
| Cloud (Fleet, Relay, Provisioning, Registry) + self-host | EPIC-21 | VA-16 |
| Revenue: device sales, fleet SaaS, marketplace, SMS bridge, relay federation | S-21-022/023/024/027; S-24-020/031; `05_INFRASTRUCTURE_AND_SCALE.md` | VA-06, 11, 13, 19, 21 |
| Quality: CI, simulation, fuzzing, soak, supply chain | EPIC-22/23 | VA-01, 02, 16, 20 |
| Compliance: radio, safety, environmental, privacy, export, battery, kids | EPIC-24 (37 stories) | VA-09, 10, 15, 17, 26 |
| Legal/business formation, insurance, trademark | S-24-015/016/024; `docs/TRADEMARK.md` | VA-06, 07, 08, 28 |
| Community, governance, foundation | EPIC-01; S-24-021/022/023/026/028 | VA-02, 05, 14, 23, 24, 25, 27 |

No dimension exists outside these two registers. If one is discovered, it gets a story (Class 1) or a VA number (Class 2) — never a third place.

---

## 4. The immediate queue (state as of this document's creation)

1. ~~`git init` + initial commit of the certified portfolio + scaffold~~ — done alongside this document.
2. **S-01-004 / S-01-005 / S-01-007** — executed to the limit of Class 1 (S-01-005 remains gated on VA-04 keys).
3. **S-02-001 / S-02-002** — DONE: devcontainer + digest-pinned Dockerfile + CI workflow + board-selecting build wrapper; containerized build verified green (VA-01 DONE).
4. **VA-02** (GitHub org + branch protection) and **VA-03** (order two Lite boards) — the two highest-leverage human actions; both are prerequisites for nothing-blocked M1 execution. *Update 2026-07-05: both DONE — VA-03 via D-0013 (hardware in hand), VA-02 via D-0011/D-0014 (repo live, `main` pushed, branch protection + required checks applied).*
5. Then: promote EPIC-02 P0 DRAFT stories (RTOS baseline, logging, storage, power) to READY and execute in DAG order toward the M1 demo.

---

## 5. Cross-references

- Buildable-work order: `04_BLUEPRINT.md` (milestones, DAG, RACI, risks).
- Definition of Done: `00_METHODOLOGY.md` §5 (12 gates).
- Buildability certification: `07_FINAL_READINESS_TRIAGE.md` (T-ledger; this doc closes T-21).
- Revenue architecture: `05_INFRASTRUCTURE_AND_SCALE.md`; business stories: `epics/EPIC-24-*/STORIES.md`.
- Governance phases (bounty, foundation triggers): `../../06_GOVERNANCE.md`.

---

*This document is CC-BY-4.0. It contains ordering, gating milestones, and cost bands — never calendar dates or duration estimates.*
