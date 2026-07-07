// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// Host tests for the S-02-011 resource-telemetry core
// (firmware/main/ss_memwatch_core.c): heap fragmentation math, wrapping
// idle-load deltas, threshold predicates, and the machine-parseable line
// grammar. The contract lives in ss_memwatch_core.h.

#include <gtest/gtest.h>

#include <string>

extern "C" {
#include "ss_memwatch_core.h"
}

namespace
{

// ---- 1. Heap fragmentation percentage --------------------------------------

TEST(MemwatchFrag, ZeroFreeIsZeroPercent)
{
    EXPECT_EQ(ss_memwatch_frag_pct(0u, 0u), 0u);
    EXPECT_EQ(ss_memwatch_frag_pct(0u, 100u), 0u); // skew + no free -> 0
}

TEST(MemwatchFrag, WholeBlockFreeIsZeroPercent)
{
    EXPECT_EQ(ss_memwatch_frag_pct(1000u, 1000u), 0u);
}

TEST(MemwatchFrag, LargestExceedingFreeClampsToZero)
{
    // Skewed pair (largest > free) must not underflow — report 0%.
    EXPECT_EQ(ss_memwatch_frag_pct(1000u, 1500u), 0u);
}

TEST(MemwatchFrag, HalfFragmented)
{
    EXPECT_EQ(ss_memwatch_frag_pct(1000u, 500u), 50u);
}

TEST(MemwatchFrag, RoundsToNearest)
{
    // 667/1000 = 66.7% -> 67 (round half up via +b/2).
    EXPECT_EQ(ss_memwatch_frag_pct(1000u, 333u), 67u);
    // 2/3 = 66.67% -> 67.
    EXPECT_EQ(ss_memwatch_frag_pct(3u, 1u), 67u);
}

TEST(MemwatchFrag, NeverExceedsHundred)
{
    EXPECT_LE(ss_memwatch_frag_pct(UINT32_MAX, 0u), 100u);
    EXPECT_EQ(ss_memwatch_frag_pct(UINT32_MAX, 0u), 100u);
}

// ---- 2. Wrapping 32-bit deltas + idle percentage ---------------------------

TEST(MemwatchDelta, PlainDelta)
{
    EXPECT_EQ(ss_memwatch_u32_delta(100u, 350u), 250u);
    EXPECT_EQ(ss_memwatch_u32_delta(0u, 0u), 0u);
}

TEST(MemwatchDelta, WrapsAcrossZero)
{
    // prev near the top, now just past the wrap -> modular elapsed count.
    EXPECT_EQ(ss_memwatch_u32_delta(0xFFFFFFF0u, 0x10u), 0x20u);
    EXPECT_EQ(ss_memwatch_u32_delta(UINT32_MAX, 0u), 1u);
}

TEST(MemwatchIdlePct, ZeroTotalIsZero)
{
    EXPECT_EQ(ss_memwatch_idle_pct(0u, 0u), 0u);
    EXPECT_EQ(ss_memwatch_idle_pct(500u, 0u), 0u);
}

TEST(MemwatchIdlePct, IdleAtOrAboveTotalClampsToHundred)
{
    EXPECT_EQ(ss_memwatch_idle_pct(1000u, 1000u), 100u);
    EXPECT_EQ(ss_memwatch_idle_pct(1200u, 1000u), 100u); // skew -> clamp
}

TEST(MemwatchIdlePct, ProportionAndRounding)
{
    EXPECT_EQ(ss_memwatch_idle_pct(250u, 1000u), 25u);
    // 1/3 -> 33.3 -> 33.
    EXPECT_EQ(ss_memwatch_idle_pct(1u, 3u), 33u);
    // 2/3 -> 66.7 -> 67.
    EXPECT_EQ(ss_memwatch_idle_pct(2u, 3u), 67u);
}

TEST(MemwatchIdleUpdate, FirstSampleHasNoInterval)
{
    ss_memwatch_idle_state_t st = {}; // zero-init: valid == false
    uint32_t pct = 0xDEADu;
    EXPECT_FALSE(ss_memwatch_idle_update(&st, 100u, 400u, &pct));
    EXPECT_EQ(pct, 0xDEADu); // left unwritten on first sample
    EXPECT_TRUE(st.valid);
    EXPECT_EQ(st.idle_counter, 100u);
    EXPECT_EQ(st.total_counter, 400u);
}

TEST(MemwatchIdleUpdate, SecondSampleComputesInterval)
{
    ss_memwatch_idle_state_t st = {};
    uint32_t pct = 0u;
    ASSERT_FALSE(ss_memwatch_idle_update(&st, 100u, 400u, &pct)); // baseline
    // idle advanced 150, total advanced 600 -> 25%.
    EXPECT_TRUE(ss_memwatch_idle_update(&st, 250u, 1000u, &pct));
    EXPECT_EQ(pct, 25u);
    EXPECT_EQ(st.idle_counter, 250u);
    EXPECT_EQ(st.total_counter, 1000u);
}

TEST(MemwatchIdleUpdate, IntervalHandlesCounterWrap)
{
    ss_memwatch_idle_state_t st = {};
    uint32_t pct = 0u;
    ASSERT_FALSE(ss_memwatch_idle_update(&st, 0xFFFFFF00u, 0xFFFFFE00u, &pct));
    // Both counters wrap to 0: idle advances 0x100 (256), total 0x200 (512),
    // so this interval is 50% idle.
    EXPECT_TRUE(ss_memwatch_idle_update(&st, 0x00u, 0x00u, &pct));
    EXPECT_EQ(pct, 50u);
}

// ---- 3. Threshold predicates -----------------------------------------------

TEST(MemwatchThreshold, HeapLowIsInclusiveBoundary)
{
    EXPECT_TRUE(ss_memwatch_heap_low(100u, 100u));  // at floor
    EXPECT_TRUE(ss_memwatch_heap_low(99u, 100u));   // below
    EXPECT_FALSE(ss_memwatch_heap_low(101u, 100u)); // above
}

TEST(MemwatchThreshold, StackLowIsInclusiveBoundary)
{
    EXPECT_TRUE(
        ss_memwatch_stack_low(SS_MEMWATCH_STACK_FLOOR_BYTES, SS_MEMWATCH_STACK_FLOOR_BYTES));
    EXPECT_TRUE(ss_memwatch_stack_low(0u, SS_MEMWATCH_STACK_FLOOR_BYTES));
    EXPECT_FALSE(
        ss_memwatch_stack_low(SS_MEMWATCH_STACK_FLOOR_BYTES + 1u, SS_MEMWATCH_STACK_FLOOR_BYTES));
}

// ---- 4. Machine-parseable line grammar -------------------------------------

TEST(MemwatchFormat, RegionGroupExactText)
{
    ss_memwatch_region_t r;
    r.name = "internal";
    r.free_bytes = 200000u;
    r.min_free_bytes = 150000u;
    r.largest_free_block = 100000u; // 50% fragmented
    char buf[96];
    const int n = ss_memwatch_format_region(buf, sizeof(buf), &r);
    EXPECT_GT(n, 0);
    EXPECT_LT((size_t)n, sizeof(buf));
    EXPECT_STREQ(buf, "internal[free=200000 min=150000 largest=100000 frag=50%]");
}

TEST(MemwatchFormat, RegionTruncationReportsWouldBeLength)
{
    ss_memwatch_region_t r;
    r.name = "spiram";
    r.free_bytes = 1u;
    r.min_free_bytes = 1u;
    r.largest_free_block = 1u;
    char buf[8];
    const int n = ss_memwatch_format_region(buf, sizeof(buf), &r);
    // snprintf semantics: return is the full length even when truncated.
    EXPECT_GE((size_t)n, sizeof(buf));
    EXPECT_EQ(buf[sizeof(buf) - 1], '\0'); // still NUL-terminated
}

TEST(MemwatchFormat, TaskLineExactText)
{
    ss_memwatch_task_line_t t;
    t.name = "ss_uart";
    t.stack_hwm_bytes = 1234u;
    t.prio = 12u;
    t.state = 'B';
    t.core = 0;
    char buf[128];
    const int n = ss_memwatch_format_task(buf, sizeof(buf), &t);
    EXPECT_GT(n, 0);
    EXPECT_STREQ(buf, "memwatch-task: name=ss_uart prio=12 stack_hwm=1234 state=B core=0");
}

TEST(MemwatchFormat, TaskLineNoAffinityIsNegativeOne)
{
    ss_memwatch_task_line_t t;
    t.name = "IDLE0";
    t.stack_hwm_bytes = 512u;
    t.prio = 0u;
    t.state = 'R';
    t.core = -1; // tskNO_AFFINITY mapped by the target glue
    char buf[128];
    ss_memwatch_format_task(buf, sizeof(buf), &t);
    EXPECT_STREQ(buf, "memwatch-task: name=IDLE0 prio=0 stack_hwm=512 state=R core=-1");
}

} // namespace
