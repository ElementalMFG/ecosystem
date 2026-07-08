<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# SS‑SP Lite — Authoritative Hardware & Software Reference
**Physical device:** Elecrow CrowPanel Advance 3.5" HMI ESP32 AI Display (Meshtastic edition)
**Board revs supported:** V1.2, V1.3, V1.4 (current)
**Purpose:** This document is the **single source of truth** for the SS‑SP Lite board bring-up. Everything above the HAL in the SS‑SP universal software stack targets this file's contract.

**Sources (verified):**
- [Elecrow product page (Meshtastic edition)](https://www.elecrow.com/crowpanel-advance-3-5-hmi-esp32-ai-display-for-meshtastic-320x240-ips-artificial-intelligent-screen.html)
- [Elecrow wiki — CrowPanel Advance 3.5" HMI](https://www.elecrow.com/pub/wiki/CrowPanel_Advance_3.5-HMI_ESP32_AI_Display.html)
- [Meshtastic docs — CrowPanel Advance Series](https://meshtastic.org/docs/hardware/devices/elecrow/crowpanel/)
- [Meshtastic firmware issue #7993 — board bring-up (closed/completed)](https://github.com/meshtastic/firmware/issues/7993)
- [Meshtastic firmware variant — `variants/esp32s3/elecrow_panel/`](https://github.com/meshtastic/firmware/tree/master/variants/esp32s3/elecrow_panel)
- [Elecrow-RD GitHub — official board repo (schematics, 3D, factory FW)](https://github.com/Elecrow-RD/CrowPanel-Advance-3.5-HMI-ESP32-S3-AI-Powered-IPS-Touch-Screen-480x320)
- [Elecrow-RD parent repo — CrowPanel Advance series](https://github.com/Elecrow-RD/CrowPanel-Advance-HMI-ESP32-AI-Display)
- [Rokland US reseller listing (confirms MCU/PSRAM/flash + SX1262)](https://store.rokland.com/products/crowpanel-advance-3-5-hmi-esp32-s3-ips-touch-screen-480x320-meshtastic-ready)
- [ESPHome feature request — SC7277/GT911 CrowPanel Advance driver notes](https://github.com/esphome/feature-requests/issues/3034)

---

## Table of contents

- [1. Device Identity](#1-device-identity)
- [2. Core Silicon (Bill of Materials — verified)](#2-core-silicon-bill-of-materials--verified)
- [3. Definitive GPIO Map (source‑verified)](#3-definitive-gpio-map-sourceverified)
- [4. Software Stack — Fitting the SS‑SP Universal Architecture](#4-software-stack--fitting-the-sssp-universal-architecture)
- [5. Bring‑up Sequence (Lite v1)](#5-bringup-sequence-lite-v1)
- [6. Legal / Licensing Notes Specific to Lite](#6-legal--licensing-notes-specific-to-lite)
- [7. Known Hardware Limitations (design around them)](#7-known-hardware-limitations-design-around-them)
- [8. Documents to Retrieve and Vendor Locally](#8-documents-to-retrieve-and-vendor-locally)
- [9. Delta from the Master Plan](#9-delta-from-the-master-plan)
- [10. What "Universal Yet Lite‑First" Means in Practice](#10-what-universal-yet-litefirst-means-in-practice)
- [11. Sign‑off Checklist Before Coding](#11-signoff-checklist-before-coding)
- [12. Appendix — Delta Record: External "Master Project Directive" vs Verified Hardware](#12-appendix--delta-record-external-master-project-directive-vs-verified-hardware)
- [13. One‑Line Summary](#13-oneline-summary)

## 1. Device Identity

| Field | Value |
|---|---|
| Product name | CrowPanel Advance 3.5" HMI ESP32 AI Display (Meshtastic) |
| SKU family | CrowPanel Advance HMI Series (2.4 / 2.8 / 3.5 / 4.3 / 5.0 / 7.0) |
| SS‑SP designation | **SS‑SP Lite v1** |
| PlatformIO env | `elecrow_panel` with `-D CROW_SELECT=1` (Meshtastic variant) |
| Meshtastic firmware bin | `firmware-elecrow-adv-35-tft-X.X.X.xxxxxxx.bin` |
| Current HW rev | V1.4 (FPC connector refinement) |
| Package includes | Board, USB‑C cable, DuPont wires, SMA antenna (LoRa) |
| Dimensions | 101.4 × 63.3 × 15.8 mm |
| Weight | 120 g |
| Power input | 5 V / 2 A (USB‑C or UART) |
| Operating temp | −20 °C to +70 °C |
| Storage temp | −30 °C to +80 °C |

**Enclosure:** plastic; not IP‑rated. Lite is a dev/canary; ruggedized housing is Alpha's job.

---

## 2. Core Silicon (Bill of Materials — verified)

| Ref | Function | Part |
|---|---|---|
| **U1** | Main MCU | **Espressif ESP32‑S3‑WROOM‑1‑N16R8** (Xtensa LX7 dual‑core @ 240 MHz, 512 KB SRAM, 384 KB ROM, 8 MB PSRAM, 16 MB flash, Wi‑Fi 4 b/g/n 2.4 GHz, BLE 5.0) |
| **U2** | IO expander / peripheral manager | **STC micro STC8H1K28** (8‑bit MCU acting as: backlight PWM 0–244, buzzer on/off, touch reset seq, audio amp power gating) |
| **U3** | Display driver | **Sitronix ILI9488** (480×320, 16‑bit color, 4‑wire SPI) |
| **U4** | Touch controller | **Goodix GT911** (capacitive, I²C, addr **0x5D**, INT + RST) |
| **U5** | LoRa transceiver (Meshtastic edition) | **Semtech SX1262** (sub‑GHz LoRa, TCXO 3.3 V, U.FL antenna) |
| **U6** | RTC | Onboard RTC + coin cell backup (I²C shared with touch bus) |
| Codec/Amp | Audio path | I²S digital mic + Class‑D speaker amp (V1.2+ optimized, GPIO21 mute) |
| Buzzer | Piezo | Onboard, GPIO8 |
| Charger | Battery mgmt | LiPo charger (TP4056‑class) with **PWR** and **CHG** status LEDs |
| Storage | External | microSD (SPI mode; shares bus with LoRa on some routings) |
| USB | USB‑C | Native USB‑Serial‑JTAG via ESP32‑S3 |

**Wireless header** on the board also accepts (as alternatives to SX1262):
- Espressif ESP32‑H2 module (Thread/Zigbee/802.15.4 + BLE)
- Espressif ESP32‑C6 (Wi‑Fi 6 + BLE 5.3 + 802.15.4)
- nRF24L01 (2.4 GHz)
This makes the same board a **multi‑radio experimentation platform** — SS‑SP Lite's radio interface must be abstracted to accommodate any of these.

---

## 3. Definitive GPIO Map (source‑verified)

Values are the **union** of the two authoritative sources: the Elecrow wiki pinout and Meshtastic's `variants/esp32s3/elecrow_panel/variant.h` + `platformio.ini` for `CROW_SELECT=1` (the 3.5" small‑display profile). Where the two disagree we note both.

### 3.1 Display (ILI9488, SPI)
| Signal | GPIO | Notes |
|---|---|---|
| SCLK | **42** | Meshtastic variant |
| MOSI | **39** | Meshtastic variant |
| DC | **41** | Meshtastic variant |
| CS | **40** | Meshtastic variant |
| Backlight EN/PWM | **38** | PWM via STC8H1K28 in factory FW; direct GPIO usable |
| Reset | driven by STC8H1K28 | Not on ESP32 GPIO in current rev |

*(Wiki also lists MOSI=IO6, MISO=IO4, CLK=IO5 for an "LCD (SPI)" line — this is the **legacy CrowPanel 3.5" (non‑Advance)** pinout. Do **not** use for the Advance edition; use the Meshtastic‑verified pins above.)*

### 3.2 Capacitive touch (GT911, I²C)
| Signal | GPIO |
|---|---|
| SDA | **15** |
| SCL | **16** |
| INT | **47** |
| RST | **48** |
| I²C addr | **0x5D** |

### 3.3 microSD card (SPI)
| Signal | GPIO |
|---|---|
| MOSI | **6** |
| MISO | **4** |
| SCK | **5** |
| CS | **7** |
| Mode | Soft‑SPI (`-D SDCARD_USE_SOFT_SPI` in Meshtastic build) |

### 3.4 LoRa SX1262 (Meshtastic edition) — CROW_SELECT=1 pinout
| Signal | GPIO |
|---|---|
| CS  | **0** |
| SCK | **10** |
| MISO | **9** |
| MOSI | **3** |
| RESET | **2** |
| DIO1 | **1** |
| DIO2 (RF switch) | **46** |
| TCXO voltage | 3.3 V |

**CRITICAL:** The SX1262 SPI (10/9/3) is **separate** from the SD SPI (5/4/6). Good — no bus contention at the SPI layer. The mode‑select MUX still gates some signals; see §3.9.

### 3.5 Audio — I²S microphone (input)
| Signal | GPIO | Notes |
|---|---|---|
| MIC_CLK (BCLK) | **9** | ⚠️ Overlaps LoRa MISO — mic and LoRa are **mutually exclusive** on shared pins; mode‑selected |
| MIC_WS (LRC)  | **3** | ⚠️ Overlaps LoRa MOSI |
| MIC_SD (DIN)  | **10** | ⚠️ Overlaps LoRa SCK |

### 3.6 Audio — I²S speaker/amp (output)
| Signal | GPIO |
|---|---|
| I2S_LRCLK | **11** |
| I2S_BCLK  | **13** |
| I2S_SDIN (DOUT to amp) | **12** |
| Amp mute (V1.2+) | **21** |

### 3.7 Buzzer
| Signal | GPIO |
|---|---|
| Buzzer PWM | **8** |

### 3.8 Serial (UART)
The console runs on the ESP32‑S3 **native USB‑Serial‑JTAG** (USB‑C), so both UART peripherals are free for expansion modules.

| Port | RX | TX | Header label | Purpose (SS‑SP assignment) |
|---|---|---|---|---|
| UART1 | **18** | **17** | UART‑IN | **GNSS module (BN‑880)** — NMEA‑0183 @ 9600 baud, DMA ring buffer |
| UART2 | **44** | **43** | UART‑OUT | **Mesh coprocessor (ESP32‑C6/H2)** — framed link @ 115200, DMA ring buffer |

*Pins 44/43 are the chip's default UART0 pins; SS‑SP maps the UART2 peripheral onto them via the GPIO matrix and leaves UART0 unused. Boot‑ROM log still appears on 43 at reset — the coprocessor link protocol must tolerate/discard this preamble.*

### 3.9 Mode‑select MUX
| Signal | GPIO | Function |
|---|---|---|
| MODE_SEL | **45** | **HIGH** = enable **I²S microphone** path; **LOW** = enable **wireless module** path (SX1262 / H2 / C6 / nRF24) |

**Consequence:** on the current HW rev you **cannot use LoRa and the onboard mic simultaneously** — they share GPIO 3/9/10. The SS‑SP Lite firmware must:
1. Expose a runtime mode switch (`RADIO_MODE` vs `MIC_MODE`) via `hal_muxctl.h`.
2. Warn if an app tries to use both.
3. In "PTT‑over‑LoRa" workflows: sample voice → switch mux to RADIO → transmit; then switch back.
4. Alpha does not have this limitation (dedicated mic + HaLow radio).

### 3.10 Onboard LEDs / status
| LED | Meaning |
|---|---|
| **PWR** | Power good (hardware, no GPIO) |
| **CHG** | Battery charging (hardware, no GPIO) |

No user‑controllable status LED on Lite. Bezel effects on Lite are done **on-screen** (a virtual LED ring widget in `ss_ui`) until an external NeoPixel ring is added via I²C/UART header.

### 3.11 Buttons
| Button | Function | GPIO |
|---|---|---|
| **BOOT** | ESP32‑S3 boot select | 0 (shared with LoRa CS — held only at reset) |
| **RESET** | Hard reset | EN (not a GPIO) |

**No user‑facing UI buttons** exist as GPIOs on the Advance 3.5. All UI input is capacitive touch. This has real consequences for glove/wet operation — the SS‑SP Lite firmware must support:
- On‑screen "big‑touch" mode.
- USB‑HID keyboard/joystick as accessibility input.
- BLE HID input from a paired phone/pendant.
- Voice command as primary input (STT wake word).

### 3.12 Battery / power
| Signal | GPIO |
|---|---|
| Battery voltage sense | **not exposed** on this HW rev (Meshtastic issue #7993 confirms) |
| Charger status | LEDs only (PWR/CHG) |
| Power input | 5 V USB‑C or 5 V via UART header |
| Battery | 3.7 V LiPo via JST‑PH (user supplied) |

**Consequence:** software cannot report SoC%. `hal_fuelgauge.c` on Lite returns `UNKNOWN` with a "battery telemetry unavailable — Lite hardware limitation" flag. Alpha ships MAX17048 and provides real SoC.

### 3.13 Expansion I/O
- **USB‑C**: native USB (D+/D‑ on ESP32‑S3), used for flash + serial + optional USB‑HID/MSC.
- **UART header**: 3.3 V, RX/TX/GND/5V — for LoRa/GPS/aux modules.
- **I²C Grove**: SDA/SCL shared with touch bus (0x5D reserved).
- **Speaker JST** and **Mic JST** (V1.2+ audio path).
- **Battery JST‑PH**.

### 3.14 Reserved / do‑not‑touch pins
- USB D+/D‑ (internal to package).
- Flash/PSRAM SPI (internal on WROOM‑1).
- BOOT strap pin 0 during reset.
- STC8H1K28 comms lines (backlight/buzzer/audio‑amp EN) — the STC handles these; the ESP32 controls the STC over UART/I²C in factory FW.

### 3.15 Optional expansion modules — the "fully‑loaded Lite" configuration
The SS‑SP Lite blueprint assumes **all features present** (design supposition); each module is nonetheless independently optional and gated by a Kconfig switch + capability flag, so absence never breaks the build or the apps.

| Module | Attach point | Kconfig | Capability flags added |
|---|---|---|---|
| **HaLow radio** (Elecrow HaLow module, sub‑GHz antenna) | Wireless header (replaces SX1262; same SPI 0/10/9/3, RST 2, IRQ 1; mux 45 = LOW) | `CONFIG_SS_LITE_MOD_HALOW` | `SS_CAP_RADIO_HALOW` (drops `SS_CAP_RADIO_LORA`) |
| **BN‑880 GNSS + compass** (u‑blox M8N class + HMC5883L) | GNSS → UART1 (18/17) @ 9600; compass → I²C0 (15/16) addr **0x1E** | `CONFIG_SS_LITE_MOD_GNSS_BN880` | `SS_CAP_GNSS_L1` + `SS_CAP_MAGNETOMETER` |
| **Mesh coprocessor** (ESP32‑C6 or ‑H2 module) | UART2 (44/43) @ 115200, framed link | `CONFIG_SS_LITE_MOD_COPROC_C6` / `_H2` | C6: `SS_CAP_RADIO_WIFI6`; H2: 802.15.4 only |
| **6‑axis IMU** (MPU‑6050/ICM‑426xx class) | I²C Grove (I²C0, 15/16) addr 0x68 | `CONFIG_SS_LITE_MOD_IMU` | `SS_CAP_IMU` |

**I²C0 bus roster (400 kHz, pins 15/16):** GT911 touch @ 0x5D · RTC (onboard) · HMC5883L @ 0x1E (module) · IMU @ 0x68 (module) · ATECC608 (optional). No address collisions.

**Compass note:** with `SS_CAP_MAGNETOMETER` + `SS_CAP_IMU` both present, `ss_compass` runs tilt‑compensated heading (mag + accel pitch/roll). Magnetometer alone = flat‑hold heading with a UI "hold level" hint. Neither = phone‑fed heading over BLE (base Lite behaviour).

**Dev-fleet configuration (D-0013):** the project holds **2× CrowPanel Advance 3.5″ dev units, both fitted with the HaLow wireless‑header module**; ESP32‑C6 mesh‑coprocessor modules and GNSS + 3‑axis compass modules are staged for attachment per the table above (exact GNSS/compass part numbers confirmed at attachment — any deviation updates this table and the pin map first). Once the C6 link and GNSS/compass paths validate on these units, the fully‑loaded configuration is declared the **locked base Lite v1 hardware spec** (`governance/decisions.md` D-0013). These two units are the primary dev/test/prototype devices for the whole portfolio; Lite is the first shipping product, with Alpha/Omega in engineering.

---

## 4. Software Stack — Fitting the SS‑SP Universal Architecture

### 4.1 Base
- **Framework:** ESP‑IDF v5.3+ (Arduino component optional; we ship pure IDF).
- **Board file:** `boards/lite/board_config.h` — sets `CROW_SELECT=1`, all pin defs above.
- **SDK config:** PSRAM enabled (Octal, 8 MB), flash 16 MB, USB‑Serial‑JTAG console, secure boot v2 (production), flash encryption (production).
- **Toolchain:** pinned via `.tool-versions` and a Docker CI image for reproducible builds.

### 4.2 Drivers we vendor
| Driver | Source | License |
|---|---|---|
| ILI9488 SPI panel | LovyanGFX (or LVGL's `esp_lcd_ili9488`) | FreeBSD / MIT |
| GT911 touch | `esp_lcd_touch_gt911` (Espressif component) | Apache‑2.0 |
| SX1262 LoRa | RadioLib (Arduino) or Semtech LoRaMac‑node C (IDF) | MIT / BSD |
| microSD | `sdmmc` IDF component (SPI mode) | Apache‑2.0 |
| I²S audio | `esp_audio` / `es_i2s` IDF | Apache‑2.0 |
| LVGL | LVGL 9.x | MIT |
| Reticulum C port | community `rns‑c` (extended by us) | MIT |
| Meshtastic protobufs | vendored from meshtastic/protobufs | GPL‑3.0 (⚠️ see §6) |
| Opus codec | Xiph opus 1.5 | BSD |
| Codec2 (LoRa voice) | codec2 1.2 | LGPL‑2.1 |
| STT | Sherpa‑onnx (whisper‑tiny int8) or Vosk‑lite | Apache‑2.0 |
| TTS | Piper (int8) or on‑device formant synth | MIT |

### 4.3 HAL implementation map (Lite)
```
hal_display.c    → LovyanGFX(ILI9488) on SPI2, pins 42/39/41/40, BL 38
hal_touch.c      → GT911 on I²C0 (15/16), INT 47, RST 48, addr 0x5D
hal_storage.c    → sdmmc SPI, MOSI 6, MISO 4, SCK 5, CS 7 (soft-SPI)
hal_audio_in.c   → I²S1 RX; BCLK 9, WS 3, DIN 10; mux GPIO 45 must be HIGH
hal_audio_out.c  → I²S0 TX; BCLK 13, WS 11, DOUT 12; mute GPIO 21
hal_haptic.c     → buzzer PWM GPIO 8 (single-channel piezo library)
hal_led.c        → virtual 12-LED ring drawn by ss_ui (no physical WS2812 on Lite)
hal_button.c     → touch-only + BLE HID pendant + USB HID
hal_radio_lora.c → RadioLib SX1262; CS 0, SCK 10, MISO 9, MOSI 3, RST 2, DIO1 1,
                   DIO2 46 (RF switch); mux GPIO 45 must be LOW
hal_radio_wifi.c → esp_wifi native (STA + softAP + ESP‑NOW as a bearer option)
hal_radio_ble.c  → NimBLE stack; GATT service + BLE HID for pendant input
hal_gnss.c       → BN-880 on UART1 (18/17) @9600, DMA ring buffer, NMEA-0183 parser
                   (module optional; fallback: phone GNSS over BLE)
hal_imu.c        → optional HMC5883L mag (I2C0 @0x1E) + optional 6-axis IMU (I2C0
                   @0x68) for tilt-compensated compass; fallback: phone heading
hal_coproc.c     → ESP32-C6/H2 mesh coprocessor on UART2 (44/43) @115200, framed
                   link with CRC + boot-preamble discard (module optional)
hal_pmic.c       → NONE (charger is hardwired); expose charging state via PWR/CHG hint
hal_fuelgauge.c  → returns UNKNOWN + capability=false
hal_muxctl.c     → GPIO 45 MODE_SEL arbiter; enforces radio-vs-mic exclusivity
hal_backlight.c  → GPIO 38 PWM (LEDC)
hal_secure.c     → eFuse + optional external ATECC608 on I²C header
hal_power.c      → light/deep sleep + wake sources: touch INT GPIO 47 (light-only,
                   not RTC-capable), LoRa DIO1 GPIO 1 (light+deep), RTC timer
                   (ss_power_wake_timer_set, S-03-030)
hal_time.c       → RTC + monotonic; GNSS PPS on Alpha only
```

### 4.4 The mux — enforced software contract
```c
// hal_muxctl.h
typedef enum { SS_MUX_RADIO, SS_MUX_MIC } ss_mux_mode_t;

// Blocks until safe. Fails if another owner already holds the mux.
esp_err_t ss_mux_acquire(ss_mux_mode_t mode, TickType_t timeout, ss_mux_owner_t owner);
esp_err_t ss_mux_release(ss_mux_owner_t owner);
```
The Voice PTT app must:
1. `ss_mux_acquire(SS_MUX_MIC, ...)` → capture N ms of Opus/Codec2.
2. `ss_mux_release()`.
3. `ss_mux_acquire(SS_MUX_RADIO, ...)` → transmit frames.
4. `ss_mux_release()`.

Full‑duplex voice is **not possible on Lite**. This is a hardware truth. Alpha is full‑duplex. The UI shows a "Half‑duplex (Lite)" badge in the voice app.

### 4.5 Transports available on Lite
| Bearer | Status | Purpose |
|---|---|---|
| **LoRa (SX1262)** | ✅ primary long-range | Meshtastic‑compat + Reticulum LoRa interface |
| **Wi‑Fi STA/AP** | ✅ | Internet backhaul, ESP‑NOW mesh option, RNS TCP interface |
| **BLE 5** | ✅ | Phone tether, HID pendant, RNS BLE interface |
| **USB‑CDC** | ✅ | Serial mesh (RNode compat), debug console, RNS TCPClient‑style |
| **ESP‑NOW** | ✅ | 2.4 GHz mesh **without** an AP (short‑range Reticulum bearer, ~200 m LOS) |
| **HaLow** | ⚙️ module option | Wireless‑header HaLow module (replaces SX1262, §3.15); native on Alpha |
| **802.15.4 / Wi‑Fi 6** | ⚙️ module option | Via ESP32‑C6/H2 mesh coprocessor on UART2 (§3.15) |

Base Lite runs **5 transports** in parallel; fully‑loaded Lite runs up to 6 — perfect for validating the transport‑agnostic Reticulum architecture before Alpha bring-up. The wireless header carries exactly one long‑range radio (LoRa **or** HaLow); `ss_link` treats either as just another bearer.

### 4.6 UI layout for Lite (definitive)
- Physical: **480×320 landscape** by default (native panel long axis horizontal).
- SS‑SP Lite orientation choices, selectable at boot:
  - **Landscape** (default): 480 wide × 320 tall.
  - **Portrait** (recommended for a pager form factor when handheld vertically): 320 wide × 480 tall — the same UI, layouts rotated by `ss_ui`.
- Safe area insets: `{top:0, right:0, bottom:0, left:0}` (no bezel keep-out on this SKU).
- Bezel LED count: **0** (`ss_ui` draws a virtual ring around the primary tile).
- Input: capacitive touch + optional BLE HID + optional voice.

### 4.7 What Lite gets vs Alpha (feature parity matrix)

| Feature | Lite | Alpha |
|---|---|---|
| Text chat (LXMF + Meshtastic compat) | ✅ | ✅ |
| Voice PTT (half‑duplex, Codec2) | ✅ over LoRa | ✅ over HaLow (Opus) |
| Full‑duplex voice | ❌ | ✅ |
| Long‑range mesh | LoRa (km‑scale) | HaLow 1 W (16 km ground / 100+ km LoS) |
| Bandwidth | ~5 kbps LoRa | 43 Mbps HaLow |
| Reticulum identity | ✅ | ✅ |
| Meshtastic phone app compat | ✅ (native) | ✅ (native) |
| Seekie compass | ⚠️ Requires phone heading over BLE | ✅ BMM350 native |
| Dual‑band GNSS | ⚠️ External UART module or phone GNSS | ✅ MIA‑M10Q L1/L5 |
| Battery SoC% | ❌ hardware limitation | ✅ MAX17048 |
| IP68 | ❌ | ✅ |
| Haptic LRA | ❌ (buzzer only) | ✅ DRV2625 + 1027 LRA |
| Bezel LED ring | virtual on‑screen | 12× SK6805 physical |
| Simultaneous mic + long‑range TX | ❌ (mux constraint) | ✅ |
| STT/TTS | on‑device (tiny) + phone‑assisted | on‑device (larger models) |
| ESP‑NOW short‑range mesh | ✅ | ✅ |
| BLE tether | ✅ BLE 5 | ✅ BLE 5.4 (via C6) |
| Wi‑Fi 6 | ❌ (Wi‑Fi 4) | ✅ (via C6) |

---

## 5. Bring‑up Sequence (Lite v1)

Ordered. Each step gates the next.

1. **Toolchain + repo skeleton**: ESP‑IDF v5.3, monorepo layout from Master Plan §8, `boards/lite/` created with `board_config.h` matching §3.
2. **Blink + serial**: prove build, flash, and USB‑Serial‑JTAG console. LED‑equivalent = backlight PWM sweep on GPIO 38.
3. **PSRAM + heap**: verify 8 MB PSRAM enumerated; run heap stress; confirm no lockups.
4. **Display bring-up**: LovyanGFX ILI9488 on SPI2, pins 42/39/41/40, BL 38. Show test pattern.
5. **Touch bring-up**: GT911 on I²C 15/16, INT 47, RST 48, addr 0x5D. Show touch coordinates on‑screen.
6. **LVGL 9.x integration**: 30 FPS target with PSRAM framebuffer. Ship `ss_ui` layout engine skeleton.
7. **SD card**: sdmmc SPI soft mode on 6/4/5/7; mount FAT; verify read/write.
8. **Audio out**: I²S TX on 13/11/12; play a 1 kHz tone; then a WAV from SD.
9. **Audio in**: `ss_mux_acquire(MIC)`; capture 3 s at 16 kHz; store to SD; play back to verify path.
10. **Buzzer haptic**: GPIO 8 PWM patterns library (ack, error, incoming, SOS).
11. **Wi‑Fi**: STA join + softAP; ping test; ESP‑NOW echo.
12. **BLE**: NimBLE GATT server; connect from phone; expose device info.
13. **LoRa SX1262**: `ss_mux_acquire(RADIO)`; RadioLib TX/RX at 869 MHz (EU) / 915 MHz (US); loopback with a second Lite.
14. **Meshtastic serial API**: expose `TextMessage` and `NodeInfo` over UART/BLE so an unmodified Meshtastic Android app pairs and messages.
15. **Reticulum C port**: bring up RNS on desktop sim first, then port to Lite; add BLE, Wi‑Fi, LoRa interfaces.
16. **LXMF chat app**: two‑device chat via RNS over LoRa; then multi‑hop through a third node.
17. **`ss_ui` real apps**: Chat, Roster, Settings, Signal (radio graph), Compass (phone‑fed heading), Voice PTT (mux‑arbitrated).
18. **OTA over Wi‑Fi**: signed A/B partition update; rollback on boot failure.
19. **Secure boot + flash encryption**: enable on production units; verify golden‑image flash.
20. **Factory tool**: provisioning app that burns unique RNS identity, region profile, serial, first‑boot key.
21. **Field pilot**: 10‑unit field test — range, mesh stability over 24 h, battery life.

Ship Lite v1.0 firmware. Everything above the HAL is now proven and portable to Alpha.

---

## 6. Legal / Licensing Notes Specific to Lite

- **Meshtastic firmware is GPL‑3.0.** If we vendor any Meshtastic **firmware** code (not just `.proto` files), our derivative is GPL‑3.0. **We do not vendor firmware** — we only vendor the `protobufs` repo (which is Apache‑2.0 / MIT‑friendly) and re‑implement the wire layer clean. Companion apps we fork are GPL‑3.0 → we accept GPL‑3.0 for the apps or write clean‑room clients.
- **LovyanGFX**: FreeBSD — fine for Apache‑2.0 firmware.
- **Reticulum (RNS)**: MIT.
- **LVGL**: MIT.
- **ESP‑IDF**: Apache‑2.0.
- **Radio compliance**: SX1262 duty cycle + max EIRP is region‑locked. First‑boot region profile is mandatory; firmware refuses to key the PA until region is set and antenna‑gain declared.

---

## 7. Known Hardware Limitations (design around them)

1. **No battery voltage sense** → SoC unknown; UI shows "External telemetry required" hint.
2. **Radio/mic mux on GPIO 45** → half‑duplex voice only; enforced by `hal_muxctl`.
3. **Wi‑Fi 4 only natively** → no Wi‑Fi 6/6E on the S3 itself; Wi‑Fi 6 arrives via the C6 coprocessor module and HaLow via the wireless‑header module (§3.15, D-0013 dev fleet is HaLow‑fitted). *Native/integrated* HaLow requires Alpha.
4. **Single capacitive touch** → no multi‑touch gestures; UI must be single‑touch friendly.
5. **No physical UI buttons** → touch + BLE HID pendant + voice; must ship a "big‑touch" mode for gloved use.
6. **No IP rating** → Lite is dev/prosumer only; field deployment requires Alpha or a user‑built enclosure.
7. **Non‑rugged plastic housing** → no drop/vibration certification.
8. **Two shared SPI buses** (display 42/39/41/40, aux 5/4/6 for SD+high‑speed) — plan bus contention when adding accessories on the header.
9. **Speaker + amp** is mono; localization/spatialization not possible on Lite.
10. **microSD is soft‑SPI in factory FW** — slower than SDMMC; acceptable for logs and TTS voice packs, not for high‑bitrate video capture.

---

## 8. Documents to Retrieve and Vendor Locally

Do this once and store in `docs/vendor/`:
- Elecrow-RD official repo master zip (schematics `Eagle_SCH&PCB/`, datasheets `Datasheet/`, factory FW `factory_firmware/`, factory sourcecode `factory_sourcecode/`, examples `example/`, 3D files, LVGL library, Squareline Studio project). See [Elecrow-RD/CrowPanel-Advance-3.5-HMI-ESP32-S3-AI-Powered-IPS-Touch-Screen-480x320](https://github.com/Elecrow-RD/CrowPanel-Advance-3.5-HMI-ESP32-S3-AI-Powered-IPS-Touch-Screen-480x320).
- ILI9488 datasheet (Sitronix).
- GT911 datasheet (Goodix) + register map.
- SX1262 datasheet (Semtech) + LoRaWAN regional parameters spec (RP002).
- ESP32‑S3 technical reference manual + WROOM‑1‑N16R8 datasheet.
- STC8H1K28 datasheet (STC micro) — required to understand backlight/buzzer/audio‑amp control.
- Meshtastic `variant.h` and `platformio.ini` for `variants/esp32s3/elecrow_panel/` — golden source for pin definitions.

---

## 9. Delta from the Master Plan

The Master Plan (`00_MASTER_SOFTWARE_PLAN.md`) is updated as follows for Lite specifics:

- Base OS: **ESP‑IDF v5.3+ (FreeRTOS)** — confirmed.
- Display driver: **ILI9488** (not the earlier assumed CST-family driver).
- Touch: **GT911** at 0x5D — confirmed.
- Long‑range radio on Lite: **LoRa SX1262 or the HaLow wireless‑header module** — exactly one at a time on the shared header (§3.15); the D-0013 dev fleet runs HaLow‑fitted. Native/integrated HaLow only on Alpha. Reticulum treats either as a bearer via `hal_radio_*`.
- Battery telemetry: **not available on Lite** — plan surfaces this correctly.
- Mic + LoRa mutex on GPIO 45 → adds `hal_muxctl` requirement to all L2/L3 code.
- Bezel: virtualized (drawn on screen) until an accessory NeoPixel ring is added.
- Buttons: none physical; input via touch/BLE‑HID/voice. `ss_ui` must be usable with **one** input modality alone.

---

## 10. What "Universal Yet Lite‑First" Means in Practice

Every design choice here must satisfy **both** of these tests:

1. **Lite Test:** does it run on this exact 480×320 ILI9488 + GT911 + SX1262 + I²S audio + no battery sense + mic/radio mux board?
2. **Universal Test:** does it work unchanged when the HAL swaps to Alpha (P4 + HaLow + BMM350 + MAX17048 + 12‑LED ring + IP68), a round 240×240 display, an e‑ink 128×296 pager, or a Linux SoM headless gateway?

If a design choice fails either test, it must be redesigned or gated behind a capability flag in `hal_caps.h`. The capability catalog:

```
SS_CAP_DISPLAY_TFT, SS_CAP_DISPLAY_ROUND, SS_CAP_DISPLAY_EINK, SS_CAP_DISPLAY_NONE
SS_CAP_TOUCH_CAP, SS_CAP_TOUCH_MULTI, SS_CAP_TOUCH_NONE
SS_CAP_INPUT_BUTTONS, SS_CAP_INPUT_ROTARY, SS_CAP_INPUT_VOICE, SS_CAP_INPUT_HID
SS_CAP_RADIO_LORA, SS_CAP_RADIO_HALOW, SS_CAP_RADIO_WIFI4, SS_CAP_RADIO_WIFI6,
    SS_CAP_RADIO_BLE, SS_CAP_RADIO_ESPNOW, SS_CAP_RADIO_SAT
SS_CAP_AUDIO_IN, SS_CAP_AUDIO_OUT, SS_CAP_AUDIO_FULLDUPLEX, SS_CAP_AUDIO_STT, SS_CAP_AUDIO_TTS
SS_CAP_GNSS_L1, SS_CAP_GNSS_L1L5, SS_CAP_GNSS_EXTERNAL, SS_CAP_GNSS_NONE
SS_CAP_IMU_MAG, SS_CAP_IMU_ACCEL, SS_CAP_IMU_GYRO, SS_CAP_IMU_NONE
SS_CAP_HAPTIC_BUZZER, SS_CAP_HAPTIC_LRA, SS_CAP_HAPTIC_NONE
SS_CAP_LED_RING_PHYSICAL, SS_CAP_LED_RING_VIRTUAL
SS_CAP_FUELGAUGE, SS_CAP_BATT_SENSE_NONE
SS_CAP_SECURE_ELEMENT, SS_CAP_SECURE_BOOT, SS_CAP_FLASH_ENCRYPT
SS_CAP_IP_RATED, SS_CAP_RUGGEDIZED
```

Every app checks `ss_caps_has(SS_CAP_X)` and adapts. Nothing hardcoded.

---

## 11. Sign‑off Checklist Before Coding

- [ ] Confirm current HW rev in hand (V1.4 preferred).
- [ ] Order at least **2× Lite units** (mesh testing needs peer).
- [ ] Order SMA / U.FL antennas for correct region.
- [ ] Order LiPo batteries (~1000–3000 mAh, JST‑PH).
- [ ] Order 2× microSD cards (≥16 GB, industrial temp preferred).
- [ ] Confirm region (US/EU/JP/…) for SX1262 duty‑cycle profile.
- [ ] Register `elecrow_panel` variant reservation with Meshtastic if we plan to upstream any changes.
- [ ] Decide licensing posture for companion apps (fork Meshtastic clients → GPL‑3.0, or write clean‑room → Apache‑2.0).
- [ ] Reserve organization names on GitHub / GitLab / crates.io / npm.

---

## 12. Appendix — Delta Record: External "Master Project Directive" vs Verified Hardware

An externally supplied SS‑SP‑Lite directive (2026‑07) specified pin assignments that **do not match the schematic‑verified board**. This file remains authoritative. Recorded so nobody "fixes" the firmware back to the wrong pins:

| Directive claim | Verified reality (this doc) | Disposition |
|---|---|---|
| Display SPI CLK=36, MOSI=35, CS=37, DC=38, RST=39 | SCLK=**42**, MOSI=**39**, DC=**41**, CS=**40**, BL=**38**; panel reset via STC8H1K28 (no ESP32 GPIO) | **Rejected** — directive pins match a generic ESP32‑S3 template, not the CrowPanel Advance schematic; 38 is the backlight, 39 is MOSI |
| Touch/compass I²C SDA=4, SCL=5 @ 400 kHz | SDA=**15**, SCL=**16** @ 400 kHz; pins 4/5 belong to the **microSD** soft‑SPI (MISO=4, SCK=5) | **Rejected** — using 4/5 as I²C would break the SD card |
| GPS on "UART‑IN", mesh coprocessor on "UART‑OUT" | Honoured: UART‑IN = UART1 (18/17), UART‑OUT = UART2 (44/43) | **Adopted** (§3.8) |
| 40 MHz display SPI, async multi‑page UI | Matches `SS_LCD_SPI_FREQ_HZ 40000000` + LVGL 9 plan | **Adopted** |
| Onboard 6‑axis IMU assumed | No IMU on the stock board | **Adopted as optional module** (I²C Grove, §3.15) |
| Low‑battery power‑down via battery ADC | **No battery‑voltage sense exists** on this HW rev (§3.12) | **Amended** — power‑down policy uses charger‑state hint + external fuel‑gauge module if fitted; ADC path is a no‑op on Lite |
| HaLow module + sub‑GHz antenna onboard | Meshtastic edition ships SX1262; HaLow is a wireless‑header module option | **Adopted as module option** (§3.15) |

---

## 13. One‑Line Summary

**SS‑SP Lite = a 480×320 ILI9488 IPS + GT911 touch + ESP32‑S3‑WROOM‑1‑N16R8 (8 MB PSRAM / 16 MB flash) + SX1262 LoRa + I²S mic/speaker + microSD + Wi‑Fi 4 + BLE 5 device, running the SS‑SP universal firmware whose HAL respects the GPIO 45 mic/radio mux, treats LoRa/Wi‑Fi/BLE/ESP‑NOW/USB‑CDC as parallel Reticulum bearers, ships a Meshtastic‑compat serial API for instant phone‑app pairing, and renders a virtual bezel + capability‑adaptive UI via `ss_ui` on LVGL 9 — proving the universal stack before Alpha (HaLow) bring‑up.**
