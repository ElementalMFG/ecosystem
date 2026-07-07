// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_recovery.h — safe-mode / recovery boot path (S-02-016, T2).
//
// CONTRACT
// --------
// Recovery is a minimal, radio-less console mode a device owner can always
// reach, offering exactly: status, OTA rollback, factory reset, panic-counter
// clear, core-dump export, reboot. It exists so a bad update or corrupt
// state never bricks the device (PRD NF-REL-03; 05_SECURITY_MODEL.md §6).
//
// ENTRY PATHS (three, all converging on ss_recovery_console_loop):
//
//   1. Button gesture — "press RESET, then hold BOOT for 3 s".
//      GPIO0 is the ESP32-S3 BOOT strapping pin: LOW at reset release puts
//      the ROM in download mode, so the hold must START AFTER reset. A
//      lightweight watcher task (ss_recovery_watch_start) samples the button
//      for the first CONFIG_SS_RECOVERY_ENTRY_WINDOW_MS of a normal boot; a
//      continuous CONFIG_SS_RECOVERY_HOLD_MS press sets the RTC request flag
//      and restarts. The next boot's ss_recovery_boot_gate consumes the flag
//      and enters the console. Normal boots pay zero added latency
//      (S-02-010 budget unaffected); the watcher self-terminates when the
//      window closes.
//
//      PIN-SHARE CONSTRAINT (Lite): GPIO0 is also SS_LORA_PIN_CS
//      (board_config.h). The watcher claims GPIO0 as input+pull-up only
//      inside the entry window (SX1262 CS floats deselected — harmless) and
//      resets the pin config when the window closes. Radio bring-up
//      (EPIC-05) MUST init LoRa SPI only after the window closes, or first
//      shrink/disable the window via Kconfig.
//
//   2. Crash-loop breaker — a safe-mode boot (S-02-008, panic count >= 3)
//      calls ss_recovery_console_loop directly from app_main instead of the
//      minimal ss_panic_guard_safe_mode_loop, giving the stranded user the
//      full recovery action set. Safe-mode entry NEVER clears the panic
//      counter (S-02-008 contract); only the explicit `clear-panic` /
//      `rollback` / `factory-reset` actions below do.
//
//   3. Programmatic — ss_recovery_request() (future UI "reboot to recovery",
//      EPIC-15) sets the same RTC flag and restarts.
//
// REQUEST FLAG: an ss_recovery_flag_t in RTC_NOINIT RAM, integrity-guarded
// by magic + inverted-shadow exactly like ss_panic_record_t; consumed
// (cleared) by the boot gate on every boot, so a request is one-shot and
// cold-boot garbage can never fake one.
//
// ACTIONS (console commands; USB-Serial-JTAG, plain line protocol):
//
//   * status        — running slot + project/version of both OTA slots,
//                     otadata state, panic count, core-dump presence/size.
//                     Must surface the single-slot NEWEST-WINS dump semantic
//                     inherited from S-02-008.
//   * rollback      — boot the other OTA slot. Performed EXCLUSIVELY via
//                     esp_ota_set_boot_partition(), which re-verifies the
//                     target image; once EPIC-08 enables Secure Boot v2 and
//                     CONFIG_BOOTLOADER_ANTI_ROLLBACK, that same call
//                     enforces signature + eFuse secure_version checks.
//                     Recovery NEVER writes otadata bytes or eFuses
//                     directly, so anti-rollback guarantees are preserved
//                     by construction, and doc 05 §6's "recovery always
//                     accepts the current version" holds (same-version
//                     images always pass the secure_version test). A slot
//                     that is absent, empty, or fails verification is
//                     REFUSED with a reason — never forced. Success clears
//                     the panic counter (the rolled-back-to image must not
//                     inherit the failed image's crash count) and reboots.
//   * factory-reset — erase exactly the data partitions listed in
//                     ss_recovery_factory_plan (nvs, storage, coredump —
//                     dump hygiene: stored stacks/registers are user data),
//                     clear the panic counter, reboot. NEVER erases
//                     otadata/ota_0/ota_1: factory reset resets data, not
//                     firmware; slot choice is `rollback`'s job.
//   * clear-panic   — explicit S-02-008 counter clear (the "recovery UX
//                     explicitly clears it" path of that contract).
//   * export-dump   — stream the stored core dump over the console,
//                     base64 in fixed-size framed chunks with a trailing
//                     CRC32, for capture on flash-encrypted release units
//                     where raw partition reads return ciphertext
//                     (S-02-008 dump-hygiene contract). No dump -> says so.
//   * reboot        — esp_restart() into a normal boot.
//
// The console runs before any app task, radio, or NVS handle exists;
// nothing in recovery mode touches the network. The decision logic below is
// IDF-free and host-tested (firmware/test/host/tests/test_ss_recovery.cpp).

#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// ---- Pure decision core (no IDF dependencies; host-testable) --------------

#define SS_RECOVERY_FLAG_MAGIC 0x53525143u // "SRQC"

