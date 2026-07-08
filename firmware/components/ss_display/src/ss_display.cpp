// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_display.cpp — ESP-IDF glue implementing the frozen ss_hal_display.h
// contract for the Lite board (Elecrow CrowPanel Advance 3.5", ILI9488 over
// SPI3 @ 40 MHz). Contract per C-01 §3 (§3.1 Display).
//
// All pure geometry/format/timing lives in ss_display_core (host-tested); this
// file only turns those decisions into esp_lcd_panel_io transactions and reads
// the SS_LCD_* board macros — no geometry/pins/freq are hardcoded here.
//
// ILI9488 quirk (HW ref §3.1): in 4-wire SPI the controller accepts only
// 18-bit RGB666 pixel data (COLMOD 0x66) — 3 bytes/pixel on the wire. Upper
// layers (LVGL, ss_ui) stay in RGB565 per SS_LCD_FMT; conversion happens here.
//
// TEAR-FREE SYNC: a single DMA-capable conversion scratch is reused across the
// chunks of one flush. To avoid overwriting bytes still being clocked out, an
// on_color_trans_done callback gives a binary semaphore when each chunk's DMA
// completes; flush() blocks on that semaphore before refilling the scratch and
// returns only after the final chunk is on the wire (fully synchronous).

#include "ss_display.h"
#include "ss_display_core.h"

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "driver/ledc.h"
#include "driver/spi_master.h"
#include "esp_attr.h" // IRAM_ATTR
#include "esp_check.h"
#include "esp_heap_caps.h"
#include "esp_lcd_panel_io.h"
#include "esp_log.h"

#include "board_config.h"
#include "ss_hal.h" // ss_hal_has_cap()

static const char* TAG = "ss.disp";

// Max pixels sent per SPI transaction (bounded scratch for 565->666 convert).
static constexpr size_t kChunkPx = 4096;

static esp_lcd_panel_io_handle_t s_io = nullptr;
static uint8_t* s_chunk = nullptr;             // kChunkPx * 3 bytes, DMA-capable
static SemaphoreHandle_t s_dma_done = nullptr; // given by on_color_trans_done
static bool s_bl_ready = false;                // LEDC backlight configured

// Current logical geometry (native landscape by default; swapped on portrait
// orientations). ss_display_info() always reports the native panel resolution.
static uint16_t s_w = SS_LCD_W_PX;
static uint16_t s_h = SS_LCD_H_PX;

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
    {0x36, {0xE8}, 1, 0},                   // MADCTL: landscape, BGR (native ORIENT_90)
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

// DMA-completion callback (ISR context): unblock the flush waiting on this
// chunk so the shared scratch can be safely refilled.
static bool IRAM_ATTR on_color_trans_done(esp_lcd_panel_io_handle_t /*io*/,
                                          esp_lcd_panel_io_event_data_t* /*edata*/,
                                          void* /*user_ctx*/)
{
    BaseType_t hp_task_woken = pdFALSE;
    if (s_dma_done) { xSemaphoreGiveFromISR(s_dma_done, &hp_task_woken); }
    return hp_task_woken == pdTRUE;
}

static esp_err_t ili_set_window(int16_t x, int16_t y, int16_t w, int16_t h)
{
    const uint16_t x1 = (uint16_t)x, x2 = (uint16_t)(x + w - 1);
    const uint16_t y1 = (uint16_t)y, y2 = (uint16_t)(y + h - 1);
    const uint8_t ca[4] = {(uint8_t)(x1 >> 8), (uint8_t)x1, (uint8_t)(x2 >> 8), (uint8_t)x2};
    const uint8_t pa[4] = {(uint8_t)(y1 >> 8), (uint8_t)y1, (uint8_t)(y2 >> 8), (uint8_t)y2};
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(s_io, 0x2A, ca, 4), TAG, "caset");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(s_io, 0x2B, pa, 4), TAG, "paset");
    return ESP_OK;
}

