<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# Building SS-SP Firmware

Verified build instructions for the SS-SP firmware monorepo. The canonical,
release-grade path is the **digest-pinned container** (RFC-0002); a local
ESP-IDF install works for day-to-day development.

## Prerequisites

- **Container path (canonical):** Docker (or Podman). Nothing else — the
  toolchain lives in the image.
- **Local path:** ESP-IDF **v5.3.x** installed and exported
  (`. $IDF_PATH/export.sh`), Python 3.10+.

## Option A — reproducible container build (canonical)

The pinned image is defined in `../../ci/containers/firmware/Dockerfile`:
`espressif/idf@sha256:114a9f8cde8bdc5a7e95809745b492663f7c5afe9c2821a4127ff133754fafd2`
(ESP-IDF v5.3.5). CI (`../../.github/workflows/firmware-build.yml`) and the
devcontainer (`../../.devcontainer/devcontainer.json`) use the same digest.

```bash
# From repo root
docker run --rm \
  -v "$PWD":/workspace -w /workspace/firmware \
  espressif/idf@sha256:114a9f8cde8bdc5a7e95809745b492663f7c5afe9c2821a4127ff133754fafd2 \
  idf.py -B build/lite -DSS_BOARD=lite build
```

Output: `firmware/build/lite/ss_sp_firmware.bin`.

Notes:

- The container runs as root, so `firmware/build/` artifacts may be
  root-owned on the host; clean with `sudo rm -rf firmware/build/<board>`.
- First build downloads ~1.5 GB of image layers; subsequent builds reuse them.

## Option B — local ESP-IDF via the Makefile wrapper

```bash
. $IDF_PATH/export.sh   # ESP-IDF v5.3.x
make lite               # or: make alpha | make omega
make flash-lite         # build + flash + monitor
make clean-lite         # remove that board's build dir
```

The wrapper (root `Makefile`, S-02-002) forwards to
`idf.py -B build/<board> -DSS_BOARD=<board> build`. Board can also be chosen
with the `BOARD` environment variable when invoking `idf.py` directly; an
unknown board name fails the configure step with a clear error listing valid
boards.

Only **Lite** has a complete `board_config.h` today; `alpha` and `omega` are
skeleton targets that fail at configure until their board headers land
(EPIC-04/EPIC-05).

## Docs lint

```bash
make lint-docs          # links, anchors, SPDX headers, constitution TOCs
```

## WSL2 environment notes

- The Docker daemon does **not** auto-start after a WSL reboot; start it with
  `sudo service docker start`.
- USB flashing from WSL2 requires attaching the serial device with
  [usbipd-win](https://github.com/dorssel/usbipd-win) from the Windows side
  (`usbipd attach --wsl --busid <id>`), after which `/dev/ttyACM0` /
  `/dev/ttyUSB0` appears in WSL.

## Troubleshooting

- **`hal/aes_types.h` not found** — you have a stale checkout from before the
  `ss_hal` rename (triage ledger T-22); pull latest.
- **Credential-helper errors pulling the image on WSL2/Docker Desktop
  remnants** — point Docker at an empty config:
  `DOCKER_CONFIG=$(mktemp -d) && echo '{}' > $DOCKER_CONFIG/config.json`,
  then rerun the pull with that env var set.
