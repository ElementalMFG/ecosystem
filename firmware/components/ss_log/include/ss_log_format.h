// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_log_format.h — pure, ESP-IDF-free formatter core for ss_log.
//
// This is the redaction guarantee (NF-SEC-03, 05_SECURITY_MODEL.md). It is kept
// dependency-free (C11, freestanding-friendly: only <stdarg.h> <stddef.h>
// <stdint.h> <stdio.h> <string.h>) so it can be audited in isolation and reused
// by a host test harness with no firmware toolchain.
//
// SECURITY CONTRACT for the custom `%k` conversion:
//   (1) `%k` marks key / sensitive material. It consumes exactly ONE
//       `const void*` vararg, NEVER dereferences it, and ALWAYS emits the fixed
//       string "[REDACTED]" — at every level, in every build type. No length,
//       digest, hash, or any data-derived output is ever produced.
//   (2) Rationale: diagnostics must never leak key material. A data-independent
//       placeholder is the only output that cannot become a side channel.

#pragma once

#include <stdarg.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Format `fmt` into `out`, handling the custom `%k` conversion (see the security
// contract above) and delegating every STANDARD conversion to snprintf, one
// conversion at a time.
//
// Supported standard conversions: d i u o x X c s p f F e E g G, with flags
// (- + space 0 #), width (including `*`), precision (including `.*`) and length
// modifiers (h hh l ll z t j). `%%` copies a literal '%'.
//
// Fail-closed behavior (documented, deliberate):
//   - `%n` is rejected: emits "[!fmt]" and STOPS (no write-back primitive is
//     ever honored in a log formatter).
//   - An unknown conversion `%<c>` emits "[!fmt:<c>]" and STOPS processing.
//     Continuing would require guessing the arg width and risk misreading the
//     rest of the va_list, so we stop rather than guess.
//   - A malformed / over-long spec emits "[!fmt]" and STOPS.
//   After a STOP no further varargs are consumed.
//
// pre:  `out` points to at least `out_size` bytes (or out_size == 0); `fmt` is
//       non-null; `args` holds the arguments matching `fmt`.
// post: `out` is always NUL-terminated when out_size > 0. Output is truncated
//       (never overflowed) to fit `out_size`, exactly like snprintf.
// error: no error return; malformed specs are handled fail-closed as above.
// return: the number of characters that WOULD have been written had `out_size`
//         been unbounded (excluding the NUL), matching snprintf semantics.
int ss_log_vformat(char* out, size_t out_size, const char* fmt, va_list args);

#ifdef __cplusplus
}
#endif
