<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# EPIC-14 — Stories

Format per `../../00_METHODOLOGY.md` §2.7. Meta lines are machine-parsed.

### S-14-001 — Opus encoder wrapper (SILK narrowband)
As a firmware engineer I want an Opus encoder wrapper tuned to the SILK narrowband low-bitrate profile so that captured PTT audio and voice notes fit the 6–24 kbps budget defined for mesh voice.
- AC: Encoder produces valid Opus frames at configurable 6–24 kbps from the audio-HAL capture path; encode of a 20 ms frame completes within its real-time deadline on Alpha; output decodes cleanly in reference libopus with no frame errors
- Meta: Shard=A | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=A,O | PRD=F-VOX-03, F-MSG-03 | Const=C-00

### S-14-002 — Opus decoder wrapper
As a firmware engineer I want an Opus decoder wrapper feeding the audio-HAL playback path so that received voice frames play back on the device speaker.
- AC: Decoder plays reference Opus streams at 6–24 kbps without artifacts on Alpha hardware; corrupt or truncated frames are handled via concealment hooks, never a crash; decode of a 20 ms frame completes within its real-time deadline
- Meta: Shard=A | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=A,O | PRD=F-VOX-03, F-MSG-03 | Const=C-00

### S-14-003 — Codec2 3.2 kbps integration
As a firmware engineer I want Codec2 3200 encode/decode integrated as a licence-isolated module so that intelligible voice notes can traverse LoRa where Opus bitrates do not fit.
- AC: Codec2 3200 round-trip (encode → decode) is intelligible in a scored listening test; Codec2 is built as an isolated loadable module per the C-04 licence-isolation requirement, with no static linkage into BSL-licensed firmware code; encoded frame size and rate match the LoRa voice-mode budget in `08_UNIVERSAL_CONNECTIVITY.md`
- Meta: Shard=B | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=A,O | PRD=F-VOX-04 | Const=C-00, C-04

### S-14-004 — Codec2 1600/700C for LoRa constrained
As a firmware engineer I want the Codec2 1600 and 700C modes available so that emergency voice remains possible on the most constrained LoRa links.
- AC: Codec2 1600 and 700C encode/decode round-trips pass an intelligibility listening test for short emergency phrases; mode selection falls back 3200 → 1600 → 700C automatically when the bearer reports insufficient throughput, matching the rate ladder in `02_PROTOCOL_STACK.md` §5.4; module reuses the S-14-003 licence-isolation boundary
- Meta: Shard=B | Type=Feature | Size=M | Prio=P2 | Status=DRAFT | SKU=A,O | PRD=F-VOX-04 | Const=C-00, C-04

### S-14-005 — PTT event pipeline
As a device owner I want hold-to-talk capture that encodes and transmits while I speak so that pressing PTT is all it takes to send live voice.
- AC: PTT press starts capture → encode → send within 150 ms and release flushes the final frame; frames are sent as prioritised LXMF-Voice traffic ahead of bulk queue items; a mid-transmission bearer loss is surfaced to the user and the pipeline recovers without reboot
- Meta: Shard=C | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=A,O | PRD=F-MSG-04, F-VOX-03 | Const=C-00

### S-14-006 — Half-duplex bearer mutex
As a firmware engineer I want a half-duplex arbitration mutex on shared radios so that receive audio is muted during transmit and the radio is never asked to do both.
- AC: On a shared radio, playback is muted within 20 ms of PTT-down and restored on PTT-up; TX/RX arbitration never deadlocks under a stress test of rapid PTT toggling; arbitration state is exposed to the UI so the user sees TX vs RX
- Meta: Shard=D | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=A,O | PRD=F-MSG-04 | Const=C-00, C-08

