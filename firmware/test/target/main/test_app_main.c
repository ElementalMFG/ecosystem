// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// test_app_main.c — entry point for the on-target Unity test app (S-02-015).
//
// The ESP-IDF Unity component auto-registers every TEST_CASE() in the linked
// test sources. app_main runs them all and prints the standard Unity summary
// ("N Tests M Failures K Ignored" / "OK" | "FAIL") over the console serial;
// pytest-embedded's dut.expect_unity_test_output() parses exactly that summary
// in CI (see ../pytest_ss_target.py).
//
// Auto-run (not the interactive unity_run_menu()) is deliberate so a plain
// `idf.py flash monitor` — and an unattended CI runner — both get a complete,
// self-terminating result without any serial input.

#include "unity.h"

void app_main(void)
{
    UNITY_BEGIN();
    unity_run_all_tests();
    UNITY_END();
}
