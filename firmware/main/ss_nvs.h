// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_nvs.h — versioned NVS namespace scheme + migrations (S-02-017, T2).
//
// CONTRACT
// --------
// Every SS-SP subsystem persists settings in its own NVS namespace, and the
// LAYOUT of that namespace (which keys exist, what they mean, how values are
// encoded) is described by a single uint32 SCHEMA VERSION. This wrapper makes
// version handling impossible to forget: every access carries the caller's
// schema version, and a mismatch is resolved (or refused) BEFORE any user
// key is read or written. Persisted settings thereby survive firmware
// upgrades by construction, never by luck.
//
// VERSIONED-NAMESPACE SCHEME:
//
//   * Each namespace stores its schema version as a uint32 under the
//     RESERVED key SS_NVS_VER_KEY ("__ver"). The "__" prefix is reserved
//     for this module in BOTH namespaces and keys; the validators below
//     reject user names carrying it, so collision is structurally
//     impossible rather than merely discouraged.
//   * The `ver` argument of ss_nvs_get/ss_nvs_set is the CALLER's current
//     (code) schema version for that namespace — the version its read/write
//     code is written against. Valid schema versions are >= 1; 0 is invalid
//     as a caller version (it can only appear as a corrupt/legacy STORED
//     value, which migrates like any other older version).
//   * On the first access to a namespace (per boot; the glue caches the
//     verdict), the stored "__ver" is compared with `ver` and the pure
//     planner below decides: NONE (equal — proceed), FRESH (no stored
//     version — namespace is new; stamp "__ver" = ver and proceed),
//     UPGRADE (stored < ver — run the registered migration hook, then
//     stamp), or DOWNGRADE_REFUSE (stored > ver — REFUSE the access).
//     A newer image wrote that data; touching it from older code risks
//     silent corruption, so like ss_recovery_rollback_eval the policy is
//     refuse-with-reason, never force. Recovery from a refused downgrade
//     is rolling forward again or an explicit factory reset (S-02-016).
//
// MIGRATION HOOK:
//
//   * Subsystems register at most one hook per namespace via
//     ss_nvs_register_migration (fixed static registry, no heap,
//     SS_NVS_MAX_MIGRATIONS slots — pool philosophy of S-02-021).
//   * On an UPGRADE verdict the glue invokes the hook ONCE with
//     (ns, from_ver = stored, to_ver = caller ver). The hook owns any
//     multi-version stepping internally (from may be several versions
//     behind after a big OTA jump). Return 0 = success: the glue stamps
//     "__ver" = to_ver and the triggering access proceeds. Non-zero =
//     failure: nothing is stamped and the access fails with
//     SS_NVS_ERR_MIGRATION; the next access retries (migration hooks must
//     therefore be idempotent/resumable).
//   * Inside a hook, ss_nvs_get/ss_nvs_set on the MIGRATING namespace are
//     legal and skip the version gate (the glue marks the namespace
//     in-migration); the hook passes ver = to_ver by convention.
//   * UPGRADE with NO registered hook is REFUSED (SS_NVS_ERR_MIGRATION):
//     the layout changed and nobody claimed responsibility for the old
//     data. A subsystem whose old layout needs no transformation registers
//     a trivial hook returning 0 — an explicit, greppable statement that
//     the compatibility was considered.
//
// SECURITY / SCOPE (05_SECURITY_MODEL.md):
//
//   * This API is for SETTINGS-CLASS data only. It MUST NOT hold keys,
//     ratchet state, or the OTA anti-rollback counter: key material lives
//     per doc 05 §3/§5, the tamper-evident rollback counter of §6.4 is
//     EPIC-08/09 T1 work, and encrypted user data (§11.1) lives on
//     `fs_user`, not NVS. NVS-at-rest confidentiality arrives with flash
//     encryption (EPIC-08) and is transparent to this contract.
//   * Deliberately out of scope here: NVS partition encryption keys
//     (EPIC-08), OTA rollback semantics (EPIC-09 / S-02-016), pool-backed
//     blob buffers (S-02-021).
//
// The decision logic below is IDF-free and host-tested
// (firmware/test/host/tests/test_ss_nvs.cpp); ss_nvs.cpp supplies the
// nvs_flash glue and serializes all public calls with one internal mutex
// (the pure core is reentrant by construction).

#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// ---- Pure decision core (no IDF dependencies; host-testable) --------------

// ESP-IDF NVS hard limit: namespace and key names are at most 15 bytes
// (excluding the NUL terminator).
#define SS_NVS_NAME_MAX 15

// Names starting with this prefix belong to ss_nvs; user namespaces and
// keys carrying it are rejected by the validators below.
#define SS_NVS_RESERVED_PREFIX "__"

// Reserved per-namespace key holding the uint32 schema version.
#define SS_NVS_VER_KEY "__ver"

// Fixed capacity of the migration-hook registry.
#define SS_NVS_MAX_MIGRATIONS 8

// Name validity (shared rule for namespaces and keys): non-NULL, length
// 1..SS_NVS_NAME_MAX bytes, every byte printable non-space ASCII
// (0x21..0x7E), and NOT starting with SS_NVS_RESERVED_PREFIX. Two symbols
// so call sites read unambiguously; the rule is identical.
bool ss_nvs_ns_valid(const char* ns);
bool ss_nvs_key_valid(const char* key);

