<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# 03 — SS-SP Architecture

*Comprehensive technical architecture. Reader: engineering.*

Prerequisite reading: `01_BRIEF.md`, `02_PRD.md`. Cross-references: `02_PROTOCOL_STACK.md`, `03_UI_LAYOUT_SPEC.md`, `05_SECURITY_MODEL.md`, `08_UNIVERSAL_CONNECTIVITY.md`.

---

## 1. System-of-systems view

```
+------------------------------------------------------------------+
|                    SS-SP GLOBAL ECOSYSTEM                        |
|                                                                  |
|  +----------------+    +------------------+    +--------------+  |
|  |  DEVICE FLEET  |<-->| RETICULUM PUBLIC |<-->| CLOUD (SS-SP)|  |
|  | Lite/Alpha/Omg |    | TRANSPORT NODES  |    | Fleet/Reg/Prv|  |
|  +----------------+    +------------------+    +--------------+  |
|         ^   ^                                        ^           |
|         |   |                                        |           |
|  +------+   +------+                          +------+---+       |
|  | COMPANION APPS |                           | 3RD PARTY|       |
|  | iOS/Android/etc|                           | CERTIFIED|       |
|  +----------------+                           +----------+       |
|                                                                  |
+------------------------------------------------------------------+
```

Everything is optional except the device fleet. Devices work with no cloud, no companion app, no transport nodes.

---

## 2. Device architecture (firmware)

### 2.1 Layered stack

```
+---------------------------------------------------------------+
|                    APPLICATION LAYER                          |
|  Chats · Contacts · Map · Seekie · Settings · SOS · Onboarding|
+---------------------------------------------------------------+
|                    ss_ui (LAYOUT ENGINE)                      |
|  Aspect-agnostic tokens · Screens · Widgets · Themes · L10n   |
+---------------------------------------------------------------+
|                    MESSAGE LAYER (LXMF)                       |
|  Delivery receipts · Attachments · Groups · S&F                |
+---------------------------------------------------------------+
|                    IDENTITY & CRYPTO LAYER                    |
|  Double Ratchet · MLS-simplified · Signatures · KDF · PQ hyb  |
+---------------------------------------------------------------+
|                    ROUTING (RETICULUM)                        |
|  Announces · Paths · Transport encryption · Hop-by-hop        |
+---------------------------------------------------------------+
|                    ss_link (BEARER ABSTRACTION)               |
|  Bearer selection · Framing · QoS · Power arbitration         |
+---------------------------------------------------------------+
|          LoRa   HaLow   Wi-Fi   BLE   Cellular   USB          |
+---------------------------------------------------------------+
|                    HAL (BOARD-SPECIFIC)                       |
|  Display · Touch · Audio · Radios · Storage · Power · SecEl   |
+---------------------------------------------------------------+
|                    RTOS (FreeRTOS via ESP-IDF)                |
+---------------------------------------------------------------+
|                    HARDWARE (Lite / Alpha / Omega)            |
+---------------------------------------------------------------+
```

### 2.2 Task / process model
- **Application tasks:** UI thread (LVGL-compatible tick), chat controller, map controller, Seekie compass task.
- **Message tasks:** LXMF inbound, LXMF outbound, LXMF S&F worker.
- **Routing task:** Reticulum event loop.
- **Bearer tasks:** one task per bearer (LoRa RX/TX, HaLow, Wi-Fi, BLE); each owns its ISR.
- **System tasks:** watchdog, power manager, mux arbitrator (Lite only), OTA agent, logger.
- **Security tasks:** key ratchet, secure element session, rng entropy pool.

Task priorities set to guarantee SOS beacon path preempts non-emergency work. Full priority table in `firmware/main/task_priorities.h` (to be created).

### 2.3 Concurrency & IPC
- FreeRTOS queues, notifications, event groups.
- Zero shared globals except constant tables. All state hangs off owning-task pointers.
- Message queues typed via generated headers from `protocol/schemas/*.msg`.

### 2.4 Memory posture
- No dynamic allocation on hot path (after boot). Pool allocators for LXMF payloads and RNS frames.
- PSRAM for larger buffers (chat cache, tile cache, voice message ring buffer).
- Deterministic stack sizes checked in CI.