// One-shot recovery request in RTC_NOINIT RAM. Valid iff
// magic == SS_RECOVERY_FLAG_MAGIC && inverse == ~magic; anything else
// (cold-boot garbage, torn write) reads as "no request".
typedef struct {
    uint32_t magic;
    uint32_t inverse;
} ss_recovery_flag_t;

// True iff *flag holds a valid pending request. Pre: flag non-NULL.
bool ss_recovery_flag_pending(const ss_recovery_flag_t* flag);

// Arm / clear the request flag. Pre: flag non-NULL. Never fail.
void ss_recovery_flag_arm(ss_recovery_flag_t* flag);
void ss_recovery_flag_clear(ss_recovery_flag_t* flag);

// Button-hold detector state machine. Feed one debounced sample per call
// with a monotonic millisecond timestamp; the detector requires an
// UNBROKEN press of hold_ms within the entry window. Any release resets
// the accumulated hold; window expiry retires the detector for this boot.
typedef enum {
    SS_RECOVERY_HOLD_IDLE = 0,      // in window, button not (yet) pressed
    SS_RECOVERY_HOLD_ARMED = 1,     // pressed, accumulating hold time
    SS_RECOVERY_HOLD_TRIGGERED = 2, // continuous hold reached -> request!
    SS_RECOVERY_HOLD_EXPIRED = 3,   // window closed without a trigger
} ss_recovery_hold_state_t;

typedef struct {
    ss_recovery_hold_state_t state;
    uint32_t window_deadline_ms; // absolute: start_ms + window_ms
    uint32_t press_start_ms;     // valid in ARMED
    uint32_t hold_ms;            // config: required continuous press
} ss_recovery_hold_t;

// Initialize a detector. window_ms/hold_ms come from Kconfig on target.
// Pre: h non-NULL, hold_ms > 0, window_ms >= hold_ms.
void ss_recovery_hold_init(ss_recovery_hold_t* h, uint32_t start_ms, uint32_t window_ms,
                           uint32_t hold_ms);

// Advance the detector with one sample. Returns the post-sample state.
// TRIGGERED and EXPIRED are terminal (further calls are no-ops returning
// the terminal state). now_ms may wrap; callers on target sample well
// inside a 32-bit ms epoch so wrap handling is not part of this contract.
ss_recovery_hold_state_t ss_recovery_hold_step(ss_recovery_hold_t* h, bool pressed,
                                               uint32_t now_ms);

// Rollback precondition verdict: pure mapping from what the target glue
// observed about the other OTA slot to a user-facing decision. The REAL
// authority is esp_ota_set_boot_partition()'s own image verification — this
// core only decides whether to attempt it and how to report a refusal;
// it can never override a verification failure.
typedef enum {
    SS_RECOVERY_RB_OK = 0,            // attempt esp_ota_set_boot_partition
    SS_RECOVERY_RB_NO_OTHER_SLOT = 1, // partition table has no second slot
    SS_RECOVERY_RB_EMPTY_SLOT = 2,    // no readable app image in the slot
    SS_RECOVERY_RB_VERIFY_FAILED = 3, // set_boot_partition refused the image
} ss_recovery_rb_verdict_t;

typedef struct {
    bool other_slot_exists;  // esp_ota_get_next_update_partition != NULL
    bool other_slot_has_app; // esp_ota_get_partition_description == ESP_OK
    bool set_boot_ok;        // esp_ota_set_boot_partition == ESP_OK
} ss_recovery_rb_inputs_t;

// Evaluate rollback preconditions/result. Pre: in non-NULL. The verdict is
// SS_RECOVERY_RB_OK only when every stage passed; every other verdict is a
// refusal that the console must report verbatim (never retried, never
// forced).
ss_recovery_rb_verdict_t ss_recovery_rollback_eval(const ss_recovery_rb_inputs_t* in);

// Factory-reset erase plan: NULL-terminated array of partition labels, in
// erase order. Data partitions only — MUST NOT name otadata/ota_0/ota_1
// (host-tested invariant).
extern const char* const ss_recovery_factory_plan[];

// ---- Target glue (implemented in ss_recovery.cpp; not host-buildable) -----

// Consumes the RTC request flag. If a request was pending, enters
// ss_recovery_console_loop (never returns in that case). MUST be called
// once per boot, immediately after ss_panic_guard_boot_gate() and before
// any app bring-up. Never fails.
void ss_recovery_boot_gate(void);

// Starts the BOOT-button watcher task for this boot's entry window (see
// ENTRY PATHS). Call once per NORMAL boot, right after the boot gates.
// No-op if CONFIG_SS_RECOVERY_ENTRY_WINDOW_MS is 0. Errors are logged, not
// propagated (worst case: gesture unavailable this boot; safe mode still
// reaches recovery).
void ss_recovery_watch_start(void);

// Programmatic entry: arms the RTC flag and restarts. Never returns.
void ss_recovery_request(void) __attribute__((noreturn));

// The recovery console (see ACTIONS). Never returns; every exit is via
// esp_restart(). Runs with no app tasks and no radio.
void ss_recovery_console_loop(void) __attribute__((noreturn));

#ifdef __cplusplus
}
#endif
