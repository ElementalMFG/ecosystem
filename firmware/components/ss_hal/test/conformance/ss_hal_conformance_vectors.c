// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2026 SS-SP Project Contributors
//
// ss_hal_conformance_vectors.c — the builtin static-const conformance vector
// tables (S-03-022). Implements the ENTIRE minimum vector matrix of
// CONFORMANCE_SPEC.md §3; a builder may add vectors but never remove a
// mandatory one. These tables are normative for BOTH the host mock and the
// future on-target driver adapter (S-02-015): a mock that drifts and a driver
// that drifts fail the same vectors with the same diff lines.
//
// Purity (CONFORMANCE_SPEC.md §1): includes ONLY the conformance header. It
// therefore cannot include ss_hal_caps.h; the capability bits below are literal
// mirrors of ss_hal_caps.h with the enumerator cited in a comment (same policy
// the header uses for opcodes/states). Likewise the wake-timer over-max probe
// is the literal from CONFORMANCE_SPEC.md §3 rule 8, citing the macro.

#include "ss_hal_conformance.h"

// --- capability-bit mirrors (ss_hal_caps.h — cannot #include it here) --------
// Verified against firmware/components/ss_hal/include/ss_hal_caps.h.
#define CAP_MIC         (1ULL << 6)  // SS_CAP_MIC
#define CAP_SPEAKER     (1ULL << 7)  // SS_CAP_SPEAKER
#define CAP_BUZZER      (1ULL << 8)  // SS_CAP_BUZZER
#define CAP_RADIO_LORA  (1ULL << 15) // SS_CAP_RADIO_LORA
#define CAP_RADIO_BLE   (1ULL << 17) // SS_CAP_RADIO_BLE
#define CAP_RADIO_WIFI4 (1ULL << 18) // SS_CAP_RADIO_WIFI4

// --- wake-timer over-max probe (CONFORMANCE_SPEC.md §3 rule 8) ---------------
// Literal for SS_PWR_WAKE_TIMER_MAX_US + 1 (ss_hal_power.h caps at 30 days of
// microseconds). If that cap is ever raised, update this in the same commit.
#define PWR_TIMER_OVER_MAX_US (30ULL * 24ULL * 60ULL * 60ULL * 1000000ULL + 1ULL)
#define PWR_TIMER_VALID_US    (1000000ULL) // 1 s — a plausible periodic wake

// --- typed payload mirrors (env adapters translate to the real HAL types) ----
static const ss_conf_audio_fmt_t k_mic_fmt = {16000u, 1u, 16u};
static const ss_conf_audio_fmt_t k_spk_fmt = {16000u, 1u, 16u};

static const ss_conf_lora_cfg_t k_lora_cfg = {
    915000000u, // freq_hz
    14,         // tx_power_dbm
    9u,         // spreading_factor
    125u,       // bandwidth_khz (125/250 only — see header)
    5u,         // coding_rate
    8u,         // preamble_len
    false,      // iq_inverted
    0x12u,      // sync_word
};

static const ss_conf_wifi_cfg_t k_wifi_cfg = {
    "ss-sp-conf", // ssid
    "conf-pass",  // pass
    true,         // sta_mode
    false,        // ap_mode
};

static const char k_ble_name[] = "ss-sp-conf";

// ============================================================================
// POWER (requires_caps = 0 — power is unconditional)
// ============================================================================

static const ss_conf_step_t k_pwr_lifecycle_status[] = {
    {SS_CONF_OP_PWR_INIT, 0u, 0u, 0u, NULL,
     {SS_CONF_RET_OK, 0u, 0u, SS_CONF_CHECK_RET}},
    {SS_CONF_OP_PWR_STATUS, 0u, 0u, 0u, NULL,
     {SS_CONF_RET_OK, 0u, 0u, SS_CONF_CHECK_RET}},
    {SS_CONF_OP_PWR_ENTER, SS_CONF_PWR_LIGHT_SLEEP, 0u, 0u, NULL,
     {SS_CONF_RET_OK, SS_CONF_PWR_LIGHT_SLEEP, 0u,
      SS_CONF_CHECK_RET | SS_CONF_CHECK_STATE}},
};