---

## 3. HAL

### 3.1 Header contracts (already in tree)
See `firmware/components/hal/include/`.

Every subsystem exposes:
- `esp_err_t init(void)` / `esp_err_t deinit(void)`
- capability macros in `ss_hal_caps.h`
- concrete typed API (display, touch, audio, radios, etc.)

Boards return `ESP_ERR_NOT_SUPPORTED` cleanly for absent capabilities. Higher layers check `ss_hal_has_cap(SS_CAP_*)` before calling.

### 3.2 Per-board implementations
- `firmware/boards/lite/` — S3 + ILI9488 + GT911 + SX1262 + I²S. Mic/LoRa mux via GPIO 45.
- `firmware/boards/alpha/` — P4 + MM8108 + SX1262 + ES8311 + GNSS + IMU + Bezel LEDs.
- `firmware/boards/omega/` — P4 + LTE modem + cellular power domain.

### 3.3 Mux arbitration (Lite)
`ss_hal_muxctl` mediates GPIO 45 between mic and LoRa. Priority: SOS > active voice capture > LoRa RX > LoRa TX > mic idle. Contention handled with a lock, timeout, and forced release with logged reason.

---

## 4. Networking

### 4.1 SS-Link (bearer abstraction)
See `08_UNIVERSAL_CONNECTIVITY.md` §2.

- Bearer plugin API: `send`, `receive`, `peer_scan`, `link_stats`, `set_power_mode`, `caps`.
- Frame envelope: `[2B: next-header][payload]`.
- Selection algorithm: table-driven (`firmware/components/ss_link/select_policy.c`), tuneable via RFC.

### 4.2 Reticulum interfaces
- `ss_rns_iface_lora` — RNS over LoRa physical.
- `ss_rns_iface_halow` — RNS over UDP over 802.11ah IP.
- `ss_rns_iface_wifi` — RNS over UDP multicast (LAN) + TCP (Internet).
- `ss_rns_iface_cellular` — RNS over TCP over cellular (Omega only).
- `ss_rns_iface_ble` — RNS over BLE GATT (and USB serial) to a paired app node, making phone + device one routing domain (F-NODE-04).

Multi-interface path selection is Reticulum's own (upstream feature).

### 4.3 LXMF
Runs on top of RNS. Payload types include text, voice-message (Opus), attachment, presence, location, group-membership-change, receipt.

### 4.4 Meshtastic wire compat
See `02_PROTOCOL_STACK.md` §2. `ss_meshtastic_compat` translates between Meshtastic frames on LoRa and SS-Link internal frames. Enabled by default; user-disable-able.

### 4.5 Home Gateway
See `08_UNIVERSAL_CONNECTIVITY.md` §7. Composite state machine gated on: docked, external power, trusted-Wi-Fi, user opt-in, battery ≥ 50 %.

### 4.6 Forward-compatibility seams (EUD, IP interop, TAK) — build now, deliver later

The full EUD/interop/app-node capability set (F-EUD-01..03, F-INT-01..04, F-BR-08..09, F-VOX-05..06, F-NODE-01..05, F-ECO-01..03 in `02_PRD.md`) lands after v1.0, but the **seams are frozen in the v1.0 core** so that later delivery is additive, never a redesign. Five seams:

1. **Bearer ABI reserves an L2/IP bearer class.** The bearer plugin `caps` bitfield reserves flags for `IP_NATIVE`, `L2_MESH`, and `CHANNEL_AGILE` from day one, and the scheduler treats unknown bearer classes generically. An 802.11s interop bearer (F-BR-09) or any future IP radio then slots in as a plugin with no ABI break.
2. **Channel-agility hook points in `ss_link`.** Every bearer exposes `interference_metrics()` and an optional `move_channel(within_region_plan)` callback; the scheduler samples metrics even where the v1.0 policy is "never move" (LoRa). Agility policy (F-BR-08) becomes a policy-table change, not a driver change, and stays inside the region plan by construction (NF-REG-04).
3. **IP-over-RNS tunnel is a first-class RNS interface type.** A TUN-semantics interface (`ss_rns_iface_tun`) is specified by RFC — MTU discipline, fragmentation policy, addressing derived from RNS identity — before any consumer exists. EUD tethering (F-EUD-02), the HGW dock Ethernet path, and IP-mesh bridging all become consumers of this one spec.
4. **LXMF reserved fields namespace.** A registered namespace in the LXMF fields map (RFC-governed registry) carries structured third-party payloads — CoT/TAK events (F-INT-01/02), telemetry, future schemas — so interop gateways translate at the edge and **never fork the wire protocol**. Any LXMF implementation (Sideband, Nomad Network, third-party) can adopt the registry entries independently.
5. **The protocol core is portable by construction.** `ss_rns`, `ss_lxmf`, and the crypto core compile as a host-portable library (`ss_node_core`) from v1.0: no ESP-IDF/FreeRTOS types in public headers, platform glue behind a narrow OS-abstraction layer, CI building and testing the host targets alongside firmware. Companion apps and the headless daemon then become full mesh nodes (F-NODE, §9.3) by **embedding the same code the firmware runs** — never by re-implementing the protocol.

Two standing design rules make the whole surface universal: **(a) standard protocols on documented endpoints** — NMEA-0183 for network GPS, CoT for TAK, RTSP/RTP for video, HTTP(S) for the admin console — never bespoke clients; **(b) API parity** — the on-device web console, companion apps, SDKs, and agents all consume the same device API; no capability is console-only or app-only.

---

## 5. Cryptography and identity

### 5.1 Key hierarchy
Root seed `S_dev` sealed in eFuse. All keys derived via HKDF-SHA-256 with domain-separated labels. Full derivation table in `05_SECURITY_MODEL.md` §5.

### 5.2 Messaging cryptography
- 1:1: Signal Double Ratchet on X25519 + XChaCha20-Poly1305.
- Groups: MLS-simplified sender-keys with per-epoch rotation.
- Optional PQ hybrid: X25519 + ML-KEM-768 (KEM output XOR-combined).

### 5.3 Storage cryptography
- At-rest FS: XChaCha20-Poly1305 with per-file nonce; key from `HKDF(S_dev, "ss/storage/fs/aead/v1")`.
- Secrets partition: hardware-flash-encrypted (AES-256-XTS).

### 5.4 OTA cryptography
- Dual signatures (Ed25519) on the whole manifest — `RelKey_A` and `RelKey_B`.
- Anti-rollback counter in eFuse (monotonically increasing).
- Cold rotation via escrowed `RelKey_C` produces a signed key-rotation manifest.

### 5.5 Random number generation
- HW-RNG on ESP32. Entropy pool mixed with SHA-256 in a Fortuna-like scheme.
- Optional hardware TRNG on Alpha/Omega via secure element (if fitted).

---

## 6. Application layer

- `ss_app_chats` — LXMF conversation model, thread state, search.
- `ss_app_contacts` — verified fingerprint book with QR-based verification.
- `ss_app_map` — offline vector tiles + optional online enrichment; render via `ss_ui` map widget.
- `ss_app_seekie` — compass-style peer bearing widget, using GNSS + BMM350 magnetometer.
- `ss_app_settings` — categorised settings screen with search.
- `ss_app_sos` — dedicated one-button flow, cross-bearer beacon, panic wipe.
- `ss_app_onboarding` — first-boot flow (device-alone or companion-assisted).

Each app is a subscriber to the LXMF event bus and the `ss_link` connectivity events. UI screens use `ss_ui` tokens/widgets.

---

## 7. UI (`ss_ui`)

See `03_UI_LAYOUT_SPEC.md` (root-level) for the design token system, aspect-agnostic layout engine, widget catalogue, and theme structure. Renderer targets LVGL v9 initially; abstracted so a canvas backend (e-ink, HMD) can replace it without touching app code.

---

## 8. Voice subsystem

- **STT:** on-device Whisper-tiny.en int8 for voice commands and voice-message transcription. Runs on ESP32-S3 PSRAM (Lite) with degraded latency, and on P4 (Alpha) with real-time performance.
- **TTS:** on-device Piper (int8) for message readout in EN + ES.
- **PTT codec:** Opus 6/12/24 kbps auto-selected.
- **Emergency LoRa voice:** loadable Codec2 module (LGPL isolation) selectable via policy.