### S-14-007 — Jitter buffer + PLC
As a firmware engineer I want an adaptive jitter buffer with packet-loss concealment so that live voice stays intelligible over lossy mesh bearers.
- AC: Speech remains intelligible at 10% simulated packet loss and 100 ms jitter in bench tests; buffer depth adapts to measured jitter without exceeding the end-to-end latency budget contribution assigned to it; lost frames trigger PLC instead of silence gaps
- Meta: Shard=E | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=A,O | PRD=F-MSG-04, F-VOX-03 | Const=C-00

### S-14-008 — Echo suppression (WebRTC AEC3 lite)
As a device owner I want echo suppression on the speaker/mic path so that the far end does not hear their own voice echoed back.
- AC: Measured echo return loss enhancement meets the target set in the voice design note on Alpha hardware; suppression stays within the EPIC-14 CPU budget (voice subsystem < 20% CPU on Alpha); double-talk does not clip the near-end speaker unacceptably in listening tests
- Meta: Shard=F | Type=Feature | Size=L | Prio=P1 | Status=DRAFT | SKU=A,O | PRD=F-MSG-04 | Const=C-00

### S-14-009 — Sidetone playback
As a device owner I want low-latency sidetone of my own voice while transmitting so that PTT feels live and I speak at the right level.
- AC: Sidetone latency from mic to speaker is ≤ 30 ms; sidetone level is user-adjustable including fully off; sidetone is automatically disabled when echo risk is detected on open-speaker use
- Meta: Shard=F | Type=Feature | Size=S | Prio=P2 | Status=DRAFT | SKU=A,O | PRD=F-MSG-04 | Const=C-00

### S-14-010 — Voice-note record → LXMF attachment
As a device owner I want to record a voice note that is sent as an LXMF attachment so that voice reaches recipients who are offline or beyond live-voice range.
- AC: Notes up to 60 s record, Opus-encode, and enqueue as LXMF attachments that store-and-forward like any message; recording is cancellable and partial recordings leave no orphaned data; on LoRa-only paths the note is offered in Codec2 mode when the Opus payload will not fit
- Meta: Shard=G | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=A,O | PRD=F-MSG-03, F-VOX-03, F-VOX-04 | Const=C-00

### S-14-011 — Voice-note playback UI
As a UI engineer I want an in-thread voice-note player widget so that received voice notes can be played, paused, and scrubbed from the Messages app.
- AC: Voice notes render in-thread with duration, play/pause, and scrub controls built on `ss_ui` widgets; playback routes through the half-duplex mutex so it never talks over an active PTT TX; unplayed notes carry a visible unread indicator
- Meta: Shard=G | Type=Feature | Size=S | Prio=P1 | Status=DRAFT | SKU=A,O | PRD=F-MSG-03 | Const=C-00, C-03

### S-14-012 — VOX trigger option
As a device owner I want an optional voice-activated transmit mode so that I can operate hands-free when PTT is impractical.
- AC: VOX opens TX within 200 ms of speech onset and closes after a configurable hang time; trigger threshold is user-adjustable and VOX is off by default; false triggers from ambient noise stay below the rate agreed in the design note during bench noise tests
- Meta: Shard=H | Type=Feature | Size=S | Prio=P2 | Status=DRAFT | SKU=A,O | PRD=F-MSG-04 | Const=C-00

### S-14-013 — Wake-word (Alpha DSP)
As a device owner I want an on-device wake word running on the Alpha DSP so that I can invoke voice command entry without touching the device.
- AC: Wake word detects at ≥ 90% true-positive / ≤ 2 false-accepts per 24 h in bench audio testing; detection runs entirely on-device on the Alpha DSP with no audio leaving the device; feature is opt-in and fully disabled (no mic sampling) when off
- Meta: Shard=I | Type=Feature | Size=L | Prio=P2 | Status=DRAFT | SKU=A | PRD=F-VOX-01 | Const=C-00, C-05

