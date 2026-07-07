// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// Host tests for the S-02-016 recovery decision core
// (firmware/main/ss_recovery_core.c): the request flag integrity guard, the
// BOOT-button hold FSM, the rollback precondition verdict, and the
// factory-reset erase plan. The contract lives in ss_recovery.h.

#include <cstring>

#include <gtest/gtest.h>

extern "C" {
#include "ss_recovery.h"
}

namespace
{

// ---- Request flag ----------------------------------------------------------

ss_recovery_flag_t garbage_flag()
{
    ss_recovery_flag_t f;
    f.magic = 0xDEADBEEFu;
    f.inverse = 0x12345678u;
    return f;
}

TEST(RecoveryFlag, GarbageIsNotPending)
{
    ss_recovery_flag_t f = garbage_flag();
    EXPECT_FALSE(ss_recovery_flag_pending(&f));
}

TEST(RecoveryFlag, ValidHandCraftedIsPending)
{
    ss_recovery_flag_t f;
    f.magic = SS_RECOVERY_FLAG_MAGIC;
    f.inverse = ~SS_RECOVERY_FLAG_MAGIC;
    EXPECT_TRUE(ss_recovery_flag_pending(&f));
}

TEST(RecoveryFlag, GoodMagicBadShadowRejected)
{
    ss_recovery_flag_t f;
    f.magic = SS_RECOVERY_FLAG_MAGIC;
    f.inverse = 0u; // shadow disagrees
    EXPECT_FALSE(ss_recovery_flag_pending(&f));
}

TEST(RecoveryFlag, ArmClearRoundTrip)
{
    ss_recovery_flag_t f = garbage_flag();
    ss_recovery_flag_arm(&f);
    EXPECT_TRUE(ss_recovery_flag_pending(&f));
    EXPECT_EQ(f.magic, SS_RECOVERY_FLAG_MAGIC);
    EXPECT_EQ(f.inverse, ~f.magic);

    ss_recovery_flag_clear(&f);
    EXPECT_FALSE(ss_recovery_flag_pending(&f));

    // Re-arm after clear still works (one-shot re-use).
    ss_recovery_flag_arm(&f);
    EXPECT_TRUE(ss_recovery_flag_pending(&f));
}

// ---- BOOT-button hold FSM --------------------------------------------------

constexpr uint32_t kHold = 3000u;
constexpr uint32_t kWindow = 10000u;

ss_recovery_hold_t make_hold(uint32_t start = 0u, uint32_t window = kWindow, uint32_t hold = kHold)
{
    ss_recovery_hold_t h;
    ss_recovery_hold_init(&h, start, window, hold);
    return h;
}

TEST(RecoveryHold, InitIsIdle)
{
    ss_recovery_hold_t h = make_hold();
    EXPECT_EQ(h.state, SS_RECOVERY_HOLD_IDLE);
    EXPECT_EQ(h.window_deadline_ms, kWindow);
    EXPECT_EQ(h.hold_ms, kHold);
}

TEST(RecoveryHold, TriggersAfterExactlyHoldMs)
{
    ss_recovery_hold_t h = make_hold();
    EXPECT_EQ(ss_recovery_hold_step(&h, true, 0u), SS_RECOVERY_HOLD_ARMED);
    EXPECT_EQ(ss_recovery_hold_step(&h, true, kHold - 1u), SS_RECOVERY_HOLD_ARMED);
    EXPECT_EQ(ss_recovery_hold_step(&h, true, kHold), SS_RECOVERY_HOLD_TRIGGERED);
}

TEST(RecoveryHold, ReleaseResetsAccumulation)
{
    ss_recovery_hold_t h = make_hold();
    EXPECT_EQ(ss_recovery_hold_step(&h, true, 0u), SS_RECOVERY_HOLD_ARMED);
    EXPECT_EQ(ss_recovery_hold_step(&h, true, 1000u), SS_RECOVERY_HOLD_ARMED);
    // Release discards the accumulated hold.
    EXPECT_EQ(ss_recovery_hold_step(&h, false, 1500u), SS_RECOVERY_HOLD_IDLE);
    // Fresh press starts a new accumulation window.
    EXPECT_EQ(ss_recovery_hold_step(&h, true, 2000u), SS_RECOVERY_HOLD_ARMED);
    // 4000ms wall clock but only 2000ms of continuous press -> not yet.
    EXPECT_EQ(ss_recovery_hold_step(&h, true, 4000u), SS_RECOVERY_HOLD_ARMED);
    EXPECT_EQ(ss_recovery_hold_step(&h, true, 5000u), SS_RECOVERY_HOLD_TRIGGERED);
}

TEST(RecoveryHold, LatePressStillTriggersBeforeDeadline)
{
    ss_recovery_hold_t h = make_hold();
    // Idle for most of the window, then a full hold that completes before the
    // 10000ms deadline.
    EXPECT_EQ(ss_recovery_hold_step(&h, false, 5000u), SS_RECOVERY_HOLD_IDLE);
    EXPECT_EQ(ss_recovery_hold_step(&h, true, 6000u), SS_RECOVERY_HOLD_ARMED);
    EXPECT_EQ(ss_recovery_hold_step(&h, true, 9000u), SS_RECOVERY_HOLD_TRIGGERED);
}

TEST(RecoveryHold, WindowExpiresFromIdle)
{
    ss_recovery_hold_t h = make_hold();
    EXPECT_EQ(ss_recovery_hold_step(&h, false, kWindow), SS_RECOVERY_HOLD_EXPIRED);
}

TEST(RecoveryHold, WindowExpiresFromArmed)
{
    ss_recovery_hold_t h = make_hold();
    // Press starts too late to complete a full hold before the deadline.
    EXPECT_EQ(ss_recovery_hold_step(&h, true, 8000u), SS_RECOVERY_HOLD_ARMED);
    EXPECT_EQ(ss_recovery_hold_step(&h, true, kWindow), SS_RECOVERY_HOLD_EXPIRED);
}

TEST(RecoveryHold, CompletedHoldWinsAtExactDeadline)
{
    // window == hold (contract permits window_ms >= hold_ms): a hold that
    // completes exactly at the deadline must trigger, not expire.
    ss_recovery_hold_t h = make_hold(0u, kHold, kHold);
    EXPECT_EQ(ss_recovery_hold_step(&h, true, 0u), SS_RECOVERY_HOLD_ARMED);
    EXPECT_EQ(ss_recovery_hold_step(&h, true, kHold), SS_RECOVERY_HOLD_TRIGGERED);
}

TEST(RecoveryHold, TriggeredIsSticky)
{
    ss_recovery_hold_t h = make_hold();
    ss_recovery_hold_step(&h, true, 0u);
    ASSERT_EQ(ss_recovery_hold_step(&h, true, kHold), SS_RECOVERY_HOLD_TRIGGERED);
    // Further samples (release, deadline) are no-ops.
    EXPECT_EQ(ss_recovery_hold_step(&h, false, kHold + 100u), SS_RECOVERY_HOLD_TRIGGERED);
    EXPECT_EQ(ss_recovery_hold_step(&h, true, 1000000u), SS_RECOVERY_HOLD_TRIGGERED);
}

TEST(RecoveryHold, ExpiredIsSticky)
{
    ss_recovery_hold_t h = make_hold();
    ASSERT_EQ(ss_recovery_hold_step(&h, false, kWindow), SS_RECOVERY_HOLD_EXPIRED);
    // Even a would-be full hold cannot revive an expired detector.
    EXPECT_EQ(ss_recovery_hold_step(&h, true, kWindow + 1u), SS_RECOVERY_HOLD_EXPIRED);
    EXPECT_EQ(ss_recovery_hold_step(&h, true, kWindow + kHold), SS_RECOVERY_HOLD_EXPIRED);
}

// ---- Rollback precondition verdict -----------------------------------------

ss_recovery_rb_verdict_t eval(bool exists, bool has_app, bool set_boot_ok)
{
    ss_recovery_rb_inputs_t in;
    in.other_slot_exists = exists;
    in.other_slot_has_app = has_app;
    in.set_boot_ok = set_boot_ok;
    return ss_recovery_rollback_eval(&in);
}

TEST(RecoveryRollback, EachVerdict)
{
    EXPECT_EQ(eval(true, true, true), SS_RECOVERY_RB_OK);
    EXPECT_EQ(eval(false, true, true), SS_RECOVERY_RB_NO_OTHER_SLOT);
    EXPECT_EQ(eval(true, false, true), SS_RECOVERY_RB_EMPTY_SLOT);
    EXPECT_EQ(eval(true, true, false), SS_RECOVERY_RB_VERIFY_FAILED);
}

TEST(RecoveryRollback, ExhaustiveEightCombinations)
{
    for (int bits = 0; bits < 8; bits++) {
        const bool exists = (bits & 0x4) != 0;
        const bool has_app = (bits & 0x2) != 0;
        const bool set_boot_ok = (bits & 0x1) != 0;

        const ss_recovery_rb_verdict_t v = eval(exists, has_app, set_boot_ok);

        // OK iff and only iff all three preconditions hold.
        EXPECT_EQ(v == SS_RECOVERY_RB_OK, exists && has_app && set_boot_ok) << "bits=" << bits;

        // Refusal precedence: earliest failing stage wins.
        if (!exists) {
            EXPECT_EQ(v, SS_RECOVERY_RB_NO_OTHER_SLOT) << "bits=" << bits;
        } else if (!has_app) {
            EXPECT_EQ(v, SS_RECOVERY_RB_EMPTY_SLOT) << "bits=" << bits;
        } else if (!set_boot_ok) {
            EXPECT_EQ(v, SS_RECOVERY_RB_VERIFY_FAILED) << "bits=" << bits;
        }
    }
}

// ---- Factory-reset erase plan ----------------------------------------------

TEST(RecoveryFactoryPlan, ExactContentAndOrder)
{
    ASSERT_STREQ(ss_recovery_factory_plan[0], "nvs");
    ASSERT_STREQ(ss_recovery_factory_plan[1], "storage");
    ASSERT_STREQ(ss_recovery_factory_plan[2], "coredump");
    EXPECT_EQ(ss_recovery_factory_plan[3], nullptr); // NULL-terminated
}

TEST(RecoveryFactoryPlan, NeverNamesFirmwareOrPhyPartitions)
{
    // Factory reset resets DATA, not firmware: otadata/ota_0/ota_1 (slot
    // choice is rollback's job) and phy_init must never appear.
    static const char* const kForbidden[] = {"otadata", "ota_0", "ota_1", "phy_init"};
    for (const char* const* p = ss_recovery_factory_plan; *p != nullptr; p++) {
        for (const char* forbidden : kForbidden) { EXPECT_STRNE(*p, forbidden); }
    }
}

} // namespace
