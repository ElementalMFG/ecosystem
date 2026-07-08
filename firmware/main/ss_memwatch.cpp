// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_memwatch.cpp — target glue for the periodic resource-telemetry task
// (S-02-011). The arithmetic and line formatting are pure and host-tested in
// ss_memwatch_core.c; this file owns the static task, the ESP-IDF heap/task
// queries, and the per-sample logging. See ss_memwatch.h for the contract.

#include "ss_memwatch.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_heap_caps.h"

#include "ss_log.h"
#include "ss_memwatch_core.h"
#include "ss_tasks.h"

static const char* TAG = "ss.mem";

// Static task footprint (no heap on the memwatch path — see ss_memwatch.h).
// StackType_t is uint8_t on ESP-IDF, so the buffer element count IS the byte
// depth passed to xTaskCreateStatic.
static constexpr uint32_t kStackBytes = 4096;

// Upper bound on tasks enumerated per sample. The snapshot buffer is static;
// if the live task count ever exceeds this, uxTaskGetSystemState returns 0 and
// we log a single WARN rather than enumerate a partial, misleading set.
static constexpr UBaseType_t kMaxTasks = 24;

static StackType_t s_stack[kStackBytes];
static StaticTask_t s_tcb;
static TaskHandle_t s_handle = nullptr;

// Per-sample snapshot + per-core idle-load trackers. Static so the sample path
// touches no heap and the idle baselines survive between periods.
static TaskStatus_t s_task_status[kMaxTasks];
static ss_memwatch_idle_state_t s_idle[2];

// FreeRTOS run state -> one-char code for the machine-parseable per-task line.
static char state_char(eTaskState s)
{
    switch (s) {
    case eRunning:
        return 'X';
    case eReady:
        return 'R';
    case eBlocked:
        return 'B';
    case eSuspended:
        return 'S';
    case eDeleted:
        return 'D';
    default:
        return '?';
    }
}

// Core index of an "IDLE0"/"IDLE1" task from its name, or -1 if not an idle
// task. Idle-load is attributed per core by name so no IDF-internal idle-handle
// API is required.
static int idle_core_from_name(const char* name)
{
    if (name != nullptr && name[0] == 'I' && name[1] == 'D' && name[2] == 'L' && name[3] == 'E') {
        if (name[4] == '0') { return 0; }
        if (name[4] == '1') { return 1; }
    }
    return -1;
}

static int task_core(TaskHandle_t h)
{
    const BaseType_t id = xTaskGetCoreID(h);
    return (id == tskNO_AFFINITY) ? -1 : (int)id;
}

// Append " <region-group>" to the summary line at *pos, advancing *pos.
// Returns true iff the region's free size is at/below the low-heap floor.
static bool append_region(char* line, size_t cap, int* pos, uint32_t caps, const char* name)
{
    ss_memwatch_region_t r;
    r.name = name;
    r.free_bytes = (uint32_t)heap_caps_get_free_size(caps);
    r.min_free_bytes = (uint32_t)heap_caps_get_minimum_free_size(caps);
    r.largest_free_block = (uint32_t)heap_caps_get_largest_free_block(caps);

    char grp[96];
    ss_memwatch_format_region(grp, sizeof(grp), &r);

    const int n = snprintf(line + *pos, cap - (size_t)*pos, " %s", grp);
    if (n > 0 && (size_t)(*pos + n) < cap) { *pos += n; }

    return ss_memwatch_heap_low(r.free_bytes, SS_MEMWATCH_HEAP_FLOOR_BYTES);
}

static void append_idle(char* line, size_t cap, int* pos, int core, bool avail, uint32_t pct)
{
    int n;
    if (avail) {
        n = snprintf(line + *pos, cap - (size_t)*pos, " idle%d=%u%%", core, (unsigned)pct);
    } else {
        n = snprintf(line + *pos, cap - (size_t)*pos, " idle%d=n/a", core);
    }
    if (n > 0 && (size_t)(*pos + n) < cap) { *pos += n; }
}

