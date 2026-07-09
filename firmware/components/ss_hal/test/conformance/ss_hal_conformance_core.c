// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2026 SS-SP Project Contributors
//
// ss_hal_conformance_core.c — the pure-C conformance runner + diff formatter
// (S-03-022). Implements ss_conf_run, ss_conf_diff_format, ss_conf_domain_name
// and ss_conf_op_name per the frozen semantics in ss_hal_conformance.h.
//
// Purity (see header + CONFORMANCE_SPEC.md §1): this TU includes ONLY the
// conformance header (which itself pulls in <stdint.h>/<stdbool.h>/<stddef.h>).
// No <stdio.h>, no heap — hex/decimal are rendered by hand so the identical
// object links under the host googletest harness today and the on-target Unity
// runner (S-02-015) later, unchanged.

#include "ss_hal_conformance.h"

// ============================================================================
// Name tables (frozen — see header). Unknown values return "?", never NULL.
// ============================================================================

const char* ss_conf_domain_name(ss_conf_domain_t d)
{
    switch (d) {
        case SS_CONF_DOM_POWER: return "power";
        case SS_CONF_DOM_AUDIO: return "audio";
        case SS_CONF_DOM_LORA:  return "lora";
        case SS_CONF_DOM_WIFI:  return "wifi";
        case SS_CONF_DOM_BLE:   return "ble";
        case SS_CONF_DOM_COUNT:
        default:                return "?";
    }
}

const char* ss_conf_op_name(ss_conf_op_t op)
{
    switch (op) {
        // power
        case SS_CONF_OP_PWR_INIT:             return "PWR_INIT";
        case SS_CONF_OP_PWR_STATUS:           return "PWR_STATUS";
        case SS_CONF_OP_PWR_ENTER:            return "PWR_ENTER";
        case SS_CONF_OP_PWR_WAKE_SOURCE_ADD:  return "PWR_WAKE_SOURCE_ADD";
        case SS_CONF_OP_PWR_WAKE_TIMER_SET:   return "PWR_WAKE_TIMER_SET";
        case SS_CONF_OP_PWR_WAKE_TIMER_CLEAR: return "PWR_WAKE_TIMER_CLEAR";
        // audio
        case SS_CONF_OP_AUD_MIC_OPEN:    return "AUD_MIC_OPEN";
        case SS_CONF_OP_AUD_MIC_READ:    return "AUD_MIC_READ";
        case SS_CONF_OP_AUD_MIC_CLOSE:   return "AUD_MIC_CLOSE";
        case SS_CONF_OP_AUD_SPK_OPEN:    return "AUD_SPK_OPEN";
        case SS_CONF_OP_AUD_SPK_WRITE:   return "AUD_SPK_WRITE";
        case SS_CONF_OP_AUD_SPK_CLOSE:   return "AUD_SPK_CLOSE";
        case SS_CONF_OP_AUD_SPK_MUTE:    return "AUD_SPK_MUTE";
        case SS_CONF_OP_AUD_SPK_VOLUME:  return "AUD_SPK_VOLUME";
        case SS_CONF_OP_AUD_BUZZER_BEEP: return "AUD_BUZZER_BEEP";
        // LoRa
        case SS_CONF_OP_LORA_INIT:     return "LORA_INIT";
        case SS_CONF_OP_LORA_CONFIG:   return "LORA_CONFIG";
        case SS_CONF_OP_LORA_TX:       return "LORA_TX";
        case SS_CONF_OP_LORA_RX_START: return "LORA_RX_START";
        case SS_CONF_OP_LORA_RX_STOP:  return "LORA_RX_STOP";
        case SS_CONF_OP_LORA_SLEEP:    return "LORA_SLEEP";
        case SS_CONF_OP_LORA_STATS:    return "LORA_STATS";
        // Wi-Fi
        case SS_CONF_OP_WIFI_INIT:   return "WIFI_INIT";
        case SS_CONF_OP_WIFI_CONFIG: return "WIFI_CONFIG";
        case SS_CONF_OP_WIFI_START:  return "WIFI_START";
        case SS_CONF_OP_WIFI_STOP:   return "WIFI_STOP";
        case SS_CONF_OP_WIFI_SLEEP:  return "WIFI_SLEEP";
        // BLE
        case SS_CONF_OP_BLE_INIT:           return "BLE_INIT";
        case SS_CONF_OP_BLE_ADVERTISE:      return "BLE_ADVERTISE";
        case SS_CONF_OP_BLE_STOP_ADVERTISE: return "BLE_STOP_ADVERTISE";
        case SS_CONF_OP_BLE_SCAN_START:     return "BLE_SCAN_START";
        case SS_CONF_OP_BLE_SCAN_STOP:      return "BLE_SCAN_STOP";
        case SS_CONF_OP_BLE_SLEEP:          return "BLE_SLEEP";
        default:                            return "?";
    }
}

