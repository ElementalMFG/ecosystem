// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// Host tests for the S-02-017 versioned-NVS decision core
// (firmware/main/ss_nvs_core.c): the shared name validators and the migration
// planner. Only the pure core is host-testable; the nvs_flash glue in
// ss_nvs.cpp is exercised on target. The contract lives in ss_nvs.h.

#include <string>

#include <gtest/gtest.h>

extern "C" {
#include "ss_nvs.h"
}

namespace
{

// ---- Migration planner -----------------------------------------------------

TEST(NvsPlan, SameVersionIsNone)
{
    ss_nvs_plan_t p = ss_nvs_plan_migration(true, 3u, 3u);
    EXPECT_EQ(p.verdict, SS_NVS_MIG_NONE);
    EXPECT_EQ(p.from_ver, 0u); // not meaningful
    EXPECT_EQ(p.to_ver, 0u);   // not meaningful
}

TEST(NvsPlan, NoStoredIsFreshToCode)
{
    ss_nvs_plan_t p = ss_nvs_plan_migration(false, 0u, 5u);
    EXPECT_EQ(p.verdict, SS_NVS_MIG_FRESH);
    EXPECT_EQ(p.from_ver, 0u); // not meaningful for FRESH
    EXPECT_EQ(p.to_ver, 5u);   // stamp the code version
}

TEST(NvsPlan, NoStoredIgnoresStoredArg)
{
    // has_stored == false means the "__ver" key is absent; stored_ver is
    // meaningless and must not affect the verdict.
    ss_nvs_plan_t p = ss_nvs_plan_migration(false, 99u, 2u);
    EXPECT_EQ(p.verdict, SS_NVS_MIG_FRESH);
    EXPECT_EQ(p.to_ver, 2u);
}

TEST(NvsPlan, StoredBelowCodeIsUpgrade)
{
    ss_nvs_plan_t p = ss_nvs_plan_migration(true, 2u, 7u);
    EXPECT_EQ(p.verdict, SS_NVS_MIG_UPGRADE);
    EXPECT_EQ(p.from_ver, 2u); // == stored
    EXPECT_EQ(p.to_ver, 7u);   // == code
}

TEST(NvsPlan, LegacyZeroStoredIsUpgrade)
{
    // A corrupt/legacy stored value of 0 migrates like any older version.
    ss_nvs_plan_t p = ss_nvs_plan_migration(true, 0u, 1u);
    EXPECT_EQ(p.verdict, SS_NVS_MIG_UPGRADE);
    EXPECT_EQ(p.from_ver, 0u);
    EXPECT_EQ(p.to_ver, 1u);
}

TEST(NvsPlan, StoredAboveCodeIsDowngradeRefuse)
{
    ss_nvs_plan_t p = ss_nvs_plan_migration(true, 9u, 4u);
    EXPECT_EQ(p.verdict, SS_NVS_MIG_DOWNGRADE_REFUSE);
    EXPECT_EQ(p.from_ver, 0u); // not meaningful
    EXPECT_EQ(p.to_ver, 0u);   // not meaningful
}

// ---- Name validators (identical rule for namespaces and keys) --------------

TEST(NvsValidators, TypicalNamesAccepted)
{
    for (const char* n : {"radio", "ui", "compass_cfg", "a", "net.v2", "K3y-9"}) {
        EXPECT_TRUE(ss_nvs_ns_valid(n)) << n;
        EXPECT_TRUE(ss_nvs_key_valid(n)) << n;
    }
}

TEST(NvsValidators, NullRejected)
{
    EXPECT_FALSE(ss_nvs_ns_valid(nullptr));
    EXPECT_FALSE(ss_nvs_key_valid(nullptr));
}

TEST(NvsValidators, EmptyRejected)
{
    EXPECT_FALSE(ss_nvs_ns_valid(""));
    EXPECT_FALSE(ss_nvs_key_valid(""));
}

TEST(NvsValidators, MaxLengthBoundary)
{
    const std::string ok(SS_NVS_NAME_MAX, 'x');           // 15 bytes
    const std::string too_long(SS_NVS_NAME_MAX + 1, 'x'); // 16 bytes
    EXPECT_EQ(ok.size(), 15u);
    EXPECT_EQ(too_long.size(), 16u);
    EXPECT_TRUE(ss_nvs_ns_valid(ok.c_str()));
    EXPECT_TRUE(ss_nvs_key_valid(ok.c_str()));
    EXPECT_FALSE(ss_nvs_ns_valid(too_long.c_str()));
    EXPECT_FALSE(ss_nvs_key_valid(too_long.c_str()));
}

TEST(NvsValidators, ReservedPrefixRejected)
{
    for (const char* n : {"__", "__ver", "__x", "__anything"}) {
        EXPECT_FALSE(ss_nvs_ns_valid(n)) << n;
        EXPECT_FALSE(ss_nvs_key_valid(n)) << n;
    }
    // A single leading underscore is fine (only the "__" prefix is reserved).
    EXPECT_TRUE(ss_nvs_ns_valid("_ver"));
    EXPECT_TRUE(ss_nvs_key_valid("_ver"));
}

TEST(NvsValidators, VerKeyRejectedAsUserKey)
{
    // The literal reserved key must never be usable as a user key/namespace.
    EXPECT_FALSE(ss_nvs_key_valid(SS_NVS_VER_KEY));
    EXPECT_FALSE(ss_nvs_ns_valid(SS_NVS_VER_KEY));
}

TEST(NvsValidators, EmbeddedSpaceRejected)
{
    EXPECT_FALSE(ss_nvs_ns_valid("a b"));
    EXPECT_FALSE(ss_nvs_key_valid("a b"));
    EXPECT_FALSE(ss_nvs_ns_valid(" lead"));   // leading space (0x20)
    EXPECT_FALSE(ss_nvs_key_valid("trail ")); // trailing space
}

TEST(NvsValidators, NonPrintableRejected)
{
    const char tab[] = {'a', '\t', 'b', '\0'};
    const char del[] = {'a', 0x7f, '\0'};        // 0x7F is above the 0x7E ceiling
    const char high[] = {'a', (char)0x80, '\0'}; // non-ASCII high byte
    const char ctrl[] = {0x01, 'a', '\0'};
    EXPECT_FALSE(ss_nvs_ns_valid(tab));
    EXPECT_FALSE(ss_nvs_ns_valid(del));
    EXPECT_FALSE(ss_nvs_ns_valid(high));
    EXPECT_FALSE(ss_nvs_ns_valid(ctrl));
    EXPECT_FALSE(ss_nvs_key_valid(tab));
    EXPECT_FALSE(ss_nvs_key_valid(del));
    EXPECT_FALSE(ss_nvs_key_valid(high));
    EXPECT_FALSE(ss_nvs_key_valid(ctrl));
}

TEST(NvsValidators, PrintableAsciiBoundaries)
{
    const char bang[] = {0x21, '\0'};  // '!' — lowest legal byte
    const char tilde[] = {0x7e, '\0'}; // '~' — highest legal byte
    EXPECT_TRUE(ss_nvs_ns_valid(bang));
    EXPECT_TRUE(ss_nvs_ns_valid(tilde));
    EXPECT_TRUE(ss_nvs_key_valid(bang));
    EXPECT_TRUE(ss_nvs_key_valid(tilde));
}

} // namespace
