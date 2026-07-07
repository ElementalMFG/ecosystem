// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_nvs.cpp — target glue for the versioned NVS namespace scheme (S-02-017).
// The name validators and migration planner are pure and host-tested in
// ss_nvs_core.c; this file owns nvs_flash bring-up, the static migration-hook
// registry, the per-boot version gate, and the typed accessors. Every public
// call is serialized on one internal recursive mutex (the pure core is
// reentrant by construction); the recursive mutex also lets a migration hook
// re-enter ss_nvs_get/ss_nvs_set on the namespace it is migrating. esp_err_t
// values never leak — they are mapped to ss_nvs_err_t. See ss_nvs.h for the
// full contract.

extern "C" {
#include "ss_nvs.h"
}

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "nvs.h"
#include "nvs_flash.h"

#include "ss_log.h"

static const char* TAG = "ss.nvs";

// Upper bound on distinct namespaces whose gate verdict we cache per boot. If
// exceeded, the extra namespaces simply re-run the (idempotent) gate on every
// access — correctness is preserved, only the read is repeated.
#define SS_NVS_GATE_CACHE_MAX 16

// ---- Module state (all guarded by s_mutex once init has run) ---------------

namespace
{

struct migration_entry_t {
    char ns[SS_NVS_NAME_MAX + 1];
    ss_nvs_migrate_fn fn;
    void* ctx;
};

// Per-boot resolution of a namespace's version gate.
enum gate_status_t {
    GATE_READY = 0,    // NONE / FRESH-stamped / UPGRADE-completed: proceed
    GATE_DOWNGRADE = 1 // stored newer than code: refuse every access this boot
};

struct gate_cache_entry_t {
    char ns[SS_NVS_NAME_MAX + 1];
    gate_status_t status;
};

bool s_init = false;
SemaphoreHandle_t s_mutex = nullptr;

migration_entry_t s_registry[SS_NVS_MAX_MIGRATIONS];
size_t s_registry_count = 0;

gate_cache_entry_t s_gate_cache[SS_NVS_GATE_CACHE_MAX];
size_t s_gate_cache_count = 0;

// In-migration carve-out: while a hook runs, the migrating namespace skips the
// gate for the task that is running the hook (recursive re-entry).
bool s_migrating = false;
char s_migrating_ns[SS_NVS_NAME_MAX + 1] = {0};
TaskHandle_t s_migrating_task = nullptr;

void copy_name(char* dst, const char* src)
{
    // src is a validated name (<= SS_NVS_NAME_MAX bytes); dst is sized
    // SS_NVS_NAME_MAX + 1. strncpy + explicit terminator (no truncation here by
    // construction, but bound the copy defensively).
    strncpy(dst, src, (size_t)SS_NVS_NAME_MAX);
    dst[SS_NVS_NAME_MAX] = '\0';
}

// Map an esp_err_t from the nvs layer onto the public error convention. Keeps
// esp_err_t from leaking through the contract boundary.
ss_nvs_err_t map_err(esp_err_t e)
{
    switch (e) {
    case ESP_OK:
        return SS_NVS_OK;
    case ESP_ERR_NVS_NOT_FOUND:
        return SS_NVS_ERR_NOT_FOUND;
    case ESP_ERR_NVS_INVALID_LENGTH:
    case ESP_ERR_NVS_NOT_ENOUGH_SPACE:
        return SS_NVS_ERR_NO_SPACE;
    case ESP_ERR_NVS_INVALID_NAME:
    case ESP_ERR_NVS_KEY_TOO_LONG:
    case ESP_ERR_NVS_INVALID_HANDLE:
        return SS_NVS_ERR_INVALID;
    default:
        return SS_NVS_ERR_STORAGE;
    }
}

// Cache helpers -------------------------------------------------------------

const gate_cache_entry_t* cache_lookup(const char* ns)
{
    for (size_t i = 0; i < s_gate_cache_count; i++) {
        if (strcmp(s_gate_cache[i].ns, ns) == 0) { return &s_gate_cache[i]; }
    }
    return nullptr;
}

void cache_store(const char* ns, gate_status_t status)
{
    for (size_t i = 0; i < s_gate_cache_count; i++) {
        if (strcmp(s_gate_cache[i].ns, ns) == 0) {
            s_gate_cache[i].status = status;
            return;
        }
    }
    if (s_gate_cache_count < SS_NVS_GATE_CACHE_MAX) {
        copy_name(s_gate_cache[s_gate_cache_count].ns, ns);
        s_gate_cache[s_gate_cache_count].status = status;
        s_gate_cache_count++;
    }
    // Full cache: silently skip — the gate re-runs (idempotent) next access.
}

const migration_entry_t* find_migration(const char* ns)
{
    for (size_t i = 0; i < s_registry_count; i++) {
        if (strcmp(s_registry[i].ns, ns) == 0) { return &s_registry[i]; }
    }
    return nullptr;
}

// Stamp the schema version and commit. Caller holds an open RW handle on ns.
ss_nvs_err_t stamp_version(nvs_handle_t h, uint32_t ver)
{
    esp_err_t e = nvs_set_u32(h, SS_NVS_VER_KEY, ver);
    if (e == ESP_OK) { e = nvs_commit(h); }
    return map_err(e);
}

// Run the per-boot version gate for (ns, ver). Returns SS_NVS_OK when the
// access may proceed; a negative error otherwise. Caller holds s_mutex.
ss_nvs_err_t run_gate(const char* ns, uint32_t ver)
{
    // Carve-out: the task running a migration hook may touch the migrating
    // namespace without re-triggering the gate (contract / MIGRATION HOOK).
    if (s_migrating && strcmp(ns, s_migrating_ns) == 0 &&
        xTaskGetCurrentTaskHandle() == s_migrating_task) {
        return SS_NVS_OK;
    }

    const gate_cache_entry_t* cached = cache_lookup(ns);
    if (cached != nullptr) {
        return (cached->status == GATE_READY) ? SS_NVS_OK : SS_NVS_ERR_DOWNGRADE;
    }

    nvs_handle_t h;
    esp_err_t e = nvs_open(ns, NVS_READWRITE, &h);
    if (e != ESP_OK) {
        SS_LOGE(TAG, "nvs_open('%s') failed: %d", ns, (int)e);
        return map_err(e);
    }

    uint32_t stored = 0;
    esp_err_t re = nvs_get_u32(h, SS_NVS_VER_KEY, &stored);
    bool has_stored;
    if (re == ESP_OK) {
        has_stored = true;
    } else if (re == ESP_ERR_NVS_NOT_FOUND) {
        has_stored = false;
    } else {
        SS_LOGE(TAG, "read __ver of '%s' failed: %d", ns, (int)re);
        nvs_close(h);
        return map_err(re);
    }

    const ss_nvs_plan_t plan = ss_nvs_plan_migration(has_stored, stored, ver);
    ss_nvs_err_t rc = SS_NVS_OK;

    switch (plan.verdict) {
    case SS_NVS_MIG_NONE:
        cache_store(ns, GATE_READY);
        break;

    case SS_NVS_MIG_FRESH:
        rc = stamp_version(h, plan.to_ver);
        if (rc == SS_NVS_OK) { cache_store(ns, GATE_READY); }
        break;

    case SS_NVS_MIG_UPGRADE: {
        const migration_entry_t* reg = find_migration(ns);
        if (reg == nullptr) {
            // Layout changed and nobody claimed the old data.
            SS_LOGE(TAG, "upgrade of '%s' (%u->%u) has no migration hook", ns, (unsigned)stored,
                    (unsigned)ver);
            rc = SS_NVS_ERR_MIGRATION;
            break;
        }
        // Enter the carve-out, saving any outer migration state so nested
        // cross-namespace migrations restore correctly.
        const bool prev_migrating = s_migrating;
        char prev_ns[SS_NVS_NAME_MAX + 1];
        copy_name(prev_ns, s_migrating_ns);
        TaskHandle_t prev_task = s_migrating_task;

        s_migrating = true;
        copy_name(s_migrating_ns, ns);
        s_migrating_task = xTaskGetCurrentTaskHandle();

        const int hr = reg->fn(ns, plan.from_ver, plan.to_ver, reg->ctx);

        s_migrating = prev_migrating;
        copy_name(s_migrating_ns, prev_ns);
        s_migrating_task = prev_task;

        if (hr != 0) {
            // Nothing stamped; the next access retries (hooks are idempotent).
            SS_LOGE(TAG, "migration hook for '%s' (%u->%u) failed: %d", ns, (unsigned)stored,
                    (unsigned)ver, hr);
            rc = SS_NVS_ERR_MIGRATION;
            break;
        }
        rc = stamp_version(h, plan.to_ver);
        if (rc == SS_NVS_OK) { cache_store(ns, GATE_READY); }
        break;
    }

    case SS_NVS_MIG_DOWNGRADE_REFUSE:
    default:
        SS_LOGE(TAG, "downgrade refused for '%s': stored %u newer than code %u", ns,
                (unsigned)stored, (unsigned)ver);
        cache_store(ns, GATE_DOWNGRADE);
        rc = SS_NVS_ERR_DOWNGRADE;
        break;
    }

    nvs_close(h);
    return rc;
}

// Validate the common (ns, key, ver) preconditions shared by the accessors.
ss_nvs_err_t validate_access(const char* ns, const char* key, uint32_t ver)
{
    if (!ss_nvs_ns_valid(ns) || !ss_nvs_key_valid(key) || ver < 1u) { return SS_NVS_ERR_INVALID; }
    return SS_NVS_OK;
}

} // namespace

