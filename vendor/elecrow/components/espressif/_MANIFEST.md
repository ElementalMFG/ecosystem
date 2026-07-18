<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# Espressif vendor docs — Elecrow round-1 SoCs (ESP32-P4 host + ESP32-C6 co-processor)

Official Espressif datasheets, TRMs, and ESP-Hosted docs for the SoCs on the Elecrow round-1
boards. Downloaded read-only for reference (S-05-044: C6<->P4 bus). Licenses are Espressif's;
this manifest is CC-BY-4.0.

| filename | what it is | source URL | fetched | bytes | sha256 | note |
|---|---|---|---|---|---|---|
| esp32-p4_datasheet_en.pdf | ESP32-P4 datasheet | https://documentation.espressif.com/esp32-p4-chip-revision-v1.3_datasheet_en.pdf | 2026-07-18 | 1603774 | b2b0ae6fb8e92d23dbbfbd4c79e10ff58d549bac7788a69a8c2b34cdff6c4d8d | Latest official: chip rev v1.3, datasheet v1.2. Supersedes the preliminary v0.5 at www.espressif.com/sites/default/files/documentation/esp32-p4_datasheet_en.pdf |
| esp32-p4_technical_reference_manual_en.pdf | ESP32-P4 Technical Reference Manual | https://documentation.espressif.com/esp32-p4-chip-revision-v1.3_technical_reference_manual_en.pdf | 2026-07-18 | 20786220 | ae1fa2a411776760e03329adc3f9a7c13e98c440168cab208f06e3b2d5818830 | Latest official: chip rev v1.3, TRM pre-release v0.4 |
| esp32-c6_datasheet_en.pdf | ESP32-C6 datasheet | https://www.espressif.com/sites/default/files/documentation/esp32-c6_datasheet_en.pdf | 2026-07-18 | 975625 | 372a5b42b2900c83ef4309149c8835e9ae2d1be19244995bf2fb83af4dc5edf1 | ESP32-C6 series datasheet (Wi-Fi 6 / BLE co-processor on the 5-inch P4 board) |
| esp32-c6_technical_reference_manual_en.pdf | ESP32-C6 Technical Reference Manual | https://www.espressif.com/sites/default/files/documentation/esp32-c6_technical_reference_manual_en.pdf | 2026-07-18 | 12683821 | e08d5b61ebcfa2a78f9735e3f721817cb1dc2df9cf05096d2f0399eaba9f1734 | ESP32-C6 series TRM |
| esp-hosted-mcu.zip | ESP-Hosted-MCU repo snapshot (C6-as-coprocessor over SDIO/SPI/UART; docs/sdio.md, docs/spi_*.md, docs/esp32_p4_function_ev_board.md) | https://codeload.github.com/espressif/esp-hosted-mcu/zip/refs/heads/main | 2026-07-18 | 9592565 | f8bcee78fedb8dcfaaaf5c08ee8693a4aa03f74a17f1a5a1b39acc5e411c5256 | main branch, commit 949bb30612747a3bd9e402eda8d01fbfa1f8503e (2026-07-15). Directly relevant to S-05-044 (C6<->P4 bus). Contains the P4-Function-EV-Board reference-design doc (P4 host + on-board C6 over SDIO) |
| esp32-p4_function_ev_board_user_guide_en.pdf | ESP32-P4-Function-EV-Board user guide (reference P4+C6 design) | https://www.espressif.com/sites/default/files/documentation/esp32-p4_function_ev_board_user_guide_en.pdf | 2026-07-18 | — | — | UNAVAILABLE — sites/default path now returns the Espressif doc-portal SPA (HTML), not a PDF; docs.espressif.com esp-dev-kits PDF (…/esp32p4/esp-dev-kits-en-master-esp32p4.pdf) is network-unreachable from the build env (ECONNREFUSED). HTML guide: https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32p4/esp32-p4-function-ev-board/user_guide.html . Equivalent P4+C6 reference-design content is in esp-hosted-mcu.zip -> docs/esp32_p4_function_ev_board.md |

## ESP-IDF / SoC-support reference (not downloaded — URLs pinned)

- ESP32-P4 hardware reference (ESP-IDF stable): https://docs.espressif.com/projects/esp-idf/en/stable/esp32p4/hw-reference/index.html
- ESP32-C6 hardware reference (ESP-IDF stable): https://docs.espressif.com/projects/esp-idf/en/stable/esp32c6/hw-reference/index.html
- ESP-Hosted-MCU repo/docs: https://github.com/espressif/esp-hosted-mcu (transports: SDIO/SPI/UART; SDIO master supports ESP32-P4, SDIO slave supports ESP32-C6)
