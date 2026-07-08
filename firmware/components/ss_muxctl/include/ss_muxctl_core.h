// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_muxctl_core.h — pure, host-testable decision core for the ss_muxctl HAL.
//
// This layer holds NO ESP-IDF runtime dependency: it reasons only about the
// frozen contract types (ss_hal_types.h) so it can be exercised with host
// gtest. The IDF glue (ss_muxctl.c) turns these decisions into GPIO drives and
// FreeRTOS semaphore operations.
//
// Ownership model (non-reentrant): the mux has at most one owner at a time. A
// holder MUST release before it can acquire again — a same-owner re-acquire
// while still held is reported BUSY, never silently granted. SS_MUX_OWNER_NONE
// is not a valid caller identity for acquire/release (INVALID_ARG).

#pragma once
#include "ss_hal_types.h"
#include <stdbool.h>

typedef enum {
    SS_MUXCTL_OK = 0,      // granted / released
    SS_MUXCTL_BUSY,        // held by a different owner
    SS_MUXCTL_NOT_OWNER,   // release/double-release by a non-holder
    SS_MUXCTL_INVALID_ARG, // owner == SS_MUX_OWNER_NONE
} ss_muxctl_result_t;

typedef struct {
    ss_mux_mode_t  mode;         // current physical mode
    ss_mux_owner_t owner;        // current owner, SS_MUX_OWNER_NONE if free
    ss_mux_mode_t  default_mode; // resting mode restored on init/release/force
} ss_muxctl_state_t;

/**
 * Initialize the core state to the resting mux configuration.
 *
 * Pre:  st != NULL.
 * Post: st->default_mode == default_mode, st->mode == default_mode,
 *       st->owner == SS_MUX_OWNER_NONE.
 */
void ss_muxctl_core_init(ss_muxctl_state_t *st, ss_mux_mode_t default_mode);

/**
 * Attempt to acquire the mux for `owner` in `mode`.
 *
 * Pre:  st != NULL.
 * Post/return:
 *   - SS_MUXCTL_INVALID_ARG if owner == SS_MUX_OWNER_NONE (state unchanged).
 *   - SS_MUXCTL_BUSY if the mux is already held (state unchanged); this
 *     includes a same-owner re-acquire (non-reentrant).
 *   - SS_MUXCTL_OK otherwise: st->owner = owner, st->mode = mode.
 */
ss_muxctl_result_t ss_muxctl_core_acquire(ss_muxctl_state_t *st, ss_mux_mode_t mode,
                                          ss_mux_owner_t owner);

/**
 * Release the mux previously acquired by `owner`.
 *
 * Pre:  st != NULL.
 * Post/return:
 *   - SS_MUXCTL_INVALID_ARG if owner == SS_MUX_OWNER_NONE (state unchanged).
 *   - SS_MUXCTL_NOT_OWNER if the mux is free or held by a different owner
 *     (state unchanged) — covers double-release and wrong-owner release.
 *   - SS_MUXCTL_OK otherwise: st->owner = SS_MUX_OWNER_NONE and st->mode
 *     reverts to st->default_mode.
 */
ss_muxctl_result_t ss_muxctl_core_release(ss_muxctl_state_t *st, ss_mux_owner_t owner);

/**
 * Unconditionally return the core to its resting state, ignoring ownership.
 * For watchdog / crash recovery.
 *
 * Pre:  st != NULL.
 * Post: st->owner == SS_MUX_OWNER_NONE, st->mode == st->default_mode.
 */
void ss_muxctl_core_force_release(ss_muxctl_state_t *st);