Mux arbitration for Lite: SOS > voice-record > mic-idle.

---

## 9. Companion apps

### 9.1 Structure
- `companion/shared/` — TypeScript / Dart shared code (models, protocol client, cryptography).
- `companion/mobile/ios/` — SwiftUI wrapper around shared Dart-via-FFI or full native.
- `companion/mobile/android/` — Compose wrapper around shared Kotlin/Dart-via-FFI or full native.
- `companion/desktop/` — Tauri (target choice — see Q-02 in PRD).
- `companion/web/` — Progressive Web App, BLE-Web + LAN, no cloud required.

### 9.2 Pairing
- BLE-first (LE Secure Connections + OOB via QR).
- LAN mDNS second (`_ss-sp._udp`).
- Cloud-pair third (opt-in encrypted blob relay).

### 9.3 App node modes (`ss_node_core`) — every install is a potential mesh node

Beneath the shared TS/Dart core (UI, sync, models), the apps embed the host-portable `ss_node_core` library via FFI — the **same** `ss_rns` + `ss_lxmf` + crypto code the firmware runs (§4.6 seam 5). Three user-switchable modes:

1. **Companion** (default, today's behaviour) — thin client paired to an SS-SP device; the protocol terminates on the device.
2. **Standalone node** — the app is a full RNS/LXMF node over the host's own IP connectivity (Wi-Fi, cellular, Internet) via RNS TCP/UDP interfaces. It interoperates with any RNS node — SS-SP devices, Sideband, Nomad Network, the Python reference — with no SS-SP hardware present.
3. **Gateway** — opt-in transport role: the app bridges its host's uplinks and any attached device into one routing domain. A BLE/USB-attached device contributes LoRa/HaLow interfaces; the phone contributes Wi-Fi/cell/Internet; Reticulum's own multi-interface routing picks the path. Result: a device with no HaLow peer in range still reaches the mesh through the phone's Internet, and a phone with no signal still messages through the device's LoRa.

Policy: gateway role requires explicit opt-in with a persistent indicator; cellular is marked metered using the same bearer-policy vocabulary as firmware; background behaviour stays within iOS/Android execution and store rules. The headless daemon (F-NODE-05) is the fourth embedding of `ss_node_core` — transport + LXMF propagation node for PCs, servers, and routers.

Ecosystem rule: wearable/vehicle integrations (Garmin Connect IQ, Android Auto/CarPlay, Ride Command-class head units) consume the public app/device API and standard interchange formats (GPX/FIT/KML/GeoJSON, NMEA-0183, CoT) — never vendor-specific protocol forks (F-ECO, `02_PRD.md` §2.16).

---

## 10. Cloud services

### 10.1 Fleet Console
- Node.js + TypeScript backend (or Rust axum — choice ADR pending).
- Postgres store; Redis for real-time; S3-compatible for artifact storage.
- OIDC + SCIM for enterprise IdP integration.
- Multi-tenant, RBAC.
- Deploy target: Kubernetes (Terraform-managed).

### 10.2 Provisioning Service
- HSM-backed ceremony service. Public API for OEM enrolment.
- Immutable transparency log (Rekor-compatible) for every issued identity manifest.
- Postgres store + WORM archive for audit.

### 10.3 Plugin Registry
- Signed-blob CDN (Cloudflare R2 or S3).
- Submission API with automated static + fuzz scanning.
- Review queue UI for `wg-security`.
- Revocation stream consumed by Fleet Console and devices.

### 10.4 Relay Federation Tooling
- Multi-tenant RNS transport orchestration.
- Billing hooks (Stripe).
- Abuse reporting flow.

### 10.5 Community OTA channel
- Static-signed manifests hosted on any TLS endpoint.
- Mirror-friendly (single JSON + signed artifacts).
- No dynamic backend required for the community channel — signed static manifests only.

---

## 11. SDKs

Shared conventions across languages:
- Semantic version aligned to protocol version (major = wire break).
- Core primitives: `Identity`, `Message`, `Bearer`, `Contact`, `Group`.
- Zero-cost abstractions where possible; explicit allocator on C.
- Full test-vector suite from `protocol/testvectors/` runs identically in each language binding.

---

## 12. Data model (device)

Persistent stores (encrypted-at-rest):

- **`identity.blob`** — sealed identity manifest.
- **`contacts.db`** — SQLite with fingerprint verifications.
- **`messages.db`** — SQLite with LXMF messages; TTL policy per user setting.
- **`fs_media/`** — attachments (voice, images); reference-counted; garbage-collected.
- **`settings.blob`** — user prefs; small; versioned.
- **`ota_state.blob`** — A/B slot pointer, rollback counter, staged download.
- **`network_log/`** — opt-in per-bearer ring log.
- **`plugins/`** — sandboxed WASM blobs + capability tokens.

---

## 13. Interface contracts

### 13.1 Wire formats
Every wire format frozen by protocol version (semver). Backwards-compat rules in `06_GOVERNANCE.md` §5.

### 13.2 IPC (device internal)
Between tasks: strongly-typed queues generated from `protocol/schemas/*.msg`.

### 13.3 IPC (device ↔ companion)
Framed CBOR over BLE GATT and LAN TLS. Schema defined in `protocol/schemas/pairing.cbor`.

### 13.4 API (cloud services)
JSON REST v1 with OpenAPI schema; gRPC v2 planned. Idempotency keys required for state-changing endpoints.

### 13.5 API (SDK ↔ device)
Same as IPC 13.3, exposed via SDK wrappers.

---

## 14. Non-functional posture

| Aspect | Approach |
|---|---|
| Performance | Real-time task priorities; SOS pre-emption; UI 60 fps target Alpha, 30 fps Lite. |
| Availability | Zero required backend; multi-bearer fallback; A/B OTA + rollback. |
| Scalability (cloud) | Horizontal via K8s; multi-region for enterprise SLAs. |
| Security | Defense in depth; least privilege; sandboxed plugins; hardware root of trust. |
| Privacy | Data minimisation; on-device processing; opt-in telemetry; k-anonymity ≥ 100. |
| Cost | Hardware BOM budgeted per SKU; cloud cost modelled per enrolled device. |
| Longevity | 5-yr security fix window; escrow instrument; reproducible builds. |

---

## 15. Technology choices with rationale

| Layer | Choice | Rationale |
|---|---|---|
| Firmware RTOS | FreeRTOS via ESP-IDF | Native ESP32 support, well-known, permissive licence. |
| UI toolkit (device) | LVGL v9 | Widely supported, good touch, MIT. |
| Filesystem | LittleFS + optional FAT (SD) | Wear-levelled, crash-safe, MIT. |
| Crypto lib | libsodium + PSA (mbedTLS) | Audited, ubiquitous, permissive. |
| LoRa driver | RadioLib | MIT, active, multi-radio. |
| Reticulum | RNS upstream Python (bridged to C reference) | Original impl; contribute upstream. |
| LXMF | LXMF upstream (bridged to C reference) | Same. |
| WASM runtime | WAMR (Wasm3-compatible mode) | Small, sandboxable, Apache-2.0. |
| Companion mobile | Native iOS/Android with shared Dart | Best UX and platform APIs; shared logic. |
| Desktop | Tauri | Small, Rust-native, cross-platform. |
| Web | Vite + React or Svelte | Modern, small, clear ecosystem. |
| Cloud backend | Rust axum or Node TS (ADR TBD) | Performance + type safety vs velocity. |
| DB | Postgres + Redis + S3-compatible | Boring, works, portable. |
| Orchestration | Kubernetes + Terraform | Enterprise-grade, portable. |
| Transparency log | Sigstore Rekor | Standardised, community. |
| CI | GitHub Actions + self-hosted HIL runners | Portable; HIL needs custom rigs. |

Each choice recorded as an ADR under `docs/dev/adr/`.

---

## 16. Cross-cutting concerns

### 16.1 Observability
- Structured logs with severity levels; no plaintext user data by default.
- Metrics: connectivity histograms, bearer switch rates, OTA outcomes. Opt-in aggregation.
- Traces: distributed tracing on cloud services (OTel).

### 16.2 Feature flags
- Server-side flags for cloud services.
- Compile-time flags for firmware (per-SKU) via `board_config.h`.

### 16.3 Localisation
- `ss_i18n` string extraction; translation memory in `assets/locale/`; runtime lookup.

### 16.4 Accessibility
- Screen-reader parity on device.
- WCAG 2.2 AA on companion apps.
- Colour palette with contrast tested.

### 16.5 Reproducibility
- Deterministic firmware builds; Sigstore attestations; SBOM in CycloneDX.
- Toolchain pinned in `.buildenv/`.

---

## 17. Data flow examples

### 17.1 Ana sends "reached the ridge"
1. UI → `ss_app_chats` composes LXMF message.
2. `lxmf_out` encrypts (Double Ratchet), submits to RNS.
3. RNS picks path via `ss_rns_iface_lora`.
4. `ss_link` picks LoRa bearer (Wi-Fi absent), fragments if needed.
5. LoRa driver TX; other node RX.
6. Recipient RNS reassembles; LXMF decrypts; UI notifies.

### 17.2 SOS
1. Button press → `ss_app_sos` builds emergency LXMF payload with GPS coords.
2. `ss_link` broadcasts on **every** bearer simultaneously.
3. Recipients render high-priority alert with location.
4. Optional: cellular POST to Fleet Console if enrolled.

### 17.3 Home Gateway HaLow ↔ Internet
1. Neighbour SS-SP transmits LXMF over HaLow.
2. Home Gateway HaLow AP RX; framed into `ss_link`.
3. RNS selects `ss_rns_iface_wifi` for onward hop.
4. Wi-Fi station forwards to public RNS transport node.
5. Transport node relays to destination network.

---

## 18. Deployment topology (cloud)

```
+---------+   +-------------------+   +-----------------+
| CDN     |-->|  Ingress (NGINX)  |-->| Fleet Console   |
+---------+   +-------------------+   +-----------------+
                                      | Provisioning Svc|
                                      | Plugin Registry |
                                      | Relay Federator |
                                      +--------+--------+
                                               |
                                      +--------+--------+
                                      | Postgres        |
                                      | Redis           |
                                      | S3-compatible   |
                                      | Rekor           |
                                      | HSM             |
                                      +-----------------+
```

Multi-AZ. Terraform-managed. K8s-native. Backups per compliance window.

---

## 19. Failure and recovery

### 19.1 Device failure modes and recovery
- Boot failure → rollback to previous A/B slot.
- Crash loop → factory-safe partition boot with recovery UI (network log upload optional).
- Storage corruption → LittleFS crash-safe replay; per-store checksums.
- Lost/stolen → duress-PIN wipe; last-known-location beacon via LXMF (opt-in).
- Firmware key rotation → escrow ceremony + signed rotation manifest.

### 19.2 Cloud failure and recovery
- Region failover via K8s multi-cluster.
- OTA channel: static content, CDN-served, mirrorable.
- HSM: multi-vendor, tamper-evident audit.
- Data loss: PITR (point-in-time-recovery) on Postgres 30 d; WORM archive off-site.

---

## 20. Architecture decision records (ADR)

Every non-obvious choice is captured as an ADR under `docs/dev/adr/NNNN-title.md`. ADRs referenced by number in this doc, in PRD, and in stories.

Initial ADRs to author:
- ADR-0001 RTOS choice (FreeRTOS/ESP-IDF)
- ADR-0002 UI toolkit (LVGL v9)
- ADR-0003 WASM runtime (WAMR)
- ADR-0004 Desktop framework (Tauri vs Electron)
- ADR-0005 Cloud backend language (Rust axum vs Node TS)
- ADR-0006 Voice codec choice (Opus + optional Codec2 module)
- ADR-0007 PQ hybrid rollout policy
- ADR-0008 Reproducible build tooling
- ADR-0009 Transparency log (Rekor)
- ADR-0010 Cellular modem vendor (Omega)

---

*See `04_BLUEPRINT.md` for delivery ordering.*

*End of 03 — Architecture.*
