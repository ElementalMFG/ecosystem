<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# EPIC-24 — Compliance, Business & Support

**Primary WG:** wg-legal, wg-community · **Contributing:** wg-ops, wg-hardware, wg-cloud
**Priority:** P0 · **SKU:** ★ · **Milestone:** M3 (continuous)

## Outcome
The non-code deliverables that make a real product real: radio certification (FCC/CE/ISED/ACMA/…), safety (UL/IEC), privacy (GDPR/CCPA/PIPEDA), Cyber Resilience Act (CRA) SBOM & vuln disclosure obligations, warranty/RMA process, support portal, marketing/sales enablement, community/enterprise SLAs, and the Phase-2 foundation transfer plan execution.

## Constitution
C-06 `06_GOVERNANCE.md` §RFC & foundation transition; C-TM `docs/TRADEMARK.md`; C-SEC `SECURITY.md` §vulnerability disclosure; C-07 `07_BUSINESS_MODEL_AND_OPEN_SOURCE.md` §support & certification.

## Dependencies
Every other epic (compliance surface follows features).

## Shards
- **S-24.A Radio cert plan** — FCC 15.247/15.407, ETSI EN 300 220 / 328 / 300 665 for HaLow, per SKU + per region.
- **S-24.B Safety cert** — UL 62368-1 / IEC 62368-1.
- **S-24.C Environmental cert** — RoHS 3, REACH, WEEE.
- **S-24.D Privacy compliance** — GDPR, CCPA, PIPEDA, LGPD data flow docs + DPAs.
- **S-24.E CRA readiness** — SBOM per artifact, vuln disclosure SLA, EoL policy.
- **S-24.F Warranty + RMA process.**
- **S-24.G Support portal (docs + tickets).**
- **S-24.H Marketing + sales enablement kit.**
- **S-24.I SLA definitions (community + enterprise tiers).**
- **S-24.J Foundation transfer execution** — RFC → contract → asset transfer.
- **S-24.K Trademark policing + certification program ops.**

## Exit criteria
1. Lite FCC 15.247 approval on file before M2 ship.
2. GDPR DPA published; DSR fulfillment tested.
3. CRA-compliant SBOM + vuln SLA operational.
4. Support portal live with SLAs published.
5. Foundation transfer contract signed by Phase-2 gate.

## Risks
| # | Risk | Mitigation |
|---|------|-----------|
| R24-01 | Cert delays | Start early, use accredited lab partner |
| R24-02 | GDPR fine | Data-min, minimal-cloud, encrypted backups |
| R24-03 | Trademark dilution | Active policing, cease-and-desist toolkit |
| R24-04 | Foundation transfer stalls | IP escrow contract as fallback |
