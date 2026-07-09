<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# SS-SP (Seekie‑Speakie Smart Pager) — Master Software Plan
**Scope:** Universal software system for the SS‑SP device family
**First target:** SS‑SP **Lite** (Elecrow CrowPanel Advance 3.5" HMI Meshtastic edition — ESP32‑S3‑WROOM‑1‑N16R8, ILI9488 480×320 IPS + GT911, SX1262 LoRa, I²S mic/spk, microSD)
**Follow-on targets:** SS‑SP **Alpha 1.0** (ESP32‑P4 + MM8108 HaLow + SKY66423 1 W FEM, 2.4" 320×240) and SS‑SP **Omega** (next‑gen)
**Governing philosophy:** Universal, open, mesh-native, sovereign, production-grade, future‑hardened.

**Companion documents in this repo:**
- `01_SS-SP_LITE_HARDWARE_REFERENCE.md` — authoritative Lite pin map, source-verified BOM, capability matrix, bring-up sequence.
- `02_PROTOCOL_STACK.md` — wire format, LXMF↔Meshtastic bridging, RNS-HaLow interface spec.
- `03_UI_LAYOUT_SPEC.md` — aspect-ratio-agnostic layout engine (`ss_ui`).
- `04_LICENSING_AND_FORK_STRATEGY.md` — **binding** commercial-sale licensing decision, Meshtastic fork verdict, OSS component audit. Supersedes §9 below.
- `05_SECURITY_MODEL.md` — threat model, cryptographic primitives, key hierarchy, secure boot, dual-signed OTA, factory provisioning, duress PIN, WASM plugin sandbox. Supersedes any prior security notes.
- `06_GOVERNANCE.md` — governance phases, Steering Committee, Working Groups, RFC process, contribution model, release management, foundation transition. Supersedes §11 below.
- `07_BUSINESS_MODEL_AND_OPEN_SOURCE.md` — **binding** monetisation strategy, community-vs-commercial edition boundary, anti-rug-pull rules, certification program.
- `08_UNIVERSAL_CONNECTIVITY.md` — **binding** multi-bearer mesh design, SS-Link abstraction, bearer selection algorithm, RNS/LXMF universal underlay, Meshtastic-compat as LoRa profile, Home Gateway Mode (HaLow/LoRa/Wi-Fi bridge + range extender).
- `governance/OPEN_ASSURANCE.md` — public anti-rug-pull commitments.
- `docs/TRADEMARK.md` — trademark policy.

---

## Table of contents

- [0. Executive Summary](#0-executive-summary)
- [1. Product Assessment — What SS‑SP Actually Is](#1-product-assessment--what-sssp-actually-is)
- [2. Full Feature & Capability Inventory (What This HW Can Actually Do)](#2-full-feature--capability-inventory-what-this-hw-can-actually-do)
- [3. Software Architecture — Layered, Universal, Future‑Hardened](#3-software-architecture--layered-universal-futurehardened)
- [4. HaLow + Reticulum Integration — the Hard Part](#4-halow--reticulum-integration--the-hard-part)
- [5. UI Framework — Universal Multi‑Aspect Design](#5-ui-framework--universal-multiaspect-design)
- [6. Security, OTA, Hardening](#6-security-ota-hardening)
- [7. Cross‑Cutting Concerns](#7-crosscutting-concerns)
- [8. Recommended Repo & Codebase Layout](#8-recommended-repo--codebase-layout)
- [9. Meshtastic Fork Strategy — Decision Matrix](#9-meshtastic-fork-strategy--decision-matrix)
- [10. Business / Logistics / Governance](#10-business--logistics--governance)
- [11. Roadmap — Ordered, No Time Estimates](#11-roadmap--ordered-no-time-estimates)
- [12. Immediate Next Actions (What to Do This Week)](#12-immediate-next-actions-what-to-do-this-week)
- [13. Open Questions to Confirm Before Coding](#13-open-questions-to-confirm-before-coding)
- [14. Guiding Principles (nail these to the door)](#14-guiding-principles-nail-these-to-the-door)

## 0. Executive Summary

The SS‑SP is a **rugged, sovereign, off‑grid tactical/civilian smart pager and mesh gateway**. It fuses:

- **Wi‑Fi HaLow (802.11ah)** as the long‑range, high‑throughput mesh backbone (16 km ground, 100 km LoS, up to 43 Mbps).
- **Reticulum (RNS)** as the transport‑agnostic, cryptographic, unstoppable mesh identity/routing layer.
- **Meshtastic‑style application UX** (channels, chats, position, telemetry) as the familiar UI/protocol veneer.
- **Voice + text + presence + location** as first‑class citizens: on‑device STT/TTS, digital voice, "Seekie" compass‑pointer.
- A **universal, resolution/aspect‑ratio‑agnostic UI framework** so the same firmware runs on 3.5" rectangular Lite, 2.4" square‑ish Alpha, and future round/circular/eink/HMD variants.
- A **mass‑adoption, open‑source, mesh‑of‑meshes protocol stack** designed to interoperate with Meshtastic, LoRa RNodes, AREDN, LibreMesh, Iridium/Starlink gateways, TAK/ATAK, and generic MQTT/Matrix/Nostr backhauls.

Deliverable is not just firmware. It is a **complete ecosystem**: firmware, phone apps, mesh protocol, cloud‑optional bridge, developer SDK, hardware abstraction layer, factory provisioning, OTA, and a governance model that positions the SS‑SP as the **de‑facto open standard for sovereign smart‑pager mesh**.

---

## 1. Product Assessment — What SS‑SP Actually Is

### 1.1 Concept in one sentence
A pocket‑sized, waterproof, 1W long‑range mesh communicator that reads/speaks messages, points at people/places, works with no cell/internet, bridges to your phone, and forms a self‑healing mesh with any number of peers.

### 1.2 Product tiers

| Tier | MCU | Radio | Display | Purpose |
|---|---|---|---|---|
| **Lite (v1 — you build now)** | ESP32‑S3 (CrowPanel Advance 3.5") | Wi‑Fi 4 (2.4 GHz) + BLE 5 + wireless header: **HaLow module (dev fleet is HaLow‑fitted — D‑0013)** or LoRa SX1262 | 3.5" IPS 320×480 cap‑touch | Dev kit / entry unit / first product / bench validation |
| **Alpha 1.0** | ESP32‑P4 + ESP32‑C6 bridge | MM8108 HaLow (900 MHz) + 1W SKY66423 FEM | 2.4" IPS 320×240 + 12‑LED bezel | Flagship production tactical pager |
| **Omega (next)** | TBD (RISC‑V + Linux SoM possible) | HaLow + LoRa + Cellular fallback + LEO SatCom | Larger IPS / OLED, dual‑radio | Enterprise / heavy‑duty |

**Omega hardware baseline (D-0020, 2026-07-09):** the Omega row above is the roadmap intent; the *signed-off* Omega v1.0 board (PCB release v69, 2026-07-08) resolves it as **ESP32-P4 SoM + ESP32-C6 bridge + MM8108 HaLow** with GNSS/magnetometer/haptics/3.92″ IPS — **no LoRa, no cellular, no satellite modem, no expansion interface** on v1.0 (those defer to a board revision). Authoritative part list: `docs/dev/OMEGA_HW_BASELINE.md`.

The Lite is the **software canary** — every subsystem must be written so it lights up on Lite, then scales to Alpha unchanged.

### 1.3 Real hardware envelope (Lite — CrowPanel Advance 3.5" HMI, Meshtastic edition)
Verified against Elecrow wiki, Elecrow-RD GitHub, and Meshtastic `variants/esp32s3/elecrow_panel/`. See `01_SS-SP_LITE_HARDWARE_REFERENCE.md` for the authoritative pinout, source citations, and bring-up sequence.

- **MCU:** **ESP32‑S3‑WROOM‑1‑N16R8** (Xtensa LX7 dual‑core @ 240 MHz, 512 KB SRAM, 8 MB PSRAM (octal), 16 MB flash, Wi‑Fi 4 b/g/n 2.4 GHz, BLE 5.0). USB‑Serial‑JTAG native.
- **IO expander / peripheral MCU:** STC8H1K28 (backlight PWM 0–244, buzzer, touch reset seq, amp power).
- **Display:** 3.5" IPS **480×320**, driver **ILI9488** on SPI (SCLK 42, MOSI 39, DC 41, CS 40, BL PWM 38), 400 cd/m², 178° VA, 16‑bit color.
- **Touch:** **Goodix GT911** capacitive single‑touch on I²C (SDA 15, SCL 16, INT 47, RST 48, addr 0x5D).
- **LoRa radio (Meshtastic edition):** **Semtech SX1262** on separate SPI (CS 0, SCK 10, MISO 9, MOSI 3, RST 2, DIO1 1, DIO2 46), TCXO 3.3 V, U.FL antenna.
- **Wireless-module header** also accepts ESP32‑H2, ESP32‑C6, nRF24L01 as alternates.
- **Audio in:** I²S digital MEMS mic (BCLK 9, WS 3, DIN 10) — **shares SPI pins with LoRa via mode mux on GPIO 45**.
- **Audio out:** I²S speaker path + onboard amp (BCLK 13, WS 11, DOUT 12, mute 21 on V1.2+).
- **Buzzer:** GPIO 8 PWM.
- **Storage:** microSD in soft‑SPI (MOSI 6, MISO 4, SCK 5, CS 7).
- **Serial:** UART0 (RX 44 / TX 43) console; UART1 (RX 18 / TX 17) aux.
- **USB:** USB‑C native (D+/D‑ from S3), power 5 V/2 A.
- **Battery:** LiPo JST‑PH + TP4056‑class charger; **no battery voltage sense pin exposed** — SoC% is unavailable in software.
- **LEDs:** PWR + CHG (hardware only, no user GPIO).
- **Buttons:** BOOT + RESET only. **No user UI buttons** — all UI input is capacitive touch (single‑point) unless BLE‑HID pendant is paired.
- **Sensors:** RTC + coin cell backup on I²C bus (shared with touch).
- **Enclosure:** plastic, non‑IP‑rated. Dimensions 101.4 × 63.3 × 15.8 mm. Weight 120 g.
- **Op temp:** −20 to +70 °C.

**HW consequence to design around:** GPIO 45 is a mux — HIGH = mic path enabled, LOW = wireless (SX1262/H2/C6/nRF24) path enabled. Long‑range voice on Lite is therefore **half‑duplex** (capture → switch → transmit). This is enforced by `hal_muxctl`. Alpha does not have this constraint.

### 1.4 Real hardware envelope (Alpha 1.0 — per your BOM)
- **MCU:** ESP32‑P4NRW32X Rev 3.1 (dual‑core RISC‑V @ 400 MHz, 32 MB PSRAM, H.264 accel, 16‑bit parallel LCD)
- **Bridge MCU:** ESP32‑C6‑MINI‑1U (Wi‑Fi 6 + BLE 5.4 for phone tether)
- **Mesh radio:** Morse Micro **MM8108** (802.11ah HaLow, 900 MHz, up to 43.33 Mbps)
- **RF FEM:** Skyworks **SKY66423‑11** (1 W / 30 dBm PA + LNA)
- **GNSS:** u‑blox **MIA‑M10Q‑00B** dual‑band L1/L5
- **Magnetometer:** Bosch **BMM350** (compass/heading for "Seekie")
- **Audio codec:** ES8311 + NS4150B 3 W Class‑D amp + MSM381A digital MEMS mic
- **Haptics:** TI **DRV2625YFFR** closed‑loop LRA driver + 1027 LRA
- **Display:** ER‑TFT024IPS‑3 2.4" 320×240 IPS + cap touch
- **Bezel:** 12× SK6805‑EC15 addressable RGB
- **Power:** EA3059QDR 4‑ch PMIC + TP4056 charger + MAX17048 fuel gauge + 5000 mAh LiPo
- **Storage:** microSD (SDIO 2.0)
- **I/O:** USB‑C (IP‑rated mid‑mount), IP‑rated buttons, SMA bulkhead antenna, PCA9555 GPIO expander
- **Rugged:** IPC Class 2 + Class 3 RF path, IP65/66 sealing (D-0021: Omega v1.0 targets **IP65/66**; IP68 is aspirational for a later rev — `docs/dev/OMEGA_HW_BASELINE.md`), -40 to +85 °C

---

## 2. Full Feature & Capability Inventory (What This HW Can Actually Do)

Grouped by capability layer. Every item is derived from the real silicon in the BOM.

### 2.1 Communication
- HaLow mesh chat (short + long messages, store‑and‑forward)
- HaLow digital voice PTT (Opus/Codec2 encoded)
- HaLow **broadcast beacon / SOS**
- HaLow **live video micro‑frames** (H.264 hardware‑accelerated where the SoC supports it — the P4 has a HW H.264 encoder; **Lite/ESP32‑S3 has none, so Lite is JPEG stills only**, D-0021 `docs/dev/OMEGA_HW_BASELINE.md`)
- **Reticulum** identity + LXMF messaging + micronets (transport‑agnostic — works over HaLow, LoRa, TCP, Wi‑Fi, serial)
- **BLE tether** to phone (companion app is a "remote UI + backhaul")
- **Wi‑Fi 6 hotspot / STA** (Alpha via C6, Lite native) for internet bridging when available
- **Optional LoRa** expansion (Lite header, future Omega dual‑radio) — bridged to HaLow via router (D-0021: **Omega dual‑radio = post‑v1.0 board respin** — v69 has **no expansion interface**, so it cannot be added as a module; see D-0020 note §1 and `docs/dev/OMEGA_HW_BASELINE.md`)
- **Cross‑protocol gateway** (HaLow ↔ LoRa ↔ MQTT ↔ Matrix ↔ Meshtastic ↔ APRS ↔ Nostr ↔ Iridium/Starlink IP)

### 2.2 Multimodal I/O
- **STT** (on‑device via Whisper‑tiny / Vosk‑lite; cloud‑assisted via phone)
- **TTS** (on‑device via Piper / on‑MCU Talkie‑style formant synth; cloud‑assisted fallback)
- **Digital voice calls** (full‑duplex on HaLow, half‑duplex PTT on LoRa)
- **Audio‑reactive bezel** (VU meter, direction pulse)
- **Haptic feedback** patterns (LRA library)
- **Cap touch UI** + system‑level side keys (per‑SKU input, D-0021 `docs/dev/OMEGA_HW_BASELINE.md`: **Omega v69 = 4 side keys** power/vol+/vol−/reset, touch‑primary, **no rocker/D‑pad**; **Lite** = touch + BLE/USB‑HID; **Alpha** input set TBD at lock)
- **Voice wake word** (optional, "Hey Seekie")

### 2.3 Location, Navigation, Sensing
- Dual‑band GNSS position, PPS time sync (Alpha)
- **Seekie compass**: BMM350 heading fused with GNSS bearing to a target peer/waypoint → bezel LED points
- Offline map tiles (SD storage, MBTiles/PMTiles)
- Geofencing, breadcrumb, track‑back
- Rally‑point / muster‑point broadcast
- Man‑down / dead‑man switch (**hardware‑gated**, D-0021 `docs/dev/OMEGA_HW_BASELINE.md`: Omega v69 has **no IMU** so no wake‑on‑motion source; Lite via the optional IMU module S-03-036; Alpha TBD at lock — else motion inferred via mic/haptics feedback)
- Ambient light + temperature (via ESP32 internal ADC / added sensor)

### 2.4 Data / Payload
- Encrypted text messaging (per‑channel + per‑contact keys)
- File transfer (chunked, resumable, CRDT‑merged)
- Sensor telemetry (JSON‑CBOR)
- Micro‑video/still image share (JPEG fragments on all SKUs; H.264 fragments only where a HW encoder exists — P4/Omega, **not** Lite/S3, D-0021)
- Distributed key/value store (CRDT — for shared maps, waypoints, roster)
- **Offline map + waypoint sync** across mesh (only diffs propagate)

### 2.5 Identity, Security, Sovereignty
- **Reticulum identity** (Ed25519 + X25519, no central authority, self‑sovereign)
- Per‑channel symmetric keys + per‑user asymmetric keys
- **Forward secrecy** via ephemeral session keys (Signal‑style double ratchet optional layer)
- **Secure boot** + **flash encryption** (ESP32‑S3 / P4 native)
- Hardware‑backed key store (eFuse + optional external ATECC608 — D-0021: SE scoped to the **Lite D-0013 attachment** now / **Omega rev-2** later; no on‑board SE on Omega v69, `docs/dev/OMEGA_HW_BASELINE.md`)
- **Firmware signature** + **anti‑rollback** counter in eFuse
- Panic wipe / plausible‑deniability duress PIN
- **No‑cloud mode** as a first‑class deployment
- Optional Tor / Reticulum‑over‑Tor for backhauled traffic

### 2.6 Fleet / Ops
- OTA over HaLow, USB, and Wi‑Fi (delta + signed)
- Remote diagnostics (opt‑in) via Reticulum admin channel
- Provisioning QR / NFC handoff (**NFC future‑board‑gated** — no NFC on any v1.0 board, D-0021 `docs/dev/OMEGA_HW_BASELINE.md`)
- Battery health, TX/RX counters, mesh graph telemetry
- Group management: roles (admin/operator/observer), channel ACLs
- Tactical overlays (ATAK/CoT interop bridge on phone side)

### 2.7 Developer / Extensibility
- **Plugin system** on device (Lua or WASM sandbox)
- Companion‑app plugin SDK (React Native + Flutter both)
- Public SDKs for HaLow mesh, Reticulum, and the SS‑SP UI toolkit
- Debug console via USB CDC and Reticulum
- Simulator / desktop build (Linux/macOS) for CI

### 2.8 Physical UX (Bezel + Haptics + Display)
- 12‑LED status ring: connectivity, direction, VU, alerts, breathing modes
- Haptic library: acknowledge, error, incoming, direction‑tick, SOS pulse
- 3‑layer UI: **glance** (bezel + haptics only) → **skim** (main tile) → **deep** (full screen)

---

## 3. Software Architecture — Layered, Universal, Future‑Hardened

```
┌───────────────────────────────────────────────────────────────────┐
│  L7  Applications                                                 │
│      Chat • Voice PTT • Seekie • Map • SOS • Roster • Plugins     │
├───────────────────────────────────────────────────────────────────┤
│  L6  Universal UI Framework (LVGL‑based, aspect‑ratio agnostic)   │
│      Themes • Layouts (rect/square/round) • Input abstraction     │
├───────────────────────────────────────────────────────────────────┤
│  L5  Application Protocol Layer                                   │
│      LXMF (Reticulum msg) • Meshtastic‑compat protobuf • CoT/ATAK │
│      • MQTT‑bridge • Matrix‑bridge • Nostr‑bridge                 │
├───────────────────────────────────────────────────────────────────┤
│  L4  Mesh Identity + Routing (Reticulum RNS)                      │
│      Cryptographic identities • Path discovery • Announces        │
│      • Transport‑agnostic • Ratchets                              │
├───────────────────────────────────────────────────────────────────┤
│  L3  Mesh Data Plane                                              │
│      Store‑and‑forward • CRDT KV • File chunker • Voice frames    │
├───────────────────────────────────────────────────────────────────┤
│  L2  Transport Interfaces (pluggable)                             │
│      HaLow driver • Wi‑Fi 4/6 • BLE • LoRa • USB CDC • TCP/UDP    │
│      • Serial (Meshtastic UART) • Iridium/Sat modem               │
├───────────────────────────────────────────────────────────────────┤
│  L1  Hardware Abstraction Layer (HAL)                             │
│      Display • Touch • LED ring • Audio codec • MIC • GNSS • IMU  │
│      • Haptics • PMIC • Fuel gauge • Buttons • SD                 │
├───────────────────────────────────────────────────────────────────┤
│  L0  Base OS: ESP‑IDF (FreeRTOS) — Lite & Alpha                   │
│      Zephyr port target for Omega / Linux SoM                     │
└───────────────────────────────────────────────────────────────────┘
```

### 3.1 Base OS choice
- **ESP‑IDF (FreeRTOS)** for Lite (S3) and Alpha (P4). Native support, largest ecosystem, secure boot, OTA, encrypted flash, PSRAM handling.
- **Zephyr** as a secondary target (for future non‑Espressif SoCs and for teams that prefer it). All L1+ code must be OS‑abstracted (thin `os_port.h`).
- **Linux (buildroot)** target for Omega and desktop simulator.

### 3.2 Hardware Abstraction Layer (L1)
A single `hal_*.h` API per subsystem, with per‑board `board_lite.c`, `board_alpha.c` implementations. This is the **one place** hardware differences live. Everything above L1 is board‑agnostic.

```
hal_display.h  hal_touch.h  hal_led.h  hal_audio.h  hal_mic.h
hal_gnss.h     hal_imu.h    hal_haptic.h hal_pmic.h hal_fuelgauge.h
   // hal_imu.h is capability-flagged per SKU (D-0021): SS_CAP_IMU=0 on Omega v69 (no IMU); present on Lite via optional module S-03-036; Alpha TBD at lock.
hal_button.h   hal_storage.h hal_radio.h (HaLow, LoRa, Wi‑Fi, BLE)
hal_power.h    hal_secure.h  hal_time.h
```

Board definitions live in `boards/lite/board_config.h` and `boards/alpha/board_config.h` and are selected at compile time by a Kconfig menu.

### 3.3 Mesh identity + routing (L4) — Reticulum
**Why Reticulum:** transport‑agnostic, cryptographic identity, no central authority, works over any bearer (HaLow, LoRa, TCP, serial), well‑engineered, MIT‑licensed. It's the closest thing to "TCP/IP for sovereign meshes."

**How we use it:**
- Port `RNS` core to ESP‑IDF (there is community work — `RNode` firmware is on ESP32; we extend for HaLow bearer).
- Add a new **HaLow interface** to RNS (`HaLowInterface` — subclasses `Interface`, wraps the MM8108 SDIO driver).
- Add a **LoRa interface** on Lite for expansion.
- Add a **BLE interface** for phone tether.
- Use **LXMF** as the delivery layer for messages, files, sensor payloads.
- Use RNS **announces** for peer discovery + Seekie targeting.

### 3.4 Application protocol layer (L5) — the "Meshtastic question"

**Recommendation: fork Meshtastic's *app UX and companion apps*, but keep our own mesh layer.**

- Meshtastic's protobuf messages (`TextMessage`, `Position`, `NodeInfo`, `Waypoint`, `Telemetry`, `RouteDiscovery`) are excellent, widely adopted, and have mature phone apps (iOS + Android + web).
- Meshtastic's underlying **radio layer is LoRa‑only and single‑channel**. It is not designed for HaLow's 43 Mbps.
- Reticulum is the better routing/identity substrate.

**So we do a *dual‑protocol* architecture:**

1. **Native mode (Reticulum + LXMF over HaLow / LoRa / BLE / TCP)** — the "real" SS‑SP mesh, high‑bandwidth, sovereign.
2. **Meshtastic‑compat mode** — expose a Meshtastic‑compatible **serial/BLE** interface so existing Meshtastic phone apps *just work* out of the box for text/position/telemetry. A translation layer maps `TextMessage` ↔ LXMF, `Position` ↔ RNS announce, etc.
3. **Bridge mode** — every SS‑SP is also a Meshtastic gateway, an MQTT client, a Matrix bridge, a Nostr relay (opt‑in). This is how we get mass adoption: **users don't have to abandon their existing tools.**

We also fork the **Meshtastic Android/iOS/web clients** into `SeekieSpeakieUI` companion apps (Apache 2.0 license permits) — reskinned, with our extra features (Seekie, HaLow voice, Reticulum identity, etc.).

### 3.5 UI framework (L6) — LVGL, universal
- **LVGL 9.x** as the graphics engine (proven on ESP32, supports touch, animations, themes, custom widgets).
- Build a **`ss_ui/` layout engine on top of LVGL** that:
  - Takes a *logical* screen (grid of tiles), and *renders* to the physical panel using layout templates.
  - Ships templates for: **rectangular portrait (Lite 320×480), rectangular landscape, square (240×240), round (240/320/480 circular), tall bar (128×320), e‑ink (SPI), HMD/glass mono**.
  - All widgets are **DPI‑ and aspect‑ratio‑aware**, use relative units, and support **safe‑area insets** (for bezels, notches, round corners).
- Input abstraction: capacitive touch, system‑level side keys, voice — all normalized to a single `ss_input_event_t` (rotary/rocker are roadmap input classes, **not present on any v1.0 board**; Omega v69 exposes 4 side keys only, D-0021 `docs/dev/OMEGA_HW_BASELINE.md`).
- **Themes**: Tactical (dark, high‑contrast), Civilian (light, friendly), Night‑vision (deep red), High‑vis (yellow/black), Accessibility (large font, screen‑reader over TTS).

### 3.6 Application layer (L7)
Every "app" is a plugin with a manifest:

```
apps/
  chat/          # LXMF chat + Meshtastic text
  voice/         # PTT + full‑duplex voice
  seekie/        # compass pointer to peer
  map/           # offline tiles + waypoints
  sos/           # emergency beacon + man‑down
  roster/        # peer directory + presence
  telemetry/     # sensor + battery + mesh graph
  settings/      # provisioning, keys, radio config
  plugins/       # user‑installed Lua/WASM
```

---

## 4. HaLow + Reticulum Integration — the Hard Part

### 4.1 HaLow driver
- Use Morse Micro's official ESP‑IDF integration: the **`morsemicro/halow` component from the ESP Component Registry** (Apache‑2.0; the older `mm-iot-esp32` repo is deprecated). Host interface: for **Omega the answer is fixed — MM8108 on SDIO** per the released v69 board (D-0021, `docs/dev/OMEGA_HW_BASELINE.md`); the SDIO-vs-USB selection spike **S‑04‑023 remains open only for the unlocked Alpha board** (MM8108 supports both; see `docs/portfolio/08_HALOW_TECHNOLOGY_DOSSIER.md`). Wrap in `hal_radio_halow.c`.
- Expose two modes:
  - **Infrastructure**: SS‑SP acts as AP or STA on an 802.11ah network (for interop with commercial HaLow deployments — AREDN‑HaLow, industrial IoT gateways).
  - **Mesh (802.11s over HaLow)**: peer‑to‑peer mesh using 802.11s protocol on HaLow — this is what enables the tactical use case with no infrastructure. Morse Micro's stack supports 802.11s.
- **Regulatory:** MM8108 supports region‑locked channels (US 902‑928, EU 863‑868, JP 916‑927, etc.). Region is chosen at first‑boot provisioning + can be overridden by signed operator profile. **We must not ship a firmware that violates regional TX power/duty‑cycle rules.**

### 4.2 Reticulum over HaLow
- Implement `HaLowInterface(Interface)` in the RNS C port. Each frame is an RNS packet inside an 802.11s data frame (or a raw L2 frame if 802.11s unavailable).
- MTU: HaLow supports large MTUs; RNS handles fragmentation. Set `HW_MTU = 500` to stay safe on lossy links.
- QoS: RNS packet priorities map to 802.11e WMM access categories (voice → AC_VO, background sync → AC_BK).

### 4.3 Multi‑transport bridging
Every SS‑SP is inherently a **transport bridge**:
- HaLow ↔ LoRa (with add‑on module)
- HaLow ↔ Wi‑Fi (via C6 on Alpha; native on Lite)
- HaLow ↔ BLE tether (to phone → phone's internet → RNS TCP interface)
- HaLow ↔ USB serial (to a laptop / Meshtastic app / RNode compatible)

Bridging is done at L4 (RNS) so **routing is unified** — no double‑NAT, no duplicate identities.

### 4.4 Reticulum's role vs Meshtastic's role — clearly separated

| Concern | Reticulum | Meshtastic protocol |
|---|---|---|
| Identity | ✅ Ed25519, self‑sovereign | ❌ Random 32‑bit node ID |
| Encryption | ✅ Native, per‑hop + end‑to‑end | ⚠️ Channel PSK only |
| Routing | ✅ Multi‑hop, path discovery | ✅ (limited, 7 hops) |
| Transport | ✅ Any bearer | ❌ LoRa‑centric |
| Apps we adopt | LXMF, Nomad Net, Sideband | TextMessage, Position, Waypoint proto |
| Phone apps we fork | Sideband (already Reticulum) | Meshtastic Android/iOS/Web |

We use **Reticulum as the trust + routing spine**, and **Meshtastic's protobufs + phone apps as a compatibility surface** for mass adoption.

---

## 5. UI Framework — Universal Multi‑Aspect Design

### 5.1 Lite (3.5", 320×480 portrait)
- 480 px tall × 320 px wide. Enough room for a real 3‑pane UI:
  - **Top status bar** (24 px): battery, mesh peers, radio state, time, GNSS fix.
  - **Content** (~400 px): active app.
  - **Bottom nav** (56 px): 4‑tab app switcher (Chat / Seekie / Map / Menu).
- Design at 1× (native 320 px width). Use LVGL 9 flex + grid.

### 5.2 Alpha (2.4", 320×240 landscape) + 12‑LED bezel
- Landscape orientation. Bezel LEDs become part of the UI (peripheral cues).
- Bottom nav becomes a **4‑icon side rail** (right edge).
- Top status collapses to a **left rail** to preserve map/chat area.

### 5.3 Round / circular targets (future)
- 240×240 round or 320×320 round.
- Layout switches to **radial**: center content, bezel = LED ring (physical or virtual), rotary crown navigation.

### 5.4 Aspect‑ratio abstraction
Define a **layout descriptor**:

```
layout: {
  shape: rect | square | round | bar | eink
  width_px, height_px, dpi
  safe_area: {top, right, bottom, left}
  bezel_led_count: N   // 0 for none
  input: [touch, rocker, rotary, voice, physical_keys]   // D-0021: enumerable classes; actual fit is per-SKU — Omega v69 = touch + 4 physical_keys only (no rocker/rotary)
}
```

Each app declares which layouts it supports and provides layout‑specific view files:

```
apps/chat/
  view.rect_portrait.c
  view.rect_landscape.c
  view.square.c
  view.round.c
  view.bar.c
```

The `ss_ui` core loads the correct view for the running board. **One codebase, every form factor.**

### 5.5 Themes + accessibility
- Default themes: `tactical_dark`, `civilian_light`, `nightvision_red`, `highvis`, `accessible_xl`.
- Every widget respects a `ThemeToken` (color, spacing, radius, font).
- **Full screen‑reader mode**: TTS reads focused element; haptic tick on focus change; bezel LED highlights focused edge.

---

## 6. Security, OTA, Hardening

### 6.1 Threat model
Adversaries considered:
- Nation‑state passive/active RF interception.
- Physical seizure of a device (compelled unlock).
- Supply‑chain firmware tampering.
- Mesh flooding / Sybil / eclipse attacks.
- Companion‑phone malware.

### 6.2 Controls
- **Secure boot v2** + **flash encryption** (ESP32‑S3/P4 native), keys in eFuse.
- Firmware signing with **two‑key** scheme (vendor key + operator key). Anti‑rollback via eFuse monotonic counter.
- **Reticulum identity** stored in encrypted namespace on flash + optional ATECC608 secure element (D-0021: SE = **Lite D-0013 attachment** now / **Omega rev-2** later; no on‑board SE on Omega v69, `docs/dev/OMEGA_HW_BASELINE.md`).
- Duress PIN → panic wipe (fills PSRAM + flash region with random, then reboots to factory).
- Deniable dual‑volume mode (real vs cover‑persona identity).
- Mesh‑layer protections: RNS announces are rate‑limited; **proof‑of‑work challenge** on new peer to slow Sybil; per‑channel admission control.
- Every RF TX gated by regulatory profile (region + antenna gain declared).

### 6.3 OTA strategy
- **Signed A/B partitions**. Fallback on boot failure. Delta updates via bsdiff.
- OTA transports (priority order): USB → Wi‑Fi → HaLow → LoRa. Same signed artifact.
- Fleet operators can push signed profiles (channels, region, keys) without full firmware update.

### 6.4 Provisioning
- QR‑code + BLE onboarding (Wi‑Fi Easy Connect style).
- NFC handoff for zero‑touch mesh join (**future‑board‑gated** — no NFC on any v1.0 board, D-0021 `docs/dev/OMEGA_HW_BASELINE.md`; Alpha TBD at lock).
- Factory‑programmed unique key + serial + eFuse burn; certificate of provenance signed by vendor CA.

---

## 7. Cross‑Cutting Concerns

### 7.1 Power management
- Wake‑on‑radio (HaLow TWT — target wake time — for long battery life).
- Wake‑on‑button (wake‑on‑motion is **hardware‑gated** — requires an IMU, **absent on Omega v69**; Lite via optional module S-03-036; Alpha TBD at lock, D-0021 `docs/dev/OMEGA_HW_BASELINE.md`).
- Idle: <5 mA target on Alpha (RTC + fuel gauge + radio TWT).
- TX bursts: 1 W radio only when actually transmitting; PA fully shut down via SKY66423 CSD.
- Per‑app power budget (declared in manifest).

### 7.2 Time
- GNSS PPS on Alpha (µs‑accurate).
- NTP over BLE tether or Wi‑Fi backhaul.
- Mesh time sync (RNS timestamped announces) as fallback.
- All logs UTC + monotonic.

### 7.3 Storage
- SD card layout: `/maps/`, `/voices/`, `/logs/`, `/plugins/`, `/keys/`, `/updates/`.
- Log rotation, ring‑buffer telemetry.
- CRDT KV stored under `/state/`.

### 7.4 Observability
- Structured logs (CBOR) + human‑readable console.
- On‑device mesh graph visualizer (in Roster app).
- Optional remote telemetry to an operator‑controlled server (never a public cloud by default).

### 7.5 Internationalization
- All strings via `i18n_t("KEY")`. Ship EN + ES + FR + DE + JA + AR + ZH day 1.
- RTL support in LVGL.

---

## 8. Recommended Repo & Codebase Layout

```
SS-SP-SOFTWARE/
├── 00_MASTER_SOFTWARE_PLAN.md          (this file)
├── docs/
│   ├── architecture.md
│   ├── protocol.md
│   ├── security.md
│   ├── ui_layouts.md
│   ├── halow_bringup.md
│   ├── reticulum_port.md
│   └── governance.md
├── firmware/
│   ├── CMakeLists.txt
│   ├── sdkconfig.defaults
│   ├── boards/
│   │   ├── lite/          # CrowPanel 3.5"
│   │   ├── alpha/         # SS-SP-Alpha 1.0
│   │   └── sim/           # Linux simulator
│   ├── components/
│   │   ├── hal/           # L1 HAL
│   │   ├── ss_radio/      # L2 transports
│   │   ├── ss_mesh/       # L3 data plane
│   │   ├── rns_port/      # L4 Reticulum (C port + HaLow interface)
│   │   ├── ss_proto/      # L5 app proto (LXMF + Meshtastic compat)
│   │   ├── ss_ui/         # L6 universal UI on LVGL
│   │   └── ss_apps/       # L7 apps
│   └── main/
├── companion/
│   ├── android/           # fork of Meshtastic Android
│   ├── ios/               # fork of Meshtastic iOS
│   ├── web/               # PWA
│   └── desktop/           # Tauri wrapper
├── sdk/
│   ├── c/                 # native plugin SDK
│   ├── lua/               # scripting SDK
│   └── wasm/              # sandboxed plugin SDK
├── tools/
│   ├── flasher/           # web + CLI flasher
│   ├── provisioner/       # factory tool
│   ├── simulator/         # desktop virtual device
│   └── protocol-fuzzer/
├── protocol/
│   ├── ss_proto.proto     # our extensions
│   ├── meshtastic_compat/ # vendored .proto files
│   └── lxmf_extensions/
├── infra/
│   ├── ota-server/        # self‑hostable
│   ├── mqtt-bridge/
│   ├── matrix-bridge/
│   └── nostr-bridge/
├── ci/
│   ├── github/
│   ├── hardware-in-loop/
│   └── coverage/
└── LICENSE  (Apache-2.0 recommended — matches Meshtastic & LVGL)
```

---

## 9. Meshtastic Fork Strategy — Decision Matrix

| Option | Pros | Cons | Verdict |
|---|---|---|---|
| **A. Full fork of Meshtastic firmware** | Fastest to a working device; huge community | Locked into LoRa architecture; can't do HaLow properly; app protocol is limited | ❌ Not for firmware |
| **B. New firmware, adopt Meshtastic *phone apps* + *protobuf* only** | Instant compat with existing users; keep our clean HaLow + RNS spine | Have to reimplement mesh + routing (but we want to anyway) | ✅ **This is the way** |
| **C. Reticulum + LXMF only, ignore Meshtastic** | Cleanest; most sovereign | Loses mass‑adoption on‑ramp; no existing phone apps besides Sideband | ❌ Alone, insufficient |
| **D. Ship both compat modes simultaneously** | Best of both worlds; users choose | Testing surface doubles | ✅ **Do this — B + C** |

**Final answer: fork Meshtastic's Android/iOS/Web apps and vendor their protobufs for compat; build our own firmware on RNS + HaLow.**

---

## 10. Business / Logistics / Governance

### 10.1 Licensing
- **Firmware: Apache‑2.0** (compatible with Meshtastic, LVGL, ESP‑IDF, Reticulum MIT).
- **Companion apps: Apache‑2.0** (matches Meshtastic).
- **Protocol specs: CC‑BY 4.0** (encourages third‑party implementations).
- **Trademarks reserved**: "SS‑SP", "Seekie‑Speakie", "Seekie", "Omega" — trademark grant to conformant implementations.

### 10.2 Governance
- Establish **SS‑SP Foundation** (nonprofit or open‑source foundation) to own trademarks + protocol spec.
- **Technical Steering Committee** with rotating seats: hardware vendor, security auditor, community reps.
- **Protocol RFC process** (like IETF drafts): every wire‑format change is an RFC.
- **Reference implementation** (this codebase) is the compliance oracle.
- **Certification program** for third‑party devices: "SS‑SP Compatible" mark.

### 10.3 Supply chain
- Multi‑source every critical part where possible (already done in your Alpha BOM — replaced NRND parts).
- Sign every firmware artifact; publish SBOM (SPDX) with each release.
- Reproducible builds (fixed toolchain container).
- Public build attestations (SLSA level 3 target).

### 10.4 Compliance roadmap
- **FCC Part 15 subpart Y** (US HaLow), **ETSI EN 300 220 / EN 303 204** (EU), **ARIB** (JP) — required for the Alpha to ship.
- **RoHS, REACH, WEEE** — EU market.
- **CE / UKCA** marking.
- **Optional**: FIPS 140‑3 for the crypto module (long‑horizon, for gov procurement).
- **Optional**: FirstNet / P25 interop bridges (not on device, on gateway).

### 10.5 Data + privacy
- **GDPR‑by‑design**: no PII collection by default; opt‑in telemetry.
- **CCPA** compliant.
- **HIPAA‑adjacent** posture for optional medical/EMS deployments (BAA‑ready gateway).

### 10.6 Manufacturing / logistics
- Factory provisioning tool programs: unique ID, RNS identity, region profile, first‑boot key.
- Golden‑image ATE test at end of line: RF (TX power, RX sensitivity), audio (mic + speaker loopback), display, buttons, GNSS cold‑fix, sensor sanity, battery charge test.
- Post‑ship: OTA + service portal + RMA workflow.

### 10.7 Distribution + community
- Open‑hardware documentation (KiCad + BOM published).
- Public forum + Matrix room + Reticulum LXMF address.
- Bug bounty (Immunefi or similar).
- Annual "SeekCon" community meetup.

---

## 11. Roadmap — Ordered, No Time Estimates

### Phase 0 — Foundations (do first, in this order)
1. Lock this plan; open issue tracker.
2. Bootstrap monorepo skeleton (Section 8 layout).
3. Choose LVGL 9.x, ESP‑IDF v5.x baseline. Freeze versions in `sdkconfig.defaults`.
4. Stand up CI (build for Lite, Alpha stub, sim). Add SLSA provenance.
5. Import Reticulum C port; get "hello world" packet loop on desktop sim.

### Phase 1 — SS‑SP Lite bring‑up (canary)
1. Board support: display, touch, mic, speaker, SD, LEDs, battery, buttons.
2. HAL implementation for Lite.
3. LVGL running with `ss_ui` layout engine on 320×480.
4. Reticulum over BLE + Wi‑Fi + HaLow (the D‑0013 dev fleet carries the HaLow wireless‑header module; LoRa on stock SX1262 units).
5. Chat app + Roster app end‑to‑end between two Lites.
6. Meshtastic‑compat serial interface → Meshtastic Android app connects.
7. Basic OTA over Wi‑Fi.
8. First public dev‑kit release (community can build + flash + chat).

### Phase 2 — SS‑SP Alpha 1.0 bring‑up
1. HAL for Alpha (P4 + C6 dual‑MCU coordination via ESP‑Hosted).
2. MM8108 HaLow driver integration → `HaLowInterface` for Reticulum.
3. SKY66423 PA control + regulatory profile system.
4. GNSS + BMM350 + Seekie app.
5. 12‑LED bezel driver + effects library.
6. Digital voice PTT over HaLow (Opus).
7. Map + Waypoint app with offline tiles.
8. Field tests: range, mesh stability, battery life.
9. Alpha production firmware v1.0.

### Phase 3 — Ecosystem & mass adoption
1. Companion apps GA (Android, iOS, Web, Desktop).
2. Plugin SDK (Lua + WASM) + plugin store.
3. Bridge services (MQTT, Matrix, Nostr, TAK).
4. Meshtastic ↔ SS‑SP gateway (both directions).
5. Protocol RFCs published, foundation established.
6. Third‑party device certification opens.

### Phase 4 — Omega
1. Next‑gen SoC selection (Linux SoM likely).
2. Dual‑radio (HaLow + LoRa hardware onboard).
3. Cellular + LEO fallback.
4. AI accelerator on‑device (Whisper + Piper full).
5. Enterprise fleet management console.

*Phase-4 baseline note (D-0020): items 1–3 are deferred beyond Omega v1.0 — the released v69 board is P4-based (not a Linux SoM) and carries HaLow only; dual-radio/cellular/LEO require an Omega board revision (`docs/dev/OMEGA_HW_BASELINE.md`).*

---

## 12. Immediate Next Actions (What to Do This Week)

1. **Approve this plan.**
2. Create the monorepo skeleton in `SS-SP-SOFTWARE/` per Section 8.
3. Order/confirm: 2× CrowPanel Advance 3.5" HMI kits, USB‑C cables, microSD, LiPo, LoRa expansion (if using).
4. Install ESP‑IDF v5.x + toolchain, verify a hello‑world build for ESP32‑S3.
5. Pull in: Reticulum (Python + community C port), LVGL 9, Meshtastic firmware (for reference only), Meshtastic Android/iOS (as fork base for companion).
6. Draft the `hal_*.h` API headers (no impl yet) — this is the contract for everything else.
7. Draft first two RFCs: **RFC‑0001 SS‑SP Wire Format**, **RFC‑0002 Aspect‑Ratio‑Agnostic Layout Descriptor**.

---

## 13. Open Questions to Confirm Before Coding

- [ ] Legal entity for the SS‑SP Foundation — form now or after Alpha ships?
- [ ] Trademark strategy — file "Seekie‑Speakie" and "SS‑SP" marks in US/EU/JP?
- [ ] LoRa expansion on Lite: which module (RAK4200 / Ebyte E22 / SX1262 breakout)?
- [ ] Regional launch order for Alpha: US → EU → JP, or parallel?
- [ ] Do we run our own OTA/bridge infra, or ship self‑hostable only?
- [ ] Companion app store presence — publish under org account now to reserve names?
- [ ] Certification target dates (FCC/CE) — start pre‑scan on Alpha EVT now?

---

## 14. Guiding Principles (nail these to the door)

1. **Universal by construction.** Every layer above HAL is board‑agnostic.
2. **Sovereign by default.** No cloud required to operate any feature.
3. **Interoperable at the edges.** Speak Meshtastic, MQTT, Matrix, Nostr, ATAK — but be pure inside.
4. **Cryptographic identity, always.** No unsigned peers, no plaintext by default.
5. **Fail safe, degrade gracefully.** Lost radio? Switch bearer. Lost bearer? Store‑and‑forward. Lost display? Bezel + haptics + TTS still work.
6. **One codebase, every form factor.** Aspect‑ratio is a runtime concern, not a fork.
7. **Open, but curated.** Apache‑2.0 code, RFC'd protocol, trademarked conformance.
8. **Mass‑adoption on‑ramp.** A Meshtastic user can pair an SS‑SP in 60 seconds and it just works.
9. **Future‑hardened.** New radios, new SoCs, new cryptography — pluggable at L1/L2/L4.
10. **No hidden state.** Every setting exportable, every log auditable, every key rotatable.
