---
# SPDX-License-Identifier: Apache-2.0
paths:
  - "firmware/**"
---

# Firmware domain facts (verified 2026-07)

- Toolchain: ESP-IDF **v5.3.5**, canonical build = digest-pinned container (`ci/containers/firmware/Dockerfile`, policy RFC-0002). Local: `make lite|alpha|omega` wraps `idf.py -B build/<board> -DSS_BOARD=<board> build`. Board selected via `SS_BOARD` (default `lite`).
- Target: ESP32-S3-WROOM-1-N16R8. **S3 sleep-API gotcha:** no `esp_deep_sleep_enable_gpio_wakeup` on Xtensa — deep wake = `ext0`/`ext1` on RTC-capable GPIO 0..21 only (touch INT GPIO47 is light-sleep-wake only via `gpio_wakeup_enable`). `sdkconfig.defaults`: 16 MB flash, 8 MB octal PSRAM @ 80 MHz, console on **USB-Serial-JTAG** (UART1/2 free for peripherals), FreeRTOS tick 1000 Hz. Secure boot + flash encryption options present but **commented out until EPIC-08** — do not enable ad hoc.
- `partitions.csv`: A/B OTA layout (EPIC-09) — `ota_0`/`ota_1` 6 MB each, `nvs`, `otadata`, `phy_init`, ~3.9 MB SPIFFS `storage`, 64 KiB `coredump` (S-02-008). FROZEN map — changes follow the freeze note + RFC-0003.
- `firmware/main/` is the only real application code today: `main.cpp` (startup + UART engine init), `ss_compass`, `ss_diag`, `ss_display_boot`, `ss_uart_engine`, `Kconfig.projbuild`.
- `firmware/components/`: real components are `ss_hal` (header-only contracts; exports the selected board's `board_config.h`), `ss_log` (S-02-007), and `ss_power` (S-03-001). Remaining `ss_*` components are empty epic-gated scaffolds — first code in one means creating its `CMakeLists.txt`.
- Boards: ALL THREE ports exist and compile in CI (`board-parity` enforces the 104-define set). `lite/board_config.h` is the authoritative pin map (changes require lockstep doc-01 update; treat as T1); `alpha`/`omega` are honest TODO(models/CATALOG) skeletons.
- App code queries `ss_hal_has_cap()` — never `CONFIG_*` or board macros directly (Universal Test rule, `ss_hal_caps.h`).
- C standard: clang-format, `-Wall -Wextra -Wshadow -Wconversion -Werror`, MISRA-C:2012 where practical; public functions document pre/post-conditions and error behavior; SPDX Apache-2.0 + copyright header on new files.
- T1 paths (stop/escalate per doc 10 §8.3): `components/ss_crypto/**`, `components/ss_hal/**`, `bootloader/**`, `ota/**`, `provisioning/**`, `firmware/security/**`.
- Clean-room: never copy from Meshtastic firmware or its `.proto` files (`CONTRIBUTING.md` §6).
- Gate: `make lite` must pass; CI `firmware-build.yml` builds the FULL [lite, alpha, omega] matrix on any PR touching `firmware/**` (required check: `build (lite)`).
