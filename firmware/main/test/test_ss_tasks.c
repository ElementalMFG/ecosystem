// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// test_ss_tasks.c — RTOS baseline tests for the priority-ceiling policy and
//                   stack-canary detection (S-02-006).
//
// NOTE: This file is intentionally NOT part of the firmware image build.
// firmware/main/CMakeLists.txt lists SRCS explicitly (no glob), so this
// directory is invisible to `make lite`. It is written to compile and run
// under the on-target Unity test runner that lands with S-02-014/015; until
// then it is documentation-grade and never executed.

#include "unity.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "ss_tasks.h"

// -----------------------------------------------------------------------------
// Case 1: the RTOS tick rate is the pinned 1 kHz baseline (sdkconfig.defaults).
// A pure compile-time constant check — safe to run anywhere.
// -----------------------------------------------------------------------------
TEST_CASE("freertos tick rate is 1000 Hz", "[ss_tasks]")
{
    TEST_ASSERT_EQUAL(1000, configTICK_RATE_HZ);
}

// -----------------------------------------------------------------------------
// Case 2: stack-overflow detection.
//
// A deliberately stack-hungry task blows a small (~1 KiB) stack. With
// CONFIG_FREERTOS_CHECK_STACKOVERFLOW_CANARY=y the kernel's canary check must
// trip and panic (the S-02-008 flash-dump path captures the post-mortem).
//
// WARNING: this task PANICS the device by design. It requires the S-02-014/015
// on-target test runner (which asserts on the resulting panic / reset reason);
// it must NOT be linked into the normal firmware image. Guarded off by default.
// -----------------------------------------------------------------------------
#ifdef SS_TASKS_RUN_OVERFLOW_TEST
static void stack_hog_task(void* arg)
{
    (void)arg;
    // Touch far more stack than allocated so the canary is overwritten; marked
    // volatile so the compiler cannot optimise the array away.
    volatile uint8_t blowout[4096];
    for (size_t i = 0; i < sizeof(blowout); ++i) { blowout[i] = (uint8_t)i; }
    // Never reached — the canary check panics before we return.
    for (;;) { vTaskDelay(1); }
}

TEST_CASE("stack canary trips on overflow", "[ss_tasks][overflow]")
{
    // 1 KiB stack vs a 4 KiB local array -> guaranteed overflow -> panic.
    // Created through the policy wrapper (SS_PRIO_IDLE_MON == idle+1) so even
    // the test tree obeys the priority-ceiling policy.
    const BaseType_t ok =
        ss_task_create(stack_hog_task, "ss_stack_hog", 1024, NULL, SS_PRIO_IDLE_MON, NULL);
    TEST_ASSERT_EQUAL(pdPASS, ok);

    // The test runner (S-02-014/015) expects the ensuing canary panic; control
    // does not return here in a real run. Yield to let the hog run.
    vTaskDelay(pdMS_TO_TICKS(100));
    TEST_FAIL_MESSAGE("stack canary did not trip");
}
#endif // SS_TASKS_RUN_OVERFLOW_TEST
