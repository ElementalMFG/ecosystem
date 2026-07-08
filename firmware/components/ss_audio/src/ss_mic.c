// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_mic.c — ESP-IDF glue implementing the mic half of the frozen
// ss_hal_audio.h contract (ss_mic_open / ss_mic_read / ss_mic_close). All
// format validation and DMA sizing live in the host-testable ss_audio_core;
// this file turns those decisions into I2S std-driver calls and mux ownership.
//
// DEGRADATION: the microphone is a capability, not a given. On boards whose
// caps mask lacks SS_CAP_MIC (Alpha/Omega today) ss_mic_open returns
// ESP_ERR_NOT_SUPPORTED before touching any pin, and read/close are safe. App
// code must gate on ss_hal_has_cap(SS_CAP_MIC).
//
// MUX: on Lite the mic shares GPIO45 with the radio path (SS_CAP_MUX_MIC_RADIO).
// ss_mic_open acquires SS_MUX_MODE_MIC as SS_MUX_OWNER_AUDIO_MIC (25 ms cap,
// inside the <25 ms PTT budget from S-03-004) and ss_mic_close releases it. On
// boards without the mux ss_mux_acquire/release are OK no-ops.
//
// INMP441 24-in-32 -> 16-bit PCM: the INMP441 shifts out a 24-bit sample MSB
// first, left-justified in a 32-bit BCLK slot. We set slot_bit_width to 32 so
// the mic gets its full 32 BCLKs per WS, but data_bit_width to 16 so the I2S
// peripheral captures only the top 16 bits of each slot — i.e. the 16 most
// significant bits of the 24-bit sample — and stores them straight to the DMA
// buffer as int16 PCM. ss_mic_read therefore yields ready-to-use 16-bit mono
// PCM with no post-processing.

#include "ss_hal.h" // ss_hal_has_cap()
#include "ss_hal_audio.h"
#include "ss_hal_caps.h"
#include "ss_hal_muxctl.h"
#include "ss_audio_core.h"

#include "board_config.h" // canonical Lite pin map (via ss_hal include path)

#include "driver/i2s_std.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "ss_mic";

// Mux acquire budget: kept inside the <25 ms PTT latency budget (S-03-004).
#define SS_MIC_MUX_TIMEOUT_MS 25u
// DMA buffering target: ~10 ms bounds per-read jitter well under the PTT budget.
#define SS_MIC_TARGET_LATENCY_MS 10u

static i2s_chan_handle_t s_rx; // RX channel handle, NULL when closed
static bool s_opened;          // true between a successful open and close

// Map a core validation result onto the frozen esp_err_t contract. NULL args are
// invalid arguments; an absent mic capability or unsupported format parameter is
// "not supported" for this board.
static esp_err_t map_validate(ss_audio_core_result_t r)
{
    switch (r) {
    case SS_AUDIO_OK:
        return ESP_OK;
    case SS_AUDIO_ERR_NULL:
        return ESP_ERR_INVALID_ARG;
    case SS_AUDIO_ERR_NO_MIC_CAP:
    case SS_AUDIO_ERR_RATE:
    case SS_AUDIO_ERR_CHANNELS:
    case SS_AUDIO_ERR_BITS:
    default:
        return ESP_ERR_NOT_SUPPORTED;
    }
}

