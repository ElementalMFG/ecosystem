// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_storage.c — ESP-IDF glue implementing ss_hal_storage.h for the LittleFS
// user filesystem (S-03-019). The mount/format/self-heal policy is decided by
// the pure, host-tested core in ss_storage_fs_core.c; this file only performs
// the real esp_littlefs operations the core asks for and feeds the results
// back. Peripheral availability is gated on ss_hal_has_cap — never CONFIG_* or
// board macros.
//
// The FS is mounted on base path "/user" over the frozen "storage" partition
// (partitions.csv is FROZEN — RFC-0003). joltwallet/esp_littlefs registers by
// partition_label, so the existing data/spiffs-subtype "storage" entry is used
// as-is without touching the partition table.
//
// TODO(EPIC-08): the user FS is PLAINTEXT. At-rest encryption lands with the
// EPIC-08 flash-encryption work; flash encryption is intentionally NOT enabled
// here.

#include "ss_hal.h"
#include "ss_hal_storage.h"
#include "ss_storage.h"
#include "ss_storage_fs_core.h"

#include "esp_err.h"
#include "esp_littlefs.h"
#include "esp_log.h"
#include "esp_partition.h"

static const char* TAG = "ss.storage";

#define SS_STORAGE_PARTITION_LABEL "storage"
#define SS_STORAGE_BASE_PATH "/user"
// The self-heal FSM is bounded (mount ≤2, format ≤1); this cap is a belt-and-
// braces guard so the drive loop can never spin even if the core regressed.
#define SS_STORAGE_MAX_STEPS 8u

static bool s_initialised = false;

esp_err_t ss_storage_init(void)
{
    s_initialised = true;
    return ESP_OK;
}

// Perform one real mount attempt against the "storage" partition.
static ss_fs_result_t do_mount(void)
{
    const esp_vfs_littlefs_conf_t conf = {
        .base_path = SS_STORAGE_BASE_PATH,
        .partition_label = SS_STORAGE_PARTITION_LABEL,
        // Self-heal is driven by the core FSM, not by the driver, so the driver
        // must NOT auto-format on a failed mount.
        .format_if_mount_failed = false,
        .dont_mount = false,
        .grow_on_mount = true,
    };
    const esp_err_t err = esp_vfs_littlefs_register(&conf);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "littlefs mount failed: %s", esp_err_to_name(err));
        return SS_FS_R_ERR;
    }
    return SS_FS_R_OK;
}

// Format the "storage" partition.
static ss_fs_result_t do_format(void)
{
    ESP_LOGW(TAG, "formatting '%s' partition to recover the user FS", SS_STORAGE_PARTITION_LABEL);
    const esp_err_t err = esp_littlefs_format(SS_STORAGE_PARTITION_LABEL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "littlefs format failed: %s", esp_err_to_name(err));
        return SS_FS_R_ERR;
    }
    return SS_FS_R_OK;
}

// Run the pure self-heal FSM against the real filesystem. Returns ESP_OK on a
// successful (possibly self-healed) mount, ESP_FAIL otherwise.
static esp_err_t drive_self_heal_mount(void)
{
    ss_fs_heal_state_t state;
    ss_fs_heal_init(&state);

    // Perform-then-advance: the first action is always a MOUNT; after each
    // step we feed its result to the core to obtain the next step. Terminal
    // steps are never "performed" — they end the loop.
    ss_fs_step_t step = SS_FS_STEP_MOUNT;

    for (unsigned i = 0u; i < SS_STORAGE_MAX_STEPS; i++) {
        ss_fs_result_t res;
        switch (step) {
        case SS_FS_STEP_MOUNT:
            res = do_mount();
            break;
        case SS_FS_STEP_FORMAT:
            res = do_format();
            break;
        case SS_FS_STEP_DONE_OK:
            ESP_LOGI(TAG, "user FS mounted at %s (label '%s'%s)", SS_STORAGE_BASE_PATH,
                     SS_STORAGE_PARTITION_LABEL, state.formatted ? ", self-healed by format" : "");
            return ESP_OK;
        case SS_FS_STEP_FAILED:
        default:
            ESP_LOGE(TAG, "user FS unrecoverable (formatted=%d)", (int)state.formatted);
            return ESP_FAIL;
        }
        step = ss_fs_heal_next(&state, step, res);
    }
    // Unreachable given the bounded FSM; fail-safe if it ever is reached.
    ESP_LOGE(TAG, "self-heal exceeded step budget");
    return ESP_FAIL;
}

esp_err_t ss_storage_mount(ss_storage_kind_t kind)
{
    switch (kind) {
    case SS_STORAGE_INTERNAL_FS:
        if (!ss_hal_has_cap(SS_CAP_INTERNAL_FLASH_FS)) {
            ESP_LOGW(TAG, "internal flash FS not present on this board");
            return ESP_ERR_NOT_SUPPORTED;
        }
        return drive_self_heal_mount();

    // TODO(S-03-023): NVS/SD/MODELS kinds are out of scope for S-03-019 and are
    // wired by the ss_storage aggregation story.
    case SS_STORAGE_INTERNAL_NVS:
    case SS_STORAGE_SD:
    case SS_STORAGE_MODELS:
    default:
        return ESP_ERR_NOT_SUPPORTED;
    }
}

esp_err_t ss_storage_unmount(ss_storage_kind_t kind)
{
    if (kind != SS_STORAGE_INTERNAL_FS) { return ESP_ERR_NOT_SUPPORTED; }
    return esp_vfs_littlefs_unregister(SS_STORAGE_PARTITION_LABEL);
}

esp_err_t ss_storage_stat(ss_storage_kind_t kind, uint64_t* total_bytes, uint64_t* free_bytes)
{
    if (kind != SS_STORAGE_INTERNAL_FS) { return ESP_ERR_NOT_SUPPORTED; }

    size_t total = 0u;
    size_t used = 0u;
    const esp_err_t err = esp_littlefs_info(SS_STORAGE_PARTITION_LABEL, &total, &used);
    if (err != ESP_OK) { return err; }

    const ss_fs_usage_t usage = {.total_bytes = (uint64_t)total, .used_bytes = (uint64_t)used};
    if (total_bytes != NULL) { *total_bytes = usage.total_bytes; }
    if (free_bytes != NULL) { *free_bytes = ss_fs_free_bytes(usage); }
    return ESP_OK;
}

bool ss_storage_present(ss_storage_kind_t kind)
{
    if (kind != SS_STORAGE_INTERNAL_FS) { return false; }
    if (!ss_hal_has_cap(SS_CAP_INTERNAL_FLASH_FS)) { return false; }
    const esp_partition_t* part = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, SS_STORAGE_PARTITION_LABEL);
    return part != NULL;
}

void ss_storage_log_stats(void)
{
    if (!s_initialised) { ESP_LOGW(TAG, "wear stats requested before init"); }

    uint64_t total = 0u;
    uint64_t freeb = 0u;
    const esp_err_t err = ss_storage_stat(SS_STORAGE_INTERNAL_FS, &total, &freeb);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "wear stats unavailable: %s", esp_err_to_name(err));
        return;
    }
    const ss_fs_usage_t usage = {.total_bytes = total, .used_bytes = total - freeb};
    ESP_LOGI(TAG, "user FS wear: total=%llu used=%llu free=%llu fill=%u%%",
             (unsigned long long)usage.total_bytes, (unsigned long long)usage.used_bytes,
             (unsigned long long)freeb, (unsigned)ss_fs_fill_pct(usage));
}
