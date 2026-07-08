// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_muxctl_core.c — pure ownership arbitration for the mic/radio mux. See
// ss_muxctl_core.h for the ownership model and per-function contracts.

#include "ss_muxctl_core.h"

void ss_muxctl_core_init(ss_muxctl_state_t *st, ss_mux_mode_t default_mode)
{
    st->default_mode = default_mode;
    st->mode = default_mode;
    st->owner = SS_MUX_OWNER_NONE;
}

ss_muxctl_result_t ss_muxctl_core_acquire(ss_muxctl_state_t *st, ss_mux_mode_t mode,
                                          ss_mux_owner_t owner)
{
    if (owner == SS_MUX_OWNER_NONE) {
        return SS_MUXCTL_INVALID_ARG;
    }
    if (st->owner != SS_MUX_OWNER_NONE) {
        // Non-reentrant: even the same owner re-acquiring while held is BUSY.
        return SS_MUXCTL_BUSY;
    }
    st->owner = owner;
    st->mode = mode;
    return SS_MUXCTL_OK;
}

ss_muxctl_result_t ss_muxctl_core_release(ss_muxctl_state_t *st, ss_mux_owner_t owner)
{
    if (owner == SS_MUX_OWNER_NONE) {
        return SS_MUXCTL_INVALID_ARG;
    }
    if (st->owner == SS_MUX_OWNER_NONE || st->owner != owner) {
        // Free (double-release) or held by someone else.
        return SS_MUXCTL_NOT_OWNER;
    }
    st->owner = SS_MUX_OWNER_NONE;
    st->mode = st->default_mode; // revert to resting radio path
    return SS_MUXCTL_OK;
}

void ss_muxctl_core_force_release(ss_muxctl_state_t *st)
{
    st->owner = SS_MUX_OWNER_NONE;
    st->mode = st->default_mode;
}
