# 06 — SS-SP GOVERNANCE, RFC PROCESS, & FOUNDATION MODEL

> **Scope.** How SS-SP is governed as an open, commercially-sold, standards-track project: decision-making, contribution flow, RFC/versioning process, trademark stewardship, security-response governance, foundation transition path, and commercial-alignment rules.
>
> **Goal.** Give contributors, customers, and partners **maximal confidence** that the SS-SP project is:
>
> - Governed transparently and predictably.
> - Not a rug-pull risk (relicense, kill-switch, "leave the community edition to die" scenarios).
> - Standards-worthy: capable of hosting a wire protocol that other vendors will ship on their hardware without fear.
> - Commercially healthy: a vendor entity can invest in it and sell hardware without hidden legal traps.

---

## 0. TL;DR

| Question | Decision |
|---|---|
| Who runs SS-SP today? | The **founding vendor entity** ("SS-SP Inc." working title). |
| How is technical direction decided? | **Meritocratic Steering Committee (SC)** + public RFC process, modeled after IETF + Python-SIG + LLVM. |
| Where does the code live? | GitHub org `ss-sp/` with mirrors on Codeberg and self-hosted Forgejo. |
| Who owns the trademark? | Founding vendor entity for v1; **transferred to a neutral foundation by v2.0**. |
| How does the project reach neutral governance? | Phased plan: single-vendor → Advisory Board → Independent Foundation (Linux Foundation Project or Apache Incubator target). |
| How are breaking changes made? | Formal RFC with 30-day comment window, SC vote, deprecation window ≥12 months. |
| How are security fixes coordinated? | Private Security Response Team (SRT) with embargo protocol; coordinated disclosure. |
| How is the community protected from the vendor? | Signed public license grants, foundation transition commitment, Contributor Covenant CoC, and reproducible builds so anyone can verify the shipped artifacts. |
| How is the vendor protected from bad-faith community forks? | Trademark policy, CLA, signed release keys, brand-guard tooling. |

---

## 1. Governance Phases (roadmap of the project itself)

Governance evolves with the project. Each phase has clear entry criteria and deliverables.

### Phase 0 — Vendor-Founded (now → v0.9)
- Single vendor entity owns copyrights, trademarks, keys.
- Public source repositories from day one, Apache-2.0 licensed (see `04_LICENSING_AND_FORK_STRATEGY.md`).
- Contribution model: DCO + CLA to vendor entity.
- Steering Committee: internal, 3 seats (Founder, VP Engineering, Security Officer).
- **Entry criterion**: none — this is where we start.
- **Deliverables**: v1.0 firmware, protocol spec, companion apps, licensing & security docs.

### Phase 1 — Advisory Board (v1.0 → v1.5)
- SC expanded to **5 seats**: 3 vendor, 2 elected community members (elected by contributors with ≥5 merged PRs).
- Advisory Board of **7 seats** advises SC: interoperability partners (Meshtastic-community rep invited if willing), silicon vendors, at least one first-responder org, at least one enterprise customer, and academia.
- Public quarterly community calls, minutes published.
- **Entry criterion**: 25 external contributors, 3 shipping product SKUs.
- **Deliverables**: RFC process operational, first external RFCs accepted, first interoperability event.

### Phase 2 — Independent Foundation (v2.0)
- Neutral non-profit foundation formed (or joined). Target hosts:
  - **Linux Foundation Project** (light-touch), or
  - **Apache Software Foundation** (incubator → TLP), or
  - **Zephyr / Eclipse Foundation** if embedded fit is stronger, or
  - **NLnet / Sovrin-style community foundation** for the mesh-freedom identity.
- **Trademark transferred to foundation.**
- **Vendor entity** retains commercial products, becomes a "Strategic Member."
- SC becomes formal foundation Technical Steering Committee.
- **Entry criterion**: 3 independent vendor implementations of the protocol shipping, 100+ external contributors, mature RFC process.
- **Deliverables**: foundation charter, IP transfer, first foundation-hosted release.

