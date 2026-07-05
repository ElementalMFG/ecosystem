<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# 08 — Universal Connectivity, Multi-Bearer Mesh, and Home Gateway Mode

*Binding founding document. Extends `02_PROTOCOL_STACK.md`. Companion to `01_SS-SP_LITE_HARDWARE_REFERENCE.md`, `04_LICENSING_AND_FORK_STRATEGY.md`, `05_SECURITY_MODEL.md`.*

---

## Table of contents

- [0. TL;DR](#0-tldr)
- [1. Bearer inventory and role](#1-bearer-inventory-and-role)
- [2. The SS-Link abstraction](#2-the-ss-link-abstraction)
- [3. Bearer selection algorithm](#3-bearer-selection-algorithm)
- [4. Reticulum as the universal routing substrate](#4-reticulum-as-the-universal-routing-substrate)
- [5. LXMF as the universal message layer](#5-lxmf-as-the-universal-message-layer)
- [6. Meshtastic wire compatibility](#6-meshtastic-wire-compatibility)
- [7. Home Gateway Mode — the marquee feature](#7-home-gateway-mode--the-marquee-feature)
- [8. Multi-radio simultaneous operation](#8-multi-radio-simultaneous-operation)
- [9. Decentralisation guarantees](#9-decentralisation-guarantees)
- [10. UX rules — how it feels to a user](#10-ux-rules--how-it-feels-to-a-user)
- [11. Testability and telemetry](#11-testability-and-telemetry)
- [12. Failure modes and mitigations](#12-failure-modes-and-mitigations)
- [13. Roadmap alignment](#13-roadmap-alignment)
- [14. What we explicitly do not do](#14-what-we-explicitly-do-not-do)
- [15. Cross-references](#15-cross-references)

## 0. TL;DR

- One unified network layer, **SS-Link**, sits above every radio.
- Bearer selection is automatic: message properties + peer capability + power state pick the bearer(s).
- Reticulum (RNS) provides identity, routing, encryption; LXMF provides messages. Both operate identically across every bearer.
- Meshtastic wire compatibility is a *bearer profile on LoRa only*, not a separate stack.
- "Home Gateway Mode" turns a docked device into a HaLow ↔ LoRa ↔ Wi-Fi ↔ Internet bridge and, on capable boards, a Wi-Fi range extender.
- No central server required for any core feature; internet is used only when available and helpful.

---

## 1. Bearer inventory and role

| Bearer | Chip / Standard | Typical range | Typical bitrate | Primary role | Boards |
|---|---|---|---|---|---|
| Wi-Fi 5 GHz | ESP32-C6 / P4 802.11ax/n | 20–40 m indoor | 100–400+ Mbps | Home/office, backhaul | Alpha, Omega |
| Wi-Fi 2.4 GHz | ESP32 802.11 b/g/n | 30–60 m indoor | 20–100 Mbps | Everywhere, phone AP | Lite, Alpha, Omega |
| Bluetooth LE 5.x | ESP32 BLE | 10–30 m | 1–2 Mbps | Phone pairing, close peer | Lite, Alpha, Omega |
| Wi-Fi HaLow (802.11ah) | Morse Micro MM8108 | 100 m – 1 km+ (16 km ground / 100+ km LoS point-to-point) | 150 kbps – 43 Mbps (MCS0–10, 1/2/4/8 MHz, 256-QAM; region/BW dependent — see `docs/portfolio/08_HALOW_TECHNOLOGY_DOSSIER.md`) | Neighbourhood mesh, home extended range | Alpha, Omega |
| LoRa | Semtech SX1262 | 1–15 km LOS | 0.3–50 kbps | Off-grid, wilderness, emergency | Lite, Alpha, Omega |
| Cellular LTE-M / NB-IoT (opt) | Quectel BG95 / Nordic 9160 | Unlimited | 100–300 kbps | Global fallback, gateway backhaul | Omega only |

Every device becomes a mesh node on every radio it physically has.

---

## 2. The SS-Link abstraction

`ss_link` is the unifying network layer. Every bearer implements the same tiny interface:

```
struct ss_bearer_ops {
    esp_err_t (*send)(ss_frame_t *frame, ss_send_hints_t hints);
    esp_err_t (*receive)(ss_frame_t *frame_out, TickType_t timeout);
    esp_err_t (*peer_scan)(ss_peer_list_t *out);
    esp_err_t (*link_stats)(ss_link_stats_t *out);
    esp_err_t (*set_power_mode)(ss_power_mode_t mode);
    uint32_t   caps;  // SS_BEARER_CAP_* flags (encrypted, mesh-native, high-bandwidth, low-latency, …)
};
```

Bearers register at boot. Higher layers (RNS, LXMF, Meshtastic-compat, WiFi-transport) submit frames via `ss_link_submit()` with routing hints; SS-Link picks one or more bearers.

Frame overhead is normalised: SS-Link inserts a 2-byte "next-header" tag so any payload — an RNS packet, an LXMF blob, a Meshtastic-compatible frame, a Wi-Fi tunnel packet — can flow across any bearer.

---

## 3. Bearer selection algorithm

Every outgoing message has `ss_send_hints_t`:

```
priority     : LOW | NORMAL | HIGH | EMERGENCY
size_hint    : bytes
latency_pref : BEST_EFFORT | INTERACTIVE | REAL_TIME
peer_id      : optional target
allow_relay  : bool
allow_cost   : NONE | LOW_POWER_ONLY | ANY_POWER | ANY_INCLUDING_CELL
```

Selection logic (simplified):

1. **EMERGENCY messages** flood on every available bearer, ignoring cost and duty cycle where safety-legal. SOS is always beaconed on LoRa (long range) even if WiFi succeeded.
2. **Direct peer known and reachable** → cheapest bearer that meets `latency_pref` and `size_hint`.
3. **Peer unknown** → RNS announce path; SS-Link asks RNS for a hint; falls back to broadcast on the smallest-cost bearer that supports it.
4. **Size vs bearer**: >4 KB avoids LoRa; >64 KB avoids HaLow low-rate profile; >1 MB requires WiFi or cellular.
5. **Power state**: when battery <15 %, radios are restricted per `05_SECURITY_MODEL.md` §11 — LoRa low duty cycle, HaLow off, WiFi station-only, no AP.

Selection is table-driven so it can be tuned in RFCs without recompiling.

---

## 4. Reticulum as the universal routing substrate

RNS gives us for free:

- Cryptographic identities (Ed25519 + X25519) generated on-device with no authority.
- Transport encryption (X25519 → AES-128-GCM or ChaCha20-Poly1305, forward-secret with per-link ephemerals).
- Path discovery via *announces* that propagate through the mesh.
- Retransmission and hop-by-hop resilience.

We add three SS-Link–backed **RNS interfaces**:

- `ss_rns_iface_lora` — RNS over our LoRa PHY (LoRa is native RNS territory).
- `ss_rns_iface_halow` — RNS over 802.11ah, framed as UDP over the HaLow IP link.
- `ss_rns_iface_wifi` — RNS over Wi-Fi (LAN broadcast + UDP multicast for local; TCP to a configured public RNS transport node for global).

Reticulum's own logic handles which interface a given packet uses. It already supports multi-interface path selection; we just supply the interfaces.

**Consequence:** a message from Alice to Bob traverses whichever bearers happen to be available. It might go LoRa → HaLow-gateway → Internet → HaLow → LoRa without either endpoint doing anything special.

---

## 5. LXMF as the universal message layer

LXMF sits on top of RNS. It gives us:

- Store-and-forward messaging with delivery receipts.
- Attachments, structured fields, signed messages.
- Group messaging via propagation nodes.

Every SS-SP device is an LXMF endpoint. Every gateway is optionally an LXMF propagation node (opt-in per device, rate-limited, storage-quota'd).

Voice messages, PTT audio blobs, file transfers, presence, and location updates are all LXMF payload types. There is no "second messaging system" for voice — same identity, same routing, same encryption.

---

## 6. Meshtastic wire compatibility

Meshtastic-compat is *a bearer profile of SS-Link on LoRa only*. It is not a parallel network stack.

- Implemented in `firmware/components/ss_meshtastic_compat/`.
- Consumes Meshtastic-format frames from the LoRa radio, translates to SS-Link internal frames, and vice versa.
- Uses the same clean-room implementation as described in `02_PROTOCOL_STACK.md` and legally covered in `04_LICENSING_AND_FORK_STRATEGY.md` §3.
- Enabled by default on the LoRa bearer so users can talk to existing Meshtastic devices immediately.
- Never enabled on non-LoRa bearers (Meshtastic has no defined behaviour there).
- Disabled if the user disables Meshtastic-compat in settings (still allowed to send SS-native LoRa frames).

Meshtastic devices see SS-SP devices as regular Meshtastic peers. SS-SP devices see Meshtastic peers as `caps=meshtastic_only` peers with reduced capabilities (no LXMF, no voice, no encrypted groups).

---

## 7. Home Gateway Mode — the marquee feature

### 7.1 When it activates

Automatically, when **all** of:

- Device is on external power (USB, dock, or wall).
- Wi-Fi station is associated to a configured trusted SSID **or** Ethernet-over-USB is up.
- User has enabled Home Gateway (default: prompt on first dock; opt-in).
- Battery ≥50 % or on charger.

Deactivates when any of the above stops being true.

### 7.2 What it does

**a. Wi-Fi HaLow ↔ Internet bridge.** The device runs its HaLow radio in AP mode, advertising an SS-SP HaLow ESSID. Any HaLow-capable SS-SP within range (≤1 km typical) can associate, and its RNS traffic tunnels through the home Wi-Fi to the global RNS network. Effect: your home extends the mesh across the whole neighbourhood.

**b. LoRa ↔ Internet bridge.** LoRa traffic addressed to nodes on the wider RNS network is forwarded via the home Wi-Fi. Return traffic likewise. Rate-limited and duty-cycle-respecting per regulatory rules (868/915 MHz duty cycle).

**c. Meshtastic-compat gateway.** Legacy Meshtastic nodes in LoRa range are exposed to the SS-SP network for interoperation. Traffic origin remains marked so users can filter.

**d. RNS transport node.** Device announces itself as a transport peer to configured public RNS routers, extending the mesh globally.

**e. Wi-Fi range extender / hotspot (Alpha, Omega only).** Because ESP32-P4 supports simultaneous STA + AP, the device optionally re-broadcasts the home SSID (repeater mode) or a distinct SS-SP hotspot SSID with the same backhaul. Wi-Fi extension is off by default — user opts in per SSID because of the security implications of proxying a home network.

**f. LXMF propagation node.** Optional. Provides store-and-forward for offline peers within its mesh. Uses a small disk quota. User can enable/disable.

**g. Fleet-check-in (paid Fleet Console customers only, opt-in).** Device reports fleet-scoped diagnostics to the Fleet Console over Wi-Fi. Community edition never enables this.

### 7.3 Security posture in Home Gateway Mode

- All bridged traffic remains **end-to-end encrypted at the RNS/LXMF layer.** The gateway never sees plaintext.
- Gateway operator (the home user) cannot read anyone's messages, cannot inject messages under someone else's identity, cannot re-route without cryptographic acceptance by the endpoint.
- Rate limits per-source per-hour prevent abuse and DoS.
- Gateway signs its own presence with its device Ed25519 key so downstream peers know who is relaying.
- User can inspect (in the mobile companion app) how much traffic they have relayed and disable any specific relay function.
- If Wi-Fi extender mode is enabled with the home SSID, the STA-side password is stored only in encrypted flash (see `05_SECURITY_MODEL.md` §12) and never transmitted to the SS-SP cloud.

### 7.4 Regulatory compliance

- Wi-Fi and HaLow AP modes obey local channel regulations. HaLow band is region-configured at first boot (US: 902–928 MHz, EU: 863–868 MHz, etc.); wrong region = HaLow disabled with a clear error.
- LoRa gateway function obeys 868/915 MHz duty-cycle limits. If the duty cycle is exhausted, the gateway will drop LoRa TX and log the reason, not exceed the limit.
- Wi-Fi extender mode advertises the same regulatory country code as the home AP it is bridging.
- All country-code detection is done via Wi-Fi scan + user confirmation, never fixed.

### 7.5 Discovery

An SS-SP that is not currently connected discovers nearby Home Gateways by:

- HaLow beacon scan → SS-SP HaLow ESSID.
- LoRa RNS announce hearing → gateway node ID.
- BLE proximity advertisement if user is nearby with a paired phone.
- On the LAN, mDNS `_ss-sp._udp` if the user is on the same Wi-Fi.

Discovered gateways get a trust score (paired-in-companion-app, seen-many-times, home network match) and the SS-Link selection algorithm biases towards higher-trust gateways.

---

## 8. Multi-radio simultaneous operation

Radio conflicts are managed by the HAL:

- **Lite:** LoRa and microphone share pins via GPIO 45 mux (`ss_hal_muxctl`). At any moment only one is active. Voice PTT capture pauses LoRa RX for the capture duration; SOS overrides mic.
- **Alpha:** LoRa, HaLow, Wi-Fi, BLE run concurrently. All are on separate PHYs and separate chips (SX1262 + MM8108 + ESP32-P4's on-die WiFi/BLE). No mux.
- **Omega:** Alpha bearers + LTE-M module. LTE and Wi-Fi 2.4 GHz coexistence handled by the LTE modem's built-in AGC and off-time.

Concurrency policy is expressed as a small state machine per SKU, not per-radio code. It lives at `firmware/components/ss_link/policy_<board>.c`.

---

## 9. Decentralisation guarantees

- **Zero required central service.** A group of SS-SP devices with LoRa can form a working mesh with no infrastructure whatsoever.
- **No proprietary handshake to reach global RNS.** Public RNS transport nodes exist (community-run). We publish a bootstrap list but the user may replace it with any RNS-speaking node.
- **Identity is user-owned.** Ed25519 keypair is generated on-device from an eFuse-sealed seed. No cloud account is ever required to send a message.
- **Companion app pairing is peer-to-peer** by default (BLE + LAN). Cloud-mediated pairing exists for convenience but is opt-in.
- **No mandatory upstream.** Community OTA channel is one endpoint; users can point at any RSS/JSON manifest URL over TLS with the correct signatures.
- **DHT-optional discovery.** For internet-connected nodes we support a Kademlia-style DHT (Reticulum has built-in mechanisms) but the mesh functions fully without it.

---

## 10. UX rules — how it feels to a user

- **No mode picker.** No "Wi-Fi vs LoRa vs Bluetooth" switch. The device shows one connectivity indicator: dot green/yellow/red, and a small icon strip of active bearers underneath. Long-press the indicator to see details.
- **No routing questions.** Users never see "select gateway" or "select bearer." They send a message; the system decides.
- **Predictable degradation.** If the message cannot be delivered at the requested latency/size, the user is told so *before* they hit send (e.g. "Voice too large for current mesh, will send when Wi-Fi returns; send as text now? [Yes] [Cancel]").
- **Home vs away is inferred.** The device knows when it is docked/plugged/on-trusted-Wi-Fi and behaves accordingly. No manual "Home Mode" toggle for basic behaviour, only for the *advanced* (extender / hotspot) functions.
- **SOS is one button.** Beacons on every radio simultaneously. Stops only on explicit clear.

---

## 11. Testability and telemetry

- **Simulator (`tools/sim/`)** models a mesh of virtual SS-SP nodes with configurable bearer availability, packet loss, and delay per bearer. Regression tests hammer this before every release.
- **Protocol fuzzer (`tools/protocol-fuzzer/`)** targets each bearer's frame parser plus the SS-Link selection algorithm.
- **On-device network log** is opt-in per bearer, capped at N KB, never leaves device without explicit share.
- **Zero identifying telemetry** in community edition. Anonymous connectivity histograms are opt-in and aggregated with k-anonymity ≥100 before upload.

---

## 12. Failure modes and mitigations

| Failure | Mitigation |
|---|---|
| All bearers down | Local UI shows red dot, queues messages in encrypted outbox, retries on wake |
| Only LoRa up, message too big | Auto-fragment if practical; else defer; else send text preview + defer voice |
| Home Gateway loops (gateway A relays to gateway B relays back) | RNS path length limit + gateway ID in envelope + loop-detection cache |
| Meshtastic peer is spammy | Per-peer rate limit; block list; SS-SP flag "Meshtastic ambient noise" |
| Rogue HaLow AP impersonating a Home Gateway | Gateway advertisements are Ed25519-signed; unsigned/wrongly-signed APs ignored |
| Cellular billing runaway (Omega) | Hard monthly cap; user-configurable warn at 50 / 80 / 100 %; auto-disable at 100 % |
| Firmware upgrade breaks a bearer | A/B slots + rollback (see `05_SECURITY_MODEL.md` §7); bearer regression gates in CI |

---

## 13. Roadmap alignment

- **v1.0 (Lite launch):** LoRa (RNS + LXMF + Meshtastic-compat) + Wi-Fi 2.4 + BLE. Home Gateway Mode limited to LoRa↔Wi-Fi.
- **v1.5 (Alpha launch):** Adds HaLow, Wi-Fi 5, full Home Gateway Mode including HaLow↔Internet.
- **v2.0 (Omega launch):** Adds LTE-M / NB-IoT fallback for gateways in areas without home broadband.
- **v2.x:** DHT peer discovery, RNS store-and-forward propagation-node tooling, third-party bearer plugins (LoRa 2.4 GHz Meshtastic mode, DECT NR+, RS-232 over serial for offline hardware).

---

## 14. What we explicitly do not do

- We do not require the user to have an account.
- We do not require any Wi-Fi or cellular connection for core messaging.
- We do not restrict what wire formats the community can add on top; SS-Link is designed to carry anything.
- We do not privilege our cloud in the routing algorithm — a user-run RNS transport node is a first-class citizen.
- We do not deprecate a bearer without a two-release deprecation window announced in `governance/decisions.md`.

---

## 15. Cross-references

- `01_SS-SP_LITE_HARDWARE_REFERENCE.md` — Lite bearer inventory and mic/LoRa mux constraint.
- `02_PROTOCOL_STACK.md` — Wire formats consumed by SS-Link, including Meshtastic clean-room frame layout.
- `04_LICENSING_AND_FORK_STRATEGY.md` §3, §4 — Legal basis for Meshtastic-compat and Reticulum adoption.
- `05_SECURITY_MODEL.md` §5, §8, §11, §12 — Identity, transport keys, network security, storage security.
- `06_GOVERNANCE.md` §4, §5 — RFC process for bearer additions and interface changes.
- `07_BUSINESS_MODEL_AND_OPEN_SOURCE.md` §2 — Community edition guarantees the bearers listed here.

---

*End of 08.*
