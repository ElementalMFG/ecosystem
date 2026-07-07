// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_recovery.cpp — target glue for the safe-mode / recovery boot path
// (S-02-016). The decision logic (request flag, hold FSM, rollback verdict,
// erase plan) is pure and host-tested in ss_recovery_core.c; this file owns the
// RTC-noinit request flag, the BOOT-button watcher task, and the radio-less
// recovery console. See ss_recovery.h for the full contract.

#include "ss_recovery.h"

#include <inttypes.h>
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "esp_app_desc.h"
#include "esp_attr.h"
#include "esp_core_dump.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "esp_rom_crc.h"
#include "esp_system.h"
#include "esp_timer.h"

#include "ss_log.h"
#include "ss_panic_guard.h"
#include "ss_tasks.h"

static const char* TAG = "ss.recovery";

// One-shot recovery request. RTC_NOINIT: survives every reset except power
// loss; cold-boot garbage is rejected by the magic + inverted-shadow test in
// ss_recovery_core.c, and the boot gate clears it on every boot.
static RTC_NOINIT_ATTR ss_recovery_flag_t s_recovery_flag;

// ---------------------------------------------------------------------------
// Programmatic entry / boot gate
// ---------------------------------------------------------------------------

void ss_recovery_request(void)
{
    ss_recovery_flag_arm(&s_recovery_flag);
    SS_LOGW(TAG, "recovery requested — restarting into recovery console");
    esp_restart(); // noreturn
}

void ss_recovery_boot_gate(void)
{
    if (ss_recovery_flag_pending(&s_recovery_flag)) {
        ss_recovery_flag_clear(&s_recovery_flag); // one-shot: consume before use
        SS_LOGW(TAG, "recovery request flag consumed — entering recovery console");
        ss_recovery_console_loop(); // never returns
    }
}

// ---------------------------------------------------------------------------
// BOOT-button watcher task
// ---------------------------------------------------------------------------

#if CONFIG_SS_RECOVERY_ENTRY_WINDOW_MS == 0

void ss_recovery_watch_start(void)
{
    // Entry window disabled in Kconfig: the button gesture is unavailable this
    // build (safe mode still reaches recovery). Compile-time no-op.
}

#else

// Static task footprint (no heap on the watcher path).
static constexpr uint32_t kWatchStackBytes = 3072;
static StackType_t s_watch_stack[kWatchStackBytes];
static StaticTask_t s_watch_tcb;
static TaskHandle_t s_watch_handle = nullptr;

static uint32_t now_ms(void)
{
    return (uint32_t)(esp_timer_get_time() / 1000);
}

