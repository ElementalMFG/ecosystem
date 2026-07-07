---
# SPDX-License-Identifier: Apache-2.0
name: story-run
description: Execute one portfolio story end-to-end — select, elaborate (Tasks/Deps per §2.7), tier-route per doc 10, implement, verify AC + gates, update Status, regenerate the index, commit with a Story trailer. The standard unit of work for docs/portfolio stories.
# Orchestration is not intelligence-sensitive (doc 11 §3): run it at medium
# automatically, overriding the session effort while this skill is active.
# Deliberately NO model override here — per-turn model reversion would be a
# silent-demotion trap; the session model comes from tools/claude/work.sh.
effort: medium
---

# Story run (the standard execution loop)

One story per run. Do not batch stories in a single run — small, traceable
increments are the methodology (`00_METHODOLOGY.md`).

## 1. Select

- Given a story ID, find it in `docs/portfolio/epics/EPIC-NN-*/STORIES.md`.
- Given no ID, propose the next story: filter `Status=READY` first, then
  `DRAFT`, by `Prio` (P0 → P3), preferring stories whose `Deps` are all DONE.
  Ask the user to confirm before starting.
- Read the full story block **and** the epic's `EPIC.md`, plus every PRD ID
  and constitution clause (`Const=`) the Meta line cites.

## 2. Elaborate (DRAFT → READY)

Per `00_METHODOLOGY.md` §2.7, before implementation the story must carry, in
place in its `STORIES.md` block:

- `- Tasks: spec … · design … · impl … · test … · docs …`
- `- Deps: <story IDs / RFCs / external, or —>`

`tools/gen-stories-index.py --check` blocks any non-DRAFT status without
them. Every `S-NN-MMM` in Deps must exist; incomplete deps (not DONE) must be
flagged to the user before proceeding.

## 3. Tier-route (mechanical — no judgment call)

Run `python3 tools/allocation.py --story S-NN-MMM` and follow its recipe:

- **T1** → stop; use the `t1-pipeline` skill instead (never demote T1).
- **T1?** (keyword-flagged) → resolve the flag NOW as part of elaboration:
  read the story against doc 10 §5/§7; if genuinely T1, record it in
  `tools/allocation.py`'s override table (with citation) and switch to
  `t1-pipeline`; else proceed on the floor recipe and note the resolution.
- **T2/T3/T4** → compare the recipe's orchestrator (model @ effort) to the
  statusline. Over-provisioned (e.g. Fable session running a T3/T4 story) →
  proceed, but flag the waste in the report. Dispatch to the recipe's tier
  agent(s), with an explicit output budget in the prompt (doc 11 §4.6).
- **T2 flow** (doc 11 §6f): dispatch `t2-designer` (fable@medium) with the
  story AC + contract references to produce the FROZEN contract; review its
  contract report; then dispatch `t2-builder` (opus@high) against those
  contract paths. The orchestrator never authors T2 designs itself.
- **Escalation (doc 10 §8, mechanical):** a tier agent failing the same task
  twice → re-dispatch the same spec one tier up (`t3-standard` →
  `t2-builder`; `t2-designer` → report to the user for a Fable@high design
  session). Never retry a third time at the same level.

## 4. Implement

- Set `Status=IN_PROGRESS` in the Meta line at start of work.
- Honor domain rules (`.claude/rules/`): HAL caps via `ss_hal_has_cap()`,
  clean-room boundaries, T1 path stops, SPDX headers on new files.
- Work the `Tasks:` decomposition in order: spec → design → impl → test → docs.

## 5. Verify against AC

Walk the story's `- AC:` clause by clause with concrete evidence (command
output, file path, test result). An AC that cannot be verified now (e.g.
needs hardware or a hosted repo) blocks `DONE` — use `IN_REVIEW` or `BLOCKED`
and tell the user why.

## 5a. Knowledge sweep (before gates — makes fresh sessions lossless)

Ask: did this story learn anything a FUTURE story needs that is not already
in a contract doc-block, a `.claude/rules/` file, or a `TODO(S-NN-MMM)` code
marker? If yes, append 1–3 lines to `docs/dev/ENGINEERING_LOG.md`
(append-only, `- S-NN-MMM (date): fact.`); promote durable domain facts to
the matching rules file. If no, that's fine — but the step is mandatory and
the report must state `Learnings: <where recorded | none>`. Also grep the
log at story START for the component you're touching.

## 6. Gates + status + commit

1. Update `Status=` (`DONE` only when every AC has evidence).
2. `python3 tools/gen-stories-index.py` AND `python3 tools/allocation.py
   --generate` (both files are generated — never hand-edit; the allocation
   map carries story status, so EVERY status change must regenerate it or
   the docs-lint CI gate fails).
3. Run the `verify` skill (stage → docs lint → index --check → firmware
   build if `firmware/**` touched → DCO).
4. Update `docs/CHANGELOG.md` if user-visible (CONTRIBUTING.md §3).
5. Commit (Conventional Commits, `git commit -s`) with a trailer:
   `Story: S-NN-MMM` (CONTRIBUTING.md §8).

## 7. Report

Story ID + title, tier used, AC evidence table, gate results, new Status,
and any follow-up stories or blockers discovered (propose them as DRAFT
stories rather than doing unplanned work).
