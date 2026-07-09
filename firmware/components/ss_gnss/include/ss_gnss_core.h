// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_gnss_core.h — pure, host-testable NMEA-0183 parse core for the ss_gnss HAL.
//
// This layer holds NO ESP-IDF runtime dependency: it only reasons about the
// frozen fix type (ss_hal_gnss.h -> ss_gnss_fix_t) so it can be exercised with
// host gtest. The IDF glue (ss_gnss.c) owns the UART, mutex, tap and stats.
//
// Part-agnostic: the parser accepts any NMEA-0183 talker ($GP/$GN/$BD/…);
// no BN-880-specific assumptions are made.

#pragma once
#include <stdbool.h>
#include <stddef.h>

#include "ss_hal_gnss.h" // ss_gnss_fix_t (plain struct; no HAL impl required)

#ifdef __cplusplus
extern "C" {
#endif

// Outcome of parsing one NMEA sentence, so the glue can bump the right counter.
typedef enum {
    SS_GNSS_PARSE_CHECKSUM_FAIL, // "$..*hh" XOR mismatch / malformed frame
    SS_GNSS_PARSE_NO_UPDATE,     // well-formed but not an RMC/GGA we consume
    SS_GNSS_PARSE_RMC,           // RMC applied (has_fix/lat/lon/speed/course)
    SS_GNSS_PARSE_GGA,           // GGA applied (sats_used/hdop/alt)
} ss_gnss_parse_result_t;

// Verify an "$....*hh" NMEA sentence checksum (XOR of bytes between '$' and
// '*', compared to the two hex digits after '*').
// pre:  `s` points at `len` readable bytes.
// post: returns true iff the frame is well-formed and the checksum matches.
bool ss_gnss_core_checksum_ok(const char* s, size_t len);

// Convert an NMEA "ddmm.mmmm" coordinate field + hemisphere char to signed
// decimal degrees (S/W negative). Returns 0.0 for a NULL/empty field.
double ss_gnss_core_coord(const char* f, char dir);

// Parse one NMEA sentence and, for RMC/GGA, update *fix in place.
// pre:  `line` is a NUL-terminated, writable buffer of `len` chars; `fix` non-NULL.
// post: on RMC/GGA the corresponding fields of *fix are written and the RMC/GGA
//       result is returned; on checksum failure *fix is untouched and
//       CHECKSUM_FAIL is returned. `line` MAY be mutated in place (the trailing
//       "*hh" is stripped and commas are replaced with NUL for field splitting).
ss_gnss_parse_result_t ss_gnss_core_parse(char* line, size_t len, ss_gnss_fix_t* fix);

#ifdef __cplusplus
} // extern "C"
#endif