esp_err_t ss_mic_open(const ss_audio_fmt_t* fmt)
{
    if (!ss_hal_has_cap(SS_CAP_MIC)) {
        ESP_LOGD(TAG, "no microphone on this board");
        return ESP_ERR_NOT_SUPPORTED;
    }
    if (s_opened) { return ESP_ERR_INVALID_STATE; }

    const ss_audio_core_result_t vr = ss_audio_core_validate_mic_fmt(fmt, true);
    if (vr != SS_AUDIO_OK) { return map_validate(vr); }

#if SS_MIC_PIN_BCLK >= 0
    // Acquire the mic path first; on shared-mux boards this blocks the radio out.
    esp_err_t err =
        ss_mux_acquire(ss_audio_core_mic_mux_mode(), pdMS_TO_TICKS(SS_MIC_MUX_TIMEOUT_MS),
                       ss_audio_core_mic_mux_owner());
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "mux acquire failed: %s", esp_err_to_name(err));
        return err; // nothing acquired to unwind
    }

    ss_mic_dma_plan_t plan;
    const ss_audio_core_result_t pr = ss_audio_core_plan_dma(fmt, SS_MIC_TARGET_LATENCY_MS, &plan);
    if (pr != SS_AUDIO_OK) {
        ss_mux_release(ss_audio_core_mic_mux_owner());
        return map_validate(pr);
    }

    i2s_chan_config_t chan_cfg =
        I2S_CHANNEL_DEFAULT_CONFIG((i2s_port_t)SS_MIC_I2S_PORT, I2S_ROLE_MASTER);
    chan_cfg.dma_desc_num = plan.desc_num;
    chan_cfg.dma_frame_num = plan.frame_num;
    err = i2s_new_channel(&chan_cfg, NULL, &s_rx);
    if (err != ESP_OK) {
        ss_mux_release(ss_audio_core_mic_mux_owner());
        return err;
    }

    i2s_std_config_t std = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(SS_MIC_SAMPLE_RATE_HZ),
        .slot_cfg =
            I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg =
            {
                .mclk = I2S_GPIO_UNUSED,
                .bclk = (gpio_num_t)SS_MIC_PIN_BCLK,
                .ws = (gpio_num_t)SS_MIC_PIN_WS,
                .dout = I2S_GPIO_UNUSED,
                .din = (gpio_num_t)SS_MIC_PIN_DIN,
            },
    };
    // 32 BCLK/slot so the INMP441 shifts its full 24-bit word; capturing 16 data
    // bits keeps the 16 MSBs as int16 PCM (see file header).
    std.slot_cfg.slot_bit_width = I2S_SLOT_BIT_WIDTH_32BIT;

    err = i2s_channel_init_std_mode(s_rx, &std);
    if (err != ESP_OK) {
        i2s_del_channel(s_rx);
        s_rx = NULL;
        ss_mux_release(ss_audio_core_mic_mux_owner());
        return err;
    }

    err = i2s_channel_enable(s_rx);
    if (err != ESP_OK) {
        i2s_del_channel(s_rx);
        s_rx = NULL;
        ss_mux_release(ss_audio_core_mic_mux_owner());
        return err;
    }

    s_opened = true;
    ESP_LOGI(TAG, "mic open: %uHz mono 16b, frame=%u desc=%u jitter<=%uus",
             (unsigned)SS_MIC_SAMPLE_RATE_HZ, (unsigned)plan.frame_num, (unsigned)plan.desc_num,
             (unsigned)plan.max_jitter_us);
    return ESP_OK;
#else
    // Board defines the mic capability but no valid pins — treat as unsupported
    // rather than programming an invalid GPIO. (Not reachable on Lite.)
    return ESP_ERR_NOT_SUPPORTED;
#endif
}

esp_err_t ss_mic_read(void* buf, size_t bytes, size_t* out_bytes, uint32_t timeout_ms)
{
    if (out_bytes != NULL) { *out_bytes = 0; }
    if (buf == NULL || out_bytes == NULL) { return ESP_ERR_INVALID_ARG; }
    if (!s_opened || s_rx == NULL) { return ESP_ERR_INVALID_STATE; }

    size_t got = 0;
    const esp_err_t err = i2s_channel_read(s_rx, buf, bytes, &got, pdMS_TO_TICKS(timeout_ms));
    *out_bytes = got;
    return err; // ESP_ERR_TIMEOUT propagates unchanged
}

esp_err_t ss_mic_close(void)
{
    if (!s_opened && s_rx == NULL) {
        return ESP_OK; // idempotent
    }
    if (s_rx != NULL) {
        i2s_channel_disable(s_rx);
        i2s_del_channel(s_rx);
        s_rx = NULL;
    }
    ss_mux_release(ss_audio_core_mic_mux_owner());
    s_opened = false;
    return ESP_OK;
}
