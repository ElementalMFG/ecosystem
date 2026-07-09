// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2026 SS-SP Project Contributors
//
// ss_hal_mock_env.c — pure-C mock backend for the HAL conformance suite
// (S-03-022). Each exec models the documented HAL semantics for one opcode and
// reports the frozen state/aux word for the opcode's domain afterwards. Where a
// domain header is silent, it follows house policy encoded by the builtin
// vectors: NULL required pointer -> INVALID_ARG (0x102); use-before-open/init
// -> INVALID_STATE (0x103).

#include "ss_hal_mock_env.h"

// --- capability-bit mirrors (ss_hal_caps.h — pure TU cannot include it) ------
#define CAP_MIC (1ULL << 6)          // SS_CAP_MIC
#define CAP_SPEAKER (1ULL << 7)      // SS_CAP_SPEAKER
#define CAP_BUZZER (1ULL << 8)       // SS_CAP_BUZZER
#define CAP_RADIO_LORA (1ULL << 15)  // SS_CAP_RADIO_LORA
#define CAP_RADIO_BLE (1ULL << 17)   // SS_CAP_RADIO_BLE
#define CAP_RADIO_WIFI4 (1ULL << 18) // SS_CAP_RADIO_WIFI4

// SS_PWR_WAKE_TIMER_MAX_US mirror (ss_hal_power.h caps at 30 days of us).
#define PWR_TIMER_MAX_US (30ULL * 24ULL * 60ULL * 60ULL * 1000000ULL)

// ============================================================================
// Frozen state/aux word computation per domain
// ============================================================================

static uint32_t pwr_state_word(const ss_hal_mock_ctx_t* c)
{
    return c->pwr_state;
}

static uint32_t pwr_aux_word(const ss_hal_mock_ctx_t* c)
{
    uint32_t aux = c->pwr_timer_armed ? SS_CONF_PWR_AUX_TIMER_ARMED : 0u;
    aux |= (c->pwr_gpio_count << SS_CONF_PWR_AUX_GPIO_SHIFT);
    return aux;
}

static uint32_t aud_state_word(const ss_hal_mock_ctx_t* c)
{
    uint32_t s = 0u;
    if (c->mic_open) { s |= SS_CONF_AUD_STATE_MIC_OPEN; }
    if (c->spk_open) { s |= SS_CONF_AUD_STATE_SPK_OPEN; }
    if (c->muted) { s |= SS_CONF_AUD_STATE_MUTED; }
    return s;
}

static uint32_t aud_aux_word(const ss_hal_mock_ctx_t* c)
{
    return (c->volume & SS_CONF_AUD_AUX_VOL_MASK) |
           (c->last_buzz_freq << SS_CONF_AUD_AUX_BUZZ_SHIFT);
}

static uint32_t lora_state_word(const ss_hal_mock_ctx_t* c)
{
    return c->lora_state;
}

static uint32_t lora_aux_word(const ss_hal_mock_ctx_t* c)
{
    return (c->lora_tx_count & SS_CONF_LORA_AUX_TX_MASK) |
           (c->lora_cfg_count << SS_CONF_LORA_AUX_CFG_SHIFT);
}

static uint32_t wifi_state_word(const ss_hal_mock_ctx_t* c)
{
    return c->wifi_state;
}
static uint32_t wifi_aux_word(const ss_hal_mock_ctx_t* c)
{
    return c->wifi_cfg_count;
}

static uint32_t ble_state_word(const ss_hal_mock_ctx_t* c)
{
    uint32_t s = 0u;
    if (c->ble_init) { s |= SS_CONF_BLE_STATE_INIT; }
    if (c->ble_adv) { s |= SS_CONF_BLE_STATE_ADVERTISING; }
    if (c->ble_scan) { s |= SS_CONF_BLE_STATE_SCANNING; }
    if (c->ble_sleep) { s |= SS_CONF_BLE_STATE_SLEEPING; }
    return s;
}

static uint32_t ble_aux_word(const ss_hal_mock_ctx_t* c)
{
    return (c->ble_adv_count & SS_CONF_BLE_AUX_ADV_MASK) |
           (c->ble_scan_count << SS_CONF_BLE_AUX_SCAN_SHIFT);
}

