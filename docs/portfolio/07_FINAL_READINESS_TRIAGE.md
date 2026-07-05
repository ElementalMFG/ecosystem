<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# 07 — Final Readiness Triage (Sole-Source Buildability Audit)

*Decision record. This document is the permanent triage ledger of the final portfolio audit: whether the portfolio + constitution are sufficient as the **sole source of information and instruction** for an implementing agent (human team or AI coding agent) to develop the entire SS-SP software, network, and ecosystem. It records what was checked, how, every finding, its disposition, and the final verdict.*

**Status: PASS — portfolio certified as sole-source buildable (see §7 for the conditions that make this true).**

---

## 1. Audit method

Four systematic passes, each machine-verified where possible:

| Pass | Question | Method |
|---|---|---|
| P1 Structural | Do all documents exist, non-trivially, with valid cross-linkage? | Scripted inventory of 14 constitution/governance docs, 10 portfolio docs, 24 × (EPIC.md + STORIES.md), rfcs/, tools/, firmware/ |
| P2 Layer sufficiency | Could each layer be built from these docs alone? | 3 independent deep audits (firmware/protocol/security · UI/apps/SDK/plugins · cloud/gateway/testing/CI/compliance), each classifying 15 normative contracts per layer as **A** explicitly specified / **B** delegated to a story with concrete ACs / **C** missing |
| P3 Consistency & edge cases | Are cross-document facts consistent; are real-world failure modes storied? | Prior audit round (recorded here): part numbers, crypto suite, pin maps, milestones, pricing, licence split; 27 failure scenarios |
| P4 Traceability | Does every requirement have stories and every story reference resolve? | `tools/gen-stories-index.py --check` + bidirectional PRD↔story validation + Meta-field schema validation |

**Classification rule.** Per `00_METHODOLOGY.md`, a story with concrete acceptance criteria **is** the specification vehicle of this portfolio — implementation detail is deliberately carried in story ACs and RFC deliverables, not frozen prematurely in prose docs. Therefore **B (delegated) counts as covered**, provided the delegation target exists and its ACs pin the contract. Only **C (missing — neither specified nor assigned)** is a defect. This rule is what separates a *roadmap gap* from *healthy spec-by-story*.

---

## 2. P1 — Structural inventory: PASS

- 14/14 constitution + governance documents present and substantive (`00_MASTER_SOFTWARE_PLAN.md` … `08_UNIVERSAL_CONNECTIVITY.md`, `governance/OPEN_ASSURANCE.md`, `governance/decisions.md`, `docs/TRADEMARK.md`, `CODE_OF_CONDUCT.md`, `SECURITY.md`)
- 10/10 portfolio documents present (`00_METHODOLOGY` … `06_COMPETITIVE_LANDSCAPE`, `EPIC_INDEX`, `STORIES_INDEX`, `README`)
- 24/24 epics with both `EPIC.md` and `STORIES.md`, all non-trivial
- `rfcs/`, `tools/` (index generator), `firmware/` (compile-ready Lite scaffold) present

---

## 3. P2 — Layer-sufficiency matrices

### 3.1 Firmware · protocol · security (root 00/01/02/05/08, portfolio 03, EPIC-02…14)

| Contract | Class | Anchor |
|---|---|---|
| HAL C API surface | A | `01_…HARDWARE_REFERENCE.md` §4.3 (`hal_*.h` map), `03_ARCHITECTURE.md` §3 |
| SS-Link bearer plugin interface | B | S-10-001 (`ss_bearer_ops`), RFC via S-10-019 |
| On-air wire format (LoRa) | B | S-10-008 → RFC-0001 |
| LXMF record types + extensions | A | `02_PROTOCOL_STACK.md` §4.2 (25 record types, CBOR, reserved ranges) |
| Key hierarchy + derivation | A | `05_SECURITY_MODEL.md` §3 (HKDF labels from S_dev), `03_ARCHITECTURE.md` §5 |
| OTA manifest + signing chain | B | S-09-001/002/015 (dual-sig, anti-rollback, SLSA-3) |
| Secure boot + eFuse plan | B | S-08-001/002/004/006/010 |
| Bearer selection algorithm | B | S-10-009 + RFC-0006 (S-10-019); policy sketch `08_…CONNECTIVITY.md` §3 |
| Power/sleep state machine | A/B | HW ref §4.3 + S-03-003, S-10-018 |
| Voice framing + jitter buffer | A/B | `02_PROTOCOL_STACK.md` §5.4 (20 ms, 60–180 ms adaptive) + S-14-001/003/004/007 |
| RTOS, task model, memory budget | A | FreeRTOS/ESP-IDF (`03_ARCHITECTURE.md` §2), S-02-006 priority ceiling |
| Partition/flash map per SKU | A | `05_SECURITY_MODEL.md` §6.1 + S-03-018; Lite scaffold `firmware/partitions.csv` |
| Meshtastic interop boundary | A | `02_PROTOCOL_STACK.md` §4.3 mapping table + `08` §6 (LoRa-only) + 16 EPIC-13 stories |
| Error/logging/diagnostics | B | S-02-007/008/011, S-10-014 |
| Wire versioning/compat policy | B→**FIXED** | per-layer S-06-016/S-12-014/S-10-008 existed; cross-layer policy was **C** → closed by **S-01-017** |

