<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# 11 — Token Economy Playbook (companion to doc 10)

Binding operational rules for **where premium model capacity is genuinely
required and where it is waste**, derived from (a) a full-portfolio tier
audit, (b) measured spend patterns of the first 12 executed stories, and
(c) current official guidance (researched 2026-07-07). Doc 10 defines *what
tier work is*; this doc defines *how to run sessions so the tiers cost what
they should*.

## 1. Evidence — where the money actually went

Observed across the first execution sessions (S-01-011 → S-02-010):

- **The main session is the dominant cost, not the subagents.** Delegated
  work ran on pinned cheap models (Haiku ~8–46k, Opus-low 13–36k,
  Opus-high 53–96k tokens per story) while the orchestrating session sat on
  Fable 5 with an ever-growing context. Every turn re-reads that context;
  past ~200k tokens, long-context premium pricing applies on 1M-window
  models, so *session length compounds cost quadratically in practice*.
- **CI-wait monitors were expensive conveniences.** Each background monitor
  event re-invoked the main session against the full cached context
  (~15+ events in one day). Convenience, not correctness.
- **Fable 5 @ high/xhigh was used for orchestration that needed neither.**
  Elaborating a T4 story, staging commits, and reading CI results are not
  intelligence-sensitive activities; the tier agents already guarantee the
  quality of the work itself.
- **T1 was rare and cheap relative to its value:** one T1 story (S-02-008)
  consumed ~290k subagent tokens across four review passes — and caught
  four MAJOR defects. That is the *right* place to spend.

## 2. Research findings (official, verified 2026-07-07)

From `code.claude.com/docs/en/costs` and `/en/model-config`:

- **Effort is the first-class cost dial.** Levels `low → max` on Fable 5 /
  Opus 4.8 / Sonnet 5; default `high`. Thinking tokens bill as output and
  can be "tens of thousands of tokens per request". `medium` is the
  documented cost-sensitive setting; `xhigh`/`max` are for demanding tasks
  (`max` is "prone to overthinking"). Fable 5 cannot disable thinking.
- **`ultrathink` keyword**: one-off deep reasoning on a single turn without
  raising session effort — the correct tool for isolated hard decisions
  inside an otherwise cheap session.
- **Switching models resets effort to that model's default** — re-check
  `/effort` after every `/model` (already a CLAUDE.md rule).
