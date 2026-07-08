// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_tasks.h — FreeRTOS priority-ceiling policy + task-creation wrappers
//              (S-02-006, RTOS baseline).
//
// PRIORITY-CEILING POLICY
// -----------------------
// Every application task is assigned exactly one of a small set of named
// priority bands. This keeps the scheduler's relative ordering legible in one
// place and reserves the top of the range for ESP-IDF's own system tasks
// (wifi / bt / esp_timer service / ipc, etc.) so an application task can never
// out-prioritise or starve them.
//
//   band                     prio  use
//   -----------------------  ----  ------------------------------------------
//   SS_PRIO_IDLE_MON            1   background monitors (just above idle)
//   SS_PRIO_HOUSEKEEPING        5   diagnostics / power watchdog
//   SS_PRIO_SENSOR              8   compass / IMU sample pumps
//   SS_PRIO_COMMS              12   GNSS / radio byte pumps
//   SS_PRIO_COMMS_CRITICAL     14   coprocessor / time-critical pumps
//   SS_PRIO_CEILING            18   application ceiling — priorities STRICTLY
//                                   ABOVE this are reserved for IDF system
//                                   tasks and MUST NOT be used by app code.
//
// All application tasks MUST be created through ss_task_create() or
// ss_task_create_pinned(); a raw xTaskCreate/xTaskCreatePinnedToCore call
// anywhere in firmware/main (outside this file) is rejected by
// tools/task-policy-check.py (CI gate). Passing a bare numeric-literal
// priority to the wrappers is likewise rejected — always name a band.
//
// STACK-OVERFLOW PROTECTION
// -------------------------
// The build enables the FreeRTOS stack canary check
// (CONFIG_FREERTOS_CHECK_STACKOVERFLOW_CANARY=y, sdkconfig.defaults). On a
// task-stack overflow the kernel's canary comparison fails and the system
// panics through the FreeRTOS stack-overflow hook. Structured capture of that
// panic (flash-dump / post-mortem) lands with S-02-008; this story guarantees
// only the detection + panic path. ESP-IDF v5.3 exposes no app-level runtime
// overflow-callback registration API, so none is invented or used here — the
// config-level canary plus this documented panic path is the deliverable.
//
// WATCHDOG PARTICIPATION (S-02-009 / S-03-039)
// --------------------------------------------
// The task-watchdog participation contract (ss_task_wdt_register/feed/
// unregister) and its who-must-subscribe policy live in the HAL contract of
// record, ss_hal_watchdog.h — adopted there by the S-03-039 reconciliation,
// semantics unchanged from S-02-009. Included below so existing ss_tasks.h
// call sites keep compiling unchanged.

#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "ss_hal_watchdog.h" // watchdog participation contract (see above)

#ifdef __cplusplus
extern "C" {
#endif

// Named application priority bands (see the policy table above). Values are the
// raw FreeRTOS task priorities; everything strictly above SS_PRIO_CEILING is
// reserved for ESP-IDF system tasks.
typedef enum {
    SS_PRIO_IDLE_MON = 1,        // background monitors (just above idle)
    SS_PRIO_HOUSEKEEPING = 5,    // diagnostics / power watchdog
    SS_PRIO_SENSOR = 8,          // compass / IMU sample pumps
    SS_PRIO_COMMS = 12,          // GNSS / radio byte pumps
    SS_PRIO_COMMS_CRITICAL = 14, // coprocessor / time-critical pumps
    SS_PRIO_CEILING = 18,        // application ceiling (system tasks live above)
} ss_task_prio_t;

// Create an application task at a named priority band (wraps xTaskCreate).
//
// pre:  `prio` is a valid band, i.e. 0 < prio <= SS_PRIO_CEILING (checked with
//       a runtime assert); `task` and `name` are non-null.
// post: on success a task is scheduled and, if `out` is non-null, its handle is
//       written there; all remaining arguments are forwarded unchanged.
// error: returns pdPASS on success or errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY
//        when the task/stack allocation fails (as per xTaskCreate).
BaseType_t ss_task_create(TaskFunction_t task, const char* name, uint32_t stack_depth, void* arg,
                          ss_task_prio_t prio, TaskHandle_t* out);

// Create an application task pinned to a core at a named priority band
// (wraps xTaskCreatePinnedToCore).
//
// pre:  as ss_task_create(); `core_id` is a valid core index or tskNO_AFFINITY.
// post: as ss_task_create(), with the task pinned to `core_id`.
// error: returns pdPASS on success or errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY
//        when the task/stack allocation fails (as per xTaskCreatePinnedToCore).
BaseType_t ss_task_create_pinned(TaskFunction_t task, const char* name, uint32_t stack_depth,
                                 void* arg, ss_task_prio_t prio, TaskHandle_t* out,
                                 BaseType_t core_id);

#ifdef __cplusplus
}
#endif