// Migration planner verdict (see CONTRACT / VERSIONED-NAMESPACE SCHEME).
typedef enum {
    SS_NVS_MIG_NONE = 0,             // stored == code: proceed, no action
    SS_NVS_MIG_FRESH = 1,            // no stored version: stamp code_ver, proceed
    SS_NVS_MIG_UPGRADE = 2,          // stored < code: run hook, stamp on success
    SS_NVS_MIG_DOWNGRADE_REFUSE = 3, // stored > code: refuse access entirely
} ss_nvs_mig_verdict_t;

typedef struct {
    ss_nvs_mig_verdict_t verdict;
    uint32_t from_ver; // meaningful for UPGRADE (== stored version)
    uint32_t to_ver;   // version to stamp for FRESH/UPGRADE (== code_ver)
} ss_nvs_plan_t;

// Pure mapping from (stored, code) versions to a migration plan.
// has_stored is false when the namespace carries no "__ver" key (new
// namespace, or pre-versioning legacy — indistinguishable, both FRESH).
// Pre: code_ver >= 1 (callers enforce via SS_NVS_ERR_INVALID before
// planning; behavior for code_ver == 0 is undefined by this contract).
// Post: verdict follows the table in the CONTRACT block exactly;
// from_ver/to_ver are set as documented and zero when not meaningful.
ss_nvs_plan_t ss_nvs_plan_migration(bool has_stored, uint32_t stored_ver, uint32_t code_ver);

// Error/result convention for the public API. 0 success, negative failure
// (esp_err_t values never leak through; the glue maps them).
typedef enum {
    SS_NVS_OK = 0,
    SS_NVS_ERR_INVALID = -1,   // bad name, ver == 0, NULL arg, bad length
    SS_NVS_ERR_NOT_FOUND = -2, // key absent in an otherwise healthy namespace
    SS_NVS_ERR_NO_SPACE = -3,  // buffer too small (get) / partition full (set)
    SS_NVS_ERR_MIGRATION = -4, // hook failed or UPGRADE with no hook
    SS_NVS_ERR_DOWNGRADE = -5, // stored ver newer than code ver: refused
    SS_NVS_ERR_STORAGE = -6,   // underlying nvs_flash failure
} ss_nvs_err_t;

// App-supplied migration hook (see CONTRACT / MIGRATION HOOK).
// Pre: invoked only with from_ver < to_ver, on the caller's task, with the
// namespace in-migration (version gate suspended for ns). Must be
// idempotent. Post: 0 commits (glue stamps to_ver); non-zero aborts
// (nothing stamped, access fails SS_NVS_ERR_MIGRATION, retried next
// access).
typedef int (*ss_nvs_migrate_fn)(const char* ns, uint32_t from_ver, uint32_t to_ver, void* ctx);

// ---- Target glue (implemented in ss_nvs.cpp; not host-buildable) ----------

// One-time init: nvs_flash_init (with the standard erase-and-retry on
// NVS_NO_FREE_PAGES/NEW_VERSION_FOUND) + registry/mutex setup. Call once
// from app_main before any other ss_nvs call. Idempotent; returns
// SS_NVS_OK or SS_NVS_ERR_STORAGE.
ss_nvs_err_t ss_nvs_init(void);

// Register the migration hook for a namespace. Pre: ss_nvs_ns_valid(ns);
// fn non-NULL; ns not already registered; registry not full
// (SS_NVS_MAX_MIGRATIONS). Returns false (and logs) on any violation.
// Register during subsystem init, before the namespace's first access.
// ctx is retained and passed through verbatim (may be NULL).
bool ss_nvs_register_migration(const char* ns, ss_nvs_migrate_fn fn, void* ctx);

// Write `len` bytes under ns/key, declaring caller schema version `ver`.
// Pre: valid ns/key (validators above), ver >= 1, data non-NULL if
// len > 0. Runs the version gate first (FRESH stamp / UPGRADE hook /
// DOWNGRADE refusal, per CONTRACT). Post: on SS_NVS_OK the value is
// committed (nvs_commit) — no deferred-write window.
ss_nvs_err_t ss_nvs_set(const char* ns, const char* key, uint32_t ver, const void* data,
                        uint32_t len);

// Read ns/key into buf, declaring caller schema version `ver`.
// Pre: valid ns/key, ver >= 1, len non-NULL. In: *len = capacity of buf.
// Out on SS_NVS_OK: *len = bytes written. buf == NULL is a size query:
// *len receives the stored size, nothing is copied, returns SS_NVS_OK.
// SS_NVS_ERR_NO_SPACE when capacity < stored size (*len set to required
// size, buf untouched). Runs the same version gate as ss_nvs_set.
ss_nvs_err_t ss_nvs_get(const char* ns, const char* key, uint32_t ver, void* buf, uint32_t* len);

// Convenience wrappers for the dominant settings case (uint32 scalars).
// Same version gate, preconditions, and error mapping as get/set above.
ss_nvs_err_t ss_nvs_set_u32(const char* ns, const char* key, uint32_t ver, uint32_t value);
ss_nvs_err_t ss_nvs_get_u32(const char* ns, const char* key, uint32_t ver, uint32_t* value);

// Delete one user key (never SS_NVS_VER_KEY — key validator applies).
// Same version gate. SS_NVS_ERR_NOT_FOUND if absent; SS_NVS_OK commits.
ss_nvs_err_t ss_nvs_erase_key(const char* ns, const char* key, uint32_t ver);

#ifdef __cplusplus
}
#endif
