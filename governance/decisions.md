<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# Steering Committee Decisions Log

Chronological, append-only. Each entry is signed by the Chair at close.

## Schema, counter, and quorum

- **Schema.** Every entry is `## D-NNNN — <title>` followed by `- Date:` (ISO date, or `2025-Q? (bootstrap)` for pre-log decisions reconstructed at bootstrap), `- Decision:` (normative text), optional `- Rationale:`, optional `- Status:` (`RECORDED` default; `SUPERSEDED by D-NNNN` when overturned), and `- Reference:` (constitution docs, RFCs, or stories).
- **Counter.** `NNNN` is a zero-padded monotonically increasing integer. Numbers are assigned at append time and never reused. Next free number: **D-0020**.
- **Quorum.** Steady state: a decision closes with a simple majority of the steering committee, minimum two-thirds attendance, recorded by the Chair. Bootstrap (before S-01-009 staffs the working groups): the program lead records decisions unilaterally, each such entry is implicitly `(bootstrap)`, and all bootstrap decisions are re-ratified at the first quarterly constitutional review (S-01-016). RFC acceptances always get a corresponding entry (RFC-0001 §Detailed design).
- **Immutability.** Entries are append-only; corrections are made by a superseding entry, never by editing history.

---

## D-0001 — Program launch

- Date: 2025-Q?  (bootstrap)
- Decision: Establish SS-SP program under founding vendor entity. Adopt Apache-2.0 for firmware/apps, CC-BY 4.0 for spec docs.
- Status: RECORDED
- Reference: `04_LICENSING_AND_FORK_STRATEGY.md`, `06_GOVERNANCE.md`

## D-0002 — No Meshtastic firmware fork

- Date: 2025-Q?
- Decision: SS-SP does not fork or link against the Meshtastic firmware or `.proto` files. Interoperability with Meshtastic is achieved via clean-room wire-format reimplementation under Apache-2.0.
- Rationale: GPL-3.0 viral licensing incompatible with commercial-sale device shipping with secure boot, and with paid enterprise extensions.
- Reference: `04_LICENSING_AND_FORK_STRATEGY.md` §3

## D-0003 — Adopt Reticulum / LXMF under Reticulum License

- Date: 2025-Q?
- Decision: Adopt Reticulum Network Stack and LXMF as core mesh substrate. Honour the "no-harm" and "no-AI-training" clauses in TOS, AUP, and data-processing architecture.
- Reference: `04_LICENSING_AND_FORK_STRATEGY.md` §4

## D-0004 — First target device: Lite (Elecrow CrowPanel Advance 3.5" HMI)

- Date: 2025-Q?
- Decision: Lite v1 is the first commercial SKU. Pin map derived from Meshtastic firmware `variants/esp32s3/elecrow_panel/` and Elecrow-RD schematic.
- Reference: `01_SS-SP_LITE_HARDWARE_REFERENCE.md`

## D-0005 — Founding security posture

- Date: 2025-Q?
- Decision: Adopt security-model baseline in `05_SECURITY_MODEL.md`. Dual-signed OTA, per-device seed, no key escrow that decrypts user messages, duress PIN with beacon.
- Reference: `05_SECURITY_MODEL.md`

## D-0006 — RTOS baseline: ESP-IDF FreeRTOS behind the `ss_hal` abstraction

- Date: 2026-07-04 (bootstrap)
- Decision: The firmware RTOS baseline is the FreeRTOS kernel as shipped in the pinned ESP-IDF release (tick 1 kHz, priority-ceiling policy, stack-overflow hooks per S-02-006). All application and protocol code binds to `firmware/components/ss_hal` contracts, not to FreeRTOS or ESP-IDF APIs directly, so future non-ESP32 SKUs can substitute another kernel without touching the upper layers.
- Rationale: ESP-IDF's FreeRTOS is the only vendor-supported option with Wi-Fi/BLE coexistence on ESP32-S3; the HAL boundary preserves portability without paying an abstraction tax twice.
- Status: RECORDED
- Reference: `00_MASTER_SOFTWARE_PLAN.md` §HAL contracts, EPIC-02 (S-02-006), `docs/portfolio/03_ARCHITECTURE.md`

## D-0007 — Post-quantum posture: classical now, hybrid-ready capability flags

