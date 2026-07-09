// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_gnss_core.c — pure NMEA-0183 parse core for the ss_gnss HAL. No ESP-IDF
// runtime calls: this file must stay host-linkable for gtest.

#include "ss_gnss_core.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

bool ss_gnss_core_checksum_ok(const char* s, size_t len)
{
    if (len < 9 || s[0] != '$') return false;
    const char* star = (const char*)memchr(s, '*', len);
    if (!star || (size_t)(star - s) + 3 > len) return false;
    uint8_t x = 0;
    for (const char* p = s + 1; p < star; ++p) x ^= (uint8_t)(*p);
    return strtoul(star + 1, NULL, 16) == x;
}

double ss_gnss_core_coord(const char* f, char dir)
{
    if (!f || !*f) return 0.0;
    const double v = atof(f);
    const double deg = floor(v / 100.0);
    double out = deg + (v - deg * 100.0) / 60.0;
    if (dir == 'S' || dir == 'W') out = -out;
    return out;
}

// Split a sentence into comma fields in place. Returns field count.
static size_t split_fields(char* s, const char* fields[], size_t max)
{
    size_t n = 0;
    for (char* p = s; p && n < max;) {
        fields[n++] = p;
        p = strchr(p, ',');
        if (p) *p++ = '\0';
    }
    return n;
}

ss_gnss_parse_result_t ss_gnss_core_parse(char* line, size_t len, ss_gnss_fix_t* fix)
{
    if (!ss_gnss_core_checksum_ok(line, len)) return SS_GNSS_PARSE_CHECKSUM_FAIL;

    // Strip "*hh" so field splitting is clean.
    char* star = strchr(line, '*');
    if (star) *star = '\0';

    const char* f[24];
    const size_t n = split_fields(line, f, 24);
    if (n < 2) return SS_GNSS_PARSE_NO_UPDATE;
    const char* type = f[0] + 3; // skip "$GP"/"$GN"/…

    if (strncmp(type, "RMC", 3) == 0 && n >= 10) {
        // $..RMC,time,status,lat,NS,lon,EW,sog_kn,cog,date,...
        fix->has_fix = (f[2][0] == 'A');
        fix->lat_deg = ss_gnss_core_coord(f[3], f[4][0]);
        fix->lon_deg = ss_gnss_core_coord(f[5], f[6][0]);
        fix->speed_mps = (float)(atof(f[7]) * 0.514444);
        fix->course_deg = (float)atof(f[8]);
        return SS_GNSS_PARSE_RMC;
    }
    if (strncmp(type, "GGA", 3) == 0 && n >= 10) {
        // $..GGA,time,lat,NS,lon,EW,quality,sats,hdop,alt,...
        fix->sats_used = (uint8_t)atoi(f[7]);
        fix->hdop = (float)atof(f[8]);
        fix->alt_m = (float)atof(f[9]);
        return SS_GNSS_PARSE_GGA;
    }
    return SS_GNSS_PARSE_NO_UPDATE;
}