### Phase 3 — Standards-Track (v2.5+)
- Protocol spec submitted to IETF (individual submission → possibly WG) and/or IEEE for standardization of relevant layers.
- Independent test suite maintained by foundation, used for "SS-SP Certified" compliance mark (foundation-owned trademark).
- Multiple certification labs.
- **Entry criterion**: 5 independent implementations, deployed at scale.
- **Deliverables**: RFC / IEEE draft, certification program.

Each transition is a **commitment**, not aspirational marketing. Delaying is allowed with SC + community vote; skipping is not.

---

## 2. Steering Committee (SC)

### 2.1 Composition (Phase 1 onward)
- **5 members** (odd number to avoid ties).
- Terms staggered 2-year, initially bootstrapped with 1-year and 2-year initial terms.
- Chair elected internally, 1-year term.
- Quorum: 3.

### 2.2 Seats
| Seat | Selection | Term |
|---|---|---|
| Founder / Vendor CTO | Appointed by vendor | Permanent while vendor exists |
| Security Officer | Appointed by vendor, confirmed by SC | 2 yr |
| Community Elected #1 | Elected by contributors | 2 yr |
| Community Elected #2 | Elected by contributors | 2 yr |
| Rotating Interop Seat | Rotates yearly among ecosystem partners | 1 yr |

### 2.3 Responsibilities
- Accept / reject RFCs.
- Approve release timing.
- Adjudicate CoC escalations that reach the SC.
- Approve foundation-transition milestones.
- Charter and disband working groups.
- Approve trademark uses.

### 2.4 Decision-making
- Prefer **lazy consensus** (silence = assent within 72 hours on ≤moderate items).
- **Rough consensus** on RFCs (no strong sustained objection).
- **Formal vote** when consensus not reachable. Simple majority; Chair breaks ties.
- **Supermajority (4/5)** required for: license changes, trademark changes, foundation transition, revoking a maintainer.

### 2.5 Transparency
- All SC decisions logged in `governance/decisions.md`, signed.
- Minutes of every meeting published within 7 days.
- Voting records public unless the vote concerns a personnel/CoC matter.

---

## 3. Working Groups (WGs)

Chartered by SC. Each WG owns a technical domain, has 1 Chair, 2+ maintainers, and public mailing list / channel.

Initial WGs:

| WG | Domain |
|---|---|
| `wg-firmware` | ESP-IDF firmware core, HAL, board configs |
| `wg-protocol` | Wire format, LXMF+SS-Ext, Meshtastic-compat, RNS integration |
| `wg-ui` | `ss_ui` layout engine, themes, accessibility |
| `wg-radio` | LoRa driver, HaLow driver, RF regulatory |
| `wg-security` | Cryptography, secure boot, OTA, disclosure — see also SRT |
| `wg-companion` | Mobile + desktop + web apps |
| `wg-cloud` | Fleet console, tenant infrastructure |
| `wg-ai` | On-device STT/TTS/LLM, model catalog & provenance |
| `wg-docs` | User docs, developer docs, translations |
| `wg-compliance` | Regulatory (RED, FCC, CRA, GDPR, export) |

WGs may spin down or split by SC vote.

---

## 4. RFC Process

Modeled on IETF-Draft + Rust-RFC + PEP.

### 4.1 When an RFC is required
- Any change to wire format (protocol layer).
- Any change to public HAL API.
- Any new capability flag / cryptographic primitive.
- Any change to release cadence, LTS commitments, or support-window promises.
- Any charter-level change (governance, licensing, trademark).

Not required for: bug fixes, doc typos, board-specific tweaks that don't affect the shared HAL, community-edition feature additions that follow existing patterns.

### 4.2 Lifecycle
```
IDEA → DRAFT → REVIEW → FINAL CALL → ACCEPTED/REJECTED → IMPLEMENTED → OBSOLETED
```

- **DRAFT**: PR to `rfcs/` with the RFC template. Author + 1 shepherd.
- **REVIEW**: ≥30 days open discussion. Concerns tracked in a live "unresolved questions" section.
- **FINAL CALL**: 14 days; if no new substantive objections, moves to acceptance.
- **ACCEPTED**: SC records vote in `governance/decisions.md`. RFC gets a permanent number (RFC-0000+).
- **IMPLEMENTED**: PR closing the RFC, referencing merges that implement it.
- **OBSOLETED**: superseded by a later RFC; kept for history.

### 4.3 Template (excerpt)
```
Title:
Author(s):
Shepherd:
Status: DRAFT | REVIEW | FINAL CALL | ACCEPTED | REJECTED | IMPLEMENTED | OBSOLETED
Start Date:
Feature area (WG):
Requires: (list of RFCs)
Supersedes: (list of RFCs)

# Summary
# Motivation
# Detailed design
# Wire-format changes (if any) — MUST include hex byte-layout examples
# Security considerations — MUST be signed off by wg-security
# Interoperability impact — MUST include Meshtastic-compat impact
# Backward compatibility & migration
# Rejected alternatives
# Unresolved questions
# Prior art
# Implementation plan
```

### 4.4 Wire-format stability contract
- Wire format frozen at v1.0 with **12-month deprecation for any removed field**.
- Additive changes always allowed (unknown fields ignored per encoding rules).
- Breaking changes require an RFC accepted at least 12 months before removal.
- LTS releases receive backports of security fixes for **5 years**.

---

## 5. Contribution Model

### 5.1 Contributor pathway
1. **Sign the DCO** (per commit trailer).
2. **Sign the CLA** (once) — individual or corporate.
3. Open a PR against the appropriate WG.
4. CI must pass: build, tests, license scan, style, security lint, SPDX headers.
5. Review by ≥1 CODEOWNER (2 for security-sensitive paths).
6. Merge by CODEOWNER or maintainer.
7. Change referenced in changelog for next release.

### 5.2 CLA rationale
Chose Apache-style CLA (individual + corporate) because:
- We may transfer copyright to a foundation later; CLA makes this legally clean.
- Ensures contributor represents they have the right to contribute.
- Grants a patent license from the contributor to us & downstream users.

The CLA is **not** a copyright assignment (contributor retains their copyright). We only get a broad license.

### 5.3 CODEOWNERS
- Every path has a maintainer (`.github/CODEOWNERS`).
- Security-sensitive paths (`components/ss_hal/**`, `firmware/security/**`, `bootloader/**`, `protocol/**`, `ota/**`, `provisioning/**`) require **2 maintainer approvals + wg-security sign-off**.
- Trademark, license, and governance docs require SC review.

### 5.4 Maintainer promotion
- Contributor becomes maintainer after: 20+ merged high-quality PRs OR nomination by 2 existing maintainers + no objection from SC.
- Maintainer can be demoted for CoC violation or persistent inactivity (12 months inactive → emeritus).

### 5.5 Coding standards
- Firmware C: MISRA-C:2012 where practical, `clang-format` enforced.
- Rust: `rustfmt` + `clippy -- -D warnings`.
- TypeScript / Dart: project-standard linters, strict mode on.
- Every public function documented.
- Tests required for new features.

### 5.6 Commit hygiene
- Conventional Commits (`feat:`, `fix:`, `docs:`, `refactor:`, `test:`, `chore:`, `security:`, `breaking:`).
- Small, focused commits preferred.
- Sign-off (DCO) mandatory.
- Merge strategy: **squash by default**, `merge` for RFCs and multi-commit features that warrant preserved history.

---

## 6. Release Management

### 6.1 Versioning
- **Firmware**: SemVer 2.0 (`MAJOR.MINOR.PATCH`).
- **Wire protocol**: independent `PROTO-vN` version (integer). Compatibility: `PROTO-vN` peers must interoperate with `PROTO-v(N-1)` for one full LTS window.
- **HAL API**: independent SemVer; ABI break requires major bump.
- **Companion apps**: independent SemVer; API-compat expressed via feature negotiation.

