// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_storage_fs_core.c — pure self-heal mount FSM + wear-stat arithmetic. See
// ss_storage_fs_core.h for scope, the bounded-FSM policy, and the fail-safe
// rationale. No ESP-IDF or HAL dependency lives here; all math is integer (no
// libm), no dynamic alloc, no mutable globals.

#include "ss_storage_fs_core.h"

// A mount is tried at most twice: once before formatting, once after.
#define SS_FS_MAX_MOUNT_ATTEMPTS 2u

void ss_fs_heal_init(ss_fs_heal_state_t* state)
{
    if (state == NULL) { return; }
    state->mount_attempts = 0u;
    state->formatted = false;
}

ss_fs_step_t ss_fs_heal_next(ss_fs_heal_state_t* state, ss_fs_step_t last, ss_fs_result_t res)
{
    // Fail-safe: no state, or a nonsense result value, halts the machine.
    if (state == NULL) { return SS_FS_STEP_FAILED; }
    if (res != SS_FS_R_OK && res != SS_FS_R_ERR) { return SS_FS_STEP_FAILED; }

    switch (last) {
    case SS_FS_STEP_MOUNT:
        // Count the attempt we were told about; guard against overflow / a
        // caller that ignored a terminal result and kept mounting.
        if (state->mount_attempts < SS_FS_MAX_MOUNT_ATTEMPTS) { state->mount_attempts++; }

        if (res == SS_FS_R_OK) { return SS_FS_STEP_DONE_OK; }
        // Mount failed. Format exactly once, then allow one remount.
        if (!state->formatted) { return SS_FS_STEP_FORMAT; }
        return SS_FS_STEP_FAILED; // post-format remount failed → unrecoverable

    case SS_FS_STEP_FORMAT:
        state->formatted = true;
        if (res == SS_FS_R_OK) { return SS_FS_STEP_MOUNT; } // retry the mount
        return SS_FS_STEP_FAILED;                           // format failed → give up

    case SS_FS_STEP_DONE_OK:
    case SS_FS_STEP_FAILED:
    default:
        // Terminal or garbage `last` → fail-safe.
        return SS_FS_STEP_FAILED;
    }
}

uint8_t ss_fs_fill_pct(ss_fs_usage_t usage)
{
    // No geometry → treat as full so callers back off rather than over-write.
    if (usage.total_bytes == 0u) { return 100u; }
    if (usage.used_bytes >= usage.total_bytes) { return 100u; }
    // used < total here, so the product cannot overflow relative to total and
    // the quotient is in 0..99.
    return (uint8_t)((usage.used_bytes * 100u) / usage.total_bytes);
}

uint64_t ss_fs_free_bytes(ss_fs_usage_t usage)
{
    if (usage.used_bytes >= usage.total_bytes) { return 0u; }
    return usage.total_bytes - usage.used_bytes;
}
