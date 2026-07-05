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

- **MAX** — the most capable frontier model available (currently *Opus 4.8*): strongest reasoning, best at novel design, subtle correctness, adversarial thinking, long-horizon coherence. Highest cost per token; a scarce resource to be spent deliberately.
- **STD** — the standard workhorse model (currently *Fable 5*): fully competent for well-specified engineering, pattern-following implementation, refactoring, documentation, and test writing. Much cheaper; the default.

Spending MAX everywhere wastes budget; spending STD on the wrong things creates silent, expensive-to-find defects (T-22 proved that untested claims hide real bugs; a weak-model crypto bug would be far worse). This document fixes, in advance, **which work gets which tier**, so allocation is a governed decision rather than a per-session mood.

## 2. Capability tiers

| Tier | Resource recipe | When |
|------|-----------------|------|
| **T1 — MAX-critical** | MAX model, maximum reasoning effort, multi-pass (design → implement → independent MAX re-review), plus every applicable automated gate | Flaws are catastrophic, hard to detect, or irreversible |
| **T2 — MAX-designs / STD-builds** | MAX writes the contract (spec, header, state machine, test vectors, RFC); STD implements against it; MAX reviews the diff once | Hard design, routine construction |
| **T3 — STD-standard** | STD end-to-end; automated gates + spot human review | Well-specified work with strong verification |
| **T4 — STD-mechanical** | STD, minimal effort mode; gates only | Mechanical, fully checkable by tooling |

A tier is a *floor* for scrutiny, not a ceiling: anything may be escalated (§8), nothing on T1 may be demoted without a decisions-log entry.

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

Demotion is allowed only story-by-story, only one tier, and never out of T1 domains (§5 column 2 lists the floor).

## 9. Cost-efficiency playbook

1. **Contracts-first sequencing** buys the largest saving: one MAX-day of frozen headers/RFCs/vectors unlocks weeks of parallel STD implementation (M2 is designed around this).
2. **Batch T4 mechanically** — docs/index/lint chores in bulk STD sessions; the gates carry correctness.
3. **MAX reviews diffs, not files** — on T2 items MAX sees the contract + the diff, not the whole tree.
4. **Never use MAX to generate what a tool can verify** — regeneratable indexes, TOCs, formatting: tool + STD.
5. **Never use STD to check what only reasoning can catch** — STD self-review of crypto/races is a false economy; that is what T1 review passes are for.
6. **Spike-then-build** — for novel areas (HaLow, bearer scheduling) run a small MAX spike producing a design note; STD builds from the note. Cheaper than MAX building or STD flailing.
7. **Verification investment compounds** — every T1 hour spent on vectors/fuzzers/simulators permanently lowers the tier needed for all future work in that area. When in doubt, spend MAX on the *oracle*, not the code.

## 10. Review matrix

| Work produced by | Reviewed by | Gate |
|------------------|-------------|------|
| STD, T4 | Tooling only | lint-docs, index `--check`, CI build |
| STD, T3 | Tooling + STD self-review; MAX spot-samples ~1 in 5 in security-adjacent areas | CI + HIL where applicable |
| STD, T2 | **MAX diff review, once, against the contract** | CI + conformance vectors |
| MAX, T1 | **Independent MAX re-review in a fresh session** (no shared context anchoring) + all gates + human sign-off for wg-security paths per CONTRIBUTING §9 | KATs, vectors, fuzz soak |
| Anything touching `components/ss_crypto/**`, `bootloader/**`, `provisioning/**`, `ota/**`, `protocol/**` | MAX review regardless of author tier | 2 CODEOWNER + wg-security rule |

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

This document is re-validated at every milestone exit and whenever an escalation trigger (§8) fires twice in the same epic — the second firing means the epic's default tier is wrong; correct it here and record the change in `governance/decisions.md` if it moves work out of T1. Model names are examples of the current MAX/STD pairing; when the available models change, re-map the tiers to the new pair — the tier logic (§3) is model-agnostic and does not change.
