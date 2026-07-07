// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_memwatch_core.h — pure resource-telemetry math + line formatting
//                      (S-02-011, T3).
//
// CONTRACT
// --------
// This is the IDF-free, host-tested core of the memory-watch task
// (ss_memwatch.cpp): it owns every non-trivial arithmetic and string decision
// so the target glue can stay a thin drain of ESP-IDF heap/task APIs into
// these functions. Keep it free of ESP-IDF includes and side effects — the
// target glue (heap_caps_*, uxTaskGetSystemState, xTaskCreateStatic) lives in
// ss_memwatch.cpp, and only these functions are exercised by
// firmware/test/host/tests/test_ss_memwatch.cpp.
//
// Three families of pure logic back the three acceptance clauses:
//   1. Heap fragmentation math + a low-heap threshold predicate.
//   2. Idle-load deltas over 32-bit run-time-stats counters that WRAP, with an
//      explicit first-sample (no baseline yet) case.
//   3. Machine-parseable line formatting (one summary field-group per heap
//      region, one line per task) plus a low-stack threshold predicate.
//
// LINE GRAMMAR (scraped by host tooling; keep stable — see ss_bootmark.h for
// the sibling `boot-report:` convention):
//   region field-group : <name>[free=<u> min=<u> largest=<u> frag=<u>%]
//   per-task line       : memwatch-task: name=<s> prio=<u> stack_hwm=<u>
//                         state=<c> core=<d>   (single line on the wire)
// The summary line (`memwatch:` prefix + region groups + idle fields) is
// assembled in the target glue from these field-groups.

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// ---- Thresholds (shared by core + target glue) ----------------------------

// Free-stack floor (bytes): a task whose stack high-water mark is at or below
// this margin is reported at WARN and flagged in its line. 512 B leaves room
// for one more nested call + ISR frame before the canary would trip.
#define SS_MEMWATCH_STACK_FLOOR_BYTES 512u

// Low-heap floor (bytes): a region whose free size is at or below this is
// reported at WARN. Chosen well above a single allocation so the warning
// precedes, not accompanies, an allocation failure.
#define SS_MEMWATCH_HEAP_FLOOR_BYTES 8192u

// ---- 1. Heap fragmentation ------------------------------------------------

// Fragmentation percentage of a heap region: how much of the free space is
// NOT available as one contiguous block, i.e.
//   round(100 * (free - largest_free_block) / free).
//
// pre:  none. `largest_free_block` is expected to be <= `free_bytes`; a
//       larger value (should never happen) is clamped so the result is 0.
// post: returns a value in [0, 100]. `free_bytes == 0` returns 0 (no free
//       memory means no fragmentation to report, not a divide-by-zero).
// error: none.
uint32_t ss_memwatch_frag_pct(uint32_t free_bytes, uint32_t largest_free_block);

// Low-heap predicate. pre: none. post: true iff `free_bytes <= floor_bytes`.
// error: none.
bool ss_memwatch_heap_low(uint32_t free_bytes, uint32_t floor_bytes);

// ---- 2. Idle-load deltas over wrapping 32-bit counters ---------------------

// Per-core idle-load tracker. The run-time-stats counters are unsigned 32-bit
// and wrap; `valid` is false until the first sample establishes a baseline.
typedef struct {
    uint32_t idle_counter;  // last sampled idle-task run-time counter
    uint32_t total_counter; // last sampled total run-time counter
    bool valid;             // false until the first update() call
} ss_memwatch_idle_state_t;

// Modular 32-bit counter delta. pre: none. post: returns (now - prev) computed
// in unsigned 32-bit arithmetic, i.e. the elapsed count across at most one
// wrap of the counter. error: none.
uint32_t ss_memwatch_u32_delta(uint32_t prev, uint32_t now);

// Idle percentage from a pair of deltas:
//   round(100 * idle_delta / total_delta), clamped to [0, 100].
// pre: none. post: `total_delta == 0` returns 0; an `idle_delta` exceeding
// `total_delta` (counter skew) is clamped to 100. error: none.
uint32_t ss_memwatch_idle_pct(uint32_t idle_delta, uint32_t total_delta);

// Fold a fresh (idle, total) sample into `st` and, when a baseline already
// exists, compute this interval's idle percentage.
//
// pre:  `st` non-NULL; `out_idle_pct` non-NULL. On the very first call for a
//       core, `st->valid` must be false (zero-initialised state suffices).
// post: `*st` is updated to the new sample and marked valid. Returns true and
//       writes `*out_idle_pct` (see ss_memwatch_idle_pct) when a prior
//       baseline existed; returns false on the first sample (no interval yet)
//       and leaves `*out_idle_pct` unwritten.
// error: none.
bool ss_memwatch_idle_update(ss_memwatch_idle_state_t* st, uint32_t idle_now, uint32_t total_now,
                             uint32_t* out_idle_pct);

// ---- 3. Line formatting + stack threshold ----------------------------------

// One heap region's telemetry (target glue fills this from heap_caps_*).
typedef struct {
    const char* name;            // region label, e.g. "internal" / "spiram"
    uint32_t free_bytes;         // currently free
    uint32_t min_free_bytes;     // minimum-ever free (low-water)
    uint32_t largest_free_block; // largest contiguous free block
} ss_memwatch_region_t;

// One task's watermark telemetry (target glue fills this from TaskStatus_t).
typedef struct {
    const char* name;         // task name (never NULL; glue substitutes "?")
    uint32_t stack_hwm_bytes; // minimum-ever free stack, in bytes
    uint32_t prio;            // current priority
    char state;               // one-char run state (see ss_memwatch.cpp)
    int core;                 // pinned core id, or -1 for no affinity
} ss_memwatch_task_line_t;

// Format a region field-group into `buf`:
//   <name>[free=<u> min=<u> largest=<u> frag=<u>%]
//
// pre:  `buf` non-NULL with capacity `cap` > 0; `r` non-NULL with non-NULL
//       `r->name`.
// post: writes a NUL-terminated string and returns the number of characters
//       that WOULD have been written excluding the NUL (snprintf semantics);
//       a return >= cap means the output was truncated.
// error: none.
int ss_memwatch_format_region(char* buf, size_t cap, const ss_memwatch_region_t* r);

// Format a full per-task line into `buf`:
//   memwatch-task: name=<s> prio=<u> stack_hwm=<u> state=<c> core=<d>
//
// pre/post/error: as ss_memwatch_format_region, with `t` (non-NULL, non-NULL
// name) in place of `r`.
int ss_memwatch_format_task(char* buf, size_t cap, const ss_memwatch_task_line_t* t);

// Low-stack predicate. pre: none. post: true iff
// `stack_hwm_bytes <= floor_bytes`. error: none.
bool ss_memwatch_stack_low(uint32_t stack_hwm_bytes, uint32_t floor_bytes);

#ifdef __cplusplus
}
#endif
