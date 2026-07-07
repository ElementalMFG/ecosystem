<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# SS-SP — session policy

Monorepo for the SS-SP mesh-pager program: ESP-IDF firmware (Lite/Alpha/Omega), protocol specs, companion apps, cloud, governance. Portfolio of record: `docs/portfolio/` (24 epics; story count and per-story allocation in the generated `STORIES_INDEX.md` / `ALLOCATION_MAP.md`).

## Build & verify

- Firmware: `make lite` (digest-pinned container path in `docs/dev/BUILDING.md`).
- Docs gate: `make lint-docs` and `python3 tools/gen-stories-index.py --check` — run from repo root, after `git add` (untracked files are invisible to the linter).
- Every commit: `git commit -s` (DCO, real identity). Conventional Commits style.

## Model & effort allocation (binding — `docs/portfolio/10_MODEL_ALLOCATION_STRATEGY.md`)

Work is tiered T1–T4; the tier of any story/epic is in doc 10 §5/§7/§11. Before non-trivial work, determine the tier and match the recipe:

| Tier | Recipe | Delegate to |
|---|---|---|
| T1 (crypto, keys, secure-boot, wire formats, sandbox, fuzzer/vector design) | Fable 5 @ xhigh; then `t1-review` + `t1-cross-review` agents | `t1-review`, `t1-cross-review` |
| T2 (novel design → routine build) | Fable 5 @ high designs; build via `t2-builder` or Fable @ medium | `t2-builder` |
| T3 (well-specified, gate-verified) | Opus 4.8 @ high or Fable 5 @ low–medium | `t3-standard` |
| T4 (mechanical, tool-checkable) | Opus 4.8 @ low, batched | `t4-mechanical` |

Standing rules:

- **Token economy is binding** (`docs/portfolio/11_TOKEN_ECONOMY.md`): sessions start on **Opus 4.8 @ medium** for T3/T4 orchestration; switch to Fable 5 only for T1 authorship (@ xhigh) / T2 design (@ high). One story batch per session; keep main context under ~150k (`/clear` + `/rename` between batches). Use `ultrathink` in-prompt for isolated hard calls instead of raising session effort.
- **CI-wait policy** (doc 11 §4.5): routine pushes get a single `gh run list` check at the next natural step — no live monitors except for high-risk pushes (T1, workflow changes). Flip story status when the check lands.
- **Every subagent dispatch states an output budget** (e.g. "report ≤ 40 lines"); verbose evidence stays in the subagent's context (doc 11 §4.6).
- **Delegate searches/reads to the `retrieval` agent** (Haiku) — keep bulk file contents out of this context.
- Portfolio work runs through the `story-run` skill (select → elaborate → tier-route → implement → verify AC → status/index → `Story:` trailer commit); one story per run.
- T1 work: run the `t1-pipeline` skill (encodes the §10 double-review workflow). Pre-commit: the `verify` skill runs all gates.
- **Tune effort before switching model** (doc 10 §1.3). Re-check `/effort` after every `/model` — switching resets it.
- Never demote T1 work; a Fable refusal is handled per doc 10 §1.4.1 (reframe as defensive/own-product security engineering, re-run), never silently completed on a weaker model.
- Escalation triggers (doc 10 §8): two failed STD attempts, surprise contract contradiction, scope creep into keys/wire/eFuse/persistence — stop and escalate.

## Compact instructions

When compacting, preserve: current story ID + tier, contract/AC text, diffs and decisions, open verification failures, git state. Drop: raw file reads, tool output already acted on, search results.
