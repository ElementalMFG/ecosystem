<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# EPIC-01 — Governance & Constitution

**Primary WG:** wg-community · **Contributing:** wg-legal, wg-security, wg-docs
**Priority:** P0 · **SKU:** ★ · **Milestone:** M0

## Outcome
The SS-SP program has a complete, published, and operational founding constitution: RFC process running, DCO+CLA on every PR, decisions log active, working-group charters ratified, code of conduct enforced, security disclosure channel open, trademark rules published, open-assurance commitments codified.

## Constitution clauses satisfied
C-04 `04_LICENSING_AND_FORK_STRATEGY.md` §contribution licensing; C-06 `06_GOVERNANCE.md` §RFC process, §working groups, §merge rules; C-07 `07_BUSINESS_MODEL_AND_OPEN_SOURCE.md` §open-source commitments; C-OA `governance/OPEN_ASSURANCE.md` §covenants, §escrow; C-TM `docs/TRADEMARK.md` §mark usage; C-CoC `CODE_OF_CONDUCT.md`; C-SEC `SECURITY.md` §disclosure.

## Dependencies
None (foundational).

## Shards
- **S-01.A Constitution documents** — all 00–08 markdown files + OPEN_ASSURANCE + TRADEMARK exist, are consistent, and link to each other.
- **S-01.B RFC process** — `rfcs/TEMPLATE.md`, RFC lifecycle (DRAFT → REVIEW → FINAL CALL → ACCEPTED → IMPLEMENTED), authorship rules, shepherd rotation.
- **S-01.C Contribution mechanics** — CONTRIBUTING.md, DCO trailer requirement, CLA gate, PR templates, review rules, two-approver policy.
- **S-01.D Working Group charters** — each of 12 WGs has a charter file, chair, member roster, meeting cadence, escalation path.
- **S-01.E Decisions log** — `governance/decisions.md` schema, D-number counter, quorum rules for architectural decisions.
- **S-01.F Code of Conduct** — ratified CoC, incident response plan, enforcement roster.
- **S-01.G Security disclosure** — SECURITY.md published, encrypted intake channel (age/PGP), triage SLA, hall-of-fame.
- **S-01.H Trademark policy** — TRADEMARK.md published, `SS-SP-Certified` mark rules, opt-in cert program stub.
- **S-01.I Foundation transition plan** — Phase-2 target foundation shortlist, IP-transfer clauses, escrow.

## Exit criteria
1. Every document in the constitution set exists and is peer-reviewed by ≥2 non-authors.
2. Two independent DCO-signed PRs have merged through the process end-to-end.
3. `governance/decisions.md` contains at least D-0001 through D-0010.
4. All 12 WG charters are merged.
5. `SECURITY.md` intake channel receives and answers a synthetic test report inside the SLA.
6. `docs/TRADEMARK.md` referenced from README.

## RACI
- **R:** wg-community chair · **A:** founding vendor CEO (Phase-0) · **C:** wg-legal, wg-security · **I:** all WGs.

## Risks
| # | Risk | Mitigation |
|---|------|-----------|
| R01-01 | Constitutional drift as codebase grows | Quarterly constitutional review, `decisions.md` immutability |
| R01-02 | RFC process becomes bottleneck | Small-change fast-path, shepherd rotation |
| R01-03 | Foundation transfer stalls in Phase 2 | Named foundation shortlist + IP escrow contract |
