<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# EPIC-19 — Stories

Format per `../../00_METHODOLOGY.md` §2.7. Meta lines are machine-parsed.

### S-19-001 — Shared TS core: protocol client
As a mobile app developer I want a shared TypeScript protocol client (SS-Link/RNS/LXMF framing over BLE and LAN) so that all four app shells talk to the device through one tested code path.
- AC: core client pairs with a device over BLE and completes an LXMF message round-trip in integration tests; the identical core package is consumed by iOS, Android, desktop, and web builds with no platform-specific protocol forks; protocol failures surface as typed errors with documented retry semantics
- Meta: Shard=A | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-CA-05 | Const=C-00

### S-19-002 — Shared TS core: crypto wrappers
As a mobile app developer I want shared crypto wrappers (Ed25519/X25519 identity, session encryption) in the TS core so that every app performs identity verification and message crypto identically and auditably.
- AC: wrappers pass the shared cross-implementation test vectors also used by firmware; exactly one implementation per primitive is used across all four shells; key material is held only in platform secure storage (Keychain/Keystore/equivalent), verified by test
- Meta: Shard=A | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-CA-05 | Const=C-00, C-05

### S-19-003 — Shared TS core: sync engine
As a mobile app developer I want a resumable device↔app sync engine in the shared core so that contacts and messages stay consistent across connection drops.
- AC: sync resumes after BLE disconnect without message loss or duplication; a 500-message backlog syncs over BLE within a documented time budget in test; conflict resolution is deterministic with the device as authority and is covered by unit tests
- Meta: Shard=A | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-CA-05 | Const=C-00

### S-19-004 — iOS SwiftUI shell + BLE service
As a mobile app developer I want a SwiftUI shell over the shared core with a background BLE service so that iPhone users can pair, sync, and message within App Store rules.
- AC: pairing plus first message round-trip completes in ≤ 90 s from install on a reference iPhone; BLE reconnect works under iOS background-execution constraints; the shell contains no protocol or crypto logic outside the shared core
- Meta: Shard=B | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-CA-01 | Const=C-00

### S-19-005 — Android Compose shell + BLE service
As a mobile app developer I want a Jetpack Compose shell over the shared core with a foreground BLE service so that Android users can pair, sync, and message within Play policy.
- AC: pairing plus first message round-trip completes in ≤ 90 s from install on a reference Android device; foreground service survives doze mode with reconnect verified by test; the shell contains no protocol or crypto logic outside the shared core
- Meta: Shard=C | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-CA-02 | Const=C-00

### S-19-006 — Desktop Tauri shell (macOS/Win/Linux)
As a mobile app developer I want a single Tauri codebase producing macOS, Windows, and Linux desktop apps so that desktop users get the big-screen companion without per-OS forks.
- AC: one Tauri source tree builds signed artifacts for macOS, Windows, and Linux in CI; BLE/serial pairing verified on all three OSes; binary-size and memory budgets documented per platform
- Meta: Shard=D | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-CA-03 | Const=C-00

### S-19-007 — Web app (React) w/ Web-BLE fallback
As a mobile app developer I want a progressive web app using Web-BLE with graceful fallback so that users on any modern browser can at least provision a device.
- AC: provisioning flow works on Chromium-family browsers via Web-BLE; browsers without Web-BLE get a functional LAN/WebSerial path or a clear capability message, never a broken flow; app is installable as a PWA
- Meta: Shard=E | Type=Feature | Size=L | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-CA-04 | Const=C-00

### S-19-008 — Pairing UX + QR key-fingerprint
As a device owner I want guided BLE pairing with QR key-fingerprint verification so that I can trust the app-device link is not intercepted.
- AC: pairing shows the same fingerprint on device and app and requires explicit user confirmation; QR path verifies the full Ed25519 fingerprint, not a truncated form; verification failure aborts pairing with clear recovery guidance
- Meta: Shard=F | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-APP-07 | Const=C-00, C-05

### S-19-009 — Contact sync flow
As a device owner I want my contact book (with verified-fingerprint state) synced between device and app so that contacts and their trust status are consistent everywhere.
- AC: contacts sync bidirectionally including verification state; verification status never upgrades silently during sync (verified-on-device stays authoritative); contact deletion propagates to the paired side
- Meta: Shard=A | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-APP-02, F-CA-05 | Const=C-00