static const ss_conf_step_t k_pwr_wake_timer_arg_validation[] = {
    // valid arm -> OK, timer armed
    {SS_CONF_OP_PWR_WAKE_TIMER_SET, 0u, 0u, PWR_TIMER_VALID_US, NULL,
     {SS_CONF_RET_OK, 0u, SS_CONF_PWR_AUX_TIMER_ARMED,
      SS_CONF_CHECK_RET | SS_CONF_CHECK_AUX}},
    // us == 0 -> INVALID_ARG, prior arming unchanged
    {SS_CONF_OP_PWR_WAKE_TIMER_SET, 0u, 0u, 0u, NULL,
     {SS_CONF_RET_ERR_INVALID_ARG, 0u, SS_CONF_PWR_AUX_TIMER_ARMED,
      SS_CONF_CHECK_RET | SS_CONF_CHECK_AUX}},
    // us > MAX -> INVALID_ARG, prior arming unchanged
    {SS_CONF_OP_PWR_WAKE_TIMER_SET, 0u, 0u, PWR_TIMER_OVER_MAX_US, NULL,
     {SS_CONF_RET_ERR_INVALID_ARG, 0u, SS_CONF_PWR_AUX_TIMER_ARMED,
      SS_CONF_CHECK_RET | SS_CONF_CHECK_AUX}},
};

static const ss_conf_step_t k_pwr_wake_timer_clear_idempotent[] = {
    {SS_CONF_OP_PWR_WAKE_TIMER_CLEAR, 0u, 0u, 0u, NULL,
     {SS_CONF_RET_OK, 0u, 0u, SS_CONF_CHECK_RET | SS_CONF_CHECK_AUX}},
    {SS_CONF_OP_PWR_WAKE_TIMER_CLEAR, 0u, 0u, 0u, NULL,
     {SS_CONF_RET_OK, 0u, 0u, SS_CONF_CHECK_RET | SS_CONF_CHECK_AUX}},
};

static const ss_conf_step_t k_pwr_shutdown_disarms_wake[] = {
    {SS_CONF_OP_PWR_WAKE_TIMER_SET, 0u, 0u, PWR_TIMER_VALID_US, NULL,
     {SS_CONF_RET_OK, 0u, SS_CONF_PWR_AUX_TIMER_ARMED,
      SS_CONF_CHECK_RET | SS_CONF_CHECK_AUX}},
    {SS_CONF_OP_PWR_ENTER, SS_CONF_PWR_SHUTDOWN, 0u, 0u, NULL,
     {SS_CONF_RET_OK, SS_CONF_PWR_SHUTDOWN, 0u,
      SS_CONF_CHECK_RET | SS_CONF_CHECK_STATE | SS_CONF_CHECK_AUX}},
};

// ============================================================================
// AUDIO
// ============================================================================

static const ss_conf_step_t k_aud_mic_lifecycle[] = {
    {SS_CONF_OP_AUD_MIC_OPEN, 0u, 0u, 0u, &k_mic_fmt,
     {SS_CONF_RET_OK, SS_CONF_AUD_STATE_MIC_OPEN, 0u,
      SS_CONF_CHECK_RET | SS_CONF_CHECK_STATE}},
    {SS_CONF_OP_AUD_MIC_READ, 256u, 100u, 0u, NULL,
     {SS_CONF_RET_OK, 0u, 0u, SS_CONF_CHECK_RET}},
    {SS_CONF_OP_AUD_MIC_CLOSE, 0u, 0u, 0u, NULL,
     {SS_CONF_RET_OK, 0u, 0u, SS_CONF_CHECK_RET | SS_CONF_CHECK_STATE}},
};

static const ss_conf_step_t k_aud_spk_volume_mute[] = {
    {SS_CONF_OP_AUD_SPK_OPEN, 0u, 0u, 0u, &k_spk_fmt,
     {SS_CONF_RET_OK, SS_CONF_AUD_STATE_SPK_OPEN, 0u,
      SS_CONF_CHECK_RET | SS_CONF_CHECK_STATE}},
    {SS_CONF_OP_AUD_SPK_VOLUME, 40u, 0u, 0u, NULL,
     {SS_CONF_RET_OK, 0u, 40u, SS_CONF_CHECK_RET | SS_CONF_CHECK_AUX}},
    {SS_CONF_OP_AUD_SPK_MUTE, 1u, 0u, 0u, NULL,
     {SS_CONF_RET_OK, SS_CONF_AUD_STATE_SPK_OPEN | SS_CONF_AUD_STATE_MUTED, 0u,
      SS_CONF_CHECK_RET | SS_CONF_CHECK_STATE}},
    {SS_CONF_OP_AUD_SPK_MUTE, 0u, 0u, 0u, NULL,
     {SS_CONF_RET_OK, SS_CONF_AUD_STATE_SPK_OPEN, 0u,
      SS_CONF_CHECK_RET | SS_CONF_CHECK_STATE}},
};

