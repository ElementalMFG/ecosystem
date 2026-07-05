<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# EPIC-12 — Stories

Format per `../../00_METHODOLOGY.md` §2.7. Meta lines are machine-parsed.

### S-12-001 — LXMF struct + wire serialisation
As a protocol engineer I want the LXMF message structure and wire serialisation implemented so that SS-SP messages are byte-compatible with the LXMF ecosystem.
- AC: encoder/decoder round-trips the LXMF fields (destination, source, timestamp, content, fields map) byte-exactly against published test vectors; messages serialised on-device parse in the reference LXMF implementation and vice versa; malformed inputs are rejected without crashes in fuzz testing
- Meta: Shard=A | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-MSG-01 | Const=C-02

### S-12-002 — Sender state machine + retry
As a protocol engineer I want a sender state machine with retry and store-and-forward fallback so that messages eventually deliver even when the recipient is unreachable.
- AC: direct delivery is attempted first and falls back to a propagation node when the recipient is offline per F-MSG-09; retry uses bounded exponential backoff and gives up into a user-visible failed state after the configured limit; state survives reboot (queued messages persist and resume)
- Meta: Shard=B | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-MSG-01, F-MSG-09 | Const=C-02

### S-12-003 — Receiver state machine + dedupe
As a protocol engineer I want a receiver state machine with duplicate detection so that retried or multi-path messages surface to the user exactly once.
- AC: the same message received via direct delivery and via a propagation node is stored and displayed once; dedupe window and its memory bound are documented and enforced; out-of-order arrivals are accepted and timestamped correctly
- Meta: Shard=B | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-MSG-01 | Const=C-02

### S-12-004 — Delivery receipt round-trip
As a device owner I want delivery receipts (and optional read receipts) round-tripped so that I know whether my message reached the other device.
- AC: delivery receipt returns to the sender end-to-end over LoRa RNS (exit criterion 4); read receipts are emitted only when the recipient has opted in per F-MSG-01; receipt loss does not cause message re-send (receipts are idempotent)
- Meta: Shard=C | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-MSG-01 | Const=C-02

### S-12-005 — Priority queue integration w/ SS-Link
As a protocol engineer I want LXMF priority classes (SOS, high, normal, bulk) mapped onto the SS-Link QoS queue so that urgent messages preempt bulk traffic on-air.
- AC: SOS-class LXMF messages preempt lower classes under load (exit criterion 5); class mapping to SS-Link QoS hints is specified and covered by tests; bulk-class transfers never delay an SOS beyond the NF-PERF-04 budget
- Meta: Shard=D | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-MSG-08, NF-PERF-04 | Const=C-02, C-08

### S-12-006 — Propagation-node role on HGW
As a neighbourhood-mesh enthusiast I want my Home Gateway device to act as an LXMF propagation node so that neighbours' messages are stored and forwarded while recipients are offline.
- AC: store-and-forward through an HGW-hosted propagation node works end-to-end (exit criterion 2); role is opt-in and bounded by a disk quota per F-HGW-07; stored messages are held encrypted and expire per retention policy; node announces its propagation service on the mesh
- Meta: Shard=E | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-HGW-07, F-MSG-09 | Const=C-08, C-02

### S-12-007 — Message storage encrypted at rest
As a security engineer I want the on-device message store encrypted with the device key so that no message content is recoverable from a captured device's flash.
- AC: message database is unreadable from a raw flash image (ciphertext only) per NF-SEC-02; no plaintext message content appears in logs by default per NF-SEC-03; store survives unclean power-down without corruption in the power-cycling test
- Meta: Shard=G | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=NF-SEC-02, NF-SEC-03 | Const=C-05

### S-12-008 — Read/unread + threading UI hooks
As a firmware engineer I want read/unread state and conversation threading exposed as UI hooks so that the Chats app can render conversations without touching LXMF internals.
- AC: per-message read/unread state persists across reboots; messages group into per-contact and per-group threads with stable ordering; hook API delivers change events the `ss_ui` Chats screen consumes without polling
- Meta: Shard=H | Type=Feature | Size=S | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-APP-01 | Const=C-02, C-03

### S-12-009 — Voice-note attachment (Codec2 3.2 kbps)
As a device owner I want voice notes carried as Codec2 3.2 kbps LXMF attachments so that spoken messages deliver even over the narrow LoRa bearer.
- AC: a recorded voice note encodes, transmits as an LXMF attachment, and plays back intelligibly on the receiving device (exit criterion 3); Codec2 code stays module-isolated per the licensing strategy noted in F-VOX-04; attachment size for a 10 s note fits the LoRa store-and-forward budget
- Meta: Shard=F | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-MSG-03, F-VOX-04 | Const=C-02, C-04

### S-12-010 — Small-file attachment (< 32 KB, chunked)
As a device owner I want small file attachments (< 32 KB) sent chunked over LXMF so that I can share waypoints, photos thumbnails, and configs across the mesh.
- AC: a < 32 KB file transfers chunked with reassembly hash-verified at the receiver; transfer resumes after a dropped link without restarting from chunk zero; oversize files are refused up-front with a clear user-facing error
- Meta: Shard=F | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-MSG-05 | Const=C-02

### S-12-011 — Group thread adapter w/ sender-key crypto
As a first responder I want group messages carried through LXMF with sender-key encryption so that my team channel stays confidential with efficient group fan-out.
- AC: group of ≥ 3 devices exchanges messages with per-group encryption per F-MSG-02; membership change triggers key update and evicted members cannot decrypt subsequent messages; sender-key layer receives formal review per risk R12-03 before status leaves review
- Meta: Shard=I | Type=Feature | Size=L | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-MSG-02, F-SEC-05 | Const=C-05, C-02

