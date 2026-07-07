<!-- SPDX-License-Identifier: Apache-2.0 -->
---
name: defects-coredump-panic
description: Recurring defect classes in ESP-IDF coredump/panic-handler/crash-loop-breaker code (S-02-008 family)
metadata:
  type: project
---

Recurring defects seen in panic/coredump T1 work (first hit: S-02-008).

- **espcoredump `--core-format` default.** A dump read out of the flash `coredump` partition (esptool read_flash) is format `raw` (carries `core_dump_header_t`; `get_core_file_format` returns `raw`). Only an already-extracted file starts with `\x7fELF` → `elf`. espcoredump's own default is `auto` (auto-detects). Any wrapper that hardcodes `--core-format elf` breaks the offline/file decode path. **Why:** the AC "record decodes with the tool" targets the field/no-debugger path, which is the file path. **How to apply:** default wrappers to `auto` (verify the IDF-bundled version supports it; IDF 5.3.5 does) or `raw`, and make the docstring example a `.bin`, not `.elf`.

- **Reset-reason enum gaps.** `is_crash_reset()` allow-lists PANIC/TASK_WDT/INT_WDT/WDT. IDF v5.x also has ESP_RST_CPU_LOCKUP (double-exception/lockup — a genuine crash) and ESP_RST_PWR_GLITCH/EFUSE/USB/JTAG. Omitting CPU_LOCKUP means that crash-loop class never trips the breaker (esp. relevant for the P4/RISC-V Alpha port where lockup is a distinct HW reset). The `default:` branch is **fail-open**: it *zeroes* the counter, so a stray ESP_RST_UNKNOWN mid-loop resets progress toward safe mode. **How to apply:** diff the switch against the pinned container's `esp_system.h`; decide the fail direction for unknown/new reasons deliberately.

- **CONFIG_ESP_COREDUMP_CAPTURE_DRAM not pinned.** Default is `n` (stacks+TCBs only). If flipped to `y`, the dump captures all .data/.bss/heap = all live secrets, to a partition that is plaintext until flash-encryption lands. A hygiene contract that claims "stacks and TCBs only" must pin `=n` explicitly (same self-documentation logic the file already applies to the stack canary).

- **App-level breaker scope.** A boot gate in `app_main` cannot catch crashes before app_main: 2nd-stage bootloader, `ESP_SYSTEM_INIT_FN` hooks, C++ global ctors, scheduler start. The counter never increments → unbounded loop the breaker misses. A truly robust breaker needs a bootloader-level counter. Watch for headers claiming "can never be re-entered."

- **Coredump register spillage.** Even with secrets off-stack and zeroized, the crashing task's saved register file is in the dump and can hold in-flight secret values. "stacks/TCBs + zeroize" guidance doesn't cover it.

- **v5.3.5 esp_reset_reason_t has ALL 16 enumerators** (verified against tag v5.3.5 `esp_system/include/esp_system.h`): UNKNOWN, POWERON, EXT, SW, PANIC, INT_WDT, TASK_WDT, WDT, DEEPSLEEP, BROWNOUT, SDIO, USB, JTAG, EFUSE, PWR_GLITCH, CPU_LOCKUP. So a `classify_reset` switch naming CPU_LOCKUP/USB/JTAG/EFUSE/PWR_GLITCH compiles clean on 5.3.5 — no build break (none are 5.4+ only). Also verified: `CONFIG_ESP_COREDUMP_CAPTURE_DRAM` is a real Kconfig symbol (`components/espcoredump/Kconfig`) so pinning `=n` is effective, not phantom. And IDF 5.3.5 pins `esp-coredump~=1.10`, whose `cli_ext.py` `--core-format` choices are `{auto,b64,elf,raw}` default `auto` — a wrapper defaulting to `auto` is valid.

- **`classify_reset` (the esp_reset_reason→class mapping) is untested by design.** It lives in the IDF glue (`.cpp`), so the host/core split leaves the single most enum-drift-prone function with ZERO automated coverage — verified only by review against the pinned enum. This is the exact function a prior review's F2 was about. Acceptable per the team's glue-review pattern, but worth a standing NIT: a host-testable mirror enum + mapping table would let the classification be unit-tested.