static const char* conf_field_name(ss_conf_field_t f)
{
    switch (f) {
        case SS_CONF_FIELD_RET:   return "ret";
        case SS_CONF_FIELD_STATE: return "state";
        case SS_CONF_FIELD_AUX:   return "aux";
        case SS_CONF_FIELD_EXEC:  return "exec";
        default:                  return "?";
    }
}

// ============================================================================
// Hand-rolled, truncation-safe rendering primitives (no <stdio.h>, no heap).
//
// Each returns the running "would-be" length (chars that WOULD be emitted for
// a large-enough buffer). A char is stored only while there is still room for
// it plus the terminating NUL (pos + 1 < cap). The caller NUL-terminates once
// and derives the true written count as min(would-be, cap - 1).
// ============================================================================

static uint32_t conf_app_str(char* out, uint32_t cap, uint32_t pos, const char* s)
{
    for (; *s != '\0'; ++s) {
        if (pos + 1u < cap) {
            out[pos] = *s;
        }
        ++pos;
    }
    return pos;
}

static uint32_t conf_app_dec(char* out, uint32_t cap, uint32_t pos, uint32_t v)
{
    char     tmp[10];
    uint32_t n = 0u;
    if (v == 0u) {
        tmp[n] = '0';
        ++n;
    } else {
        while (v > 0u) {
            tmp[n] = (char)('0' + (int)(v % 10u));
            ++n;
            v /= 10u;
        }
    }
    while (n > 0u) {
        --n;
        if (pos + 1u < cap) {
            out[pos] = tmp[n];
        }
        ++pos;
    }
    return pos;
}

static uint32_t conf_app_hex8(char* out, uint32_t cap, uint32_t pos, uint32_t v)
{
    static const char digits[] = "0123456789abcdef";
    pos = conf_app_str(out, cap, pos, "0x");
    for (int32_t shift = 28; shift >= 0; shift -= 4) {
        char c = digits[(v >> (uint32_t)shift) & 0xFu];
        if (pos + 1u < cap) {
            out[pos] = c;
        }
        ++pos;
    }
    return pos;
}

uint32_t ss_conf_diff_format(const ss_conf_diff_t* d, char* out, uint32_t cap)
{
    if (d == NULL || out == NULL || cap == 0u) {
        return 0u;
    }

    uint32_t pos = 0u;
    pos = conf_app_str(out, cap, pos, "CONF-FAIL dom=");
    pos = conf_app_str(out, cap, pos, ss_conf_domain_name(d->domain));
    pos = conf_app_str(out, cap, pos, " vec=");
    pos = conf_app_str(out, cap, pos, (d->vector_name != NULL) ? d->vector_name : "?");
    pos = conf_app_str(out, cap, pos, " step=");
    pos = conf_app_dec(out, cap, pos, (uint32_t)d->step_index);
    pos = conf_app_str(out, cap, pos, " op=");
    pos = conf_app_str(out, cap, pos, ss_conf_op_name(d->op));
    pos = conf_app_str(out, cap, pos, " field=");
    pos = conf_app_str(out, cap, pos, conf_field_name(d->field));
    pos = conf_app_str(out, cap, pos, " expected=");
    pos = conf_app_hex8(out, cap, pos, d->expected);
    pos = conf_app_str(out, cap, pos, " actual=");
    pos = conf_app_hex8(out, cap, pos, d->actual);

    // Single NUL-terminate; true written count is min(would-be, cap - 1).
    uint32_t term = (pos < cap) ? pos : (cap - 1u);
    out[term]     = '\0';
    return term;
}

// ============================================================================
// Runner
// ============================================================================