### S-19-010 — Message mirroring policy setting
As a device owner I want an explicit inbox-mirroring policy (off / metadata-only / full) so that message content only leaves the device when I opt in.
- AC: default is mirroring off; changing the policy requires on-device confirmation; enforcement lives in the sync engine and a test proves no message body crosses the link when mirroring is off
- Meta: Shard=H | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=NF-PRIV-02 | Const=C-00, C-05

### S-19-011 — Key backup UX (seed + cloud E2EE)
As a device owner I want to back up my identity seed to written recovery words and optionally to E2EE cloud storage so that losing the device does not lose my identity.
- AC: seed export requires PIN/biometric re-authentication; cloud backup is encrypted client-side and the server verifiably never receives key material or the passphrase; an unassisted usability test participant completes backup without support
- Meta: Shard=G | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-00, C-05

### S-19-012 — Restore-from-cloud on second device
As a device owner I want to restore my identity from the E2EE cloud backup onto a second device so that device replacement is painless.
- AC: restore on a second device reproduces the identity and re-establishes contacts end-to-end (epic exit criterion 4); a wrong passphrase yields no partial information and rate-limits retries; the procedure for revoking the lost device is documented in the flow
- Meta: Shard=G | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-00, C-05

### S-19-013 — Fleet-Console operator role (view fleet)
As a fleet admin I want a read-only fleet view inside the companion apps so that I can check device health from my phone without opening the full console.
- AC: operator authenticates via Fleet Console tenant RBAC; fleet view lists devices with status, firmware version, and last-seen; view-only role exposes no write operations, verified by API-permission test
- Meta: Shard=I | Type=Feature | Size=L | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-CL-01 | Const=C-00, C-07

### S-19-014 — Provisioning flow via app
As a device owner I want an app-guided first-boot provisioning flow so that a new device is keyed, named, and connected in minutes.
- AC: flow implements companion-app-guided first boot with all keys generated on-device, never in the app; guided pairing completes within the 90 s epic budget; the standalone (no-app) provisioning path remains functional and is regression-tested
- Meta: Shard=F | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-APP-07 | Const=C-00, C-05

### S-19-015 — Push-notification bridge (via encrypted relay)
As a device owner I want push notifications relayed through an encrypted bridge so that my phone alerts me to new messages without any relay seeing plaintext.
- AC: the relay handles only opaque ciphertext plus a wake token — no plaintext, contact, or location data (per NF-PRIV-02); APNs/FCM wake delivers a notification within a documented latency budget in test; the bridge can be fully disabled by the user
- Meta: Shard=H | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-CA-01, F-CA-02, NF-PRIV-02 | Const=C-00, C-05

### S-19-016 — CI matrix for iOS, Android, macOS, Windows, Linux, Web
As a release manager I want a CI matrix building and testing all six app targets on every PR so that cross-platform drift (risk R19-03) is caught before merge.
- AC: PR pipeline builds iOS, Android, macOS, Windows, Linux, and web targets; shared-core test coverage floor is enforced in the pipeline; failure of any target blocks merge per governance CI rules
- Meta: Shard=— | Type=Ops | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-CA-05 | Const=C-00, C-06

### S-19-017 — TestFlight + Play beta releases
As a release manager I want automated beta distribution to TestFlight and Play Console so that testers get builds without manual store uploads.
- AC: tagged builds upload automatically to TestFlight and Play internal track with generated release notes; a store-policy compliance checklist (risk R19-01) is attached to each beta release; epic exit criterion 1 (both betas live) is demonstrably met
- Meta: Shard=B, C | Type=Ops | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-CA-01, F-CA-02 | Const=C-00

### S-19-018 — Accessibility audit (VoiceOver, TalkBack)
As a compliance officer I want a VoiceOver/TalkBack accessibility audit of the companion apps so that we meet WCAG 2.2 AA before beta widens.
- AC: audit covers pairing, chat, backup, and provisioning flows on iOS and Android; every P0/P1 finding is fixed or has a recorded waiver; WCAG 2.2 AA checklist results are archived with the audit
- Meta: Shard=— | Type=Task | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=NF-A11Y-01 | Const=C-00

### S-19-019 — Localisation infra (EN/ES/FR/DE/PT/ZH)
As a community manager I want string extraction and runtime locale switching for EN/ES/FR/DE/PT/ZH so that translators can localise the apps without code changes.
- AC: all user-facing strings are externalised into the localisation pipeline; a pseudo-locale build reveals zero hard-coded strings; locale switches at runtime without app restart
- Meta: Shard=— | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=NF-L10N-01 | Const=C-00

