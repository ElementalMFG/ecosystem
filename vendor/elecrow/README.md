<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# Elecrow round-1 hardware reference set

Local, engineering-reference collection of datasheets, schematics, board
support, and vendor documentation for the **round-1 (D-0026) Elecrow devices**
and their key components. Assembled so the team can build, bring up, and
support these boards (firmware board ids `elecrow5`, `elecrow35-s3`) without
depending on live vendor pages.

## Licensing / redistribution

The files under this tree are **third-party, copyrighted vendor documents**
(Espressif, Elecrow, Morse Micro, Goodix, etc.). They are retained **locally
for internal engineering use only** and are **git-ignored** — this public repo
tracks only our own `*.md` manifests, never the vendor binaries. Do not commit
or redistribute the datasheets/schematics/firmware archives. Each subfolder's
`_MANIFEST.md` records the exact source URL, fetch date, and SHA-256 so the set
is reproducible on demand.

## Structure

```
vendor/elecrow/
├── devices/
│   ├── crowpanel-5in-esp32p4/    # round-1 Omega tier (elecrow5)
│   └── crowpanel-3p5in-esp32s3/  # round-1 Lite/Alpha (elecrow35-s3)
├── modules/
│   └── halow-mm6108/             # Wi-Fi HaLow SPI module (mandatory bearer)
├── components/
│   ├── espressif/                # ESP32-P4 / ESP32-C6 datasheets + TRMs, esp-hosted-mcu
│   ├── espressif-ecosystem/      # esp-bsp, esp32-camera, esp-iot-solution cmps, P4 EV-board schematics
│   └── peripherals/              # GT911, NS4168, SX1262, ILI9488, MM6108 + confirmed 5″-P4 BOM parts
├── repos/
│   ├── elecrow-org/              # Elecrow-RD repos: GT911 driver, ESPHome/LVGL demos, HaLow+camera, P4 kit
│   └── community/                # Meshtastic elecrow_panel variant pin-maps (GPL-3.0, reference-only)
└── <sub>/_MANIFEST.md            # per-folder source + checksum ledger
```

## Confirmed 5″ P4 (`elecrow5`) BOM — from the board's own Eagle schematic

Extracted from the board repo (`Eagle_SCH&PCB/1.0/ESP32-P4 Display 5.0 inch V1.0.sch`); datasheets for each fetched under `components/`:

- **SoC:** ESP32-P4 + **ESP32-C6-MINI-1** (2.4 GHz Wi-Fi 6 / BLE companion) · **flash** W25Q128JV (16 MB) · **aux MCU** STC8H1K08.
- **Display:** **16-bit parallel RGB565 800×480** (40-pin FPC JP1; `esp_lcd` **RGB** panel path — **NOT MIPI-DSI**; D-0027 netlist read corrected the earlier P4-web-spec assumption) + **GT911** touch.
- **Audio:** **NS4168** Class-D I²S amp ×2 + **MMICT5838** MEMS mic (no public DS).
- **Camera:** **none on-board** — only a 24-pin **MIPI-CSI FPC header** (external module; the OV5647/SC2336 question is therefore moot for the PCB).
- **USB-serial:** WCH **CH340K**. **Charger:** Top Power **TP4059** (600 mA linear). **Fuel gauge:** none fitted.
- **Power:** discrete rails — TI **TLV62569** buck ×2 + Micro One **ME6211** LDOs + AEROSEMI **MT9201** backlight boost + SG Micro **SGM3005** switch (no single-chip PMIC).
- **HaLow:** Morse Micro **MM6108** module in the SPI slot; ESP-IDF driver = MorseMicro **esp-halow** (Apache-2.0) under `modules/halow-mm6108/`.

## Key engineering findings (inform EPIC-05 bring-up)

- **C6↔P4 bus = SDIO / ESP-Hosted** (the ESP32-P4-Function-EV-Board reference schematic in `components/espressif-ecosystem/` wires C6-as-radio over SDIO; ESP-IDF **v5.3+** required). Confirm the exact bus on the Elecrow board against its extracted Eagle schematic — informs **S-05-044**.
- **Camera is external-only** — no on-board sensor to auto-detect; scope **S-05-045** to the CSI header + optional module, not a fixed part.
- **HaLow-over-SPI** driver basis exists: `esp-halow` (Apache-2.0) + `mm-iot-esp32` reference — informs **S-05-043**.

## Rehydrating

Each `_MANIFEST.md` lists source URLs; re-run the recorded `curl -L` commands to
re-download. Gated/NDA datasheets (e.g. some Morse Micro MM6108 material) are
marked `UNAVAILABLE (gated)` with the request path noted.

Related: `docs/dev/OMEGA_HW_BASELINE.md`, decision **D-0026**, EPIC-05 stories
`S-05-041..047`.
