// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2026 SS-SP Project Contributors
//
// ss_hal_conformance.h — HAL conformance test-vector core (S-03-022).
//
// CONTRACT
// ========
// Purpose: a data-driven conformance suite for five HAL domain contracts —
// power, audio, LoRa, Wi-Fi, BLE. Each vector is a scripted sequence of HAL
// calls plus the observable outcome each step must produce (return code,
// domain state word, domain aux word). One pure-C runner walks the vectors,
// compares expected vs actual, and emits one frozen-format diff line per
// mismatched field, so a failing run names exactly what diverged.
//
// House pattern (ss_panic_guard / ss_log_format / ss_memwatch): this header
// and both of its translation units (ss_hal_conformance_core.c, the runner +
// diff formatter; ss_hal_conformance_vectors.c, the builtin static-const
// tables) are PURE C — they may include ONLY <stdint.h>, <stdbool.h>,
// <stddef.h>. No esp_err.h, no HAL headers, no <stdio.h> (the diff formatter
// renders hex/decimal itself). This is what lets the identical objects link
// under the host googletest harness (S-02-014) today and the on-target Unity
// runner (S-02-015, deferred — no board yet) later, unchanged.
//
// The core never calls a HAL function. All execution goes through an
// environment vtable (ss_conf_env_t): the host binds a mock backend that
// models the documented HAL semantics; the future target binds a thin adapter
// that calls the real ss_* driver API and reads back observable state. The
// vector tables are therefore normative for BOTH: a mock that drifts and a
// driver that drifts fail the same vectors with the same diff lines.
//
// Normativity: where a HAL domain header already documents pre/post-conditions
// (ss_hal_power.h wake-timer semantics), vectors MUST encode exactly those.
// Where a domain header is silent on error behavior (audio/Wi-Fi/BLE), the
// builtin vector table is the tie-breaker of record, following house policy
// (refuse-with-reason, never force): NULL required pointer -> INVALID_ARG
// (0x102), use-before-open/init -> INVALID_STATE (0x103). Changing a frozen
// vector expectation later is a contract change to the domain header first,
// vector second — never a quiet table edit.
//
// Security constraints (05_SECURITY_MODEL.md): the conformance surface is
// deliberately radio-lifecycle-and-state only. It never touches key material,
// pairing, wire bytes, or persistence — BLE pairing/LTK conformance is T1
// (S-03-016/S-03-017 per doc 05 §4/§6), secure-element and RNG conformance is
// T1, and both are OUT OF SCOPE here. Vectors carry no secrets; payload
// buffers are synthesized by the environment (arg gives length only), so no
// captured traffic or key-shaped constants can end up in a table.
//
// Out of scope (and owners):
//   - On-target Unity wiring and target env adapter    -> S-02-015
//   - Mock backend + builtin vector tables + host test -> t2-builder (this story)
//   - Display/touch/input conformance                  -> S-03-004/S-03-006 cores
//   - BLE pairing, crypto, wire formats                -> T1 stories (doc 10 §8)
//
// Testability: host-side, gtest calls ss_conf_run() over
// ss_conf_builtin_vectors() with the mock env and asserts a zero return; the
// diff formatter is unit-tested standalone against the frozen grammar.
//
// Concurrency: single-threaded by contract, like the HAL it exercises.
// Memory: no heap anywhere in the core; tables are static const (flash/rodata
// resident on target).

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// Return-code mirrors (frozen ABI facts — numerically equal to ESP-IDF
// esp_err.h; the host mocks/esp_err.h must define the same values)
// ============================================================================

#define SS_CONF_RET_OK                 ((int32_t)0)
#define SS_CONF_RET_FAIL               ((int32_t)-1)
#define SS_CONF_RET_ERR_NO_MEM         ((int32_t)0x101)
#define SS_CONF_RET_ERR_INVALID_ARG    ((int32_t)0x102)
#define SS_CONF_RET_ERR_INVALID_STATE  ((int32_t)0x103)
#define SS_CONF_RET_ERR_TIMEOUT        ((int32_t)0x107)

// ============================================================================
// Domains and opcodes
// ============================================================================

typedef enum {
    SS_CONF_DOM_POWER = 0,
    SS_CONF_DOM_AUDIO = 1,
    SS_CONF_DOM_LORA  = 2,
    SS_CONF_DOM_WIFI  = 3,
    SS_CONF_DOM_BLE   = 4,
    SS_CONF_DOM_COUNT = 5,
} ss_conf_domain_t;