static const ss_conf_step_t k_aud_use_before_open[] = {
    {SS_CONF_OP_AUD_MIC_READ, 256u, 100u, 0u, NULL,
     {SS_CONF_RET_ERR_INVALID_STATE, 0u, 0u, SS_CONF_CHECK_RET}},
    {SS_CONF_OP_AUD_SPK_WRITE, 256u, 0u, 0u, NULL,
     {SS_CONF_RET_ERR_INVALID_STATE, 0u, 0u, SS_CONF_CHECK_RET}},
};

static const ss_conf_step_t k_aud_buzzer_beep[] = {
    {SS_CONF_OP_AUD_BUZZER_BEEP, 2000u, 100u, 0u, NULL,
     {SS_CONF_RET_OK, 0u, (2000u << SS_CONF_AUD_AUX_BUZZ_SHIFT),
      SS_CONF_CHECK_RET | SS_CONF_CHECK_AUX}},
};

// ============================================================================
// LORA (requires_caps = SS_CAP_RADIO_LORA)
// ============================================================================

static const ss_conf_step_t k_lora_init_config_tx[] = {
    {SS_CONF_OP_LORA_INIT, 0u, 0u, 0u, NULL,
     {SS_CONF_RET_OK, SS_CONF_LORA_IDLE, 0u,
      SS_CONF_CHECK_RET | SS_CONF_CHECK_STATE}},
    {SS_CONF_OP_LORA_CONFIG, 0u, 0u, 0u, &k_lora_cfg,
     {SS_CONF_RET_OK, 0u, (1u << SS_CONF_LORA_AUX_CFG_SHIFT),
      SS_CONF_CHECK_RET | SS_CONF_CHECK_AUX}},
    {SS_CONF_OP_LORA_TX, 32u, 1000u, 0u, NULL,
     {SS_CONF_RET_OK, 0u, ((1u << SS_CONF_LORA_AUX_CFG_SHIFT) | 1u),
      SS_CONF_CHECK_RET | SS_CONF_CHECK_AUX}},
};

static const ss_conf_step_t k_lora_rx_start_stop[] = {
    {SS_CONF_OP_LORA_INIT, 0u, 0u, 0u, NULL,
     {SS_CONF_RET_OK, SS_CONF_LORA_IDLE, 0u,
      SS_CONF_CHECK_RET | SS_CONF_CHECK_STATE}},
    {SS_CONF_OP_LORA_RX_START, 0u, 0u, 0u, NULL,
     {SS_CONF_RET_OK, SS_CONF_LORA_RX, 0u,
      SS_CONF_CHECK_RET | SS_CONF_CHECK_STATE}},
    {SS_CONF_OP_LORA_RX_STOP, 0u, 0u, 0u, NULL,
     {SS_CONF_RET_OK, SS_CONF_LORA_IDLE, 0u,
      SS_CONF_CHECK_RET | SS_CONF_CHECK_STATE}},
};

static const ss_conf_step_t k_lora_use_before_init[] = {
    {SS_CONF_OP_LORA_CONFIG, 0u, 0u, 0u, &k_lora_cfg,
     {SS_CONF_RET_ERR_INVALID_STATE, 0u, 0u, SS_CONF_CHECK_RET}},
    {SS_CONF_OP_LORA_TX, 32u, 1000u, 0u, NULL,
     {SS_CONF_RET_ERR_INVALID_STATE, 0u, 0u, SS_CONF_CHECK_RET}},
};

static const ss_conf_step_t k_lora_sleep_wake[] = {
    {SS_CONF_OP_LORA_INIT, 0u, 0u, 0u, NULL,
     {SS_CONF_RET_OK, SS_CONF_LORA_IDLE, 0u,
      SS_CONF_CHECK_RET | SS_CONF_CHECK_STATE}},
    {SS_CONF_OP_LORA_SLEEP, 1u, 0u, 0u, NULL,
     {SS_CONF_RET_OK, SS_CONF_LORA_SLEEP, 0u,
      SS_CONF_CHECK_RET | SS_CONF_CHECK_STATE}},
    {SS_CONF_OP_LORA_SLEEP, 0u, 0u, 0u, NULL,
     {SS_CONF_RET_OK, SS_CONF_LORA_IDLE, 0u,
      SS_CONF_CHECK_RET | SS_CONF_CHECK_STATE}},
};

// ============================================================================
// WIFI (requires_caps = SS_CAP_RADIO_WIFI4)
// ============================================================================

