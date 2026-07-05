// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_display_boot.cpp — ILI9488 first light over SPI3 @ 40 MHz.
//
// Verified pins (HW ref §3.1): SCLK 42, MOSI 39, DC 41, CS 40, BL 38.
// Panel reset is driven by the STC8H1K28 IO expander, NOT an ESP32 GPIO.
//
// ILI9488 quirk: in 4-wire SPI mode the controller only accepts 18-bit
// RGB666 pixel data (COLMOD 0x66) — 3 bytes/pixel on the wire. Upper layers
// (LVGL, ss_ui) stay in RGB565; this module converts during blit.

#include "ss_display_boot.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/ledc.h"
#include "esp_lcd_panel_io.h"
#include "esp_log.h"
#include "esp_check.h"
#include "esp_heap_caps.h"

#include "board_config.h"

static const char* TAG = "ss.disp";

static esp_lcd_panel_io_handle_t s_io = nullptr;

// Max pixels sent per SPI transaction (bounded scratch for 565→666 convert).
static constexpr size_t kChunkPx = 4096;
static uint8_t* s_chunk = nullptr; // kChunkPx * 3 bytes, DMA-capable

// ---------------------------------------------------------------------------
// ILI9488 init sequence (Elecrow factory FW / LovyanGFX consensus values)
// ---------------------------------------------------------------------------
struct ili_cmd_t {
    uint8_t cmd;
    uint8_t data[16];
    uint8_t len;
    uint16_t delay_ms;
};

static const ili_cmd_t kInitSeq[] = {
    {0xE0,
     {0x00, 0x03, 0x09, 0x08, 0x16, 0x0A, 0x3F, 0x78, 0x4C, 0x09, 0x0A, 0x08, 0x16, 0x1A, 0x0F},
     15,
     0},
    {0xE1,
     {0x00, 0x16, 0x19, 0x03, 0x0F, 0x05, 0x32, 0x45, 0x46, 0x04, 0x0E, 0x0D, 0x35, 0x37, 0x0F},
     15,
     0},
    {0xC0, {0x17, 0x15}, 2, 0},             // Power control 1
    {0xC1, {0x41}, 1, 0},                   // Power control 2
    {0xC5, {0x00, 0x12, 0x80}, 3, 0},       // VCOM control
    {0x36, {0xE8}, 1, 0},                   // MADCTL: landscape, BGR
    {0x3A, {0x66}, 1, 0},                   // COLMOD: 18-bit (SPI mode)
    {0xB0, {0x00}, 1, 0},                   // Interface mode
    {0xB1, {0xA0}, 1, 0},                   // Frame rate 60 Hz
    {0xB4, {0x02}, 1, 0},                   // 2-dot inversion
    {0xB6, {0x02, 0x02, 0x3B}, 3, 0},       // Display function
    {0xB7, {0xC6}, 1, 0},                   // Entry mode
    {0xF7, {0xA9, 0x51, 0x2C, 0x82}, 4, 0}, // Adjust control 3
    {0x11, {}, 0, 120},                     // Sleep out
    {0x29, {}, 0, 20},                      // Display on
};

static esp_err_t ili_set_window(int16_t x, int16_t y, int16_t w, int16_t h)
{
    const uint16_t x1 = x, x2 = x + w - 1, y1 = y, y2 = y + h - 1;
    const uint8_t ca[4] = {uint8_t(x1 >> 8), uint8_t(x1), uint8_t(x2 >> 8), uint8_t(x2)};
    const uint8_t pa[4] = {uint8_t(y1 >> 8), uint8_t(y1), uint8_t(y2 >> 8), uint8_t(y2)};
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(s_io, 0x2A, ca, 4), TAG, "caset");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(s_io, 0x2B, pa, 4), TAG, "paset");
    return ESP_OK;
}

