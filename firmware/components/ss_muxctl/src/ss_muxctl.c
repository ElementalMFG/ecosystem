// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_muxctl.c — ESP-IDF glue implementing the frozen ss_hal_muxctl.h contract.
// All arbitration logic lives in the host-testable ss_muxctl_core; this file
// turns those decisions into GPIO drives and a FreeRTOS ownership gate.
//
// Concurrency model: a binary semaphore (s_token) is the blocking ownership
// gate — a caller must take it before it can own the mux, and gives it back on
// release. The short state struct is additionally guarded by a portMUX spinlock
// for consistent multi-core reads. GPIO writes happen OUTSIDE the spinlock.

#include "ss_hal.h" // ss_hal_has_cap()
#include "ss_hal_muxctl.h"
#include "ss_hal_caps.h"
#include "ss_muxctl_core.h"

#include "board_config.h" // canonical Lite pin map (via ss_hal include path)

#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

static const char* TAG = "ss_muxctl";

static bool s_present;            // cap present on this board
static SemaphoreHandle_t s_token; // binary ownership gate, starts AVAILABLE
static portMUX_TYPE s_spin = portMUX_INITIALIZER_UNLOCKED;
static ss_muxctl_state_t s_state;

// Drive the selector GPIO to the level matching `mode`. Must be called in task
// context and NOT while holding s_spin (gpio_set_level is task-safe only).
// Guarded on the pin being valid: boards without the mux define
// SS_MUX_MIC_RADIO_PIN == -1 (parity placeholder), and the whole GPIO path is
// dead code there (s_present is false), but must still compile clean under
// -Werror (a constant `1ULL << -1` / negative pin is a hard warning).
static void drive_mux(ss_mux_mode_t mode)
{
#if SS_MUX_MIC_RADIO_PIN >= 0
    gpio_set_level(SS_MUX_MIC_RADIO_PIN,
                   mode == SS_MUX_MODE_MIC ? SS_MUX_MIC_LEVEL : SS_MUX_RADIO_LEVEL);
#else
    (void)mode;
#endif
}

esp_err_t ss_mux_init(void)
{
    if (!ss_hal_has_cap(SS_CAP_MUX_MIC_RADIO)) {
        s_present = false;
        ESP_LOGI(TAG, "no mux on this board (no-op)");
        return ESP_OK;
    }

    ss_muxctl_core_init(&s_state, SS_MUX_DEFAULT_MODE);

#if SS_MUX_MIC_RADIO_PIN >= 0
    const gpio_config_t cfg = {
        .pin_bit_mask = 1ULL << SS_MUX_MIC_RADIO_PIN,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&cfg);
#endif
    drive_mux(SS_MUX_DEFAULT_MODE);

    // Guard against double-init: keep the existing token if already created.
    if (s_token == NULL) {
        s_token = xSemaphoreCreateBinary();
        xSemaphoreGive(s_token); // start AVAILABLE (mux free)
    }

    s_present = true;
    return ESP_OK;
}

esp_err_t ss_mux_acquire(ss_mux_mode_t mode, TickType_t timeout, ss_mux_owner_t owner)
{
    if (!s_present) {
        return ESP_OK; // no-op board always succeeds
    }
    if (owner == SS_MUX_OWNER_NONE) { return ESP_ERR_INVALID_ARG; }
    if (xSemaphoreTake(s_token, timeout) != pdTRUE) {
        return ESP_ERR_TIMEOUT; // another owner holds the mux
    }
    // Token held: the core is guaranteed free, so acquire returns OK.
    taskENTER_CRITICAL(&s_spin);
    (void)ss_muxctl_core_acquire(&s_state, mode, owner);
    taskEXIT_CRITICAL(&s_spin);
    drive_mux(mode);
    return ESP_OK;
}

esp_err_t ss_mux_release(ss_mux_owner_t owner)
{
    if (!s_present) { return ESP_OK; }
    taskENTER_CRITICAL(&s_spin);
    const ss_muxctl_result_t r = ss_muxctl_core_release(&s_state, owner);
    taskEXIT_CRITICAL(&s_spin);

    if (r == SS_MUXCTL_NOT_OWNER) { return ESP_ERR_INVALID_STATE; }
    if (r == SS_MUXCTL_INVALID_ARG) { return ESP_ERR_INVALID_ARG; }
    // OK: revert to the resting/radio level and unblock any waiter.
    drive_mux(s_state.default_mode);
    xSemaphoreGive(s_token);
    return ESP_OK;
}

ss_mux_mode_t ss_mux_current_mode(void)
{
    if (!s_present) { return SS_MUX_MODE_RADIO; }
    taskENTER_CRITICAL(&s_spin);
    const ss_mux_mode_t mode = s_state.mode;
    taskEXIT_CRITICAL(&s_spin);
    return mode;
}

ss_mux_owner_t ss_mux_current_owner(void)
{
    if (!s_present) { return SS_MUX_OWNER_NONE; }
    taskENTER_CRITICAL(&s_spin);
    const ss_mux_owner_t owner = s_state.owner;
    taskEXIT_CRITICAL(&s_spin);
    return owner;
}

void ss_mux_force_release(void)
{
    if (!s_present) { return; }
    taskENTER_CRITICAL(&s_spin);
    ss_muxctl_core_force_release(&s_state);
    taskEXIT_CRITICAL(&s_spin);
    drive_mux(s_state.default_mode);
    // Giving an already-available binary semaphore is a harmless no-op, so this
    // is safe whether or not the mux was actually held.
    xSemaphoreGive(s_token);
}
