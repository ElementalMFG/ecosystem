// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_bootmark.h — boot-time budget instrumentation (S-02-010).
//
// A dependency-light stopwatch for the boot path: each ss_bootmark() call
// records a named milestone with esp_timer_get_time() (µs since boot) and logs
// the absolute timestamp plus the delta since the previous mark. After the boot
// sequence completes, ss_bootmark_report() emits ONE machine-parseable line
// that a host-side checker (tools/boot-budget-check.py) scrapes to assert the
// boot budget.
//
// The table is static (capacity SS_BOOTMARK_CAP), so instrumentation costs no
// heap and cannot fail. Marks beyond capacity are dropped silently, with a
// single warning logged the first time it happens.

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Maximum number of milestones retained. Marks past this are dropped (see note).
#define SS_BOOTMARK_CAP 16

// Record a boot milestone `name` at the current esp_timer time.
//
// pre:  `name` is a non-null, static/long-lived string (stored by pointer, not
//       copied); called from a normal task context (esp_timer running).
// post: on success the (name, timestamp) pair is appended to the static table
//       and an INFO line is logged:
//         bootmark <name> t=<us>us (+<delta_since_prev>us)
//       When the table is already full the mark is dropped and, the first time
//       this occurs, a single WARN line is logged.
// error: none — the call cannot fail and never allocates.
void ss_bootmark(const char* name);

// Emit the machine-parseable boot report line for all recorded milestones.
//
// pre:  called once, after the boot sequence has placed its marks.
// post: logs exactly one INFO line of space-separated name=value pairs in the
//       order the marks were recorded, followed by the total:
//         boot-report: <name1>=<us1> <name2>=<us2> ... total=<last_mark_us>
//       `total` is the timestamp of the last recorded mark (0 if none).
// error: none.
void ss_bootmark_report(void);

#ifdef __cplusplus
}
#endif
