<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# Omega concerns-ledger → portfolio projection (D-0021, 2026-07-09)

Plan of record for absorbing the PCB repo's closure artifacts —
`closure_work/UNIFIED_CONCERNS_LEDGER.md` (~88 concerns),
`closure_work/OPTIMIZATION_DECISIONS.md`, and
`closure_work/HW_SW_ALIGNMENT_AUDIT.md` — into this portfolio. The board they
audit is the **released ss-sp omega** (v69, SHA 054eaa8b…); see
`OMEGA_HW_BASELINE.md`. The HW repo confirms **zero open PCB work** — every
ledger round is software-repo work, tracked here.

## §1 · Decision D-0021 — re-projection rules (binding)

The ledger predates this repo's tier naming in its body text: it maps
concerns onto "EPIC-04 / Alpha / S-04-xxx" because the board was historically
called "SP2 Alpha". Per `NAMING.md` (HW repo, cross-repo binding) that board
is the **Omega**. Therefore:

1. **EPIC-05 owns the entire v69 Omega HAL**, including core platform
   bring-up (display, touch, audio, power, buttons, USB, SD, GNSS, haptics,
   HaLow/C6 radio plumbing, thermal/safety firmware). New shards J/K/L and
   stories S-05-021…S-05-039 carry it. EPIC-05's old dependency on "EPIC-04
   frozen" is inverted: Omega ships first; Alpha later reuses the shared
   platform components.
2. **EPIC-04 (Alpha) is preserved, not force-aligned.** Alpha is not locked
   (v14 "DO NOT FABRICATE"; v15 unverified). Ledger verdicts that would
   delete/rewrite Alpha stories from v69 evidence are **re-scoped to
   annotations** — capability is never dropped on unverified evidence
   (anti-rug-pull). Alpha stories get their hardware-empirical rewrite when
   an Alpha release package exists.
3. **Chip-level facts apply everywhere the chip appears** and are fixed
   outright in the founding docs: P4 has no on-die Wi-Fi/BLE (C6 UART
   bridge); C6 is 2.4 GHz-only; ESP32-S3 (Lite) has no H.264 encoder; P4 uses
   RISC-V PMP not MPU; P4 Secure Boot V2 profile is pinned RSA-3072-PSS.
4. **Board-level facts are proven only for Omega v69** (no IMU, no ALS, 4
   side keys, 480×480 I8080 panel, GT911, EA3059, routed SKY66423): asserted
   for Omega, annotated as TBD for Alpha.
5. Nothing in the HW repo is mutated; historical artifacts keep legacy names.

## §2 · Firmware must/should-fix (ledger FW-1…FW-19) → stories

| Ledger | Portfolio owner | Note |
|---|---|---|
| FW-1 + FW-8 (TX duty cap + case-temp model, IEC 62368-1) | **S-05-028** | EPIC-24 safety story cites it at next touch (SPEC-2) |
| FW-2 (GPIO34/35/36 boot-strap deferral) | **S-05-027** | |
| FW-3 (USB↔LCD GPIO26/27 arbitration) | **S-05-026** | also SPEC-7 AC on S-07-011 |
| FW-4 (JTAG/ROM-download lockdown, eFuse) | **S-05-033** | **T1 domain** (eFuse/secure-boot) |
| FW-5 (PA VSWR watchdog on pa_pdet) | **S-05-029** | pa_pdet IS routed (G1 correction) |
| FW-6 + FW-12 (low-batt shutdown; MAX17048 modeled current) | **S-05-024** | FW-12 fixes the "current" AC as *modeled* ΔV/Δt |
| FW-7 (I²C 100 kHz default, 400 kHz EVT gate) | **S-05-034** | pairs HW-4/5 + EVT-4 |
| FW-9 (NS4150B SD-pin gating) | **S-05-023** | |
| FW-10 (HaLow EU region loader + duty cycle) | **S-05-030** | region-blob **signing is T1** |
| FW-11 + FW-19 (ESP-Hosted-NG transport + wake latency) | **S-05-031** | ADR required |
| FW-13 (480×480 square UI layout + focus nav) | **S-05-039** | adds Omega descriptor to doc 03 |
| FW-14 (IMU stubs/no-op) | baseline + board_config | Omega caps already 0; no deletion — Lite keeps optional IMU module (S-03-036) |
| FW-15 (backlight PWM ≥ 20 kHz) | AC inside **S-05-021** | |
| FW-16 (GT911 D-27 strap sequence) | **S-05-022** | 0x5D vs BMM350 0x14 collision |
| FW-17 (BLE LTK confined to C6) | **S-05-032** | **T1 domain** (keys crossing a boundary) |
| FW-18 (voice CPU budget on P4) | annotation on S-14-008 | per-SKU per-stage budget |

New core-HAL stories with no FW-item (from the Present table): **S-05-021**
display, **S-05-025** EA3059/TP4056 power-tree, **S-05-035** buttons,
**S-05-036** microSD, **S-05-037** GNSS MIA-M10Q (L1-only), **S-05-038**
DRV2625 haptics.

## §3 · Doc fixes (ledger DOC-1…DOC-32) → verdicts

