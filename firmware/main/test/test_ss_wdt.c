// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// test_ss_wdt.c — task / interrupt watchdog policy tests (S-02-009).
//
// NOTE: This file is intentionally NOT part of the firmware image build.
// firmware/main/CMakeLists.txt lists SRCS explicitly (no glob), so this
// directory is invisible to `make lite`. It is written to compile and run
// under the on-target Unity test runner that lands with S-02-014/015; until
// then it is documentation-grade and never executed.
//
// WARNING: EVERY case below CRASHES the target BY DESIGN — the whole point is to
// prove the watchdogs recover a wedged system via panic -> reboot. They require
// the S-02-014/015 on-target runner (hardware or QEMU), which asserts on the
// resulting panic / reset reason. They must NOT be linked into a normal image
// and are guarded off by default.

#include "unity.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "ss_tasks.h"

// -----------------------------------------------------------------------------
// Case 1: Task Watchdog (TWDT) trips on a task that stops feeding.
//
// The task subscribes through the policy API, then busy-loops WITHOUT calling
// ss_task_wdt_feed(). With CONFIG_ESP_TASK_WDT_TIMEOUT_S=5 and
// CONFIG_ESP_TASK_WDT_PANIC=y (sdkconfig.defaults) the TWDT must fire after ~5 s
// and panic -> reboot.
//
// Expected observable (asserted by the S-02-014/015 runner): a "Task watchdog
// got triggered" report naming this task, followed by a panic and a reset whose
// reason is the task watchdog. Control never returns from the busy loop.
// -----------------------------------------------------------------------------
#ifdef SS_WDT_RUN_HANG_TESTS
static void twdt_hang_task(void* arg)
{
    (void)arg;
    // Subscribe, then deliberately never feed: the TWDT deadline elapses.
    TEST_ASSERT_EQUAL(ESP_OK, ss_task_wdt_register());
    // Spin without feeding or yielding. Marked volatile so the compiler cannot
    // optimise the loop away; the TWDT panics before this ever terminates.
    volatile uint32_t spin = 0;
    for (;;) { spin++; }
}

TEST_CASE("task watchdog panics on unfed task", "[ss_wdt][hang]")
{
    // Created through the policy wrapper (SS_PRIO_IDLE_MON == idle+1) so even the
    // test tree obeys the priority-ceiling policy.
    const BaseType_t ok =
        ss_task_create(twdt_hang_task, "ss_twdt_hang", 2048, NULL, SS_PRIO_IDLE_MON, NULL);
    TEST_ASSERT_EQUAL(pdPASS, ok);

    // The runner (S-02-014/015) expects the ensuing TWDT panic (~5 s); control
    // does not return here in a real run. Wait out the deadline with margin.
    vTaskDelay(pdMS_TO_TICKS(7000));
    TEST_FAIL_MESSAGE("task watchdog did not trip");
}

// -----------------------------------------------------------------------------
// Case 2: Interrupt Watchdog (IWDT) trips on interrupts disabled too long.
//
// With interrupts disabled the FreeRTOS tick ISR can no longer feed the IWDT;
// after ~300 ms (IDF-default IWDT timeout, CONFIG_ESP_INT_WDT=y) the interrupt
// watchdog must fire and panic -> reboot. This is orthogonal to the TWDT: no
// task-level feed can save a core whose interrupts are off.
//
// Expected observable (asserted by the S-02-014/015 runner): an "Interrupt wdt
// timeout" panic and a reset whose reason is the interrupt watchdog. Control
// never returns from the spin.
// -----------------------------------------------------------------------------
static void iwdt_hang_task(void* arg)
{
    (void)arg;
    // Kill interrupts on this core and spin: the tick ISR can no longer feed the
    // IWDT, so it must fire within ~300 ms.
    taskDISABLE_INTERRUPTS();
    volatile uint32_t spin = 0;
    for (;;) { spin++; }
}

TEST_CASE("interrupt watchdog panics on interrupts disabled", "[ss_wdt][hang]")
{
    const BaseType_t ok =
        ss_task_create(iwdt_hang_task, "ss_iwdt_hang", 2048, NULL, SS_PRIO_IDLE_MON, NULL);
    TEST_ASSERT_EQUAL(pdPASS, ok);

    // The runner expects the ensuing IWDT panic (~300 ms); control does not
    // return here in a real run.
    vTaskDelay(pdMS_TO_TICKS(2000));
    TEST_FAIL_MESSAGE("interrupt watchdog did not trip");
}
#endif // SS_WDT_RUN_HANG_TESTS
