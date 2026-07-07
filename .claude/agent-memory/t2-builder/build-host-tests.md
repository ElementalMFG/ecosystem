<!-- SPDX-License-Identifier: Apache-2.0 -->
---
name: build-host-tests
description: gcc host-test harness gotchas in this repo (-Werror traps, staged compiled binary, sandbox tool block)
metadata:
  type: feedback
---

Writing gcc host tests (`firmware/components/*/test/host/` and `firmware/test/host/`) with `-std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined`:

- A deliberate small-buffer `snprintf` (e.g. to build a truncation oracle) trips `-Werror=format-truncation`. **How to apply:** compute the would-be length / expected content from a LARGE buffer and derive the truncated string with `memcpy`, never call snprintf into a knowingly-too-small buffer.
- `intmax_t`/`uintmax_t` need `<stdint.h>`, `ptrdiff_t`/`size_t` need `<stddef.h>` in the test .c even if the header pulls them in.
- `git add -A` after `make test` stages the compiled test binary. **How to apply:** add a `.gitignore` in the test/host dir listing the binary; the T2 task says don't commit, but don't leave the artifact staged either.
- `#pragma GCC diagnostic push/pop` are LEXICAL, not runtime — exactly one `pop` per `push` in source text regardless of `goto`/branch flow. Multiple pops for one push = compile error.
- **SANDBOX BLOCKS BUILD TOOLS:** in the agent bash sandbox `gcc`, `cmake`, `ctest`, and `make` all return "This command requires approval" (even with `dangerouslyDisableSandbox=true`) — the `firmware/test/host` cmake build also needs network for googletest FetchContent. **How to apply:** you often cannot run `make lite` or ctest yourself; fall back to strict static review — grep the header for the exact macro/type names, confirm enum `switch` covers all cases, cast every CONFIG int / `size_t` at printf and function-arg boundaries, and report the gate as "not run — sandbox-blocked" rather than claiming green.
- **Why:** the -Werror items cost a rebuild cycle to discover. The firmware CI (`make lite`) compiles pure-core .c under stricter `-Wshadow -Wconversion` than the host cmake (ss_units has no extra flags), so the core is where conversion warnings actually bite. See [[pattern-pure-core-component]].
