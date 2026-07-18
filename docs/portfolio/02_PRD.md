<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# 02 — Product Requirements Document (PRD)

*Comprehensive. Reader: PM, engineering leads, external partners under NDA.*

Prerequisite reading: `01_BRIEF.md`.

---

## 1. Personas

### 1.1 P1 — "Off-Grid Individual" (Ana)
Weekend hiker, part-time overlander, part-time sailor. Owns one SS-SP. Uses it to stay in touch with a partner when out of cell coverage, share position, trigger SOS. Non-technical. Cares about battery life and simple UI.

### 1.2 P2 — "Neighbourhood-Mesh Enthusiast" (Ben)
Home in a rural town. Owns two SS-SPs (Alpha) + one home dock. Runs Home Gateway mode. Extends neighbourhood mesh 1–2 km around his house via HaLow. Runs a public Reticulum transport node. Technical, tinkers with plugins.

### 1.3 P3 — "First Responder" (Chen)
Volunteer search-and-rescue coordinator. Deploys SS-SP kits to team members during incidents. Uses group channels, live location, PTT voice. Needs reliability more than features. Cares about interop with existing agency comms.

### 1.4 P4 — "Enterprise Fleet Manager" (Dana)
Runs 400 devices across a mining operation. Uses Fleet Console for provisioning, compliance auditing, remote firmware updates, per-device policy. Cares about SLAs and audit trails.

### 1.5 P5 — "Sovereignty-Conscious User" (Eli)
Journalist in a country with hostile telecom controls. Uses SS-SP to communicate with sources over LoRa and HaLow, off cellular. Cares about verifiable no-backdoor firmware and duress-wipe.

### 1.6 P6 — "Third-Party Manufacturer" (Foxtrot Co.)
Consumer-electronics firm building a rugged handset for outdoor recreation. Licenses SS-SP-Certified. Cares about certification test suite, brand permissions, security response.

### 1.7 P7 — "Plugin Developer" (Grace)
Independent developer building a marine-navigation plugin. Cares about SDK stability, sandbox capabilities, revenue share, distribution.

---

## 2. Functional requirements

**Hardware rounds & brand abstraction (D-0026, 2026-07-14):** the product ships in two rounds — **round-1** is market-ready NOW on off-the-shelf **Elecrow CrowPanel Advance** boards (3.5″ ESP32-S3 for Lite/Alpha, 5″ ESP32-P4 `elecrow5` for the Omega tier); **round-2** is the in-house, ruggedizable board line (IP-rated; the signed-off Omega v69 + successors), deferred until fabricated and re-brandable (it may adopt a new family name, e.g. "DecoMesh"). Naming is decoupled into four independent axes — ecosystem/protocol (SS-SP, the constant), product brand, SKU tier role (Lite/Alpha/Omega, board-independent), and hardware target (`elecrow5`, `omega-v69`, …); requirements key off tier role + hardware target, never a marketing name. **HaLow is mandatory on every V1 device; LoRa, multi-band Wi-Fi, BLE, GPS, compass, speaker, and camera are additive-optional and runtime-detected** (`ss_hal_has_cap`). See D-0026 + `docs/dev/OMEGA_HW_BASELINE.md`.

### 2.1 Messaging
- **F-MSG-01** — 1:1 text messages between any two SS-SP devices with delivery receipt and read receipt (optional per user).
- **F-MSG-02** — Group text messages (≤128 members) with per-group encryption and membership changes.
- **F-MSG-03** — Voice messages up to 60 s recorded on-device, transmitted as compressed Opus / low-bitrate voice packets.
- **F-MSG-04** — Push-to-talk (PTT) live-voice over Wi-Fi/HaLow bearers; store-and-forward voice over LoRa when live not available.
- **F-MSG-05** — File attachments up to 8 MB (bearer permitting).
- **F-MSG-06** — Presence: online / away / DND / SOS.
- **F-MSG-07** — Location share on-demand and live (opt-in duration).
- **F-MSG-08** — SOS one-button broadcast on every available bearer.
- **F-MSG-09** — Store-and-forward via propagation nodes when recipient is offline.

