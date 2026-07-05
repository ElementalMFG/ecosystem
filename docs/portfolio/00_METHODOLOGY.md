<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# 00 — Portfolio Methodology

*Binding. Read before touching any other artifact in this directory.*

---

## 0. What this portfolio is

A structured, top-down decomposition of the SS-SP program into:

```
Constitution   ── founding docs (00–08, OPEN_ASSURANCE, TRADEMARK, CoC, SECURITY)
     │
Brief          ── docs/portfolio/01_BRIEF.md
     │
PRD            ── docs/portfolio/02_PRD.md
     │
Architecture   ── docs/portfolio/03_ARCHITECTURE.md
     │
Blueprint      ── docs/portfolio/04_BLUEPRINT.md   (rollout, RACI, delivery order)
     │
Epics          ── docs/portfolio/epics/EPIC-NN-*/EPIC.md   (24 epics)
     │
Sharded Epics  ── shards inside each EPIC.md (sub-outcomes)
     │
Stories        ── docs/portfolio/epics/EPIC-NN-*/STORIES.md   (all stories per epic)
     │
Traceability   ── docs/portfolio/STORIES_INDEX.md   (constitution → story lineage)
```

Every artifact traces upward: story → shard → epic → PRD requirement → constitution clause. If it doesn't trace, it doesn't ship.

---

## 1. Why this methodology (research summary)

After evaluating the 2026 spec-driven-development landscape (Kiro, GitHub Spec Kit, BMAD-METHOD v6.6, OpenSpec, Tessl, Superpowers, Agent OS) we adopted **BMAD-METHOD v6 as the backbone**, with two augmentations:

- **From GitHub Spec Kit:** the "constitution" concept. Our constitution already exists as `04–08_*.md` + `governance/OPEN_ASSURANCE.md` + `docs/TRADEMARK.md` + `CODE_OF_CONDUCT.md` + `SECURITY.md`. Every epic must reference at least one clause.
- **From AWS Kiro:** the `spec → design → tasks → implementation` decomposition inside each story. This closes BMAD's known handoff gap where design issues surfaced only at implementation time.

**Why BMAD as backbone:**
- Enterprise scale (100 k+ device fleets, multi-year lifecycle).
- Safety- and regulation-critical (FCC, CE, CRA, GDPR).
- Audit-friendly artifacts required for both certification and disclosure.
- Multi-team, multi-language, multi-hardware — needs a full agile-team simulation.

**Why not Kiro alone:** it's an IDE, not a portfolio methodology. We use its decomposition idea, not the product.

**Why not Spec Kit alone:** no code-review checkpoint; regenerates full plans on iteration (bad for a codebase this size).

Sources logged in `governance/decisions.md` D-0006 (this decision).

---

## 2. Artifact definitions

### 2.1 Brief (`01_BRIEF.md`)
Executive-level statement of the problem, users, market, and the core outcomes we intend to deliver. Two-pager. Reader: anyone new to the program.

### 2.2 PRD (`02_PRD.md`)
Product Requirements Document. Functional and non-functional requirements, user personas and journeys, MVP scope vs. later, KPIs, out-of-scope, open questions. Reader: PMs, engineering leads, external partners under NDA.

### 2.3 Architecture (`03_ARCHITECTURE.md`)
Technical architecture across firmware, apps, cloud, network, hardware, security, data. Includes: component diagram (ASCII), data flows, interface contracts, technology choices with rationale, non-functional posture (perf, security, availability, cost), open architectural questions. Reader: engineering.

### 2.4 Blueprint (`04_BLUEPRINT.md`)
Delivery plan: workstream RACI, dependency ordering, milestones (M0..MN, no dates — see PRD §non-timeline rule), risk register, rollout gates, launch criteria per SKU. Reader: program managers, engineering leadership, ops.

### 2.5 Epic (`epics/EPIC-NN-*/EPIC.md`)
A durable body of work producing one or more shippable outcomes, larger than any single release. Each epic:

- Names its **outcome** (the observable change when done).
- Names the **constitution clauses** it satisfies.
- Names its **shards** (sub-outcomes, i.e. sharded-epic sections).
- Names its **dependencies** on other epics.
- Names its **exit criteria** (measurable, testable).
- Names its **RACI**.
- Names its **risks** and mitigations.
- Links every story that belongs to it.

### 2.6 Shard (section inside `EPIC.md`)
A cohesive sub-outcome inside an epic, small enough to be delivered end-to-end by one team as a contiguous block of work. Each shard names its stories.

### 2.7 Story (`epics/EPIC-NN-*/STORIES.md` sections)
The atomic unit of delivery. Every story in `STORIES.md` has, in this exact shape:

```markdown
### S-NN-MMM — <title>
As <role> I want <capability> so that <benefit>.
- AC: <testable bullet>; <testable bullet>[; ...]
- Meta: Shard=<letter(s)> | Type=<type> | Size=<XS..XL> | Prio=<P0..P3> | Status=<status> | SKU=<L/A/O/★> | PRD=<F-*/NF-* ids or —> | Const=<clause key(s), see §2.9>
```

- ID: `S-NN-MMM` where NN is epic number, MMM is a monotonically increasing counter. IDs are never reused, even for DROPPED stories.
- The `Meta:` line is **machine-parsed** by `tools/gen-stories-index.py` to regenerate `STORIES_INDEX.md`. Field order and `|` separators are mandatory.
- **Two elaboration states.** A story as registered above is sufficient for status `DRAFT`. Before a story may move `DRAFT → READY` it must be elaborated in place with the Kiro decomposition: `- Tasks: spec … · design … · impl … · test … · docs …` and `- Deps: <story IDs / RFCs / external>`. CI blocks the status change without them.

### 2.8 Traceability Matrix (`STORIES_INDEX.md`)
Master ledger. Rows = stories. Columns = epic, shard, PRD requirement satisfied, constitution clause satisfied, target SKU, priority, size, status. **Generated** — never hand-edit the tables; run `python3 tools/gen-stories-index.py` after any `STORIES.md` change. Hand-written sections (coverage check, WG summary) live below the generated marker.

### 2.9 Constitution clause keys
All portfolio artifacts reference constitution documents by **clause key**, never by prose nicknames. The canonical key → file mapping (files live at repo root unless a path is shown):

| Key | File |
|---|---|
| C-00 | `00_MASTER_SOFTWARE_PLAN.md` |
| C-01 | `01_SS-SP_LITE_HARDWARE_REFERENCE.md` |
| C-02 | `02_PROTOCOL_STACK.md` |
| C-03 | `03_UI_LAYOUT_SPEC.md` |
| C-04 | `04_LICENSING_AND_FORK_STRATEGY.md` |
| C-05 | `05_SECURITY_MODEL.md` |
| C-06 | `06_GOVERNANCE.md` |
| C-07 | `07_BUSINESS_MODEL_AND_OPEN_SOURCE.md` |
| C-08 | `08_UNIVERSAL_CONNECTIVITY.md` |
| C-OA | `governance/OPEN_ASSURANCE.md` |
| C-TM | `docs/TRADEMARK.md` |
| C-CoC | `CODE_OF_CONDUCT.md` |
| C-SEC | `SECURITY.md` |

Topic routing (what lives where): firmware stack / HAL / apps / SDK / voice / test strategy → **C-00**; hardware → **C-01** (Lite) and C-00 §tiers + `models/CATALOG` (Alpha/Omega); protocol wire formats → **C-02**; UI → **C-03**; licensing/BSL/Meshtastic fork stance → **C-04**; crypto / identity / secure boot / OTA security → **C-05**; governance/RFC/CI/merge → **C-06**; business/pricing/Home-Mode economics → **C-07**; SS-Link / bearers / Home Gateway / connectivity → **C-08**.

---

## 3. Story sizing

We use T-shirt sizes expressing **relative complexity and blast radius**, never hours/days/sprints. See non-timeline rule below.