// ---- Locking helpers -------------------------------------------------------

static inline bool lock(void)
{
    if (!s_init || s_mutex == nullptr) { return false; }
    return xSemaphoreTakeRecursive(s_mutex, portMAX_DELAY) == pdTRUE;
}

static inline void unlock(void)
{
    xSemaphoreGiveRecursive(s_mutex);
}

// ---- Public API ------------------------------------------------------------

ss_nvs_err_t ss_nvs_init(void)
{
    if (s_init) { return SS_NVS_OK; } // idempotent

    esp_err_t e = nvs_flash_init();
    if (e == ESP_ERR_NVS_NO_FREE_PAGES || e == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // Truncated/upgraded partition: erase and retry once (standard idiom).
        SS_LOGW(TAG, "nvs_flash_init returned %d — erasing and retrying", (int)e);
        if (nvs_flash_erase() == ESP_OK) { e = nvs_flash_init(); }
    }
    if (e != ESP_OK) {
        SS_LOGE(TAG, "nvs_flash_init failed: %d", (int)e);
        return SS_NVS_ERR_STORAGE;
    }

    s_mutex = xSemaphoreCreateRecursiveMutex();
    if (s_mutex == nullptr) {
        SS_LOGE(TAG, "mutex create failed");
        return SS_NVS_ERR_STORAGE;
    }
    s_init = true;
    return SS_NVS_OK;
}

