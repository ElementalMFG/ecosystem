<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# EPIC-14 — Voice Subsystem

**Primary WG:** wg-firmware, wg-ui-ux · **Contributing:** wg-protocol
**Priority:** P1 · **SKU:** A, O · **Milestone:** M5

## Outcome
Half-duplex push-to-talk voice works over the mesh: user holds PTT, speaks, releases; audio is Opus- or Codec2-encoded, sent as prioritised LXMF-Voice frames over the best available bearer, decoded and played on receiver. Optional voice-note store-and-forward.

## Constitution
C-00 `00_MASTER_SOFTWARE_PLAN.md` §voice; C-08 `08_UNIVERSAL_CONNECTIVITY.md`; C-04 `04_LICENSING_AND_FORK_STRATEGY.md` §Codec2 licence isolation.

## Dependencies
EPIC-03/04 (audio HAL), EPIC-10, EPIC-12.

## Shards
- **S-14.A Opus encode/decode integration** — SILK narrowband.
- **S-14.B Codec2 encode/decode integration** — 1.2 / 3.2 kbps LoRa modes.
- **S-14.C PTT event pipeline** — button → capture → encode → send.
- **S-14.D Half-duplex bearer arbitration** — mute audio during TX on shared radio.
- **S-14.E Jitter buffer + PLC.**
- **S-14.F Echo suppression / sidetone.**
- **S-14.G Voice-note store-and-forward** — LXMF attachment path.
- **S-14.H VOX (voice-activity) trigger — optional.**
- **S-14.I DSP wake-word (Alpha only).**

## Exit criteria
1. Alpha ↔ Alpha half-duplex PTT over HaLow with < 250 ms end-to-end latency.
2. Codec2 3.2 kbps voice-note over LoRa decodable end-to-end.
3. Voice occupies < 20 % CPU on Alpha idle.

## Risks
| # | Risk | Mitigation |
|---|------|-----------|
| R14-01 | Latency budget blown by bearer overhead | Priority scheduling, dedicated queue |
| R14-02 | Opus MIT licence footprint | Audit + bundle notice |
| R14-03 | AEC quality on cheap mic | Board rev optimisation |
