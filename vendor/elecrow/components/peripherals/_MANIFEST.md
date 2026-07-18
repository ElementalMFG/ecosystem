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
| OV5647-datasheet.pdf | OV5647 (OmniVision) | 5MP MIPI-CSI camera sensor — reference camera for optional CSI module | https://cdn.sparkfun.com/datasheets/Dev/RaspberryPi/ov5647_full.pdf | 2026-07-18 | 2524291 | 4c974dfb7a54bc9aedce444a2b3407fc2d6711409a017b666ae250652cebefb3 | 140 pages, full preliminary spec. Kept as the ESP32-P4/RPi reference MIPI-CSI camera. **NOTE:** schematic confirms NO camera sensor is fitted on the 5" P4 PCB — the board only exposes a 24-pin MIPI-CSI FPC header (`CSI_Interface`, connector AFC24-S24FIA-00) for an optional external camera module. See Confirmed BOM below. |
| TLV62569-datasheet.pdf | TLV62569 (Texas Instruments) | 3A 17V synchronous step-down (buck) converter — main DC-DC rail (U9/U10) | https://www.ti.com/lit/ds/symlink/tlv62569.pdf | 2026-07-18 | 1356806 | 8783d5302cef63f9e072d8f3f4cd75086caa2bf11e74ed57e51cb2c9f82c93d4 | 30 pages, official TI datasheet. `dev` note in .sch lists MT3406 as a drop-in alternate. |
| ME6211-datasheet.pdf | ME6211 (Nanjing Micro One / MICRONE) | 500 mA CMOS LDO regulator family — 1.8 V/2.8 V/3.3 V rails (IC2 C18M, IC3 C28M, IC7 C33M) | https://datasheet4u.com/pdf/825724/ME6211.pdf | 2026-07-18 | 715221 | 970bbfaa1476c32eb5e06a600c44c4d3c3ca43e849e55b29597a2c252ee6d2cc | 25 pages. datasheet4u mirror (LCSC/manufacturer Cloudflare/China-gated). |
| TP4059-datasheet.pdf | TP4059 (Top Power ASIC / NanJing Top Power) | 600 mA linear Li-ion battery charger, SOT23-6, with reverse-battery protection (U2) | https://datasheet4u.com/pdf/1299989/TP4059.pdf | 2026-07-18 | 827063 | 1b024e025dc7612eed8d016910817e2ad3a990794ace53b495a769aa451cd9d3 | 12 pages. datasheet4u mirror (manufacturer toppwr.com unreachable from env). |
| MT9201-datasheet.pdf | MT9201 (AEROSEMI) | High-efficiency boost white-LED driver — LCD backlight driver (U6, lib name COOPER_MT9201) | https://datasheet4u.com/pdf/1538474/MT9201.pdf | 2026-07-18 | 453925 | e1fef8aa5a1be44d81e29ed80bf6a4e296ae5d6e498c87ecb0ea4c4f731e0476 | 10 pages. datasheet4u mirror. |
| SGM3005-datasheet.pdf | SGM3005 (SG Micro / Shengbang) | Low-voltage dual SPDT analog switch — signal routing (U5) | https://datasheet4u.com/pdf/1005019/SGM3005.pdf | 2026-07-18 | 1197871 | 3269ea0471f0f49e0d3a948711be43263ec71cad177a817e636b4aca88184d7a | 13 pages. datasheet4u mirror (sg-micro.com China-hosted, unreachable). |
| CH340K-datasheet.pdf | CH340K (WCH / 沁恒) | USB-to-serial bridge (U1) — used for programming/UART | https://cdn.sparkfun.com/datasheets/Dev/Arduino/Other/CH340DS1.PDF | 2026-07-18 | 147893 | 04c805e8242885fd1cf21f05dbfd9d16b9fa38f0439ce0d3c6d7f74ebe4cf4af | 6 pages. WCH CH340DS1 (SparkFun mirror); the DS1 covers the whole CH340 family incl. CH340K. |
| W25Q128JV-datasheet.pdf | W25Q128JV (Winbond) | 128 Mbit (16 MB) SPI/QSPI NOR flash (IC4) | https://www.winbond.com/resource-files/w25q128jv%20revf%2003272018%20plus.pdf | 2026-07-18 | 2462647 | 809f066e62bcde10b12c2202daf05f4776929ad7dc5f9d3b5131cdcc84502bc1 | 78 pages, official Winbond Rev.F. |
| STC8H1K08-datasheet.pdf | STC8H1K08 (STC / 姚永平) | 8051-core MCU, TSSOP-20 — auxiliary/power-key coprocessor (U14, val STC8H1K17) | https://datasheet4u.com/pdf/1305886/STC8H1K08S2.pdf | 2026-07-18 | 6838682 | 476052942b028be70f4bf42852272f64c82a8c55622a218f6bf4e245c1fcadbb | 509 pages, full STC8H series manual (covers STC8H1K08). datasheet4u mirror. |

## Confirmed BOM (5″ P4)

Identities extracted from the official Elecrow Eagle schematic + netlist inside
`vendor/elecrow/devices/crowpanel-5in-esp32p4/repo-crowpanel-5in-esp32p4.zip` at
`…-master/Eagle_SCH&PCB/1.0/ESP32-P4 Display 5.0 inch V1.0.sch` (and its `.pdf`).
Evidence = the Eagle `deviceset`/`value` for each part reference designator.

