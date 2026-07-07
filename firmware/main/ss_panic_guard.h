// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_panic_guard.h — panic post-mortem + crash-loop breaker (S-02-008, T1).
//
// CONTRACT
// --------
// Every panic path (fault, failed assert, stack-canary trip, TWDT/IWDT
// expiry) is captured by the ESP-IDF core-dump writer configured in
// sdkconfig.defaults: an ELF core dump is written to the dedicated
// `coredump` partition (partitions.csv), then the SoC reboots. The record
// decodes offline with tools/xtensa-decode-crash.py (espcoredump under the
// hood). Single dump slot, NEWEST WINS — deliberate tradeoff: in a crash
// loop the surviving dump is from the latest panic, not the (often more
// causal) first one; chosen because a fresh field crash must never be
// masked by a stale stored dump. S-02-016's export UX inherits and must
// surface this semantic.
//
// CRASH-LOOP BREAKER
// ------------------
// A consecutive-panic counter lives in RTC_NOINIT RAM (survives every
// reset except power loss) as an ss_panic_record_t, integrity-guarded by
// a magic word plus an inverted-count shadow so cold-boot garbage can
// never masquerade as a valid count. Every reset reason is classified
// three ways (ss_reset_class_t; mapping in ss_panic_guard.cpp):
//
//   * CRASH (panic, task/int/other WDT, CPU lockup) -> count += 1 (sat.)
//   * BENIGN (power-on, external pin, sw reset, deep-sleep wake,
//     brown-out, glitch, USB/JTAG/SDIO)             -> count  = 0
//     (brown-out persistence is S-02-012's contract, not a crash)
//   * INDETERMINATE (unknown/future reasons, plus eFuse-error resets:
//     a hardware fault is neither environmental/intentional nor a
//     firmware crash)                               -> count preserved
//     — fail-safe for a breaker: an unclassifiable reset must neither
//     count as a crash nor erase accumulated evidence of one.
//
//   count >= 3 -> boot SAFE MODE instead of the application.
//
// So after the 3rd consecutive panic the device stops re-entering the
// crashing application. SCOPE: the gate anchors at the top of app_main,
// so it breaks loops caused by subsystems started at or after app_main.
// Crashes earlier in boot (2nd-stage bootloader, ESP_SYSTEM_INIT_FN
// hooks, C++ global constructors) reboot before the gate runs and are
// NOT covered — today firmware/main has no non-POD globals so the
// practical exposure is nil; the durable fix is a bootloader-stage
// counter, tracked as S-02-022 (DRAFT).
//
// Safe mode NEVER clears the counter (a safe-mode panic keeps the device
// in safe mode). The counter clears only when (a) a normal boot survives
// the 60 s stability window, (b) a BENIGN reset occurs, or (c) recovery
// UX explicitly clears it (S-02-016). A device that panics slower than
// the stability window reboot-cycles by design — that is degraded field
// operation, not a tight loop; safe-mode UX for that case is S-02-016.
//
// DUMP-CONTENT HYGIENE (security contract — binding on future components)
// -----------------------------------------------------------------------
// The ELF dump persists task stacks, TCBs, and the crashing task's saved
// REGISTER FILE to flash (CONFIG_ESP_COREDUMP_CAPTURE_DRAM is pinned OFF
// in sdkconfig.defaults so .data/.bss/heap are NOT captured — do not flip
// it without wg-security sign-off). Therefore:
//   * Components handling secret material (ss_crypto, provisioning,
//     ratchet state) MUST keep secrets in dedicated heap/static buffers
//     that are zeroized immediately after use (05_SECURITY_MODEL.md
//     §11.1) and MUST NOT place long-lived key material on task stacks.
//   * Residual risk (accepted): a secret held in CPU registers at fault
//     time lands in the dumped register file. Mitigated only by narrow
//     zeroize windows and flash encryption — cannot be eliminated.
//   * The coredump partition carries the `encrypted` flag: once flash
//     encryption lands (EPIC-08, S-08-004/005) dumps are ciphertext at
//     rest from the first release build. Consequence: on release units
//     raw serial partition reads return ciphertext — dumps must then be
//     exported via the device-side path (S-02-016), not esptool/--port.
//   * Crash-report UPLOAD is opt-in only (05_SECURITY_MODEL.md §12); no
//     transport of dumps exists in this story.
//   * The duress-wipe path MUST erase the coredump partition along with
//     the archive — now an explicit AC clause of S-07-021 (EPIC-07).
//
// The decision core below is IDF-free and host-tested
// (firmware/test/host/tests/test_ss_panic_guard.cpp).

