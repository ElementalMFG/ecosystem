// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_spk.c — ESP-IDF glue implementing the speaker half of the frozen
// ss_hal_audio.h contract (ss_spk_open / ss_spk_write / ss_spk_close /
// ss_spk_mute / ss_spk_volume). All format validation, DMA sizing, pop-free
// sequencing, and Q15 volume math live in the host-testable ss_spk_core; this
// file turns those decisions into I2S std-driver calls and amp enable toggles.
//
// DEGRADATION: the speaker is a capability, not a given. On boards whose caps
// mask lacks SS_CAP_SPEAKER ss_spk_open returns ESP_ERR_NOT_SUPPORTED before
// touching any pin, and write/close are safe. App code must gate on
// ss_hal_has_cap(SS_CAP_SPEAKER).
//
// MUX: the speaker takes NO mux and makes NO ss_mux_* calls — it is
// mux-independent per ss_hal_audio.h (its I2S pins are dedicated, not shared
// with the radio). That is what keeps the sidetone path available while the mic
// owns the mux during capture.
//
// AMP: the Lite class-D amp (MAX98357A-class) has an active-high SD/enable pin
// on SS_SPK_PIN_MUTE. It has no gain register, so ss_spk_volume applies a Q15
// gain in software (ss_spk_core_apply_gain) inside ss_spk_write. The amp is kept
// off while the I2S clocks are down: open brings the clocks up, settles, then
// enables the amp; close mutes the amp, settles, then stops the clocks — the
// pop-free order computed by ss_spk_core_open_seq / ss_spk_core_close_seq.

#include "ss_hal.h" // ss_hal_has_cap()
#include "ss_hal_audio.h"
#include "ss_hal_caps.h"
#include "ss_spk_core.h"

#include "board_config.h" // canonical Lite pin map (via ss_hal include path)

#include "driver/gpio.h"
#include "driver/i2s_std.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "ss_spk";

// DMA buffering target: ~10 ms bounds per-write latency well within playback.
#define SS_SPK_TARGET_LATENCY_MS 10u

static i2s_chan_handle_t s_tx;     // TX channel handle, NULL when closed
static bool s_opened;              // true between a successful open and close
static int16_t s_gain_q15 = 32767; // software volume, defaults to 100%
static bool s_muted;               // true while the amp is held muted

// Map a core validation result onto the frozen esp_err_t contract. NULL args are
// invalid arguments; an absent speaker capability or unsupported format
// parameter is "not supported" for this board.
static esp_err_t map_validate(ss_spk_core_result_t r)
{
    switch (r) {
    case SS_SPK_OK:
        return ESP_OK;
    case SS_SPK_ERR_NULL:
        return ESP_ERR_INVALID_ARG;
    case SS_SPK_ERR_NO_SPK_CAP:
    case SS_SPK_ERR_RATE:
    case SS_SPK_ERR_CHANNELS:
    case SS_SPK_ERR_BITS:
    default:
        return ESP_ERR_NOT_SUPPORTED;
    }
}

// Drive the class-D amp SD/enable pin. No-op on boards without a mute GPIO.
static void amp_set(bool enable)
{
#if SS_SPK_HAS_MUTE_GPIO
    gpio_set_level((gpio_num_t)SS_SPK_PIN_MUTE, enable ? 1 : 0);
#else
    (void)enable;
#endif
}

esp_err_t ss_spk_open(const ss_audio_fmt_t* fmt)
{
    if (!ss_hal_has_cap(SS_CAP_SPEAKER)) {
        ESP_LOGD(TAG, "no speaker on this board");
        return ESP_ERR_NOT_SUPPORTED;
    }
    if (s_opened) { return ESP_ERR_INVALID_STATE; }

    const ss_spk_core_result_t vr = ss_spk_core_validate_fmt(fmt, true);
    if (vr != SS_SPK_OK) { return map_validate(vr); }

#if SS_SPK_PIN_BCLK >= 0
    ss_spk_dma_plan_t plan;
    const ss_spk_core_result_t pr = ss_spk_core_plan_dma(fmt, SS_SPK_TARGET_LATENCY_MS, &plan);
    if (pr != SS_SPK_OK) { return map_validate(pr); }

    // Hold the amp shut down before the clocks start so nothing thumps the
    // driver while the I2S line is undriven (pop-free open, see file header).
#if SS_SPK_HAS_MUTE_GPIO
    gpio_set_direction((gpio_num_t)SS_SPK_PIN_MUTE, GPIO_MODE_OUTPUT);
#endif
    amp_set(false);

    i2s_chan_config_t chan_cfg =
        I2S_CHANNEL_DEFAULT_CONFIG((i2s_port_t)SS_SPK_I2S_PORT, I2S_ROLE_MASTER);
    chan_cfg.dma_desc_num = plan.desc_num;
    chan_cfg.dma_frame_num = plan.frame_num;
    esp_err_t err = i2s_new_channel(&chan_cfg, &s_tx, NULL); // TX handle, RX NULL
    if (err != ESP_OK) { return err; }

    i2s_std_config_t std = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(SS_SPK_SAMPLE_RATE_HZ),
        .slot_cfg =
            I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg =
            {
                .mclk = I2S_GPIO_UNUSED,
                .bclk = (gpio_num_t)SS_SPK_PIN_BCLK,
                .ws = (gpio_num_t)SS_SPK_PIN_WS,
                .dout = (gpio_num_t)SS_SPK_PIN_DOUT,
                .din = I2S_GPIO_UNUSED,
            },
    };

    err = i2s_channel_init_std_mode(s_tx, &std);
    if (err != ESP_OK) {
        i2s_del_channel(s_tx);
        s_tx = NULL;
        return err;
    }

    // Execute the pop-free OPEN sequence: clocks up, settle, amp on.
    ss_spk_step_t steps[3];
    const size_t n =
        ss_spk_core_open_seq(SS_SPK_HAS_MUTE_GPIO, steps, sizeof(steps) / sizeof(steps[0]));
    for (size_t i = 0; i < n; i++) {
        switch (steps[i]) {
        case SS_SPK_STEP_I2S_ENABLE:
            err = i2s_channel_enable(s_tx);
            if (err != ESP_OK) {
                i2s_del_channel(s_tx);
                s_tx = NULL;
                return err;
            }
            break;
        case SS_SPK_STEP_SETTLE:
            // Round the settle time up to whole ticks (5 ms -> 5 ms).
            vTaskDelay(pdMS_TO_TICKS((ss_spk_core_settle_us() + 999u) / 1000u));
            break;
        case SS_SPK_STEP_AMP_ENABLE:
            amp_set(true);
            break;
        case SS_SPK_STEP_AMP_DISABLE:
        case SS_SPK_STEP_I2S_DISABLE:
        default:
            break; // not part of the open sequence
        }
    }

    s_opened = true;
    s_muted = false;
    ESP_LOGI(TAG, "spk open: %uHz mono 16b, frame=%u desc=%u latency<=%uus",
             (unsigned)SS_SPK_SAMPLE_RATE_HZ, (unsigned)plan.frame_num, (unsigned)plan.desc_num,
             (unsigned)plan.max_latency_us);
    return ESP_OK;