// Blit a pre-clipped RGB565 rect: convert each chunk into the shared scratch,
// send it, then wait for the DMA-done semaphore before reusing the scratch.
static esp_err_t blit_rect(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t* px565)
{
    ESP_RETURN_ON_ERROR(ili_set_window(x, y, w, h), TAG, "window");

    size_t remaining = (size_t)w * (size_t)h;
    const uint16_t* src = px565;
    uint8_t cmd = 0x2C; // RAMWR, then RAMWRC (0x3C)
    while (remaining) {
        const size_t n = remaining < kChunkPx ? remaining : kChunkPx;
        for (size_t i = 0; i < n; ++i) { ss_display_core_px565_to_666(src[i], &s_chunk[i * 3]); }
        ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_color(s_io, cmd, s_chunk, n * 3), TAG, "tx color");
        // Tear-free: block until this chunk's DMA is done before refilling.
        xSemaphoreTake(s_dma_done, portMAX_DELAY);
        src += n;
        remaining -= n;
        cmd = 0x3C;
    }
    return ESP_OK;
}

// Fill the whole panel with one RGB565 colour (boot self-test helper).
static esp_err_t fill_solid(uint16_t rgb565)
{
    if (!s_io || !s_chunk) { return ESP_ERR_INVALID_STATE; }
    ESP_RETURN_ON_ERROR(ili_set_window(0, 0, (int16_t)s_w, (int16_t)s_h), TAG, "window");
    for (size_t i = 0; i < kChunkPx; ++i) { ss_display_core_px565_to_666(rgb565, &s_chunk[i * 3]); }

    size_t remaining = (size_t)s_w * (size_t)s_h;
    uint8_t cmd = 0x2C;
    while (remaining) {
        const size_t n = remaining < kChunkPx ? remaining : kChunkPx;
        ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_color(s_io, cmd, s_chunk, n * 3), TAG, "tx color");
        xSemaphoreTake(s_dma_done, portMAX_DELAY);
        remaining -= n;
        cmd = 0x3C;
    }
    return ESP_OK;
}

// ===========================================================================
// Frozen ss_hal_display.h contract. Since S-03-032 the HAL headers carry
// extern "C" guards, so these definitions use C linkage from this C++ TU.
// Callers are C++ (main.cpp, future ss_ui) and future C drivers alike.
// ===========================================================================

esp_err_t ss_display_init(void)
{
    if (!ss_hal_has_cap(SS_CAP_DISPLAY)) { return ESP_ERR_NOT_SUPPORTED; }

    ESP_LOGI(TAG, "ILI9488 bring-up: SPI%d SCLK=%d MOSI=%d DC=%d CS=%d BL=%d @%d Hz",
             (int)SS_LCD_SPI_HOST + 1, SS_LCD_PIN_SCLK, SS_LCD_PIN_MOSI, SS_LCD_PIN_DC,
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

    // 2b. DMA-done semaphore + completion callback (tear-free scratch reuse).
    if (!s_dma_done) { s_dma_done = xSemaphoreCreateBinary(); }
    if (!s_dma_done) { return ESP_ERR_NO_MEM; }
    esp_lcd_panel_io_callbacks_t cbs = {};
    cbs.on_color_trans_done = on_color_trans_done;
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_register_event_callbacks(s_io, &cbs, nullptr), TAG, "cbs");

    // 3. Conversion scratch (DMA-capable internal RAM).
    s_chunk =
        static_cast<uint8_t*>(heap_caps_malloc(kChunkPx * 3, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL));
    if (!s_chunk) { return ESP_ERR_NO_MEM; }

    // 4. Panel reset: driven by the STC8H1K28 (§3.14); software reset for
    //    determinism.
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(s_io, 0x01, nullptr, 0), TAG, "swreset");
    vTaskDelay(pdMS_TO_TICKS(150));

    // 5. Init sequence.
    for (const auto& c : kInitSeq) {
        ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(s_io, c.cmd, c.len ? c.data : nullptr, c.len),
                            TAG, "init cmd");
        if (c.delay_ms) { vTaskDelay(pdMS_TO_TICKS(c.delay_ms)); }
    }

    // Native geometry follows SS_LCD_NATIVE_ORIENT (landscape 480x320).
    s_w = SS_LCD_W_PX;
    s_h = SS_LCD_H_PX;

    // 6. Backlight PWM (LEDC), start dark; self-test / apps ramp it. Gated on
    //    the backlight-PWM capability.
    if (ss_hal_has_cap(SS_CAP_BACKLIGHT_PWM)) {
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
        s_bl_ready = true;
    }

    ESP_LOGI(TAG, "panel ready (%ux%u RGB565->RGB666)", SS_LCD_W_PX, SS_LCD_H_PX);
    return ESP_OK;
}