### 3.2 UI · applications · companion apps · SDK · plugins (root 03, EPIC-15/16/18/19/20)

| Contract | Class | Anchor |
|---|---|---|
| Screen inventory + nav map | A | `03_UI_LAYOUT_SPEC.md` §8 (10 canonical screens, per-form-factor templates §4) |
| Widget library, theming, fonts | A | §7 widgets, §3 `ss_theme_t` tokens, 6 themes, S-15-019 fonts |
| Input model + accessibility | A | §6 `ss_input_mask_t`, §10 a11y; S-15-014/015/016 |
| App list w/ feature ACs | B | F-APP-01..07 → S-16-001…023 (mapping verified 1:1, §3.4 below) |
| Companion stacks per platform | A | PRD §2.8: SwiftUI / Kotlin Compose / **Tauri (Q-02 resolved)** / web PWA / shared TS-Dart core |
| Device↔app sync protocol | B→**hardened** | S-19-001/003 + `03_ARCHITECTURE.md` §13.3; pairing spec now RFC'd via **S-19-030** |
| Plugin ABI (WASM, caps, manifest) | B | S-18-001…005 (WAMR/Wasm3, manifest v1, capability grants, syscall table) |
| Plugin registry format | B | S-18-008/009 (signing service, HSM) |
| SDK languages + API surface | B | F-SDK-01..05 → S-20-001…012 (C/Rust/Py/Dart/TS + wire testkit vectors) |
| Config schema / node profile | B | S-16-024 (one schema, every form factor) |
| Onboarding flow | A/B | UI spec §8.10 + S-16-016 (≤3 min, fully offline-capable) |
| i18n mechanism | A | UI spec §11 (`i18n_t`, catalogs, RTL) + S-15-012/013 |
| Offline map tiles | A/B | PMTiles/MBTiles (§8.5) + S-16-023, S-19-026 (GPX/FIT/KML/GeoJSON) |
| Notification model | B | S-15-010, S-16-019, S-19-015 |
| Design tokens / visual identity | A | UI spec §3 + S-15-004 icon set |

UI spec confirmed to cover **all form factors** (Lite 480×320, Alpha 320×240+LED ring, round, e-ink, bar, headless, HMI panels).

### 3.3 Cloud · gateway · testing · CI/CD · governance · compliance (EPIC-01/17/21/22/23/24)

| Contract | Class | Anchor |
|---|---|---|
| Cloud service inventory + boundaries | A | EPIC-21 shards; S-21-001…025 |
| Cloud backend stack | B | **S-21-028** ADR spike (Rust axum vs Node TS) — intentional, gated before detailed design |
| Cloud↔device protocol | C→**FIXED** | enrollment/policy payloads unpinned → closed by **S-21-029** (OpenAPI) + **S-16-026** (device side) + RFC gate on wire changes |
| Multi-tenancy + auth | A | S-21-003 (RBAC, isolation tests, audit log) |
| Self-host packaging | A | S-21-016 Helm / S-21-017 compose (10-min local deploy) |
| Entitlement enforcement | B | S-21-022 + S-22-023 blackhole soak ("lapsed = no new premium, zero degradation") |
| Home Gateway functional spec | A | EPIC-17 (bridging, propagation, soft-AP, fair-use, admin API) |
| Test infra (unit/HIL/sim/fuzz) | A | S-22-001…014 — every modality storied |
| Coverage/mutation gates + DoD | A | `00_METHODOLOGY.md` §5 (12 gates) + S-22-015/016, S-23-012/013 |
| CI + reproducible builds/SLSA/SBOM | A | S-23-001…007 (digest-pinned builder, SLSA-3, CycloneDX) |
| Release process | A | `06_GOVERNANCE.md` §6 + S-23-008/017 |
| RFC + decision flow | A | `06_GOVERNANCE.md` §4 + S-01-006/010 |
| Certification program | A | S-24-025 + S-22-025 conformance suite |
| Support/warranty/RMA | A | S-24-015…018 |
| Data-privacy architecture | A | S-21-018, S-24-010…012, NF-PRIV-01..04 |