static void watch_task(void* arg)
{
    (void)arg;

    const gpio_num_t btn = (gpio_num_t)CONFIG_SS_RECOVERY_BTN_GPIO;

    // Claim GPIO0 as input+pull-up ONLY for the entry window (contract: the
    // SX1262 CS floats deselected — harmless; radio bring-up must wait for the
    // window to close). Reset the pin when the window retires.
    const gpio_config_t io = {
        .pin_bit_mask = 1ULL << (unsigned)CONFIG_SS_RECOVERY_BTN_GPIO,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    esp_err_t err = gpio_config(&io);
    if (err != ESP_OK) {
        SS_LOGW(TAG, "recovery watcher: gpio_config failed: %d — gesture unavailable", (int)err);
        vTaskDelete(nullptr);
        return;
    }

    ss_recovery_hold_t h;
    ss_recovery_hold_init(&h, now_ms(), (uint32_t)CONFIG_SS_RECOVERY_ENTRY_WINDOW_MS,
                          (uint32_t)CONFIG_SS_RECOVERY_HOLD_MS);

    for (;;) {
        const bool pressed = (gpio_get_level(btn) == 0); // active-low BOOT button
        const ss_recovery_hold_state_t st = ss_recovery_hold_step(&h, pressed, now_ms());

        if (st == SS_RECOVERY_HOLD_TRIGGERED) {
            SS_LOGW(TAG, "BOOT-hold gesture detected — entering recovery");
            ss_recovery_request(); // noreturn: arms flag + restarts
        }
        if (st == SS_RECOVERY_HOLD_EXPIRED) {
            gpio_reset_pin(btn); // release GPIO0 for radio bring-up (EPIC-05)
            SS_LOGD(TAG, "recovery entry window closed — watcher retiring");
            s_watch_handle = nullptr;
            vTaskDelete(nullptr);
            return;
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void ss_recovery_watch_start(void)
{
    if (s_watch_handle != nullptr) {
        return; // idempotent
    }
    // Static allocation (no heap); low priority just above idle (ss_tasks.h).
    s_watch_handle = xTaskCreateStatic(watch_task, "ss_recovery", kWatchStackBytes, nullptr,
                                       (UBaseType_t)SS_PRIO_IDLE_MON, s_watch_stack, &s_watch_tcb);
    if (s_watch_handle == nullptr) {
        // Non-fatal by contract: worst case the gesture is unavailable this boot.
        SS_LOGW(TAG, "recovery watcher task create failed");
    }
}

#endif // CONFIG_SS_RECOVERY_ENTRY_WINDOW_MS

// ---------------------------------------------------------------------------
// Recovery console
// ---------------------------------------------------------------------------

// Blocking line reader over the console (USB-Serial-JTAG). Assembles a line
// from getchar(); EOF (no data available) yields to the scheduler so the idle
// tasks feed the TWDT. Returns a NUL-terminated line with the newline stripped.
static void read_line(char* buf, size_t cap)
{
    size_t i = 0;
    for (;;) {
        const int c = getchar();
        if (c == EOF) {
            vTaskDelay(pdMS_TO_TICKS(20));
            continue;
        }
        if (c == '\r' || c == '\n') {
            putchar('\n');
            fflush(stdout);
            break;
        }
        if (c == 0x08 || c == 0x7f) { // backspace / delete
            if (i > 0) {
                i--;
                fputs("\b \b", stdout);
                fflush(stdout);
            }
            continue;
        }
        if (i + 1 < cap && c >= 0x20 && c < 0x7f) {
            buf[i++] = (char)c;
            putchar(c);
            fflush(stdout);
        }
    }
    buf[i] = '\0';
}

static bool str_eq(const char* a, const char* b)
{
    size_t i = 0;
    for (; a[i] != '\0' && b[i] != '\0'; i++) {
        if (a[i] != b[i]) { return false; }
    }
    return a[i] == b[i];
}

static void print_banner(void)
{
    printf("\n");
    printf("=== SS-SP RECOVERY CONSOLE (radio off, no app tasks) ===\n");
    printf("commands:\n");
    printf("  status        — running slot, both OTA slots, dump presence\n");
    printf("  rollback      — boot the other OTA slot (verified; refused if bad)\n");
    printf("  factory-reset — erase data partitions (nvs/storage/coredump)\n");
    printf("  clear-panic   — clear the consecutive-panic counter\n");
    printf("  export-dump   — stream the stored core dump (base64 + crc32)\n");
    printf("  reboot        — restart into a normal boot\n");
    fflush(stdout);
}

static const char* ota_state_name(esp_ota_img_states_t st)
{
    switch (st) {
    case ESP_OTA_IMG_NEW:
        return "new";
    case ESP_OTA_IMG_PENDING_VERIFY:
        return "pending-verify";
    case ESP_OTA_IMG_VALID:
        return "valid";
    case ESP_OTA_IMG_INVALID:
        return "invalid";
    case ESP_OTA_IMG_ABORTED:
        return "aborted";
    case ESP_OTA_IMG_UNDEFINED:
    default:
        return "undefined";
    }
}

static void print_slot(const esp_partition_t* part)
{
    if (part == nullptr) { return; }
    esp_app_desc_t desc;
    if (esp_ota_get_partition_description(part, &desc) == ESP_OK) {
        esp_ota_img_states_t st = ESP_OTA_IMG_UNDEFINED;
        esp_ota_get_state_partition(part, &st);
        printf("  slot %-6s project=%s version=%s state=%s\n", part->label, desc.project_name,
               desc.version, ota_state_name(st));
    } else {
        printf("  slot %-6s empty\n", part->label);
    }
}

static const esp_partition_t* coredump_partition(void)
{
    return esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_COREDUMP,
                                    nullptr);
}

static void cmd_status(void)
{
    const esp_partition_t* running = esp_ota_get_running_partition();
    printf("running partition: %s\n", (running != nullptr) ? running->label : "?");

    print_slot(
        esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_0, nullptr));
    print_slot(
        esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_1, nullptr));

    // Core-dump presence/size stands in for the (non-exposed) panic count.
    if (esp_core_dump_image_check() == ESP_OK) {
        size_t addr = 0;
        size_t size = 0;
        if (esp_core_dump_image_get(&addr, &size) == ESP_OK) {
            printf("core dump: present, %u bytes @ 0x%08x\n", (unsigned)size, (unsigned)addr);
        } else {
            printf("core dump: present (size query failed)\n");
        }
    } else {
        printf("core dump: none\n");
    }
    // Inherited S-02-008 semantic: a single dump slot, NEWEST WINS.
    printf("note: single dump slot — NEWEST panic WINS (a fresh crash overwrites"
           " an older stored dump)\n");
    fflush(stdout);
}

static void cmd_rollback(void)
{
    ss_recovery_rb_inputs_t in = {};
    const esp_partition_t* other = esp_ota_get_next_update_partition(nullptr);
    in.other_slot_exists = (other != nullptr);

    esp_app_desc_t desc;
    if (in.other_slot_exists) {
        in.other_slot_has_app = (esp_ota_get_partition_description(other, &desc) == ESP_OK);
    }
    if (in.other_slot_exists && in.other_slot_has_app) {
        // The REAL authority: set_boot_partition re-verifies the image (and,
        // once EPIC-08 lands, signature + anti-rollback). We never force it.
        in.set_boot_ok = (esp_ota_set_boot_partition(other) == ESP_OK);
    }

    switch (ss_recovery_rollback_eval(&in)) {
    case SS_RECOVERY_RB_OK:
        ss_panic_guard_recovery_clear(); // rolled-back image must not inherit crash count
        SS_LOGW(TAG, "rollback to %s accepted — restarting", other->label);
        printf("rollback: booting %s — restarting\n", other->label);
        fflush(stdout);
        esp_restart();
        break; // unreachable
    case SS_RECOVERY_RB_NO_OTHER_SLOT:
        printf("rollback refused: no second OTA slot in the partition table\n");
        break;
    case SS_RECOVERY_RB_EMPTY_SLOT:
        printf("rollback refused: the other slot holds no readable app image\n");
        break;
    case SS_RECOVERY_RB_VERIFY_FAILED:
        printf("rollback refused: the other slot's image failed verification\n");
        break;
    }
    fflush(stdout);
}

static void cmd_factory_reset(void)
{
    for (const char* const* label = ss_recovery_factory_plan; *label != nullptr; label++) {
        const esp_partition_t* part =
            esp_partition_find_first(ESP_PARTITION_TYPE_ANY, ESP_PARTITION_SUBTYPE_ANY, *label);
        if (part == nullptr) {
            SS_LOGW(TAG, "factory-reset: partition '%s' not found — skipped", *label);
            printf("factory-reset: '%s' not found (skipped)\n", *label);
            continue;
        }
        const esp_err_t err = esp_partition_erase_range(part, 0, part->size);
        if (err == ESP_OK) {
            printf("factory-reset: erased '%s' (%u bytes)\n", *label, (unsigned)part->size);
        } else {
            SS_LOGW(TAG, "factory-reset: erase '%s' failed: %d", *label, (int)err);
            printf("factory-reset: erase '%s' FAILED: %d\n", *label, (int)err);
        }
        fflush(stdout);
    }
    ss_panic_guard_recovery_clear();
    printf("factory-reset: data cleared — restarting\n");
    fflush(stdout);
    esp_restart();
}

static void cmd_clear_panic(void)
{
    ss_panic_guard_recovery_clear();
    printf("clear-panic: consecutive-panic counter cleared\n");
    fflush(stdout);
}

static const char kB64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// Base64-encode n (<= 48) raw bytes into a NUL-terminated string in out.
static void base64_encode(const uint8_t* in, size_t n, char* out)
{
    size_t oi = 0;
    size_t i = 0;
    for (; i + 3 <= n; i += 3) {
        const uint32_t v =
            ((uint32_t)in[i] << 16) | ((uint32_t)in[i + 1] << 8) | (uint32_t)in[i + 2];
        out[oi++] = kB64[(v >> 18) & 0x3fu];
        out[oi++] = kB64[(v >> 12) & 0x3fu];
        out[oi++] = kB64[(v >> 6) & 0x3fu];
        out[oi++] = kB64[v & 0x3fu];
    }
    const size_t rem = n - i;
    if (rem == 1) {
        const uint32_t v = (uint32_t)in[i] << 16;
        out[oi++] = kB64[(v >> 18) & 0x3fu];
        out[oi++] = kB64[(v >> 12) & 0x3fu];
        out[oi++] = '=';
        out[oi++] = '=';
    } else if (rem == 2) {
        const uint32_t v = ((uint32_t)in[i] << 16) | ((uint32_t)in[i + 1] << 8);
        out[oi++] = kB64[(v >> 18) & 0x3fu];
        out[oi++] = kB64[(v >> 12) & 0x3fu];
        out[oi++] = kB64[(v >> 6) & 0x3fu];
        out[oi++] = '=';
    }
    out[oi] = '\0';
}

static void cmd_export_dump(void)
{
    if (esp_core_dump_image_check() != ESP_OK) {
        printf("export-dump: no valid core dump stored\n");
        fflush(stdout);
        return;
    }
    const esp_partition_t* part = coredump_partition();
    if (part == nullptr) {
        printf("export-dump: no coredump partition\n");
        fflush(stdout);
        return;
    }

    // Stream only the used length (from the coredump image header) when
    // available; fall back to the whole partition otherwise.
    size_t addr = 0;
    size_t used = 0;
    size_t len = part->size;
    if (esp_core_dump_image_get(&addr, &used) == ESP_OK && used > 0 && used <= part->size) {
        len = used;
    }

    printf("export-dump: %u bytes (NEWEST-WINS single slot)\n", (unsigned)len);
    uint8_t raw[48];
    char b64[65];
    uint32_t crc = 0;
    size_t off = 0;
    while (off < len) {
        size_t n = len - off;
        if (n > sizeof(raw)) { n = sizeof(raw); }
        if (esp_partition_read(part, off, raw, n) != ESP_OK) {
            printf("DUMP-ERR read failed at %u\n", (unsigned)off);
            fflush(stdout);
            return;
        }
        crc = esp_rom_crc32_le(crc, raw, (uint32_t)n);
        base64_encode(raw, n, b64);
        printf("DUMP %u %s\n", (unsigned)off, b64);
        off += n;
        fflush(stdout);
    }
    printf("DUMP-END %08" PRIx32 " %u\n", crc, (unsigned)len);
    fflush(stdout);
}

void ss_recovery_console_loop(void)
{
    print_banner();
    char line[64];
    for (;;) {
        printf("recovery> ");
        fflush(stdout);
        read_line(line, sizeof(line));

        if (str_eq(line, "status")) {
            cmd_status();
        } else if (str_eq(line, "rollback")) {
            cmd_rollback();
        } else if (str_eq(line, "factory-reset")) {
            cmd_factory_reset();
        } else if (str_eq(line, "clear-panic")) {
            cmd_clear_panic();
        } else if (str_eq(line, "export-dump")) {
            cmd_export_dump();
        } else if (str_eq(line, "reboot")) {
            printf("rebooting…\n");
            fflush(stdout);
            esp_restart();
        } else {
            // unknown / empty
            printf("commands: status rollback factory-reset clear-panic export-dump reboot\n");
            fflush(stdout);
        }
    }
}
