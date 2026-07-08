// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// test_hal_mock.cpp — proves the mocked HAL header set is available and usable
// host-side (the S-02-014 "mocked HAL headers available" acceptance criterion).
//
// It includes the host board_config.h mock (which in turn pulls in the real,
// dependency-free ss_hal_caps.h and ss_hal_types.h) and asserts that the board
// identity and capability-mask relationships compile and hold on the host —
// exactly the surface a pure-C HAL consumer touches via ss_hal_has_cap().

#include <cstdint>

#include "gtest/gtest.h"

extern "C" {
#include "board_config.h" // host mock; pulls real ss_hal_caps.h + ss_hal_types.h
}

// Compile-time checks: the caps mask fits its 64-bit carrier, and BASE / RADIO
// are disjoint (a real board_config OR-s independent bit groups).
static_assert(SS_CAP_HEADLESS == (1ULL << 38), "highest cap flag moved — re-audit ss_hal_has_cap width");
static_assert((SS_BOARD_CAPS >> 39) == 0, "caps mask exceeds bit 38 — re-audit accessor width");
static_assert((SS_BOARD_CAPS_BASE & SS_BOARD_CAPS_RADIO) == 0, "cap groups must be disjoint");

// The board identity macro is a compile-time string constant.
TEST(HalMock, BoardIdentityIsHostMock)
{
    EXPECT_STREQ(SS_BOARD_ID_STR, "ss-sp-host-mock");
    EXPECT_EQ(SS_BOARD_ID_INT, 0xFFFF);
}

// Capability queries are the app-facing contract (ss_hal_has_cap reads this
// mask). A present cap tests nonzero; an unfitted one tests zero.
TEST(HalMock, CapsMaskHoldsAdvertisedFlags)
{
    const uint64_t caps = SS_BOARD_CAPS;
    EXPECT_NE(caps & SS_CAP_DISPLAY, 0ULL);    // present on the mock board
    EXPECT_NE(caps & SS_CAP_RADIO_LORA, 0ULL); // OR-ed in via SS_BOARD_CAPS_RADIO
    EXPECT_EQ(caps & SS_CAP_PMIC, 0ULL);       // deliberately not fitted
    EXPECT_EQ(caps & SS_CAP_FUEL_GAUGE, 0ULL); // deliberately not fitted
}

// Shared HAL types (ss_hal_types.h) and geometry macros resolve host-side.
TEST(HalMock, DisplayGeometryMacrosResolve)
{
    EXPECT_GT(SS_LCD_W_PX, 0);
    EXPECT_GT(SS_LCD_H_PX, 0);
    EXPECT_EQ(SS_LCD_NATIVE_ORIENT, SS_ORIENT_90);
    EXPECT_EQ(SS_LCD_FMT, SS_PIXFMT_RGB565);
    EXPECT_EQ(SS_BATTERY_SENSE_PRESENT, 0);
}
