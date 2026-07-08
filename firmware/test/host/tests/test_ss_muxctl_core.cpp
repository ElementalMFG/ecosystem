// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// Host tests for the S-03-033 ss_muxctl arbitration core
// (firmware/components/ss_muxctl/src/ss_muxctl_core.c): ownership acquire /
// release / force-release semantics for the mic-vs-radio GPIO mux. The frozen
// contract types live in ss_hal_types.h.

#include <gtest/gtest.h>

extern "C" {
#include "ss_muxctl_core.h"
}

namespace
{

// ---- init ------------------------------------------------------------------

TEST(MuxctlInit, RestingRadioAndFree)
{
    ss_muxctl_state_t st;
    ss_muxctl_core_init(&st, SS_MUX_MODE_RADIO);
    EXPECT_EQ(st.mode, SS_MUX_MODE_RADIO);
    EXPECT_EQ(st.owner, SS_MUX_OWNER_NONE);
    EXPECT_EQ(st.default_mode, SS_MUX_MODE_RADIO);
}

// ---- acquire ---------------------------------------------------------------

TEST(MuxctlAcquire, FreeGrantsEachOwner)
{
    const ss_mux_owner_t owners[] = {
        SS_MUX_OWNER_RADIO_LORA, SS_MUX_OWNER_RADIO_BLE, SS_MUX_OWNER_RADIO_WIFI,
        SS_MUX_OWNER_AUDIO_MIC,  SS_MUX_OWNER_TEST,
    };
    const ss_mux_mode_t modes[] = {SS_MUX_MODE_RADIO, SS_MUX_MODE_MIC};
    for (ss_mux_owner_t o : owners) {
        for (ss_mux_mode_t m : modes) {
            ss_muxctl_state_t st;
            ss_muxctl_core_init(&st, SS_MUX_MODE_RADIO);
            EXPECT_EQ(ss_muxctl_core_acquire(&st, m, o), SS_MUXCTL_OK);
            EXPECT_EQ(st.owner, o);
            EXPECT_EQ(st.mode, m);
        }
    }
}

TEST(MuxctlAcquire, NoneOwnerIsInvalidArg)
{
    ss_muxctl_state_t st;
    ss_muxctl_core_init(&st, SS_MUX_MODE_RADIO);
    EXPECT_EQ(ss_muxctl_core_acquire(&st, SS_MUX_MODE_MIC, SS_MUX_OWNER_NONE),
              SS_MUXCTL_INVALID_ARG);
    // State unchanged.
    EXPECT_EQ(st.owner, SS_MUX_OWNER_NONE);
    EXPECT_EQ(st.mode, SS_MUX_MODE_RADIO);
}

TEST(MuxctlAcquire, ContentionByOtherOwnerIsBusy)
{
    ss_muxctl_state_t st;
    ss_muxctl_core_init(&st, SS_MUX_MODE_RADIO);
    ASSERT_EQ(ss_muxctl_core_acquire(&st, SS_MUX_MODE_MIC, SS_MUX_OWNER_AUDIO_MIC),
              SS_MUXCTL_OK);
    EXPECT_EQ(ss_muxctl_core_acquire(&st, SS_MUX_MODE_RADIO, SS_MUX_OWNER_RADIO_LORA),
              SS_MUXCTL_BUSY);
    // Still owned by A, mode unchanged.
    EXPECT_EQ(st.owner, SS_MUX_OWNER_AUDIO_MIC);
    EXPECT_EQ(st.mode, SS_MUX_MODE_MIC);
}

TEST(MuxctlAcquire, NonReentrantSameOwnerIsBusy)
{
    ss_muxctl_state_t st;
    ss_muxctl_core_init(&st, SS_MUX_MODE_RADIO);
    ASSERT_EQ(ss_muxctl_core_acquire(&st, SS_MUX_MODE_MIC, SS_MUX_OWNER_AUDIO_MIC),
              SS_MUXCTL_OK);
    EXPECT_EQ(ss_muxctl_core_acquire(&st, SS_MUX_MODE_MIC, SS_MUX_OWNER_AUDIO_MIC),
              SS_MUXCTL_BUSY);
    EXPECT_EQ(st.owner, SS_MUX_OWNER_AUDIO_MIC);
}

// ---- release ---------------------------------------------------------------

TEST(MuxctlRelease, HolderReleasesRevertsToDefault)
{
    ss_muxctl_state_t st;
    ss_muxctl_core_init(&st, SS_MUX_MODE_RADIO);
    ASSERT_EQ(ss_muxctl_core_acquire(&st, SS_MUX_MODE_MIC, SS_MUX_OWNER_AUDIO_MIC),
              SS_MUXCTL_OK);
    EXPECT_EQ(ss_muxctl_core_release(&st, SS_MUX_OWNER_AUDIO_MIC), SS_MUXCTL_OK);
    EXPECT_EQ(st.owner, SS_MUX_OWNER_NONE);
    EXPECT_EQ(st.mode, SS_MUX_MODE_RADIO);
}

TEST(MuxctlRelease, WrongOwnerIsNotOwner)
{
    ss_muxctl_state_t st;
    ss_muxctl_core_init(&st, SS_MUX_MODE_RADIO);
    ASSERT_EQ(ss_muxctl_core_acquire(&st, SS_MUX_MODE_MIC, SS_MUX_OWNER_AUDIO_MIC),
              SS_MUXCTL_OK);
    EXPECT_EQ(ss_muxctl_core_release(&st, SS_MUX_OWNER_RADIO_BLE), SS_MUXCTL_NOT_OWNER);
    // Still owned by A.
    EXPECT_EQ(st.owner, SS_MUX_OWNER_AUDIO_MIC);
    EXPECT_EQ(st.mode, SS_MUX_MODE_MIC);
}

TEST(MuxctlRelease, DoubleReleaseIsNotOwner)
{
    ss_muxctl_state_t st;
    ss_muxctl_core_init(&st, SS_MUX_MODE_RADIO);
    EXPECT_EQ(ss_muxctl_core_release(&st, SS_MUX_OWNER_AUDIO_MIC), SS_MUXCTL_NOT_OWNER);
}

TEST(MuxctlRelease, NoneOwnerIsInvalidArg)
{
    ss_muxctl_state_t st;
    ss_muxctl_core_init(&st, SS_MUX_MODE_RADIO);
    EXPECT_EQ(ss_muxctl_core_release(&st, SS_MUX_OWNER_NONE), SS_MUXCTL_INVALID_ARG);
}

// ---- force_release ---------------------------------------------------------

TEST(MuxctlForceRelease, HeldReturnsToDefault)
{
    ss_muxctl_state_t st;
    ss_muxctl_core_init(&st, SS_MUX_MODE_RADIO);
    ASSERT_EQ(ss_muxctl_core_acquire(&st, SS_MUX_MODE_MIC, SS_MUX_OWNER_TEST), SS_MUXCTL_OK);
    ss_muxctl_core_force_release(&st);
    EXPECT_EQ(st.owner, SS_MUX_OWNER_NONE);
    EXPECT_EQ(st.mode, SS_MUX_MODE_RADIO);
}

TEST(MuxctlForceRelease, FreeIsIdempotent)
{
    ss_muxctl_state_t st;
    ss_muxctl_core_init(&st, SS_MUX_MODE_RADIO);
    ss_muxctl_core_force_release(&st);
    EXPECT_EQ(st.owner, SS_MUX_OWNER_NONE);
    EXPECT_EQ(st.mode, SS_MUX_MODE_RADIO);
}

// ---- PTT-over-LoRa flow ----------------------------------------------------

TEST(MuxctlFlow, MicThenRadioTransitions)
{
    ss_muxctl_state_t st;
    ss_muxctl_core_init(&st, SS_MUX_MODE_RADIO);

    // PTT: grab the mic.
    ASSERT_EQ(ss_muxctl_core_acquire(&st, SS_MUX_MODE_MIC, SS_MUX_OWNER_AUDIO_MIC),
              SS_MUXCTL_OK);
    EXPECT_EQ(st.mode, SS_MUX_MODE_MIC);

    // Release mic; mux rests back on the radio path.
    ASSERT_EQ(ss_muxctl_core_release(&st, SS_MUX_OWNER_AUDIO_MIC), SS_MUXCTL_OK);
    EXPECT_EQ(st.mode, SS_MUX_MODE_RADIO);
    EXPECT_EQ(st.owner, SS_MUX_OWNER_NONE);

    // Transmit over LoRa.
    ASSERT_EQ(ss_muxctl_core_acquire(&st, SS_MUX_MODE_RADIO, SS_MUX_OWNER_RADIO_LORA),
              SS_MUXCTL_OK);
    EXPECT_EQ(st.mode, SS_MUX_MODE_RADIO);
    EXPECT_EQ(st.owner, SS_MUX_OWNER_RADIO_LORA);
    ASSERT_EQ(ss_muxctl_core_release(&st, SS_MUX_OWNER_RADIO_LORA), SS_MUXCTL_OK);
    EXPECT_EQ(st.owner, SS_MUX_OWNER_NONE);
}

} // namespace