### S-14-014 — CPU + latency instrumentation
As a firmware engineer I want continuous CPU and end-to-end latency instrumentation of the voice pipeline so that the EPIC-14 exit budgets are measurable and regressions are caught.
- AC: Instrumentation reports per-stage latency (capture, encode, transport, decode, playback) and total CPU share of the voice subsystem; CI perf job fails when Alpha idle voice CPU exceeds 20% or PTT end-to-end latency exceeds 250 ms over HaLow; metrics are viewable in the on-device diagnostics screen
- Meta: Shard=— | Type=Ops | Size=S | Prio=P1 | Status=DRAFT | SKU=A,O | PRD=NF-PERF | Const=C-00

### S-14-015 — Interop test Alpha ↔ Alpha over HaLow
As a firmware engineer I want an automated Alpha-to-Alpha PTT interop test over HaLow so that the epic's primary exit criterion is verified on real hardware.
- AC: Hardware-in-loop rig runs a scripted PTT exchange between two Alphas over HaLow and asserts end-to-end latency < 250 ms; audio round-trip passes an automated intelligibility (PESQ-style) score threshold; test runs in CI on every voice-subsystem change
- Meta: Shard=— | Type=Ops | Size=M | Prio=P1 | Status=DRAFT | SKU=A,O | PRD=F-MSG-04, NF-PERF | Const=C-00, C-08

### S-14-016 — Interop test Codec2 voice-note LoRa
As a firmware engineer I want an automated Codec2 voice-note test over LoRa so that the constrained-bearer voice path is proven end-to-end.
- AC: A Codec2 3.2 kbps voice note sent over a real LoRa link decodes and plays intelligibly on the receiving device; test asserts EU 868 duty-cycle limits are respected during transfer; failure produces captured on-air frames as a debugging artifact
- Meta: Shard=B | Type=Ops | Size=M | Prio=P1 | Status=DRAFT | SKU=A,O | PRD=F-VOX-04, NF-REG | Const=C-00, C-08

### S-14-017 — Licence audit (Opus/Codec2)
As a firmware engineer I want a licence audit of the Opus and Codec2 integrations so that the BSD (Opus) and LGPL (Codec2) obligations are met before ship.
- AC: Audit confirms Opus attribution/notice bundling and records it in the SBOM; audit confirms Codec2 LGPL isolation as a loadable module per `04_LICENSING_AND_FORK_STRATEGY.md` with relink ability preserved; wg-legal sign-off is recorded in the PR
- Meta: Shard=— | Type=Task | Size=S | Prio=P0 | Status=DRAFT | SKU=A,O | PRD=F-VOX-03, F-VOX-04 | Const=C-04, C-00

### S-14-018 — Group PTT relay node (multicast → unicast fan-out)
As a first responder I want an Alpha/Omega device to act as a group PTT relay that re-fans group voice as per-member unicast streams so that group voice stays reliable on links where multicast delivery is weak.
- AC: relay role is opt-in and announced on the mesh so members discover it automatically; a ≥ 3-member group sustains PTT through the relay with end-to-end latency within the F-VOX-02 budget; relay failure degrades gracefully back to direct group delivery with no stuck-mic or lost-floor state; per-member fan-out count is bounded and documented for the relay device class
- Meta: Shard=— | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=A,O | PRD=F-VOX-05 | Const=C-00, C-08

### S-14-019 — Real-time 1:1 voice calls over IP-capable paths
As a device owner I want full-duplex voice calls with mesh peers over HaLow/Wi-Fi/Internet paths so that conversation-grade voice works wherever the path supports it, without any telephony provider.
- AC: a full-duplex Opus call completes between two nodes over HaLow, Wi-Fi, and Internet RNS paths with mouth-to-ear latency inside a documented budget; call setup/teardown is signalled over the RNS link with encryption inherited from the link layer (no separate call-crypto scheme); when path quality falls below the codec floor the call degrades gracefully to PTT/voice-message with clear user notice, never a silent drop; app ↔ app calls with no SS-SP hardware pass the same test suite
- Meta: Shard=— | Type=Feature | Size=L | Prio=P2 | Status=DRAFT | SKU=A,O | PRD=F-VOX-06 | Const=C-00, C-08
