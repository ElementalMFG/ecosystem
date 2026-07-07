<!-- SPDX-License-Identifier: Apache-2.0 -->
---
name: workflow-cross-review
description: How SS-SP T1 cross-review work is structured and verified
metadata:
  type: project
---

- T1 firmware stories split into a **pure, host-tested decision core** (IDF-free `.c`, gtest under `firmware/test/host/`) plus **IDF glue** (`.cpp`, not host-buildable). Review the core via the host tests (`cmake -S firmware/test/host -B <build> && ctest`), and review the glue by reasoning against IDF semantics.
- Hardware-dependent AC clauses are **honestly parked at Status=IN_REVIEW** pending on-target frameworks (S-02-014 host / S-02-015 Unity). Don't ding a story for lacking on-hardware proof if the AC/meta explicitly defers it — but DO ding claims of verification the artifact can't back.
- Pinned toolchain: ESP-IDF **v5.3.5** (`ci/containers/firmware/Dockerfile`, sha256-pinned). Verify IDF-version-specific claims (enums, Kconfig, tool defaults) against that version; `esp-coredump` is pip-fetchable for CLI verification (`pip download esp-coredump`).
