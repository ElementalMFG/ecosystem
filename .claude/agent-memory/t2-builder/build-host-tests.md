<!-- SPDX-License-Identifier: Apache-2.0 -->
---
name: build-host-tests
description: gcc host-test harness gotchas in this repo (-Werror traps, staged compiled binary)
metadata:
  type: feedback
---

Writing gcc host tests (`firmware/components/*/test/host/`) with `-std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined`:

- A deliberate small-buffer `snprintf` (e.g. to build a truncation oracle) trips `-Werror=format-truncation`. **How to apply:** compute the would-be length / expected content from a LARGE buffer and derive the truncated string with `memcpy`, never call snprintf into a knowingly-too-small buffer.
- `intmax_t`/`uintmax_t` need `<stdint.h>`, `ptrdiff_t`/`size_t` need `<stddef.h>` in the test .c even if the header pulls them in.
- `git add -A` after `make test` stages the compiled test binary. **How to apply:** add a `.gitignore` in the test/host dir listing the binary; the T2 task says don't commit, but don't leave the artifact staged either.
- `#pragma GCC diagnostic push/pop` are LEXICAL, not runtime — exactly one `pop` per `push` in source text regardless of `goto`/branch flow. Multiple pops for one push = compile error.
- **Why:** these all cost a rebuild cycle to discover. The firmware CI also compiles pure-core .c under stricter `-Wshadow -Wconversion` than the host Makefile, so syntax-check the core with those flags (`gcc -fsyntax-only`) before reporting green. See [[pattern-pure-core-component]].