static const ss_conf_step_t k_wifi_lifecycle[] = {
    {SS_CONF_OP_WIFI_INIT, 0u, 0u, 0u, NULL,
     {SS_CONF_RET_OK, SS_CONF_WIFI_READY, 0u,
      SS_CONF_CHECK_RET | SS_CONF_CHECK_STATE}},
    {SS_CONF_OP_WIFI_CONFIG, 0u, 0u, 0u, &k_wifi_cfg,
     {SS_CONF_RET_OK, 0u, 1u, SS_CONF_CHECK_RET | SS_CONF_CHECK_AUX}},
    {SS_CONF_OP_WIFI_START, 0u, 0u, 0u, NULL,
     {SS_CONF_RET_OK, SS_CONF_WIFI_STARTED, 0u,
      SS_CONF_CHECK_RET | SS_CONF_CHECK_STATE}},
    {SS_CONF_OP_WIFI_STOP, 0u, 0u, 0u, NULL,
     {SS_CONF_RET_OK, SS_CONF_WIFI_READY, 0u,
      SS_CONF_CHECK_RET | SS_CONF_CHECK_STATE}},
};

static const ss_conf_step_t k_wifi_config_null[] = {
    {SS_CONF_OP_WIFI_INIT, 0u, 0u, 0u, NULL,
     {SS_CONF_RET_OK, 0u, 0u, SS_CONF_CHECK_RET}},
    {SS_CONF_OP_WIFI_CONFIG, 0u, 0u, 0u, NULL,
     {SS_CONF_RET_ERR_INVALID_ARG, 0u, 0u,
      SS_CONF_CHECK_RET | SS_CONF_CHECK_AUX}},
};

static const ss_conf_step_t k_wifi_sleep_toggle[] = {
    {SS_CONF_OP_WIFI_INIT, 0u, 0u, 0u, NULL,
     {SS_CONF_RET_OK, SS_CONF_WIFI_READY, 0u,
      SS_CONF_CHECK_RET | SS_CONF_CHECK_STATE}},
    {SS_CONF_OP_WIFI_START, 0u, 0u, 0u, NULL,
     {SS_CONF_RET_OK, SS_CONF_WIFI_STARTED, 0u,
      SS_CONF_CHECK_RET | SS_CONF_CHECK_STATE}},
    {SS_CONF_OP_WIFI_SLEEP, 1u, 0u, 0u, NULL,
     {SS_CONF_RET_OK, SS_CONF_WIFI_SLEEPING, 0u,
      SS_CONF_CHECK_RET | SS_CONF_CHECK_STATE}},
    // sleep off -> prior state (STARTED)
    {SS_CONF_OP_WIFI_SLEEP, 0u, 0u, 0u, NULL,
     {SS_CONF_RET_OK, SS_CONF_WIFI_STARTED, 0u,
      SS_CONF_CHECK_RET | SS_CONF_CHECK_STATE}},
};

// ============================================================================
// BLE (requires_caps = SS_CAP_RADIO_BLE; NO pairing/crypto — T1)
// ============================================================================

static const ss_conf_step_t k_ble_advertise_stop[] = {
    {SS_CONF_OP_BLE_INIT, 0u, 0u, 0u, NULL,
     {SS_CONF_RET_OK, SS_CONF_BLE_STATE_INIT, 0u,
      SS_CONF_CHECK_RET | SS_CONF_CHECK_STATE}},
    {SS_CONF_OP_BLE_ADVERTISE, 0u, 0u, 0u, k_ble_name,
     {SS_CONF_RET_OK, SS_CONF_BLE_STATE_INIT | SS_CONF_BLE_STATE_ADVERTISING, 1u,
      SS_CONF_CHECK_RET | SS_CONF_CHECK_STATE | SS_CONF_CHECK_AUX}},
    {SS_CONF_OP_BLE_STOP_ADVERTISE, 0u, 0u, 0u, NULL,
     {SS_CONF_RET_OK, SS_CONF_BLE_STATE_INIT, 0u,
      SS_CONF_CHECK_RET | SS_CONF_CHECK_STATE}},
};

static const ss_conf_step_t k_ble_scan_lifecycle[] = {
    {SS_CONF_OP_BLE_INIT, 0u, 0u, 0u, NULL,
     {SS_CONF_RET_OK, SS_CONF_BLE_STATE_INIT, 0u,
      SS_CONF_CHECK_RET | SS_CONF_CHECK_STATE}},
    {SS_CONF_OP_BLE_SCAN_START, 0u, 0u, 0u, NULL,
     {SS_CONF_RET_OK, SS_CONF_BLE_STATE_INIT | SS_CONF_BLE_STATE_SCANNING,
      (1u << SS_CONF_BLE_AUX_SCAN_SHIFT),
      SS_CONF_CHECK_RET | SS_CONF_CHECK_STATE | SS_CONF_CHECK_AUX}},
    {SS_CONF_OP_BLE_SCAN_STOP, 0u, 0u, 0u, NULL,
     {SS_CONF_RET_OK, SS_CONF_BLE_STATE_INIT, 0u,
      SS_CONF_CHECK_RET | SS_CONF_CHECK_STATE}},
};