Fix-outright (chip facts / Omega-proven, per §1.3–1.4): DOC-2 (P4+S3 ROM
libs), DOC-3/4/5 (per-SKU secure-boot/MAC/PMP in doc 05), DOC-12
(`SS_CAP_HALOW_TX` example), DOC-13 (per-SKU FIPS path), DOC-19/20/21/22
(doc 08: C6 bridge, 2.4 GHz-only, no USB-host Ethernet), DOC-23 (IP65/66 for
Omega v1.0), DOC-28 (JPEG stills on Lite — S3 has no H.264), DOC-16 (doc 02
NFC qualified "where fitted — none on v1.0 boards").

Annotate/scope (plan claims on unlocked hardware, per §1.2): DOC-1 (MIA-M10Q
as platform precedent), DOC-6 (`hal_imu.h` stays — capability-flagged per
SKU), DOC-7/8/9/17 (ATECC608 → Lite D-0013 attachment + Omega rev-2),
DOC-10/11 (fingerprint → future-hardware-gated), DOC-14 (wake-on-motion /
dead-man → hardware-gated), DOC-15 (NFC → future board rev), DOC-24 (buttons:
Omega = 4 side keys; Lite = touch+HID; Alpha TBD), DOC-27/30 (S-04-023 spike
stays for Alpha, cites Omega precedent: MM8108 on **SDIO**), DOC-31
(S-04-019 HIL fixtures re-derived at Alpha lock), DOC-32 (dual-radio =
post-v1.0 respin).

Special: DOC-25/26 → **add** an Omega 480×480 layout descriptor to doc 03
(S-05-039); Alpha's descriptor waits for its lock. DOC-18 → an Alpha hardware
reference is authored **only from a released Alpha package** (deferred);
Omega's deep reference is S-05-020's output. DOC-29 → superseded (S-04-006
corrected 2026-07-09 to v69-release references + G1 correction).

Moot: AL-10/S-04-030 (story never existed).

## §4 · Spec gaps (SPEC-1…8) → owners

SPEC-1→S-05-029 · SPEC-2→S-05-028 (+EPIC-24 cite) · SPEC-3→S-05-030 ·
SPEC-4→S-05-022 · SPEC-5→new EPIC-07 story (dual-eFuse identity anchor: P4
UID + C6 MAC signed manifest — **T1**) · SPEC-6→S-05-031 · SPEC-7→AC note on
S-07-011 · SPEC-8→AC note on S-24-032.

## §5 · SKU retags (RETAG-1…14)

Mechanical `SKU=` narrowing where story text is LoRa-/cellular-specific;
capability for other SKUs is preserved via bearer-per-SKU annotations, never
silently dropped: S-10-002→L; LoRa-wire S-13 stories→L (paperwork stories
S-13-001/002/012/015/016 stay ★); S-14-004→L (+HaLow-MCS ladder note for
A/O); S-11-023→L (+HaLow scenario note); S-11-007→O (+per-interface timeout
note); S-19-020/022→L; S-22-022→L (+HaLow benchmark note); S-24-002→L;
S-17-021 + S-17.E LoRa-gateway stories→L. Annotation-only (stay ★):
S-12-004/S-12-009 (bearer-name/size templated per SKU), S-11-020 (HGW-only),
S-22-012 (per-SKU bearer set). S-24-005/006 → BLOCKED, rev-2-gated (D-0020).

## §6 · EVT / optional-HW / leave-as-is

EVT-1…9 → annotated into **S-05-016** (Omega HIL/EVT test-plan) as the EVT
matrix; each has a defined post-EVT fix path. HW-1…5 (BOM tweaks, ~$0.003)
→ recorded as the "Omega rev-2 optimization batch" in EPIC-05's baseline
section — no stories until a respin is decided. HW-6 (NTC bypass) is a
documented design choice; FW-8 owns thermal. AS-1…7 need no action (AS-1
range wording flows into marketing copy stories at their touch).

## §7 · Execution rounds & status

| Round | Content | Status |
|---|---|---|
| 0 | Baseline corrections (GT911, routed FEM, EA3059, PDM mic, buttons, USB mux) + this doc + D-0020/D-0021 recorded | **DONE 2026-07-09** |
| 1 | Founding-doc fixes (§3) + SKU retags (§5) | **DONE 2026-07-09** |
| 2 | EPIC-05 core-HAL stories S-05-021…039 filed; EPIC.md shards J/K/L; dependency inversion; S-05-016 EVT matrix; EPIC-07 identity story | **DONE 2026-07-09** |
| 3 | Story-run execution of S-05-020 spec-lock (board_config claims from v69), then the S-05-02x frontier per tier recipes | open |
| 4 | Firmware implementation per §2 (≈300 LOC + ADRs; T1 items via t1-pipeline) | open |
| 5 | EVT on real boards (S-05-016) → discharge ⚪ items | open |
| 6 | Compliance package (EPIC-24) with Omega scoped per D-0020/D-0021 | open |

## §8 · Open governance flags (owner decisions required)

- **RFC-0004 SL-5 tension:** the ratified scope lock says "v1.0 ships Lite +
  Alpha", but Omega is the released board and Alpha is not fabricatable.
  Reality is Lite + **Omega** first. Needs an owner-ratified RFC-0004
  amendment; not silently edited here.
- **Omega rev-2 respin:** owns the 16 BLOCKED EPIC-05 bearer/SE stories and
  the HW-1…5 optimization batch. No date; decision open.
- **Alpha lock:** EPIC-04's hardware-empirical rewrite (ledger AL-verdicts)
  fires only when an Alpha release package (v69-style) exists.