// Opcode values are FROZEN: bits 7..4 carry the domain, bits 3..0 the op
// index, so SS_CONF_OP_DOMAIN(op) is a pure shift. Never renumber.
#define SS_CONF_OP_DOMAIN(op) ((ss_conf_domain_t)(((uint32_t)(op)) >> 4))

typedef enum {
    // --- power (maps 1:1 onto ss_hal_power.h) ------------------------------
    SS_CONF_OP_PWR_INIT             = 0x00, // args: none
    SS_CONF_OP_PWR_STATUS           = 0x01, // args: none
    SS_CONF_OP_PWR_ENTER            = 0x02, // arg_a = SS_CONF_PWR_* state code
    SS_CONF_OP_PWR_WAKE_SOURCE_ADD  = 0x03, // arg_a = gpio, arg_b = level
    SS_CONF_OP_PWR_WAKE_TIMER_SET   = 0x04, // arg_u64 = microseconds
    SS_CONF_OP_PWR_WAKE_TIMER_CLEAR = 0x05, // args: none

    // --- audio (maps 1:1 onto ss_hal_audio.h) ------------------------------
    SS_CONF_OP_AUD_MIC_OPEN    = 0x10, // arg_ptr = const ss_conf_audio_fmt_t*
    SS_CONF_OP_AUD_MIC_READ    = 0x11, // arg_a = bytes, arg_b = timeout_ms
    SS_CONF_OP_AUD_MIC_CLOSE   = 0x12, // args: none
    SS_CONF_OP_AUD_SPK_OPEN    = 0x13, // arg_ptr = const ss_conf_audio_fmt_t*
    SS_CONF_OP_AUD_SPK_WRITE   = 0x14, // arg_a = bytes (env synthesizes buffer)
    SS_CONF_OP_AUD_SPK_CLOSE   = 0x15, // args: none
    SS_CONF_OP_AUD_SPK_MUTE    = 0x16, // arg_a = 0|1
    SS_CONF_OP_AUD_SPK_VOLUME  = 0x17, // arg_a = percent 0..100
    SS_CONF_OP_AUD_BUZZER_BEEP = 0x18, // arg_a = freq_hz, arg_b = duration_ms

    // --- LoRa (maps 1:1 onto ss_hal_radio_lora.h) --------------------------
    SS_CONF_OP_LORA_INIT     = 0x20, // args: none
    SS_CONF_OP_LORA_CONFIG   = 0x21, // arg_ptr = const ss_conf_lora_cfg_t*
    SS_CONF_OP_LORA_TX       = 0x22, // arg_a = bytes, arg_b = timeout_ms
    SS_CONF_OP_LORA_RX_START = 0x23, // args: none (env supplies cb/user)
    SS_CONF_OP_LORA_RX_STOP  = 0x24, // args: none
    SS_CONF_OP_LORA_SLEEP    = 0x25, // arg_a = 0|1
    SS_CONF_OP_LORA_STATS    = 0x26, // args: none

    // --- Wi-Fi (maps 1:1 onto ss_hal_radio_wifi.h) -------------------------
    SS_CONF_OP_WIFI_INIT   = 0x30, // args: none
    SS_CONF_OP_WIFI_CONFIG = 0x31, // arg_ptr = const ss_conf_wifi_cfg_t*
    SS_CONF_OP_WIFI_START  = 0x32, // args: none
    SS_CONF_OP_WIFI_STOP   = 0x33, // args: none
    SS_CONF_OP_WIFI_SLEEP  = 0x34, // arg_a = 0|1

    // --- BLE (maps 1:1 onto ss_hal_radio_ble.h; NO pairing/crypto — T1) ----
    SS_CONF_OP_BLE_INIT           = 0x40, // args: none
    SS_CONF_OP_BLE_ADVERTISE      = 0x41, // arg_ptr = const char* name (may be NULL)
    SS_CONF_OP_BLE_STOP_ADVERTISE = 0x42, // args: none
    SS_CONF_OP_BLE_SCAN_START     = 0x43, // args: none
    SS_CONF_OP_BLE_SCAN_STOP      = 0x44, // args: none
    SS_CONF_OP_BLE_SLEEP          = 0x45, // arg_a = 0|1
} ss_conf_op_t;

// ============================================================================
// Pure mirror payload structs (env adapters translate to the real HAL types;
// field meanings identical to ss_hal_audio.h / ss_hal_radio_*.h)
// ============================================================================