### S-12-012 — Retention policy + user quota
As a device owner I want configurable message retention and a storage quota so that the message store cannot grow until it fills device flash (risk R12-02).
- AC: retention policy deletes messages past the configured age or when the quota is reached, oldest-first with pinned-thread exemptions; user sees current usage against quota in settings; deletion is secure (no recoverable plaintext remnants) consistent with NF-SEC-02
- Meta: Shard=G | Type=Feature | Size=S | Prio=P0 | Status=DRAFT | SKU=★ | PRD=NF-SEC-02 | Const=C-02

### S-12-013 — Message search index (on-device)
As a device owner I want on-device message search so that I can find old messages without any cloud service.
- AC: search returns matches by content and contact across the encrypted store entirely on-device; index adds a bounded, documented storage overhead within the user quota; index updates incrementally without blocking message receive
- Meta: Shard=H | Type=Feature | Size=M | Prio=P2 | Status=DRAFT | SKU=★ | PRD=F-APP-01, NF-PRIV-02 | Const=C-00

### S-12-014 — LXMF version negotiation
As a protocol engineer I want LXMF version negotiation between peers so that spec drift does not silently break messaging (risk R12-01).
- AC: peers agree on the highest mutually supported LXMF version before message exchange; an unsupported-version peer produces a defined, user-visible incompatibility state instead of silent loss; negotiation behaviour is covered by tests against the reference implementation
- Meta: Shard=A | Type=Feature | Size=S | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-MSG-01 | Const=C-02

### S-12-015 — Interop test w/ Sideband client
As a protocol engineer I want automated interop tests against the Sideband client so that SS-SP messaging is proven compatible with the most-used LXMF application.
- AC: bidirectional 1:1 text exchange with delivery receipts succeeds between an SS-SP device and Sideband; store-and-forward via a shared propagation node works in both directions; suite runs in CI against the pinned Sideband version on every LXMF change
- Meta: Shard=A | Type=Ops | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-MSG-01, F-MSG-09 | Const=C-02

### S-12-016 — Interop test w/ Nomad Network
As a protocol engineer I want interop tests against Nomad Network so that compatibility is proven against a second independent LXMF client.
- AC: bidirectional text exchange succeeds between an SS-SP device and Nomad Network; incompatibilities are filed with minimal reproductions and tracked to closure; suite is runnable in CI on demand against a pinned version
- Meta: Shard=A | Type=Ops | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-MSG-01 | Const=C-02

### S-12-017 — LXMF RFC (RFC-0008)
As a protocol engineer I want RFC-0008 specifying SS-SP's LXMF profile (priority classes, attachments, propagation-node behaviour, version policy) so that the messaging layer is reviewable and stable for third-party implementers.
- AC: RFC-0008 covers the priority-class mapping, attachment framing, propagation-node semantics, and upstream-tracking/version-negotiation policy; RFC reaches ACCEPTED via the governance process; accepted RFC is referenced from the LXMF component docs and the certification test suite
- Meta: Shard=— | Type=RFC | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-MSG-01 | Const=C-02, C-06

### S-12-018 — LXMF reserved fields namespace + registry (RFC)
As a protocol engineer I want a reserved, RFC-governed namespace in the LXMF fields map so that structured third-party payloads (CoT/TAK, telemetry, future schemas) ride LXMF without ever forking the wire protocol.
- AC: registry document defines the reserved namespace, allocation rules, and collision policy, and is governed by the RFC process; encoder/decoder round-trips registered structured fields byte-exactly and unknown registry entries pass through untouched; registry entries are published so any LXMF implementation (Sideband, Nomad Network, third-party) can adopt them independently
- Meta: Shard=A | Type=RFC | Size=S | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-INT-02 | Const=C-02, C-06

### S-12-019 — CoT ↔ LXMF translation gateway
As a first responder I want Cursor-on-Target PLI and chat translated to and from LXMF at the mesh edge so that TAK clients and SS-SP devices share one live tactical picture.
- AC: CoT position (PLI) and GeoChat events map bidirectionally to registered LXMF fields per the S-12-018 registry with no loss of required CoT attributes; an ATAK client and an SS-SP device exchange positions and chat end-to-end over the mesh in an integration test; the mapping is published as an open RFC so non-SS-SP LXMF clients can implement it
- Meta: Shard=— | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-INT-01, F-INT-02 | Const=C-02

### S-12-020 — Propagation-node peering quotas + LXMF stamp economics
As a propagation-node operator I want quota-bounded peering and stamp-based spam economics so that no participant can exhaust the propagation network's storage or bandwidth at million-user scale.
- AC: per-peer storage and bandwidth quotas with negotiated sync windows are enforced, and a hostile-peer test shows a flooding peer is throttled without degrading service to compliant peers; LXMF stamps (sender proof-of-work) are supported wire-compatibly with the upstream LXMF stamp design so interop with Sideband/Nomad Network is preserved, with operator-configurable stamp cost for unsolicited delivery; retention tiers are expressible per peer/tenant so commercial propagation nodes can sell longer retention (entitlement via S-21-022) while the community default retention is never reduced; quota and stamp policies are published so third-party propagation nodes can adopt them
- Meta: Shard=— | Type=Feature | Size=L | Prio=P1 | Status=DRAFT | SKU=★ | PRD=NF-SCALE-06 | Const=C-02, C-04, C-OA
