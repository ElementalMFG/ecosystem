<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# SS‑SP Protocol Stack — Wire Format, Bridging, Transports
**Status:** Draft‑1 (normative for reference implementation)
**Owner:** SS‑SP TSC
**Companion docs:** `00_MASTER_SOFTWARE_PLAN.md`, `01_SS-SP_LITE_HARDWARE_REFERENCE.md`

---

## Table of contents

- [0. Design goals](#0-design-goals)
- [1. Layered view (recap)](#1-layered-view-recap)
- [2. Identity](#2-identity)
- [3. Reticulum as the routing spine (L4)](#3-reticulum-as-the-routing-spine-l4)
- [4. Application protocol layer (L5)](#4-application-protocol-layer-l5)
- [5. Data plane (L3)](#5-data-plane-l3)
- [6. Transport / bearer specifics (L2)](#6-transport--bearer-specifics-l2)
- [7. Cryptography](#7-cryptography)
- [8. Time](#8-time)
- [9. Quality of Service (QoS)](#9-quality-of-service-qos)
- [10. Fuzz + conformance](#10-fuzz--conformance)
- [11. Extending the protocol — RFC process](#11-extending-the-protocol--rfc-process)
- [12. Sequence examples](#12-sequence-examples)
- [13. Open items / to be RFC'd](#13-open-items--to-be-rfcd)
- [14. Summary](#14-summary)

## 0. Design goals

1. **Transport‑agnostic** — every message flows over any bearer (HaLow, LoRa, BLE, Wi‑Fi, ESP‑NOW, USB‑CDC, Ethernet, Serial, TCP/UDP over Internet, Iridium/Starlink IP).
2. **Sovereign identity by default** — every node has a self‑generated Ed25519/X25519 identity; no CA required.
3. **End‑to‑end confidential + authenticated** — even the reference implementation refuses to transmit plaintext outside "public safety broadcast" mode.
4. **Zero configuration** — two SS‑SP devices power on, discover each other, exchange identities, and are talking within seconds. No accounts, no servers.
5. **Interop with the world** — a Meshtastic user with an unmodified Meshtastic phone app can pair to an SS‑SP and message the mesh unchanged.
6. **Store‑and‑forward + multi‑hop** — mesh survives sparse connectivity, node churn, and long silences.
7. **QoS aware** — voice, alerts, telemetry, and bulk transfer share the medium sanely.
8. **Extensible** — new payload types added via CBOR‑tagged records without breaking older nodes.
9. **Auditable + testable** — every framer/parser has a golden‑vector fuzz corpus.

---

## 1. Layered view (recap)

```
L7  Applications: Chat, Voice PTT, Seekie, Map, SOS, Roster, Plugins
L6  UI framework: ss_ui on LVGL 9 (renders L7 state)
L5  Application protocol: LXMF + SS‑Ext (CBOR) + Meshtastic‑compat proto
L4  Mesh identity + routing: Reticulum (RNS) — Ed25519 identity, path discovery
L3  Data plane: fragmentation, store‑and‑forward, CRDT KV, voice framer
L2  Transport interfaces (bearers): HaLow, Wi‑Fi, BLE, LoRa, ESP‑NOW, USB‑CDC,
    UART, TCP, UDP, IPv6 (Sat/Cell)
L1  HAL (hardware abstraction)
L0  Base OS (ESP‑IDF/FreeRTOS on MCU targets; Linux on gateway targets)
```

The **innovation** is at L4 (Reticulum as the routing spine) and L5 (a small set of well‑specified CBOR record types + a Meshtastic compatibility bridge).

---

## 2. Identity

### 2.1 Node identity
- Ed25519 signing keypair + X25519 exchange keypair, generated on first boot inside `hal_secure`.
- Stored in encrypted namespace on flash; optional migration to external ATECC608 secure element when present.
- Identity hash (`node_id`): first 16 bytes of `SHA‑256(pub_sign || pub_ex)` — used as the RNS destination hash.
- Human‑readable **callsign** is a *label*, not an identifier. It can collide; identity hash cannot.

### 2.2 Group / channel identity
- A "channel" is a symmetric AEAD context (XChaCha20‑Poly1305, 256‑bit key, 24‑byte nonce) plus a group name and metadata blob.
- Channels are shared by QR code / NFC handoff / RNS out‑of‑band invite.
- Rotating group keys ("epoch") supported: each channel carries a monotonically increasing epoch counter; new epoch keys are ratcheted from `HKDF(prev_key, "ss-ep\|epoch")`.

### 2.3 Contact (peer‑to‑peer) identity
- Each user maintains a **roster** of `(node_id, callsign, public_keys, last_seen, rssi_stats, notes)`.
- Peer‑to‑peer messaging uses X25519 ECDH → HKDF → XChaCha20‑Poly1305 with a Signal‑style Double Ratchet layered on for forward secrecy (see §7).

---

## 3. Reticulum as the routing spine (L4)

We use Reticulum unmodified where possible and extend it with **two new interface types**:

### 3.1 `HaLowInterface` (Alpha)
- Wraps the Morse Micro MM8108 driver.
- Two modes: **802.11s mesh** (peer‑to‑peer, preferred) and **infrastructure STA/AP** (for interop with commercial HaLow AP deployments).
- MTU: 500 bytes at RNS layer (fragmentation above); underlying HaLow frame carries multiple RNS packets when possible.
- QoS: maps RNS packet priorities → 802.11e WMM Access Categories:
  - Voice → `AC_VO`
  - Alert (SOS, presence) → `AC_VI`
  - Interactive text / small telemetry → `AC_BE`
  - Bulk (files, maps, OTA) → `AC_BK`

### 3.2 `LoRaInterface` (Lite + Alpha add‑on)
- SX1262 via RadioLib (Arduino) or native LoRaMac‑node (IDF).
- Regional profiles: US902‑928, EU863‑870, AU915, JP920‑928, KR920, IN865, RU864, CN470.
- Duty‑cycle enforced in firmware per region.
- Spreading factor + BW selectable per channel: default SF9/BW125 for chat, SF7/BW250 for voice, SF12/BW125 for extreme range.
- Reticulum runs unmodified over LoRa — mature, well‑tested (this is what RNode already does).

### 3.3 `BLEInterface`
- NimBLE server + client. Custom GATT service `0x5353` with characteristics:
  - `0xTX` (write no response) — outbound bytes to bearer
  - `0xRX` (notify) — inbound bytes from bearer
  - `0xST` (read/notify) — bearer status
- Used for phone tether and pendant.
- Also carries **BLE HID** input as a separate service (for pendant/button accessories).

### 3.4 `WiFiInterface`
- STA + softAP + **ESP‑NOW** each usable as bearers.
- STA: RNS TCPInterface over Internet or LAN.
- SoftAP: acts as a captive portal + local RNS TCP endpoint for phone/laptop.
- ESP‑NOW: connectionless 2.4 GHz mesh, ~200 m LOS, no AP needed — good short‑range fallback.

### 3.5 `USBCDCInterface`
- USB‑Serial console doubles as an RNS `TCPClientInterface` over Serial (SLIP‑framed).
- Compatible with Reticulum's `TCPInterface` and RNode `RNodeInterface`.

### 3.6 Cross‑bearer routing
- RNS handles bearer selection, path discovery, and next‑hop retransmission automatically.
- Every SS‑SP is inherently a **multi‑bearer router**: e.g., receive a chat message over LoRa → forward over BLE to the paired phone → phone relays to Internet via RNS TCP.

### 3.7 Announces
- Nodes broadcast RNS `Announce` packets on their identity destinations at exponentially backed‑off intervals: 5 s, 15 s, 60 s, 5 min, 30 min, 6 h, then every 24 h until state changes.
- Announces include:
  - Node identity + signature
  - Optional callsign + emoji flag
  - Optional coarse position (S2 cell at level 8 — ~1 km²) when in **discoverable** mode
  - Bearer hints (which interfaces this node accepts)
  - Software version (for compat gating)

---

## 4. Application protocol layer (L5)

### 4.1 LXMF (Lightweight eXtensible Message Format) — the base carrier
- Native Reticulum message format.
- Fields: `destination`, `source`, `signature`, `payload`, `timestamp`, `title` (optional), `content`, `fields` (optional map).
- We use LXMF as the **envelope** for all SS‑SP payloads.

### 4.2 SS‑Ext records — CBOR, tag‑extensible
SS‑SP‑specific payload is CBOR inside `LXMF.content`. Top‑level CBOR structure:

```
[
  0xSSSP,                    // magic
  version (uint),            // spec version, currently 1
  record_type (uint),        // see registry below
  record_body (any CBOR)
]
```

**Record type registry** (Draft‑1):

| Type | Name | Body sketch |
|---|---|---|
| 1  | text.message           | `{text, mentions?, reply_to?, edit_of?, expire_at?}` |
| 2  | voice.frame            | `{codec:"opus"|"codec2", seq, ts_us, data}` (many per PTT session) |
| 3  | voice.session          | `{session_id, start|end, sample_rate, codec, ptt}` |
| 4  | presence.position      | `{lat_e7, lon_e7, alt_dm, accuracy_dm, heading_cd?, speed_cmps?}` |
| 5  | presence.status        | `{state:"ok"|"help"|"sos"|"off", battery_pct?, note?}` |
| 6  | seekie.target          | `{node_id, bearing_cd, distance_dm, until_ts}` |
| 7  | telemetry              | `{sensors:{name:value}, ts}` |
| 8  | file.chunk             | `{file_id, seq, total, data, sha256_final?}` |
| 9  | crdt.op                | `{doc_id, op, vector_clock}` |
| 10 | ota.manifest           | `{version, size, sha256, sig, min_prev_version}` |
| 11 | ota.chunk              | `{version, seq, total, data}` |
| 12 | admin.command          | `{cmd, args, nonce, sig}` (signed by operator key) |
| 13 | admin.telemetry_report | `{node_id, uptime, mesh_peers, rssi_stats, ...}` |
| 14 | sos.beacon             | `{severity, category, lat, lon, note, at_ts}` |
| 15 | waypoint               | `{id, name, lat, lon, alt, icon, ttl, author}` |
| 16 | roster.introduction    | `{callsign, avatar_hash?, capabilities:[SS_CAP_*]}` |
| 17 | plugin.envelope        | `{plugin_id, opaque_bytes}` |
| 18 | ack                    | `{ref_id, code, note?}` |
| 19 | bridge.foreign         | `{foreign_proto, foreign_payload}` (e.g., Matrix/Nostr/MQTT tunneled) |
| 20 | key.rotate             | `{channel_id, new_epoch, wrapped_key_for_each_member}` |
| 21 | route.hint             | `{dst_hash, path:[node_id...], cost}` (optional path advertising) |
| 22 | time.sync              | `{utc_us, source:"gnss"|"ntp"|"mesh", accuracy_us}` |
| 23 | mesh.graph             | `{edges:[{a,b,rssi,seen_ts}], gen_id}` (visualizer) |
| 24 | video.mframe           | `{stream_id, seq, ts_us, codec:"h264", data}` (Alpha only) |
| 25 | map.tile               | `{z, x, y, format:"pmtiles"|"mbtiles", data}` |

**Reserved:** 100–199 for future core; 200–999 for vendor extensions (must be registered).

### 4.3 Meshtastic compatibility mode
- SS‑SP exposes a **Meshtastic‑compatible transport** on:
  - BLE GATT service matching Meshtastic's UUIDs
  - USB‑CDC using Meshtastic's toRadio/fromRadio protobuf framing
- Incoming Meshtastic protobufs are translated:
  - `TextMessage`   ↔ SS‑Ext `text.message` (record 1)
  - `Position`      ↔ `presence.position` (record 4)
  - `NodeInfo`      ↔ `roster.introduction` (16) + RNS `Announce`
  - `Waypoint`      ↔ `waypoint` (15)
  - `Telemetry`     ↔ `telemetry` (7)
  - `RouteDiscovery` ↔ RNS path request/reply
- The translation is **bidirectional and lossless within the intersection of feature sets**; SS‑Ext fields with no Meshtastic equivalent are dropped when going outbound and reappear only when the peer is SS‑SP‑aware.
- Result: an unmodified Meshtastic Android/iOS app pairs to an SS‑SP and behaves like a Meshtastic node. Users on the other side of the mesh get full SS‑Ext.

### 4.4 Foreign‑protocol bridges (opt‑in, gateway role)
Every SS‑SP can be configured as a bridge to one or more foreign networks:
- **MQTT** (broker on operator server) — bidirectional text + telemetry.
- **Matrix** (via a self‑hosted bridge) — chat rooms mapped to channels.
- **Nostr** — public relay, chat + presence.
- **APRS‑IS** — position + short text.
- **TAK / ATAK** via CoT (Cursor on Target) — for tactical map overlays.
- **Iridium SBD / Starlink IP** — for backhaul when out of everything.

Bridges are strict:
- Foreign traffic entering the mesh is wrapped in `bridge.foreign` (record 19) with a signed origin note.
- The bridge annotates its own node_id as the origin; end users can filter bridged content.
- No foreign protocol implicitly downgrades mesh‑local encryption.

---

## 5. Data plane (L3)

### 5.1 Fragmentation
- MTU inheritance: RNS packet size (typically 500 B) → SS‑Ext records ≤ 480 B; larger records split into `file.chunk` streams.
- Chunks carry `file_id + seq + total + sha256_final`. Missing chunks are refetched via `ack` records with negative fields.

### 5.2 Store‑and‑forward
- Every node keeps a **store** of recently seen records in `/state/store/` (bounded, LRU + priority).
- On new peer discovery, exchange **Bloom filters** of stored record IDs to sync missing content efficiently.
- Records carry `ttl` (hop count) and `expire_at` (wall clock) — nodes must honor both.

### 5.3 CRDT KV
- For shared state (roster metadata, waypoints, group membership): a delta‑CRDT map.
- Encoded as `crdt.op` records (type 9).
- Well‑suited to lossy, partitioned meshes; converges without central coordination.

### 5.4 Voice framer
- Codec2 700C / 1600 / 3200 bps for LoRa PTT; Opus 8/16/24 kbps for HaLow full‑duplex.
- 20 ms frames. Jitter buffer 60–180 ms, adaptive.
- Voice sessions have their own signal state machine: `INVITE → RINGING → ACTIVE → HANGUP` sent as `voice.session` records; frames are `voice.frame`.

### 5.5 Video (Alpha only)
- H.264 hardware encoder on ESP32‑P4.
- `video.mframe` records (type 24) at 5–15 fps, 320×240 or smaller.
- Not for cinematic use — for situational awareness (still cam, brief look).

---

## 6. Transport / bearer specifics (L2)

### 6.1 Bearer capabilities table
| Bearer | Bitrate | MTU | Latency | Range | Duplex | Cost | Use |
|---|---|---|---|---|---|---|---|
| HaLow 802.11s (Alpha) | up to 43 Mbps | 1500 | 5–50 ms | 1–16 km / 100 km LoS | full | high TX | primary mesh (Alpha) |
| LoRa SF7/BW250 | ~11 kbps | 250 | 100 ms | 1–5 km | half | low | voice PTT / short chat |
| LoRa SF9/BW125 | ~1.7 kbps | 250 | 400 ms | 5–15 km | half | low | chat, telemetry |
| LoRa SF12/BW125 | ~250 bps | 250 | 1–2 s | up to 40 km | half | low | beacon / SOS |
| ESP‑NOW (2.4 GHz) | ~1 Mbps | 250 | <10 ms | ~200 m | full | mid | short‑range mesh |
| BLE 5.x | 100 kbps ea | 244 | 30 ms | ~30 m | full | low | phone tether, pendant |
| Wi‑Fi 4 STA/AP | 20+ Mbps | 1500 | <10 ms | ~50 m | full | mid | internet backhaul |
| USB‑CDC | 1 Mbps+ | 4096 | <1 ms | wired | full | none | serial / RNode compat |
| TCP/UDP (Sat/Cell) | varies | 1500 | 100–1500 ms | global | full | $$$ | backhaul of last resort |

### 6.2 Bearer selection policy (Reticulum + our extension)
- RNS handles path discovery; we add a **cost function** per bearer:
  - `cost = base_cost(bearer) + hop_count + rssi_penalty + duty_cycle_penalty`
- The stack picks the lowest‑cost viable path per packet class (voice preempts bulk).
- Fallback rules:
  1. Try direct HaLow / LoRa mesh.
  2. Fall back to ESP‑NOW if in range.
  3. Fall back to phone‑tether over BLE → phone's Internet → RNS TCP.
  4. Fall back to Sat modem if operator has configured one.
- Every bearer switch is logged; nothing is silent.

### 6.3 Regulatory compliance
- Region profile (US/EU/JP/…) is set at first boot and is signed as part of the provisioning bundle.
- PA cannot be keyed until profile is set.
- Antenna gain is declared (or read from an NFC tag on the antenna); EIRP is capped in software to the regional limit.

---

## 7. Cryptography

### 7.1 Primitives
- **Signatures:** Ed25519 (per node) + BLS12‑381 optional for group aggregate sigs (future).
- **Key exchange:** X25519.
- **AEAD:** XChaCha20‑Poly1305.
- **Hash:** SHA‑256 (RNS default) + BLAKE3 (fast hash for content addressing).
- **KDF:** HKDF‑SHA‑256.
- **Password KDF:** Argon2id for local passphrase unlock.

### 7.2 Peer‑to‑peer messaging — Double Ratchet
- Optional layer on top of RNS Link (already E2E‑encrypted). Enable per‑contact.
- Provides forward secrecy + break‑in recovery.
- Ratchet state stored in encrypted flash namespace.

### 7.3 Group / channel messaging
- Symmetric AEAD with a **shared group key** per epoch.
- Sender‑signed messages so receivers can attribute + verify.
- Membership change (add/remove/rotate) uses `key.rotate` (record 20) — new epoch key wrapped for each remaining member.

### 7.4 Broadcast / public‑safety mode
- Explicit "public SOS" mode transmits `sos.beacon` **in the clear** on a well‑known public destination so any nearby SS‑SP or Meshtastic device can pick it up.
- All other broadcast traffic is encrypted by default.

### 7.5 Anti‑replay
- 96‑bit random nonce + monotonic per‑sender counter. Receivers keep a sliding window (`SS_REPLAY_WINDOW=128`) per sender.

### 7.6 Anti‑Sybil / anti‑flood
- Rate limits on `Announce` acceptance per identity (1 per 3 s).
- Optional **proof‑of‑work** puzzle for new peer admission on public channels (adjustable difficulty).
- Group admins can maintain allow/deny lists (signed roster records).

---

## 8. Time

- GNSS PPS on Alpha → µs‑accurate UTC.
- NTP over Wi‑Fi/BLE‑tether backhaul when available.
- `time.sync` records (type 22) propagate best‑known UTC across the mesh with an accuracy estimate.
- All logs and records are UTC + monotonic; skew statistics tracked per peer.

---

## 9. Quality of Service (QoS)

Priority classes and per‑class treatment:

| Class | Latency target | Drop policy | Bearer preference |
|---|---|---|---|
| **VOICE** | ≤ 200 ms one‑way | drop oldest frame | HaLow AC_VO, LoRa SF7/BW250, ESP‑NOW |
| **ALERT** | ≤ 1 s | never drop | any, retransmit aggressively |
| **INTERACTIVE** | ≤ 2 s | small queue | any |
| **BULK** | best effort | large queue | prefer HaLow / Wi‑Fi backhaul |
| **BACKGROUND** | opportunistic | tail drop | idle only |

QoS is enforced by:
1. Per‑class outbound queues in `ss_mesh`.
2. Per‑class credit at bearers.
3. HaLow WMM mapping (Alpha).
4. LoRa airtime accounting per class per region.

---

## 10. Fuzz + conformance

- Golden‑vector corpus in `tests/vectors/`:
  - RNS Announce with known keys → deterministic output bytes.
  - Every SS‑Ext record type has ≥5 encode/decode round‑trip vectors.
  - Meshtastic bridge in both directions has vectors verifying no data loss for the intersection subset.
- Fuzzers: `libFuzzer` targets for every parser (CBOR record, protobuf compat, RNS packet, LoRa frame, HaLow frame parser).
- Conformance test suite (`tools/conformance/`) that any third‑party implementation must pass to display the "SS‑SP Compatible" mark.

---

## 11. Extending the protocol — RFC process

- Any wire‑format change requires an **RFC** in `docs/rfcs/`.
- Templates provided (`RFC-TEMPLATE.md`).
- RFCs must include: motivation, wire format, encoding examples, backwards‑compat plan, security review, reference implementation link.
- Reference implementation (this repo) is the compliance oracle; RFCs land only after ref impl passes conformance tests.

---

## 12. Sequence examples

### 12.1 Two Lite units message each other over LoRa (no Internet)

```
Node A boot → generate identity if none → RNS announce over LoRaInterface
Node B boot → same → hears A's announce → stores path
User A types "Hello" in Chat app
  → text.message CBOR record wrapped in LXMF
  → RNS packet targeted at B's destination hash
  → LoRaInterface transmits (SF9/BW125)
Node B RX → RNS decrypts link → LXMF payload → SS‑Ext parser
  → Chat app receives text.message → renders in UI
Node B auto‑sends `ack` record (18) → A shows delivered ✓
```

### 12.2 Meshtastic Android app pairs to a Lite unit

```
Phone (Meshtastic app) → BLE scan → finds Lite advertising Meshtastic UUIDs
Phone connects → GATT: fromRadio/toRadio characteristics
Lite exposes Meshtastic-compat proto stream to phone
Phone user sends TextMessage → toRadio → Lite bridge translates → SS‑Ext text.message
  → RNS packet → LoRa TX
Other node RX → SS‑Ext text.message → if paired to a Meshtastic phone,
  reverse-translate → deliver as Meshtastic TextMessage
```

### 12.3 Alpha unit relays a chat from LoRa to HaLow

```
Alpha unit has both LoRaInterface (add-on) and HaLowInterface active
Incoming LoRa record → RNS routes to a next-hop known only over HaLow
  → HaLowInterface transmits at 43 Mbps
End-to-end confidentiality preserved (LXMF payload untouched)
```

### 12.4 Voice PTT on Lite (half-duplex due to mux)

```
User press-and-holds Talk button (touch or BLE HID)
UI → Voice app → ss_mux_acquire(MIC) → I²S RX 20 ms frames → Codec2 encode
On release: ss_mux_release()
  → ss_mux_acquire(RADIO)
  → serialize buffered voice.frame records into LoRa TX burst
  → ss_mux_release()
Receivers reassemble, jitter-buffer, decode, playback
UI badge: "Half-duplex (Lite)"
```

### 12.5 SOS beacon (public safety mode)

```
User triggers SOS (long-press or 3-tap or dead-man switch)
Presence.status set to "sos", severity + GPS attached
sos.beacon record broadcast in the clear on well-known public destination
  every 30 s until user cancels or 24 h expires
Every SS-SP and Meshtastic device in range hears it and displays
Alpha: bezel LEDs pulse red 360°, TTS repeats callsign + coords
```

---

## 13. Open items / to be RFC'd

- Formal spec of `bridge.foreign` per foreign protocol (Matrix event → CBOR mapping etc.).
- Group multicast optimization (BLS aggregate sigs to reduce per‑member key‑rotate size).
- Deterministic replay ID vs random — decide.
- Roaming semantics (moving between HaLow BSSs without dropping RNS link).
- Mesh graph gossip: `mesh.graph` record vs pure RNS observability — pick one.

---

## 14. Summary

The SS‑SP protocol stack is:
- **Reticulum for identity + routing** (proven, MIT, transport‑agnostic).
- **LXMF as the envelope**.
- **A small CBOR record vocabulary (SS‑Ext) for our extra capabilities**.
- **A Meshtastic protobuf compat layer at the phone edge** so we inherit their user base.
- **Bearer‑pluggable transports** — HaLow, LoRa, Wi‑Fi, BLE, ESP‑NOW, USB, sat.
- **XChaCha20‑Poly1305 + Ed25519 + X25519 crypto** with optional Double Ratchet for contacts and epoch‑ratcheted keys for groups.
- **QoS classes** enforced end‑to‑end.
- **Fuzz + golden‑vector conformance suite** as the compliance oracle.
- **RFC process** for any wire change.

This is deliberately a *small* protocol built on top of *proven* protocols. The novelty is in the composition, not in reinventing crypto or routing.
