// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_storage_fs_core.h — pure, host-testable core for the LittleFS user-FS
// mount policy (S-03-019).
//
// SCOPE: this layer holds NO ESP-IDF and NO HAL dependency — it includes only
// <stdint.h>, <stdbool.h>, <stddef.h>. It reasons purely about a self-heal
// mount state machine and about usage/wear-stat arithmetic, so it can be
// exercised with a plain gcc host harness under -fsanitize=address,undefined
// with no include hacks and no libm. The on-target esp_littlefs glue lives in
// ss_storage.c and is compiled on-target only; it drives this FSM by feeding
// back the result of each real mount/format step.
//
// SELF-HEAL MOUNT FSM: the policy encodes "try mount → on failure format →
// remount → else fail" as a deterministic, BOUNDED state machine. The caller
// asks the core for the next step to perform, executes it against the real
// filesystem, and feeds the OK/ERR result back to ask for the next step. The
// machine tries FORMAT at most once and terminates in DONE_OK or FAILED — it
// can never loop forever. FAIL-SAFE: NULL/garbage input maps to FAILED so a
// confused caller stops rather than spins.
//
// TODO(EPIC-08): the user FS is PLAINTEXT for now. At-rest encryption lands
// with EPIC-08 flash-encryption; do not enable flash encryption here.

#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
// Self-heal mount FSM
// -----------------------------------------------------------------------------

// A step the caller is asked to perform (or a terminal outcome). The terminal
// values are distinct so the caller can loop `while step is MOUNT or FORMAT`.
typedef enum {
    SS_FS_STEP_MOUNT,   // perform a mount attempt, then feed the result back
    SS_FS_STEP_FORMAT,  // format the partition, then feed the result back
    SS_FS_STEP_DONE_OK, // terminal: filesystem is mounted and usable
    SS_FS_STEP_FAILED,  // terminal: unrecoverable, do not use the filesystem
} ss_fs_step_t;

// Result of the step the caller just performed.
typedef enum {
    SS_FS_R_OK,
    SS_FS_R_ERR,
} ss_fs_result_t;

// Bounded self-heal state. Treat as opaque; initialise with ss_fs_heal_init.
typedef struct {
    uint8_t mount_attempts; // number of MOUNT steps handed out so far (0..2)
    bool formatted;         // a FORMAT step has been handed out (at most once)
} ss_fs_heal_state_t;

// Initialise the self-heal state. NULL-safe (no-op on NULL).
// post: *state is a fresh machine whose first step is SS_FS_STEP_MOUNT.
void ss_fs_heal_init(ss_fs_heal_state_t* state);

// Advance the self-heal FSM (perform-then-advance convention).
//   `state` — machine state (mutated). NULL → SS_FS_STEP_FAILED (fail-safe).
//   `last`  — the step the caller JUST PERFORMED. The caller performs the first
//             step unconditionally (it is always a MOUNT), then calls this with
//             that step and its result to obtain the NEXT step.
//   `res`   — result of performing `last`.
// Returns the next step to perform, or a terminal SS_FS_STEP_DONE_OK /
// SS_FS_STEP_FAILED. Loop until the returned step is terminal; never "perform"
// a terminal step.
//
// Policy (bounded, format tried at most once):
//   MOUNT  ok                         → DONE_OK
//   MOUNT  err & not yet formatted     → FORMAT
//   MOUNT  err & already formatted     → FAILED   (2nd mount failed)
//   FORMAT ok                          → MOUNT    (2nd attempt)
//   FORMAT err                         → FAILED
// Any terminal `last`, garbage enum, or NULL state → FAILED (fail-safe).
ss_fs_step_t ss_fs_heal_next(ss_fs_heal_state_t* state, ss_fs_step_t last, ss_fs_result_t res);

// Convenience: is `step` a terminal outcome?
static inline bool ss_fs_step_is_terminal(ss_fs_step_t step)
{
    return step == SS_FS_STEP_DONE_OK || step == SS_FS_STEP_FAILED;
}

// -----------------------------------------------------------------------------
// Wear / usage stat aggregation
// -----------------------------------------------------------------------------

typedef struct {
    uint64_t total_bytes;
    uint64_t used_bytes;
} ss_fs_usage_t;

// Fill percentage 0..100. Div-by-zero safe: total==0 → 100 (treat an
// unavailable/empty-geometry FS as "full" so callers back off). Clamps
// used>total to 100.
uint8_t ss_fs_fill_pct(ss_fs_usage_t usage);

// Free bytes = total - used, saturating at 0 when used>total.
uint64_t ss_fs_free_bytes(ss_fs_usage_t usage);

#ifdef __cplusplus
} // extern "C"
#endif
