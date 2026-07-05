---
# SPDX-License-Identifier: Apache-2.0
paths:
  - "firmware/**"
---

# Firmware domain facts (verified 2026-07)

- Toolchain: ESP-IDF **v5.3.5**, canonical build = digest-pinned container (`ci/containers/firmware/Dockerfile`, policy RFC-0002). Local: `make lite|alpha|omega` wraps `idf.py -B build/<board> -DSS_BOARD=<board> build`. Board selected via `SS_BOARD` (default `lite`).
- Target: ESP32-S3-WROOM-1-N16R8. `sdkconfig.defaults`: 16 MB flash, 8 MB octal PSRAM @ 80 MHz, console on **USB-Serial-JTAG** (UART1/2 free for peripherals), FreeRTOS tick 1000 Hz. Secure boot + flash encryption options present but **commented out until EPIC-08** — do not enable ad hoc.
- `partitions.csv`: A/B OTA layout (EPIC-09) — `ota_0`/`ota_1` 6 MB each, `nvs`, `otadata`, `phy_init`, ~4 MB SPIFFS `storage`.
- `firmware/main/` is the only real application code today: `main.cpp` (startup + UART engine init), `ss_compass`, `ss_diag`, `ss_display_boot`, `ss_uart_engine`, `Kconfig.projbuild`.
- `firmware/components/`: **only `ss_hal` is real** (header-only contracts in `include/ss_hal_*.h`; exports the selected board's `board_config.h`). All other `ss_*` components are empty epic-gated scaffolds — first code in one means creating its `CMakeLists.txt` (follow `main/` pattern), not editing existing code.
- Boards: only `firmware/boards/lite/board_config.h` exists (authoritative pin map — changes require lockstep update of `01_SS-SP_LITE_HARDWARE_REFERENCE.md` + the pin-map CI test; treat as T1). `alpha`/`omega` fail at configure by design.
- App code queries `ss_hal_has_cap()` — never `CONFIG_*` or board macros directly (Universal Test rule, `ss_hal_caps.h`).
- C standard: clang-format, `-Wall -Wextra -Wshadow -Wconversion -Werror`, MISRA-C:2012 where practical; public functions document pre/post-conditions and error behavior; SPDX Apache-2.0 + copyright header on new files.
- T1 paths (stop/escalate per doc 10 §8.3): `components/ss_crypto/**`, `components/ss_hal/**`, `bootloader/**`, `ota/**`, `provisioning/**`, `firmware/security/**`.
- Clean-room: never copy from Meshtastic firmware or its `.proto` files (`CONTRIBUTING.md` §6).
- Gate: `make lite` must pass; CI `firmware-build.yml` builds `lite` on any PR touching `firmware/**` (required check).