// ============================================================================
// Per-opcode execution (returns the HAL-shaped return code)
// ============================================================================

static int32_t exec_power(ss_hal_mock_ctx_t* c, const ss_conf_step_t* step)
{
    switch (step->op) {
    case SS_CONF_OP_PWR_INIT:
        c->pwr_state = SS_CONF_PWR_ON;
        return SS_CONF_RET_OK;
    case SS_CONF_OP_PWR_STATUS:
        return SS_CONF_RET_OK;
    case SS_CONF_OP_PWR_ENTER:
        c->pwr_state = step->arg_a;
        if (step->arg_a == SS_CONF_PWR_SHUTDOWN) {
            // SHUTDOWN disables every wake source (ss_hal_power.h).
            c->pwr_timer_armed = false;
            c->pwr_gpio_count = 0u;
        }
        return SS_CONF_RET_OK;
    case SS_CONF_OP_PWR_WAKE_SOURCE_ADD:
        c->pwr_gpio_count += 1u;
        return SS_CONF_RET_OK;
    case SS_CONF_OP_PWR_WAKE_TIMER_SET:
        // us == 0 or us > MAX -> INVALID_ARG, prior arming unchanged.
        if (step->arg_u64 == 0u || step->arg_u64 > PWR_TIMER_MAX_US) {
            return SS_CONF_RET_ERR_INVALID_ARG;
        }
        c->pwr_timer_armed = true;
        return SS_CONF_RET_OK;
    case SS_CONF_OP_PWR_WAKE_TIMER_CLEAR:
        c->pwr_timer_armed = false; // idempotent
        return SS_CONF_RET_OK;
    default:
        return SS_CONF_RET_FAIL;
    }
}

static int32_t exec_audio(ss_hal_mock_ctx_t* c, const ss_conf_step_t* step)
{
    switch (step->op) {
    case SS_CONF_OP_AUD_MIC_OPEN:
        if (step->arg_ptr == NULL) { return SS_CONF_RET_ERR_INVALID_ARG; }
        c->mic_open = true;
        return SS_CONF_RET_OK;
    case SS_CONF_OP_AUD_MIC_READ:
        if (!c->mic_open) { return SS_CONF_RET_ERR_INVALID_STATE; }
        return SS_CONF_RET_OK;
    case SS_CONF_OP_AUD_MIC_CLOSE:
        c->mic_open = false;
        return SS_CONF_RET_OK;
    case SS_CONF_OP_AUD_SPK_OPEN:
        if (step->arg_ptr == NULL) { return SS_CONF_RET_ERR_INVALID_ARG; }
        c->spk_open = true;
        return SS_CONF_RET_OK;
    case SS_CONF_OP_AUD_SPK_WRITE:
        if (!c->spk_open) { return SS_CONF_RET_ERR_INVALID_STATE; }
        return SS_CONF_RET_OK;
    case SS_CONF_OP_AUD_SPK_CLOSE:
        c->spk_open = false;
        c->muted = false;
        return SS_CONF_RET_OK;
    case SS_CONF_OP_AUD_SPK_MUTE:
        if (!c->spk_open) { return SS_CONF_RET_ERR_INVALID_STATE; }
        c->muted = (step->arg_a != 0u);
        return SS_CONF_RET_OK;
    case SS_CONF_OP_AUD_SPK_VOLUME:
        if (!c->spk_open) { return SS_CONF_RET_ERR_INVALID_STATE; }
        if (step->arg_a > 100u) { return SS_CONF_RET_ERR_INVALID_ARG; }
        c->volume = step->arg_a;
        return SS_CONF_RET_OK;
    case SS_CONF_OP_AUD_BUZZER_BEEP:
        c->last_buzz_freq = step->arg_a;
        return SS_CONF_RET_OK;
    default:
        return SS_CONF_RET_FAIL;
    }
}