### 6.2 Cadence
- **Nightly** builds on `main` (unsigned, developer-only).
- **Beta** on `beta` branch (release-signed with `RelKey_A` only, community testing).
- **Release Candidate** promoted to `release` after canary bake; dual-signed (`RelKey_A` + `RelKey_B`).
- **General Availability**: minor releases quarterly; patch releases as needed.
- **LTS**: one release per year is marked LTS with 5-year security-fix commitment.

### 6.3 Deprecation & sunset
- Any deprecation announced ≥12 months before removal.
- Sunset list published in `docs/DEPRECATIONS.md` and cross-linked from release notes.

### 6.4 Reproducible builds
- CI produces bit-identical release artifacts from tagged sources.
- Public build verifier at `build-verify.ss-sp.org`; anyone can rebuild and diff.
- Build environment fully described in `.buildenv/` (Dockerfiles, toolchain hashes).

### 6.5 SBOM & signing
- SPDX 2.3 + CycloneDX 1.5 published per artifact.
- Sigstore/cosign attestations published.
- HW SBOM (BOM of assembled unit) published for each hardware SKU.

---

## 7. Security Response Team (SRT)

### 7.1 Composition
- 5 members: Security Officer (chair), a `wg-firmware` maintainer, a `wg-protocol` maintainer, a companion-app maintainer, and one rotating external security researcher (yearly, invited).
- All members sign an NDA with each other for embargoed handling.

### 7.2 Process (see also `05_SECURITY_MODEL.md` §13)
1. Report received at `security@ss-sp.org` (PGP encrypted).
2. Ack within 24 h.
3. Triage within 72 h → severity via CVSS.
4. Fix developed on private embargo repo/branch.
5. Coordinated disclosure with reporter, upstream partners (Espressif, Morse Micro if hw-adjacent), interoperability partners (Meshtastic if wire-format-relevant).
6. Advisory + patch released, CVE assigned.
7. Post-mortem within 30 days for P1/P2.

### 7.3 Embargo windows
- Default 90 days.
- Extension possible for hardware fixes.
- Reporter's disclosure preference respected within reason.

### 7.4 Bug bounty
- Public program, per-tier rewards.
- Safe harbor language in the program terms.
- Ineligible: physical, social engineering, DoS, out-of-scope third-party components.

---

## 8. Code of Conduct

- Adopt **Contributor Covenant 2.1**, adapted lightly for our context (adds "no political demonstrations in official channels" for the neutrality of standards work; adds mesh-safety norms).
- Enforcement handled by a **Community Council** (3 members, non-overlapping with SC where possible).
- Escalation path: reporter → CoC Council → SC (only if Council is impaired).
- Anonymous reporting supported via a third-party form.

---

## 9. Trademarks & Branding

### 9.1 Marks
- **Word marks**: "SS-SP", "Seekie-Speakie", "Seekie", "Speakie", "SS-SP-Certified".
- **Design marks**: shield/dot logo, radar-cursor logo.
- Registered in: US, EU, UK, Canada, Japan, Australia. (Filings tracked in `governance/trademarks.md`.)

### 9.2 Community usage policy
- **Allowed** unmodified use in factual references, documentation, "works with SS-SP" badges (subject to compliance).
- **Not allowed** in the name of a forked project, in ways that imply endorsement, or on hardware not certified by the foundation (Phase 2+).
- Community-edition builds may keep the marks *if unmodified*; modified builds must remove or replace them.

### 9.3 Certification (Phase 2+)
- Foundation defines a **certification** test suite and grants the "SS-SP Certified" mark under a written agreement.
- Vendors can ship certified devices; the foundation collects a modest certification fee that funds test-suite maintenance.

### 9.4 Meshtastic (third-party mark)
- We respect Meshtastic LLC's trademark; only nominative fair use is permitted in our materials (see `04_LICENSING_AND_FORK_STRATEGY.md` §3.3).

---

## 10. Commercial Alignment Rules

The vendor entity operates alongside the open project. Rules exist to avoid conflicts of interest.

