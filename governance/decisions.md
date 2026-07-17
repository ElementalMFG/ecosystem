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
- Status: RECORDED (retroactive entry — decision applied 2026-07-09 in commit 4c3ff7b; ledger entry added same day). Ship-order AMENDED by D-0026 (2026-07-14): round-1 ships on off-the-shelf Elecrow boards (`elecrow5`); v69 is the round-2 / in-house board-of-record. All v69 hardware-truth and rev-2 preservation in this entry otherwise stand.
- Reference: `docs/dev/OMEGA_HW_BASELINE.md`, `ss-pcb-design-engeneering/release_v69/`, EPIC-05, S-05-020, D-0026

## D-0021 — Omega concerns-ledger projection: EPIC-05 owns the v69 HAL

- Date: 2026-07-09
- Decision: The HW repo's closure artifacts (`UNIFIED_CONCERNS_LEDGER.md` ~88 concerns, `OPTIMIZATION_DECISIONS.md`, `HW_SW_ALIGNMENT_AUDIT.md`) are absorbed via the re-projection rules in `docs/dev/OMEGA_LEDGER_ALIGNMENT.md`: (1) per `NAMING.md` the audited board IS the ss-sp omega, so the ledger's legacy "Alpha/EPIC-04/S-04-xxx" targets re-project onto **EPIC-05**, which owns the entire v69 HAL (new shards J/K/L, stories S-05-021…039); EPIC-05's dependency on "EPIC-04 frozen" is inverted. (2) EPIC-04 (Alpha, unlocked) is preserved and annotated, never force-aligned to v69 evidence. (3) Chip-level facts are fixed outright in founding docs; board-level facts are asserted for Omega only. (4) Factual corrections to D-0020's baseline recorded: touch = GT911 (not FT6206), SKY66423 FEM + pa_pdet ARE routed ("gap G1" was a misread GNSS pin name), EA3059 3-ch PMIC present, mic is PDM.
- Rationale: Single projection of record prevents the ledger's ~88 findings being re-litigated per story; the naming re-projection prevents Omega board evidence from deleting Alpha capability (anti-rug-pull).
- Status: RECORDED. Open flag RESOLVED by D-0026 (2026-07-14): round-1 ships on off-the-shelf Elecrow boards (Lite/Alpha = 3.5″ S3, Omega tier = 5″ P4), so ship-order no longer depends on the in-house PCBs; RFC-0004 SL-5 + `OMEGA_LEDGER_ALIGNMENT.md` §8 to be updated to match (filed with the D-0026 propagation).
- Reference: `docs/dev/OMEGA_LEDGER_ALIGNMENT.md`, `docs/dev/OMEGA_HW_BASELINE.md`, `ss-pcb-design-engeneering/NAMING.md`, D-0020, D-0019

## D-0022 — Signature-algorithm coherence: dual chain (RSA-3072-PSS SBv2 + Ed25519 application manifests)

- Date: 2026-07-09
- Decision: The firmware trust chain uses **two deliberately distinct algorithms**: (1) **Secure Boot V2 = RSA-3072-PSS** (silicon-locked eFuse digest — S-08-001, identical profile on S3/Lite and P4/Omega); (2) **all application-layer signed artifacts = Ed25519** — the OTA manifest inner signature over the SBv2-verified image (S-09-001) and, by the same rule, the HaLow region blob (S-05-030) and any future signed config/manifest. Rule of thumb: silicon-locked chains use what the ROM verifies; everything above it uses Ed25519.
- Rationale: Resolves HW-repo owner decision #7 (EXECUTION_READINESS_AUDIT §11): S-08-001 and S-09-001 already embodied exactly this split — ratifying it prevents an accidental "unification" in either direction and unblocks HW ST-M2.4's eFuse-state hand-off spec. Ratified via the bootstrap governance provision (program lead as SC per D-0015).
- Status: RECORDED
- Reference: S-08-001, S-09-001, S-05-030, `ss-pcb-design-engeneering/closure_work/OPEN_DECISIONS_TO_OWNER.md` #7, D-0015

## D-0023 — Elecrow module variant matrix; provisional ss-sp alpha v1; Omega rev-2 LoRa priority

