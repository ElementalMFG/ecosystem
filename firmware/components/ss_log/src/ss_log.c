// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_log.c — ESP-IDF sink layer for ss_log: level filter, colorized console
// prefix, and a truncation-safe stack buffer feeding the pure formatter core.

#include "ss_log.h"
#include "ss_log_format.h"

#include <stdio.h>

#include "esp_timer.h"

// ANSI color codes matching the ESP-IDF console convention (E=red, W=yellow,
// I=green, D=plain). Kept as explicit constants so the colorized-console
// requirement is verifiable by inspection / CI grep.
#define SS_LOG_COLOR_ERROR "\033[0;31m"
#define SS_LOG_COLOR_WARN "\033[0;33m"
#define SS_LOG_COLOR_INFO "\033[0;32m"
#define SS_LOG_COLOR_DEBUG ""
#define SS_LOG_COLOR_RESET "\033[0m"

static ss_log_level_t s_level = SS_LOG_INFO;

void ss_log_set_level(ss_log_level_t level)
{
    s_level = level;
}

void ss_log_write(ss_log_level_t level, const char* tag, const char* fmt, ...)
{
    if (level > s_level) { return; }

    const char* color;
    char letter;
    switch (level) {
    case SS_LOG_ERROR:
        color = SS_LOG_COLOR_ERROR;
        letter = 'E';
        break;
    case SS_LOG_WARN:
        color = SS_LOG_COLOR_WARN;
        letter = 'W';
        break;
    case SS_LOG_DEBUG:
        color = SS_LOG_COLOR_DEBUG;
        letter = 'D';
        break;
    case SS_LOG_INFO:
    default:
        color = SS_LOG_COLOR_INFO;
        letter = 'I';
        break;
    }

    char msg[256];
    va_list ap;
    va_start(ap, fmt);
    (void)ss_log_vformat(msg, sizeof msg, fmt, ap);
    va_end(ap);

    // Millisecond monotonic tick, matching the ESP-IDF "(timestamp)" style.
    uint32_t ts_ms = (uint32_t)(esp_timer_get_time() / 1000);

    char hdr[64];
    (void)snprintf(hdr, sizeof hdr, "%c (%lu) %s: ", letter, (unsigned long)ts_ms,
                   (tag != NULL) ? tag : "?");

    fputs(color, stdout);
    fputs(hdr, stdout);
    fputs(msg, stdout);
    fputs(SS_LOG_COLOR_RESET, stdout);
    putchar('\n');
}