typedef struct {
    uint32_t sample_rate_hz;
    uint8_t  channels;        // 1|2
    uint8_t  bits_per_sample; // 16|32
} ss_conf_audio_fmt_t;

typedef struct {
    uint32_t freq_hz;
    int8_t   tx_power_dbm;
    uint8_t  spreading_factor; // 5..12
    uint8_t  bandwidth_khz;    // 125|250 (500 stored mod-256 is FORBIDDEN in
                               // vectors: use 125/250 only; ss_lora_cfg_t
                               // carries bandwidth as uint8_t)
    uint8_t  coding_rate;      // 5..8
    uint16_t preamble_len;
    bool     iq_inverted;
    uint32_t sync_word;
} ss_conf_lora_cfg_t;

typedef struct {
    const char* ssid; // NUL-terminated, <= 32 chars; NULL exercises arg checks
    const char* pass; // NUL-terminated, <= 64 chars; NULL = open network
    bool        sta_mode;
    bool        ap_mode;
} ss_conf_wifi_cfg_t;

// ============================================================================
// Observable state encodings (FROZEN — the env adapter, mock or target, MUST
// report exactly these words for its domain after each executed step)
// ============================================================================

// POWER: state = last power state entered (values numerically equal to
// ss_power_state_t): 0 ON, 1 LIGHT_SLEEP, 2 DEEP_SLEEP, 3 HIBERNATE,
// 4 SHUTDOWN. Light-sleep return resumes with state reported as the entered
// value (the vector's next step observes what WAS entered, not "ON").
// aux: bit0 = wake timer armed; bits 8..15 = count of GPIO wake sources added.
#define SS_CONF_PWR_ON          0u
#define SS_CONF_PWR_LIGHT_SLEEP 1u
#define SS_CONF_PWR_DEEP_SLEEP  2u
#define SS_CONF_PWR_HIBERNATE   3u
#define SS_CONF_PWR_SHUTDOWN    4u
#define SS_CONF_PWR_AUX_TIMER_ARMED (1u << 0)
#define SS_CONF_PWR_AUX_GPIO_SHIFT  8u

// AUDIO: state bitfield: bit0 mic open, bit1 spk open, bit2 muted.
// aux: bits 0..7 = current volume percent; bits 16..31 = last buzzer freq_hz.
#define SS_CONF_AUD_STATE_MIC_OPEN (1u << 0)
#define SS_CONF_AUD_STATE_SPK_OPEN (1u << 1)
#define SS_CONF_AUD_STATE_MUTED    (1u << 2)
#define SS_CONF_AUD_AUX_VOL_MASK   0xFFu
#define SS_CONF_AUD_AUX_BUZZ_SHIFT 16u

// LORA: state = 0 UNINIT, 1 IDLE (initialized, not receiving), 2 RX, 3 SLEEP.
// aux: bits 0..15 = tx call count (successful), bits 16..31 = config count.
#define SS_CONF_LORA_UNINIT 0u
#define SS_CONF_LORA_IDLE   1u
#define SS_CONF_LORA_RX     2u
#define SS_CONF_LORA_SLEEP  3u
#define SS_CONF_LORA_AUX_TX_MASK   0xFFFFu
#define SS_CONF_LORA_AUX_CFG_SHIFT 16u

// WIFI: state = 0 UNINIT, 1 READY (initialized or stopped), 2 STARTED,
// 3 SLEEPING. aux = config apply count (successful ss_wifi_config calls).
#define SS_CONF_WIFI_UNINIT   0u
#define SS_CONF_WIFI_READY    1u
#define SS_CONF_WIFI_STARTED  2u
#define SS_CONF_WIFI_SLEEPING 3u

// BLE: state bitfield: bit0 initialized, bit1 advertising, bit2 scanning,
// bit3 sleeping. aux: bits 0..15 = advertise call count (successful),
// bits 16..31 = scan-start call count (successful).
#define SS_CONF_BLE_STATE_INIT        (1u << 0)
#define SS_CONF_BLE_STATE_ADVERTISING (1u << 1)
#define SS_CONF_BLE_STATE_SCANNING    (1u << 2)
#define SS_CONF_BLE_STATE_SLEEPING    (1u << 3)
#define SS_CONF_BLE_AUX_ADV_MASK    0xFFFFu
#define SS_CONF_BLE_AUX_SCAN_SHIFT  16u

