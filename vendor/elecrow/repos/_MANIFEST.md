<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# Elecrow hardware-reference: extended GitHub repo set

Supplementary snapshot of Elecrow-RD (GitHub user `Elecrow-RD`, id 24800915 — a
**user** account, not an org; the `orgs/Elecrow-RD` API 404s) source repos plus
one community pin-map reference, beyond the three per-board repos already held
under `../devices/` and `../modules/`. Assembled for the round-1 (D-0026) Elecrow
boards `elecrow5` (CrowPanel Advanced 5" ESP32-P4) and `elecrow35-s3` (CrowPanel
Advance 3.5" ESP32-S3) and the Wi-Fi HaLow (MM6108) bearer module.

All archives are `codeload.github.com/Elecrow-RD/<name>/zip/refs/heads/<branch>`
snapshots. Binaries are git-ignored (only this `*.md` is tracked); rehydrate by
re-running the recorded URLs. **Downloaded code was NEVER executed.**

## Already held elsewhere (NOT re-downloaded)
- `-CrowPanel-Advanced-5inch-ESP32-P4-HMI-AI-Display-800x480-IPS-Touch-Screen` → `../devices/crowpanel-5in-esp32p4/`
- `CrowPanel-Advance-3.5-HMI-ESP32-S3-AI-Powered-IPS-Touch-Screen-480x320` → `../devices/crowpanel-3p5in-esp32s3/`
- `Wireless-Module-for-Wi-Fi-HaLow` → `../modules/halow-mm6108/`

## Downloaded — Elecrow-RD (`elecrow-org/`)

| filename | repo / description | source URL | fetched | bytes | sha256 | license note |
|---|---|---|---|---|---|---|
| gt911_for_crowpanel.zip | GT911 capacitive-touch driver for CrowPanel (our boards use GT911) | https://codeload.github.com/Elecrow-RD/gt911_for_crowpanel/zip/refs/heads/main | 2026-07-18 | 5203 | 433e6f97ac1065de22fff469b4966dd55e537dcc36e15dfa7c8fb4406a9cd985 | © Elecrow-RD; no LICENSE file in archive |
| CrowPanel_for_ESPHome.zip | ESPHome YAML configs for CrowPanel displays | https://codeload.github.com/Elecrow-RD/CrowPanel_for_ESPHome/zip/refs/heads/main | 2026-07-18 | 5621 | 7334fa5c27758ffd00717de6c499f805245b24198bf751b97685a46e1239b2f6 | **MIT** (LICENSE: "MIT License, Copyright (c) 2024 Elecrow") |
| ESPHome_ESP32_Display_2-4and2-8.zip | ESPHome configs, CrowPanel 2.4/2.8 displays (Advance-S3 family peers) | https://codeload.github.com/Elecrow-RD/ESPHome_ESP32_Display_2-4and2-8/zip/refs/heads/main | 2026-07-18 | 261680 | 9c3f077ff8468cafefe8b257619bad291b8030726773d6af93ec4cfb80080302 | © Elecrow-RD; no LICENSE file in archive |
| ESPHome_ESP32_Terminal_SPI.zip | ESPHome configs, CrowPanel ESP32 Terminal (SPI, camera) | https://codeload.github.com/Elecrow-RD/ESPHome_ESP32_Terminal_SPI/zip/refs/heads/main | 2026-07-18 | 254255 | 5d07786ce68718e477a6a48d21067a709962c701131da17d737ff5ec6b4fc6a5 | © Elecrow-RD; no LICENSE file in archive |
| ESP32-LVGL-DESK-CLOCK.zip | LVGL desk-clock demo (LVGL UI reference) | https://codeload.github.com/Elecrow-RD/ESP32-LVGL-DESK-CLOCK/zip/refs/heads/master | 2026-07-18 | 1343038 | 5be2c0cb9d223b46ec336447c39296f2ffc70668d21b6c6a34fe27d7d75b8741 | © Elecrow-RD; no LICENSE file in archive |
| Whac-A-Mole-game-LVGL.zip | LVGL game demo (LVGL UI reference) | https://codeload.github.com/Elecrow-RD/Whac-A-Mole-game-LVGL/zip/refs/heads/main | 2026-07-18 | 2089285 | dfcff58f126d98f2098022e8a5a42f06999b37db7dbc970ff4c283542377e7d0 | © Elecrow-RD; no LICENSE file in archive |
| ESP32_Wi-Fi_HaLow_Module_with_2MP_Camera_32Mbps_High_Speed.zip | HaLow module + 2MP camera ESP-IDF/Arduino examples (camera web server, MQTT video stream) — directly relevant to HaLow bearer + camera path | https://codeload.github.com/Elecrow-RD/ESP32_Wi-Fi_HaLow_Module_with_2MP_Camera_32Mbps_High_Speed/zip/refs/heads/master | 2026-07-18 | 5428967 | 9be171fd339117ba4849ce6e3158b5a6c77b2aeb0b3635c836b902e21b5135f8 | © Elecrow-RD (no top LICENSE); bundles espressif esp32-camera/esp_jpeg (**Apache-2.0 / ESP-IDF**, permissive) |
| CrowPanel-Advance-HMI-Courses.zip | Shared Advance-series HMI course material (cross-board, incl. 3.5" S3) | https://codeload.github.com/Elecrow-RD/CrowPanel-Advance-HMI-Courses/zip/refs/heads/master | 2026-07-18 | 49215418 | 20017b818d5395651b6262cc74d48a8ed1c9c37b69da5501a4d4f8e6ef7724de | © Elecrow-RD; no LICENSE file in archive |
| CrowPanel-Advance-HMI-ESP32-AI-Display.zip | Shared Advance HMI ESP32-S3 AI-display demo code/BSP | https://codeload.github.com/Elecrow-RD/CrowPanel-Advance-HMI-ESP32-AI-Display/zip/refs/heads/master | 2026-07-18 | 102573417 | e80ed01643a28a6f4d627d76e72add7a2566268719af8bc3639d066a473634c0 | © Elecrow-RD; no top LICENSE (bundled libs carry own) |
| CrowPanel-Advance-hmi-esp32-IPS-AI-Display-project.zip | Advance HMI IPS AI-display project (AI-chat demo) | https://codeload.github.com/Elecrow-RD/CrowPanel-Advance-hmi-esp32-IPS-AI-Display-project/zip/refs/heads/master | 2026-07-18 | 111601110 | 5b66867d157b4aa0c1283a73b5a9c6107adbf736fda03bfe7d0cb8e47d8e532d | © Elecrow-RD (no top LICENSE); bundles ArduinoJson (**MIT**), LovyanGFX (**BSD/FreeBSD**) |
| All-in-one-Starter-Kit-for-ESP32-P4-with-Common-Board-design.zip | ESP32-P4 common-board peripheral examples/libraries (P4 driver reference for elecrow5) | https://codeload.github.com/Elecrow-RD/All-in-one-Starter-Kit-for-ESP32-P4-with-Common-Board-design/zip/refs/heads/master | 2026-07-18 | 212994559 | 803e1cf5820c5d47ea2122f2cd7458595c3fdec465810521c6c0b4694a2cfddf | © Elecrow-RD (no top LICENSE); bundles ESP32_Display_Panel/IO_Expander (Apache-2.0) and **Adafruit_NeoPixel (LGPL-3.0 — weak copyleft, flag)** |

All archives verified: `file` = "Zip archive data", `unzip -tqq` = OK.

## Downloaded — community (`community/`)

| filename | repo / description | source URL | fetched | bytes | sha256 | license note |
|---|---|---|---|---|---|---|
| meshtastic-elecrow-variant/variant.h | Meshtastic `variants/esp32s3/elecrow_panel` board header — pin map for CrowPanel Advance S3 (env `elecrow-adv-35-tft` = "Crowpanel Adv 3.5 TFT" = our elecrow35-s3; also 2.4/2.8/4.3/5.0/7.0) | https://raw.githubusercontent.com/meshtastic/firmware/master/variants/esp32s3/elecrow_panel/variant.h | 2026-07-18 | 2105 | 9c9b2ff52da4418cdaad569c4800785f9b7980aed31cd41eb83b1eba4581e0f5 | **GPL-3.0 (copyleft) — REFERENCE ONLY. Clean-room boundary per D-0002; MUST NOT be copied into our Apache-2.0 firmware.** |
| meshtastic-elecrow-variant/platformio.ini | Meshtastic elecrow_panel PlatformIO env/build defs (board=crowpanel, LovyanGFX, CROW_SELECT/DISPLAY_SIZE per board) | https://raw.githubusercontent.com/meshtastic/firmware/master/variants/esp32s3/elecrow_panel/platformio.ini | 2026-07-18 | 5348 | 8e0c6e6d7834bb536fe4eeb16d3813b93f8fe7f2ecf2e96ccc2ccb9b5db40aad | **GPL-3.0 (copyleft) — REFERENCE ONLY, clean-room per D-0002.** |
| meshtastic-elecrow-variant/pins_arduino.h | Meshtastic elecrow_panel Arduino pin defs | https://raw.githubusercontent.com/meshtastic/firmware/master/variants/esp32s3/elecrow_panel/pins_arduino.h | 2026-07-18 | 1508 | 5263ff99d8341bca9f2fbb4bfbce15233fc96039e4af1a4625757489b8e568ce | **GPL-3.0 (copyleft) — REFERENCE ONLY, clean-room per D-0002.** |

## Skipped / oversized / not-downloaded (with reason)

- **Per-board sibling repos, Advance ESP32-S3** (2.4 / 2.8 / 4.3 / 5 / 7 inch): `CrowPanel-Advance-{2.4,2.8,4.3,5,7}-HMI-ESP32-S3-...` — each 600–900 MB. Skipped: not our exact board (we hold the 3.5" S3), and the shared `CrowPanel-Advance-HMI-*` repos above already cover cross-board BSP/LVGL. Rehydrate individually if a sibling pin map is needed.
- **Per-board sibling repos, Advanced ESP32-P4** (7 / 9 / 10.1 inch): `CrowPanel-Advanced-{7,9,10.1}inch-ESP32-P4-...` — ~237–258 MB each. Skipped: not our board (we hold the 5" P4); P4 driver reference covered by the P4 common-board kit above.
- **`esphome-docs`** (~169 MB) — full mirror of ESPHome documentation site; general, not board-specific. Skipped.
- **`ESP32-RGB-SPI-Lesson-Code`** (~119 MB), **`AI_Camera_Development_Board_...ESP32`** (~207 MB), **`CrowVision`** (~47 MB) — generic ESP32 lesson/camera repos, not our exact boards. Skipped (rehydrate on demand).
- **`hotspot-manufacturers`** (~128 MB) — LoRaWAN hotspot vendor material, unrelated to our HaLow bearer. Skipped.
- **ThinkNode M1/M2/M3/M4/M5/M6/M7, G3** — Elecrow's own Meshtastic **LoRa** transceivers (nRF52840 / ESP32-S3); different product line, LoRa not our bearer. Skipped.
- **LoRa / LoRaWAN / nRFLR / LR1262 / LR1302 modules, Pico/CrowPi/starter-kit/RPi repos** — out of scope for the CrowPanel/HaLow reference set. Skipped.
- **`crowpanel-esp32s3-5-epaper`** (Meshtastic variant) — 5" e-paper CrowPanel, a different board than our IPS Advance units; not fetched (only `elecrow_panel` variant, which covers our 3.5" S3, was pulled).
- **Board schematics / mechanical drawings** — Elecrow does not publish standalone schematic/mechanical PDFs on the product/wiki pages. **Exception (2026-07-18): the 5″ ESP32-P4 board's official Eagle schematic IS bundled inside its example-repo zip** (`repo-crowpanel-5in-esp32p4.zip` → `Eagle_SCH&PCB/1.0/`, `.sch`/`.brd`/`.pdf` V1.0) — it is the BOM/netlist source for the D-0027 schematic-validated facts (see the crowpanel-5in-esp32p4 device manifest). The 3.5″ S3 board schematic remains UNAVAILABLE (no bundled schematic found).
