// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_nvs_core.c — pure decision logic for the versioned NVS namespace scheme
// (S-02-017). IDF-free on purpose: this file is the audited, host-tested core
// (firmware/test/host/tests/test_ss_nvs.cpp) of the name validators and the
// migration planner. Keep it free of ESP-IDF includes and side effects — the
// nvs_flash glue lives in ss_nvs.cpp. See ss_nvs.h for the full contract.

#include "ss_nvs.h"

#include <stddef.h>
#include <string.h>

// ---- Name validity (shared rule for namespaces and keys) -------------------

// Pre: name may be NULL. Post: true iff name is non-NULL, 1..SS_NVS_NAME_MAX
// bytes long, every byte is printable non-space ASCII (0x21..0x7E), and it does
// not start with SS_NVS_RESERVED_PREFIX. No side effects.
static bool name_valid(const char* name)
{
    if (name == NULL) { return false; }

    // Reserved-prefix reject first: the "__" namespace/key space belongs to
    // this module (SS_NVS_VER_KEY lives there), so user names carrying it are
    // structurally refused rather than merely discouraged.
    const size_t prefix_len = strlen(SS_NVS_RESERVED_PREFIX);
    if (strncmp(name, SS_NVS_RESERVED_PREFIX, prefix_len) == 0) { return false; }

    size_t len = 0;
    for (const char* p = name; *p != '\0'; p++) {
        const unsigned char c = (unsigned char)*p;
        if (c < 0x21u || c > 0x7Eu) { return false; } // printable non-space ASCII
        len++;
        if (len > (size_t)SS_NVS_NAME_MAX) { return false; } // early out past limit
    }
    return len >= 1u; // reject the empty string
}

bool ss_nvs_ns_valid(const char* ns)
{
    return name_valid(ns);
}

bool ss_nvs_key_valid(const char* key)
{
    return name_valid(key);
}

// ---- Migration planner -----------------------------------------------------

ss_nvs_plan_t ss_nvs_plan_migration(bool has_stored, uint32_t stored_ver, uint32_t code_ver)
{
    ss_nvs_plan_t plan;
    plan.verdict = SS_NVS_MIG_NONE;
    plan.from_ver = 0u;
    plan.to_ver = 0u;

    if (!has_stored) {
        // New namespace (or pre-versioning legacy — indistinguishable): stamp
        // the caller's code version and proceed.
        plan.verdict = SS_NVS_MIG_FRESH;
        plan.to_ver = code_ver;
    } else if (stored_ver == code_ver) {
        // Layout matches: nothing to do.
        plan.verdict = SS_NVS_MIG_NONE;
    } else if (stored_ver < code_ver) {
        // Older data under newer code: run the migration hook, then stamp.
        plan.verdict = SS_NVS_MIG_UPGRADE;
        plan.from_ver = stored_ver;
        plan.to_ver = code_ver;
    } else {
        // Newer data under older code: refuse (a newer image wrote it).
        plan.verdict = SS_NVS_MIG_DOWNGRADE_REFUSE;
    }
    return plan;
}
