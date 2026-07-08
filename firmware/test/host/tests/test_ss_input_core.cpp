// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// Host tests for the S-03-004 ss_input decision cores
// (firmware/components/ss_input/src/ss_input_core.c): the touch gesture
// recogniser (tap / long-press / 4-way swipe / slop rejection) and the button
// debouncer (bounce rejection, press->long-press->release, short-blip
// rejection). Contracts: ss_hal_touch.h / ss_hal_buttons.h + ss_hal_types.h.

#include <gtest/gtest.h>

extern "C" {
#include "ss_input_core.h"
}

namespace
{

// Microsecond timestamp helper for a millisecond offset.
constexpr ss_mono_us_t us(uint64_t ms) { return static_cast<ss_mono_us_t>(ms) * 1000u; }

// ---- gesture recogniser ----------------------------------------------------

TEST(GestureTap, DownUpWithinSlopAndTimeIsTap)
{
    ss_gesture_t g{};
    ss_gesture_init(&g);
    ss_input_event_t ev{};
    EXPECT_FALSE(ss_gesture_step(&g, SS_TOUCH_SAMPLE_DOWN, 100, 100, us(0), &ev));
    ASSERT_TRUE(ss_gesture_step(&g, SS_TOUCH_SAMPLE_UP, 102, 101, us(120), &ev));
    EXPECT_EQ(ev.kind, SS_INPUT_TAP);
}

TEST(GestureTap, SmallJitterStillTap)
{
    ss_gesture_t g{};
    ss_gesture_init(&g);
    ss_input_event_t ev{};
    ss_gesture_step(&g, SS_TOUCH_SAMPLE_DOWN, 100, 100, us(0), &ev);
    // moved 10x8 px — within the 12 px slop, still a tap (slop rejection).
    ASSERT_TRUE(ss_gesture_step(&g, SS_TOUCH_SAMPLE_UP, 110, 108, us(150), &ev));
    EXPECT_EQ(ev.kind, SS_INPUT_TAP);
}

TEST(GestureLongPress, HeldToReleaseIsLongPress)
{
    ss_gesture_t g{};
    ss_gesture_init(&g);
    ss_input_event_t ev{};
    ss_gesture_step(&g, SS_TOUCH_SAMPLE_DOWN, 50, 50, us(0), &ev);
    ASSERT_TRUE(ss_gesture_step(&g, SS_TOUCH_SAMPLE_UP, 52, 52, us(600), &ev));
    EXPECT_EQ(ev.kind, SS_INPUT_LONG_PRESS);
}

TEST(GestureLongPress, EmittedOnMoveThenNotRepeatedOnUp)
{
    ss_gesture_t g{};
    ss_gesture_init(&g);
    ss_input_event_t ev{};
    ss_gesture_step(&g, SS_TOUCH_SAMPLE_DOWN, 50, 50, us(0), &ev);
    ASSERT_TRUE(ss_gesture_step(&g, SS_TOUCH_SAMPLE_MOVE, 51, 51, us(550), &ev));
    EXPECT_EQ(ev.kind, SS_INPUT_LONG_PRESS);
    // Release must not fire a second long-press for the same press.
    EXPECT_FALSE(ss_gesture_step(&g, SS_TOUCH_SAMPLE_UP, 51, 51, us(700), &ev));
}

TEST(GestureSwipe, Right)
{
    ss_gesture_t g{};
    ss_gesture_init(&g);
    ss_input_event_t ev{};
    ss_gesture_step(&g, SS_TOUCH_SAMPLE_DOWN, 10, 100, us(0), &ev);
    ASSERT_TRUE(ss_gesture_step(&g, SS_TOUCH_SAMPLE_UP, 80, 105, us(120), &ev));
    EXPECT_EQ(ev.kind, SS_INPUT_SWIPE_R);
    EXPECT_EQ(ev.delta, 70);
}

TEST(GestureSwipe, Left)
{
    ss_gesture_t g{};
    ss_gesture_init(&g);
    ss_input_event_t ev{};
    ss_gesture_step(&g, SS_TOUCH_SAMPLE_DOWN, 80, 100, us(0), &ev);
    ASSERT_TRUE(ss_gesture_step(&g, SS_TOUCH_SAMPLE_UP, 10, 105, us(120), &ev));
    EXPECT_EQ(ev.kind, SS_INPUT_SWIPE_L);
    EXPECT_EQ(ev.delta, 70);
}

TEST(GestureSwipe, Up)
{
    ss_gesture_t g{};
    ss_gesture_init(&g);
    ss_input_event_t ev{};
    ss_gesture_step(&g, SS_TOUCH_SAMPLE_DOWN, 100, 200, us(0), &ev);
    ASSERT_TRUE(ss_gesture_step(&g, SS_TOUCH_SAMPLE_UP, 105, 120, us(120), &ev));
    EXPECT_EQ(ev.kind, SS_INPUT_SWIPE_U);
    EXPECT_EQ(ev.delta, 80);
}

TEST(GestureSwipe, Down)
{
    ss_gesture_t g{};
    ss_gesture_init(&g);
    ss_input_event_t ev{};
    ss_gesture_step(&g, SS_TOUCH_SAMPLE_DOWN, 100, 120, us(0), &ev);
    ASSERT_TRUE(ss_gesture_step(&g, SS_TOUCH_SAMPLE_UP, 105, 200, us(120), &ev));
    EXPECT_EQ(ev.kind, SS_INPUT_SWIPE_D);
    EXPECT_EQ(ev.delta, 80);
}

// ---- button debouncer ------------------------------------------------------

TEST(BtnDebounce, ShortBlipRejected)
{
    ss_debounce_t d{};
    ss_debounce_init(&d);
    ss_input_event_t ev{};
    // Press appears then vanishes before the 20 ms debounce interval.
    EXPECT_FALSE(ss_debounce_step(&d, true, 0, &ev));
    EXPECT_FALSE(ss_debounce_step(&d, false, 10, &ev));
    EXPECT_FALSE(ss_debounce_step(&d, false, 30, &ev));
}

TEST(BtnDebounce, BounceNoiseRejected)
{
    ss_debounce_t d{};
    ss_debounce_init(&d);
    ss_input_event_t ev{};
    // Chatter: never stable for a full debounce interval -> no commit.
    EXPECT_FALSE(ss_debounce_step(&d, true, 0, &ev));
    EXPECT_FALSE(ss_debounce_step(&d, false, 5, &ev));
    EXPECT_FALSE(ss_debounce_step(&d, true, 10, &ev));
    EXPECT_FALSE(ss_debounce_step(&d, false, 15, &ev));
}

TEST(BtnDebounce, PressLongPressRelease)
{
    ss_debounce_t d{};
    ss_debounce_init(&d);
    ss_input_event_t ev{};

    EXPECT_FALSE(ss_debounce_step(&d, true, 0, &ev)); // candidate press
    ASSERT_TRUE(ss_debounce_step(&d, true, 25, &ev)); // stable -> DOWN
    EXPECT_EQ(ev.kind, SS_INPUT_BUTTON_DOWN);

    ASSERT_TRUE(ss_debounce_step(&d, true, 900, &ev)); // held >= 800 ms
    EXPECT_EQ(ev.kind, SS_INPUT_LONG_PRESS);
    EXPECT_FALSE(ss_debounce_step(&d, true, 1000, &ev)); // no repeat long-press

    EXPECT_FALSE(ss_debounce_step(&d, false, 1010, &ev)); // candidate release
    ASSERT_TRUE(ss_debounce_step(&d, false, 1035, &ev));  // stable -> UP
    EXPECT_EQ(ev.kind, SS_INPUT_BUTTON_UP);
}

} // namespace
