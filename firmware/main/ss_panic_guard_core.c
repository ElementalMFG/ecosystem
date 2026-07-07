// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_panic_guard_core.c — pure crash-loop-breaker decision logic (S-02-008).
//
// IDF-free on purpose: this file is the audited, host-tested core of the
// safe-mode gate (firmware/test/host/tests/test_ss_panic_guard.cpp). Keep it
// free of ESP-IDF includes and side effects — the target glue lives in
// ss_panic_guard.cpp.

#include "ss_panic_guard.h"

// True iff *rec is internally consistent (magic + inverted-count shadow).
static bool record_valid(const ss_panic_record_t* rec)
{
    return rec->magic == SS_PANIC_RECORD_MAGIC && rec->inverse == ~rec->count;
}

static void record_set(ss_panic_record_t* rec, uint32_t count)
{
    rec->magic = SS_PANIC_RECORD_MAGIC;
    rec->count = count;
    rec->inverse = ~count;
}

void ss_panic_guard_record_clear(ss_panic_record_t* rec)
{
    record_set(rec, 0u);
}

ss_boot_decision_t ss_panic_guard_decide(ss_reset_class_t cls, ss_panic_record_t* rec)
{
    uint32_t count = record_valid(rec) ? rec->count : 0u;

    switch (cls) {
    case SS_RESET_CLASS_CRASH:
        if (count < UINT32_MAX) { count += 1u; }
        break;
    case SS_RESET_CLASS_BENIGN:
        count = 0u;
        break;
    case SS_RESET_CLASS_INDETERMINATE:
    default:
        // Fail-safe for a breaker: an unclassifiable reset neither counts as
        // a crash nor erases accumulated evidence of one.
        break;
    }

    record_set(rec, count);

    return (count >= SS_PANIC_SAFE_MODE_THRESHOLD) ? SS_BOOT_SAFE_MODE : SS_BOOT_NORMAL;
}
