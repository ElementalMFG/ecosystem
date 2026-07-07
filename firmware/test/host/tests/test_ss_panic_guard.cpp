// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// Host tests for the S-02-008 crash-loop-breaker decision core
// (firmware/main/ss_panic_guard_core.c). This is the audited pure logic that
// decides normal-vs-safe-mode boot; the contract lives in ss_panic_guard.h.

#include <gtest/gtest.h>

extern "C" {
#include "ss_panic_guard.h"
}

namespace
{

ss_panic_record_t garbage_record()
{
    ss_panic_record_t rec;
    rec.magic = 0xDEADBEEFu;
    rec.count = 0xA5A5A5A5u;
    rec.inverse = 0x12345678u;
    return rec;
}

ss_panic_record_t valid_record(uint32_t count)
{
    ss_panic_record_t rec;
    rec.magic = SS_PANIC_RECORD_MAGIC;
    rec.count = count;
    rec.inverse = ~count;
    return rec;
}

void expect_normalized(const ss_panic_record_t& rec, uint32_t count)
{
    EXPECT_EQ(rec.magic, SS_PANIC_RECORD_MAGIC);
    EXPECT_EQ(rec.count, count);
    EXPECT_EQ(rec.inverse, ~count);
}

TEST(PanicGuard, ColdBootGarbageIsFreshAndNormal)
{
    ss_panic_record_t rec = garbage_record();
    EXPECT_EQ(ss_panic_guard_decide(SS_RESET_CLASS_BENIGN, &rec), SS_BOOT_NORMAL);
    expect_normalized(rec, 0u);
}

TEST(PanicGuard, GarbageWithCrashCountsFromZero)
{
    // Cold-boot garbage followed by a crash-classified reset must count 1,
    // not trust the garbage count.
    ss_panic_record_t rec = garbage_record();
    EXPECT_EQ(ss_panic_guard_decide(SS_RESET_CLASS_CRASH, &rec), SS_BOOT_NORMAL);
    expect_normalized(rec, 1u);
}

TEST(PanicGuard, BadInverseShadowIsRejected)
{
    ss_panic_record_t rec = valid_record(2u);
    rec.inverse = ~3u; // shadow disagrees with count -> invalid
    EXPECT_EQ(ss_panic_guard_decide(SS_RESET_CLASS_CRASH, &rec), SS_BOOT_NORMAL);
    expect_normalized(rec, 1u);
}

TEST(PanicGuard, WrongMagicWithCorrectInverseIsRejected)
{
    // Closes the record_valid 2x2 matrix: inverse consistent, magic wrong.
    ss_panic_record_t rec = valid_record(2u);
    rec.magic = 0xBAD0BAD0u;
    EXPECT_EQ(ss_panic_guard_decide(SS_RESET_CLASS_CRASH, &rec), SS_BOOT_NORMAL);
    expect_normalized(rec, 1u);
}

TEST(PanicGuard, ThirdConsecutivePanicEntersSafeMode)
{
    ss_panic_record_t rec = garbage_record();
    EXPECT_EQ(ss_panic_guard_decide(SS_RESET_CLASS_CRASH, &rec), SS_BOOT_NORMAL);    // 1
    EXPECT_EQ(ss_panic_guard_decide(SS_RESET_CLASS_CRASH, &rec), SS_BOOT_NORMAL);    // 2
    EXPECT_EQ(ss_panic_guard_decide(SS_RESET_CLASS_CRASH, &rec), SS_BOOT_SAFE_MODE); // 3
    expect_normalized(rec, 3u);
}

TEST(PanicGuard, SafeModeStickyWhilePanicsContinue)
{
    ss_panic_record_t rec = valid_record(3u);
    EXPECT_EQ(ss_panic_guard_decide(SS_RESET_CLASS_CRASH, &rec), SS_BOOT_SAFE_MODE);
    expect_normalized(rec, 4u);
    EXPECT_EQ(ss_panic_guard_decide(SS_RESET_CLASS_CRASH, &rec), SS_BOOT_SAFE_MODE);
    expect_normalized(rec, 5u);
}

TEST(PanicGuard, BenignResetClearsCount)
{
    ss_panic_record_t rec = valid_record(2u);
    EXPECT_EQ(ss_panic_guard_decide(SS_RESET_CLASS_BENIGN, &rec), SS_BOOT_NORMAL);
    expect_normalized(rec, 0u);

    // Even from safe-mode territory: a power-on / sw reset re-arms normally.
    rec = valid_record(7u);
    EXPECT_EQ(ss_panic_guard_decide(SS_RESET_CLASS_BENIGN, &rec), SS_BOOT_NORMAL);
    expect_normalized(rec, 0u);
}

TEST(PanicGuard, IndeterminateResetPreservesCount)
{
    // Fail-safe: unknown reset reasons neither count as a crash nor erase
    // accumulated evidence of one.
    ss_panic_record_t rec = valid_record(2u);
    EXPECT_EQ(ss_panic_guard_decide(SS_RESET_CLASS_INDETERMINATE, &rec), SS_BOOT_NORMAL);
    expect_normalized(rec, 2u); // preserved, not cleared, not incremented

    // At/above threshold an indeterminate reset keeps the device in safe mode.
    rec = valid_record(3u);
    EXPECT_EQ(ss_panic_guard_decide(SS_RESET_CLASS_INDETERMINATE, &rec), SS_BOOT_SAFE_MODE);
    expect_normalized(rec, 3u);
}

TEST(PanicGuard, IndeterminateWithGarbageIsFresh)
{
    ss_panic_record_t rec = garbage_record();
    EXPECT_EQ(ss_panic_guard_decide(SS_RESET_CLASS_INDETERMINATE, &rec), SS_BOOT_NORMAL);
    expect_normalized(rec, 0u);
}

TEST(PanicGuard, RecordClearNormalizesAnything)
{
    ss_panic_record_t rec = garbage_record();
    ss_panic_guard_record_clear(&rec);
    expect_normalized(rec, 0u);

    rec = valid_record(9u);
    ss_panic_guard_record_clear(&rec);
    expect_normalized(rec, 0u);
}

TEST(PanicGuard, CountSaturatesWithoutWrap)
{
    ss_panic_record_t rec = valid_record(UINT32_MAX);
    EXPECT_EQ(ss_panic_guard_decide(SS_RESET_CLASS_CRASH, &rec), SS_BOOT_SAFE_MODE);
    expect_normalized(rec, UINT32_MAX); // saturated, no wrap to 0
}

TEST(PanicGuard, ThresholdIsExactlyThree)
{
    EXPECT_EQ(SS_PANIC_SAFE_MODE_THRESHOLD, 3u);
    ss_panic_record_t rec = valid_record(1u);
    EXPECT_EQ(ss_panic_guard_decide(SS_RESET_CLASS_CRASH, &rec), SS_BOOT_NORMAL);    // 2 < 3
    EXPECT_EQ(ss_panic_guard_decide(SS_RESET_CLASS_CRASH, &rec), SS_BOOT_SAFE_MODE); // 3 >= 3
}

} // namespace
