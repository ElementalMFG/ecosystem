// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// Host tests for the S-03-001 ss_power decision core
// (firmware/components/ss_power/src/ss_power_core.c): the state-transition
// decision table, the wake-source table validation, and the "no fuel gauge"
// status fill. The contract lives in ss_hal_power.h.

#include <cstring>

#include <gtest/gtest.h>

extern "C" {
#include "ss_power_core.h"
}

namespace
{

// ---- decide() --------------------------------------------------------------

TEST(PowerDecide, OnStaysAwake)
{
    EXPECT_EQ(ss_power_core_decide(SS_PWR_STATE_ON, SS_PWR_STATE_ON), SS_PWR_ACTION_NONE);
    EXPECT_EQ(ss_power_core_decide(SS_PWR_STATE_LIGHT_SLEEP, SS_PWR_STATE_ON), SS_PWR_ACTION_NONE);
}

TEST(PowerDecide, ValidSleepTransitions)
{
    EXPECT_EQ(ss_power_core_decide(SS_PWR_STATE_ON, SS_PWR_STATE_LIGHT_SLEEP),
              SS_PWR_ACTION_LIGHT_SLEEP);
    EXPECT_EQ(ss_power_core_decide(SS_PWR_STATE_ON, SS_PWR_STATE_DEEP_SLEEP),
              SS_PWR_ACTION_DEEP_SLEEP);
    EXPECT_EQ(ss_power_core_decide(SS_PWR_STATE_ON, SS_PWR_STATE_HIBERNATE),
              SS_PWR_ACTION_HIBERNATE);
    EXPECT_EQ(ss_power_core_decide(SS_PWR_STATE_ON, SS_PWR_STATE_SHUTDOWN), SS_PWR_ACTION_SHUTDOWN);
}

TEST(PowerDecide, OutOfRangeIsInvalid)
{
    const auto bad = static_cast<ss_power_state_t>(SS_PWR_STATE_SHUTDOWN + 1);
    EXPECT_EQ(ss_power_core_decide(bad, SS_PWR_STATE_ON), SS_PWR_ACTION_INVALID);
    EXPECT_EQ(ss_power_core_decide(SS_PWR_STATE_ON, bad), SS_PWR_ACTION_INVALID);
    const auto neg = static_cast<ss_power_state_t>(-1);
    EXPECT_EQ(ss_power_core_decide(neg, SS_PWR_STATE_ON), SS_PWR_ACTION_INVALID);
    EXPECT_EQ(ss_power_core_decide(SS_PWR_STATE_ON, neg), SS_PWR_ACTION_INVALID);
}

// ---- wake_add() ------------------------------------------------------------

TEST(PowerWakeAdd, AcceptsValidEntries)
{
    ss_power_wake_table_t t{};
    EXPECT_TRUE(ss_power_core_wake_add(&t, 5, 0));
    EXPECT_TRUE(ss_power_core_wake_add(&t, 6, 1));
    EXPECT_EQ(t.count, 2u);
    EXPECT_EQ(t.items[0].gpio, 5);
    EXPECT_EQ(t.items[0].level, 0);
    EXPECT_EQ(t.items[1].gpio, 6);
    EXPECT_EQ(t.items[1].level, 1);
}

TEST(PowerWakeAdd, RejectsNullTable)
{
    EXPECT_FALSE(ss_power_core_wake_add(nullptr, 5, 0));
}

TEST(PowerWakeAdd, RejectsBadLevel)
{
    ss_power_wake_table_t t{};
    EXPECT_FALSE(ss_power_core_wake_add(&t, 5, 2));
    EXPECT_FALSE(ss_power_core_wake_add(&t, 5, -1));
    EXPECT_EQ(t.count, 0u);
}

TEST(PowerWakeAdd, RejectsNegativeGpio)
{
    ss_power_wake_table_t t{};
    EXPECT_FALSE(ss_power_core_wake_add(&t, -1, 0));
    EXPECT_EQ(t.count, 0u);
}

TEST(PowerWakeAdd, RejectsDuplicateGpio)
{
    ss_power_wake_table_t t{};
    EXPECT_TRUE(ss_power_core_wake_add(&t, 7, 0));
    EXPECT_FALSE(ss_power_core_wake_add(&t, 7, 1));
    EXPECT_EQ(t.count, 1u);
}

TEST(PowerWakeAdd, RejectsOverflowAtNinth)
{
    ss_power_wake_table_t t{};
    for (int i = 0; i < SS_PWR_MAX_WAKE_SOURCES; i++) {
        EXPECT_TRUE(ss_power_core_wake_add(&t, i, i & 1));
    }
    EXPECT_EQ(t.count, static_cast<unsigned>(SS_PWR_MAX_WAKE_SOURCES));
    // 9th distinct gpio must be rejected: table full.
    EXPECT_FALSE(ss_power_core_wake_add(&t, 100, 0));
    EXPECT_EQ(t.count, static_cast<unsigned>(SS_PWR_MAX_WAKE_SOURCES));
}

// ---- fill_nogauge_status() -------------------------------------------------

TEST(PowerStatus, NoGaugeValues)
{
    ss_power_status_t s;
    std::memset(&s, 0xFF, sizeof(s)); // poison to prove the fill zeroes it
    ss_power_core_fill_nogauge_status(&s);
    EXPECT_FALSE(s.battery_sense_valid);
    EXPECT_EQ(s.v_mv, 0u);
    EXPECT_EQ(s.i_ma, 0);
    EXPECT_EQ(s.soc_percent, 0u);
    EXPECT_EQ(s.temp_c_x10, 0);
    EXPECT_EQ(s.charge_state, SS_CHG_UNKNOWN);
    EXPECT_FALSE(s.on_external_power);
    EXPECT_FALSE(s.usb_present);
}

TEST(PowerStatus, NullIsNoOp)
{
    ss_power_core_fill_nogauge_status(nullptr); // must not crash
}

} // namespace