#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// ---- Pure decision core (no IDF dependencies; host-testable) --------------

#define SS_PANIC_RECORD_MAGIC 0x53505043u // "SPPC"
#define SS_PANIC_SAFE_MODE_THRESHOLD 3u

typedef enum {
    SS_BOOT_NORMAL = 0,
    SS_BOOT_SAFE_MODE = 1,
} ss_boot_decision_t;

// Reset-reason classification (see CRASH-LOOP BREAKER above). The mapping
// from esp_reset_reason_t lives in the target glue; the core only consumes
// the class.
typedef enum {
    SS_RESET_CLASS_BENIGN = 0,
    SS_RESET_CLASS_CRASH = 1,
    SS_RESET_CLASS_INDETERMINATE = 2,
} ss_reset_class_t;

// Persistent consecutive-panic record (RTC_NOINIT RAM on target).
// Valid iff magic == SS_PANIC_RECORD_MAGIC && inverse == ~count; any other
// state (cold boot garbage, bit rot, torn write) is treated as a fresh
// record with count 0.
typedef struct {
    uint32_t magic;
    uint32_t count;
    uint32_t inverse;
} ss_panic_record_t;

// Boot-gate decision. Pre: rec non-NULL (any content). Post: *rec is a
// valid, normalized record reflecting this boot per the classification
// rules above. Returns SS_BOOT_SAFE_MODE iff the updated count has reached
// SS_PANIC_SAFE_MODE_THRESHOLD. Never fails.
ss_boot_decision_t ss_panic_guard_decide(ss_reset_class_t cls, ss_panic_record_t* rec);

// Clears the consecutive-panic count in *rec (normalizing it if invalid).
// Called on target when a normal boot survives the stability window, or by
// recovery UX (S-02-016). Pre: rec non-NULL. Never fails.
void ss_panic_guard_record_clear(ss_panic_record_t* rec);

// ---- Target glue (implemented in ss_panic_guard.cpp; not host-buildable) --

// Runs the boot gate: classifies the SoC reset reason, updates the
// RTC-noinit record, logs the verdict plus core-dump presence/size (with
// partition-fill percentage so margin erosion is visible). MUST be called
// exactly once, at the top of app_main, before any application task is
// started. Never fails.
void ss_panic_guard_boot_gate(void);

// True iff this boot was gated into safe mode. Valid after boot_gate.
bool ss_panic_guard_in_safe_mode(void);

// Arms the one-shot 60 s stability timer; on expiry the panic count is
// cleared. Call AT MOST ONCE per boot, after a NORMAL-mode boot completes
// app bring-up. No-op in safe mode. The timer handle is retained for the
// lifetime of the boot by design (one-shot; deleted only on start
// failure). Never fails fatally (errors are logged, not propagated: worst
// case the count clears on the next BENIGN reset instead).
void ss_panic_guard_arm_stability_timer(void);

// Minimal safe-mode service loop: periodic console status (reset reason,
// panic count, dump presence/size), no app tasks, no radio. Never returns.
// Full recovery UX (dump export, guided reflash, counter clear) is S-02-016.
void ss_panic_guard_safe_mode_loop(void) __attribute__((noreturn));

#ifdef __cplusplus
}
#endif