### 2.2 Bearers
- **F-BR-01** — LoRa (SX1262) — mandatory on all SKUs. *(Board reality, D-0020: fitted via Lite's wireless header only — the signed-off Alpha 1.0 and Omega v1.0 (v69) designs carry no SX1262; requirement deferred to board revisions for those SKUs, and F-BR-07 scope rides with it. See `docs/dev/OMEGA_HW_BASELINE.md`.)*
- **F-BR-02** — Wi-Fi 2.4 GHz — mandatory on all SKUs.
- **F-BR-03** — Bluetooth LE 5.x — mandatory on all SKUs.
- **F-BR-04** — Wi-Fi HaLow (MM8108) — mandatory Alpha, Omega.
- **F-BR-05** — Wi-Fi 5 GHz — mandatory Alpha, Omega.
- **F-BR-06** — Cellular LTE-M / NB-IoT — mandatory Omega. *(Board reality, D-0020/D-0024: no modem on Omega v1.0; cellular is an UNSCHEDULED roadmap option — not in current product intent; bearer abstraction stays cellular-capable. See `docs/dev/OMEGA_HW_BASELINE.md`.)*
- **F-BR-07** — Meshtastic wire compat on LoRa — mandatory all SKUs.
- **F-BR-08** — Bearer channel agility — interference/degradation detection with automatic channel move **within the region plan** on every bearer that supports it; agility events logged. All SKUs (depth varies by radio).
- **F-BR-09** — 802.11s L2 IP-mesh interop bearer (opt-in) — join or serve standard 802.11s meshes (OpenWRT/MANET-class gear) as an additional SS-Link bearer. Alpha, Omega.

### 2.3 Home Gateway Mode
- **F-HGW-01** — Auto-activate on dock + trusted-Wi-Fi + battery ≥ 50 %.
- **F-HGW-02** — HaLow ↔ Internet bridging (Alpha, Omega).
- **F-HGW-03** — LoRa ↔ Internet bridging (all SKUs).
- **F-HGW-04** — Meshtastic-compat gateway on LoRa.
- **F-HGW-05** — Public Reticulum transport node role (opt-in).
- **F-HGW-06** — Wi-Fi range extender / hotspot (Alpha, Omega, opt-in per SSID).
- **F-HGW-07** — LXMF propagation node role (opt-in, disk-quota bounded).

### 2.4 Identity, keys, security
- **F-SEC-01** — Per-device Ed25519 signing identity derived from eFuse seed.
- **F-SEC-02** — Per-device X25519 KEX identity derived from eFuse seed.
- **F-SEC-03** — Hybrid PQ (X25519 + ML-KEM-768) available and selectable by policy.
- **F-SEC-04** — Signal Double Ratchet for 1:1 messaging.
- **F-SEC-05** — MLS-simplified sender-keys for group messaging.
- **F-SEC-06** — Secure Boot v2 with RSA-3072-PSS on ESP32-S3.
- **F-SEC-07** — Flash encryption (AES-256-XTS).
- **F-SEC-08** — Dual-signed OTA (RelKey_A + RelKey_B) with anti-rollback.
- **F-SEC-09** — Escrowed RelKey_C for cold emergency rotation.
- **F-SEC-10** — Duress PIN → silent wipe + compromise beacon.
- **F-SEC-11** — WASM plugin sandbox (Wasm3/WAMR) with capability tokens.

### 2.5 User interface
- **F-UI-01** — Aspect-ratio-agnostic layout engine (`ss_ui`) rendering identical semantics on 3.5" 480×320, 2.4" 320×240, and future round/e-ink displays.
- **F-UI-02** — Screen-reader mode for accessibility (voice UI with TTS).
- **F-UI-03** — Haptic feedback on interactive elements (where hardware present).
- **F-UI-04** — Localisation infrastructure — English (US) mandatory at v1.0; Spanish + one non-Latin script (e.g. Japanese or Arabic) at v1.5.
- **F-UI-05** — High-contrast and daylight-readable palettes.
- **F-UI-06** — One-hand operable — no critical function requires two-thumb use.

### 2.6 Voice
- **F-VOX-01** — On-device STT (Whisper-tiny.en int8) for voice-command entry and voice-to-text of received voice messages.
- **F-VOX-02** — On-device TTS (Piper) for message readout.
- **F-VOX-03** — Voice codec: Opus low-bitrate profile (6–24 kbps) for voice messages and PTT.
- **F-VOX-04** — LoRa voice mode: sub-1-kbps codec for absolute emergencies (Codec2 3200/1600/700C per `02_PROTOCOL_STACK.md` §5.4, module-isolated per licence — see `04_LICENSING_AND_FORK_STRATEGY.md`).
- **F-VOX-05** — Group PTT relay — a designated relay node re-fans group voice as per-member unicast streams for range stability (one weak listener no longer degrades the whole group). Alpha, Omega relay role; all SKUs participate.
- **F-VOX-06** — Real-time voice calls — full-duplex 1:1 calls (Opus) over RNS links on IP-capable paths (HaLow, Wi-Fi, Internet; app-to-app included), with automatic graceful degradation to PTT/voice-message on narrow bearers. Alpha, Omega device-side; all app nodes.

### 2.7 Applications and features
- **F-APP-01** — Chats (1:1 and group).
- **F-APP-02** — Contacts book with verified fingerprints.
- **F-APP-03** — Map (offline tiles, optional online enrichment).
- **F-APP-04** — "Seekie" compass-style peer-pointer showing bearing and distance to a chosen contact.
- **F-APP-05** — Settings, diagnostics, network log.
- **F-APP-06** — SOS beacon UI.
- **F-APP-07** — First-boot provisioning flow (companion-app-guided or standalone).

### 2.8 Companion apps
- **F-CA-01** — iOS app (Swift/SwiftUI).
- **F-CA-02** — Android app (Kotlin/Jetpack Compose).
- **F-CA-03** — Desktop app (Tauri; decided per Q-02, implemented by EPIC-19 S-19-006).
- **F-CA-04** — Web app (progressive, works with BLE Web or LAN).
- **F-CA-05** — All companion apps ship in the same monorepo with a shared TypeScript / Dart core.

### 2.9 Cloud services
- **F-CL-01** — Fleet Console (enterprise multi-device management).
- **F-CL-02** — Provisioning Service (factory ceremony + tenant onboarding).
- **F-CL-03** — Plugin Registry (signed plugin distribution, revocation, review pipeline).
- **F-CL-04** — Relay Federation (commercial RNS transport tooling).
- **F-CL-05** — Community OTA channel (Apache-2.0 endpoint, mirrorable).
- **F-CL-06** — Community pairing bridge (opt-in, encrypted-blob-passing only).
- **F-CL-07** — Billing & entitlements service (subscriptions, marketplace payouts, certification fees; entitlements may only ADD capability, never gate core function per Business Model §4).

### 2.10 SDKs
- **F-SDK-01** — C SDK for embedded interop.
- **F-SDK-02** — Rust SDK for high-assurance clients.
- **F-SDK-03** — Python SDK for scripts and tools.
- **F-SDK-04** — Dart SDK for Flutter apps.
- **F-SDK-05** — TypeScript SDK for web and Node.

### 2.11 Manufacturing and provisioning
- **F-MFG-01** — Factory-line provisioning software with HSM ceremony.
- **F-MFG-02** — Bench-programmer variant for small-batch manufacturers.
- **F-MFG-03** — Field re-provisioning ceremony for RMA workflows.
- **F-MFG-04** — Per-device manifest logged in transparency log.

### 2.12 Certification
- **F-CERT-01** — Public interop test suite for SS-SP-Certified badge.
- **F-CERT-02** — Security floor gate (secure boot, dual-sig OTA, no backdoors).
- **F-CERT-03** — Certification portal.
- **F-CERT-04** — Revocation process.

### 2.13 EUD & network services (the device as an open peripheral for any end-user device)

*Design rule: every service below is a standard protocol on a documented port/characteristic — no proprietary client required; the companion apps and third-party EUDs (phones, laptops, ATAK devices) use the same interfaces.*

- **F-EUD-01** — Network GPS service — device GNSS fix served as standard NMEA-0183 sentences over BLE (all SKUs) and TCP (Alpha, Omega), so any attached EUD can use the device as its position source.
- **F-EUD-02** — IP tether/gateway mode — an attached EUD (USB-ECM/NCM or Wi-Fi client) receives IP connectivity routed across the mesh through the IP-over-RNS tunnel interface (see F-INT roadmap and RFC); Alpha, Omega (HGW dock may add wired Ethernet).
- **F-EUD-03** — Local web admin console — diagnostics, bearer stats, and configuration served on-device over HTTP(S) on the local link, backed by the **same API surface** the companion apps use (agent/API parity; no console-only capability).

### 2.14 Open interop gateways (F-INT)

*Design rule: interop is achieved by translation gateways specified in public RFCs and carried in reserved LXMF field namespaces — never by forking the core protocol. Any LXMF-speaking implementation gains the capability for free.*

- **F-INT-01** — TAK interop — Cursor-on-Target (CoT) PLI and chat bridged bidirectionally between TAK clients (ATAK/WinTAK) and the SS-SP mesh, via an ATAK plugin plus an on-device gateway; wire behaviour specified in an open RFC.
- **F-INT-02** — CoT ↔ LXMF mapping — CoT events carried in a reserved, registered LXMF fields namespace (RFC), so the mapping is implementation-neutral and usable by any LXMF client, not only SS-SP devices.
- **F-INT-03** — Video-over-mesh profile — RTSP/RTP streaming over high-throughput bearers (HaLow wide channels, Wi-Fi), delivered as a signed WASM plugin + SDK example, **not** core firmware; core only guarantees the QoS class and bearer throughput hooks it needs.
- **F-INT-04** — Video call profile — real-time 1:1 video chat using open codecs (Opus + AV1/VP8) over RNS links on high-throughput paths; app-layer first (app ↔ app), device participation only via plugin per F-INT-03; wire profile specified in an open RFC so any RNS client can implement it.
- **F-INT-05** — Federated messaging bridges — opt-in gateway nodes bridging LXMF to the wider open-messaging world (priority order Nostr → XMPP → Matrix), each run **inside the operator's own trust boundary** and clearly labelled *not end-to-end* (the crypto boundary ends at the bridge); the native RNS/LXMF stack + companion app remain the first-class, on-par-or-exceeding experience for the off-grid/sovereignty use case, so bridges are a reach feature, never the foundation. Design substrate: `12_TELEPHONY_AND_UNIVERSAL_INTEROP_DOSSIER.md` §4.1. Stock third-party clients (e.g. Element) are supported via a bridge but the delay-tolerance/addressing/false-E2E-shield limits are documented, not marketed away.

### 2.15 App-layer mesh nodes (F-NODE — any device with the app is a node)

*Design rule: the app is not a remote control — it embeds the same portable node core the firmware uses, so every install can be a full mesh citizen. Wire compatibility with the wider Reticulum ecosystem (Sideband, Nomad Network, Python reference) is mandatory: SS-SP apps and third-party RNS nodes are peers, never silos.*

- **F-NODE-01** — Portable node core — `ss_rns` + `ss_lxmf` + the crypto core built as one portable library (`ss_node_core`) for Android, iOS, Linux, macOS, and Windows; single implementation shared with firmware, zero protocol forks.
- **F-NODE-02** — Standalone node mode — the companion app operates as a full RNS/LXMF node over the host's own connectivity (Wi-Fi, cellular, Internet): messaging, voice notes, and data transfer work app ↔ app and app ↔ device with **no SS-SP hardware present at all**.
- **F-NODE-03** — Gateway mode — an opt-in transport role where the app bridges its host's uplinks (cell/Wi-Fi/Internet) into the mesh for attached or nearby devices — e.g. no HaLow node in range but the phone has Wi-Fi + Internet → mesh traffic routes through the phone. Cellular paths are marked metered under the same bearer-policy vocabulary the firmware uses.
- **F-NODE-04** — Phone ↔ device RNS link — BLE (and USB serial) as first-class RNS interfaces (`ss_rns_iface_ble`), so a paired phone and device form **one routing domain**: either side's bearers (LoRa/HaLow on the device; Wi-Fi/cell on the phone) carry traffic for both, with Reticulum's own multi-interface routing choosing the path.
- **F-NODE-05** — Headless node daemon — a packaged daemon (systemd unit, Docker image; Linux/macOS/Windows) turning PCs, servers, and routers into mesh transport and LXMF propagation nodes.

### 2.16 Ecosystem integrations (F-ECO — meet users on devices they already own)

*Design rule: integrations ride the public app/device API and standard interchange formats — GPX/FIT/KML/GeoJSON, NMEA-0183, CoT — never per-vendor protocol forks; if a vendor programme ends, the open interface it consumed remains for everyone else.*

- **F-ECO-01** — Wearable/handheld companions — Garmin Connect IQ app (watch widget/data field/glance: unread messages, SOS trigger, peer bearing) consuming only the public BLE API; the integration pattern is documented so Wear OS/watchOS/other-vendor ports need no SS-SP cooperation.
- **F-ECO-02** — Vehicle integration — Android Auto and Apple CarPlay projection of the companion app (message readout + voice reply, PTT, SOS shortcut, group-ride map); powersports/marine/overland head units (Polaris Ride Command-class, Garmin/NMEA-2000 ecosystems) are served through the published standard feeds (NMEA position, CoT, GPX routes) rather than bespoke per-vendor firmware.
- **F-ECO-03** — Standard geodata interchange — GPX/FIT/KML/GeoJSON import and export for waypoints, tracks, and routes across the device map and all apps, so trips planned in any third-party tool flow in, and recorded data flows back out.

### 2.17 Real-world telephony interconnect (F-TEL — reach the existing phone network, optionally and sovereignly)

*Design rule: telephony is pure ADD-capability — the mesh is fully useful with zero phone number (F-CL-07 never-degrade). All of it resolves to one carrier-neutral abstraction with pluggable backends and escalating self-sovereignty (the "Sovereignty Ladder"); we interop where standards are open, partner where regulation forces it, and build native where incumbents keep it closed. Ratified by decision **D-0025**; full design substrate in `12_TELEPHONY_AND_UNIVERSAL_INTEROP_DOSSIER.md`.*

- **F-TEL-01** — Telephony Provider Interface (TPI) — a carrier-neutral contract (`send/receive` SMS·MMS, `originate/answer/bridge` call, `provision/port/release` number, DTMF, delivery receipts) with pluggable adapters (CPaaS REST, SIP trunk via embedded baresip/PJSIP, XMPP↔PSTN, WebRTC/WHIP, eSIM); adapters are built and CI-maintained regardless of which tier ships (capability-ready per D-0023/D-0024).
- **F-TEL-02** — SMS/MMS ↔ LXMF text bridge — bidirectional text between the phone network and an LXMF identity; async store-and-forward so it works over **every** bearer including LoRa; the universal, priority capability and the default hosted offering.
- **F-TEL-03** — Number identity & addressing — a dedicated E.164 number mapped to an LXMF identity; mesh users reach the phone network via a `tel:` destination; the cryptographic RNS/LXMF identity is the owned root, the phone number a leased, portable alias.
- **F-TEL-04** — Sovereignty Ladder — one code path, escalating control: Rung 0 hosted turnkey "SS-SP Number"; Rung 1 bring-your-own-number against our bridge; Rung 2 self-hosted bridge on the Home Gateway / WASM plugin; Rung 3 self-hosted full stack (PBX/SIP or Soprani.ca/JMP); Rung 4 own ITSP/MVNE. Hosted-default; every rung above 0 is the open Apache-2.0 bridge pointed at the operator's own backend.
- **F-TEL-05** — Inbound voicemail-to-text — a call to the number is answered by the gateway, transcribed/recorded on-device/edge, and delivered as an LXMF message; works for LoRa-only users and stays off the interconnected-VoIP voice line.
- **F-TEL-06** — Live voice ↔ PSTN — full-duplex calls between an SS-SP identity and a phone number via a SIP↔LXST bridge with Opus↔G.711 transcode at the gateway; **IP-bearers only** (HaLow-gatewayed / Wi-Fi / future cellular), and **only on Rungs 1–4** (operator's own carrier account) for regulatory reasons (F-TEL-08).
- **F-TEL-07** — Number portability — clean port-in (bring your own number) and no-fee port-out (the number is never hostage), honouring "the device outlives the company."
- **F-TEL-08** — Regulatory posture — the hosted offering is **text-first, permanently** (SS-SP carries A2P 10DLC; no hosted user-originated PSTN voice, which would trigger interconnected-VoIP E911/CALEA/USF); live voice rides the operator's own carrier account so those duties attach to the CPaaS/operator, not the project; per-country availability gated by `wg-legal`; a prominent **"not for emergency calling"** notice on all telephony features (SOS remains bearer-flood per C-05); the documented plaintext boundary at the PSTN edge and retention policy pass `wg-security` review.

---

## 3. Non-functional requirements

### 3.1 Performance
- **NF-PERF-01** — UI touch-to-first-frame < 100 ms on Lite.
- **NF-PERF-02** — Bearer switch (e.g. Wi-Fi lost → HaLow selected) < 3 s.
- **NF-PERF-03** — LoRa TX latency (queue → on-air) < 250 ms outside SOS.
- **NF-PERF-04** — SOS beacon on-air within 2 s of button press.
- **NF-PERF-05** — Boot-to-usable-UI < 5 s on Lite, < 3 s on Alpha.
- **NF-PERF-06** — Fleet Console page-load p95 < 1.5 s over cable broadband.

### 3.2 Reliability and availability
- **NF-REL-01** — Field devices operate indefinitely without cloud contact.
- **NF-REL-02** — OTA update failure rate < 0.1 % without rollback trigger.
- **NF-REL-03** — Rollback success rate 100 % on signed rollback attempt.
- **NF-REL-04** — Fleet Console SLA 99.9 % monthly for enterprise tier.
- **NF-REL-05** — Community OTA channel target 99.5 % availability, mirrorable.

### 3.3 Battery / power
- **NF-PWR-01** — Lite standby ≥ 24 h with LoRa periodic.
- **NF-PWR-02** — Alpha standby ≥ 72 h with LoRa periodic + HaLow doze.
- **NF-PWR-03** — Omega standby ≥ 48 h with LTE PSM enabled.

### 3.4 Security
- **NF-SEC-01** — All keys derived at first boot from HW-RNG or per-device eFuse seed.
- **NF-SEC-02** — No plaintext user data in flash at rest.
- **NF-SEC-03** — No log of message contents at rest by default.
- **NF-SEC-04** — Rollback attacks impossible for released firmware ≤ 24 h old.
- **NF-SEC-05** — Full SBOM published per release.
- **NF-SEC-06** — Third-party pentest annually from year 2.

### 3.5 Privacy
- **NF-PRIV-01** — Community edition ships with all telemetry disabled by default.
- **NF-PRIV-02** — No message content ever transmitted off-device without explicit share.
- **NF-PRIV-03** — Location data lifetime on cloud ≤ 30 days unless user extends.
- **NF-PRIV-04** — GDPR data-subject requests fulfilled in ≤ 30 days.

### 3.6 Regulatory
- **NF-REG-01** — FCC Part 15 (US) certification for every SKU sold in the US.
- **NF-REG-02** — CE RED (EU) certification for every SKU sold in the EU.
- **NF-REG-03** — CRA-compliant SBOM and vulnerability disclosure process.
- **NF-REG-04** — HaLow region-code enforcement at first boot; wrong region disables HaLow.
- **NF-REG-05** — LoRa duty-cycle limits observed in EU 868 MHz band.

### 3.7 Localisation and accessibility
- **NF-L10N-01** — All user-facing strings extracted via `ss_i18n`.
- **NF-L10N-02** — Right-to-left script support at v1.5.
- **NF-A11Y-01** — WCAG 2.2 AA for companion apps.
- **NF-A11Y-02** — Screen-reader parity for device UI.

### 3.8 Cost
- **NF-COST-01** — Lite BOM ≤ USD 60 at 10 k volume.
- **NF-COST-02** — Alpha BOM ≤ USD 120 at 5 k volume.
- **NF-COST-03** — Cloud services gross margin ≥ 40 % at 10 k enrolled devices.

### 3.9 Sustainability / longevity
- **NF-SUS-01** — Security fixes for 5 years post last shipment (per Open Assurance).
- **NF-SUS-02** — Repairability guide, spare-parts availability commitment 3 years.
- **NF-SUS-03** — Reference firmware buildable from source using open toolchains only.

### 3.10 Scale
- **NF-SCALE-01** — Mesh + cloud architecture validated by simulation at ≥ 1 M concurrent nodes (mixed bearer profiles) before any marketing claim of "millions"; no protocol-level collapse (routing convergence, announce load, delivery rate ≥ 99 % for in-coverage peers).
- **NF-SCALE-02** — Announce/path overhead on constrained bearers bounded: LoRa announce ingress+egress ≤ 5 % of regional duty-cycle airtime budget in a 1 000-node region (transport-tier profile + announce budgets).
- **NF-SCALE-03** — A single relay node sustains ≥ 10 k concurrent RNS links on commodity hardware; relay fleet scales horizontally with no shared-state bottleneck (add-node = add-capacity, verified by load test).
- **NF-SCALE-04** — Fleet Console supports ≥ 100 k devices per tenant while holding NF-PERF-06 dashboard latency; control plane degrades gracefully (queue, don't drop) at 10× burst.
- **NF-SCALE-05** — Release-day OTA reaches 1 M devices ≤ 72 h via CDN + community mirrors; artifact distribution is fully mirrorable (no single distribution point) per Open Assurance.
- **NF-SCALE-06** — LXMF propagation-node peering is quota-bounded (per-peer storage/bandwidth budgets + stamp-based spam economics) so no participant can exhaust the propagation network.

---

## 4. Key user journeys

### 4.1 J1 — Ana in the mountains
Ana takes her Lite hiking. Cell dies at trailhead. She sends "reached the ridge" to her partner. Message queues, LoRa TX, hops via a nearby camper's device, reaches partner (who has a Home Gateway at home). Reply arrives in ~5 s. Later, Ana falls; she presses SOS; every bearer beacons; her partner and the local SAR node (Home Gateway) both alert. GPS coords broadcast.

### 4.2 J2 — Ben extends the neighbourhood
Ben mounts an Alpha in his window with a HaLow antenna. Home Gateway auto-activates. Ana's Lite comes into HaLow range 800 m away; her packets tunnel via Ben's home Wi-Fi to global RNS. Ben's Fleet Console (personal tier, free) shows relay traffic he chose to accept.

### 4.3 J3 — Chen coordinates a SAR mission
Chen distributes 8 Alpha units. Each pre-configured with the mission group. During the search, live PTT over HaLow within team, LoRa fallback when spread out, SOS override always available. Post-mission, the log is archived on his Fleet Console instance for after-action review.

### 4.4 J4 — Dana runs 400 devices in a mine
Dana provisions 400 Alphas via the Provisioning Service in a factory-line ceremony. Devices enrol into her Fleet Console tenant. Fleet Console pushes a policy: SOS beacon on shift-hour bounds, no cloud telemetry, encrypted-at-rest logs. OTA update pushed staged 5 % / 25 % / 100 % with automatic rollback on regression.

### 4.5 J5 — Eli under hostile telecom
Eli buys a Lite from a mail-order retailer using cash-equivalent. First-boot ceremony generates keys with no cloud contact. Eli communicates with sources over LoRa. If seized, duress PIN silently wipes and emits a compromise beacon that alerts the source network. Verifiable-build attestation lets Eli confirm the firmware matches published source.

---

## 5. MVP scope (v1.0 = Lite launch)

### In scope
- Lite hardware SKU.
- Firmware for Lite: bootloader, HAL, `ss_link` (HaLow/LoRa + Wi-Fi + BLE — the D-0013 dev fleet is HaLow-fitted), Reticulum, LXMF, 1:1 + group messaging, voice messages (Opus), SOS, presence, location, Meshtastic-compat on LoRa, `ss_ui` on the 480×320 display, duress PIN, dual-sig OTA, factory provisioning.
- Companion apps: iOS + Android (beta), Web (basic).
- Community OTA channel.
- SDKs: C + Rust + Python + Dart (TypeScript at v1.1).
- Home Gateway Mode limited to LoRa ↔ Wi-Fi and Meshtastic-compat gateway.
- Docs, tutorials, quick-starts.
- Portfolio and constitution (already delivered).

### Out of scope for v1.0
- Native (soldered-down) HaLow radio — Alpha onward; Lite v1 carries HaLow via the wireless-header module (D-0013), which **is** in scope above.
- Cellular (Omega only).
- Fleet Console (comes v1.1).
- LEO satellite (v3.x).
- On-device LLM assist (Omega only, v2.x).
- LTE-M billing management (Omega only).

---

## 6. Milestones (order, not dates)

- **M0 — Foundation:** constitution + portfolio delivered. Monorepo skeleton, HAL contracts, Lite `board_config.h`, repo meta files. [current state]
- **M1 — Lite bring-up:** LoRa Reticulum mesh operational. Two Lite units exchange messages.
- **M2 — Lite v1.0 ship:** all F-MSG-01..F-MSG-09 functional. Companion apps beta. OTA works. Home Gateway (LoRa↔Wi-Fi) works. Reproducible builds attested.
- **M3 — Alpha bring-up:** HaLow + Wi-Fi 5 + GNSS + IMU operational.
- **M4 — Alpha v1.5 ship:** all F-BR-* functional (except cellular). Fleet Console in early access.
- **M5 — Cloud v2:** Fleet Console GA. Provisioning Service GA. Plugin Registry GA.
- **M6 — Omega bring-up:** cellular functional.
- **M7 — Omega v2.0 ship:** all F-BR-* functional. Certification program open.
- **M8 — First third-party Certified device:** external manufacturer ships.
- **M9 — Foundation transition:** trademarks and infrastructure move to neutral foundation.

---

## 7. Key metrics (KPIs)

- **Devices in field.** Target trajectory: 5k (M2) → 25k (M4) → 100k (M7).
- **Messages per active device per day.** Target ≥ 10.
- **SOS activations honoured.** Target ≥ 99.9 %.
- **OTA update success.** Target ≥ 99.9 % across community and enterprise channels.
- **Security disclosures response time.** Target 24 h ack, 7 d triage, 30 d fix or mitigation.
- **Fleet Console monthly active tenants.** Target 5 (M4) → 50 (M7) → 200 (M9).
- **Third-party plugin installs.** Target ≥ 50 unique plugins by M7.
- **Certified device manufacturers.** Target 3 by M9.
- **User trust survey.** Target NPS ≥ 60.

---

## 8. Explicitly out-of-scope

- Consumer smartphone replacement.
- Video conferencing.
- Cryptocurrency wallet.
- Payment terminal.
- Video game console.
- Full anonymity against nation-state APT.

Out of scope may become in-scope only via a documented PRD amendment RFC.

---

## 9. Open questions (must resolve before M2)

- **Q-01** — **RESOLVED (by design).** Both: Opus primary; Codec2 as a licence-isolated loadable module for ultra-low-bitrate voice. Implemented by EPIC-14 S-14-003/S-14-004 (isolation per C-04).
- **Q-02** — **RESOLVED.** Tauri (smaller binary, Rust-native crypto). Committed in EPIC-19 (S-19-006, S-19.D); F-CA-03 updated. ADR to be logged at EPIC-19 kickoff.
- **Q-03** PQ hybrid default-on vs opt-in for v1.0? Preference: opt-in for v1.0, default-on for v2.0. (Policy flag wired in S-06-014; final posture decided before M2.)
- **Q-04** Cellular modem vendor for Omega: Quectel BG95 vs Nordic 9160 vs u-blox SARA. Scoping spike tracked as S-05-019.
- **Q-05** — **RESOLVED.** Signed, mirrorable bootstrap directory listing community and commercial endpoints; hint-only, fully overridable; third-party mirrors supported. Decided in `docs/portfolio/05_INFRASTRUCTURE_AND_SCALE.md` and tracked as S-21-025.

---

## 10. Risks (top 10)

1. **HaLow ecosystem doesn't mature** → mitigation: LoRa remains fully capable for core UX; HaLow is upside.
2. **FCC/CE certification delays** → mitigation: parallel EMC pre-testing at ID-freeze; buffer in schedule.
3. **Reticulum backwards-incompat change** → mitigation: pin version; contribute upstream; hold LTS fork if needed.
4. **Meshtastic community perception risk** → mitigation: neutral marketing, contribute test vectors, no denigration.
5. **Cellular billing runaway on Omega** → mitigation: hard user-visible caps; Fleet policy overrides.
6. **Duress-PIN accidental trigger** → mitigation: confirmation dialog off, but wipe protected by a 500 ms hold and pattern; false-positive telemetry (opt-in).
7. **Plugin marketplace attack surface** → mitigation: sandbox, signed plugins only, revocation, review pipeline.
8. **BSL → Apache-2.0 clock resets fear** → mitigation: pre-committed dates in every file; audit annually.
9. **Vendor dependence on single silicon (ESP32 line)** → mitigation: HAL abstracts; second-source SoC evaluated at M4.
10. **Founder-vendor bankruptcy** → mitigation: full escrow instrument in `governance/OPEN_ASSURANCE.md`; devices in field self-sufficient.

---

## 11. Compliance surface

Enumerated with target certification per SKU in `04_BLUEPRINT.md`. Regimes touched:

- FCC Part 15 (US), ISED (Canada), RED (EU), MIC (Japan), KC (Korea), ACMA (Australia), Anatel (Brazil).
- GDPR (EU), CCPA/CPRA (California), LGPD (Brazil), PDPA (Singapore).
- EU Cyber Resilience Act (CRA).
- EU AI Act (for on-device models).
- Colorado AI Act (2026 enforceable).
- Common Criteria EAL-2 (Lite / Alpha), EAL-4 target (Omega).

---

*See `03_ARCHITECTURE.md` for how these requirements are realised.*

*End of 02 — PRD.*
