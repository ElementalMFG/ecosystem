---
# SPDX-License-Identifier: Apache-2.0
name: t1-pipeline
description: Run the full T1 workflow for security-critical work (crypto, keys, secure-boot, wire formats, sandbox boundary, vectors) — MAX-model implementation plus both mandatory review passes plus gates, per doc 10 §2/§10.
---

# T1 pipeline

Execute in order; do not skip steps.

1. **Confirm the tier.** Look the story/epic up in `docs/portfolio/10_MODEL_ALLOCATION_STRATEGY.md` §5/§7/§11. If it is not T1, stop and use the normal tier recipe instead.
2. **Confirm (model @ effort).** T1 authorship runs on Fable 5 @ xhigh — verify via `python3 tools/allocation.py --story S-NN-MMM` and the statusline. If the SESSION is under-provisioned, STOP and give the user the exact command: either exit and run `claude --continue --model claude-fable-5 --effort xhigh` (same conversation, correctly provisioned) or start fresh with `tools/claude/work.sh S-NN-MMM`; in-session `/model` + `/effort xhigh` also works (re-check together — switching resets effort). This skill deliberately carries NO frontmatter model/effort override: those apply per-turn only and would silently revert mid-authorship (doc 11 §6a) — the SESSION itself must be provisioned. Never demote T1; a refusal is handled per doc 10 §1.4.1 (reframe as defensive/own-product security engineering and re-run).
3. **Contract first.** Read the AC, governing RFC, and the relevant sections of `05_SECURITY_MODEL.md` / `02_PROTOCOL_STACK.md` before writing anything. If the contract is missing or ambiguous, the contract itself is the first T1 deliverable.
4. **Implement** (or design). Wire bytes, key hierarchy, eFuse plans, and vector sets get byte-level care — they freeze at first ship.
5. **Review pass 1:** invoke the `t1-review` agent with only the artifact paths + contract references — never the authoring reasoning (fresh context is the point).
6. **Review pass 2:** invoke the `t1-cross-review` agent the same way.
7. **REWORK findings** → fix → re-run the affected review pass. Repeat until both verdicts are APPROVE or APPROVE-WITH-NITS (nits fixed or ticketed).
8. **Gates:** `make lite` (if firmware touched), `git add` then `python3 tools/lint-docs.py`, `python3 tools/gen-stories-index.py --check` (if stories touched) — from repo root.
9. **Report:** what was built, both verdicts with finding counts, gate results — and remind that merge requires 2 CODEOWNER approvals + wg-security sign-off (`CONTRIBUTING.md` §9), plus an RFC if wire/crypto/API changed.
