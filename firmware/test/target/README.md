<!-- SPDX-License-Identifier: Apache-2.0 -->
# On-target Unity test app (S-02-015)

A standalone ESP-IDF project that runs [Unity](https://github.com/ThrowTheSwitch/Unity)
test cases on real SS-SP silicon — the counterpart to the host-side gtest
baseline in [`../host/`](../host/) (S-02-014). Hardware-dependent code that a
host toolchain cannot exercise (HAL entry points, watchdogs, panic paths) is
tested here.

## Why a separate project

The main firmware (`firmware/CMakeLists.txt`) lists its `SRCS` explicitly and
never globs, so test sources — including the panic-by-design hang tests written
by S-02-006 / S-02-009 under `firmware/main/test/` — can never be linked into a
shipping image. This project links them (and the HAL smoke test) instead.

## Build & run (attached board)

```bash
# From this directory, with ESP-IDF v5.3.5 exported (or inside the pinned
# container — see docs/dev/BUILDING.md). Board selection mirrors the firmware.
idf.py -DSS_BOARD=lite build
idf.py -DSS_BOARD=lite flash monitor      # auto-runs every case, prints summary
```

`main/test_app_main.c` calls `UNITY_BEGIN(); unity_run_all_tests(); UNITY_END();`,
so both an interactive `monitor` and an unattended CI runner get a complete,
self-terminating Unity summary over the console serial (USB-Serial-JTAG on Lite).

## Serial results to CI

[`pytest_ss_target.py`](pytest_ss_target.py) drives the board through
[pytest-embedded](https://docs.espressif.com/projects/pytest-embedded/):

```bash
pip install pytest-embedded-idf pytest-embedded-serial-esp
pytest --embedded-services esp,idf --target esp32s3
```

It flashes the built app, reads the Unity summary over serial, and fails on any
non-zero failure/error count. This runs on a self-hosted runner with an attached
Lite board; the always-on CI job only builds the app in the pinned container
(no board there). See [`../../../.github/workflows/target-tests.yml`](../../../.github/workflows/target-tests.yml).

## Adding a suite

List the new `test_*.c` in [`main/CMakeLists.txt`](main/CMakeLists.txt) and
`REQUIRES` the component it exercises. Cases register automatically via the
`TEST_CASE("name", "[tags]")` macro; no runner edits are needed.
