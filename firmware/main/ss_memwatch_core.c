// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_memwatch_core.c — pure resource-telemetry math + line formatting
//                      (S-02-011).
//
// IDF-free on purpose: this file is the host-tested core of the memory-watch
// task (firmware/test/host/tests/test_ss_memwatch.cpp). Keep it free of
// ESP-IDF includes and side effects — the target glue lives in ss_memwatch.cpp.

#include "ss_memwatch_core.h"

#include <stdio.h>

// Round a non-negative rational a/b to the nearest integer without floating
// point. Callers guarantee b != 0.
static uint32_t div_round(uint64_t a, uint64_t b)
{
    return (uint32_t)((a + b / 2u) / b);
}

uint32_t ss_memwatch_frag_pct(uint32_t free_bytes, uint32_t largest_free_block)
{
    if (free_bytes == 0u) { return 0u; }
    // Clamp: the largest block can never exceed total free; if a caller passes
    // a skewed pair, report 0% rather than underflow the subtraction.
    if (largest_free_block >= free_bytes) { return 0u; }

    const uint32_t fragmented = free_bytes - largest_free_block;
    return div_round((uint64_t)fragmented * 100u, (uint64_t)free_bytes);
}

bool ss_memwatch_heap_low(uint32_t free_bytes, uint32_t floor_bytes)
{
    return free_bytes <= floor_bytes;
}

uint32_t ss_memwatch_u32_delta(uint32_t prev, uint32_t now)
{
    // Unsigned wraparound is well-defined and yields the elapsed count across
    // at most one wrap of the 32-bit counter.
    return now - prev;
}

uint32_t ss_memwatch_idle_pct(uint32_t idle_delta, uint32_t total_delta)
{
    if (total_delta == 0u) { return 0u; }
    if (idle_delta >= total_delta) { return 100u; }
    return div_round((uint64_t)idle_delta * 100u, (uint64_t)total_delta);
}

bool ss_memwatch_idle_update(ss_memwatch_idle_state_t* st, uint32_t idle_now, uint32_t total_now,
                             uint32_t* out_idle_pct)
{
    const bool had_baseline = st->valid;
    uint32_t pct = 0u;

    if (had_baseline) {
        const uint32_t idle_delta = ss_memwatch_u32_delta(st->idle_counter, idle_now);
        const uint32_t total_delta = ss_memwatch_u32_delta(st->total_counter, total_now);
        pct = ss_memwatch_idle_pct(idle_delta, total_delta);
    }

    st->idle_counter = idle_now;
    st->total_counter = total_now;
    st->valid = true;

    if (had_baseline) {
        *out_idle_pct = pct;
        return true;
    }
    return false;
}

int ss_memwatch_format_region(char* buf, size_t cap, const ss_memwatch_region_t* r)
{
    const uint32_t frag = ss_memwatch_frag_pct(r->free_bytes, r->largest_free_block);
    return snprintf(buf, cap, "%s[free=%u min=%u largest=%u frag=%u%%]", r->name,
                    (unsigned)r->free_bytes, (unsigned)r->min_free_bytes,
                    (unsigned)r->largest_free_block, (unsigned)frag);
}

int ss_memwatch_format_task(char* buf, size_t cap, const ss_memwatch_task_line_t* t)
{
    return snprintf(buf, cap, "memwatch-task: name=%s prio=%u stack_hwm=%u state=%c core=%d",
                    t->name, (unsigned)t->prio, (unsigned)t->stack_hwm_bytes, t->state, t->core);
}

bool ss_memwatch_stack_low(uint32_t stack_hwm_bytes, uint32_t floor_bytes)
{
    return stack_hwm_bytes <= floor_bytes;
}
