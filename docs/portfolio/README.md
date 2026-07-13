<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# SS-SP Program Portfolio

*This directory is the single source of truth for what SS-SP is going to build, in what order, by whom, and how it traces back to the constitution.*

---

## Reading order

1. **`00_METHODOLOGY.md`** — how this portfolio is structured and why. **Read first.**
2. **`01_BRIEF.md`** — two-page executive summary of the program.
3. **`02_PRD.md`** — product requirements: personas, journeys, F- and NF- requirements, KPIs.
4. **`03_ARCHITECTURE.md`** — technical architecture: firmware stack, bearers, RNS/LXMF, cloud, HAL contracts.
5. **`04_BLUEPRINT.md`** — delivery blueprint: milestones M0..M9, workstreams, RACI, risks, rollout gates.
6. **`05_INFRASTRUCTURE_AND_SCALE.md`** — decision record: decentralized vs centralized, infrastructure tiers, scale engineering (NF-SCALE), revenue architecture.
7. **`06_COMPETITIVE_LANDSCAPE.md`** — decision record: competitor/adjacent-system analysis (adopt/covered/watch/skip), anti-fragmentation strategy.
8. **`07_FINAL_READINESS_TRIAGE.md`** — decision record: sole-source buildability audit, triage ledger of all findings, technology-currency register, final PASS verdict.
9. **`08_HALOW_TECHNOLOGY_DOSSIER.md`** — decision record: Wi-Fi HaLow deep research (standard, silicon tiers, stacks, regulatory) + binding decisions D-HALOW-01..07.
10. **`09_VENTURE_EXECUTION_MAP.md`** — decision record: the two-class execution model (buildable stories vs real-world venture actions), the VA-01..VA-28 register, milestone interleave, immediate queue.
11. **`10_MODEL_ALLOCATION_STRATEGY.md`** — decision record: MAX-vs-STD model/effort allocation across every epic, milestone, and task type, with tier definitions T1–T4, escalation triggers, and the near-term story-level queue.
12. **`11_TOKEN_ECONOMY.md`** — binding session-economy policy: session roles, model-switching rules, CI-wait policy, subagent output budgets.
13. **`12_TELEPHONY_AND_UNIVERSAL_INTEROP_DOSSIER.md`** — decision record: real-world telephony (PSTN/SMS/voice) + universal interop research, the Sovereignty Ladder, the Telephony Provider Interface, AI/ML division of labor, and the interop verdict matrix (ratifies D-0025).
14. **`EPIC_INDEX.md`** — catalog of all 24 epics.
15. **`epics/EPIC-NN-*/EPIC.md`** — one per epic, with sharded sub-outcomes and exit criteria.
16. **`epics/EPIC-NN-*/STORIES.md`** — every story in the epic.
17. **`STORIES_INDEX.md`** — master traceability ledger: story → shard → epic → PRD → constitution.

---

## Files

```
docs/portfolio/
├── 00_METHODOLOGY.md            # Methodology + Definition of Done
├── 01_BRIEF.md                  # Executive brief
├── 02_PRD.md                    # Requirements
├── 03_ARCHITECTURE.md           # Technical architecture
├── 04_BLUEPRINT.md              # Delivery blueprint (milestones, RACI, risks)
├── 05_INFRASTRUCTURE_AND_SCALE.md  # Decentralization decision, scale + revenue architecture
├── 06_COMPETITIVE_LANDSCAPE.md  # Competitor analysis + anti-fragmentation strategy
├── 07_FINAL_READINESS_TRIAGE.md # Sole-source buildability audit + triage ledger
├── 08_HALOW_TECHNOLOGY_DOSSIER.md  # Wi-Fi HaLow research + decisions D-HALOW-01..07
├── 09_VENTURE_EXECUTION_MAP.md  # Two-class execution model + VA-01..28 venture-action register
├── 10_MODEL_ALLOCATION_STRATEGY.md # MAX/STD model + effort allocation (tiers T1–T4)
├── 11_TOKEN_ECONOMY.md          # Session-economy policy (roles, model switching, budgets)
├── 12_TELEPHONY_AND_UNIVERSAL_INTEROP_DOSSIER.md  # Telephony/PSTN + universal interop research + D-0025
├── EPIC_INDEX.md                # 24-epic catalog
├── STORIES_INDEX.md             # Traceability matrix
├── README.md                    # This file
└── epics/
    ├── EPIC-01-governance-constitution/
    │   ├── EPIC.md
    │   └── STORIES.md
    ├── EPIC-02-firmware-foundation/…
    ├── … (24 epics)
    └── EPIC-24-compliance-business-support/
```

---

## Methodology in one paragraph