static const ss_conf_step_t k_ble_advertise_null_name[] = {
    {SS_CONF_OP_BLE_INIT, 0u, 0u, 0u, NULL,
     {SS_CONF_RET_OK, 0u, 0u, SS_CONF_CHECK_RET}},
    {SS_CONF_OP_BLE_ADVERTISE, 0u, 0u, 0u, NULL,
     {SS_CONF_RET_ERR_INVALID_ARG, 0u, 0u, SS_CONF_CHECK_RET}},
};

static const ss_conf_step_t k_ble_use_before_init[] = {
    {SS_CONF_OP_BLE_ADVERTISE, 0u, 0u, 0u, k_ble_name,
     {SS_CONF_RET_ERR_INVALID_STATE, 0u, 0u, SS_CONF_CHECK_RET}},
};

// ============================================================================
// Vector table
// ============================================================================

#define VEC(dom, nm, caps, steps) \
    {(dom), (nm), (caps), (steps), (uint16_t)(sizeof(steps) / sizeof((steps)[0]))}

static const ss_conf_vector_t k_vectors[] = {
    // power (unconditional — requires_caps 0)
    VEC(SS_CONF_DOM_POWER, "lifecycle-status", 0u, k_pwr_lifecycle_status),
    VEC(SS_CONF_DOM_POWER, "wake-timer-arg-validation", 0u,
        k_pwr_wake_timer_arg_validation),
    VEC(SS_CONF_DOM_POWER, "wake-timer-clear-idempotent", 0u,
        k_pwr_wake_timer_clear_idempotent),
    VEC(SS_CONF_DOM_POWER, "shutdown-disarms-wake", 0u, k_pwr_shutdown_disarms_wake),

    // audio
    VEC(SS_CONF_DOM_AUDIO, "mic-lifecycle", CAP_MIC, k_aud_mic_lifecycle),
    VEC(SS_CONF_DOM_AUDIO, "spk-volume-mute", CAP_SPEAKER, k_aud_spk_volume_mute),
    VEC(SS_CONF_DOM_AUDIO, "use-before-open", CAP_MIC | CAP_SPEAKER,
        k_aud_use_before_open),
    VEC(SS_CONF_DOM_AUDIO, "buzzer-beep", CAP_BUZZER, k_aud_buzzer_beep),

    // lora
    VEC(SS_CONF_DOM_LORA, "init-config-tx", CAP_RADIO_LORA, k_lora_init_config_tx),
    VEC(SS_CONF_DOM_LORA, "rx-start-stop", CAP_RADIO_LORA, k_lora_rx_start_stop),
    VEC(SS_CONF_DOM_LORA, "use-before-init", CAP_RADIO_LORA, k_lora_use_before_init),
    VEC(SS_CONF_DOM_LORA, "sleep-wake", CAP_RADIO_LORA, k_lora_sleep_wake),

    // wifi
    VEC(SS_CONF_DOM_WIFI, "lifecycle", CAP_RADIO_WIFI4, k_wifi_lifecycle),
    VEC(SS_CONF_DOM_WIFI, "config-null", CAP_RADIO_WIFI4, k_wifi_config_null),
    VEC(SS_CONF_DOM_WIFI, "sleep-toggle", CAP_RADIO_WIFI4, k_wifi_sleep_toggle),

    // ble
    VEC(SS_CONF_DOM_BLE, "advertise-stop", CAP_RADIO_BLE, k_ble_advertise_stop),
    VEC(SS_CONF_DOM_BLE, "scan-lifecycle", CAP_RADIO_BLE, k_ble_scan_lifecycle),
    VEC(SS_CONF_DOM_BLE, "advertise-null-name", CAP_RADIO_BLE,
        k_ble_advertise_null_name),
    VEC(SS_CONF_DOM_BLE, "use-before-init", CAP_RADIO_BLE, k_ble_use_before_init),
};

const ss_conf_vector_t* ss_conf_builtin_vectors(uint32_t* out_n)
{
    if (out_n == NULL) {
        return NULL;
    }
    *out_n = (uint32_t)(sizeof(k_vectors) / sizeof(k_vectors[0]));
    return k_vectors;
}
