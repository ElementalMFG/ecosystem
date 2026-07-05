<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# Steering Committee Decisions Log

Chronological, append-only. Each entry is signed by the Chair at close.

## Schema, counter, and quorum

- **Schema.** Every entry is `## D-NNNN — <title>` followed by `- Date:` (ISO date, or `2025-Q? (bootstrap)` for pre-log decisions reconstructed at bootstrap), `- Decision:` (normative text), optional `- Rationale:`, optional `- Status:` (`RECORDED` default; `SUPERSEDED by D-NNNN` when overturned), and `- Reference:` (constitution docs, RFCs, or stories).
- **Counter.** `NNNN` is a zero-padded monotonically increasing integer. Numbers are assigned at append time and never reused. Next free number: **D-0011**.
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