- **`/clear` between unrelated tasks** is the top documented lever ("stale
  context wastes tokens on every subsequent message"); `/rename` +
  `/resume` preserve findability. Custom compact instructions are honored
  (ours exist in CLAUDE.md).
- **Prompt caching is automatic** (5-minute TTL): batching related story
  runs back-to-back keeps the cache warm; config changes to `.claude/`
  mid-session invalidate the system-prompt prefix.
- **Subagent delegation for verbose operations** and **hook-based
  preprocessing** (filter logs before they enter context) are the two
  documented isolation levers; MCP tool definitions are deferred by
  default (ours cost 0 until used).
- **Keep CLAUDE.md under ~200 lines**, move workflow detail to skills
  (ours: ~1k tokens, skills lazy — compliant).
- Community consensus (multiple 2026 guides) matches: start sessions on the
  cheaper model and escalate deliberately; compact before the auto
  threshold; route by task, not by habit.

## 3. Binding determinations — what needs Fable, what does not

Portfolio census: **533 stories; ~110 (~21%) are genuinely T1** (all of
EPIC-06/07/08 = 58 stories, plus wire-format/state-machine/sandbox cores in
EPIC-02/03/09–13/16–18/22/23). Everything else is T2 build, T3, or T4.

| Activity | Model @ effort | Rationale |
|---|---|---|
| **T1 authorship** (crypto, keys, secure-boot, wire formats, sandbox boundaries, vector/fuzzer design) | **Fable 5 @ xhigh** — never demoted | Errors freeze at ship; doc 10 §10 unchanged |
| **T1 reviews** | `t1-review` Fable @ high + `t1-cross-review` Opus @ xhigh | Locked agent pins; model diversity |
| **T2 design** (novel contracts, state machines) | `t2-designer` **Fable 5 @ medium** (§6f; was main-session Fable @ high) | Judgment-dense but short; doc 10 §1.3 parity: Fable @ medium keeps the capability edge at ~half the tokens; two failures → Fable @ high session |
| **T2 build** (against a frozen contract) | `t2-builder` Opus 4.8 @ **high** (was xhigh) | Contract is frozen; gates + tests catch drift; xhigh was paying for judgment the contract already made |
| **T3 build** | `t3-standard` Opus 4.8 @ **medium** (§6f; was high) | Verification-saturated tier; two failures → re-dispatch to `t2-builder` @ high |
| **T4 mechanical** | `t4-mechanical` Opus 4.8 @ low, batched | Unchanged |
| **Retrieval/reads** | Haiku @ low, **output-capped** | Unchanged + new output budget |
| **Main-session orchestration of T3/T4 story runs** (elaborate, dispatch, verify AC, gates, commit) | **Opus 4.8 @ medium** | Not intelligence-sensitive; the tier agents own work quality; use `ultrathink` for isolated hard calls |
| **Main-session orchestration during T1/T2-design blocks** | Fable 5 (@ xhigh only while authoring T1) | Context quality where it matters |

**The standing default flips:** sessions start on **Opus 4.8 @ medium** and
escalate to Fable for T1/T2-design blocks — not the reverse. Doc 10 §1.3's
"tune effort before switching model" still governs *within* a block.

## 4. Session lifecycle rules (the biggest lever)

1. **One story batch per session.** `/clear` (after `/rename`) between
   epics or unrelated batches. Target: keep main context **under ~150k
   tokens**; watch the statusline `ctx %`.
2. **T1 blocks start fresh**: new/cleared session, `/model` Fable,
   `/effort xhigh`, run `t1-pipeline`, then drop back (`/effort high` or
   switch to Opus @ medium for the next T3/T4 batch).
3. **Batch while the cache is warm** (5-min TTL): run related stories
   back-to-back; don't interleave unrelated work.
4. **No config edits mid-batch**: changes under `.claude/` bust the cached
   prefix — group them at batch boundaries.
5. **CI-wait policy:** for routine pushes, do **not** arm a live monitor.
   Push, keep working (or end the turn), and verify CI with a single
   `gh run list` check at the next natural step; flip story status then.
   Live monitors are reserved for high-risk pushes (T1 merges, workflow
   changes whose failure blocks everything).
6. **Subagent output budgets:** every dispatch prompt states a maximum
   report size ("return ≤ 40 lines"); retrieval returns only what will be
   acted on, never full-file dumps. Verbose evidence stays in the
   subagent's context by design.
7. **Optional (owner call):** unused marketplace plugins (legal,
   productivity, data, enterprise-search) cost ~20k context tokens per
   session in agent/skill listings. They can be disabled per-project via
   `settings.json` without uninstalling. Preserved for now per D-owner
   preference; revisit if spend pressure returns.

## 5. Guardrails — what this policy may NEVER do

- **Never demote T1** (doc 10 §1.4/§8 unchanged): tier, double-review, and
  escalation triggers are quality law; this doc only strips cost from
  *orchestration and over-provisioned effort*, not from verification.
- Gates are unaffected: every story still runs lint/index/parity/policy/
  covenant/build gates and AC-evidence verification.
- A Fable refusal on T1 work is still handled per doc 10 §1.4.1 — reframe
  as defensive/own-product security engineering, never silently complete on
  a weaker model.
- Two failed attempts at a lower tier/effort → escalate (doc 10 §8), don't
  retry-loop cheaply.
- Edge cases audited: T2-builder demotion to `high` is safe because every
  T2 build lands behind a frozen contract + host tests + CI; the two
  completed T2 builds (`ss_log`, covenant suite) would have passed
  identically at `high` (their difficulty was in the contract, which stays
  Fable-side). If a T2 build fails twice at `high`, §8 escalation applies.

## 6. Applied with this change

- `t2-builder` agent pin: `effort: xhigh` → `effort: high`.
- `retrieval` agent: explicit output-budget instruction added.
- CLAUDE.md: session-start model policy, CI-wait policy, output budgets.
- This document; doc 10 §1.3 cross-reference.

## 6a. Automation layer (added same day)

Manual dial-setting is now the fallback, not the mechanism:

- **`tools/allocation.py`** resolves every story's tier + full recipe
  mechanically (explicit overrides → keyword `T1?` flags → epic floors) and
  generates **`ALLOCATION_MAP.md`** (all stories, checked by CI in
  `docs-lint`). Audit at adoption: 63 T1 confirmed, 111 `T1?` to confirm at
  elaboration, 148 T2, 206 T3, 6 T4.
- **`tools/claude/work.sh S-NN-MMM`** (or `make story S=S-NN-MMM`) launches
  the session with the correct `--model`/`--effort` flags and the right
  skill prompt baked in — the correct allocation is the default path.
- **Skills enforce it in-flight:** `story-run` step 3 consults the resolver
  (T1 → reroute to `t1-pipeline`; `T1?` → resolve the flag at elaboration
  and record it in the override table; under-provisioned → stop and
  relaunch; over-provisioned → proceed but flag waste). `t1-pipeline`
  step 2 hard-stops on under-provisioned sessions.
- **Platform limits honestly stated:** the main session's model/effort can
  only change via user command or launch flags — hence launchers + in-skill
  stops rather than a fictional in-flight self-switch. Delegated work needs
  no such mechanism: agent pins are platform-enforced per invocation.

## 6b. Automatic effort adjustment (researched + applied, same day)

Official frontmatter reference (skills doc, verified 2026-07-07):

- **`effort:` in SKILL.md** *"overrides the session effort level"* while the
  skill is active → applied: `story-run` carries `effort: medium` and
  `verify` carries `effort: low` — orchestration and gate-running now
  auto-drop below the session level with zero user action.
- **`model:` in SKILL.md** applies *"for the rest of the current turn"* and
  *"the session model resumes on your next prompt"* → **deliberately NOT
  used on `t1-pipeline`**: a per-turn Fable override would satisfy the
  first turn and then silently revert to a weaker model on the next user
  message mid-authorship — the exact silent demotion doc 10 §1.4 forbids.
  T1 provisioning stays session-level (launcher or explicit command), with
  the skill's hard stop as the guard.
- **Context-preserving escalation:** the installed CLI accepts `--model`
  and `--effort` together with `--continue`/`--resume`, so an
  under-provisioned session can be reopened AS THE SAME CONVERSATION,
  correctly provisioned: `claude --continue --model claude-fable-5
  --effort xhigh`. The `t1-pipeline` stop message now cites this exact
  command.
- **Validated end-to-end (2026-07-07):** launcher dry-runs correct for all
  five resolution paths (T1/T2/T3/T4/T1?+warning) and error inputs exit
  non-zero; the map `--check` gate catches tampering (exit 1) and passes
  clean regeneration; the stop-and-instruct behaviour has a live
  precedent (S-02-008: pipeline halted until `/effort xhigh` was set).

## 6c. Queue runner — multi-story automation (added after three clean runs)

`tools/claude/run-queue.sh` (`make queue Q="S-AA-XXX S-BB-YYY"`) executes an
**explicit, human-approved story list** headlessly: per story it resolves
the recipe, runs `claude -p "/story-run <id>"` at the recipe's model/effort,
logs to `logs/queue/`, then enforces post-checks (clean tree, new commit
with the `Story:` trailer, pushed, CI green within 12 min) before the next
story. Hard stops: any **T1/T1?** story halts the queue (exit 3) with the
interactive launch command — T1 never runs headless; any failed check
aborts (exit 1/2). Command cadence drops to roughly one per day for T3/T4
grind while every safety layer (gates, CI, covenant, branch protection,
tier stops) stays engaged. `--dangerous` (full permission bypass) exists
but is opt-in; default is `--permission-mode acceptEdits`. Session-history
guidance: batch boundaries use fresh sessions (`make story`/`make queue` —
the repo is the memory); mid-story tier walls use
`claude --continue --model … --effort …` (same conversation, re-provisioned).

## 6d. Lossless fresh sessions (closing the context tradeoff)

Fresh-per-story sessions cost quality only if knowledge stays
session-resident. Three structural pieces make externalization automatic
rather than disciplinary:

1. **Engineering log** (`docs/dev/ENGINEERING_LOG.md`): append-only,
   committed, greppable raw-capture layer for cross-story learnings; seeded
   with the program's accumulated gotchas. Durable domain facts graduate to
   `.claude/rules/`.
2. **Mandatory knowledge sweep** (`story-run` step 5a): every story closes
   by recording its learnings (or explicitly `none`) — the report carries a
   `Learnings:` line, so omission is visible.
3. **SessionStart auto-brief** (`tools/claude/session-brief.sh`, registered
   as a SessionStart hook): every fresh session begins with ~20 injected
   lines — last commits, in-flight stories, recent log entries, policy
   pointers — so cold starts see the program state without any retrieval
   round-trip.

Together: nothing durable can silently stay behind in a dead session, and
every new session starts pre-briefed — fresh context stops being a
tradeoff and becomes strictly better (sharp attention + guaranteed
carryover of what matters).

## 6e. Ordering guard + periodic audit (added after the first queue cycle)

- **Story order is a dependency-graph property, not a numeric sequence.** An
  order is valid iff no story starts before its `Deps:` are satisfied
  (DONE, or IN_REVIEW = satisfied-with-note) and unblocked P0 work precedes
  P1/P2. `tools/allocation.py --next` prints the mechanically-derived
  eligible frontier (priority-sorted, T1 rows marked interactive-only) plus
  a ready-to-paste queue suggestion; `--eligible S-NN-MMM` is the per-story
  guard, wired into `run-queue.sh` as an ORDER-GUARD abort (exit 4) so an
  ineligible story cannot launch headlessly. Verified against the live
  portfolio (dep graph currently fully clean; blocked path proven with a
  synthetic case).
- **Periodic audit**: `weekly-audit.yml` (Mondays 06:00 UTC + manual
  dispatch) runs all seven repo gates server-side and prints an
  informational aging report of IN_REVIEW/IN_PROGRESS stories; `make audit`
  runs the same locally. Cadence guidance: manual dispatch before starting
  a new epic and after any hardware-verification day.

## 6f. Second-pass reduction (post-limit-event audit, 2026-07-07)

Triggered by real spend pressure after 17 stories; every change below keeps
an escalation guard so a quality miss becomes a retry-up, never a failure.

- **T2 re-architected** (~148 stories): orchestration moves to Opus @
  medium (same bookkeeping role as T3); the design slice — the only part
  needing Fable — moves to the new `t2-designer` agent at **Fable @
  medium** (doc 10 §1.3: capability edge retained at ~half the tokens of
  high). `t2-builder` (Opus @ high) implements, unchanged. Measured basis:
  S-02-016 ran its whole story on Fable @ high when only its contract
  design needed Fable.
- **`t3-standard` pin high → medium** (206 stories): the tier is
  verification-saturated (clang -Werror, host tests, parity/policy gates,
  3-board CI). Guard: two failed attempts → re-dispatch the same spec to
  `t2-builder` (Opus @ high) — mechanical, in the story-run skill.
- **T1 review-rerun rule**: rework re-runs only the pass that issued the
  findings unless core contract/semantics changed (S-02-008 ran both twice
  by author's choice; ~70–90k tokens per rework cycle saved).
- **Why not Fable @ low anywhere:** Fable is 2× Opus per token; at `low`
  it gives up the deliberation that is its advantage while keeping the
  price. Opus @ medium dominates it on cost for bookkeeping; Opus @ high
  dominates it on capability-per-dollar for building. Fable's only
  justified slots: T1 authorship (xhigh), T1 review (high), T2 contract
  design (medium).
- **Orchestrator LAUNCH table is role-based, not tier-prestige-based:**
  every story-run orchestrator (T2/T3/T4/T1?) launches Opus @ medium
  because select/elaborate/dispatch/verify/commit is the same job at every
  tier; the tier's intelligence lives in the pinned agents. T1 launches
  Fable @ xhigh only because there the session itself is the author.
- **Not changed:** T1 authorship/review pins, double-review protocol, all
  gates, queue post-checks, ORDER-GUARD.

## 7. Adoption checklist (owner)

- [ ] Start the next session with `/model opus` + `/effort medium` for the
      EPIC-02 T3/T4 tail (S-02-011/013/016–020).
- [ ] Use `/model` → Fable + `/effort xhigh` only when I flag a T1 story
      (next: S-02-012, S-02-021 contract, then EPIC-06/07/08).
- [ ] `/clear` between story batches (after `/rename`).
- [ ] Watch statusline `ctx %`; if it passes ~15% on a 1M window (~150k),
      finish the story and start fresh.
