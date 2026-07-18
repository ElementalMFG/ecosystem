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
│   ├── espressif/                # ESP32-P4 / ESP32-C6 datasheets + TRMs, ESP-Hosted
│   └── peripherals/              # GT911, NS4168, SX1262, display/camera, PMIC/fuel-gauge…
└── <sub>/_MANIFEST.md            # per-folder source + checksum ledger
```

## Rehydrating

Each `_MANIFEST.md` lists source URLs; re-run the recorded `curl -L` commands to
re-download. Gated/NDA datasheets (e.g. some Morse Micro MM6108 material) are
marked `UNAVAILABLE (gated)` with the request path noted.

Related: `docs/dev/OMEGA_HW_BASELINE.md`, decision **D-0026**, EPIC-05 stories
`S-05-041..047`.
