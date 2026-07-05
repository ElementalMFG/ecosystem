---
# SPDX-License-Identifier: Apache-2.0
paths:
  - "docs/portfolio/**"
---

# Portfolio domain facts (verified 2026-07)

- Layout: `docs/portfolio/epics/EPIC-NN-slug/` — each epic directory holds `EPIC.md` + `STORIES.md`. 24 epics (EPIC-01 governance … EPIC-24 compliance), 533 stories. `EPIC_INDEX.md` is the epic list; `STORIES_INDEX.md` is **generated — never hand-edit**; regenerate with `python3 tools/gen-stories-index.py`, verify with `--check` (repo root only).
- Story grammar (the index parser depends on it exactly):
  - `### S-NN-MMM — title`
  - user-story line ("As a …")
  - `- AC:` (acceptance criteria)
  - `- Meta: Shard= | Type= | Size= | Prio= | Status= | SKU= | PRD= | Const=`
  - elaboration lines after Meta: `- Tasks: spec … · design … · impl … · test … · docs …` and `- Deps: <story IDs / RFCs / external, or —>`
- `Status=` values: `DRAFT` | `READY` | `IN_PROGRESS` | `IN_REVIEW` | `DONE` | `BLOCKED` | `DROPPED`. Do not invent new fields or statuses without updating the generator.
- §2.7 elaboration rule (generator-enforced): any status other than `DRAFT`/`DROPPED` requires the `- Tasks:` and `- Deps:` lines; every `S-NN-MMM` in Deps must resolve. Use the `story-run` skill as the standard execution loop; commits carry a `Story: S-NN-MMM` trailer.
- Numbered docs `00_METHODOLOGY.md` … `10_MODEL_ALLOCATION_STRATEGY.md` are the portfolio method + strategy set; doc 10 is the binding model/effort allocation — a story's tier is looked up via doc 10 §5/§7/§11 before work starts.
- Wording of normative text (AC, Meta, tier assignments) is a judgment call — T4 agents must not rewrite it, only mechanically reformat.
