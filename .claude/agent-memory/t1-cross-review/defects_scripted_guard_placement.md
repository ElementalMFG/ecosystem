<!-- SPDX-License-Identifier: Apache-2.0 -->
---
name: defects-scripted-guard-placement
description: Recurring defect class — scripted extern "C"/ifdef guard insertion landing at the wrong point in a header
metadata:
  type: project
---

Scripted/bulk insertion of `#ifdef __cplusplus ... extern "C" {` guards is prone to landing at the wrong point. Check every touched header for:

- **Opening brace placed before a late `#include`** (wraps that header + its transitive includes in `extern "C"`). Often still compiles when the wrapped header is already-included above (idempotent no-op) or is pure-C, which hides the defect from a first-pass review — verify placement, not just that it builds. Seen in `ss_hal.h` (S-03-032): brace opened before 6 subsystem includes. Correct placement is AFTER all `#include`s.
- **Double-wrapping**: umbrella opens `extern "C"` around headers that already self-guard → nested linkage spec (legal, but a smell that the edit was mechanical, not reasoned).
- **Guard opened inside an `#if`/`#ifdef` block** so open/close land in different conditional arms.
- **C++-only constructs** (templates, namespaces, `<cstdint>`) now inside `extern "C"`.

**How to apply:** run `g++ -std=c++17 -fsyntax-only` on EVERY touched header standalone (mocks under `firmware/test/host/mocks`), then compile the umbrella as both C and C++ with freertos stubbed. Passing builds do not prove correct guard placement — read the diff.
