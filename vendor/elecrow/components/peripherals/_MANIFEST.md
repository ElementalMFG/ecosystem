<!-- SPDX-License-Identifier: CC-BY-4.0 -->

# Elecrow round-1 boards — peripheral IC datasheets

Datasheets for the peripheral ICs on the Elecrow round-1 boards (5" ESP32-P4 and
3.5" ESP32-S3). Fetched 2026-07-18. Files verified as real PDFs (`file` + `pdfinfo`),
sizes and SHA-256 below.

| filename | part | function | source URL | fetched | bytes | sha256 | note |
|---|---|---|---|---|---|---|---|
| GT911-datasheet.pdf | GT911 (Goodix) | 5-point capacitive touch controller (both 5" P4 and 3.5" S3 boards) | https://www.fortec-integrated.de/fileadmin/pdf/produkte/Touchcontroller/DDGroup/GT911_Datasheet.pdf | 2026-07-18 | 1634301 | 227d240eb4344a643237ffcbff0da08d4765dd75f6d335382c283a77fc26c8ab | 25 pages. Goodix does not host publicly; Fortec (integrated-display vendor) mirror used. |
| NS4168-datasheet.pdf | NS4168 (YONGFUKANG / ChipSourceTek) | 2.5W mono Class-D I2S audio power amplifier (speaker, 5" P4) | https://www.chipsourcetek.com/Uploads/file/NS4168.pdf | 2026-07-18 | 1368171 | 38fc6e8bc8dc2f58c21d8d5830424a590831d635f69f2e30333b4686bb8cd3ec | 13 pages. Distributor (ChipSourceTek) copy; required browser UA + referer to fetch. |
| SX1262-datasheet.pdf | SX1262 (Semtech) | Sub-GHz LoRa/FSK transceiver (round-1 LoRa sub-variant, S-05-047) | https://cdn.sparkfun.com/assets/6/b/5/1/4/SX1262_datasheet.pdf | 2026-07-18 | 2573842 | 644b55f38e97309161ea0c952a3a093020d94268ca8276eca53e5aa933ae7068 | 111 pages. Official Semtech SX1261/2 Data Sheet Rev. 1.2 (SparkFun mirror; Semtech site gated). |
| ILI9488-datasheet.pdf | ILI9488 (Ilitek) | a-Si TFT LCD single-chip driver, 320(RGB)x480, SPI (3.5" S3 display) | https://www.crystalfontz.com/controllers/uploaded/ILI9488%20Data%20Sheet_100.pdf | 2026-07-18 | 10826145 | aeb23170f809610458e05ab514a5e3017344938ccdba1c2031acb453941df38b | 343 pages, V1.00 full datasheet. Crystalfontz mirror (Ilitek gated). |
| MM6108-datasheet.pdf | MM6108-MF08651-US (Morse Micro) | 802.11ah Wi-Fi HaLow module data sheet | https://www.morsemicro.com/resources/datasheets/modules/MM6108-MF08651-US_Data_Sheet.pdf | 2026-07-18 | 1613851 | 70efa0acc87675070b39ca26a37041acbf27b976a2ca5b96c81a1d0aa4fe18da | 35 pages. Public module data sheet from manufacturer (NOT NDA-gated — full module DS obtained). Full bare-SoC MM6108 silicon datasheet remains NDA-gated. |
| MM6108-product-brief.pdf | MM6108 (Morse Micro) | Wi-Fi HaLow SoC/module product brief | https://www.morsemicro.com/wp-content/uploads/2024/01/USA_MM6108-MF08651-US_Product-Brief_240104-CES-Digital.pdf | 2026-07-18 | 304665 | 06c0546f341d824f2677ea94656db9c11114dfdcfd8bf80508f15134605db2ec | 2 pages. Public product brief, kept alongside the module DS. |
| OV5647-datasheet.pdf | OV5647 (OmniVision) | 5MP MIPI-CSI camera sensor — candidate for 5" P4 camera | https://cdn.sparkfun.com/datasheets/Dev/RaspberryPi/ov5647_full.pdf | 2026-07-18 | 2524291 | 4c974dfb7a54bc9aedce444a2b3407fc2d6711409a017b666ae250652cebefb3 | 140 pages, full preliminary spec. **UNCONFIRMED**: OV5647 is the ESP32-P4/RPi reference MIPI-CSI camera. The Elecrow CrowPanel Advanced 5" P4 advertises an *optional 2MP* camera (2MP → more likely the SmartSens SC2336; OV5647 is 5MP). Exact fitted part not confirmed. |

## UNAVAILABLE / not obtained

- **SC2336 (SmartSens) — 2MP MIPI-CSI image sensor** — best-fit candidate for the Elecrow 5" P4 "2MP camera" option. UNAVAILABLE: only public source is the China-hosted product flyer `https://smartsens.oss-cn-beijing.aliyuncs.com/web/products/SC2336_V2.0.pdf`, which is unreachable from this environment (connection refused / ECONNREFUSED 39.103.20.131:443); gophotonics product page returns HTTP 403. Full SmartSens datasheet is NDA-gated. OV5647 downloaded above as the reachable camera candidate instead. **UNCONFIRMED** which of SC2336/OV5647 is actually fitted.
- **MIPI-DSI bridge / timing-controller IC (5" P4 display)** — UNAVAILABLE / **UNCONFIRMED**. The ESP32-P4 drives the panel over its native MIPI-DSI controller, so there is likely no discrete DSI-bridge IC; the display controller is integrated in the panel and its exact part number is not documented publicly for the Elecrow board. No authoritative datasheet identified. Best source found: Elecrow product page `https://www.elecrow.com/crowpanel-advanced-5inch-esp32-p4-hmi-ai-display-800x480-ips-touch-screen-with-wifi-6.html`.
- **Li-ion charger / fuel-gauge / PMIC** — SKIPPED: no charger/fuel-gauge/PMIC part number could be confirmed for the round-1 Elecrow boards from public documentation. Not downloaded to avoid mislabeling.
