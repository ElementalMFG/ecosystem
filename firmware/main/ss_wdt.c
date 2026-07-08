// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_wdt.c — Task Watchdog participation wrappers (S-02-009 policy; contract
// of record: ss_hal_watchdog.h since the S-03-039 reconciliation).
//
// Thin, deliberate re-exports of esp_task_wdt_add/reset/delete for the
// calling task (handle == NULL) so watchdog participation reads uniformly
// across firmware/main and a subscribing task never has to hold its own
// handle. A C translation unit (not .cpp) so the contract-audit gate
// (tools/contract-audit.py), which scans firmware/**/*.c, sees these
// column-0 definitions.

#include "ss_hal_watchdog.h"

#include <stddef.h>

#include "esp_task_wdt.h"

esp_err_t ss_task_wdt_register(void)
{
    return esp_task_wdt_add(NULL);
}

void ss_task_wdt_feed(void)
{
    esp_task_wdt_reset();
}

esp_err_t ss_task_wdt_unregister(void)
{
    return esp_task_wdt_delete(NULL);
}