- Date: 2026-07-04 (bootstrap)
- Decision: v1 ships classical cryptography (X25519 key agreement, Ed25519 signatures, AES/ChaCha AEADs) as required for Reticulum/LXMF interoperability. Wire formats and identity records MUST carry crypto-suite capability flags so a hybrid suite (X25519+ML-KEM-768; Ed25519+ML-DSA for signatures) can be introduced by RFC without a breaking flag-day. No bespoke or pre-standard PQ scheme ships before an RFC adopts a NIST-standardized suite.
- Rationale: Reticulum compatibility fixes the classical baseline today; reserving negotiation space now is cheap, retrofitting it later is a wire-format break.
- Status: RECORDED
- Reference: `05_SECURITY_MODEL.md`, `02_PROTOCOL_STACK.md`, EPIC-06 (crypto core), S-06-016 (suite negotiation)

## D-0008 — Delivery methodology: sharded epic/story portfolio

- Date: 2026-07-04 (bootstrap)
- Decision: The program is delivered through the sharded epic/story portfolio methodology defined in `docs/portfolio/00_METHODOLOGY.md`: constitution docs are the single source of truth, epics carry shards/exit-criteria/RACI, stories carry machine-parsed Meta lines, and `tools/gen-stories-index.py --check` gates index drift in CI.
- Rationale: Machine-parseable planning artifacts let validation catch orphaned requirements, phantom PRD ids, and status drift automatically — the same rigor applied to code is applied to plans.
- Status: RECORDED
- Reference: `docs/portfolio/00_METHODOLOGY.md`, `docs/portfolio/STORIES_INDEX.md`

## D-0009 — Toolchain pinning by container digest

- Date: 2026-07-04 (bootstrap)
- Decision: Official firmware artifacts are built only inside the digest-pinned ESP-IDF container (currently ESP-IDF v5.3.5, digest recorded in `ci/containers/firmware/Dockerfile`); upgrades follow the reviewed procedure in RFC-0002. Tag-only pins are prohibited.
- Rationale: Digest pins are immutable; the policy was validated empirically when the first containerized build surfaced defect T-22 that untested claims had hidden.
- Status: RECORDED
- Reference: `rfcs/0002-toolchain-pinning-policy.md`, S-02-001, `docs/portfolio/07_FINAL_READINESS_TRIAGE.md` (T-22)

## D-0010 — Two-class execution model: buildable stories vs venture actions

- Date: 2026-07-04 (bootstrap)
- Decision: Program execution is tracked in two disjoint classes. Class 1: the 24-epic story portfolio — everything buildable and verifiable inside the repository. Class 2: real-world venture actions (VA-01..VA-28 in `docs/portfolio/09_VENTURE_EXECUTION_MAP.md`) — accounts, hardware purchases, keys, legal filings — which only a human can perform. Stories whose acceptance criteria depend on a venture action are gated on that VA id rather than silently faked.
- Rationale: Keeps repo status honest — a story is DONE only when its artifacts exist and verify; external-world prerequisites are named, owned, and visible instead of implied.
- Status: RECORDED
- Reference: `docs/portfolio/09_VENTURE_EXECUTION_MAP.md`, `docs/portfolio/07_FINAL_READINESS_TRIAGE.md`

## D-0011 — Development hosting: GitHub org ElementalMFG, repo `ecosystem`, public

- Date: 2026-07-05 (bootstrap)
- Decision: The development repository of record is `https://github.com/ElementalMFG/ecosystem` — GitHub (all `.github/` artifacts stay valid), organization account, public visibility. It is treated as the real/production repository for the duration of development; a brand-named org/repo MAY replace it post-development by explicit superseding decision, with full history migrated.
- Rationale: GitHub was chosen because CI workflows, Dependabot, CODEOWNERS, issue/PR templates, and the planned CLA bot are already written for it; an org (not a personal account) matches the later foundation-transition plan. Recorded from the owner's answers in `docs/OWNER_DECISIONS.md` (A1–A4).
- Status: RECORDED
- Reference: VA-02 in `docs/portfolio/09_VENTURE_EXECUTION_MAP.md`, S-01-008, S-01-012, `docs/OWNER_DECISIONS.md`

## D-0012 — Parent entity Elemental MFG; interim domain/channels on elementalmfg.com

