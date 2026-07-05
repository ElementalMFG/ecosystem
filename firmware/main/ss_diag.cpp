// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_diag.cpp — audible diagnostics + power watchdog (directive goal D).

#include "ss_diag.h"

#include <math.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "driver/i2s_std.h"
#include "esp_log.h"
#include "esp_check.h"
#include "esp_heap_caps.h"
#include "esp_system.h"

#include "board_config.h"

static const char* TAG = "ss.diag";

// ---------------------------------------------------------------------------
// Buzzer — LEDC ch1/timer1 on GPIO 8. Frequency = note, duty 50% = on.
// ---------------------------------------------------------------------------
static esp_err_t buzzer_init(void)
{
    ledc_timer_config_t tcfg = {};
    tcfg.speed_mode = LEDC_LOW_SPEED_MODE;
    tcfg.timer_num = ledc_timer_t(SS_BUZZER_LEDC_TIMER);
    tcfg.duty_resolution = LEDC_TIMER_10_BIT;
    tcfg.freq_hz = 2000; // placeholder, set per note
    tcfg.clk_cfg = LEDC_AUTO_CLK;
    ESP_RETURN_ON_ERROR(ledc_timer_config(&tcfg), TAG, "bz timer");

    ledc_channel_config_t ccfg = {};
    ccfg.gpio_num = SS_BUZZER_PIN;
    ccfg.speed_mode = LEDC_LOW_SPEED_MODE;
    ccfg.channel = ledc_channel_t(SS_BUZZER_LEDC_CH);
    ccfg.timer_sel = ledc_timer_t(SS_BUZZER_LEDC_TIMER);
    ccfg.duty = 0;
    return ledc_channel_config(&ccfg);
}

static void note(uint16_t freq_hz, uint16_t on_ms, uint16_t gap_ms)
{
    if (freq_hz) {
        ledc_set_freq(LEDC_LOW_SPEED_MODE, ledc_timer_t(SS_BUZZER_LEDC_TIMER), freq_hz);
        ledc_set_duty(LEDC_LOW_SPEED_MODE, ledc_channel_t(SS_BUZZER_LEDC_CH), 512);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, ledc_channel_t(SS_BUZZER_LEDC_CH));
    }
    vTaskDelay(pdMS_TO_TICKS(on_ms));
    ledc_set_duty(LEDC_LOW_SPEED_MODE, ledc_channel_t(SS_BUZZER_LEDC_CH), 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, ledc_channel_t(SS_BUZZER_LEDC_CH));
    if (gap_ms) vTaskDelay(pdMS_TO_TICKS(gap_ms));
}

esp_err_t ss_diag_beep(ss_diag_tone_t tone)
{
    switch (tone) {
    case SS_DIAG_TONE_BOOT:
        note(1319, 90, 30);
        note(1760, 140, 0);
        break;
    case SS_DIAG_TONE_ACK:
        note(1568, 60, 0);
        break;
    case SS_DIAG_TONE_ERROR:
        note(220, 120, 60);
        note(220, 120, 0);
        break;
    case SS_DIAG_TONE_INCOMING:
        note(2093, 50, 40);
        note(2093, 50, 40);
        note(2637, 80, 0);
        break;
    case SS_DIAG_TONE_SOS: // ... --- ...  (dit 80 ms, dah 240 ms)
        for (int i = 0; i < 3; ++i) note(1000, 80, 80);
        vTaskDelay(pdMS_TO_TICKS(160));
        for (int i = 0; i < 3; ++i) note(1000, 240, 80);
        vTaskDelay(pdMS_TO_TICKS(160));
        for (int i = 0; i < 3; ++i) note(1000, 80, 80);
        break;
    default:
        return ESP_ERR_INVALID_ARG;
    }
    return ESP_OK;
}

