<!-- SPDX-License-Identifier: CC-BY-4.0 -->
- Title: v1.0 scope lock — device identity, feature truth, and compatibility guards
- Author(s): SS-SP program lead
- Shepherd: wg-community (bootstrap: program lead)
- Status: ACCEPTED (owner ratified SL-1..SL-7 on 2026-07-08; D-0019)
- Start Date: 2026-07-07
- Feature area (WG): cross-cutting / product scope
- Requires: 0001, 0003
- Supersedes: —

# Summary

One ratified statement of what SS-SP v1.0 IS — a **sovereign multi-band mesh
communicator and universal network node** in a pager form-factor — resolving
every scope contradiction found in the 2026-07-07 drift audit, and binding
current engineering to compatibility guards so no near-term work forecloses
a declared future capability.

# Motivation

A portfolio-wide audit (verified against the story set, not just prose)
found the capability set essentially complete — HaLow/Wi-Fi/BLE/LoRa
(EPIC-03/04/05/10/11), GPS+compass (EPIC-04/05/16), on-device STT/TTS
(EPIC-14, incl. wake-word S-14-013), PTT + full-duplex voice (EPIC-14),
messaging + file transfer (EPIC-12), router/hotspot/WAP/bridge/gateway/
extender/universal-node roles (EPIC-17 F-HGW-01..07 + F-EUD-01..03),
multi-band mesh + tunneling (EPIC-10/11/13/17), and companion apps
(EPIC-19) — while the *framing documents* contradict it and each other.
Drift, not missing capability, is the risk.

# Decision sheet (RATIFIED by owner, 2026-07-08 — D-0019)

| # | Question | Decision |
|---|---|---|
| SL-1 | Device framing | **RATIFIED, owner wording:** primary identity = "sovereign multi-band mesh communicator + universal node"; descriptor = "a multi-band Wi-Fi (2.4/5 GHz) / HaLow / BLE smartphone-class device in a pager form factor". "Smart pager" retired as the primary noun in README/PRD §1; PRD §8's blanket "not a smartphone replacement" narrowed to "no cellular-voice or app-store replacement claim in v1.0" |
| SL-2 | On-device LLM | **RATIFIED as recommended:** `ss_ai` stays capability-only scaffold; STT/TTS are the v1.0 AI products; no product LLM promise before v2.x |
| SL-3 | Video calling/streaming | **RATIFIED as recommended:** signed-WASM-plugin path (EPIC-18) post-v1.0; "micro-frames" language becomes "thumbnail attachments" |
| SL-4 | On-device web browsing | **RATIFIED, refined:** browsing is an APPLICATION-LAYER capability, never core firmware. Delivery: (a) v1.0 — browse *through* the device via EPIC-17 gateway/tether roles; (b) browser-type and other smartphone-class apps ride the signed-plugin platform where hardware permits; (c) full on-device browser targets Omega (Linux SoM, v1.x). See compatibility guard 6 |
| SL-5 | Omega in v1.0 | **RATIFIED as recommended:** v1.0 ships Lite + Alpha; Omega is v1.x with its own spec-lock story *(amended 2026-07-09 — see Amendments A1)* |
| SL-6 | SDK descope | **RATIFIED as recommended:** v1.0 SDKs = C, Rust, Python; TypeScript + Dart in v1.1 |
| SL-7 | Lite honesty callouts | **RATIFIED as recommended:** half-duplex PTT + no fuel-gauge stated in the README variant matrix |

Corrections to the source audit, from verification: wake-word is NOT a gap
(S-14-013, Alpha DSP, opt-in); EPIC-13 Meshtastic descope is deferred to its
own epic-start review rather than decided blind here.

# Compatibility guards (binding on current engineering)

These ensure "nothing done now causes problems down the road":

1. **Versioned surfaces**: every new persistent or wire-adjacent surface
   declares a version field and inherits RFC-0003 windows (already true:
   ss_nvs `__ver`, frame formats, plugin ABI, node-profile schema).
2. **Capability-flagged HAL**: app code queries `ss_hal_has_cap()` — never
   board macros — so universal-node roles remain board-portable
   (Universal Test rule, already enforced).
