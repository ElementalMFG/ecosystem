<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# Espressif ecosystem reference — Elecrow round-1 boards (ESP32-P4 host + ESP32-C6 radio)

Board-support, driver, camera, and reference-design material from Espressif's GitHub
ecosystem, retained read-only to support bring-up of the Elecrow round-1 boards
(firmware board id `elecrow5` — 5-inch CrowPanel: ESP32-P4 + on-board ESP32-C6 over
SDIO, MIPI-DSI LCD, GT911 touch, optional MIPI-CSI camera). The canonical hardware
reference for this topology is the **ESP32-P4-Function-EV-Board** (P4 host + C6 radio),
so its schematics + board-support package are the core of this set (informs S-05-044
C6↔P4 bus wiring). SoC datasheets/TRMs + ESP-Hosted live in the sibling `../espressif/`.

These are third-party copyrighted vendor artifacts, git-ignored, kept locally for
internal engineering use only. Do not commit or redistribute the binaries; this
manifest (CC-BY-4.0) records source + checksum so the set is reproducible.

| filename | what it is | source URL | fetched | bytes | sha256 | note |
|---|---|---|---|---|---|---|
| esp32-p4-function-ev-board-schematics_v1.52.pdf | ESP32-P4-Function-EV-Board main schematic (6 pp) — canonical P4+C6 reference design | https://dl.espressif.com/dl/schematics/esp32-p4-function-ev-board-schematics_v1.52.pdf | 2026-07-18 | 817278 | 7a1d12a37e21db20fe7db8092f4428b87465873feab65b3cd90bc6fe7dd6673b | v1.5.2. THE canonical P4-host + on-board-C6-radio (SDIO) reference schematic → directly informs S-05-044 bus wiring. URL harvested from esp-dev-kits docs/en/…/user_guide.rst |
| esp32-p4-function-ev-board-lcd-subboard-schematics.pdf | LCD adapter sub-board schematic (MIPI-DSI) (2 pp) | https://dl.espressif.com/dl/schematics/esp32-p4-function-ev-board-lcd-subboard-schematics.pdf | 2026-07-18 | 328112 | cbb6a53dc20a68748784b5bf3548801710f925bd69dbdb75510a9a46d6d0007a | MIPI-DSI display adapter — relevant to CrowPanel MIPI-DSI LCD wiring |
| esp32-p4-function-ev-board-camera-subboard-schematics.pdf | Camera adapter sub-board schematic (MIPI-CSI) (1 p) | https://dl.espressif.com/dl/schematics/esp32-p4-function-ev-board-camera-subboard-schematics.pdf | 2026-07-18 | 332122 | dcb30637c379e9fde13dfadefe1b143902fd6ec53383e562ddbab33b271dbe37 | MIPI-CSI camera adapter — relevant to the P4 optional camera (OV5647) |
| p4-ev-board-userguide-rst/user_guide.rst | ESP32-P4-Function-EV-Board user guide source (v1.5.x) — J1 pin-header tables, block diagram, bus notes | https://github.com/espressif/esp-dev-kits (docs/en/esp32-p4-function-ev-board/user_guide.rst @ master ce50c11) | 2026-07-18 | 20964 | 163fa07ec18aabf85dc18cea320ce9401fd52e436ee7b8231c1dce106d21b4ad | Extracted from esp-dev-kits master zip (see size-skip note). Text ref for P4↔C6 + peripheral pinout |
| p4-ev-board-userguide-rst/user_guide_v1.4.rst | ESP32-P4-Function-EV-Board user guide source (v1.4) | https://github.com/espressif/esp-dev-kits (docs/en/esp32-p4-function-ev-board/user_guide_v1.4.rst @ master ce50c11) | 2026-07-18 | 20194 | 9624db709dcabe470977ecd4ed57b46d2e7fb37e649ab707597da8feee40ff6e | Prior board rev pinout, for cross-check |
| esp-bsp.zip | esp-bsp repo snapshot (22 board-support packages) | https://codeload.github.com/espressif/esp-bsp/zip/refs/heads/master | 2026-07-18 | 26680775 | 7333a2e2a6cbc2e3cb1f440d5d0f2cedb53883cf388fbb78dd5b6be4f081dfa2 | master @ 74e3cd238fdd81cda4fd03184c32f6776c41b7cd. Contains P4 board packages `bsp/esp32_p4_function_ev_board` (MIPI-DSI LCD 1024×600 / 1280×800, 2-lane @1Gbps, GT911 touch) and `bsp/esp32_p4_eye`. No CrowPanel package upstream (Elecrow board not in esp-bsp) — the P4-Function-EV-Board BSP is the closest reference |
| esp32-camera.zip | esp32-camera driver repo snapshot (DVP + MIPI-CSI) | https://codeload.github.com/espressif/esp32-camera/zip/refs/heads/master | 2026-07-18 | 353124 | 55a6e3a11dc1f3f08f48b917e8b7df26f9f677d66e7f9082952ce5f9a28fb3ad | master @ 202df95d7b1dc72e9303ad78f47b8dc9f339e6a1. Camera driver for the P4 optional camera (OV5647 in `../peripherals/`) |
| esp-iot-solution-components/ | Curated subset of esp-iot-solution `components/` (display, audio, touch, touch_ic, bus, i2c_bus, spi_bus) + top README | https://codeload.github.com/espressif/esp-iot-solution/zip/refs/heads/master | 2026-07-18 | 10538996 | (dir — see source-zip sha below) | Extracted subset only (see size-skip note). Provides MIPI-DSI LCD panel drivers (`components/display`), I2S/codec audio (`components/audio`), and touch stacks relevant to the CrowPanel |

