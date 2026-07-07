// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// test_ss_hal_smoke.c — the first on-target HAL smoke test (S-02-015 AC).
//
// It proves two things at once: (1) the Unity runner boots, executes, and
// reports on real silicon, and (2) the selected board's compile-time HAL
// contract (identity + capability mask from boards/<board>/board_config.h) is
// coherent on the target. It touches ONLY compile-time macros — no HAL driver
// entry points (those land per EPIC-03) — so it links cleanly against today's
// header-only ss_hal and can never wedge the runner.

#include <stdbool.h>
#include <stdint.h>

#include "unity.h"

#include "board_config.h" // provided by ss_hal for the selected board; pulls
                          // in ss_hal_caps.h (SS_CAP_* flags)

TEST_CASE("board identity is defined for the active board", "[ss_hal][smoke]")
{
    TEST_ASSERT_NOT_NULL(SS_BOARD_ID_STR);
    TEST_ASSERT_NOT_EQUAL('\0', SS_BOARD_ID_STR[0]);
    TEST_ASSERT_GREATER_THAN_UINT32(0, (uint32_t)SS_BOARD_ID_INT);
}

TEST_CASE("active board advertises a coherent capability mask", "[ss_hal][smoke]")
{
    const uint64_t caps = (uint64_t)SS_BOARD_CAPS;

    // Every SS-SP board is either a display unit or an explicit headless
    // gateway — never neither.
    const bool has_ui = (caps & SS_CAP_DISPLAY) != 0ULL;
    const bool headless = (caps & SS_CAP_HEADLESS) != 0ULL;
    TEST_ASSERT_TRUE(has_ui != headless);

    // A pager is useless without a radio: at least one radio bit must be set.
    const uint64_t any_radio = SS_CAP_RADIO_LORA | SS_CAP_RADIO_HALOW | SS_CAP_RADIO_BLE |
                               SS_CAP_RADIO_WIFI4 | SS_CAP_RADIO_WIFI6 | SS_CAP_RADIO_LR11XX;
    TEST_ASSERT_NOT_EQUAL(0ULL, caps & any_radio);
}