#else
    // Board defines the speaker capability but no valid pins — treat as
    // unsupported rather than programming an invalid GPIO. (Not reachable on Lite.)
    return ESP_ERR_NOT_SUPPORTED;
#endif
}

esp_err_t ss_spk_write(const void* buf, size_t bytes)
{
    if (buf == NULL) { return ESP_ERR_INVALID_ARG; }
    if (!s_opened || s_tx == NULL) { return ESP_ERR_INVALID_STATE; }
    if (bytes == 0) { return ESP_OK; }
    // PCM is 16-bit; an odd byte count cannot be a whole sample stream.
    if ((bytes % 2u) != 0u) { return ESP_ERR_INVALID_ARG; }

    if (s_gain_q15 == 32767) {
        // Unity gain: stream the caller's buffer straight to the DMA.
        const uint8_t* p = (const uint8_t*)buf;
        size_t remaining = bytes;
        while (remaining > 0) {
            size_t wrote = 0;
            const esp_err_t err = i2s_channel_write(s_tx, p, remaining, &wrote, portMAX_DELAY);
            if (err != ESP_OK) { return err; }
            p += wrote;
            remaining -= wrote;
        }
        return ESP_OK;
    }

    // Attenuated: scale samples through a fixed stack chunk, writing as we go.
    const int16_t* src = (const int16_t*)buf;
    size_t samples = bytes / 2u;
    int16_t tmp[256];
    while (samples > 0) {
        size_t chunk = samples < 256u ? samples : 256u;
        for (size_t i = 0; i < chunk; i++) { tmp[i] = ss_spk_core_apply_gain(src[i], s_gain_q15); }
        const uint8_t* p = (const uint8_t*)tmp;
        size_t remaining = chunk * 2u;
        while (remaining > 0) {
            size_t wrote = 0;
            const esp_err_t err = i2s_channel_write(s_tx, p, remaining, &wrote, portMAX_DELAY);
            if (err != ESP_OK) { return err; }
            p += wrote;
            remaining -= wrote;
        }
        src += chunk;
        samples -= chunk;
    }
    return ESP_OK;
}

esp_err_t ss_spk_close(void)
{
    if (!s_opened && s_tx == NULL) {
        return ESP_OK; // idempotent
    }

    // Execute the pop-free CLOSE sequence: amp off, settle, clocks down.
    ss_spk_step_t steps[3];
    const size_t n =
        ss_spk_core_close_seq(SS_SPK_HAS_MUTE_GPIO, steps, sizeof(steps) / sizeof(steps[0]));
    for (size_t i = 0; i < n; i++) {
        switch (steps[i]) {
        case SS_SPK_STEP_AMP_DISABLE:
            amp_set(false);
            break;
        case SS_SPK_STEP_SETTLE:
            vTaskDelay(pdMS_TO_TICKS((ss_spk_core_settle_us() + 999u) / 1000u));
            break;
        case SS_SPK_STEP_I2S_DISABLE:
            if (s_tx != NULL) {
                i2s_channel_disable(s_tx);
                i2s_del_channel(s_tx);
                s_tx = NULL;
            }
            break;
        case SS_SPK_STEP_I2S_ENABLE:
        case SS_SPK_STEP_AMP_ENABLE:
        default:
            break; // not part of the close sequence
        }
    }

    s_opened = false;
    s_muted = false;
    return ESP_OK;
}

esp_err_t ss_spk_mute(bool on)
{
    if (!s_opened) { return ESP_ERR_INVALID_STATE; }
#if SS_SPK_HAS_MUTE_GPIO
    amp_set(!on); // amp enabled == not muted
    s_muted = on;
    return ESP_OK;
#else
    (void)on;
    return ESP_ERR_NOT_SUPPORTED;
#endif
}

esp_err_t ss_spk_volume(uint8_t percent_0_100)
{
    // Volume may be set while open or closed; it takes effect on the next write.
    s_gain_q15 = ss_spk_core_gain_q15(percent_0_100);
    return ESP_OK;
}
