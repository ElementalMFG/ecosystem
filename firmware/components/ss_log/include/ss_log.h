// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_log.h — leveled, colorized console logging with key-material redaction.
//
// SECURITY CONTRACT (the reason this component exists — NF-SEC-03,
// 05_SECURITY_MODEL.md privacy posture):
//   The custom `%k` conversion marks key / sensitive material. It consumes
//   exactly ONE `const void*` vararg, NEVER dereferences it, and ALWAYS emits
//   the fixed string "[REDACTED]" — at every level (E/W/I/D), in every build
//   type (debug or release). It never emits a length, digest, hash, or any
//   other data-derived output, because any data-derived output could become a
//   side channel. Diagnostics must never leak key material; a data-independent
//   placeholder is the only safe rendering.
//
//   Usage: SS_LOGI("net", "session=%k peer=%s", session_key, peer_name);
//                                    ^ pass the pointer to the key; it is
//                                      discarded and rendered as [REDACTED].
//
// The formatter core (ss_log_format.c) implements this guarantee and is kept
// ESP-IDF-free so it can be audited and host-tested in isolation.

#pragma once

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

// Log severity. Lower value == higher severity. The runtime filter
// (ss_log_set_level) admits every level whose value is <= the configured level,
// so the default (SS_LOG_INFO) admits E/W/I and suppresses D. SS_LOGD call
// sites are always compiled in; they are merely filtered out at runtime.
typedef enum {
    SS_LOG_ERROR = 0,
    SS_LOG_WARN = 1,
    SS_LOG_INFO = 2,
    SS_LOG_DEBUG = 3,
} ss_log_level_t;

// Emit one log line. printf-like, plus the custom `%k` conversion (see the
// security contract above).
//
// NOTE: this function is intentionally declared WITHOUT a
// __attribute__((format(printf, ...))) attribute. The custom `%k` conversion is
// not part of the standard printf grammar, so the compiler's -Wformat checker
// would reject every `%k` call site as an invalid format string. The formatter
// core validates specs itself and fails closed on anything it does not
// recognize, so the compile-time attribute would cost more than it buys.
//
// pre:  `tag` and `fmt` are non-null NUL-terminated strings; varargs match
//       `fmt` (with `%k` taking a `const void*`).
// post: if `level` passes the runtime filter, a colorized, timestamped, tagged
//       line is written to stdout; otherwise nothing is emitted.
// error: none; malformed format specs are rendered fail-closed by the core.
void ss_log_write(ss_log_level_t level, const char* tag, const char* fmt, ...);

// Set the runtime severity filter (default SS_LOG_INFO). Levels with a value
// greater than `level` are suppressed.
//
// pre:  `level` is a valid ss_log_level_t.
// post: subsequent ss_log_write calls are filtered against `level`.
void ss_log_set_level(ss_log_level_t level);

#define SS_LOGE(tag, fmt, ...) ss_log_write(SS_LOG_ERROR, (tag), (fmt), ##__VA_ARGS__)
#define SS_LOGW(tag, fmt, ...) ss_log_write(SS_LOG_WARN, (tag), (fmt), ##__VA_ARGS__)
#define SS_LOGI(tag, fmt, ...) ss_log_write(SS_LOG_INFO, (tag), (fmt), ##__VA_ARGS__)
#define SS_LOGD(tag, fmt, ...) ss_log_write(SS_LOG_DEBUG, (tag), (fmt), ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif
