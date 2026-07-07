// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_tasks.cpp — priority-ceiling task-creation wrappers (S-02-006).
//
// This is the ONLY translation unit in firmware/main permitted to call the raw
// xTaskCreate / xTaskCreatePinnedToCore APIs; tools/task-policy-check.py
// enforces that. See ss_tasks.h for the priority-band policy and rationale.

#include "ss_tasks.h"

#include <assert.h>

// Compile-time sanity: the bands must stay ordered and within the ceiling, and
// the ceiling must leave headroom below configMAX_PRIORITIES for IDF system
// tasks. If any of these trip, the policy table in ss_tasks.h drifted.
static_assert(SS_PRIO_IDLE_MON > 0, "idle-monitor band must be above the idle task");
static_assert(SS_PRIO_IDLE_MON < SS_PRIO_HOUSEKEEPING, "bands must be strictly ordered");
static_assert(SS_PRIO_HOUSEKEEPING < SS_PRIO_SENSOR, "bands must be strictly ordered");
static_assert(SS_PRIO_SENSOR < SS_PRIO_COMMS, "bands must be strictly ordered");
static_assert(SS_PRIO_COMMS < SS_PRIO_COMMS_CRITICAL, "bands must be strictly ordered");
static_assert(SS_PRIO_COMMS_CRITICAL < SS_PRIO_CEILING, "bands must be strictly ordered");
static_assert(SS_PRIO_CEILING < configMAX_PRIORITIES,
              "application ceiling must leave headroom for IDF system tasks");

BaseType_t ss_task_create(TaskFunction_t task, const char* name, uint32_t stack_depth, void* arg,
                          ss_task_prio_t prio, TaskHandle_t* out)
{
    assert(prio > 0 && prio <= SS_PRIO_CEILING);
    return xTaskCreate(task, name, stack_depth, arg, UBaseType_t(prio), out);
}

BaseType_t ss_task_create_pinned(TaskFunction_t task, const char* name, uint32_t stack_depth,
                                 void* arg, ss_task_prio_t prio, TaskHandle_t* out,
                                 BaseType_t core_id)
{
    assert(prio > 0 && prio <= SS_PRIO_CEILING);
    return xTaskCreatePinnedToCore(task, name, stack_depth, arg, UBaseType_t(prio), out, core_id);
}