### S-19-020 — ATAK plugin (CoT PLI + chat via device)
As a first responder I want an open-source ATAK plugin that exchanges CoT PLI and GeoChat with my SS-SP over the tether/BLE link so that my TAK client and the mesh share one tactical picture using the published gateway mapping.
- AC: ATAK plugin exchanges position and chat with mesh peers end-to-end through the device using the S-12-019 CoT ↔ LXMF mapping; plugin consumes only the public device API (API parity — no private endpoints); plugin is published open source with the mapping RFC referenced so WinTAK/iTAK ports need no SS-SP cooperation; degraded-link behaviour (LoRa-only path) is documented with expected PLI refresh limits; D-0021: degraded-path bearer is LoRa on L, HaLow on A/O
- Meta: Shard=— | Type=Feature | Size=L | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-INT-01, F-INT-02 | Const=C-00, C-02

### S-19-021 — Standalone node mode (app as full RNS/LXMF node)
As an app user without SS-SP hardware I want the app to operate as a full mesh node over my phone's or PC's own connectivity so that I can message, send voice notes, and transfer data with mesh peers using nothing but the app.
- AC: with no paired device, the app exchanges LXMF messages with a Python RNS node, a Sideband client, and an SS-SP device over Wi-Fi/Internet via the embedded `ss_node_core`; identity keys live in platform secure storage and reuse the existing backup/restore flows; switching companion ↔ standalone mode is explicit and preserves message history and contacts; background behaviour is documented and compliant with iOS/Android background-execution and store policies
- Meta: Shard=A | Type=Feature | Size=L | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-NODE-02 | Const=C-00, C-02

### S-19-022 — Gateway mode (app bridges its uplinks into the mesh)
As a device owner I want my phone to bridge its Internet/cell/Wi-Fi connectivity into the mesh so that my device reaches distant peers even when no HaLow or LoRa path exists — and my phone reaches the mesh through the device when it has no signal.
- AC: with gateway mode enabled, a device whose only link is BLE to the phone reaches a remote Internet RNS transport node end-to-end in an integration test, and the reverse path (phone → device LoRa → peer) also passes; the role is opt-in with a persistent visible indicator and per-session data counters; cellular uplink is marked metered with bulk traffic classes deferred per bearer policy; disabling the role tears down cleanly with mesh routes re-converging in < 30 s; D-0021: reverse-path bearer is LoRa on L, HaLow on A/O
- Meta: Shard=— | Type=Feature | Size=L | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-NODE-03 | Const=C-00, C-08

### S-19-023 — Headless node daemon (Linux/macOS/Windows/Docker)
As a community mesh operator I want a packaged headless daemon embedding `ss_node_core` so that PCs, servers, and routers become mesh transport and propagation nodes with no GUI.
- AC: daemon runs as an RNS transport node with an optional LXMF propagation-node role bounded by a configurable disk quota; systemd unit and Docker image are published with a documented config file and safe defaults; interop is verified against a device, an app node, and the Python reference in CI; an OpenWRT-class router build is documented as a supported community target
- Meta: Shard=— | Type=Feature | Size=M | Prio=P2 | Status=DRAFT | SKU=★ | PRD=F-NODE-05 | Const=C-00, C-08

### S-19-024 — Android Auto + Apple CarPlay projection
As a rider or driver I want the app projected to my vehicle head unit so that message readout, voice reply, PTT, SOS, and the group-ride map are usable hands-free on the move.
- AC: message readout with voice reply, PTT, an SOS shortcut, and the group-ride map render in Android Auto and CarPlay using each platform's approved templates; driver-distraction rules are honoured (no free-text entry while moving); projection works in both companion and standalone node modes; both store review programmes accept the app
- Meta: Shard=— | Type=Feature | Size=M | Prio=P2 | Status=DRAFT | SKU=★ | PRD=F-ECO-02 | Const=C-00

### S-19-025 — Garmin Connect IQ companion (watch)
As an outdoors user I want a Garmin watch widget showing mesh activity so that I can see unread messages, trigger SOS, and follow peer bearing without touching my phone or device.
- AC: Connect IQ widget/data field shows unread-message count, peer bearing/distance, and an SOS trigger, consuming only the public BLE device/app API; the integration is published open source in the Connect IQ store; the integration pattern doc enables Wear OS/watchOS ports with no SS-SP cooperation required; SOS trigger requires a guarded confirm interaction (no accidental activation)
- Meta: Shard=— | Type=Feature | Size=M | Prio=P3 | Status=DRAFT | SKU=★ | PRD=F-ECO-01 | Const=C-00