- Date: 2026-07-09 (owner-directed, this session)
- Decision: (1) The Elecrow CrowPanel Advance 3.5″ (ESP32-S3) is the **common base platform** for a family of module-configured variants. Module slots per doc 01 §3.15 pin truth: wireless header (SPI2) = HaLow module OR SX1262 LoRa module; the **UART0-pins connector** (physical pins 44/43 — the port doc 01 maps the UART2 peripheral onto; owner calls it "UART0-in") = ESP32-C6 Wi-Fi/BLE coprocessor module OR an Elecrow LoRa module (exact part + transport verified at attachment per D-0013; any deviation updates doc 01 + pin map FIRST); UART1 = GPS module; I²C0 = 3-axis compass / IMU. (2) **Provisional lineup** (confirm at variant ratification): **ss-sp lite** = base + HaLow + GPS/compass + LoRa; **ss-sp alpha v1 (provisional)** = base + HaLow + GPS/compass + C6 coprocessor — i.e. Alpha v1 may ship as an Elecrow variant profile while the proprietary P4 Alpha matures; 2–3 customizable Lite variants are envisioned. (3) **Omega rev-2 priority = add LoRa** (S-05-040). (4) **Bearer-readiness principle (binding)**: every module/bearer driver (SX1262, HaLow, C6 ESP-Hosted, GNSS, compass) is developed and CI-maintained regardless of fitment; capability flags + variant profiles + boot-time module detection make each configuration plug-and-play — features are ready whenever a variant needs them, and their absence never breaks a build.
- Rationale: Owner direction 2026-07-09. The HAL is already capability-flagged (CONFIG_SS_LITE_MOD_*, D-0013); the gaps were named variant profiles, a CI build matrix, and runtime presence detection — filed as S-03-047/S-03-048. Additive: no existing story or SKU meaning is narrowed; NAMING.md §3's Alpha row (HW repo) gains a provisional-variant reading to be reflected there at next HW-repo touch.
- Status: RECORDED (lineup provisional — confirm before first variant ships)
- Reference: `01_SS-SP_LITE_HARDWARE_REFERENCE.md` §3.15, D-0013, S-03-047, S-03-048, S-05-040, RFC-0004 Amendment A1, `ss-pcb-design-engeneering/NAMING.md`

## D-0024 — Cellular (LTE-M/NB-IoT, SIM/eSIM) posture: unscheduled roadmap option

- Date: 2026-07-09 (owner-directed, this session)
- Decision: Cellular connectivity (SIM cards / cell towers) is **not part of current product intent** for any SKU or board revision. It is retained as an **unscheduled roadmap option**: the ten EPIC-05 cellular stories and S-24-005/006 stay BLOCKED and preserved (anti-rug-pull), but their gate is re-labeled from "Omega rev-2" to "unscheduled — revisit only if a future board rev adopts a modem AND it adds no meaningful complexity". Software keeps **architecture-level compatibility only**: the bearer abstraction (Reticulum interfaces, per-interface timeouts — S-11-007) remains bearer-universal so a cellular bearer could be added without redesign; no modem drivers, carrier work, or eSIM plumbing is planned or scheduled. PRD F-BR-06's "mandatory Omega" is annotated accordingly.
- Rationale: Owner direction 2026-07-09 ("no intention of including that specific functionality at this time"; open to costless compatibility). Preserving the stories keeps the option open at zero build cost; the universal bearer abstraction is the cheap, safe form of "compatible even if we don't use it".
- Status: RECORDED
- Reference: EPIC-05 shards A–E, S-24-005, S-24-006, S-11-007, `docs/portfolio/02_PRD.md` F-BR-06, D-0020, D-0021

## D-0025 — Real-world telephony interconnect (PSTN/SMS/voice) posture: open bridge, hosted-default + self-host, phased text→voice

- Date: 2026-07-13 (owner-directed, this session)
- Decision: Real-world telephony interconnect is established as a **first-class cross-cutting ecosystem capability** (not a single cloud story), delivered by a **bridge/gateway pattern**: the mesh/RNS side never touches the PSTN directly; an internet-connected **bridge node** terminates LXMF/RNS on the mesh side and speaks to a telephony provider — a CPaaS (e.g. Twilio/Telnyx/Bandwidth/Plivo) or a SIP trunk — on the carrier side. (1) **Two bandwidth-gated capabilities:** (a) **text/SMS ↔ LXMF** — universal, works over *every* bearer including LoRa because it is async store-and-forward; this is the priority and the default hosted offering; (b) **voice ↔ PSTN** — **IP-bearer-only** (HaLow-gatewayed / Wi-Fi / any future cellular or satellite backhaul), never available to LoRa-only field units. (2) **Topology = both hosted and self-hostable, hosted-default:** the project runs a managed bridge (expands S-21-027) for turnkey UX and as the Tier-2 paid "dedicated number" offering (the Zoleo differentiator, `06_COMPETITIVE_LANDSCAPE.md`); it also ships an **open reference bridge ("bring-your-own-number")** runnable on the Home Gateway (EPIC-17) or as a sandboxed plugin/sidecar (EPIC-18) against the *operator's own* CPaaS/SIP account. The phone number and carrier relationship always belong to the bridge operator. (3) **Provider abstraction (binding, capability-ready):** define a carrier-neutral **telephony-provider interface** (send/receive SMS, place/answer/bridge call, number provisioning, DTMF, delivery receipts) so any CPaaS or SIP trunk is pluggable; the interface + one reference adapter are built and CI-maintained regardless of which tier is activated — mirroring the D-0023 bearer-readiness principle and the D-0024 "compatible even if unused" posture (build it into the ecosystem even where it may not ship). **Addressing model:** the bridge owns an **E.164 ↔ LXMF-identity mapping**; mesh users reach the phone network via a `tel:` destination; inbound calls/texts route to the provisioned identity's LXMF inbox. (4) **Phasing (staged gates; none block Alpha/Omega v1.0):** **P1** text both-ways, hosted + self-host; **P2** inbound voicemail-to-text (call answered → recorded/transcribed → delivered as LXMF, so it still works for LoRa users); **P3** live voice ↔ PSTN on IP bearers via a **SIP ↔ LXST bridge** reusing EPIC-14 Opus and interoperating with Reticulum's LXST / `rnphone` telephony layer (interop tracked via S-19-027; a dedicated PRD requirement F-TEL-* is to be filed — F-INT-04 is video-call, not telephony). (5) **Regulatory posture (least-regulated for the project):** the project's *hosted* offering stays **text-first**; A2P **10DLC** brand/campaign registration is handled for the managed SMS service under wg-legal, per-country enablement gated by wg-legal. **Live-voice / interconnected-VoIP exposure — E911, CALEA, USF, CPNI — is carried by the operator of each bridge via their own carrier account; self-host is the release valve** that keeps that regime off the project by default. All telephony features carry a prominent **"not for emergency calling"** notice; the project offers and implies **no 911/112 termination**. (6) **Revenue + universality:** "**Reachability / Dedicated Number**" becomes a *named* Tier-2 cloud revenue line (hosted text bridge, billed via S-21-022; benchmark JMP.chat ~$4.99/mo, Zoleo differentiator), with a lower BYO-number "compute-only" tier; self-host stays free/Apache-2.0 (the anti-lock-in moat, incl. mandatory number port-out). Universal interop is pursued **where standards are open** (SIP/XMPP/Nostr/MLS/Multipath-QUIC/WHIP/eUICC), **partnered where regulation forces it** (real MSISDN via MVNE+certified eUICC, RCS-for-Business, emergency calling), and **built native where incumbents keep it closed** (carrier VoWiFi/IMS, P2P RCS, Fi-style handoff). (7) **Native-messaging posture:** COMPETE first (harden the RNS/LXMF stack + companion app UX to on-par-or-exceeding Element on the off-grid/sovereignty axes), ADOPT selectively (MLS/TreeKEM ideas + hybrid PQC + sealed-sender, run on the gateway), BRIDGE last (opt-in gateway nodes, Nostr→XMPP→Matrix, always labeled *not* end-to-end; never "stock Element on the mesh"). Full design substrate — the Sovereignty Ladder, the Universal Communications Abstraction / Telephony Provider Interface, the AI/ML division of labor (MCU=radio+codec+PQC; gateway=AI brain; phone=NPU), the interop verdict matrix, and all citations — is recorded in `docs/portfolio/12_TELEPHONY_AND_UNIVERSAL_INTEROP_DOSSIER.md`.
- Rationale: Owner direction 2026-07-13 (include the full capability across all tiers, hosted-default + self-host, build maximally into the ecosystem even where some of it may not ship). Assessment found only S-21-027 (a thin SMS-bridge cloud story) touching the phone network, no voice-to-PSTN thread, and a centralized-only framing that contradicts the sovereign thesis. The bridge pattern is the only design consistent with the constraints: PSTN numbers are regulator-assigned and cannot be decentralized, but the *client-facing* side can be — an open, self-hostable, BYO-number bridge is simultaneously the most universal (text reaches every bearer), the most decentralized (anyone runs one), and the least regulated *for the project* (per-operator carrier liability). Text-first + "not for emergencies" avoids the interconnected-VoIP/E911 trap; the provider abstraction is the cheap, safe form of "capable even if not activated".
- Status: RECORDED (capability scoped; PRD anchored — F-TEL-01..08 + F-INT-05; implementing stories FILED — S-21-027/030/031/032/033, S-17-022, S-14-020/021, S-18-019, S-24-038; TPI contract FROZEN — S-21-030, `cloud/telephony-bridge/TPI_CONTRACT.md`; adapter impl BLOCKED pending cloud scaffold S-21-033 + cloud CI S-23-004 + CPaaS sandbox VA-29; hosted voice deferred to P3, no v1.0 gate).
- Reference: `docs/portfolio/12_TELEPHONY_AND_UNIVERSAL_INTEROP_DOSSIER.md`, S-21-027, S-19-027, EPIC-10/12/14/17/18/20/21/24, `docs/portfolio/06_COMPETITIVE_LANDSCAPE.md` (Zoleo), `docs/portfolio/08_HALOW_TECHNOLOGY_DOSSIER.md`, `08_UNIVERSAL_CONNECTIVITY.md`, `docs/portfolio/02_PRD.md` F-VOX-06, D-0022, D-0023, D-0024