We use **BMAD-METHOD v6** as the backbone (enterprise scale, safety-critical, multi-team, audit-friendly), augmented with the **"constitution" concept from GitHub Spec Kit** (every epic must trace to a constitution clause key — C-00..C-08, C-OA, C-TM, C-CoC, C-SEC per `00_METHODOLOGY.md` §2.9; the underlying documents are the nine root docs `00_MASTER_SOFTWARE_PLAN.md`, `01_SS-SP_LITE_HARDWARE_REFERENCE.md`, `02_PROTOCOL_STACK.md`, `03_UI_LAYOUT_SPEC.md`, `04_LICENSING_AND_FORK_STRATEGY.md`, `05_SECURITY_MODEL.md`, `06_GOVERNANCE.md`, `07_BUSINESS_MODEL_AND_OPEN_SOURCE.md`, `08_UNIVERSAL_CONNECTIVITY.md`, plus `governance/OPEN_ASSURANCE.md`, `docs/TRADEMARK.md`, `CODE_OF_CONDUCT.md`, and `SECURITY.md`) and the **`spec → design → tasks → implementation` decomposition from AWS Kiro** applied inside each story. Full rationale and citations: `00_METHODOLOGY.md` §1.

---

## What "done" means for a story

See `00_METHODOLOGY.md` §5. Twelve gates, all must pass: AC in CI, ≥80 % coverage (mutation on security-critical), integration/HIL where applicable, docs updated, RFC ACCEPTED if wire-format changes, `wg-security` sign-off if security-relevant, `wg-legal` sign-off if licensing-relevant, compliance doc updated if regulatory-relevant, reproducible-build attestation regenerated, changelog entry, no unfiled `TODO`, two reviewer approvals (code + domain).

---

## Non-timeline rule

Portfolio artifacts contain ordering, dependencies, priorities, and sizes — **never calendar dates or duration estimates**. Turning ordering + dependencies + priorities into a calendar is a program-management concern handled in the project tracker.

---

## Scope statement — every layer covered

The 24 epics together cover:

- **Firmware layers:** RTOS baseline, HAL for three SKUs, crypto core, secure boot, OTA, SS-Link bearer, Reticulum, LXMF, Meshtastic compat, voice, UI framework, on-device applications, WASM plugin sandbox.
- **Hardware layers:** all boards (Lite/Alpha/Omega), all radios (LoRa, HaLow, Wi-Fi 2.4/5, BLE, cellular, satellite), all sensors (GNSS, IMU, baro, mag), secure element, tamper.
- **Product layers:** onboarding, apps, backup/restore, home-gateway mode, companion apps (iOS/Android/desktop/web), SDKs (5 langs).
- **Network layers:** Multi-bearer transport, RNS mesh, LXMF store-and-forward, cross-implementation interop, home-gateway propagation, cloud relay.
- **Business layers:** licensing (Apache-2.0 firmware, BSL 1.1 cloud), tiered SKU pricing, warranty/RMA, support portal, marketing, sales enablement, foundation transfer, trademark policing, certification program.
- **Governance layers:** constitution, RFC process, working-group charters, decisions log, DCO+CLA, two-approver merge, quarterly review.
- **Compliance layers:** FCC/CE/ISED/ACMA radio, UL/IEC safety, RoHS/REACH/WEEE environmental, GDPR/CCPA/PIPEDA/LGPD privacy, CRA SBOM & vuln SLA, EoL policy.
- **Testing layers:** unit, on-target, HIL, mesh simulator, fuzzer, chaos, 30-day soak, coverage/mutation gates.
- **Supply-chain layers:** reproducible builds, SLSA-3 provenance, CycloneDX SBOM, HSM signing, cosign, dep pin registry, weekly vuln scan.
- **Cloud layers:** Fleet Console, Relay, Provisioning Service, Plugin Registry, self-host packaging (Helm + compose).
- **Community/Ecosystem layers:** foundation transition plan, trademark program, certified-partner program, customer advisory board, bug bounty.

Nothing is left as a black box. Anything not covered here is either explicitly out of scope (see `02_PRD.md` §out-of-scope) or captured as an open question (`02_PRD.md` §open questions Q-01..Q-05).

---

## Cross-references

- Constitution proper: `../../00_MASTER_SOFTWARE_PLAN.md` through `../../08_UNIVERSAL_CONNECTIVITY.md` + `../../governance/OPEN_ASSURANCE.md`, `../../docs/TRADEMARK.md`, `../../CODE_OF_CONDUCT.md`, `../../SECURITY.md`.
- Decisions log: `../../governance/decisions.md`.
- RFCs: `../../rfcs/`.
- Repository skeleton: root of monorepo.

---

*This portfolio is CC-BY-4.0. Reuse encouraged with attribution.*