**Build order:** epic DAG (`04_BLUEPRINT.md` §3) verified **acyclic and executable**; EPIC-01 + EPIC-22/23 can bootstrap immediately; cloud detail-design gated only on S-21-028. **RACI:** complete, 12 workstreams × 12 roles.

---

## 4. Triage ledger — every finding and its disposition

### 4.1 Defects found and FIXED this audit cycle

| # | Finding | Severity | Disposition |
|---|---|---|---|
| T-01 | HaLow part drift MM6108 vs MM8108 across 8 files | HIGH | FIXED — standardized **MM8108** everywhere (0 stale refs) |
| T-02 | Codec2 rate drift (3200/1400/700 vs 1200 vs constitution 3200/1600/700C) | MED | FIXED — PRD F-VOX-04 + S-14-004 aligned to `02_PROTOCOL_STACK.md` §5.4 |
| T-03 | No cross-layer protocol compatibility/deprecation policy | MED | FIXED — **S-01-017** (RFC, governs every versioned surface) |
| T-04 | No pool-allocator contract for frame/message hot paths | MED | FIXED — **S-02-021** (`ss_pool`, per-pool exhaustion semantics, zero-heap RX/TX proof) |
| T-05 | F-SEC-10 duress wipe had UX + tests but **no implementing story** | HIGH | FIXED — **S-07-021** (silent wipe engine + compromise beacon, power-cut safe) |
| T-06 | No signed asset/model-pack update path (Whisper/Piper/fonts/tiles) | MED | FIXED — **S-09-023** (versioned signed packs, atomic swap, firmware-compat bounds) |
| T-07 | Fleet policy engine was server-side only; no device-side receipt/apply | MED | FIXED — **S-16-026** (signed `admin.command`, non-overridable safety denylist per C-05) |
| T-08 | Pairing protocol referenced (`pairing.cbor`) but no story delivered its spec | HIGH | FIXED — **S-19-030** (pairing RFC + conformance vectors, firmware + app cores) |
| T-09 | Fleet Console OpenAPI referenced by S-20-015 but never delivered | MED | FIXED — **S-21-029** (OpenAPI 3.1 for all four services, drift-checked in CI) |
| T-10..T-16 | 7 edge-case gaps: lost/stolen revocation, multi-device identity, hostile-network TLS, battery degradation ladder + SOS reserve, EAR/Wassenaar, UN 38.3, COPPA | MED–P0 | FIXED — S-07-020, S-19-029, S-11-024, S-16-025, S-24-034, S-24-035, S-24-036 |
| T-17 | Q-01/Q-02/Q-05 stale, F-CA-03 stale, Q-04 unstoried, no cloud-stack ADR | LOW–MED | FIXED — PRD §9 updated; S-05-019, S-21-028 added |
| T-18 | HaLow bearer-table throughput drift ("15 Mbps" vs MM8108 43.33 Mbps elsewhere) | MED | FIXED — `08_UNIVERSAL_CONNECTIVITY.md` table corrected; full research in `08_HALOW_TECHNOLOGY_DOSSIER.md` |
| T-19 | HaLow ESP-IDF driver path stale — `mm-iot-esp32` repo deprecated (archived 2026-07); supported path is the `morsemicro/halow` ESP Component Registry package | HIGH | FIXED — `00_MASTER_SOFTWARE_PLAN.md` §4.1 updated; **S-04-023** pins the component + SDIO-vs-USB spike; **S-04-024** pins TWT/RAW/DTIM power save |
| T-20 | Wi-Fi CERTIFIED HaLow interop certification absent from compliance plan; no cross-vendor interop gate | MED | FIXED — **S-24-037** (WFA certification per HaLow SKU); **S-22-026** (Morse + Newracom interop bench, release gate) |
| T-21 | Real-world venture actions (legal entity, labs, accounts, hardware purchases, external parties) implied by stories but never registered as tracked, milestone-gated work | HIGH | FIXED — `09_VENTURE_EXECUTION_MAP.md` created: two-class execution model + VA-01..VA-28 register with gating milestones, unblocked stories, cost bands, statuses; maintenance rule mirrors this ledger |
| T-22 | Firmware component `firmware/components/hal` shadowed ESP-IDF's built-in `hal` component by name — first real containerized build failed in the mbedtls port (`hal/aes_types.h` not found); the "compile-ready" scaffold claim had never been empirically tested | MED | FIXED — component renamed to `ss_hal` (consistent with all `ss_*` components); `main` REQUIRES + all doc path references updated; clean containerized build now green (ESP-IDF v5.3.5, digest-pinned, `ss_sp_firmware.bin` produced); unknown-board FATAL_ERROR path and `BOARD` env selection verified |
| T-23 | Preflight audit found DONE-story artifact drift: S-01-001 cited a nonexistent `make lint-docs` and "TOC in each" while docs 00–08 had no TOCs and 16 tracked md files lacked SPDX headers; S-01-006 claimed "first two RFCs merged as worked examples" while `rfcs/` held only README+TEMPLATE; S-01-010 claimed D-0001..D-0010 + schema + quorum while only D-0001..05 existed with neither; `docs/dev/BUILDING.md` was linked from README/CONTRIBUTING but absent; S-01-014 wording pre-assigned "RFC-0002" contrary to the number-at-acceptance convention | HIGH | FIXED — `tools/lint-docs.py` + `make lint-docs` + `docs-lint` CI workflow created (links, anchors via GitHub slugs, SPDX, constitution TOCs; 84 files, 0 findings); TOCs auto-generated into docs 00–08; SPDX added to all 16 files; RFC-0001 (process adoption, ACCEPTED) + RFC-0002 (toolchain pinning, IMPLEMENTED) + `DRAFT-foundation-transfer.md` authored and indexed; decisions log completed with schema/counter/quorum section + D-0006..D-0010; `docs/dev/BUILDING.md` written from the verified container build; S-01-014 reworded to the draft-filename convention and closed |