### 10.1 The vendor MAY
- Sell hardware devices with the open firmware pre-installed.
- Sell an **enterprise fleet console** (BSL 1.1 or proprietary) as an add-on.
- Sell certified accessories, plugins (signed), and premium support.
- Employ maintainers.
- Contribute code and pay for infrastructure.

### 10.2 The vendor MUST NOT
- Withhold security fixes from the community edition.
- Make the community edition depend on a proprietary cloud service.
- Ship community-branded devices that require paid unlocks for basic messaging, voice, presence, or SOS.
- Use its majority on the SC (in Phase 0/1) to reject an RFC purely for commercial reasons; a rejection must have a technical or licensing rationale entered into the record.
- Rug-pull: change the open license or trademark policy without SC supermajority and a public 12-month notice.

### 10.3 Anti-rug-pull commitments (public, signed)
- A signed **"Open Assurance Statement"** by the vendor, notarized at Phase 0, promising:
  - The current-and-future community-edition firmware and companion apps remain Apache-2.0.
  - Trademark license granted to the eventual foundation on Phase 2 with no reservation.
  - Vendor will provide the foundation with an initial endowment sufficient to run infrastructure for ≥3 years.
  - Vendor will not relicense retroactively.
- Statement published in `governance/OPEN_ASSURANCE.md` and mirrored in blockchain-anchored proof (Sigstore transparency log entry).

### 10.4 Bankruptcy / vendor-loss plan
- The **Bootstrap-Keys Escrow**: `RelKey_C` (see `05_SECURITY_MODEL.md` §13.4) sits in an escrow held by an independent trustee. If the vendor entity dissolves without a successor, the escrow releases the key to the foundation (or if no foundation exists, to a nominated open-source steward organisation).
- Trademarks escrowed similarly.
- Devices continue operating on the last-published firmware indefinitely.

---

## 11. Financial Model (sketch, not final)

Not part of the governance document proper, but relevant to how the project sustains itself:

- **Revenue** (vendor): device sales, enterprise console subscriptions, certification fees (post-foundation), support contracts.
- **Foundation revenue** (Phase 2+): certification fees, membership dues (silicon vendors, OEMs, cloud partners), grants (e.g. NLnet, Sovereign Tech Fund, Radio Free Asia's OTF).
- **Community expense** (foundation): CI, mailing lists, chat infra, RFC-editor stipend, security-bounty pool, conference sponsorship.
- Zero-cost path for individual contributors: no fees to submit code, no fees to run community-edition firmware.

---

## 12. Reference Architecture Docs Cross-References

| Topic | Source of truth |
|---|---|
| Product overview | `00_MASTER_SOFTWARE_PLAN.md` |
| Lite hardware | `01_SS-SP_LITE_HARDWARE_REFERENCE.md` |
| Wire protocol | `02_PROTOCOL_STACK.md` |
| UI/UX architecture | `03_UI_LAYOUT_SPEC.md` |
| Licensing & fork decisions | `04_LICENSING_AND_FORK_STRATEGY.md` |
| Security model | `05_SECURITY_MODEL.md` |
| Governance (this doc) | `06_GOVERNANCE.md` |
| Contribution details | `CONTRIBUTING.md` (repo root) |
| CoC | `CODE_OF_CONDUCT.md` |
| Trademark policy | `docs/TRADEMARK.md` |
| Security disclosure | `SECURITY.md` |
| Public open-assurance commitment | `governance/OPEN_ASSURANCE.md` |
| SC decisions | `governance/decisions.md` |
| RFCs | `rfcs/` |

---

## 13. Adoption

This document, together with `04_LICENSING_AND_FORK_STRATEGY.md` and `05_SECURITY_MODEL.md`, is the **founding governance charter** of the SS-SP program. Adoption is signaled by:

1. Merge of this document to `main` under a signed commit from the SC bootstrapping members.
2. First public release marked `v0.9.0-governance-baseline`.
3. First community RFC opened by an external contributor to exercise the process.

Amendments follow the RFC process, with SC supermajority for structural changes.

---

**End of governance baseline.**
