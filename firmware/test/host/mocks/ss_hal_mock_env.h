// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2026 SS-SP Project Contributors
//
// ss_hal_mock_env.h — pure-C mock ss_conf_env_t for the HAL conformance suite
// (S-03-022). Models the documented reset/exec/emit semantics of the five HAL
// domains (power/audio/lora/wifi/ble) and reports the frozen state/aux
// encodings after each step. The mock claims ALL five domain caps, so no
// builtin vector is skipped on the host. It is the drift oracle: if the mock
// and the builtin vectors disagree, one of them is wrong.
//
// The future on-target env adapter (S-02-015) is a SEPARATE implementation that
// calls the real ss_* HAL; this mock is host-only.

#pragma once

#include "ss_hal_conformance.h"

#ifdef __cplusplus
extern "C" {
#endif

// Capture depth for emitted diff lines (a fully-conforming run emits none).
#define SS_HAL_MOCK_MAX_DIFFS 64u

typedef struct {
    // --- domain model (ss_conf reset restores these to pristine each vector) -
    uint32_t pwr_state; // SS_CONF_PWR_*
    bool pwr_timer_armed;
    uint32_t pwr_gpio_count;

    bool mic_open;
    bool spk_open;
    bool muted;
    uint32_t volume; // 0..100
    uint32_t last_buzz_freq;

    uint32_t lora_state; // SS_CONF_LORA_*
    uint32_t lora_tx_count;
    uint32_t lora_cfg_count;

    uint32_t wifi_state;       // SS_CONF_WIFI_*
    uint32_t wifi_prior_state; // state before sleep, for sleep-off restore
    uint32_t wifi_cfg_count;

    bool ble_init;
    bool ble_adv;
    bool ble_scan;
    bool ble_sleep;
    uint32_t ble_adv_count;
    uint32_t ble_scan_count;

    // --- persistent across vectors (NOT touched by reset) --------------------
    uint64_t claimed_caps; // domain caps this mock advertises
    uint32_t diff_count;   // total diff lines emitted (may exceed capture)
    char diff_lines[SS_HAL_MOCK_MAX_DIFFS][SS_CONF_DIFF_LINE_MAX];
} ss_hal_mock_ctx_t;

// Initialize *ctx to pristine (all model state cleared, capture log empty, all
// five domain caps claimed) and wire *env's reset/exec/emit/ctx to it.
// Pre:  env != NULL, ctx != NULL.
// Post: env is ready to pass to ss_conf_run; ctx.diff_count == 0.
void ss_hal_mock_env_init(ss_conf_env_t* env, ss_hal_mock_ctx_t* ctx);

#ifdef __cplusplus
} // extern "C"
#endif