static int32_t exec_lora(ss_hal_mock_ctx_t* c, const ss_conf_step_t* step)
{
    switch (step->op) {
    case SS_CONF_OP_LORA_INIT:
        c->lora_state = SS_CONF_LORA_IDLE;
        return SS_CONF_RET_OK;
    case SS_CONF_OP_LORA_CONFIG:
        if (c->lora_state == SS_CONF_LORA_UNINIT) { return SS_CONF_RET_ERR_INVALID_STATE; }
        if (step->arg_ptr == NULL) { return SS_CONF_RET_ERR_INVALID_ARG; }
        c->lora_cfg_count += 1u;
        return SS_CONF_RET_OK;
    case SS_CONF_OP_LORA_TX:
        if (c->lora_state == SS_CONF_LORA_UNINIT) { return SS_CONF_RET_ERR_INVALID_STATE; }
        c->lora_tx_count += 1u;
        return SS_CONF_RET_OK;
    case SS_CONF_OP_LORA_RX_START:
        if (c->lora_state == SS_CONF_LORA_UNINIT) { return SS_CONF_RET_ERR_INVALID_STATE; }
        c->lora_state = SS_CONF_LORA_RX;
        return SS_CONF_RET_OK;
    case SS_CONF_OP_LORA_RX_STOP:
        c->lora_state = SS_CONF_LORA_IDLE;
        return SS_CONF_RET_OK;
    case SS_CONF_OP_LORA_SLEEP:
        if (c->lora_state == SS_CONF_LORA_UNINIT) { return SS_CONF_RET_ERR_INVALID_STATE; }
        c->lora_state = (step->arg_a != 0u) ? SS_CONF_LORA_SLEEP : SS_CONF_LORA_IDLE;
        return SS_CONF_RET_OK;
    case SS_CONF_OP_LORA_STATS:
        return SS_CONF_RET_OK;
    default:
        return SS_CONF_RET_FAIL;
    }
}

static int32_t exec_wifi(ss_hal_mock_ctx_t* c, const ss_conf_step_t* step)
{
    switch (step->op) {
    case SS_CONF_OP_WIFI_INIT:
        c->wifi_state = SS_CONF_WIFI_READY;
        return SS_CONF_RET_OK;
    case SS_CONF_OP_WIFI_CONFIG:
        if (c->wifi_state == SS_CONF_WIFI_UNINIT) { return SS_CONF_RET_ERR_INVALID_STATE; }
        if (step->arg_ptr == NULL) { return SS_CONF_RET_ERR_INVALID_ARG; }
        c->wifi_cfg_count += 1u;
        return SS_CONF_RET_OK;
    case SS_CONF_OP_WIFI_START:
        if (c->wifi_state == SS_CONF_WIFI_UNINIT) { return SS_CONF_RET_ERR_INVALID_STATE; }
        c->wifi_state = SS_CONF_WIFI_STARTED;
        return SS_CONF_RET_OK;
    case SS_CONF_OP_WIFI_STOP:
        c->wifi_state = SS_CONF_WIFI_READY;
        return SS_CONF_RET_OK;
    case SS_CONF_OP_WIFI_SLEEP:
        if (step->arg_a != 0u) {
            c->wifi_prior_state = c->wifi_state;
            c->wifi_state = SS_CONF_WIFI_SLEEPING;
        } else {
            c->wifi_state = c->wifi_prior_state;
        }
        return SS_CONF_RET_OK;
    default:
        return SS_CONF_RET_FAIL;
    }
}

static int32_t exec_ble(ss_hal_mock_ctx_t* c, const ss_conf_step_t* step)
{
    switch (step->op) {
    case SS_CONF_OP_BLE_INIT:
        c->ble_init = true;
        return SS_CONF_RET_OK;
    case SS_CONF_OP_BLE_ADVERTISE:
        if (!c->ble_init) { return SS_CONF_RET_ERR_INVALID_STATE; }
        if (step->arg_ptr == NULL) { return SS_CONF_RET_ERR_INVALID_ARG; }
        c->ble_adv = true;
        c->ble_adv_count += 1u;
        return SS_CONF_RET_OK;
    case SS_CONF_OP_BLE_STOP_ADVERTISE:
        c->ble_adv = false;
        return SS_CONF_RET_OK;
    case SS_CONF_OP_BLE_SCAN_START:
        if (!c->ble_init) { return SS_CONF_RET_ERR_INVALID_STATE; }
        c->ble_scan = true;
        c->ble_scan_count += 1u;
        return SS_CONF_RET_OK;
    case SS_CONF_OP_BLE_SCAN_STOP:
        c->ble_scan = false;
        return SS_CONF_RET_OK;
    case SS_CONF_OP_BLE_SLEEP:
        c->ble_sleep = (step->arg_a != 0u);
        return SS_CONF_RET_OK;
    default:
        return SS_CONF_RET_FAIL;
    }
}

