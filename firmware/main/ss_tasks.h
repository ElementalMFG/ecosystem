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
// WATCHDOG POLICY (S-02-009)
// --------------------------
// The build arms the ESP-IDF Task Watchdog (TWDT, 5 s deadline, panic-on-expiry)
// and the Interrupt Watchdog (IWDT, ~300 ms) — see sdkconfig.defaults. Policy:
//
//   * Any task whose loop iterates with bounded latency STRICTLY UNDER the 5 s
//     TWDT deadline MUST subscribe (ss_task_wdt_register) and feed exactly once
//     per iteration (ss_task_wdt_feed); on exit it MUST unsubscribe
//     (ss_task_wdt_unregister). A hang then trips the TWDT and the board
//     recovers via panic -> reboot instead of silently wedging.
//   * Tasks with legitimately long blocking / sleep periods >= the deadline
//     (e.g. an event pump blocked on portMAX_DELAY, or a 30 s housekeeping
//     sleep) MUST NOT register — they would false-trip the TWDT. Each such task
//     MUST state, in a comment at its creation site, why it is exempt.
//   * The IWDT protects ISR latency / interrupts-disabled windows and needs no
//     application API; it is purely a config-level guard.
//
// These wrappers are thin, deliberate re-exports of esp_task_wdt_add/reset/
// delete for the calling task (handle == NULL) so that watchdog participation
// reads uniformly across firmware/main and stays greppable in one place.

#pragma once

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

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

// Subscribe the CALLING task to the Task Watchdog (see WATCHDOG POLICY above).
// Call once, from the task, before its bounded-latency loop begins.
//
// pre:  called from task context; the TWDT is initialised (CONFIG_ESP_TASK_WDT_
//       INIT=y, sdkconfig.defaults). Only qualifying tasks may call this.
// post: on success the calling task is monitored and MUST call ss_task_wdt_feed()
//       at least once every TWDT period (5 s) until it unregisters.
// error: forwards the esp_task_wdt_add() result — ESP_OK, ESP_ERR_INVALID_STATE
//        (TWDT not initialised) or ESP_ERR_NO_MEM (subscriber table full).
esp_err_t ss_task_wdt_register(void);

// Feed (reset) the calling task's Task Watchdog timer. Call exactly once per
// loop iteration in every task that registered.
//
// pre:  the calling task previously succeeded at ss_task_wdt_register().
// post: the calling task's TWDT deadline is reset to now + 5 s.
// error: none — esp_task_wdt_reset() cannot fail for a subscribed task; if the
//        task is not subscribed the reset is a no-op.
void ss_task_wdt_feed(void);

// Unsubscribe the CALLING task from the Task Watchdog. Call before a registered
// task returns / self-deletes.
//
// pre:  the calling task previously succeeded at ss_task_wdt_register().
// post: the calling task is no longer monitored by the TWDT.
// error: forwards the esp_task_wdt_delete() result — ESP_OK or
//        ESP_ERR_INVALID_STATE / ESP_ERR_NOT_FOUND if it was not subscribed.
esp_err_t ss_task_wdt_unregister(void);

#ifdef __cplusplus
}
#endif