esp_err_t ss_display_info(ss_display_info_t* out)
{
    if (!out) { return ESP_ERR_INVALID_ARG; }
    memset(out, 0, sizeof(*out));
    out->w_px = SS_LCD_W_PX;
    out->h_px = SS_LCD_H_PX;
    out->dpi = 0; // not characterised for this panel
    out->shape = SS_SHAPE_RECT;
    out->native_orient = SS_LCD_NATIVE_ORIENT;
    out->fmt = SS_LCD_FMT;
    out->has_backlight_pwm = ss_hal_has_cap(SS_CAP_BACKLIGHT_PWM);
    return ESP_OK;
}

esp_err_t ss_display_flush(const ss_rect_t* r, const void* px)
{
    if (!s_io || !s_chunk) { return ESP_ERR_INVALID_STATE; }
    if (!r || !px) { return ESP_ERR_INVALID_ARG; }

    ss_rect_t clipped;
    if (!ss_display_core_clip(*r, s_w, s_h, &clipped)) {
        return ESP_OK; // fully off-screen: nothing to draw
    }
    // The source buffer is row-major for the *requested* rect, so any trimming
    // (left/top offsets every row; right/bottom shortens the tail) would
    // misalign it. Require the rect to be fully on-panel; reject otherwise.
    if (clipped.x != r->x || clipped.y != r->y || clipped.w != r->w || clipped.h != r->h) {
        return ESP_ERR_INVALID_ARG;
    }
    return blit_rect(clipped.x, clipped.y, clipped.w, clipped.h, static_cast<const uint16_t*>(px));
}

esp_err_t ss_display_backlight(uint8_t percent_0_100)
{
    if (!s_bl_ready) { return ESP_ERR_INVALID_STATE; }
    if (percent_0_100 > 100) { percent_0_100 = 100; }
    const uint32_t duty = (1023u * percent_0_100) / 100u;
    ledc_set_duty(LEDC_LOW_SPEED_MODE, ledc_channel_t(SS_LCD_BL_LEDC_CH), duty);
    return ledc_update_duty(LEDC_LOW_SPEED_MODE, ledc_channel_t(SS_LCD_BL_LEDC_CH));
}

esp_err_t ss_display_sleep(bool on)
{
    if (!s_io) { return ESP_ERR_INVALID_STATE; }
    if (on) {
        ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(s_io, 0x10, nullptr, 0), TAG, "sleep in");
        vTaskDelay(pdMS_TO_TICKS(120)); // datasheet: >5 ms; use safe 120 ms
    } else {
        ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(s_io, 0x11, nullptr, 0), TAG, "sleep out");
        vTaskDelay(pdMS_TO_TICKS(120)); // datasheet: >120 ms before next command
    }
    return ESP_OK;
}

esp_err_t ss_display_set_orient(ss_orient_t o)
{
    if (!s_io) { return ESP_ERR_INVALID_STATE; }
    if (o != SS_ORIENT_0 && o != SS_ORIENT_90 && o != SS_ORIENT_180 && o != SS_ORIENT_270) {
        return ESP_ERR_INVALID_ARG;
    }
    const uint8_t madctl = ss_display_core_madctl(o);
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(s_io, 0x36, &madctl, 1), TAG, "madctl");

    // Native is landscape 480x320 (ORIENT_90). Portrait orientations (0/180)
    // swap the logical width/height; landscape orientations (90/270) keep it.
    if (o == SS_ORIENT_0 || o == SS_ORIENT_180) {
        s_w = SS_LCD_H_PX;
        s_h = SS_LCD_W_PX;
    } else {
        s_w = SS_LCD_W_PX;
        s_h = SS_LCD_H_PX;
    }
    return ESP_OK;
}

esp_err_t ss_display_selftest(void)
{
    if (!s_io || !s_chunk) { return ESP_ERR_INVALID_STATE; }
    static const uint16_t bands[] = {0xF800 /*R*/, 0x07E0 /*G*/, 0x001F /*B*/};
    for (uint16_t c : bands) {
        ESP_RETURN_ON_ERROR(fill_solid(c), TAG, "fill");
        vTaskDelay(pdMS_TO_TICKS(150));
    }
    ESP_RETURN_ON_ERROR(fill_solid(0x0000), TAG, "clear");
    if (s_bl_ready) {
        for (uint8_t p = 0; p <= 100; p += 5) { // backlight ramp
            ss_display_backlight(p);
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
    return ESP_OK;
}
