// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_hal_watchdog.h — Task Watchdog (TWDT) participation contract.
//
// Contract of record since the S-03-039 reconciliation: the semantics that
// shipped with S-02-009 as firmware/main's ss_task_wdt_* wrappers are adopted
// here verbatim; this header's original, never-implemented ss_wdt_* sketch is
// deprecated below (see docs/DEPRECATIONS.md). Implementation:
// firmware/main/ss_wdt.c — thin re-exports of esp_task_wdt_add/reset/delete
// for the calling task (handle == NULL).
//
// RECONCILIATION DECISION (S-03-039, T1)
// --------------------------------------
// The shipped ss_task_wdt_* surface wins over the ss_wdt_* sketch because:
//
//   1. No runtime arming knob. ss_wdt_init(timeout_ms) let any component
//      re-arm the watchdog with a caller-chosen deadline. The TWDT deadline
//      and panic-on-expiry are build-time policy constants
//      (CONFIG_ESP_TASK_WDT_INIT=y, CONFIG_ESP_TASK_WDT_TIMEOUT_S=5,
//      CONFIG_ESP_TASK_WDT_PANIC=y — sdkconfig.defaults, S-02-009) that no
//      runtime code may weaken: a hang-recovery guarantee application code
//      can loosen is not a guarantee. The canonical contract therefore has
//      NO init/reconfigure entry point, by design.
//   2. Precise naming. Three watchdogs exist on this SoC (task WDT,
//      interrupt WDT, RTC WDT); ss_task_wdt_* names the one it feeds. The
//      IWDT is config-only policy with no application API; the RTC WDT
//      belongs to the boot path.
//   3. Shipped, reviewed, consumed. ss_task_wdt_* landed with the S-02-009
//      policy review, has an in-tree consumer (ss_compass) and on-target
//      crash tests (firmware/main/test/test_ss_wdt.c); ss_wdt_* was never
//      implemented — calling it has been a link error since inception.
//
// WATCHDOG POLICY (S-02-009 — normative home is this header)
// ----------------------------------------------------------
// The build arms the ESP-IDF Task Watchdog (TWDT, 5 s deadline,
// panic-on-expiry) and the Interrupt Watchdog (IWDT, ~300 ms) — see
// sdkconfig.defaults. Policy:
//
//   * Any task whose loop iterates with bounded latency STRICTLY UNDER the
//     5 s TWDT deadline MUST subscribe (ss_task_wdt_register) and feed
//     exactly once per iteration (ss_task_wdt_feed); on exit it MUST
//     unsubscribe (ss_task_wdt_unregister). A hang then trips the TWDT and
//     the board recovers via panic -> reboot instead of silently wedging.
//   * Tasks with legitimately long blocking / sleep periods >= the deadline
//     (e.g. an event pump blocked on portMAX_DELAY, or a 30 s housekeeping
//     sleep) MUST NOT register — they would false-trip the TWDT. Each such
//     task MUST state, in a comment at its creation site, why it is exempt.
//   * The IWDT protects ISR latency / interrupts-disabled windows and needs
//     no application API; it is purely a config-level guard.

#pragma once

#include "esp_err.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Subscribe the CALLING task to the Task Watchdog (see WATCHDOG POLICY
// above). Call once, from the task, before its bounded-latency loop begins.
//
// pre:  called from task context; the TWDT is initialised
//       (CONFIG_ESP_TASK_WDT_INIT=y, sdkconfig.defaults). Only qualifying
//       tasks may call this.
// post: on success the calling task is monitored and MUST call
//       ss_task_wdt_feed() at least once every TWDT period (5 s) until it
//       unregisters.
// error: forwards the esp_task_wdt_add() result — ESP_OK,
//        ESP_ERR_INVALID_STATE (TWDT not initialised), ESP_ERR_INVALID_ARG
//        (calling task already subscribed) or ESP_ERR_NO_MEM (subscriber
//        table full).
esp_err_t ss_task_wdt_register(void);

// Feed (reset) the calling task's Task Watchdog timer. Call exactly once per
// loop iteration in every task that registered.
//
// pre:  the calling task previously succeeded at ss_task_wdt_register().
// post: the calling task's TWDT deadline is reset to now + 5 s.
// error: none — esp_task_wdt_reset() cannot fail for a subscribed task; if
//        the task is not subscribed the reset is a no-op.
void ss_task_wdt_feed(void);

// Unsubscribe the CALLING task from the Task Watchdog. Call before a
// registered task returns / self-deletes.
//
// pre:  the calling task previously succeeded at ss_task_wdt_register().
// post: the calling task is no longer monitored by the TWDT.
// error: forwards the esp_task_wdt_delete() result — ESP_OK,
//        ESP_ERR_INVALID_STATE (TWDT not initialised) or ESP_ERR_NOT_FOUND
//        (calling task was not subscribed).
esp_err_t ss_task_wdt_unregister(void);

// ============================================================================
// DEPRECATED — losing surface of the S-03-039 reconciliation.
// ============================================================================
// The original ss_wdt_* sketch below was never implemented anywhere in-tree;
// it is kept ONLY as compiler-visible tombstones so the migration stays loud
// rather than silent (docs/DEPRECATIONS.md). On the pinned firmware
// toolchain (GCC, RFC-0002) any CALL to a tombstone is a hard compile error
// via __attribute__((error)) — deliberately flag-independent, because
// ESP-IDF's default build spec appends -Wno-error=deprecated-declarations,
// so the deprecated attribute alone would only warn. `deprecated` is kept
// alongside for IDE / clang-based tooling, which ignores the GCC error
// attribute. Migration:
//
//   ss_wdt_init        -> no replacement; the TWDT is armed at build time
//                         (see WATCHDOG POLICY above) — a runtime timeout
//                         knob is rejected policy.
//   ss_wdt_subscribe   -> ss_task_wdt_register
//   ss_wdt_feed        -> ss_task_wdt_feed  (returns void; see its contract)
//   ss_wdt_unsubscribe -> ss_task_wdt_unregister

#if defined(__GNUC__) && !defined(__clang__)
#define SS_WDT_TOMBSTONE(msg) __attribute__((deprecated(msg), error(msg)))
#else
#define SS_WDT_TOMBSTONE(msg) __attribute__((deprecated(msg)))
#endif

SS_WDT_TOMBSTONE("S-03-039: no replacement; TWDT is config-armed at build time")
esp_err_t ss_wdt_init(uint32_t timeout_ms);

SS_WDT_TOMBSTONE("S-03-039: use ss_task_wdt_register")
esp_err_t ss_wdt_subscribe(void);

SS_WDT_TOMBSTONE("S-03-039: use ss_task_wdt_unregister")
esp_err_t ss_wdt_unsubscribe(void);

SS_WDT_TOMBSTONE("S-03-039: use ss_task_wdt_feed")
esp_err_t ss_wdt_feed(void);

#undef SS_WDT_TOMBSTONE // header-local; the attributes bind at declaration

#ifdef __cplusplus
} // extern "C"
#endif
