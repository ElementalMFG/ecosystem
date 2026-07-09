// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// test_ss_gnss_core.cpp — host gtest for the pure NMEA-0183 parse core.

#include <cmath>
#include <cstring>

#include "gtest/gtest.h"

extern "C" {
#include "ss_gnss_core.h"
}

namespace
{

// Parse a NUL-terminated NMEA string (core may mutate it, so copy into a buf).
ss_gnss_parse_result_t parse(const char* s, ss_gnss_fix_t* fix)
{
    char buf[128];
    const size_t len = std::strlen(s);
    std::memcpy(buf, s, len + 1);
    return ss_gnss_core_parse(buf, len, fix);
}

TEST(GnssCore, RmcFixNorthEast)
{
    ss_gnss_fix_t fix{};
    const char* s = "$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A";
    EXPECT_EQ(parse(s, &fix), SS_GNSS_PARSE_RMC);
    EXPECT_TRUE(fix.has_fix);
    EXPECT_NEAR(fix.lat_deg, 48.1173, 1e-4);   // 48 + 07.038/60, N -> +
    EXPECT_NEAR(fix.lon_deg, 11.516667, 1e-4); // 11 + 31.000/60, E -> +
    EXPECT_NEAR(fix.speed_mps, 22.4 * 0.514444, 1e-3);
    EXPECT_NEAR(fix.course_deg, 84.4, 1e-3);
}

TEST(GnssCore, RmcSignFlipSouthWest)
{
    ss_gnss_fix_t fix{};
    const char* s = "$GPRMC,123519,A,4807.038,S,01131.000,W,022.4,084.4,230394,003.1,W*65";
    EXPECT_EQ(parse(s, &fix), SS_GNSS_PARSE_RMC);
    EXPECT_LT(fix.lat_deg, 0.0); // S -> negative
    EXPECT_LT(fix.lon_deg, 0.0); // W -> negative
    EXPECT_NEAR(fix.lat_deg, -48.1173, 1e-4);
    EXPECT_NEAR(fix.lon_deg, -11.516667, 1e-4);
}

TEST(GnssCore, GgaUpdatesSatsHdopAlt)
{
    ss_gnss_fix_t fix{};
    const char* s = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47";
    EXPECT_EQ(parse(s, &fix), SS_GNSS_PARSE_GGA);
    EXPECT_EQ(fix.sats_used, 8);
    EXPECT_NEAR(fix.hdop, 0.9, 1e-4);
    EXPECT_NEAR(fix.alt_m, 545.4, 1e-3);
}

TEST(GnssCore, BadChecksumRejectedFixUnchanged)
{
    ss_gnss_fix_t fix{};
    fix.lat_deg = 12.34;
    fix.has_fix = false;
    // Correct XOR is *6A; *00 is wrong.
    const char* s = "$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*00";
    EXPECT_EQ(parse(s, &fix), SS_GNSS_PARSE_CHECKSUM_FAIL);
    EXPECT_DOUBLE_EQ(fix.lat_deg, 12.34); // untouched
    EXPECT_FALSE(fix.has_fix);
}

TEST(GnssCore, ChecksumOkAcceptsValidXor)
{
    const char* good = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47";
    const char* bad = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*48";
    EXPECT_TRUE(ss_gnss_core_checksum_ok(good, std::strlen(good)));
    EXPECT_FALSE(ss_gnss_core_checksum_ok(bad, std::strlen(bad)));
}

TEST(GnssCore, TruncatedSentenceRejected)
{
    ss_gnss_fix_t fix{};
    // len < 9 -> checksum_ok false -> CHECKSUM_FAIL, fix untouched.
    const char* s = "$GPRMC";
    EXPECT_FALSE(ss_gnss_core_checksum_ok(s, std::strlen(s)));
    EXPECT_EQ(parse(s, &fix), SS_GNSS_PARSE_CHECKSUM_FAIL);
    EXPECT_FALSE(fix.has_fix);
}

TEST(GnssCore, CoordSignFlips)
{
    EXPECT_NEAR(ss_gnss_core_coord("4807.038", 'N'), 48.1173, 1e-4);
    EXPECT_NEAR(ss_gnss_core_coord("4807.038", 'S'), -48.1173, 1e-4);
    EXPECT_NEAR(ss_gnss_core_coord("01131.000", 'E'), 11.516667, 1e-4);
    EXPECT_NEAR(ss_gnss_core_coord("01131.000", 'W'), -11.516667, 1e-4);
    EXPECT_DOUBLE_EQ(ss_gnss_core_coord("", 'N'), 0.0);
    EXPECT_DOUBLE_EQ(ss_gnss_core_coord(nullptr, 'N'), 0.0);
}

} // namespace