## D-0026 — Round-1 device line = off-the-shelf Elecrow boards; in-house PCBs = round-2; name/brand/tier/board decoupled

- Date: 2026-07-14 (owner-directed, this session)
- Decision: The **initial, market-ready device line ("round-1") is built on off-the-shelf Elecrow CrowPanel Advance boards**, not the in-house PCBs, so the ecosystem can be built and validated on real, purchasable hardware now. (1) **Round-1 lineup:** Lite and Alpha ride the Elecrow CrowPanel Advance **3.5″ ESP32-S3** (per the D-0023 module-variant matrix); the flagship (**Omega** tier role) rides the new **Elecrow CrowPanel Advance 5″ ESP32-P4 HMI** — ESP32-P4 + onboard **ESP32-C6 (Wi-Fi 6 / BLE 5.3)**, 800×480 MIPI-DSI GT911 touch, NS4168 speaker/mic, an **SPI wireless-module slot**, optional MIPI-CSI camera, UART/I²C Crowtail expansion, USB-C + battery/charging. Firmware board id **`elecrow5`**. (2) **In-house PCBs are round-2:** the signed-off Omega **v69** (D-0020) and the unfabricated Alpha **v152** are reclassified from "ships first" to **round-2 / in-house — engineering board-of-record, manufactured later**; their capability ledgers, HAL work, and anti-rug-pull preservation (D-0020/D-0021) stand unchanged — only *ship order* changes. (3) **Naming/identity is decoupled into four independent axes** so any future rename is zero-rework: **ecosystem/protocol name** (SS-SP — the mesh/protocol/software; stable), **product brand line** (round-1 = SS-SP; the round-2 in-house line's branding is **DEFERRED** and may adopt a new family name, e.g. DecoMesh), **SKU tier role** (Lite/Alpha/Omega = capability tiers, board-independent), and **hardware target** (`elecrow5`, `elecrow35-s3`, `omega-v69`, future in-house ids). A device = brand × tier × board; docs, capability flags, and build targets key off the **hardware target + tier**, never a marketing name. (4) **Radio posture for `elecrow5`:** **HaLow** (Elecrow MM6108 module in the SPI slot) is the one **mandatory** radio for the standard SKU; **Wi-Fi 6 + BLE** (onboard C6) are always present; **a LoRa variant** swaps the SPI-slot module (LoRa **or** HaLow — one module at a time, not simultaneous). GPS (UART), 3-axis compass (I²C), speaker, and camera are **all optional and boot-time auto-detected** (D-0023 bearer-readiness + S-03-048 presence probes); absent peripherals compile/flag to zero. "Everything optional except HaLow" is the `elecrow5` capability default. (5) **Ruggedization:** round-1 Elecrow boards are **dev/functional-grade, non-IP-rated** (bare board in a custom enclosure); IP65/66 ruggedization is a **round-2 / in-house** property — round-1 docs/marketing must not claim rugged/IP ratings. (6) **Firmware policy unchanged:** ESP32-family stays the first-party firmware target (the Elecrow P4/S3 are in-family); non-ESP32 MCU firmware (Arduino/RP2040/nRF/STM32) stays deliberately out of scope (`06_COMPETITIVE_LANDSCAPE.md` §2.1). **Universal cross-platform reach is already delivered** for computers/phones/Raspberry Pi/ARM SBCs/routers/cloud via the portable `ss_node_core` + SDKs + headless daemon (S-11-021, S-19-021/023/028) — any RNS/LXMF speaker is a peer; this pivot does not change that. (7) **Scope of change (to be filed via story-run):** add firmware board `elecrow5` (board_config + MIPI-DSI display driver + HaLow-over-SPI + C6/Wi-Fi-6 ESP-Hosted link + optional GNSS/compass/speaker/camera + capability profile) and a CI build target; **retarget EPIC-05's buildable-now Omega HAL stories to `elecrow5`** as the round-1 realization while keeping v69-specific work as round-2; update hardware-truth docs (`00_MASTER_SOFTWARE_PLAN.md` §1.2, `01_SS-SP_LITE_HARDWARE_REFERENCE.md`, `docs/dev/OMEGA_HW_BASELINE.md`, `docs/portfolio/EPIC_INDEX.md` + README SKU rows); update RFC-0004 SL-5 + `docs/dev/OMEGA_LEDGER_ALIGNMENT.md` §8 (resolves the D-0021 open flag). Open HW unknowns to verify at bring-up (non-blocking): C6↔P4 link method (assume SDIO/ESP-Hosted), HaLow-over-SPI throughput ceiling + SPI/UART1 DIP-shared pins (route GPS to a free UART), MIPI-DSI bridge IC, camera sensor model, pinned ESP-IDF version.
- Rationale: Owner direction 2026-07-14 — the Elecrow boards are what is actually purchasable and market-ready; the in-house line will be deliberated and engineered substantially before it manufactures and may be rebranded. The HAL's capability-flag + variant + runtime-detection design (D-0023) already makes multi-board support first-class, so round-1 on off-the-shelf ESP32-P4/S3 hardware is additive, not a rearchitecture. Decoupling name/brand/tier/board makes ship-order a property of *round*, not of a specific PCB — this resolves the D-0021 open flag (RFC-0004 SL-5 "v1.0 ships Lite + Alpha" vs. Omega-released / Alpha-unfabricatable) and future-proofs any round-2 rebrand at zero cost. Additive: no capability, story, or SKU meaning is narrowed; D-0020/D-0021 preservation of the v69/Alpha ledgers is retained.
- Status: RECORDED (round-1 lineup + naming-decoupling binding; `elecrow5` board bring-up + EPIC-05 retarget + hardware-doc updates + CI to be filed via story-run; round-2 in-house branding deferred).
- Reference: D-0013, D-0020, D-0021, D-0023, D-0024, `01_SS-SP_LITE_HARDWARE_REFERENCE.md`, `docs/dev/OMEGA_HW_BASELINE.md`, `ss-pcb-design-engeneering/NAMING.md`, EPIC-05, EPIC-03 (S-03-047/048), EPIC-10, EPIC-11, `06_COMPETITIVE_LANDSCAPE.md` §2.1, RFC-0004 SL-5