### S-19-026 — GPX/FIT/KML/GeoJSON geodata interchange
As a trip planner I want standard-format import and export of waypoints, tracks, and routes so that anything planned in Garmin/BaseCamp-class tools, Google Earth, or any GIS flows into my mesh devices and back out.
- AC: waypoints, tracks, and routes import and export as GPX and GeoJSON, with KML and FIT accepted on import at minimum; round-trips are verified against at least two third-party tools without data loss of required fields; imported data syncs to the device map and to mesh peers via the existing sharing flows; malformed files are rejected safely with clear errors
- Meta: Shard=— | Type=Feature | Size=S | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-ECO-03 | Const=C-00

### S-19-027 — Video call profile (app ↔ app)
As an app user I want 1:1 video calls with mesh contacts over high-throughput paths so that face-to-face communication works on the same identities and links as messaging, with no external service.
- AC: a 1:1 video call completes app ↔ app over Wi-Fi/Internet RNS paths using open codecs (Opus + AV1 or VP8) per the F-INT-04 RFC profile; bandwidth adaptation steps down to audio-only before dropping the call; the wire profile is published as an open RFC so third-party RNS clients can implement it; device-side participation ships only as a signed plugin per F-INT-03 with zero video code in core firmware
- Meta: Shard=— | Type=Feature | Size=L | Prio=P3 | Status=DRAFT | SKU=★ | PRD=F-INT-04 | Const=C-00, C-02

### S-19-028 — OpenWrt package: router as RNS transport + Home Gateway
As a router owner I want an OpenWrt package that turns my router into an always-on RNS transport node and Home Gateway so that neighbourhood mesh coverage runs on hardware that is already powered 24/7.
- AC: an OpenWrt feed package installs the headless `ss_node_core` daemon with LuCI configuration for interfaces (Wi-Fi/Ethernet/USB-attached LoRa or HaLow), transport mode, and Home Gateway functions; the package builds for at least mipsel and aarch64 router targets in CI and installs on a stock OpenWrt release without custom firmware; router-hosted transport interoperates with device and app nodes in an integration test (one routing domain); the package is published in an open feed with docs so any distro packager can reproduce it
- Meta: Shard=— | Type=Feature | Size=M | Prio=P2 | Status=DRAFT | SKU=★ | PRD=F-NODE-05 | Const=C-00, C-02

### S-19-029 — Multi-device identity: linked devices, message sync, conflict semantics
As a user with a pager and a phone app I want one identity usable across linked devices with defined sync and conflict semantics so that conversations stay consistent no matter which device I pick up.
- AC: a device-linking flow (QR/BLE, mutually authenticated) enrols additional devices under one user identity with per-device sub-keys so any single device can later be unlinked/revoked (composing with S-07-020) without rotating the root identity; message history and read state converge across linked devices via CRDT semantics when they reconnect after partition, with no duplicate or lost messages in a partition/merge test; Double Ratchet session handling with linked devices is specified and RFC'd (per-device sessions or sender-key fan-out) rather than left implementation-defined; behaviour when the same message is answered from two devices simultaneously is defined and tested
- Meta: Shard=— | Type=Feature | Size=L | Prio=P2 | Status=DRAFT | SKU=★ | PRD=F-SEC-04, F-MSG-09 | Const=C-02, C-05

### S-19-030 — Device↔companion pairing & session protocol specification (RFC)
As a protocol engineer I want the pairing and companion-session protocol (BLE GATT + LAN TLS, X25519 KEX, Argon2id confirmation per `03_ARCHITECTURE.md` §9.2/§13.3) frozen as an RFC with a versioned CBOR schema so that firmware (S-03 GATT services, S-10-006 BLE bearer) and all app shells implement one contract instead of four dialects.
- AC: an ACCEPTED RFC specifies the full handshake (frames, nonces, confirmation codes, downgrade protection, error/retry states) and session-channel framing, publishing the schema at `protocol/schemas/pairing.cbor` with a version field governed by S-01-017; conformance vectors cover happy path, wrong-code rejection, replay, and mid-pair link loss, and both the firmware implementation and the shared TS/Dart cores (S-19-001) pass them in CI; MITM resistance rationale is reviewed by wg-security
- Meta: Shard=— | Type=RFC | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-CA-05, F-APP-07 | Const=C-02, C-05
