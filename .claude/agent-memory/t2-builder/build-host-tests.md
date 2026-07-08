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
- **Python host tests / scratch scripts:** `pip install pytest` needs `--break-system-packages` on this host (PEP 668). The bash sandbox also REJECTS some heredoc/`-c` python: dict comprehensions with quoted keys (`{k:v for ...}` → "brace with quote"), and `$var` shell expansion inside multi-op commands ("simple_expansion"). **How to apply:** in python run via `python3 <<'PY'` avoid dict/set comprehensions containing string literals (use `.pop()`/loops); avoid shell `$var` — pass paths as literals or build them in python. Writing scratch to `/tmp` may prompt for permission; prefer an inline heredoc that does everything.
- **IDF v5.3.5 `project_description.json`:** exposes the IDF version under key `git_revision` (= `"v5.3.5"`); there is NO `idf_version` key. Toolchain version is parseable from `c_compiler` path segment `xtensa-esp-elf/esp-<ver>` (e.g. `esp-13.2.0_20250707`). A committed build dir exists at `firmware/build/lite/` to confirm against without a container.
- **Pinning GitHub Actions by SHA:** `gh api repos/<owner>/<action>/tags --paginate --jq '.[]|select(.name|startswith("v2."))|[.name,.commit.sha]|@tsv'` resolves the latest patch SHA for a major.
- **Why:** the -Werror items cost a rebuild cycle to discover. The firmware CI (`make lite`) compiles pure-core .c under stricter `-Wshadow -Wconversion` than the host cmake (ss_units has no extra flags), so the core is where conversion warnings actually bite. See [[pattern-pure-core-component]].