// ============================================================================
// Vector representation
// ============================================================================

// Which fields of the expected outcome a step actually checks. A field not
// selected is ignored (don't-care), so vectors only pin what they mean to pin.
#define SS_CONF_CHECK_RET   (1u << 0)
#define SS_CONF_CHECK_STATE (1u << 1)
#define SS_CONF_CHECK_AUX   (1u << 2)

typedef struct {
    int32_t  ret;   // expected return (SS_CONF_RET_* mirror values)
    uint32_t state; // expected domain state word (encodings above)
    uint32_t aux;   // expected domain aux word (encodings above)
    uint8_t  check; // OR of SS_CONF_CHECK_*; 0 = step executed, nothing pinned
} ss_conf_expect_t;

typedef struct {
    ss_conf_op_t op;      // domain must equal the owning vector's domain
    uint32_t     arg_a;   // per-op meaning — see opcode comments
    uint32_t     arg_b;   // per-op meaning
    uint64_t     arg_u64; // per-op meaning (PWR_WAKE_TIMER_SET microseconds)
    const void*  arg_ptr; // per-op typed payload (mirror structs above)
    ss_conf_expect_t expect;
} ss_conf_step_t;

// One conformance vector: a named, capability-gated call script.
// Invariants (enforced by ss_conf_run, violation = SS_CONF_E_VECTOR):
//   - name != NULL, steps != NULL, n_steps >= 1
//   - every step's SS_CONF_OP_DOMAIN(op) == domain
// Authoring rules are in CONFORMANCE_SPEC.md (frozen alongside this header).
typedef struct {
    ss_conf_domain_t domain;
    const char*      name;          // kebab-case, unique within its domain
    uint64_t         requires_caps; // SS_CAP_* mask (values per ss_hal_caps.h);
                                    // env->reset may skip the vector when the
                                    // running board lacks a bit (target); the
                                    // host mock claims all five domain caps
    const ss_conf_step_t* steps;
    uint16_t              n_steps;
} ss_conf_vector_t;

// ============================================================================
// Mock-observation interface (environment vtable)
// ============================================================================

// What the environment observed for one executed step.
typedef struct {
    int32_t  ret;   // value returned by the HAL call (esp_err_t as int32_t)
    uint32_t state; // domain state word AFTER the call (encodings above)
    uint32_t aux;   // domain aux word AFTER the call
} ss_conf_actual_t;

// Prepare a fresh environment for `vec` and report whether it can run.
// Pre:  vec is a validated vector.
// Post: returns true with ALL five domains reset to their pristine
//       (uninitialized) model state — vectors never depend on each other;
//       returns false to SKIP the vector (e.g. target board lacks
//       requires_caps). Must be idempotent.
typedef bool (*ss_conf_reset_fn)(void* ctx, const ss_conf_vector_t* vec);

// Execute one step and fill *out with what actually happened.
// Pre:  step belongs to the vector most recently passed to reset; out != NULL.
// Post: returns true with *out fully written (all three fields, even ones the
//       step does not check); returns false ONLY if the environment cannot
//       execute the opcode at all (adapter gap) — the runner then fails the
//       vector with a field=exec diff. Must not longjmp/throw.
typedef bool (*ss_conf_exec_fn)(void* ctx, const ss_conf_step_t* step,
                                ss_conf_actual_t* out);

// Sink for one complete, NUL-terminated diff line (no trailing newline).
// Host: prints/records for gtest; target: routes to the Unity/console log.
typedef void (*ss_conf_emit_fn)(void* ctx, const char* line);

typedef struct {
    ss_conf_reset_fn reset; // required
    ss_conf_exec_fn  exec;  // required
    ss_conf_emit_fn  emit;  // required
    void*            ctx;   // opaque, passed through verbatim
} ss_conf_env_t;

// ============================================================================
// Actionable-diff format (FROZEN)
// ============================================================================

typedef enum {
    SS_CONF_FIELD_RET   = 0,
    SS_CONF_FIELD_STATE = 1,
    SS_CONF_FIELD_AUX   = 2,
    SS_CONF_FIELD_EXEC  = 3, // env->exec returned false (expected=1 actual=0)
} ss_conf_field_t;

typedef struct {
    ss_conf_domain_t domain;
    const char*      vector_name;
    uint16_t         step_index; // 0-based position within the vector
    ss_conf_op_t     op;
    ss_conf_field_t  field;
    uint32_t         expected;   // ret values carried as (uint32_t)(int32_t)
    uint32_t         actual;
} ss_conf_diff_t;