- Date: 2026-07-05 (bootstrap)
- Decision: Elemental MFG (registered and active in Idaho, USA) is the parent/inception company for the SS-SP program and the intended owner of the relevant IP. Until product naming settles (Seekie-Speakie and/or Smart-Pager) and a brand domain (`.org`/`.io` or subdomain) is registered, all program contact runs on the existing `elementalmfg.com` domain: security intake `security@elementalmfg.com`, developer contact `dev@elementalmfg.com`, general/support `support@elementalmfg.com`, conduct and trademark matters to the maintainer (`dylan@elementalmfg.com` / `support@elementalmfg.com`). Brand-domain registration (`ss-sp.org` etc., VA-05) is DEFERRED, and every `*.ss-sp.org` address/site in the docs is read as "brand domain — pending D-0012 supersession". Community channels: GitHub Discussions on the D-0011 repo replaces the mailing list and forum for now; a Matrix room `#ss-sp:matrix.org` is created now, IRC later. DCO sign-off identity for the founding maintainer moves to `dylan peterson <dylan@elementalmfg.com>`.
- Rationale: Uses live, real infrastructure instead of publishing dead addresses; keeps the brand decision open without blocking security intake, community channels, or DCO identity. Recorded from the owner's answers in `docs/OWNER_DECISIONS.md` (B1–B5, C2, E1, info items 3–4).
- Status: RECORDED
- Reference: `SECURITY.md`, `CODE_OF_CONDUCT.md`, `CONTRIBUTING.md` §12, `docs/TRADEMARK.md`, VA-05, `docs/OWNER_DECISIONS.md`

## D-0013 — Dev-fleet hardware in hand; fully-loaded Lite is the spec-lock target