// ---------------------------------------------------------------------------
// Speaker test tone — I2S1 std mode, 1 kHz sine, 16-bit mono @16 kHz.
// Channel is created, played, torn down: scaffolding only. hal_audio.c
// (EPIC-03) owns the persistent channel + mute/volume policy.
// ---------------------------------------------------------------------------
esp_err_t ss_diag_speaker_test(uint32_t ms)
{
    i2s_chan_handle_t tx = nullptr;
    i2s_chan_config_t chan =
        I2S_CHANNEL_DEFAULT_CONFIG(i2s_port_t(SS_SPK_I2S_PORT), I2S_ROLE_MASTER);
    ESP_RETURN_ON_ERROR(i2s_new_channel(&chan, &tx, nullptr), TAG, "i2s chan");

    i2s_std_config_t std = {};
    std.clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(SS_SPK_SAMPLE_RATE_HZ);
    std.slot_cfg =
        I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO);
    std.gpio_cfg.mclk = I2S_GPIO_UNUSED;
    std.gpio_cfg.bclk = gpio_num_t(SS_SPK_PIN_BCLK);
    std.gpio_cfg.ws = gpio_num_t(SS_SPK_PIN_WS);
    std.gpio_cfg.dout = gpio_num_t(SS_SPK_PIN_DOUT);
    std.gpio_cfg.din = I2S_GPIO_UNUSED;
    esp_err_t err = i2s_channel_init_std_mode(tx, &std);
    if (err != ESP_OK) {
        i2s_del_channel(tx);
        return err;
    }

#if SS_SPK_HAS_MUTE_GPIO
    gpio_set_direction(gpio_num_t(SS_SPK_PIN_MUTE), GPIO_MODE_OUTPUT);
    gpio_set_level(gpio_num_t(SS_SPK_PIN_MUTE), 0); // unmute
#endif

    // One 16 ms cycle-aligned buffer of 1 kHz sine (16 full periods @16 kHz).
    constexpr size_t kSamples = 256;
    int16_t* buf = static_cast<int16_t*>(
        heap_caps_malloc(kSamples * sizeof(int16_t), MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL));
    if (!buf) {
        i2s_del_channel(tx);
        return ESP_ERR_NO_MEM;
    }
    for (size_t i = 0; i < kSamples; ++i)
        buf[i] = int16_t(
            8000.0f * sinf(2.0f * float(M_PI) * 1000.0f * float(i) / float(SS_SPK_SAMPLE_RATE_HZ)));

    err = i2s_channel_enable(tx);
    const TickType_t until = xTaskGetTickCount() + pdMS_TO_TICKS(ms);
    size_t written;
    while (err == ESP_OK && xTaskGetTickCount() < until)
        err = i2s_channel_write(tx, buf, kSamples * sizeof(int16_t), &written, 100);

    i2s_channel_disable(tx);
#if SS_SPK_HAS_MUTE_GPIO
    gpio_set_level(gpio_num_t(SS_SPK_PIN_MUTE), 1); // mute again
#endif
    heap_caps_free(buf);
    i2s_del_channel(tx);
    return err;
}

// ---------------------------------------------------------------------------
// Power watchdog — directive amendment (HW ref §12): no battery-sense ADC
// exists on the Lite, so a raw voltage read is impossible on stock hardware.
// This task logs the constraint once and watches heap as a proxy health
// metric; the real policy (fuel-gauge module / charger hints / graceful
// power-down via ss_power_enter) lands with hal_power.c in EPIC-03.
// ---------------------------------------------------------------------------
static void power_watchdog_task(void*)
{
#if SS_BATTERY_SENSE_PRESENT
    ESP_LOGI(TAG, "power watchdog: battery ADC present");
#else
    ESP_LOGW(TAG, "power watchdog: NO battery-sense line on this board "
                  "(Meshtastic #7993) — voltage policy deferred to fuel-gauge "
                  "module / hal_power (HW ref §12 amendment)");
#endif
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(30000));
        ESP_LOGD(TAG, "health: heap=%u min=%u", unsigned(esp_get_free_heap_size()),
                 unsigned(esp_get_minimum_free_heap_size()));
    }
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------
esp_err_t ss_diag_start(void)
{
    ESP_RETURN_ON_ERROR(buzzer_init(), TAG, "buzzer");
    const BaseType_t ok = xTaskCreatePinnedToCore(power_watchdog_task, "ss_pwr_wd", 2560, nullptr,
                                                  5, nullptr, tskNO_AFFINITY);
    ESP_LOGI(TAG, "diagnostics ready (buzzer GPIO%d, speaker I2S%d)", SS_BUZZER_PIN,
             int(SS_SPK_I2S_PORT));
    return ok == pdPASS ? ESP_OK : ESP_ERR_NO_MEM;
}