bool ss_nvs_register_migration(const char* ns, ss_nvs_migrate_fn fn, void* ctx)
{
    if (!ss_nvs_ns_valid(ns) || fn == nullptr) {
        SS_LOGE(TAG, "register_migration: invalid ns or NULL fn");
        return false;
    }
    if (!lock()) {
        SS_LOGE(TAG, "register_migration: ss_nvs_init not called");
        return false;
    }

    bool ok = false;
    if (find_migration(ns) != nullptr) {
        SS_LOGE(TAG, "register_migration: '%s' already registered", ns);
    } else if (s_registry_count >= SS_NVS_MAX_MIGRATIONS) {
        SS_LOGE(TAG, "register_migration: registry full (%d slots)", SS_NVS_MAX_MIGRATIONS);
    } else {
        copy_name(s_registry[s_registry_count].ns, ns);
        s_registry[s_registry_count].fn = fn;
        s_registry[s_registry_count].ctx = ctx;
        s_registry_count++;
        ok = true;
    }

    unlock();
    return ok;
}

ss_nvs_err_t ss_nvs_set(const char* ns, const char* key, uint32_t ver, const void* data,
                        uint32_t len)
{
    ss_nvs_err_t v = validate_access(ns, key, ver);
    if (v != SS_NVS_OK) { return v; }
    if (len > 0u && data == nullptr) { return SS_NVS_ERR_INVALID; }
    if (!lock()) { return SS_NVS_ERR_STORAGE; }

    ss_nvs_err_t rc = run_gate(ns, ver);
    if (rc == SS_NVS_OK) {
        nvs_handle_t h;
        esp_err_t e = nvs_open(ns, NVS_READWRITE, &h);
        if (e != ESP_OK) {
            rc = map_err(e);
        } else {
            e = nvs_set_blob(h, key, data, (size_t)len);
            if (e == ESP_OK) { e = nvs_commit(h); }
            rc = map_err(e);
            nvs_close(h);
        }
    }

    unlock();
    return rc;
}

