<!-- SPDX-License-Identifier: Apache-2.0 -->
---
name: gotchas-ss-hal
description: ss_hal component seams that recur across reviews (umbrella header, host-test coverage, header self-containment)
metadata:
  type: project
---

- **`ss_hal.h` umbrella opens `extern "C" {` before 6 late `#include`s** (secure_elem/rng/time/usb/watchdog/ota; S-03-032). Compiles clean in C and C++ ONLY because those headers are already pulled in above (lines 17-20) so the wrapped includes are idempotent no-ops, and nested `extern "C"` is legal. It is still a wrong-point guard insertion — the opening brace belongs AFTER all includes, wrapping only the function declarations. Fragile: any future C++-header/construct in those 6 breaks it. See [[defects-scripted-guard-placement]].

- **Host suite (`firmware/test/host`) does NOT compile `ss_hal/src/ss_hal.c`.** `ss_units` (CMakeLists ~L47-56) omits it and `test_hal_mock.cpp` includes only the mock `board_config.h` — it tests the `SS_BOARD_CAPS` macro directly and never calls `ss_hal_has_cap()`/`ss_hal_caps_mask()`. Reason ss_hal.c can't be host-built: `ss_hal.h`→`ss_hal_muxctl.h`→`freertos/FreeRTOS.h` (no host mock). So a green "109/109" gives ZERO coverage of the accessor logic / ABI repair; only the CI on-target matrix compiles it. **How to apply:** don't accept the host-suite count as evidence for changes to ss_hal.c.

- **Two ss_hal headers are not self-contained** (masked by umbrella include order): `ss_hal_haptic.h` uses `size_t` with no `#include <stddef.h>`; `ss_hal_usb.h` uses `uint32_t` with no `#include <stdint.h>`. Both fail `g++ -fsyntax-only` standalone. `ss_hal_muxctl.h` legitimately needs a freertos mock (TickType_t). All other 21 headers pass standalone C++.

- **SS_BOARD_CAPS is 64-bit safe on all three boards**: composed by OR-ing `SS_CAP_*` (each `1ULL<<n`) plus `0` for empty groups; `(uint64_t)SS_BOARD_CAPS` cast in ss_hal.c is sound. No board currently sets a bit >31.

- **`ss_hal_has_cap` is all-bits-AND** (`(mask & flag)==flag`): `has_cap(0)==true`; `has_cap(A|B)` requires BOTH. All current call sites (ss_touch.c:186, ss_display.cpp:162/219/252) pass a single flag — safe. Footgun for future OR'd callers who expect any-of.