static void sample(void)
{
    // --- Heap regions: INTERNAL always; SPIRAM only when PSRAM is present ---
    // Presence is a runtime memory query (not a CONFIG_/board-macro test), so
    // the same binary self-describes whether PSRAM populated.
    char line[256];
    int pos = snprintf(line, sizeof(line), "memwatch:");
    if (pos < 0) { pos = 0; }

    bool heap_low = append_region(line, sizeof(line), &pos, MALLOC_CAP_INTERNAL, "internal");
    if (heap_caps_get_total_size(MALLOC_CAP_SPIRAM) > 0) {
        heap_low |= append_region(line, sizeof(line), &pos, MALLOC_CAP_SPIRAM, "spiram");
    }

    // --- Task snapshot: idle-load deltas + per-task watermarks --------------
    uint32_t total_runtime = 0;
    const UBaseType_t n_tasks = uxTaskGetSystemState(s_task_status, kMaxTasks, &total_runtime);

    uint32_t idle_pct[2] = {0, 0};
    bool idle_avail[2] = {false, false};

    if (n_tasks == 0) {
        // Snapshot buffer too small: enumerate nothing rather than mislead.
        SS_LOGW(TAG, "task snapshot skipped: live tasks exceed cap=%u", (unsigned)kMaxTasks);
    } else {
        for (UBaseType_t i = 0; i < n_tasks; i++) {
            const int core = idle_core_from_name(s_task_status[i].pcTaskName);
            if (core >= 0) {
                idle_avail[core] = ss_memwatch_idle_update(
                    &s_idle[core], (uint32_t)s_task_status[i].ulRunTimeCounter, total_runtime,
                    &idle_pct[core]);
            }
        }
    }

    append_idle(line, sizeof(line), &pos, 0, idle_avail[0], idle_pct[0]);
    append_idle(line, sizeof(line), &pos, 1, idle_avail[1], idle_pct[1]);

    if ((size_t)pos < sizeof(line)) {
        snprintf(line + pos, sizeof(line) - (size_t)pos, " tasks=%u", (unsigned)n_tasks);
    }

    if (heap_low) {
        SS_LOGW(TAG, "%s", line);
    } else {
        SS_LOGI(TAG, "%s", line);
    }

    // --- One machine-parseable line per task (WARN if stack margin low) -----
    for (UBaseType_t i = 0; i < n_tasks; i++) {
        const TaskStatus_t* ts = &s_task_status[i];
        ss_memwatch_task_line_t t;
        t.name = (ts->pcTaskName != nullptr) ? ts->pcTaskName : "?";
        t.stack_hwm_bytes = (uint32_t)ts->usStackHighWaterMark;
        t.prio = (uint32_t)ts->uxCurrentPriority;
        t.state = state_char(ts->eCurrentState);
        t.core = task_core(ts->xHandle);

        char tline[128];
        ss_memwatch_format_task(tline, sizeof(tline), &t);

        if (ss_memwatch_stack_low(t.stack_hwm_bytes, SS_MEMWATCH_STACK_FLOOR_BYTES)) {
            SS_LOGW(TAG, "%s", tline);
        } else {
            SS_LOGI(TAG, "%s", tline);
        }
    }
}

static void memwatch_task(void* arg)
{
    (void)arg;
    // TWDT-EXEMPT (ss_hal_watchdog.h WATCHDOG POLICY): this task sleeps for
    // CONFIG_SS_MEMWATCH_PERIOD_MS (default 10 s), which is >= the 5 s TWDT
    // deadline, so it deliberately does NOT subscribe — it would false-trip.
    const TickType_t period = pdMS_TO_TICKS(CONFIG_SS_MEMWATCH_PERIOD_MS);
    for (;;) {
        sample();
        vTaskDelay(period);
    }
}

void ss_memwatch_start(void)
{
    if (s_handle != nullptr) { return; } // idempotent (contract)

    // xTaskCreateStatic (not the ss_task_create wrapper) is mandated by the
    // no-heap design contract (ss_memwatch.h): the memwatch path must not
    // allocate. The priority still names the SS_PRIO_IDLE_MON band per the
    // RTOS policy (ss_tasks.h).
    s_handle = xTaskCreateStatic(memwatch_task, "ss_memwatch", kStackBytes, nullptr,
                                 UBaseType_t(SS_PRIO_IDLE_MON), s_stack, &s_tcb);
    if (s_handle == nullptr) {
        SS_LOGE(TAG, "memwatch task create failed"); // cannot happen with static buffers
    }
}