| function | confirmed part | ref | evidence (schematic deviceset/value) |
|---|---|---|---|
| Host MCU | ESP32-P4NRW32 (ESP32-P4, 32 MB PSRAM) | U7 | `deviceset ESP32-P4NRW32` |
| 2.4 GHz Wi-Fi6/BT companion | ESP32-C6-MINI-1-N4 | IC1 | `deviceset ESP32-C6-MINI-1-N4` (also stated in user manual "Wireless Chip: ESP32-C6-MINI") |
| SPI/QSPI NOR flash | Winbond W25Q128JVSIQ (16 MB) | IC4 | `deviceset W25Q128JVSIQ` |
| USB-serial bridge | **WCH CH340K** (not CP2102) | U1 | `deviceset CH340K` |
| Li-ion charger | **Top Power TP4059** (600 mA linear, SOT23-6) | U2 | `deviceset TP4059-SOT23-6` |
| Fuel gauge | **NONE fitted** — no dedicated gauge IC on the board | — | only charger (TP4059) present; no MAX1704x/CW2015/BQ27xxx in netlist. Battery sense presumed via ADC divider. |
| Main DC-DC (buck) | **TI TLV62569** (MT3406 alt) ×2 | U9, U10 | `deviceset TLV62569DBVR` |
| LDOs (1.8/2.8/3.3 V) | **Micro One ME6211** C18M/C28M/C33M | IC2, IC3, IC7… | `deviceset ME6211C18M5G-N`, values `ME6211C28M5G-N` / `ME6211C33M5G-N` |
| LCD backlight driver | **AEROSEMI MT9201** (boost WLED driver) | U6 | `deviceset COOPER_MT9201 value MT9201` |
| Analog switch / signal mux | **SG Micro SGM3005** (dual SPDT) | U5 | `deviceset SGM3005XMSTR` |
| Aux/power-key coprocessor | **STC STC8H1K08** (8051 MCU) | U14 | `deviceset STC8H1K08-36I value STC8H1K17-36I` |
| Audio power amp | **NS4168** (Class-D I²S, 2.5 W) ×2 | U13, U15 | `deviceset NS4168-ESOP-8 value NS4168` (datasheet already in repo) |
| Microphone | MEMS mic MMICT5838-00-012 | U175 | `deviceset MMICT5838-00-012_…` (analog/PDM MEMS mic; obscure vendor part) |
| Camera | **NO on-board sensor** — 24-pin MIPI-CSI FPC header only | FPC/connector AFC24-S24FIA-00 | schematic sheet `CSI_Interface`; nets `CSI_CLKP/N`, `CSI_DATAP/N0/1`. Camera is an *optional external module*; neither SC2336 nor OV5647 is populated on the PCB. |
| Display panel | **16-bit parallel RGB565** off the ESP32-P4 (`esp_lcd` RGB path) — **NOT MIPI-DSI** (corrected per D-0027) | 40-pin FPC JP1 (CROWPANEL_ADVICE_HMI) | RGB data on GPIO4–19, PCLK=GPIO3, DE=GPIO2, HSYNC=GPIO40, VSYNC=GPIO41; **no `DSI_*`/`MIPI` nets and no bridge/TCON IC** in the netlist. The earlier "MIPI-DSI/native DSI" entry was the superseded P4-web-spec reading. |
| Touch | GT911 (Goodix) capacitive | — | I²C touch (datasheet already in repo) |
| Crystals | 40 MHz (X322540MPB4SI), 24 MHz, 32.768 kHz | Y2/Y3/Y4 | `deviceset` values |
| Charge-path / load switches | MOSFETs AO3401 (P), BSS138W (N), NCE20P45Q, diode array UMH3NTN | Q*, U8 | passives-class |

**Resolution of prior UNCONFIRMED items:** the camera question is moot — no camera
sensor is fitted (CSI header only), so neither SC2336 nor OV5647 is on the board;
the display is **16-bit parallel RGB565 (no MIPI/DSI — D-0027)**, so no DSI bridge or
TCON is needed; PMIC = discrete TLV62569 buck +
ME6211 LDOs (no single-chip PMIC); charger = TP4059; **no fuel gauge present.**

## UNAVAILABLE / not obtained

- **SC2336 (SmartSens) — 2MP MIPI-CSI image sensor** — full datasheet **UNAVAILABLE (gated)**. China-hosted flyer `https://smartsens.oss-cn-beijing.aliyuncs.com/web/products/SC2336_V2.0.pdf` unreachable (connection refused); gophotonics product page `https://www.gophotonics.com/products/cmos-image-sensors/smartsens-technology/21-1025-sc2336` returns HTTP 403 (Cloudflare); smartsenstech.com China-hosted/unreachable; not on datasheet4u/alldatasheet mirrors. SmartSens full DS is NDA-gated. **No longer on the critical path:** the 5" P4 schematic confirms no camera sensor is fitted (CSI FPC header only), so no on-board camera part needs a datasheet; OV5647 (above) is retained as the ESP32-P4 reference-camera candidate.
- **MMICT5838 MEMS microphone** — datasheet not obtained; obscure vendor part (`MMICT5838-00-012_LMD3526B261-OFA03`), no public datasheet found on reachable mirrors. Best-effort only.
- Note: LCSC (`datasheet.lcsc.com` → `www.lcsc.com/datasheet/…`), SG Micro (`sg-micro.com`), Top Power (`toppwr.com`), and `datasheetspdf.com` are all Cloudflare-gated or China-hosted and unreachable from this environment; `datasheet4u.com` (direct `/pdf/<id>/<part>.pdf`) and manufacturer sites (TI, Winbond) were the reachable sources used above.