ss_nvs_err_t ss_nvs_get(const char* ns, const char* key, uint32_t ver, void* buf, uint32_t* len)
{
    ss_nvs_err_t v = validate_access(ns, key, ver);
    if (v != SS_NVS_OK) { return v; }
    if (len == nullptr) { return SS_NVS_ERR_INVALID; }
    if (!lock()) { return SS_NVS_ERR_STORAGE; }

    ss_nvs_err_t rc = run_gate(ns, ver);
    if (rc == SS_NVS_OK) {
        nvs_handle_t h;
        esp_err_t e = nvs_open(ns, NVS_READONLY, &h);
        if (e != ESP_OK) {
            rc = map_err(e);
        } else {
            size_t required = 0;
            e = nvs_get_blob(h, key, nullptr, &required);
            if (e != ESP_OK) {
                rc = map_err(e);
            } else if (buf == nullptr) {
                // Size query: report the stored size, copy nothing.
                *len = (uint32_t)required;
                rc = SS_NVS_OK;
            } else if ((size_t)*len < required) {
                // Insufficient capacity: report required size, leave buf.
                *len = (uint32_t)required;
                rc = SS_NVS_ERR_NO_SPACE;
            } else {
                size_t cap = (size_t)*len;
                e = nvs_get_blob(h, key, buf, &cap);
                *len = (uint32_t)cap;
                rc = map_err(e);
            }
            nvs_close(h);
        }
    }

    unlock();
    return rc;
}

ss_nvs_err_t ss_nvs_set_u32(const char* ns, const char* key, uint32_t ver, uint32_t value)
{
    ss_nvs_err_t v = validate_access(ns, key, ver);
    if (v != SS_NVS_OK) { return v; }
    if (!lock()) { return SS_NVS_ERR_STORAGE; }

    ss_nvs_err_t rc = run_gate(ns, ver);
    if (rc == SS_NVS_OK) {
        nvs_handle_t h;
        esp_err_t e = nvs_open(ns, NVS_READWRITE, &h);
        if (e != ESP_OK) {
            rc = map_err(e);
        } else {
            e = nvs_set_u32(h, key, value);
            if (e == ESP_OK) { e = nvs_commit(h); }
            rc = map_err(e);
            nvs_close(h);
        }
    }

    unlock();
    return rc;
}

ss_nvs_err_t ss_nvs_get_u32(const char* ns, const char* key, uint32_t ver, uint32_t* value)
{
    ss_nvs_err_t v = validate_access(ns, key, ver);
    if (v != SS_NVS_OK) { return v; }
    if (value == nullptr) { return SS_NVS_ERR_INVALID; }
    if (!lock()) { return SS_NVS_ERR_STORAGE; }

    ss_nvs_err_t rc = run_gate(ns, ver);
    if (rc == SS_NVS_OK) {
        nvs_handle_t h;
        esp_err_t e = nvs_open(ns, NVS_READONLY, &h);
        if (e != ESP_OK) {
            rc = map_err(e);
        } else {
            e = nvs_get_u32(h, key, value);
            rc = map_err(e);
            nvs_close(h);
        }
    }

    unlock();
    return rc;
}

ss_nvs_err_t ss_nvs_erase_key(const char* ns, const char* key, uint32_t ver)
{
    ss_nvs_err_t v = validate_access(ns, key, ver);
    if (v != SS_NVS_OK) { return v; }
    if (!lock()) { return SS_NVS_ERR_STORAGE; }

    ss_nvs_err_t rc = run_gate(ns, ver);
    if (rc == SS_NVS_OK) {
        nvs_handle_t h;
        esp_err_t e = nvs_open(ns, NVS_READWRITE, &h);
        if (e != ESP_OK) {
            rc = map_err(e);
        } else {
            e = nvs_erase_key(h, key);
            if (e == ESP_OK) { e = nvs_commit(h); }
            rc = map_err(e);
            nvs_close(h);
        }
    }

    unlock();
    return rc;
}