## Size-based skips & extraction decisions

- **esp-dev-kits** (github.com/espressif/esp-dev-kits) master zip = **432 MB** (mostly
  example assets/ML models) → NOT retained. It does **not** bundle the schematic PDFs
  (only `docs/_static/espressif2.pdf`); schematics are hosted on `dl.espressif.com`.
  Action taken: downloaded the three schematic PDFs directly (rows above) and extracted
  the two P4-EV-board `user_guide*.rst` files (J1 pin tables) before deleting the zip.
  Full-repo source: `https://codeload.github.com/espressif/esp-dev-kits/zip/refs/heads/master` (master @ ce50c11e1cc09040b242de4bcc079b12cdb89fd4).
- **esp-iot-solution** master zip = **85 MB** (bloated by prebuilt `.a` libs, example
  media, ML models; e.g. usb/ alone ≈ 24 MB) → full zip NOT retained. Kept a **14 MB→10.5 MB**
  curated subset of the board-relevant `components/` dirs. Full-zip sha256 for
  reproducibility: `0432f50efc2088cda5811c16c34a8495bc1a64aee8f45235e93848eab378d468`
  (master @ 9971a4692b5c50fbe055db786a9bd6f541372b6e). Re-hydrate: download zip, verify
  sha, `unzip` the `components/{display,audio,touch,touch_ic,bus,i2c_bus,spi_bus}` paths.

## ESP-IDF / ESP32-P4 + C6-as-radio SoC support (not downloaded — pinned + version notes)

- **Minimum ESP-IDF: v5.3+** for ESP-Hosted (C6-as-Wi-Fi/BLE-radio over SDIO/SPI);
  reference boot logs in `../espressif/esp-hosted-mcu.zip` show **v5.4-dev / v5.5-dev**.
  ESP32-P4 as a target is supported from ESP-IDF v5.3 and stabilized in v5.4/v5.5.
  C6-as-radio requires the managed components `espressif/esp_wifi_remote` + `espressif/esp_hosted`
  (`idf.py set-target esp32p4`, `CONFIG_SLAVE_IDF_TARGET_ESP32C6=y`). Source:
  `esp-hosted-mcu` README + `docs/esp32_p4_function_ev_board.md`.
- ESP-IDF ESP32-P4 get-started: https://docs.espressif.com/projects/esp-idf/en/stable/esp32p4/get-started/index.html
- ESP-IDF ESP32-P4 hw-reference: https://docs.espressif.com/projects/esp-idf/en/stable/esp32p4/hw-reference/index.html
- ESP-Hosted (esp_wifi_remote) API: https://docs.espressif.com/projects/esp-idf/en/stable/esp32p4/api-reference/network/index.html
- NOTE: HTML pages NOT saved — `documentation.espressif.com` serves a client-side SPA
  shell (identical 13745-byte `<title>…CDP</title>` doc for every path, no rendered content);
  `docs.espressif.com` is ECONNREFUSED in this env. Doc content is reachable only via the
  portal's JS API. RST sources are on GitHub (`espressif/esp-idf/docs/en/...`) if needed.
</content>
</invoke>
