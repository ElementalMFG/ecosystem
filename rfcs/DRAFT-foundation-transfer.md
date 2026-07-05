<!-- SPDX-License-Identifier: CC-BY-4.0 -->
- Title: Foundation transfer plan — Phase-2 neutral stewardship of the SS-SP protocol and marks
- Author(s): SS-SP program lead
- Shepherd: wg-community (bootstrap: program lead)
- Status: DRAFT
- Start Date: 2026-07-04
- Feature area (WG): wg-community / governance, legal
- Requires: 0001
- Supersedes: —

# Summary

This RFC pre-commits the project to transferring stewardship of the SS-SP protocol specifications, reference implementations, and certification trademarks to a neutral foundation once the Phase-2 entry criteria in `../06_GOVERNANCE.md` are met (3 independent vendor implementations, 100+ external contributors, mature RFC process). It shortlists three candidate foundations, defines the ordered sequence of transfer states, and drafts the IP-transfer clause. It intentionally does **not** pick the foundation today — that choice is made at FINAL CALL with community input. This draft satisfies story S-01-014.

# Motivation

The anti-rug-pull posture (`../governance/OPEN_ASSURANCE.md`) is only credible if vendor-neutrality is pre-committed while the founding vendor still holds all the keys. Ecosystem partners deciding whether to build SS-SP-compatible hardware need to know, before they invest, that the protocol cannot be captured. Publishing the transfer plan as a draft RFC — reviewable, versioned, and citable — converts a marketing promise into a governance artifact.

# Detailed design

## Candidate shortlist (3)

| # | Candidate | Model | Why shortlisted | Principal concerns |
|---|-----------|-------|-----------------|--------------------|
| 1 | **Linux Foundation** (directed fund / LF Projects LLC) | Light-touch fiscal + legal host; project keeps its own technical governance | Fastest path; keeps our RFC process and WG structure intact; strong trademark-stewardship practice | Cost of membership tiers; less prescriptive governance means we must keep our own house in order |
| 2 | **Eclipse Foundation** | Full development process + IP due-diligence regime; strong European base | CRA alignment and EU regulatory credibility matter for a radio product; Eclipse IP review de-risks the codebase for adopters; hosts comparable embedded/IoT projects (e.g. Oniro, ThreadX) | Eclipse Development Process would partially supersede our RFC lifecycle; migration of CLA/DCO regime required |
| 3 | **Apache Software Foundation** (Incubator → TLP) | Community-over-code meritocracy; The Apache Way | Deep experience turning single-vendor projects into neutral ones; strong brand-management policy | Incubation demands releases + PMC formation on ASF's terms; Apache-2.0-only stance conflicts with our dual CC-BY-4.0 spec licensing (needs legal analysis); trademark transfer to ASF is irrevocable |

A fourth model — a purpose-built independent foundation (NLnet/Sovrin-style) — was considered and is retained in `../06_GOVERNANCE.md` as a fallback, but is excluded from the shortlist because operating a bespoke legal entity is the highest-overhead option for a small ecosystem.

## Ordered timeline states (not dates)

Transfer proceeds through ordered states; each state is entered only when its predecessor's exit criteria are met, with no calendar commitments:

1. **T0 — Pre-commitment (this RFC merged as DRAFT).** Plan is public; founding vendor retains stewardship.
2. **T1 — Trigger met.** Phase-2 entry criteria verified by the quarterly constitutional review (S-01-016) and recorded in the decisions log.
3. **T2 — Selection.** This RFC moves to REVIEW/FINAL CALL with a named foundation; community comment period; selection recorded as a decision.
4. **T3 — Legal preparation.** IP inventory (specs, marks, reference code, keys), CLA/DCO compatibility analysis, escrow of signing keys per covenant terms.
5. **T4 — Assignment executed.** IP-transfer clause (below) executed; trademarks and spec copyrights assigned; repo organization transferred or mirrored under foundation control.
6. **T5 — Governance cut-over.** WG charters re-ratified under foundation process; founding vendor becomes one member among equals; veto rights (if any) extinguish.
7. **T6 — Steady state.** Certification program (`../docs/TRADEMARK.md`) administered by the foundation; founding vendor competes under the same rules as every other vendor.

## IP-transfer clause (draft)

> **Assignment.** Upon the Effective Date, [Founding Vendor] irrevocably assigns to [Foundation] all right, title, and interest in and to: (a) the SS-SP protocol specifications and conformance vectors, including all copyrights therein; (b) the word marks "SS-SP" and "SS-SP-Certified" and associated logos, together with all goodwill; and (c) the reference-implementation copyrights held by [Founding Vendor], subject to all existing outbound licenses (Apache-2.0, CC-BY-4.0, CERN-OHL-S) which survive assignment unmodified.
> **License-back.** [Foundation] grants [Founding Vendor] the same non-exclusive rights available to any ecosystem member; no exclusive rights survive.
> **Irrevocability & anti-regression.** The assignment is irrevocable. [Foundation] covenants that published specifications shall remain available under licenses no more restrictive than those in force at the Effective Date (see anti-rug-pull covenant tests, S-01-015).
> **Signing keys.** Release-signing and spec-signing keys are rotated to foundation custody at T4; prior keys are revoked and the revocation published.

# Wire-format changes

None.

# Security considerations

Key custody is the main risk surface: T4 mandates rotation (not copying) of signing keys, so vendor-held keys cannot sign post-transfer artifacts. Trademark transfer removes the single-vendor revocation threat described in `../05_SECURITY_MODEL.md`'s ecosystem-trust discussion. The escrow arrangement (covenant terms in `../governance/OPEN_ASSURANCE.md`) covers the gap between T0 and T4.

# Interoperability impact

Positive: certification and compatibility testing become vendor-neutral, which is a precondition for competitors adopting SS-Link and the Meshtastic-compat profile in good faith.

# Backward compatibility & migration

All outbound licenses survive assignment unmodified — nothing published ever changes license retroactively. Existing certified devices retain certification; the mark's ownership changes, not its rules.

# Rejected alternatives

- **Bespoke standalone foundation** — highest overhead, weakest institutional memory; retained only as fallback.
- **No transfer, perpetual benevolent-vendor model** — rejected: incompatible with the open-assurance covenants and with partner-investment credibility.
- **Immediate transfer at Phase 1** — rejected: a foundation cannot steward an ecosystem of one; the entry criteria exist so the transfer lands when neutrality has beneficiaries.

# Unresolved questions

1. Which of the three shortlisted foundations — decided at T2 FINAL CALL, not before.
2. ASF licensing-compatibility question (CC-BY-4.0 specs under ASF policy) — needs legal review if ASF advances.
3. Whether hardware-design files (CERN-OHL-S) transfer to the same foundation or a hardware-focused steward.
4. Funding commitment accompanying the transfer (staff, CI infrastructure, certification-lab costs).

# Prior art

Kubernetes → CNCF (vendor → foundation with trademark transfer); Matrix.org Foundation (protocol + marks held neutrally, vendor competes as Element); Zigbee/CSA and Bluetooth SIG (consortium-held marks + certification); OpenTofu/Linux Foundation (fork stewarded neutrally after license rug-pull — the counter-example motivating this plan).

# Implementation plan

- T0: merge this DRAFT (this PR) — closes S-01-014's drafting AC.
- T1–T2: quarterly review process (S-01-016) owns trigger verification; selection PR moves this file through REVIEW → FINAL CALL → ACCEPTED, at which point it is renumbered per `README.md` convention.
- T3–T6: tracked as new stories under EPIC-01 when T1 fires; legal execution is outside the repo but its artifacts (assignment notice, key-rotation announcement) are committed here.
