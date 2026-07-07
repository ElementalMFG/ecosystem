// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_recovery_core.c — pure decision logic for the safe-mode / recovery boot
// path (S-02-016). IDF-free on purpose: this file is the audited, host-tested
// core (firmware/test/host/tests/test_ss_recovery.cpp) of the recovery request
// flag, the BOOT-button hold detector, the rollback precondition verdict, and
// the factory-reset erase plan. Keep it free of ESP-IDF includes and side
// effects — the target glue lives in ss_recovery.cpp.

#include "ss_recovery.h"

#include <stddef.h>

// ---- Request flag (magic + inverted-shadow, like ss_panic_record_t) --------

bool ss_recovery_flag_pending(const ss_recovery_flag_t* flag)
{
    return flag->magic == SS_RECOVERY_FLAG_MAGIC && flag->inverse == ~flag->magic;
}

void ss_recovery_flag_arm(ss_recovery_flag_t* flag)
{
    flag->magic = SS_RECOVERY_FLAG_MAGIC;
    flag->inverse = ~(uint32_t)SS_RECOVERY_FLAG_MAGIC;
}

void ss_recovery_flag_clear(ss_recovery_flag_t* flag)
{
    // Any state that fails the magic/shadow test reads as "no request"; zero it
    // so cold-boot garbage or a torn write can never resurrect a stale request.
    flag->magic = 0u;
    flag->inverse = 0u;
}

// ---- BOOT-button hold detector FSM -----------------------------------------

void ss_recovery_hold_init(ss_recovery_hold_t* h, uint32_t start_ms, uint32_t window_ms,
                           uint32_t hold_ms)
{
    h->state = SS_RECOVERY_HOLD_IDLE;
    h->window_deadline_ms = start_ms + window_ms;
    h->press_start_ms = 0u;
    h->hold_ms = hold_ms;
}

ss_recovery_hold_state_t ss_recovery_hold_step(ss_recovery_hold_t* h, bool pressed, uint32_t now_ms)
{
    // TRIGGERED and EXPIRED are terminal: further samples are no-ops.
    if (h->state == SS_RECOVERY_HOLD_TRIGGERED || h->state == SS_RECOVERY_HOLD_EXPIRED) {
        return h->state;
    }

    if (pressed) {
        if (h->state == SS_RECOVERY_HOLD_IDLE) {
            // Rising edge: begin accumulating a fresh unbroken hold.
            h->state = SS_RECOVERY_HOLD_ARMED;
            h->press_start_ms = now_ms;
        }
        // A completed hold wins over the deadline: a user who finished the
        // gesture gets recovery even at the exact instant the window closes.
        if (now_ms - h->press_start_ms >= h->hold_ms) {
            h->state = SS_RECOVERY_HOLD_TRIGGERED;
            return h->state;
        }
    } else {
        // Any release discards the accumulated hold (must be UNBROKEN).
        h->state = SS_RECOVERY_HOLD_IDLE;
    }

    // Window expiry retires the detector for the rest of this boot.
    if (now_ms >= h->window_deadline_ms) { h->state = SS_RECOVERY_HOLD_EXPIRED; }
    return h->state;
}

// ---- Rollback precondition verdict -----------------------------------------

ss_recovery_rb_verdict_t ss_recovery_rollback_eval(const ss_recovery_rb_inputs_t* in)
{
    // Refusal precedence (documented in ss_recovery.h): report the earliest
    // failing stage so the user sees the root cause, never a downstream symptom.
    if (!in->other_slot_exists) { return SS_RECOVERY_RB_NO_OTHER_SLOT; }
    if (!in->other_slot_has_app) { return SS_RECOVERY_RB_EMPTY_SLOT; }
    if (!in->set_boot_ok) { return SS_RECOVERY_RB_VERIFY_FAILED; }
    return SS_RECOVERY_RB_OK;
}

// ---- Factory-reset erase plan (data partitions ONLY) -----------------------
// Host-tested invariant: MUST NOT name otadata/ota_0/ota_1 (firmware slots are
// rollback's job, not factory reset). Order is erase order.
const char* const ss_recovery_factory_plan[] = {"nvs", "storage", "coredump", NULL};