// Render `d` as EXACTLY this single line (grammar frozen; builder must match
// byte-for-byte — the host test asserts against literal strings):
//
//   CONF-FAIL dom=<domain> vec=<name> step=<N> op=<OPNAME> field=<F> expected=0x<XXXXXXXX> actual=0x<XXXXXXXX>
//
// where <domain> is the lowercase name from ss_conf_domain_name(), <N> is the
// 0-based step index in decimal with no padding, <OPNAME> is
// ss_conf_op_name(), <F> is one of ret|state|aux|exec, and both hex words are
// exactly 8 lowercase hex digits, zero-padded. Single ASCII spaces between
// tokens; no trailing newline. Example:
//
//   CONF-FAIL dom=power vec=wake-timer-arg-validation step=1 op=PWR_WAKE_TIMER_SET field=ret expected=0x00000102 actual=0x00000000
//
// Pre:  none (all-NULL tolerated).
// Post: returns the number of characters written, excluding the NUL; writes
//       at most cap bytes including the NUL (truncates safely, still
//       NUL-terminated when cap >= 1); returns 0 when d == NULL, out == NULL,
//       or cap == 0. Uses no heap and no <stdio.h>.
#define SS_CONF_DIFF_LINE_MAX 160u // always sufficient for a full line
uint32_t ss_conf_diff_format(const ss_conf_diff_t* d, char* out, uint32_t cap);

// Frozen name tables. Unknown values return "?" (never NULL).
// Domain names: "power" "audio" "lora" "wifi" "ble".
// Op names: the enumerator name minus the SS_CONF_OP_ prefix, e.g.
// "PWR_WAKE_TIMER_SET", "AUD_MIC_OPEN", "LORA_RX_START", "WIFI_CONFIG",
// "BLE_ADVERTISE".
const char* ss_conf_domain_name(ss_conf_domain_t d);
const char* ss_conf_op_name(ss_conf_op_t op);

// ============================================================================
// Runner
// ============================================================================

typedef struct {
    uint16_t vectors_run;     // executed (not skipped)
    uint16_t vectors_passed;  // executed with zero diffs
    uint16_t vectors_skipped; // env->reset returned false
    uint16_t steps_run;       // total steps handed to env->exec
    uint16_t diffs_emitted;   // total diff lines sent to env->emit
} ss_conf_result_t;

// Runner usage errors (returned by ss_conf_run; never emitted as diffs).
#define SS_CONF_E_ARG    ((int32_t)-1) // vectors/env NULL or a vtable fn NULL
#define SS_CONF_E_VECTOR ((int32_t)-2) // malformed table (invariants above)

// Run `n_vectors` vectors against `env`.
// Semantics:
//   - For each vector: env->reset; on false, count skipped and continue.
//   - Steps run in order. For each executed step, compare each field selected
//     by expect.check; every mismatch emits one diff line via env->emit.
//     A step with any mismatch fails its vector but execution CONTINUES
//     through the remaining steps (maximal diagnostic yield), except after a
//     field=exec failure, which aborts the vector (the model is now
//     undefined).
//   - Table validation happens BEFORE any reset/exec: a malformed table
//     returns SS_CONF_E_VECTOR having executed nothing.
// Pre:  env != NULL with all three functions set; vectors != NULL when
//       n_vectors > 0.
// Post: returns the number of FAILED vectors (0 = full pass; n_vectors == 0 is
//       a vacuous pass), or SS_CONF_E_ARG / SS_CONF_E_VECTOR. *out_result is
//       fully written whenever the return is >= 0; out_result may be NULL.
int32_t ss_conf_run(const ss_conf_vector_t* vectors, uint32_t n_vectors,
                    const ss_conf_env_t* env, ss_conf_result_t* out_result);

// The builtin vector tables (implemented in ss_hal_conformance_vectors.c).
// Pre:  out_n != NULL.
// Post: returns a pointer to a static const array of *out_n vectors covering
//       all five domains per the minimum matrix in CONFORMANCE_SPEC.md §3;
//       never NULL, *out_n >= 1. Returns NULL only when out_n == NULL.
const ss_conf_vector_t* ss_conf_builtin_vectors(uint32_t* out_n);

#ifdef __cplusplus
} // extern "C"
#endif
