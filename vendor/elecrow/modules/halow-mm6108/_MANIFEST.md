<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# Elecrow Wireless Module for Wi-Fi HaLow (Morse Micro MM6108) — vendor documentation

SKU: DAC00106D. GitHub: Elecrow-RD/Wireless-Module-for-Wi-Fi-HaLow (default branch: master).

| filename | description | source URL | fetched | bytes | sha256 | copyright/license note |
|---|---|---|---|---|---|---|
| product-page.html | Elecrow product page (saved HTML) | https://www.elecrow.com/wireless-module-for-wi-fi-halow.html | 2026-07-18 | 327329 | 3ef7baf22e46d2abb5fd84d6a785a279068b41e8e0080f1dca0f0e6cda5b86c4 | © Elecrow; vendor marketing page |
| wiki.html | Elecrow wiki page (saved HTML) | https://www.elecrow.com/pub/wiki/Wireless_Module_for_Wi-Fi_HaLow.html | 2026-07-18 | 139080 | 196e56844b195460cff398db2612def71f0efab68f4ace57ab0994689e4b7c3a | © Elecrow; wiki notes ESP-IDF courses "coming soon" |
| repo-Wireless-Module-for-Wi-Fi-HaLow.zip | GitHub driver/example repo (master branch snapshot) | https://codeload.github.com/Elecrow-RD/Wireless-Module-for-Wi-Fi-HaLow/zip/refs/heads/master | 2026-07-18 | 8109998 | 0c88903427cdb28eadea1a74dc157aa41b2162b35dd0d0ce75db3cee4e1992ca | © Elecrow-RD; check repo LICENSE inside zip |
| User_Manual_Wireless_Module_Wi-Fi_HaLow.pdf | User manual / datasheet for the module | https://www.elecrow.com/download/product/DAC00106D/User_Manual_of_Wireless_Module_for_Wi-Fi_HaLow.pdf | 2026-07-18 | 1293381 | f5f10608d8d4ba00e308142327db6a1e46a2aaa632cd8b08a007bad8b8cb4b75 | © Elecrow |

## Morse Micro MM6108 public SDK / driver / reference sources

Public repositories from the Morse Micro GitHub org (`https://github.com/MorseMicro`),
branch snapshots fetched 2026-07-18. These are the vendor's own open-source HaLow
software (SDK, Linux driver, control utility, ESP32 reference integration).

| filename | description | source URL | fetched | bytes | sha256 | license |
|---|---|---|---|---|---|---|
| repo-mm-esp-halow.zip | `esp-halow` — ESP-IDF component wrapping the MM6108 HaLow SDK for ESP32/ESP32-P4 hosts (API.md, Kconfig, examples: dual_if, iperf, porting_assistant). Submodule-referenced firmware/hostap/mm-iot-sdk not inlined. **Most relevant to the 5" P4 board.** | https://codeload.github.com/MorseMicro/esp-halow/zip/refs/heads/main | 2026-07-18 | 118061 | 14129a69d0a0221071addb2aaaa13b7f2b5975c95149019e1e5042cbede612d7 | Apache-2.0 (see LICENSE in zip) |
| repo-mm-iot-esp32.zip | `mm-iot-esp32` — MM6108 IoT reference integration for ESP32 (deprecated, migrating to `morsemicro/halow` on components.espressif.com; repo archived ~Jul 2026). Concrete ESP32 reference design + examples. | https://codeload.github.com/MorseMicro/mm-iot-esp32/zip/refs/heads/main | 2026-07-18 | 12317112 | 54be7267695ff6334166e4baa094ad217272bd89941e096313b1b4d3bc297d88 | check LICENSE inside zip |
| repo-morse_driver.zip | `morse_driver` — Morse Micro Linux mac80211 kernel driver (SDIO/SPI host). | https://codeload.github.com/MorseMicro/morse_driver/zip/refs/heads/main | 2026-07-18 | 581422 | b3f7452700ff6215c123d573fba479d012e27ae7bd1757521440025cb7258fc0 | GPL-2.0 |
| repo-morse_cli.zip | `morse_cli` — Morse Micro userspace control/config utility. | https://codeload.github.com/MorseMicro/morse_cli/zip/refs/heads/main | 2026-07-18 | 425246 | e869501bcddac493f1527ae8c5a5c92047058473d580fd04d964887f8b7c2048 | check LICENSE inside zip |

Other public MorseMicro repos not mirrored here (available if needed): `mm-iot-sdk`
(≈71 MB platform-agnostic SDK), `hostap`, `firmware_binaries`/`bcf_binaries` (chip
firmware, archived), `mm-iot-zephyr`, `mm-iot-cmsis`, `openwrt`/`morse-feed`, `linux`
(kernel tree with MM patches).

## Not available
- Dedicated schematic PDF / dimension drawing — UNAVAILABLE (Elecrow does not link a standalone schematic; electrical detail is in the user manual PDF above). Product page: https://www.elecrow.com/wireless-module-for-wi-fi-halow.html
- Note: Morse Micro MM6108 chip datasheet + product brief are already mirrored in the repo at vendor/elecrow/components/peripherals/ (MM6108-datasheet.pdf, MM6108-product-brief.pdf). The full bare-SoC MM6108 silicon datasheet remains NDA-gated.