### 4.2 Findings triaged as NOT defects (delegation working as designed)

| Raised concern | Ruling |
|---|---|
| "RFCs 0001/0004/0006/0007/0008 not yet authored" | Correct and intended — authoring them **is** storied work (S-10-008, S-10-019, S-06-01x, S-11-xxx, S-12-xxx). A portfolio that pre-wrote all RFCs would bypass its own governance. |
| Task-priority table "file to be created" | Delivered by S-02-006 AC ("priority-ceiling policy documented and enforced"). |
| MTU-per-bearer, QoS→RNS mapping, charger FSM, IWDT policy, RTC source priority, build-metadata layout, partition-encryption tree | Each pinned in a specific story AC (S-10-008/S-11-022, S-10-019, S-03-002, S-02-009, S-02-018, S-02-020, S-08-005). Story AC **is** the contract per §1 rule. |
| Meshtastic bit-level translation | 16 EPIC-13 stories incl. protobuf-binding regeneration; mapping table frozen in `02_PROTOCOL_STACK.md` §4.3. |
| Wire-testkit YAML schema, Home-Gateway throttle algorithm, billing PSP adapter, relay quota token format | Deliberately implementation-shaped inside S-20-011, S-17-011, S-21-022, S-21-024; counters/vectors are the frozen contract. |
| Cloud DB schema/partitioning | Correctly sequenced **after** S-21-028 stack ADR; load-gated by S-21-026 + NF-SCALE gates. |
| On-device LLM detail | Explicitly out of scope for v1 (PRD §out-of-scope, Omega v2.x). |

---

## 5. Technology-currency register (the "latest/best-in-class" check)

Every layer names a current, proven, leading choice — no legacy or deprecated tech found anywhere in the stack:

| Layer | Choice | Currency note |
|---|---|---|
| MCU/RTOS | ESP32-S3/P4 + FreeRTOS via ESP-IDF 5.x | current Espressif mainline |
| UI | LVGL v9 | current major |
| Crypto | libsodium + PSA; Ed25519/X25519; **hybrid PQ ML-KEM-768 + ML-DSA-65** (FIPS 203/204); XChaCha20-Poly1305; Double Ratchet; MLS-style sender keys | post-quantum ahead of every consumer competitor |
| Radios | SX1262 LoRa, **MM8108** HaLow (Wi-Fi CERTIFIED HaLow gen), BLE 5.x, LTE-M/NB-IoT, NTN Rel-17 + Iridium SBD | full-spectrum, standards-current |
| Voice | Opus + Codec2 3200/1600/700C (licence-isolated); Whisper-tiny int8 STT; Piper TTS | on-device AI, state of the art at this power class |
| Plugins | WASM (WAMR), capability tokens | the modern sandbox model |
| Apps | SwiftUI / Kotlin Compose / **Tauri** / PWA + WebSerial flasher | current-generation on every platform |
| Maps | PMTiles (preferred) / MBTiles | current offline-tile standard |
| Cloud | Postgres + Redis + S3-compat; K8s + Terraform + Helm; OpenAPI 3.1; Rust-axum-vs-Node ADR gated | cloud-native baseline; language decided by evidence (S-21-028) |
| Supply chain | Reproducible builds, **SLSA-3**, CycloneDX SBOM, Sigstore/Rekor, cosign, HSM keys | exceeds CRA requirements; ahead of all surveyed competitors (`06_COMPETITIVE_LANDSCAPE.md` §3) |
| Testing | gtest/Unity, HIL racks, mesh simulator, fuzzing, mutation testing, 30-day soak, chaos | beyond industry norm for this device class |

Competitive superiority evidence: `06_COMPETITIVE_LANDSCAPE.md` §2–3 (Meshtastic, MeshCore, Garmin/Zoleo/Apple analysis) — SS-SP adopts every proven strength and closes every identified weakness (PSK crypto, node-count ceilings, fragmentation, cloud lock-in).

---

## 6. Final validation state (machine-checked)

| Check | Result |
|---|---|
| Stories / epics | **533 / 24** — index regenerated, `--check` OK (529 at initial PASS; +4 from HaLow dossier closures T-18..T-20) |
| Duplicate story IDs | 0 |
| Phantom PRD references | 0 |
| Invalid Meta fields (Type/Size/Prio/Const) | 0 |
| Stories in wrong epic directory | 0 |
| PRD requirement IDs defined ↔ referenced | **140 / 140 bidirectional** — zero orphans either direction |
| EPIC_INDEX ↔ epic directories | 24/24 |
| Firmware scaffold | compile-ready (partitions/sdkconfig/boot order verified); sole external dependency: ESP-IDF toolchain install |

---

## 7. Verdict

**PASS.** The portfolio is certified sufficient as the **sole source** for developing the entire SS-SP ecosystem, under the two properties that make that statement rigorous rather than rhetorical:

1. **Everything is either specified or assigned — nothing is silent.** Every one of the 45 normative contracts audited across three layer groups is now class A (explicit in a constitution/portfolio doc) or class B (a named story whose acceptance criteria pin the contract and whose Definition of Done forces RFC ratification, security sign-off, and CI-enforced conformance before merge). Class C is empty after T-01…T-17 closures.
2. **The build order is executable.** The epic DAG is acyclic; an implementing agent starts at EPIC-01 + EPIC-22/23 bootstrap, walks the M2 critical path (EPIC-02→03→06→07→08→09→10→11→12→13→14→15→16→19), and encounters every design decision either already made or explicitly staged as a Spike/RFC/ADR story (S-21-028, S-05-019, S-10-019, S-19-030, S-01-017) at the exact point it is needed.

The remaining honest caveat, by design: stories marked DRAFT still require their `Tasks:`/`Deps:` decomposition at READY promotion (methodology §2.7), and RFC/ADR stories produce decisions rather than presuppose them. That is not missing information — it is the portfolio's governed mechanism for making the remaining decisions with evidence, sign-off, and traceability.

*Maintenance rule: any future audit finding is appended to §4's ledger with a T-number and disposition; this document is the single place where "is the portfolio complete?" is answered.*