- Date: 2026-07-05 (bootstrap)
- Decision: The program holds **2× Elecrow CrowPanel Advance 3.5″ ESP32-S3 boards in the HaLow edition ("ESP32-S3-3.5-HaLow"), ordered and in hand, HaLow fitted on both as the baseline**, plus ESP32-C6 modules staged for the mesh-coprocessor link and GNSS + 3-axis digital-compass modules staged for the remaining connectors — all per the attach points in `01_SS-SP_LITE_HARDWARE_REFERENCE.md` §3.15 (coprocessor on the UART2 peripheral mapped to pins 44/43, the chip's default UART0 pins; GNSS on UART1 18/17; compass on I²C0 15/16). Once the C6 link and GNSS/compass paths are validated on these units, this fully-loaded configuration is declared the **locked base Lite v1 hardware spec** by a superseding entry. These two units are the primary dev/test/prototype devices for the entire portfolio execution, and Lite is the first product made available; Alpha and Omega are in engineering/manufacturing and follow later. Exact GNSS/compass module part numbers are confirmed at attachment — any deviation from the §3.15 table updates that table (and the pin-map, T1) first.
- Rationale: VA-03 hardware exists now, so hardware-verified story ACs are unblocked as soon as flashing is set up; declaring the spec-lock criteria up front prevents silent scope drift on the first commercial SKU.
- Status: RECORDED
- Reference: `01_SS-SP_LITE_HARDWARE_REFERENCE.md` §3.15/§7/§9, VA-03, EPIC-03, `docs/OWNER_DECISIONS.md` (D1–D3)

## D-0014 — Branch protection with founder bypass; repo intake surfaces enabled

- Date: 2026-07-05 (bootstrap)
- Decision: `main` on the D-0011 repo is protected with required status checks `dco`, `lint-docs`, and `build (lite)` (strict/up-to-date mode), required linear history, and force-push/deletion forbidden. `enforce_admins` stays **off** during solo bootstrap: the founding maintainer may push directly to `main` (checks still run post-push); bots and all future contributors go through green PRs. The review-count requirement in CONTRIBUTING §9 is enforced socially, not mechanically, until a second maintainer joins, at which point a superseding entry turns on required reviews and admin enforcement. Alongside protection, the repository intake surfaces were switched on per prior decisions: GitHub private vulnerability reporting enabled (C3), GitHub Discussions enabled (D-0012), and the unused wiki disabled (docs of record live in-repo). This completes VA-02.
- Rationale: A solo maintainer cannot approve their own PRs; admin bypass keeps velocity honest while required checks + linear history + no force-push still protect history integrity against accidents and automation. Recorded from the owner's answer to A6 in `docs/OWNER_DECISIONS.md`.
- Status: RECORDED
- Reference: VA-02 in `docs/portfolio/09_VENTURE_EXECUTION_MAP.md`, `docs/OWNER_DECISIONS.md` (A6), `CONTRIBUTING.md` §9, `.github/workflows/`

## D-0015 — ElementalMFG personal account accepted for bootstrap (amends D-0011)

- Date: 2026-07-05 (bootstrap)
- Decision: The hosting audit found that `github.com/ElementalMFG` is a personal **user** account, not a GitHub organization as D-0011 stated. The owner accepts the user account for the solo-bootstrap phase; all D-0011 provisions (repo of record `ElementalMFG/ecosystem`, public, production status) stand unchanged. Conversion to an organization (GitHub's account-conversion flow preserves the repo, history, and URL) MUST happen before the second maintainer is onboarded or the foundation transition begins, whichever comes first, and is recorded by a superseding entry when done. Org-only features (working-group teams behind CODEOWNERS, org-wide 2FA policy) are acknowledged as deferred until then.
- Rationale: Nothing technical in the current toolchain (branch protection, required checks, Dependabot, private vulnerability reporting, Discussions) requires an org; converting mid-bootstrap adds risk with no present benefit. Recorded from the owner's answer during the pre-development readiness audit.
- Status: RECORDED
- Reference: D-0011, VA-02, `docs/OWNER_DECISIONS.md` (A2)

## D-0016 — CLA kept alongside DCO; grantee Elemental MFG; bot enforced

- Date: 2026-07-06 (bootstrap)
- Decision: The project keeps the dual DCO + CLA model documented in CONTRIBUTING §5 (owner decision A5, recommended option). The CLA grantee is **Elemental MFG** (the D-0012 parent entity), with an explicit assignment-to-successor-foundation clause. The normative CLA text is `docs/CLA.md` (Apache-style ICLA; corporate contributions authorized via a recorded issue until a standalone CCLA ships at Phase 1). Enforcement is `contributor-assistant/github-action` (`.github/workflows/cla.yml`): every PR author signs once; signatures are recorded on the `cla-signatures` branch; `dependabot[bot]`/`github-actions[bot]` are allowlisted. There is no size threshold — CLA per author, DCO per commit. The CLA check is promoted to a required status check on `main` by amending the D-0014 protection once its first live run establishes the check context.
- Rationale: Preserves the relicense-and-foundation-transition option in `04_LICENSING_AND_FORK_STRATEGY.md` without contributor re-consent later; the grantee had to wait for E1 (parent entity), which D-0012 answered.
- Status: RECORDED
- Reference: `docs/OWNER_DECISIONS.md` (A5, E1), `CONTRIBUTING.md` §5, S-01-008, `docs/CLA.md`

## D-0017 — Twelve working-group charters ratified with bootstrap staffing

- Date: 2026-07-06 (bootstrap)
- Decision: The twelve working-group charters in `governance/wg/` are ratified: wg-firmware, wg-protocol, wg-ui, wg-radio, wg-security, wg-companion, wg-cloud, wg-ai, wg-docs, wg-compliance, **wg-community**, and **wg-legal**. The last two are hereby added to the "Initial WGs" table in `06_GOVERNANCE.md` §3 — they were already load-bearing in RFC-0001, the foundation-transfer draft, and the portfolio, but missing from the table. During solo bootstrap the founding maintainer chairs all twelve; each charter states that the chair is replaced by Steering Committee appointment when the WG is staffed, at which point the corresponding CODEOWNERS entries move to that WG's team (org teams require the D-0015 org conversion first). Cadence is async-first (monthly written status while a WG has active epics), escalation paths are as stated in each charter, and disband/split is by SC vote per `06_GOVERNANCE.md` §3.
- Rationale: S-01-009's AC requires all twelve charters merged with chairs, cadence, and escalation stated; honest bootstrap staffing (one person, twelve hats) is recorded rather than inventing rosters. Fixing the doc-06 table closes the community/legal governance gap.
- Status: RECORDED
- Reference: S-01-009, `06_GOVERNANCE.md` §3, `governance/wg/`, RFC-0001, `rfcs/DRAFT-foundation-transfer.md`, D-0015

## D-0018 — RFC-0003 protocol compatibility & deprecation policy accepted

- Date: 2026-07-07
- Decision: RFC-0003 (protocol compatibility & deprecation policy, cross-layer) is accepted via the bootstrap governance provision — the program lead acts as Steering Committee per D-0015 / RFC-0001.
- Rationale: Establishes one governed rule for supported-version windows, deprecation lead times (≥2 minor releases and ≥12 months), and sunset via `docs/DEPRECATIONS.md` across all seven versioned surfaces, operationalizing `06_GOVERNANCE.md` §4.4/§6.3. In consequence, the version-negotiation stories (S-06-016, S-12-014, S-10-008) and the SDK policy (S-20-017) cite RFC-0003 instead of restating windows, and wire-format PRs must cite it in CI (S-01-017).
- Status: RECORDED
- Reference: `rfcs/0003-compat-deprecation-policy.md`, `06_GOVERNANCE.md` §4.4/§6.3, `docs/DEPRECATIONS.md`, S-06-016, S-12-014, S-10-008, S-20-017, S-01-017, D-0015, RFC-0001

## D-0019 — v1.0 scope lock ratified (RFC-0004)

- Date: 2026-07-08
- Decision: RFC-0004 (v1.0 scope lock) is accepted via the bootstrap governance provision (program lead as SC per D-0015 / RFC-0001). Ratified: SL-1 device identity — primary "sovereign multi-band mesh communicator + universal node", descriptor "a multi-band Wi-Fi (2.4/5 GHz) / HaLow / BLE smartphone-class device in a pager form factor"; SL-2 `ss_ai` capability-only scaffold, no LLM promise before v2.x; SL-3 video via signed WASM plugins post-v1.0; SL-4 browsing is application-layer only — through-device tether in v1.0, browser-type apps on the plugin platform where hardware permits, full browser targets Omega v1.x, and compatibility guard 6 binds the plugin platform's smartphone-app capability floor; SL-5 v1.0 ships Lite + Alpha; SL-6 v1.0 SDKs C/Rust/Python with TS/Dart in v1.1; SL-7 Lite constraint callouts in the README matrix.
- Rationale: A verified portfolio audit showed capability coverage was already essentially complete and the risk was framing drift; locking identity and feature truth in one RFC stops per-epic re-litigation. All changes are additive — no story is deleted or narrowed; SL-6 re-trains two SDK stories to v1.1.
- Status: RECORDED
- Reference: `rfcs/0004-scope-lock.md`, S-01-018, S-01-019, RFC-0003, D-0015, RFC-0001


## D-0020 — Software portfolio aligned to signed-off Omega v69 / Alpha reality

- Date: 2026-07-09
- Decision: Software claims follow the **released** board. Omega v1.0 hardware truth = PCB `release_v69/` (2026-07-08, TVF 69/69, SHA `054eaa8b…`; source `closure_work/v67_route_work.kicad_pcb`). Bearers/parts absent from v69 (cellular, satellite, LoRa, barometer, secure element, supercap power path) are **preserved as rev-2-gated roadmap intent**, never silently dropped: 16 EPIC-05 stories BLOCKED on the Omega rev-2 respin. Alpha v152 is NOT release-verified (v14 "FUNCTIONALLY DEAD — DO NOT FABRICATE"; v15 unverified): no hardware-empirical elaboration of EPIC-04 until an Alpha release package exists. Doc of record: `docs/dev/OMEGA_HW_BASELINE.md`.
- Rationale: The board is final and shipped to fab; software aligning to aspiration instead of copper produces ghostware stories and un-meetable ACs (the EPIC-03 pre-hardware-lock drift class, at portfolio scale).
- Status: RECORDED (retroactive entry — decision applied 2026-07-09 in commit 4c3ff7b; ledger entry added same day)
- Reference: `docs/dev/OMEGA_HW_BASELINE.md`, `ss-pcb-design-engeneering/release_v69/`, EPIC-05, S-05-020

## D-0021 — Omega concerns-ledger projection: EPIC-05 owns the v69 HAL

- Date: 2026-07-09
- Decision: The HW repo's closure artifacts (`UNIFIED_CONCERNS_LEDGER.md` ~88 concerns, `OPTIMIZATION_DECISIONS.md`, `HW_SW_ALIGNMENT_AUDIT.md`) are absorbed via the re-projection rules in `docs/dev/OMEGA_LEDGER_ALIGNMENT.md`: (1) per `NAMING.md` the audited board IS the ss-sp omega, so the ledger's legacy "Alpha/EPIC-04/S-04-xxx" targets re-project onto **EPIC-05**, which owns the entire v69 HAL (new shards J/K/L, stories S-05-021…039); EPIC-05's dependency on "EPIC-04 frozen" is inverted. (2) EPIC-04 (Alpha, unlocked) is preserved and annotated, never force-aligned to v69 evidence. (3) Chip-level facts are fixed outright in founding docs; board-level facts are asserted for Omega only. (4) Factual corrections to D-0020's baseline recorded: touch = GT911 (not FT6206), SKY66423 FEM + pa_pdet ARE routed ("gap G1" was a misread GNSS pin name), EA3059 3-ch PMIC present, mic is PDM.
- Rationale: Single projection of record prevents the ledger's ~88 findings being re-litigated per story; the naming re-projection prevents Omega board evidence from deleting Alpha capability (anti-rug-pull).
- Status: RECORDED. Open flag: RFC-0004 SL-5 ("v1.0 ships Lite + Alpha") contradicts hardware reality (Omega released, Alpha unfabricatable) — owner amendment required, tracked in `OMEGA_LEDGER_ALIGNMENT.md` §8.
- Reference: `docs/dev/OMEGA_LEDGER_ALIGNMENT.md`, `docs/dev/OMEGA_HW_BASELINE.md`, `ss-pcb-design-engeneering/NAMING.md`, D-0020, D-0019