3. **Partition headroom**: Lite's A/B 6 MB app slots + ~3.9 MB storage must
   retain ≥25% headroom through v1.0 so the WASM plugin store (EPIC-18) and
   AI model growth stay deployable; the boot log's dump fill-% pattern
   extends to image-size tracking (T4 story when CI artifacts exist).
4. **Pin-sharing ledger**: shared-pin constraints (GPIO0 recovery/LoRa CS,
   HaLow mux GPIO45, mic/LoRa SPI overlap) live in `ENGINEERING_LOG.md` +
   board headers; any new peripheral claim must check it (already logged).
5. **Role features are software**: router/hotspot/WAP/gateway/extender are
   EPIC-17 runtime roles over existing radios — no board change may remove
   a radio path that EPIC-17 declares (doc 08 is the authority).
6. **Application-layer capability (SL-4 owner mandate)**: the EPIC-18
   signed-plugin ABI and the EPIC-15 UI shell MUST be designed to host the
   smartphone-app class — networked (sockets over the device's IP paths),
   UI-rendering (shell surface access), storage-scoped plugins — so
   browser-type and comparable applications are deployable where hardware
   permits without core-firmware changes. The plugin ABI is already an
   RFC-0003 versioned surface; this guard binds its *capability floor*.
   Additive only: no existing EPIC-15/18 story is narrowed by this guard.

# Mandated doc-drift edits (become T4 stories on acceptance)

README + PRD §1/§8 reframe (SL-1); PRD §5 LLM paragraph (SL-2);
"micro-frames"→"thumbnail attachments" sweep (SL-3); README variant matrix
(SL-5/SL-7); PRD §2.3 device-roles paragraph enumerating
router/hotspot/WAP/bridge/gateway/extender/mesh-node with F-HGW/F-EUD story
links.

# Security considerations

SL-4 (no browser) removes the largest proposed attack surface. SL-3 keeps
video behind the signed-plugin sandbox (EPIC-18) rather than core. Nothing
here touches keys, wire formats, or T1 surfaces.

# Backward compatibility & migration

Documentation-truth changes only; no shipped artifact exists yet. RFC-0003
governs all versioned surfaces going forward.

# Unresolved questions

None once the decision sheet is ratified.

# Implementation plan

Ratified 2026-07-08: status ACCEPTED, renumbered 0004, decision ledger
D-0019. The mandated doc-drift edits are queued as S-01-019 (T4 sweep,
DRAFT). Everything is additive: no story is dropped or narrowed; SL-6
moves two SDK stories to the v1.1 train without deleting them; guard 6
binds a capability floor onto EPIC-15/18 without changing their stories.
S-01-018 → DONE.

# Amendments

## A1 — SL-5 device lineup rebased on hardware reality (2026-07-09)

Ratified via the bootstrap governance provision (program lead as SC per
D-0015), owner-directed 2026-07-09; recorded as part of D-0021/D-0023.

SL-5's ratified text ("v1.0 ships Lite + Alpha; Omega is v1.x") predates two
hardware facts: the **ss-sp omega board is released** (PCB `release_v69/`,
2026-07-08, D-0020) and the **proprietary P4 Alpha design is not
fabricatable** (v14 "FUNCTIONALLY DEAD — DO NOT FABRICATE"; v15 unverified).
SL-5 is amended to:

> **SL-5 (as amended):** v1.0 ships on the **Elecrow-based Lite platform** —
> ss-sp lite and its ratified module variants, including the provisional
> C6-coprocessor variant that may ship as **ss-sp alpha v1** (D-0023).
> **ss-sp omega** proceeds on its own v1.x track from the released v69 board
> (spec-lock S-05-020). The **proprietary P4 Alpha** is deferred to its own
> hardware lock and re-enters the release train when a v69-style release
> package exists.

Unchanged: SL-1..SL-4, SL-6, SL-7 and all compatibility guards. Nothing is
dropped or narrowed — the amendment names which hardware carries the already
ratified scope. Cellular remains outside v1.0 scope entirely (D-0024).
