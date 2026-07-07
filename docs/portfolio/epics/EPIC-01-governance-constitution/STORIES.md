<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# EPIC-01 — Stories

Format per `../../00_METHODOLOGY.md` §2.7. Meta lines are machine-parsed.

---

### S-01-001 — Publish constitution set (docs 00–08)
As the program lead I want all founding docs (00_MASTER_SOFTWARE_PLAN, 01_SS-SP_LITE_HARDWARE_REFERENCE, 02_PROTOCOL_STACK, 03_UI_LAYOUT_SPEC, 04_LICENSING_AND_FORK_STRATEGY, 05_SECURITY_MODEL, 06_GOVERNANCE, 07_BUSINESS_MODEL_AND_OPEN_SOURCE, 08_UNIVERSAL_CONNECTIVITY) written and cross-linked so that newcomers can onboard.
- AC: all files present; TOC in each; links resolve; `make lint-docs` passes
- Meta: Shard=A | Type=Task | Size=L | Prio=P0 | Status=DONE | SKU=★ | PRD=— | Const=C-00,C-01,C-02,C-03,C-04,C-05,C-06,C-07,C-08
- Tasks: spec (outline) · design (cross-link map) · impl (write) · test (`markdownlint`, link-check) · docs (self).
- Deps: —

### S-01-002 — Publish OPEN_ASSURANCE covenants
As a downstream integrator I want the vendor's open-assurance covenants published so that I can trust the roadmap.
- AC: covenants #1..#12 present; cryptographic-identity keys named; escrow terms stated
- Meta: Shard=A | Type=Task | Size=S | Prio=P0 | Status=DONE | SKU=★ | PRD=— | Const=C-OA
- Tasks: spec (covenant list #1..#12) · design (placement in docs 04/07) · impl (write covenants + key names + escrow terms) · test (`make lint-docs`) · docs (self).
- Deps: S-01-001

### S-01-003 — Publish trademark policy
As a certified-device manufacturer I want clear rules for using `SS-SP-Certified` so that I can brand my product without legal risk.
- AC: usage rules published; revocation clause present; appeal path documented
- Meta: Shard=H | Type=Task | Size=S | Prio=P0 | Status=DONE | SKU=★ | PRD=F-CERT-04 | Const=C-TM
- Tasks: spec (usage rules) · design (revocation clause + appeal path) · impl (write policy in doc 04) · test (`make lint-docs`) · docs (self).
- Deps: S-01-001

### S-01-004 — Publish CODE_OF_CONDUCT + enforcement plan
As a community member I want a ratified code of conduct with an enforcement plan so that incidents are handled predictably and fairly.
- AC: Contributor Covenant v2.1 baseline adopted; escalation ladder documented; enforcement roster named
- Meta: Shard=F | Type=Task | Size=S | Prio=P0 | Status=DONE | SKU=★ | PRD=— | Const=C-CoC
- Tasks: spec (Contributor Covenant v2.1 baseline) · design (escalation ladder) · impl (`CODE_OF_CONDUCT.md` + enforcement roster) · test (`make lint-docs`) · docs (self).
- Deps: —

### S-01-005 — Publish SECURITY.md + PGP/age keys
As a security researcher I want SECURITY.md with encrypted intake keys published so that I can report vulnerabilities safely.
- AC: intake email and PGP/age key fingerprints published; triage SLA (72 h ack) stated; hall-of-fame section present
- Meta: Shard=G | Type=Task | Size=S | Prio=P0 | Status=DONE | SKU=★ | PRD=NF-REG-03 | Const=C-SEC
- Tasks: spec (intake channels + 72 h SLA) · design (PGP/age key generation + fingerprint publication plan) · impl (`SECURITY.md` + publish keys) · test (`make lint-docs`; fingerprint round-trip check) · docs (self).
- Deps: S-01-001

### S-01-006 — Publish RFC template & lifecycle
As a contributor I want an RFC template and documented lifecycle so that protocol- and security-relevant changes follow one predictable path.
- AC: `rfcs/TEMPLATE.md` merged; README explains states DRAFT → REVIEW → FINAL CALL → ACCEPTED → IMPLEMENTED; first two RFCs merged as worked examples
- Meta: Shard=B | Type=Task | Size=S | Prio=P0 | Status=DONE | SKU=★ | PRD=— | Const=C-06
- Tasks: spec (lifecycle states) · design (template sections) · impl (`rfcs/TEMPLATE.md` + `rfcs/README.md` + RFC-0001/RFC-0002 as worked examples) · test (`make lint-docs`) · docs (self).
- Deps: —

### S-01-007 — CONTRIBUTING.md with DCO gate
As a maintainer I want CONTRIBUTING.md with an enforced DCO gate so that every contribution carries a provenance sign-off.
- AC: DCO trailer required on every commit; PR bot blocks unsigned commits; docs explain the rebase/sign-off fix-up flow
- Meta: Shard=C | Type=Task | Size=M | Prio=P0 | Status=DONE | SKU=★ | PRD=— | Const=C-06
- Tasks: spec (DCO requirement + fix-up flow) · design (CI check strategy) · impl (`CONTRIBUTING.md` + `.github/workflows/dco.yml`) · test (unsigned commit rejected by the DCO check) · docs (`CONTRIBUTING.md` §2).
- Deps: S-01-006

### S-01-008 — CLA gate for larger contributions
As the legal WG I want a CLA gate for larger contributions so that relicensing rights under the fork strategy are secured.
- AC: individual and corporate CLA PDFs published; GitHub CLA-assistant bot wired to block merges without signature; CLA-vs-DCO threshold documented
- Meta: Shard=C | Type=Task | Size=M | Prio=P0 | Status=IN_PROGRESS | SKU=★ | PRD=— | Const=C-04,C-06
- Tasks: spec (CLA-vs-DCO roles + no-threshold rule, D-0016) · design (grantee Elemental MFG + successor-assignment clause) · impl (`docs/CLA.md` + `.github/workflows/cla.yml`, signatures on `cla-signatures` branch) · test (live run on PR #2: bot blocked unsigned, signature recorded, `cla` promoted to required check per D-0016) · docs (CONTRIBUTING §5 link).
- Deps: S-01-007, D-0012 (grantee), D-0016; remaining: standalone corporate CLA doc (Phase 1)

### S-01-009 — Working-group charter files (12)
As a working-group chair I want all 12 WG charter files ratified so that ownership and escalation paths are unambiguous.
- AC: 12 charters merged; chairs named; meeting cadence declared; escalation path stated in each charter
- Meta: Shard=D | Type=Task | Size=M | Prio=P0 | Status=DONE | SKU=★ | PRD=— | Const=C-06
- Tasks: spec (12-WG set incl. wg-community/wg-legal, D-0017) · design (charter template: domain/chair/maintainers/channel/cadence/escalation/disband) · impl (`governance/wg/wg-*.md` ×12 + doc 06 §3 table rows + D-0017) · test (`make lint-docs`) · docs (doc 06 §3 links `governance/wg/`).
- Deps: S-01-001, D-0012 (channels), D-0015 (org teams deferred), D-0017

### S-01-010 — Decisions log bootstrap D-0001..D-0010
As an architect I want the decisions log bootstrapped with D-0001..D-0010 so that founding technical choices are immutable and citable.
- AC: 10 seed decisions covering license choices, RTOS, Reticulum adoption, PQ posture, methodology; D-number counter and schema documented; quorum rules stated
- Meta: Shard=E | Type=Task | Size=S | Prio=P0 | Status=DONE | SKU=★ | PRD=— | Const=C-06
- Tasks: spec (schema + quorum rules) · design (D-number counter) · impl (`docs/decisions.md` D-0001..D-0010) · test (`make lint-docs`) · docs (self).
- Deps: S-01-006

### S-01-011 — PR templates (feature, RFC, security, docs)
As a contributor I want PR templates for feature, RFC, security, and docs changes so that reviews start from complete information.
- AC: 4 templates in `.github/`; referenced from CONTRIBUTING.md; each template lists its required sign-offs
- Meta: Shard=C | Type=Task | Size=S | Prio=P1 | Status=DONE | SKU=★ | PRD=— | Const=C-06
- Tasks: spec sign-off matrix from 06_GOVERNANCE.md §4–§5 + CONTRIBUTING.md §9 · design GitHub `PULL_REQUEST_TEMPLATE/` dir + `?template=` selection, default template links the four · impl feature/rfc/security/docs templates · test lint-docs + index --check green · docs CONTRIBUTING.md §3 reference + changelog
- Deps: —

### S-01-012 — Two-approver merge policy enforced
As a release manager I want the two-approver merge policy enforced on `main` so that every change gets one code and one domain review.
- AC: branch protection on `main` active; code-owner rule maps paths to WGs; merges require one code + one domain reviewer
- Meta: Shard=C | Type=Ops | Size=S | Prio=P0 | Status=BLOCKED | SKU=★ | PRD=— | Const=C-06
- Tasks: spec (two-approver rule, CONTRIBUTING §9) · design (branch-protection config, D-0014) · impl (protection + required checks live; CODEOWNERS maps paths — to the maintainer, not WG teams yet) · test (protection API state verified) · docs (D-0014, D-0015).
- Deps: S-01-009; blocked on a second maintainer (required reviews deferred per D-0014) and org conversion for WG teams (D-0015)

### S-01-013 — Governance dashboard
As a community member I want a governance dashboard so that open RFCs, decisions, and WG rosters are visible without digging through the repo.
- AC: static site lists open RFCs, decisions log, and WG rosters; regenerated by CI on merge; linked from README
- Meta: Shard=B,D,E | Type=Feature | Size=M | Prio=P2 | Status=DRAFT | SKU=★ | PRD=— | Const=C-06

### S-01-014 — Foundation transfer plan RFC
As the program lead I want the foundation transfer plan captured as a draft RFC (`rfcs/DRAFT-foundation-transfer.md`; numbered at acceptance per the RFC filename convention) so that Phase-2 neutrality is credible and pre-committed.
- AC: shortlist of 3 candidate foundations; ordered timeline states (not dates); IP-transfer clause drafted
- Meta: Shard=I | Type=RFC | Size=L | Prio=P1 | Status=DONE | SKU=★ | PRD=— | Const=C-06,C-OA
- Tasks: spec (candidate-foundation shortlist) · design (ordered timeline states + IP-transfer clause) · impl (`rfcs/DRAFT-foundation-transfer.md`) · test (`make lint-docs`) · docs (self).
- Deps: S-01-006

### S-01-015 — Anti-rug-pull covenant test suite
As a downstream integrator I want an automated anti-rug-pull covenant test suite so that retroactive license changes are detected immediately.
- AC: automated check verifies no published spec has a retroactive license change; published artifacts hash-anchored; check runs in CI on every merge to `main`
- Meta: Shard=A | Type=Ops | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=— | Const=C-OA,C-04

### S-01-016 — Quarterly constitutional review process
As the community WG chair I want a quarterly constitutional review process so that constitutional drift is caught early.
- AC: recurring review event stub committed; review checklist merged; notes template merged
- Meta: Shard=A | Type=Task | Size=XS | Prio=P2 | Status=DRAFT | SKU=★ | PRD=— | Const=C-06

### S-01-017 — Protocol compatibility & deprecation policy (cross-layer)
As a protocol steward I want a single ratified compatibility-and-deprecation policy covering every versioned surface (SS-Link frames, RNS/LXMF profile, crypto suites, pairing schema, plugin ABI, node-profile schema, cloud APIs) so that "how long must old versions keep working" is a governed rule, not a per-team guess.
- AC: an ACCEPTED RFC defines, per surface, the minimum supported-version window (e.g. current plus previous major), the deprecation announcement channel and lead time expressed in release counts, and the sunset procedure; every version-negotiation story (S-06-016, S-12-014, S-10-008) and the SDK policy (EPIC-20) cite this policy rather than restating their own; CI includes a check that a wire-format PR references the policy section it complies with; the policy is linked from `02_PROTOCOL_STACK.md` §11
- Meta: Shard=— | Type=RFC | Size=S | Prio=P1 | Status=DRAFT | SKU=★ | PRD=— | Const=C-02