static void conf_emit_diff(const ss_conf_env_t* env, const ss_conf_vector_t* vec,
                           uint16_t step_index, ss_conf_op_t op, ss_conf_field_t field,
                           uint32_t expected, uint32_t actual, uint32_t* n_emitted)
{
    ss_conf_diff_t d;
    d.domain      = vec->domain;
    d.vector_name = vec->name;
    d.step_index  = step_index;
    d.op          = op;
    d.field       = field;
    d.expected    = expected;
    d.actual      = actual;

    char line[SS_CONF_DIFF_LINE_MAX];
    (void)ss_conf_diff_format(&d, line, SS_CONF_DIFF_LINE_MAX);
    env->emit(env->ctx, line);
    *n_emitted += 1u;
}

int32_t ss_conf_run(const ss_conf_vector_t* vectors, uint32_t n_vectors,
                    const ss_conf_env_t* env, ss_conf_result_t* out_result)
{
    if (env == NULL || env->reset == NULL || env->exec == NULL || env->emit == NULL) {
        return SS_CONF_E_ARG;
    }
    if (n_vectors > 0u && vectors == NULL) {
        return SS_CONF_E_ARG;
    }

    // Full table validation BEFORE any reset/exec — a malformed table returns
    // SS_CONF_E_VECTOR having executed nothing.
    for (uint32_t i = 0u; i < n_vectors; ++i) {
        const ss_conf_vector_t* v = &vectors[i];
        if (v->name == NULL || v->steps == NULL || v->n_steps < 1u) {
            return SS_CONF_E_VECTOR;
        }
        for (uint32_t s = 0u; s < (uint32_t)v->n_steps; ++s) {
            if (SS_CONF_OP_DOMAIN(v->steps[s].op) != v->domain) {
                return SS_CONF_E_VECTOR;
            }
        }
    }

    uint32_t v_run = 0u, v_pass = 0u, v_skip = 0u, s_run = 0u, d_emit = 0u, failed = 0u;

    for (uint32_t i = 0u; i < n_vectors; ++i) {
        const ss_conf_vector_t* v = &vectors[i];

        if (!env->reset(env->ctx, v)) {
            v_skip += 1u;
            continue;
        }
        v_run += 1u;

        bool vec_failed = false;
        for (uint32_t s = 0u; s < (uint32_t)v->n_steps; ++s) {
            const ss_conf_step_t* step = &v->steps[s];
            ss_conf_actual_t      actual;

            s_run += 1u;
            if (!env->exec(env->ctx, step, &actual)) {
                // Adapter cannot execute the opcode: expected=1 actual=0, then
                // abort the vector (the model is now undefined).
                conf_emit_diff(env, v, (uint16_t)s, step->op, SS_CONF_FIELD_EXEC,
                               1u, 0u, &d_emit);
                vec_failed = true;
                break;
            }

            const ss_conf_expect_t* e = &step->expect;
            if ((e->check & SS_CONF_CHECK_RET) != 0u && actual.ret != e->ret) {
                conf_emit_diff(env, v, (uint16_t)s, step->op, SS_CONF_FIELD_RET,
                               (uint32_t)e->ret, (uint32_t)actual.ret, &d_emit);
                vec_failed = true;
            }
            if ((e->check & SS_CONF_CHECK_STATE) != 0u && actual.state != e->state) {
                conf_emit_diff(env, v, (uint16_t)s, step->op, SS_CONF_FIELD_STATE,
                               e->state, actual.state, &d_emit);
                vec_failed = true;
            }
            if ((e->check & SS_CONF_CHECK_AUX) != 0u && actual.aux != e->aux) {
                conf_emit_diff(env, v, (uint16_t)s, step->op, SS_CONF_FIELD_AUX,
                               e->aux, actual.aux, &d_emit);
                vec_failed = true;
            }
        }

        if (vec_failed) {
            failed += 1u;
        } else {
            v_pass += 1u;
        }
    }

    if (out_result != NULL) {
        out_result->vectors_run     = (uint16_t)v_run;
        out_result->vectors_passed  = (uint16_t)v_pass;
        out_result->vectors_skipped = (uint16_t)v_skip;
        out_result->steps_run       = (uint16_t)s_run;
        out_result->diffs_emitted   = (uint16_t)d_emit;
    }
    return (int32_t)failed;
}
