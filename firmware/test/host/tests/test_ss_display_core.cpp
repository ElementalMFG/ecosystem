// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// Host tests for the S-03-006 ss_display core
// (firmware/components/ss_display/src/ss_display_core.c): rect clipping, the
// RGB565->RGB666 wire conversion, the 60 FPS transfer-time budget, and the
// per-orientation MADCTL byte. The contract lives in ss_hal_display.h; geometry
// is passed as arguments so no board_config is needed.

#include <cstdint>
#include <set>

#include <gtest/gtest.h>

extern "C" {
#include "ss_display_core.h"
}

namespace
{

constexpr uint16_t kW = 480; // Lite native landscape width
constexpr uint16_t kH = 320; // Lite native landscape height

// ---- clip() ----------------------------------------------------------------

TEST(DisplayClip, FullyInBoundsUnchanged)
{
    ss_rect_t out{};
    ss_rect_t in{10, 20, 30, 40};
    EXPECT_TRUE(ss_display_core_clip(in, kW, kH, &out));
    EXPECT_EQ(out.x, 10);
    EXPECT_EQ(out.y, 20);
    EXPECT_EQ(out.w, 30);
    EXPECT_EQ(out.h, 40);
}

TEST(DisplayClip, PartialOverlapTopLeft)
{
    ss_rect_t out{};
    ss_rect_t in{-5, -5, 20, 20}; // extends to (15,15)
    EXPECT_TRUE(ss_display_core_clip(in, kW, kH, &out));
    EXPECT_EQ(out.x, 0);
    EXPECT_EQ(out.y, 0);
    EXPECT_EQ(out.w, 15);
    EXPECT_EQ(out.h, 15);
}

TEST(DisplayClip, PartialOverlapBottomRight)
{
    ss_rect_t out{};
    ss_rect_t in{470, 310, 40, 40}; // extends past 480x320
    EXPECT_TRUE(ss_display_core_clip(in, kW, kH, &out));
    EXPECT_EQ(out.x, 470);
    EXPECT_EQ(out.y, 310);
    EXPECT_EQ(out.w, 10);
    EXPECT_EQ(out.h, 10);
}

TEST(DisplayClip, FullyOffScreenNotVisible)
{
    ss_rect_t out{};
    ss_rect_t in{500, 400, 10, 10};
    EXPECT_FALSE(ss_display_core_clip(in, kW, kH, &out));
    EXPECT_EQ(out.x, 0);
    EXPECT_EQ(out.y, 0);
    EXPECT_EQ(out.w, 0);
    EXPECT_EQ(out.h, 0);
}

TEST(DisplayClip, NullOutNotVisible)
{
    ss_rect_t in{0, 0, 10, 10};
    EXPECT_FALSE(ss_display_core_clip(in, kW, kH, nullptr));
}

// ---- px565_to_666() --------------------------------------------------------

TEST(DisplayConvert, PrimaryColours)
{
    uint8_t o[3];

    ss_display_core_px565_to_666(0xF800, o); // pure red
    EXPECT_EQ(o[0], 0xF8);
    EXPECT_EQ(o[1], 0x00);
    EXPECT_EQ(o[2], 0x00);

    ss_display_core_px565_to_666(0x07E0, o); // pure green
    EXPECT_EQ(o[0], 0x00);
    EXPECT_EQ(o[1], 0xFC);
    EXPECT_EQ(o[2], 0x00);

    ss_display_core_px565_to_666(0x001F, o); // pure blue
    EXPECT_EQ(o[0], 0x00);
    EXPECT_EQ(o[1], 0x00);
    EXPECT_EQ(o[2], 0xF8);
}

TEST(DisplayConvert, WhiteAndBlack)
{
    uint8_t o[3];
    ss_display_core_px565_to_666(0xFFFF, o); // white
    EXPECT_EQ(o[0], 0xF8);
    EXPECT_EQ(o[1], 0xFC);
    EXPECT_EQ(o[2], 0xF8);

    ss_display_core_px565_to_666(0x0000, o); // black
    EXPECT_EQ(o[0], 0x00);
    EXPECT_EQ(o[1], 0x00);
    EXPECT_EQ(o[2], 0x00);
}

// ---- tile_xfer_us(): the 60 FPS partial-redraw budget ----------------------

TEST(DisplayBudget, Tile32x32RGB666FitsOneFrame)
{
    // 32x32 RGB666 tile at 40 MHz must clock out well inside a 16.67 ms frame.
    EXPECT_LT(ss_display_core_tile_xfer_us(32, 32, 3, 40000000), 16667u);
}

TEST(DisplayBudget, ComputesCeiling)
{
    // 32*32*3*8 = 24576 bits; /40e6 * 1e6 = 614.4 us -> ceil 615.
    EXPECT_EQ(ss_display_core_tile_xfer_us(32, 32, 3, 40000000), 615u);
}

TEST(DisplayBudget, ZeroFreqNeverFits)
{
    EXPECT_EQ(ss_display_core_tile_xfer_us(32, 32, 3, 0), UINT32_MAX);
}

// ---- madctl() --------------------------------------------------------------

TEST(DisplayMadctl, NativeLandscapeIs0xE8)
{
    EXPECT_EQ(ss_display_core_madctl(SS_ORIENT_90), 0xE8);
}

TEST(DisplayMadctl, DistinctPerOrientation)
{
    std::set<uint8_t> vals{
        ss_display_core_madctl(SS_ORIENT_0),
        ss_display_core_madctl(SS_ORIENT_90),
        ss_display_core_madctl(SS_ORIENT_180),
        ss_display_core_madctl(SS_ORIENT_270),
    };
    EXPECT_EQ(vals.size(), 4u);
    // Every orientation keeps the BGR bit (0x08) set.
    for (uint8_t v : vals) { EXPECT_EQ(v & 0x08u, 0x08u); }
}

} // namespace
