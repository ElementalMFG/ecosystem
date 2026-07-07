// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_memwatch.h — periodic resource-telemetry task (S-02-011, T3).
//
// A low-priority background task that, once per CONFIG_SS_MEMWATCH_PERIOD_MS,
// logs three families of runtime health metrics through ss_log:
//
//   1. HEAP — free / minimum-free (low-water) / largest-free-block, plus a
//      derived fragmentation percentage, for each relevant heap_caps region
//      (MALLOC_CAP_INTERNAL always; MALLOC_CAP_SPIRAM when PSRAM is present).
//   2. STACK — per-task high-water mark (uxTaskGetSystemState +
//      uxTaskGetStackHighWaterMark), one machine-parseable line per task.
//   3. IDLE LOAD — the IDLE0/IDLE1 run-time-stats counter share of the total,
//      computed as a delta between consecutive samples.
//
// NO-HEAP PATH (design contract): the task's stack and TCB are static
// (xTaskCreateStatic), the task-status snapshot buffer is a fixed-size static
// array, and the per-sample work allocates nothing. A memory-watch tool that
// itself perturbs the heap it measures would be worse than useless, so the
// whole path is allocation-free by construction. The task count is bounded by
// SS_MEMWATCH_MAX_TASKS (ss_memwatch.cpp); tasks beyond that are not enumerated
// and a single WARN is logged.
//
// The arithmetic and line formatting are factored into the IDF-free,
// host-tested core (ss_memwatch_core.{h,c}); this header exposes only the
// target-side lifecycle entry point.

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Start the periodic memory-watch task.
//
// pre:  called once during boot, after ss_log is up and FreeRTOS is running
//       (i.e. from app_main). The build must enable configUSE_TRACE_FACILITY
//       and run-time stats (sdkconfig.defaults); without them the task-status
//       and idle-load queries degrade gracefully (zero/absent fields).
// post: a static-stack task named "ss_memwatch" runs at SS_PRIO_IDLE_MON and
//       logs one summary line plus per-task lines every
//       CONFIG_SS_MEMWATCH_PERIOD_MS. Idempotent: a second call is a no-op.
// error: none — static allocation cannot fail; a spurious failure is logged,
//        not propagated.
void ss_memwatch_start(void);

#ifdef __cplusplus
}
#endif
