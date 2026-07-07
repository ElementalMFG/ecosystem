// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_bootmark.cpp — boot-time budget instrumentation (S-02-010).
//
// See ss_bootmark.h for the contract. Implementation notes: the milestone table
// is a fixed-size static array (no heap), timestamps come straight from
// esp_timer_get_time() (µs since boot), and all output goes through ss_log.

#include "ss_bootmark.h"

#include "esp_timer.h"
#include "ss_log.h"

static const char* TAG = "ss.boot";

typedef struct {
    const char* name;
    int64_t t_us;
} ss_bootmark_entry_t;

static ss_bootmark_entry_t s_marks[SS_BOOTMARK_CAP];
static size_t s_count = 0;
static bool s_overflow_warned = false;

void ss_bootmark(const char* name)
{
    if (name == nullptr) { return; }

    const int64_t now = esp_timer_get_time();

    if (s_count >= SS_BOOTMARK_CAP) {
        if (!s_overflow_warned) {
            SS_LOGW(TAG, "bootmark table full (cap=%d) — dropping further marks", SS_BOOTMARK_CAP);
            s_overflow_warned = true;
        }
        return;
    }

    const int64_t prev = (s_count > 0) ? s_marks[s_count - 1].t_us : 0;
    s_marks[s_count].name = name;
    s_marks[s_count].t_us = now;
    s_count++;

    SS_LOGI(TAG, "bootmark %s t=%lldus (+%lldus)", name, (long long)now, (long long)(now - prev));
}

void ss_bootmark_report(void)
{
    const int64_t total = (s_count > 0) ? s_marks[s_count - 1].t_us : 0;

    // Build the line incrementally so the whole report is a single log record.
    char line[256];
    int pos = 0;
    int n = snprintf(line, sizeof(line), "boot-report:");
    if (n > 0 && (size_t)n < sizeof(line)) { pos = n; }

    for (size_t i = 0; i < s_count && (size_t)pos < sizeof(line); i++) {
        n = snprintf(line + pos, sizeof(line) - (size_t)pos, " %s=%lld", s_marks[i].name,
                     (long long)s_marks[i].t_us);
        if (n < 0 || (size_t)(pos + n) >= sizeof(line)) { break; }
        pos += n;
    }

    if ((size_t)pos < sizeof(line)) {
        snprintf(line + pos, sizeof(line) - (size_t)pos, " total=%lld", (long long)total);
    }

    SS_LOGI(TAG, "%s", line);
}
