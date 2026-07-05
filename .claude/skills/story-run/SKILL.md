---
# SPDX-License-Identifier: Apache-2.0
name: story-run
description: Execute one portfolio story end-to-end — select, elaborate (Tasks/Deps per §2.7), tier-route per doc 10, implement, verify AC + gates, update Status, regenerate the index, commit with a Story trailer. The standard unit of work for docs/portfolio stories.
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

## 3. Tier-route

Look the story up in `docs/portfolio/10_MODEL_ALLOCATION_STRATEGY.md` §5/§7/§11:

- **T1** → stop; use the `t1-pipeline` skill instead (never demote T1).
- **T2/T3/T4** → confirm (model @ effort) against doc 10 §2.2 via the
  statusline; dispatch to the matching tier agent or proceed inline.

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

## 6. Gates + status + commit

1. Update `Status=` (`DONE` only when every AC has evidence).
2. `python3 tools/gen-stories-index.py` (regenerate — never hand-edit
   `STORIES_INDEX.md`).
3. Run the `verify` skill (stage → docs lint → index --check → firmware
   build if `firmware/**` touched → DCO).
4. Update `docs/CHANGELOG.md` if user-visible (CONTRIBUTING.md §3).
5. Commit (Conventional Commits, `git commit -s`) with a trailer:
   `Story: S-NN-MMM` (CONTRIBUTING.md §8).

## 7. Report

Story ID + title, tier used, AC evidence table, gate results, new Status,
and any follow-up stories or blockers discovered (propose them as DRAFT
stories rather than doing unplanned work).
