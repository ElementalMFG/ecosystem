// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_hal.c — first compiled unit of the HAL: the board-identity and
// capability accessors (S-03-032). Values come exclusively from the selected
// board's board_config.h; there is deliberately no runtime state here.
// Subsystem init (ss_hal_init/shutdown) lands with the per-driver bring-up
// stories and MUST NOT be stubbed here in the meantime — an accidental weak
// no-op would mask missing drivers.

#include "ss_hal.h"

#include "board_config.h"

const char* ss_hal_board_id(void)
{
    return SS_BOARD_ID_STR;
}

bool ss_hal_has_cap(uint64_t cap_flag)
{
    // True iff every requested bit is present. has_cap(0) == true by
    // definition (the empty requirement); callers pass SS_CAP_* flags.
    return ((uint64_t)SS_BOARD_CAPS & cap_flag) == cap_flag;
}

uint64_t ss_hal_caps_mask(void)
{
    return (uint64_t)SS_BOARD_CAPS;
}
