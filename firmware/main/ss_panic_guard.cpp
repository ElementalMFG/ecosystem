// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_panic_guard.cpp — target glue for the panic post-mortem + crash-loop
// breaker (S-02-008). The decision logic itself is pure and host-tested in
// ss_panic_guard_core.c; this file owns the RTC-noinit record, the reset
// reason classification, the stability timer, and the safe-mode service loop.

#include "ss_panic_guard.h"

#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"
#include "esp_core_dump.h"
#include "esp_partition.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "ss_log.h"

static const char* TAG = "ss.panic";

// Coredump partition size, looked up at runtime so a pre-freeze resize in
// partitions.csv can never desync the fill-% metric (review nit). The
// fallback matches today's layout and is used only if the lookup fails.
static size_t coredump_partition_size(void)
{
    const esp_partition_t* part = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_COREDUMP, nullptr);
    return (part != nullptr) ? part->size : 0x10000; // fallback: partitions.csv value
}

// Consecutive-panic record. RTC_NOINIT: survives every reset except power
// loss; cold-boot garbage is rejected by the magic + inverted-count shadow
// (ss_panic_guard_core.c).
static RTC_NOINIT_ATTR ss_panic_record_t s_panic_rec;

static bool s_safe_mode = false;

// Stability window: a NORMAL boot that stays up this long is considered
// recovered, and the consecutive-panic count is cleared.
static constexpr uint64_t kStabilityWindowUs = 60ull * 1000ull * 1000ull;

// Reset-reason classification (contract in ss_panic_guard.h):
//   CRASH         — unambiguous firmware faults, incl. CPU lockup (real
//                   distinct reset on RISC-V targets; harmless to include
//                   on S3 where double exceptions land in ESP_RST_PANIC).
//   BENIGN        — environmental or intentional resets; brown-out is
//                   deliberately NOT a crash (S-02-012 owns brown-out).
//   INDETERMINATE — anything unknown/future, plus eFuse-error resets (a
//                   hardware fault is neither environmental/intentional nor
//                   a firmware crash): preserve the count (fail-safe for a
//                   breaker; never fail-open by clearing evidence).
// TODO(S-02-015): host-testable mirror of this mapping (review nit — the
// enum->class table is the most defect-prone piece and is review-only today).
static ss_reset_class_t classify_reset(esp_reset_reason_t reason)
{
    switch (reason) {
    case ESP_RST_PANIC:
    case ESP_RST_TASK_WDT:
    case ESP_RST_INT_WDT:
    case ESP_RST_WDT:
    case ESP_RST_CPU_LOCKUP:
        return SS_RESET_CLASS_CRASH;
    case ESP_RST_POWERON:
    case ESP_RST_EXT:
    case ESP_RST_SW:
    case ESP_RST_DEEPSLEEP:
    case ESP_RST_BROWNOUT:
    case ESP_RST_SDIO:
    case ESP_RST_USB:
    case ESP_RST_JTAG:
    case ESP_RST_PWR_GLITCH:
        return SS_RESET_CLASS_BENIGN;
    case ESP_RST_EFUSE:
    case ESP_RST_UNKNOWN:
    default:
        return SS_RESET_CLASS_INDETERMINATE;
    }
}

static void log_dump_status(void)
{
    if (esp_core_dump_image_check() == ESP_OK) {
        size_t addr = 0;
        size_t size = 0;
        if (esp_core_dump_image_get(&addr, &size) == ESP_OK) {
            const unsigned pct = unsigned((size * 100u) / coredump_partition_size());
            SS_LOGW(TAG,
                    "core dump present: %u bytes @ flash 0x%08x (%u%% of partition; decode: "
                    "tools/xtensa-decode-crash.py)",
                    unsigned(size), unsigned(addr), pct);
        } else {
            SS_LOGW(TAG, "core dump present (size query failed)");
        }
    } else {
        SS_LOGI(TAG, "no core dump stored");
    }
}

static void stability_timer_cb(void* arg)
{
    (void)arg;
    ss_panic_guard_record_clear(&s_panic_rec);
    SS_LOGI(TAG, "stability window passed — panic count cleared");
}

void ss_panic_guard_boot_gate(void)
{
    const esp_reset_reason_t reason = esp_reset_reason();
    const ss_reset_class_t cls = classify_reset(reason);

    const ss_boot_decision_t decision = ss_panic_guard_decide(cls, &s_panic_rec);
    s_safe_mode = (decision == SS_BOOT_SAFE_MODE);

    static const char* kClassNames[] = {"benign", "crash", "indeterminate"};
    static_assert(sizeof(kClassNames) / sizeof(kClassNames[0]) == SS_RESET_CLASS_INDETERMINATE + 1,
                  "kClassNames must cover every ss_reset_class_t value");
    SS_LOGI(TAG, "reset reason %d (%s), consecutive panics: %" PRIu32, int(reason),
            kClassNames[int(cls)], s_panic_rec.count);
    log_dump_status();

    if (s_safe_mode) {
        SS_LOGE(TAG, "%" PRIu32 " consecutive panics >= %u — entering SAFE MODE", s_panic_rec.count,
                unsigned(SS_PANIC_SAFE_MODE_THRESHOLD));
    }
}

bool ss_panic_guard_in_safe_mode(void)
{
    return s_safe_mode;
}

void ss_panic_guard_arm_stability_timer(void)
{
    if (s_safe_mode) {
        return; // safe mode never clears the counter (contract)
    }

    static const esp_timer_create_args_t args = {
        .callback = &stability_timer_cb,
        .arg = nullptr,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "ss_panic_stab",
        .skip_unhandled_events = false,
    };
    // One-shot; the handle is intentionally retained for the boot's lifetime
    // (contract: call at most once per boot). Deleted only if start fails.
    esp_timer_handle_t timer = nullptr;
    esp_err_t err = esp_timer_create(&args, &timer);
    if (err == ESP_OK) {
        err = esp_timer_start_once(timer, kStabilityWindowUs);
        if (err != ESP_OK) { esp_timer_delete(timer); }
    }
    if (err != ESP_OK) {
        // Non-fatal by contract: worst case the count clears on the next
        // BENIGN reset instead of after the window.
        SS_LOGW(TAG, "stability timer unavailable: %d", int(err));
    }
}

void ss_panic_guard_safe_mode_loop(void)
{
    // Minimal service loop: console status only — no app tasks, no radio, no
    // display. Recovery UX (dump export, guided reflash, counter clear) is
    // S-02-016. The 5 s delay lets the idle tasks feed the TWDT.
    for (;;) {
        SS_LOGW(TAG,
                "SAFE MODE: %" PRIu32
                " consecutive panics; flash new firmware or run recovery (S-02-016)",
                s_panic_rec.count);
        log_dump_status();
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