// ============================================================================
// vtable
// ============================================================================

static void mock_reset_model(ss_hal_mock_ctx_t* c)
{
    c->pwr_state = SS_CONF_PWR_ON;
    c->pwr_timer_armed = false;
    c->pwr_gpio_count = 0u;

    c->mic_open = false;
    c->spk_open = false;
    c->muted = false;
    c->volume = 0u;
    c->last_buzz_freq = 0u;

    c->lora_state = SS_CONF_LORA_UNINIT;
    c->lora_tx_count = 0u;
    c->lora_cfg_count = 0u;

    c->wifi_state = SS_CONF_WIFI_UNINIT;
    c->wifi_prior_state = SS_CONF_WIFI_UNINIT;
    c->wifi_cfg_count = 0u;

    c->ble_init = false;
    c->ble_adv = false;
    c->ble_scan = false;
    c->ble_sleep = false;
    c->ble_adv_count = 0u;
    c->ble_scan_count = 0u;
}

static bool mock_reset(void* ctx, const ss_conf_vector_t* vec)
{
    ss_hal_mock_ctx_t* c = (ss_hal_mock_ctx_t*)ctx;
    // Honest skip path (target parity): skip when a required cap is missing.
    if ((vec->requires_caps & ~c->claimed_caps) != 0u) { return false; }
    mock_reset_model(c);
    return true;
}

static bool mock_exec(void* ctx, const ss_conf_step_t* step, ss_conf_actual_t* out)
{
    ss_hal_mock_ctx_t* c = (ss_hal_mock_ctx_t*)ctx;
    ss_conf_domain_t dom = SS_CONF_OP_DOMAIN(step->op);
    int32_t ret;

    switch (dom) {
    case SS_CONF_DOM_POWER:
        ret = exec_power(c, step);
        out->state = pwr_state_word(c);
        out->aux = pwr_aux_word(c);
        break;
    case SS_CONF_DOM_AUDIO:
        ret = exec_audio(c, step);
        out->state = aud_state_word(c);
        out->aux = aud_aux_word(c);
        break;
    case SS_CONF_DOM_LORA:
        ret = exec_lora(c, step);
        out->state = lora_state_word(c);
        out->aux = lora_aux_word(c);
        break;
    case SS_CONF_DOM_WIFI:
        ret = exec_wifi(c, step);
        out->state = wifi_state_word(c);
        out->aux = wifi_aux_word(c);
        break;
    case SS_CONF_DOM_BLE:
        ret = exec_ble(c, step);
        out->state = ble_state_word(c);
        out->aux = ble_aux_word(c);
        break;
    case SS_CONF_DOM_COUNT:
    default:
        return false; // no such domain — adapter gap
    }

    out->ret = ret;
    return true;
}

static void mock_emit(void* ctx, const char* line)
{
    ss_hal_mock_ctx_t* c = (ss_hal_mock_ctx_t*)ctx;
    if (c->diff_count < SS_HAL_MOCK_MAX_DIFFS) {
        char* dst = c->diff_lines[c->diff_count];
        uint32_t i = 0u;
        while (line[i] != '\0' && i + 1u < SS_CONF_DIFF_LINE_MAX) {
            dst[i] = line[i];
            ++i;
        }
        dst[i] = '\0';
    }
    c->diff_count += 1u; // count truthfully even past capture depth
}

void ss_hal_mock_env_init(ss_conf_env_t* env, ss_hal_mock_ctx_t* ctx)
{
    mock_reset_model(ctx);
    ctx->claimed_caps =
        CAP_MIC | CAP_SPEAKER | CAP_BUZZER | CAP_RADIO_LORA | CAP_RADIO_BLE | CAP_RADIO_WIFI4;
    ctx->diff_count = 0u;

    env->reset = mock_reset;
    env->exec = mock_exec;
    env->emit = mock_emit;
    env->ctx = ctx;
}