| Size | Meaning |
|---|---|
| XS | Single-file change; no interface impact; trivially reviewable |
| S  | Few files, one component; no cross-component interface change |
| M  | Multi-file, single component or one interface pair; needs a design note |
| L  | Multi-component; interface or wire-format impact; needs design review before pickup |
| XL | Multi-team / multi-epic blast radius; **must be re-sharded** into ≤ M stories before pickup |

Stories at size L must pass design review, and XL must be re-sharded, before entering active work.

---

## 4. Non-timeline rule (from repo CLAUDE-level convention)

Portfolio artifacts do not contain calendar dates or duration estimates ("2 weeks", "Q3 2026"). They contain:

- Ordering (M0 before M1 before M2).
- Dependencies (X blocks Y).
- Priorities (P0, P1, P2, P3).
- Sizes (XS..XL).

Turning ordering + dependencies + priorities into a calendar is a program-management concern, done in the project tracker (Linear/GitHub Projects/Jira), not in the portfolio.

---

## 5. Definition of Done (applies to every story)

A story is DONE when **all** of the following are true:

1. Acceptance criteria pass in CI.
2. Unit tests cover ≥ 80 % of the changed lines (mutation-tested for security-sensitive code).
3. Integration or hardware-in-loop tests pass where applicable.
4. Documentation updated (user, dev, protocol, or wire, whichever applies).
5. If wire-format changes: RFC is ACCEPTED and referenced.
6. If security-relevant: `wg-security` sign-off recorded in the PR.
7. If licensing-relevant: `wg-legal` sign-off recorded in the PR.
8. If regulatory-relevant (FCC/CE/CRA/GDPR): compliance doc updated.
9. Reproducible-build attestation regenerated where relevant.
10. Changelog entry.
11. No new open `TODO` markers without a filed issue.
12. Merged to main via green PR with two reviewer approvals (one code, one domain).

---

## 6. Story types

- **Feature** — new user-visible capability.
- **Task** — engineering work with no user-visible change (refactor, upgrade, cleanup).
- **Spike** — timeboxed research producing a decision doc, not code.
- **Bug** — regression or defect fix.
- **RFC** — protocol/wire/security-relevant change requiring formal RFC.
- **Compliance** — regulation-driven work item.
- **Ops** — infrastructure, CI/CD, deployment.
- **Docs** — documentation deliverable (site, guide, reference) as the primary output.

---

## 7. Priorities

- **P0** — must ship before the next release; blocking.
- **P1** — should ship in the next release; strong desire.
- **P2** — nice to have next release; may slip.
- **P3** — backlog, ship when convenient.

Only P0 and P1 stories may be pulled into active work without a re-triage.

---

## 8. Statuses

`DRAFT → READY → IN_PROGRESS → IN_REVIEW → DONE` with two off-ramps: `BLOCKED` and `DROPPED`. Every off-ramp writes a note in the story.

---

## 9. Working-group ownership

Every epic and every story has a primary Working Group per `06_GOVERNANCE.md` §3:

- `wg-firmware`
- `wg-hardware`
- `wg-protocol`
- `wg-security`
- `wg-ui-ux`
- `wg-apps` (mobile + desktop + web)
- `wg-cloud`
- `wg-sdk`
- `wg-legal`
- `wg-docs`
- `wg-ops` (CI, infra, manufacturing line)
- `wg-community` (governance, RFC shepherding, CoC)

Only the primary WG may close a story. Contributing WGs sign off in the PR.

---

## 10. How to read this portfolio in order

1. Skim `01_BRIEF.md` — what and why.
2. Read `02_PRD.md` — the product, the users, the requirements.
3. Read `03_ARCHITECTURE.md` — how it's built.
4. Read `04_BLUEPRINT.md` — in what order and by whom.
5. Skim `EPIC_INDEX.md` — the 24 epics at a glance.
6. Read the epics relevant to your role.
7. Read the stories inside those epics.
8. Cross-reference with `STORIES_INDEX.md` when tracing requirements upward.

---

*End of 00 — Methodology.*
