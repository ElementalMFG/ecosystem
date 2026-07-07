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

## Host tests

Pure, ESP-IDF-free logic (e.g. the `ss_log` `%k` redaction formatter) is tested
on a plain host toolchain — no firmware container or target board required.

**Quick loop (gcc + ASan/UBSan)** — the fastest edit/run cycle for one core:

```bash
make -C firmware/components/ss_log/test/host test
```

**Full loop (googletest + coverage)** — the CMake project under
`firmware/test/host/` builds the tested C sources into an instrumented
`ss_units` library and runs the ported gtest suites (googletest is fetched,
pinned by commit SHA):

```bash
# From repo root; build out-of-tree
cmake -S firmware/test/host -B build/host -DSS_COVERAGE=ON
cmake --build build/host -j
ctest --test-dir build/host --output-on-failure

# Coverage summary + Cobertura XML (needs gcovr: pip install gcovr)
gcovr --root . build/host --filter 'firmware/components/.*' --txt --xml coverage.xml
```

Set `-DSS_COVERAGE=OFF` for a faster uninstrumented run. HAL-header consumers
are covered with a host mock of `board_config.h`
(`firmware/test/host/mocks/`); the real, dependency-free `ss_hal_caps.h` /
`ss_hal_types.h` are used unmodified.

CI runs both loops in [`../../.github/workflows/host-tests.yml`](../../.github/workflows/host-tests.yml):
the `ss-log-redaction` job runs the quick ASan/UBSan harness, and the
`gtest-coverage` job runs the CMake/ctest suite and uploads `coverage.xml` as a
build artifact.

## On-target tests

Hardware-dependent code (HAL entry points, watchdogs, panic paths) is exercised
by the on-target [Unity](https://github.com/ThrowTheSwitch/Unity) test app under
`firmware/test/target/` — a standalone ESP-IDF project so test sources are never
linked into a shipping image. Board selection mirrors the firmware:

```bash
# In the pinned container or with ESP-IDF v5.3.5 exported:
cd firmware/test/target
idf.py -DSS_BOARD=lite build
idf.py -DSS_BOARD=lite flash monitor   # auto-runs every case, prints Unity summary
```

Results reach CI over the console serial (USB-Serial-JTAG on Lite) via
[pytest-embedded](https://docs.espressif.com/projects/pytest-embedded/):

```bash
pip install pytest-embedded-idf pytest-embedded-serial-esp
pytest --embedded-services esp,idf --target esp32s3   # from firmware/test/target/
```

CI runs this in [`../../.github/workflows/target-tests.yml`](../../.github/workflows/target-tests.yml):
`build-target-app` compiles the app for Lite in the pinned container on every PR,
`collect-harness` validates the pytest runner discovers cleanly, and the
board-attached `on-target` job (flash + serial + Unity parse) runs on a
self-hosted runner gated by the `SS_HW_RUNNER` repo variable. See
[`firmware/test/target/README.md`](../../firmware/test/target/README.md) for
adding suites.

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
