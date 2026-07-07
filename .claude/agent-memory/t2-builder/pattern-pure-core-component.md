<!-- SPDX-License-Identifier: Apache-2.0 -->
---
name: pattern-pure-core-component
description: location of the IDF-free pure-core + host-tested component template (ss_log)
metadata:
  type: reference
---

`firmware/components/ss_log/` is the reference pattern for an "auditable pure core + IDF sink + host tests" component:

- `src/ss_log_format.c` + `include/ss_log_format.h`: pure C11 core, only `<stdarg/stddef/stdint/stdio/string.h>`, no ESP-IDF — so it builds under plain gcc.
- `src/ss_log.c`: thin IDF sink layer (needs esp_timer etc.).
- `test/host/{Makefile,test_ss_log_format.c,.gitignore}`: no-Unity harness, tiny CHECK macro, ASan/UBSan, differential-vs-`vsnprintf` oracle for standard behavior.
- `.github/workflows/host-tests.yml`: runs `make -C .../test/host test` + a grep step for evidence that can't be unit-tested (e.g. ANSI color codes in the sink).

**How to apply:** when a component has a security/format invariant that must be provable off-target, split the invariant into an IDF-free core and copy this layout. Wire the component into the 3-board build via `firmware/main/CMakeLists.txt` REQUIRES so CI actually compiles it. See [[build-host-tests]].
