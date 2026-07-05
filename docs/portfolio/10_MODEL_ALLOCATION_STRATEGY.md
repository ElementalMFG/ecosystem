<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# 10 — Model & Effort Allocation Strategy

**Status:** Living operational doc · **Owner:** program lead · **Scope:** every epic, story, phase, and document in the program
**Companions:** `09_VENTURE_EXECUTION_MAP.md` (what humans must do), this doc (which AI capability tier executes what), `07_FINAL_READINESS_TRIAGE.md` (quality ledger).

## Table of contents

- [1. Purpose](#1-purpose)
- [2. Capability tiers](#2-capability-tiers)
- [3. Allocation principles — what actually drives tier choice](#3-allocation-principles--what-actually-drives-tier-choice)
- [4. The golden rule: contracts up, implementation down](#4-the-golden-rule-contracts-up-implementation-down)
- [5. Per-epic allocation (all 24 epics)](#5-per-epic-allocation-all-24-epics)
- [6. Phase view (milestones M0–M8)](#6-phase-view-milestones-m0m8)
- [7. Task-type view (cross-cutting)](#7-task-type-view-cross-cutting)
- [8. Escalation triggers (dynamic re-tiering)](#8-escalation-triggers-dynamic-re-tiering)
- [9. Cost-efficiency playbook](#9-cost-efficiency-playbook)
- [10. Review matrix](#10-review-matrix)
- [11. Near-term concrete assignments (M0–M1 queue)](#11-near-term-concrete-assignments-m0m1-queue)
- [12. Maintenance rule](#12-maintenance-rule)

## 1. Purpose

The program has two AI execution resources with different capability and cost profiles:

- **MAX** — **Claude Fable 5** (`claude-fable-5`): Anthropic's most capable widely released model (Mythos-class, *above* the Opus class). Strongest reasoning, best at novel design, subtle correctness, adversarial thinking, long-horizon coherence. Highest per-token cost; a scarce resource to be spent deliberately.
- **STD** — **Claude Opus 4.8** (`claude-opus-4-8`): the previous frontier tier, still an excellent engineer. Fully competent for well-specified engineering, pattern-following implementation, refactoring, documentation, and test writing. Half the per-token price; the volume workhorse.

Spending MAX everywhere wastes budget; spending STD on the wrong things creates silent, expensive-to-find defects (T-22 proved that untested claims hide real bugs; a weak-model crypto bug would be far worse). This document fixes, in advance, **which work gets which tier and which effort setting**, so allocation is a governed decision rather than a per-session mood.

### 1.1 The two models — researched facts (as of 2026-07)

| Property | Claude Fable 5 (MAX) | Claude Opus 4.8 (STD) |
|---|---|---|
| Class / release | Mythos-class, released 2026-06-09 | Opus-class, released 2026-05-28 |
| API price (per Mtok in/out) | **$10 / $50** (2× Opus; 90% prompt-cache discount; no long-context surcharge) | **$5 / $25** (prompt-cache 90%, batch 50%; long-context surcharge >200k) |
| Context / output | 1M ctx default, 128k output | 1M ctx, comparable output |
| SWE-bench Pro | **80.3%** (11 pts ahead of next best) | 69.2% |
| SWE-bench Verified | (state-of-the-art on nearly all tested benchmarks) | 88.6% |
| Effort levels | low / medium / high (default) / xhigh / max — the **primary** cost control | Same five levels; guidance says start coding at xhigh |
| Thinking | Adaptive thinking always on; cannot be disabled; raw CoT never returned | Adaptive thinking; effort controls depth |
| Distinctive strengths | Long-horizon autonomous work (50M-line-codebase migration in a day); deeper knowledge; FrontierCode: highest among frontier models **even at medium effort** | ~4× less likely than its predecessor to let flaws in its own code pass unremarked — an unusually good *reviewer* |
| Safeguards | Safety classifiers may refuse (<5% of sessions; cybersecurity, bio/chem, distillation topics) with `stop_reason: "refusal"`; server/client fallback to Opus 4.8 supported; refused-before-output requests are not billed | No comparable classifier layer |
| Data retention | Mandatory 30-day retention; **no ZDR** | Standard retention options |
| Availability history | Suspended worldwide 2026-06-12 → restored 2026-07-01 (US export-control directive) | Continuously available |
| Subscription accounting | Counts **2× usage** on Claude.ai plans | 1× |

### 1.2 The effort dial — why this is a 2-D problem, not a model choice

Both models expose an `effort` parameter (`low`/`medium`/`high`/`xhigh`/`max`) that scales **all** token spend — thinking depth, tool-call count, and verbosity. Effort is a behavioral signal, not a hard budget: at low effort the model still thinks on genuinely hard problems, just less. Allocation is therefore a **(model, effort)** pair, and the two dials interact:

- Anthropic's own guidance: on Fable 5, "lower effort settings still perform well and **often exceed xhigh performance on prior models**." Cognition's FrontierCode confirms Fable 5 leads the frontier field *at medium effort*.
- Opus 4.8's guidance is the mirror image: **start at xhigh** for coding/agentic work (meaningfully more tokens than high) to get its best results.

### 1.3 Cost economics — the parity point

Per-token, Fable 5 is exactly 2× Opus 4.8. But total job cost = price × tokens, and effort moves tokens a lot (a single benchmarked task ranged ~7× in cost from low to max effort on Fable 5). Two consequences:

1. **Fable-5-at-reduced-effort ≈ Opus-4.8-at-raised-effort in total cost.** Fable 5 at `medium` spends materially fewer tokens than Opus 4.8 at its recommended `xhigh`; at roughly half the tokens the 2× price cancels and the jobs cost about the same — while Fable 5 retains a capability edge. So for T2/T3 engineering, **Fable 5 @ medium is usually the better buy than Opus 4.8 @ xhigh at similar spend**, capacity permitting.
2. **Opus 4.8 still wins on absolute floor cost.** For T4 mechanical work and high-volume T3 breadth, Opus 4.8 @ low/medium is unbeatable on price, and batch processing (another 50%) applies.

Rule of thumb: **tune effort before switching model.** Only drop from Fable 5 to Opus 4.8 when (a) the work is T4/verification-saturated, (b) Fable capacity/credits are constrained, or (c) you want Opus as an independent cross-reviewer (§10).

### 1.4 Operational constraints that shape allocation

1. **Refusal exposure is concentrated in *our* T1 domains.** Fable 5's classifiers trigger mainly on cybersecurity content — which is exactly EPIC-06/07/08/18, the fuzzer grammars, and threat-model work. Mitigations: frame prompts explicitly as defensive/own-product security engineering; handle `stop_reason: "refusal"` (HTTP 200, not an error); use server-side `fallbacks` or client retry to Opus 4.8 (fallback credit avoids double prompt-cache cost). A refused T1 task **must not** silently complete on the fallback model — it re-runs on Fable 5 with reframed context, or is consciously re-tiered.
2. **Single-model dependency is a real risk.** Fable 5 was export-control-suspended for ~3 weeks in June 2026. The plan must degrade gracefully: every T1 recipe has an Opus-4.8-only contingency (max effort, +1 extra independent review pass, slower cadence) so the program never hard-blocks on Fable availability.
3. **Data retention.** Fable 5 sessions carry mandatory 30-day retention with no ZDR. For this open-source program that is acceptable for nearly everything; for pre-publication vulnerability details or key-ceremony secrets, prefer processes that never place raw secrets in any model context (which `05_SECURITY_MODEL.md` requires anyway).
4. **Capacity/credits.** Fable 5 demand is high and it costs 2× on subscription plans; treat Fable-hours as the scarce budgeted resource this document allocates.

Sources: Anthropic launch posts and platform docs for [Fable 5 / Mythos 5](https://www.anthropic.com/news/claude-fable-5-mythos-5), [Opus 4.8](https://www.anthropic.com/news/claude-opus-4-8), the [model introduction](https://platform.claude.com/docs/en/about-claude/models/introducing-claude-fable-5-and-claude-mythos-5) and [effort parameter](https://platform.claude.com/docs/en/build-with-claude/effort) docs, and third-party analyses (Simon Willison's [initial impressions](https://simonwillison.net/2026/Jun/9/claude-fable-5/), Vellum/TrueFoundry benchmark write-ups).

## 2. Capability tiers

Each tier now specifies a **(model @ effort)** recipe, plus the contingency used if Fable 5 is unavailable (§1.4.2).

| Tier | Primary recipe | Contingency (Fable unavailable) | When |
|------|----------------|--------------------------------|------|
| **T1 — MAX-critical** | **Fable 5 @ xhigh** (max for the single hardest artifacts), multi-pass: design → implement → **independent fresh-session Fable 5 @ high re-review** → **Opus 4.8 @ xhigh cross-review** (its flaw-spotting strength), plus every applicable automated gate | Opus 4.8 @ max, **two** independent review passes, slower cadence; no T1 merge skips the extra pass | Flaws are catastrophic, hard to detect, or irreversible |
| **T2 — MAX-designs / STD-builds** | **Fable 5 @ high** writes the contract (spec, header, state machine, test vectors, RFC); implementation on **Fable 5 @ medium** *or* **Opus 4.8 @ xhigh** (≈ cost parity, §1.3 — pick by capacity); **Fable 5 @ medium** reviews the diff once | Opus 4.8 @ xhigh designs; Opus 4.8 @ high builds; extra review pass | Hard design, routine construction |
| **T3 — STD-standard** | **Fable 5 @ low–medium** *or* **Opus 4.8 @ high** end-to-end; automated gates + spot human review | Opus 4.8 @ high (no change) | Well-specified work with strong verification |
| **T4 — STD-mechanical** | **Opus 4.8 @ low–medium**, batched (cheapest floor, §1.3.2); gates only | No change | Mechanical, fully checkable by tooling |

A tier is a *floor* for scrutiny, not a ceiling: anything may be escalated (§8), nothing on T1 may be demoted without a decisions-log entry. Within a tier, **tune effort before switching model** (§1.3).

## 3. Allocation principles — what actually drives tier choice

Tier is determined by scoring five properties. High score on **any one** of the first three forces T1/T2.

1. **Irreversibility & blast radius.** Can a mistake be recalled? Wire formats, key hierarchies, eFuse burns, partition tables, anti-rollback counters, published APIs, and license/legal text are frozen once shipped → highest tier. A UI screen or internal helper can be patched next release → lower tier.
2. **Detection difficulty.** Will existing gates catch a subtle error? Crypto that "works" but leaks nonce reuse, a race that fires 1-in-10⁴ boots, an off-by-one in an interop bit field — tests pass, field fails → high tier. A broken link, failing build, or wrong pin number fails loudly in CI/HIL → lower tier.
3. **Adversarial pressure.** Is a motivated attacker searching for the flaw? Everything wg-security touches (EPIC-06/07/08/18, OTA trust chain, duress PIN) is written *against* an adversary → T1 design and review, always.
4. **Novelty & ambiguity.** Is there a well-trodden pattern? RNS-over-HaLow bearer scheduling and LXMF↔Meshtastic bridging have no reference implementation → high tier. A GATT provisioning service, an I2C sensor driver, a REST endpoint → pattern-following, low tier.
5. **Verification leverage (the discount factor).** Strong external verification *lowers* the needed generation tier: if conformance vectors, KATs, fuzzers, golden files, or the compiler will catch errors cheaply, STD may build it. **Corollary: the vectors, KATs, and fuzz harness designs themselves are T1/T2 — they are the thing everything else leans on.**

## 4. The golden rule: contracts up, implementation down

> **MAX writes and reviews contracts; STD fills them in.**

Concretely: MAX authors the HAL headers, wire-format RFCs, state-machine diagrams, threat-model deltas, KAT/conformance vectors, and exhaustion/failure semantics. STD then writes the `.c` files, bindings, tests-to-vectors, docs, and glue — work whose correctness the contract plus CI can adjudicate. MAX returns once to review the diff on T2 items. This inverts naive usage (cheap drafts, expensive cleanup), which is the most expensive pattern of all: a weak contract poisons every downstream story.

## 5. Per-epic allocation (all 24 epics)

| Epic | Default | Escalated shards / examples | Rationale |
|------|---------|------------------------------|-----------|
| 01 governance-constitution | **T3** | T2: RFC drafting with legal/irreversibility weight (foundation transfer, license changes, S-01-017 deprecation policy). T4: charters, templates, dashboards | Text is amendable pre-ratification; ratified/legal text is not |
| 02 firmware-foundation | **T3** | **T1:** S-02-012 brown-out save-state atomicity, S-02-008 panic-handler correctness (crash-loop guard), S-02-021 pool-allocator contract + exhaustion semantics. T2: S-02-016 safe-mode/recovery path, S-02-017 NVS versioning/migrations | Mostly scaffolding with loud failures, but the power/persistence corners are silent-corruption territory |
| 03 hal-lite | **T3** | **T2:** shard E SX1262 driver (IRQ ordering, FIFO races, LBT, duty-cycle guard — R03-01), shard A power/sleep sequencing (25 mA/0.5 mA exit criteria), Wi-Fi/BLE coex config (R03-02). T4: LEDs, sensor stubs, button matrix | Datasheet-driven driver work is STD's bread and butter; radio IRQ paths and power states hide races |
| 04 hal-alpha | **T3** | **T2:** MM8108 HaLow bring-up (novel vendor SDK, SDIO-vs-USB spike S-04-023, TWT/RAW power save S-04-024), FEM power-table/regulatory limits | HaLow is the least-trodden hardware path in the program |
| 05 hal-omega | **T3** | T2: cellular/LEO modem power + cost-metered bearer policy | Follows Alpha patterns once they exist |
| 06 crypto-core | **T1 (all)** | Nothing demotable except: T3 for test *runners* once MAX-authored KATs exist; T4 for docs formatting | Double Ratchet, sender keys, hybrid PQ (D-0007), constant-time review — maximum adversarial pressure, minimum detectability |
| 07 identity-provisioning | **T1 (all)** | T3: provisioning-line UI/tooling glue around a frozen ceremony spec | RelKey hierarchy and key ceremony are unrecallable once devices ship |
| 08 secure-boot-flash-enc | **T1 (all)** | — | eFuse burns and anti-rollback are literally irreversible in silicon |
| 09 ota-update-system | **T2** | **T1:** manifest format, dual-signature verification logic, rollback/anti-rollback interaction, staged-rollout kill-switch semantics. T3: delta codec integration, transport plumbing | The trust chain is T1; the plumbing around it is not |
| 10 ss-link-bearer | **T2** | **T1:** SS-Link frame wire format RFC + version negotiation (frozen forever), QoS pre-emption semantics. T3: individual bearer plugin adapters against the frozen plugin API | Wire format = irreversible; plugins = pattern-per-slot |
| 11 reticulum-integration | **T2** | **T1:** any deviation/extension touching RNS wire behaviour; interface MTU/fragmentation mapping. T3: porting/adapting well-documented RNS semantics to C, GET/PUT plumbing | Upstream protocol is documented; our novel seams are not |
| 12 lxmf-messaging | **T2** | **T1:** store-and-forward consistency + retry/dedup state machine (silent message loss is the product failing at its one job). T3: receipts, priority queues against the frozen state machine | Delivery correctness is detection-hard |
| 13 meshtastic-compat | **T1 (design+vectors), T2 (impl)** | T3: nothing until the golden-vector corpus exists, then bit-field encoders against vectors | Clean-room legal constraint (D-0002) + bit-level interop with a moving foreign target — both precision-critical |
| 14 voice-subsystem | **T3** | T2: half-duplex bearer arbitration + PTT/jitter timing (real-time races); codec integration itself is T3 | Codecs are libraries; arbitration is a timing problem |
| 15 ui-framework | **T3** | T2: layout-engine core + screen-graph lifecycle (everything downstream leans on it); T4: themes, icons, translations | One-time framework core, then pattern application |
| 16 application-layer | **T3** | **T1/T2: SOS + duress-PIN flows** (safety-critical UX + security semantics, per `05_SECURITY_MODEL.md`); rest of apps T3 | Messages/Contacts/Settings are standard app work |
| 17 home-gateway | **T2** | T1: internet-bridge security boundary (the one place the mesh meets the internet). T3: captive portal, AP setup | Bridging/throttling logic is novel; setup UX is not |
| 18 plugin-sandbox | **T1 (permission model, quota, WASM boundary)** | T3: manifest tooling, registry plumbing, sample plugins | A sandbox escape defeats the entire security story |
| 19 companion-apps | **T3** | T2: BLE pairing + key-backup/restore crypto UX (T1 review on the key-handling paths); T4: store listings, screenshots, boilerplate | Standard mobile/desktop dev against a frozen GATT/pairing spec |
| 20 sdks | **T3** | T2: C SDK API surface design (ABI frozen once published; other languages wrap it); wire testkit *vectors* T1 per §3.5 | Bindings against testkits are exactly the verification-leverage case |
| 21 cloud-services | **T3** | T2: relay quota/token format + fleet-console authz model; T1 review on anything touching user keys/metadata privacy | CRUD + infra is commodity; authz and privacy are not |
| 22 testing-simulation | **T2** | **T1: protocol-fuzzer grammar + mesh-simulator fidelity model** (a wrong simulator validates wrong behaviour program-wide); T3: unit-test authoring, HIL rack scripts; T4: coverage plumbing | The test oracle is the highest-leverage artifact in the repo |
| 23 ci-cd-supply-chain | **T2** | T1: signing pipeline + SLSA provenance + SBOM attestation design (supply-chain attack surface, RFC-0002 lineage); T3/T4: workflow YAML mechanics | CI mechanics fail loudly; trust mechanics fail silently |
| 24 compliance-business-support | **T3** | T2: FCC/CE/CRA filing content + RF-exposure claims (regulatory text is expensive to get wrong, human counsel reviews anyway); T4: support portal, RMA templates, marketing drafts | Human experts (VA-mapped) are the real gate; AI prepares |

## 6. Phase view (milestones M0–M8)

| Phase | Character | Tier mix | Guidance |
|-------|-----------|----------|----------|
| **M0** (done/closing) | Governance + scaffolding | ~10% T1/T2, 90% T3/T4 | Exactly the drift-prone doc work T-23 caught; STD + strong lint gates is correct. Fuzzer/simulator *design* (EPIC-22) is the M0-window T1 exception — do it early, everything leans on it |
| **M1** | Lite HAL + **crypto/identity/secure-boot** + UI core | **Peak T1 concentration of the whole program** | Budget MAX here above all: EPIC-06/07/08 in full, SX1262 shard, layout-engine core, HAL header contracts. Do *not* rush M1 security epics with STD to "make progress" — this is the phase where flaws become permanent |
| **M2** | Protocol tower (SS-Link, RNS, LXMF, Meshtastic-compat, OTA, apps) | T1 on wire formats/RFCs + state machines; T3 breadth for app layer | Front-load every RFC (MAX), then wide parallel STD implementation against frozen contracts — the golden rule's showcase phase |
| **M3** | Compliance + first-ship hardening | T2 filings, T1 for any security fix, T3 everything else | Escalation triggers (§8) matter more than plans here |
| **M4** | Alpha HAL + companion apps | T2 HaLow bring-up; T3 apps breadth | Two very different tracks; don't let the app track steal MAX from HaLow |
| **M5** | Voice + home gateway | T2 arbitration + bridge boundary; T3 rest | |
| **M6** | Plugins, SDKs, cloud | T1 sandbox; T3 breadth elsewhere | Highest parallel-STD throughput phase |
| **M7–M8** | Omega + LTS maturity | T3 dominant | Patterns all exist; MAX reserved for review + incidents |

## 7. Task-type view (cross-cutting)

| Task type | Tier | Notes |
|-----------|------|-------|
| Wire formats, public APIs/ABIs, capability flags | **T1** | Frozen forever; RFC-gated (RFC-0001) |
| Cryptography, key handling, trust chains | **T1** | No exceptions, incl. review of T3 code that merely *calls* crypto |
| Threat modelling, security review passes | **T1** | MAX as adversary is its highest-value use |
| Conformance vectors, KATs, fuzzer grammars, simulator models | **T1/T2** | The verification bedrock (§3.5) |
| Novel state machines (delivery, rollout, arbitration, bearer scheduling) | **T1 design / T2 impl** | Silent-failure territory |
| Concurrency/IRQ/power-state code | **T2** | Races evade tests |
| Device drivers from datasheets | **T3** | Escalate radio + power shards |
| App/UI/screens, SDK bindings, cloud CRUD | **T3** | Strong gates exist |
| Unit tests against existing contracts | **T3** | |
| Docs, TOCs, indexes, changelogs, templates, translations, scaffolds | **T4** | Fully lintable (`make lint-docs`, index `--check`) |
| Legal/regulatory final text | **T2 + mandatory human counsel** | AI prepares, humans sign (VA register) |

## 8. Escalation triggers (dynamic re-tiering)

Re-tier a story **upward immediately** when any of these fire mid-flight:

1. **Second failed attempt** — STD produced two rework rounds on the same story → hand the whole story to MAX (rework churn is costlier than MAX).
2. **Surprise discovery** — implementation contradicts the contract/constitution (a T-22-class event) → MAX investigates root cause before anyone codes further.
3. **Scope creep into a T1 domain** — a T3 story turns out to touch keys, wire bytes, eFuses, persistence atomicity, or the sandbox boundary → stop, escalate.
4. **Novel deviation from upstream** — any point where we diverge from documented Reticulum/LXMF/Meshtastic/ESP-IDF behaviour.
5. **Reviewer unease** — a MAX review flags "correct but fragile" → the *next* story in that area starts at T2.
6. **Incident/regression in the field or HIL soak** — root-cause analysis is always T1.
7. **Classifier refusal on a T1/T2 task** — not a demotion event: reframe and re-run on Fable 5 per §1.4.1. Only a *repeated* refusal after defensive-context reframing triggers a conscious, logged decision on how the work proceeds.

Demotion is allowed only story-by-story, only one tier, and never out of T1 domains (§5 column 2 lists the floor). **Effort demotion inside a tier** (e.g. Fable 5 high → medium on a T2 build) is allowed freely when evals/gates hold — that is the intended §1.3 savings lever.

## 9. Cost-efficiency playbook

1. **Contracts-first sequencing** buys the largest saving: one MAX-day of frozen headers/RFCs/vectors unlocks weeks of parallel STD implementation (M2 is designed around this).
2. **Batch T4 mechanically** — docs/index/lint chores in bulk STD sessions; the gates carry correctness.
3. **MAX reviews diffs, not files** — on T2 items MAX sees the contract + the diff, not the whole tree.
4. **Never use MAX to generate what a tool can verify** — regeneratable indexes, TOCs, formatting: tool + STD.
5. **Never use STD to check what only reasoning can catch** — STD self-review of crypto/races is a false economy; that is what T1 review passes are for.
6. **Spike-then-build** — for novel areas (HaLow, bearer scheduling) run a small MAX spike producing a design note; STD builds from the note. Cheaper than MAX building or STD flailing.
7. **Verification investment compounds** — every T1 hour spent on vectors/fuzzers/simulators permanently lowers the tier needed for all future work in that area. When in doubt, spend MAX on the *oracle*, not the code.
8. **Effort before model** — the cheapest capability downgrade is Fable 5 at a lower effort, not a model switch (§1.3): it keeps Fable's knowledge depth and long-horizon coherence while cutting tokens. Reserve the model switch for the T4 floor and volume batches.
9. **Exploit prompt caching and batch pricing** — 90% cached-input discount on both models rewards stable, contract-shaped prompts (another reason contracts-first wins); Opus 4.8 batch mode halves T4 bulk work again.
10. **Cross-model review is nearly free insurance** — an Opus 4.8 @ xhigh diff review costs a fraction of the Fable generation it checks, and Opus 4.8 is measurably strong at flagging flaws in code; use it as the second pair of eyes on every T1 merge (§2).

## 10. Review matrix

| Work produced by | Reviewed by | Gate |
|------------------|-------------|------|
| T4 (Opus 4.8 @ low–med) | Tooling only | lint-docs, index `--check`, CI build |
| T3 (Fable 5 @ low–med or Opus 4.8 @ high) | Tooling + self-review; Fable 5 @ medium spot-samples ~1 in 5 in security-adjacent areas | CI + HIL where applicable |
| T2 build (Fable 5 @ med or Opus 4.8 @ xhigh) | **Fable 5 @ medium diff review, once, against the contract** | CI + conformance vectors |
| T1 (Fable 5 @ xhigh/max) | **Independent Fable 5 re-review in a fresh session** (no shared context anchoring) + **Opus 4.8 @ xhigh cross-review** (model diversity; Opus is a demonstrably strong flaw-spotter) + human sign-off for wg-security paths per CONTRIBUTING §9 | KATs, vectors, fuzz soak |
| Anything touching `components/ss_crypto/**`, `bootloader/**`, `provisioning/**`, `ota/**`, `protocol/**` | Fable 5 review regardless of author tier | 2 CODEOWNER + wg-security rule |

## 11. Near-term concrete assignments (M0–M1 queue)

The immediate story queue, tiered:

| Story | Tier | Assignment |
|-------|------|------------|
| S-02-003..005 board_config headers | T3 | STD from datasheets; parity CI checks verify |
| S-02-006 FreeRTOS config | T3 | STD; D-0006 fixes the decisions |
| S-02-007 ss_log + redaction | T3, **T2 for the `%k` redaction guarantee + CI proof** | Redaction is a security promise (NF-SEC-03) |
| S-02-008 panic handler | **T1** | Crash-loop guard + flash-dump correctness |
| S-02-009 watchdogs | T3 | |
| S-02-010 boot budget | T4 | Instrumentation + CI assert |
| S-02-011 heap/stack tracker | T3 | |
| S-02-012 brown-out save-state | **T1** | Persistence atomicity under power loss |
| S-02-013 component scaffold | T4 | |
| S-02-014/015 test frameworks | T2 design / T3 build | Test oracle rule (§3.5) |
| S-02-016 safe-mode recovery | T2 | Anti-rollback interaction reviewed at T1 |
| S-02-017 NVS versioning | T2 | Migration correctness |
| S-02-018 RTC/clock source | T3 | |
| S-02-019 SBOM attestation | T2 | Supply-chain (EPIC-23 lineage) |
| S-02-020 version resource | T4 | |
| S-02-021 pool allocator | **T1 contract / T2 impl** | Exhaustion semantics per AC |
| EPIC-03 shard E (SX1262) | T2 | MAX design note first (spike, §9.6) |
| EPIC-03 shards B/I/J | T3/T4 | |
| EPIC-06/07/08 (all, M1) | **T1** | The program's MAX budget priority |
| ss_hal header contracts (pre-EPIC-03) | **T1** | Golden rule: contracts first |

## 12. Maintenance rule

This document is re-validated at every milestone exit and whenever an escalation trigger (§8) fires twice in the same epic — the second firing means the epic's default tier is wrong; correct it here and record the change in `governance/decisions.md` if it moves work out of T1. The concrete facts in §1.1–§1.4 (pricing, benchmarks, effort behavior, availability, refusal rates) are a snapshot dated 2026-07 and must be re-researched whenever either model is superseded or repriced; the tier logic (§3) is model-agnostic and does not change — only the (model @ effort) recipes in §2 get re-mapped.
