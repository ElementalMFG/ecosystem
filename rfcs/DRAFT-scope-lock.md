<!-- SPDX-License-Identifier: CC-BY-4.0 -->
- Title: v1.0 scope lock — device identity, feature truth, and compatibility guards
- Author(s): SS-SP program lead
- Shepherd: wg-community (bootstrap: program lead)
- Status: DRAFT (awaiting owner ratification of the decision sheet below)
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

# Decision sheet (owner ratifies; recommendations pre-filled)

| # | Question | Recommendation | Owner decision |
|---|---|---|---|
| SL-1 | Device framing: retire "smart pager" as the primary noun in README/PRD §1; primary identity = "sovereign multi-band mesh communicator + universal node"; "pager form-factor" stays as descriptor; PRD §8's blanket "not a smartphone replacement" narrowed to "no cellular-voice/app-store replacement claim" | ADOPT | [OWNER] |
| SL-2 | On-device LLM: keep `ss_ai` as a capability-only scaffold (STT/TTS are the v1.0 products); no product-level LLM promise before v2.x; delete the PRD §5 vs architecture contradiction by stating exactly this | ADOPT | [OWNER] |
| SL-3 | Video calling/streaming: not core in v1.0; formalize as a signed-WASM-plugin path (EPIC-18) post-v1.0; rename "live video micro-frames" language to "thumbnail attachments" everywhere | ADOPT | [OWNER] |
| SL-4 | On-device web browsing: out — the paired companion + IP-tether/gateway modes (EPIC-17/19) are the browsing path; device-side browser adds enormous attack surface for negative value on this hardware | ADOPT (out) | [OWNER] |
| SL-5 | Omega in v1.0: ship v1.0 as Lite + Alpha; Omega (RISC-V/Linux SoM, cellular/LEO) is v1.x with its own spec-lock story; README variant matrix says so | ADOPT | [OWNER] |
| SL-6 | SDK descope: v1.0 SDKs = C, Rust, Python; TypeScript + Dart move to v1.1 (companion apps use their native SDKs; no capability lost) | ADOPT | [OWNER] |
| SL-7 | Lite honesty callouts: half-duplex PTT and no battery fuel-gauge stated plainly in README variant matrix | ADOPT | [OWNER] |

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

Owner ratifies SL-1..SL-7 → status ACCEPTED, renumber per
`rfcs/README.md` convention, decision-ledger entry (next free D-number) →
T4 doc-edit stories queued → S-01-018 DONE.