// RGB565 → RGB666 wire format (3 bytes/px, 6 MSBs used per byte).
static inline void px565_to_666(uint16_t c, uint8_t* out)
{
    out[0] = uint8_t((c >> 8) & 0xF8); // R5 → high bits
    out[1] = uint8_t((c >> 3) & 0xFC); // G6
    out[2] = uint8_t((c << 3) & 0xF8); // B5
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------
esp_err_t ss_display_boot_init(void)
{
    ESP_LOGI(TAG, "ILI9488 bring-up: SPI%d SCLK=%d MOSI=%d DC=%d CS=%d BL=%d @%d Hz",
             int(SS_LCD_SPI_HOST) + 1, SS_LCD_PIN_SCLK, SS_LCD_PIN_MOSI, SS_LCD_PIN_DC,
             SS_LCD_PIN_CS, SS_LCD_PIN_BL, SS_LCD_SPI_FREQ_HZ);

    // 1. SPI bus (TX-only: panel has no MISO routed).
    spi_bus_config_t bus = {};
    bus.sclk_io_num = SS_LCD_PIN_SCLK;
    bus.mosi_io_num = SS_LCD_PIN_MOSI;
    bus.miso_io_num = -1;
    bus.quadwp_io_num = -1;
    bus.quadhd_io_num = -1;
    bus.max_transfer_sz = kChunkPx * 3;
    ESP_RETURN_ON_ERROR(spi_bus_initialize(SS_LCD_SPI_HOST, &bus, SPI_DMA_CH_AUTO), TAG, "spi bus");

    // 2. Panel IO (DC/CS handled by esp_lcd; command/param = 8 bit).
    esp_lcd_panel_io_spi_config_t io_cfg = {};
    io_cfg.dc_gpio_num = SS_LCD_PIN_DC;
    io_cfg.cs_gpio_num = SS_LCD_PIN_CS;
    io_cfg.pclk_hz = SS_LCD_SPI_FREQ_HZ;
    io_cfg.lcd_cmd_bits = 8;
    io_cfg.lcd_param_bits = 8;
    io_cfg.spi_mode = 0;
    io_cfg.trans_queue_depth = 10;
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_io_spi(SS_LCD_SPI_HOST, &io_cfg, &s_io), TAG, "panel io");

    // 3. Conversion scratch (DMA-capable internal RAM).
    s_chunk =
        static_cast<uint8_t*>(heap_caps_malloc(kChunkPx * 3, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL));
    if (!s_chunk) return ESP_ERR_NO_MEM;

    // 4. Panel reset: driven by the STC8H1K28 (§3.14) — power-on default is
    //    fine; software reset for determinism.
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(s_io, 0x01, nullptr, 0), TAG, "swreset");
    vTaskDelay(pdMS_TO_TICKS(150));

    // 5. Init sequence.
    for (const auto& c : kInitSeq) {
        ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(s_io, c.cmd, c.len ? c.data : nullptr, c.len),
                            TAG, "init cmd");
        if (c.delay_ms) vTaskDelay(pdMS_TO_TICKS(c.delay_ms));
    }

    // 6. Backlight PWM (LEDC, GPIO 38), start dark; test pattern ramps it.
    ledc_timer_config_t tcfg = {};
    tcfg.speed_mode = LEDC_LOW_SPEED_MODE;
    tcfg.timer_num = ledc_timer_t(SS_LCD_BL_LEDC_TIMER);
    tcfg.duty_resolution = LEDC_TIMER_10_BIT;
    tcfg.freq_hz = 20000; // inaudible
    tcfg.clk_cfg = LEDC_AUTO_CLK;
    ESP_RETURN_ON_ERROR(ledc_timer_config(&tcfg), TAG, "bl timer");

    ledc_channel_config_t ccfg = {};
    ccfg.gpio_num = SS_LCD_PIN_BL;
    ccfg.speed_mode = LEDC_LOW_SPEED_MODE;
    ccfg.channel = ledc_channel_t(SS_LCD_BL_LEDC_CH);
    ccfg.timer_sel = ledc_timer_t(SS_LCD_BL_LEDC_TIMER);
    ccfg.duty = 0;
    ESP_RETURN_ON_ERROR(ledc_channel_config(&ccfg), TAG, "bl chan");

    ESP_LOGI(TAG, "panel ready (%ux%u RGB565→RGB666)", SS_LCD_W_PX, SS_LCD_H_PX);
    return ESP_OK;
}

esp_err_t ss_display_boot_backlight(uint8_t percent)
{
    if (percent > 100) percent = 100;
    const uint32_t duty = (1023u * percent) / 100u;
    ledc_set_duty(LEDC_LOW_SPEED_MODE, ledc_channel_t(SS_LCD_BL_LEDC_CH), duty);
    return ledc_update_duty(LEDC_LOW_SPEED_MODE, ledc_channel_t(SS_LCD_BL_LEDC_CH));
}

esp_err_t ss_display_boot_blit(int16_t x, int16_t y, int16_t w, int16_t h,
                               const uint16_t* px_rgb565)
{
    if (!s_io || !s_chunk) return ESP_ERR_INVALID_STATE;
    ESP_RETURN_ON_ERROR(ili_set_window(x, y, w, h), TAG, "window");

    size_t remaining = size_t(w) * size_t(h);
    const uint16_t* src = px_rgb565;
    uint8_t cmd = 0x2C; // RAMWR, then RAMWRC
    while (remaining) {
        const size_t n = remaining < kChunkPx ? remaining : kChunkPx;
        for (size_t i = 0; i < n; ++i) px565_to_666(src[i], &s_chunk[i * 3]);
        ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_color(s_io, cmd, s_chunk, n * 3), TAG, "tx color");
        src += n;
        remaining -= n;
        cmd = 0x3C;
    }
    return ESP_OK;
}

esp_err_t ss_display_boot_fill(uint16_t rgb565)
{
    if (!s_io || !s_chunk) return ESP_ERR_INVALID_STATE;
    ESP_RETURN_ON_ERROR(ili_set_window(0, 0, SS_LCD_W_PX, SS_LCD_H_PX), TAG, "window");
    for (size_t i = 0; i < kChunkPx; ++i) px565_to_666(rgb565, &s_chunk[i * 3]);

    size_t remaining = size_t(SS_LCD_W_PX) * size_t(SS_LCD_H_PX);
    uint8_t cmd = 0x2C;
    while (remaining) {
        const size_t n = remaining < kChunkPx ? remaining : kChunkPx;
        ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_color(s_io, cmd, s_chunk, n * 3), TAG, "tx color");
        remaining -= n;
        cmd = 0x3C;
    }
    return ESP_OK;
}

esp_err_t ss_display_boot_test_pattern(void)
{
    static const uint16_t bands[] = {0xF800 /*R*/, 0x07E0 /*G*/, 0x001F /*B*/};
    for (uint16_t c : bands) {
        ESP_RETURN_ON_ERROR(ss_display_boot_fill(c), TAG, "fill");
        vTaskDelay(pdMS_TO_TICKS(150));
    }
    ESP_RETURN_ON_ERROR(ss_display_boot_fill(0x0000), TAG, "clear");
    for (uint8_t p = 0; p <= 100; p += 5) { // backlight ramp
        ss_display_boot_backlight(p);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    return ESP_OK;
}
