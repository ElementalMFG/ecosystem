// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_touch.c — ESP-IDF glue implementing the frozen ss_hal_touch.h contract
// for the Lite board's GT911 capacitive controller. All gesture decision logic
// lives in the host-testable ss_gesture core; this file only does I2C I/O,
// the GT911 reset sequence, INT-driven sampling, and callback dispatch.
//
// DEGRADATION: touch is a capability, not a given. On boards whose caps mask
// lacks SS_CAP_TOUCH (Alpha/Omega today) ss_touch_init returns
// ESP_ERR_NOT_SUPPORTED; poll_once then returns ESP_ERR_NOT_FOUND and
// register/sleep are safe no-ops. App code must gate on ss_hal_has_cap.
//
// PTT LATENCY BUDGET (on-hardware AC, measured <25 ms; NOT faked here):
// the design is INT-driven, not polled, precisely to keep the
// touch-edge -> callback path well inside the 25 ms PTT budget:
//     * GT911 INT (GPIO47, falling edge) -> ISR task-notify   : ~O(10 us)
//       (ISR latency + FreeRTOS notify wake of the reader task)
//     * one I2C point read, ~14 bytes @ 400 kHz                : ~0.5 ms
//       (status reg + 8-byte point block, incl. addr phases)
//     * build event + registered-callback dispatch            : ~O(us)
// Total budget consumed ~<1 ms of the 25 ms; the raw SS_INPUT_TOUCH_DOWN is
// emitted BEFORE the gesture core runs so the PTT edge is never delayed by
// gesture classification. This file does NOT hardcode PTT regions — a UI PTT
// region maps a TOUCH_DOWN to PTT in EPIC-15.

#include "ss_hal_buttons.h" // (shared ss_input_event_t consumers)
#include "ss_hal_touch.h"
#include "ss_hal.h"
#include "ss_input_core.h"

#include <string.h>

#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "ss_touch";

// GT911 register map (subset).
#define GT911_REG_COMMAND     0x8040u
#define GT911_REG_STATUS      0x814Eu
#define GT911_REG_POINT0      0x8150u
#define GT911_CMD_SLEEP       0x05u
#define GT911_STATUS_READY    0x80u
#define GT911_POINT_MASK      0x0Fu
#define GT911_I2C_TIMEOUT_MS  20

static i2c_master_bus_handle_t s_bus;
static i2c_master_dev_handle_t s_dev;
static TaskHandle_t s_reader;
static ss_touch_cb_t s_cb;
static void* s_user;
static ss_gesture_t s_gesture;
static bool s_ready; // init succeeded (cap present, hardware up)

// --- low-level GT911 I2C ---------------------------------------------------

static esp_err_t gt911_read(uint16_t reg, uint8_t* buf, size_t len)
{
    const uint8_t addr[2] = {(uint8_t)(reg >> 8), (uint8_t)(reg & 0xFFu)};
    return i2c_master_transmit_receive(s_dev, addr, sizeof(addr), buf, len, GT911_I2C_TIMEOUT_MS);
}

static esp_err_t gt911_write_u8(uint16_t reg, uint8_t val)
{
    const uint8_t frame[3] = {(uint8_t)(reg >> 8), (uint8_t)(reg & 0xFFu), val};
    return i2c_master_transmit(s_dev, frame, sizeof(frame), GT911_I2C_TIMEOUT_MS);
}

// --- reset sequence --------------------------------------------------------
// Drives RST/INT to bring the GT911 up at I2C address 0x5D (INT low during the
// rising edge of RST selects the 0x5D/0xBA slave address).
static void gt911_reset_sequence(void)
{
    const gpio_num_t rst = (gpio_num_t)SS_TOUCH_PIN_RST;
    const gpio_num_t intp = (gpio_num_t)SS_TOUCH_PIN_INT;
    gpio_config_t io = {
        .pin_bit_mask = (1ULL << SS_TOUCH_PIN_RST) | (1ULL << SS_TOUCH_PIN_INT),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io);

    gpio_set_level(rst, 0);
    gpio_set_level(intp, 0);
    vTaskDelay(pdMS_TO_TICKS(11)); // >10 ms reset hold
    gpio_set_level(intp, 0);       // address select = 0x5D
    esp_rom_delay_us(120);
    gpio_set_level(rst, 1);
    vTaskDelay(pdMS_TO_TICKS(6));
    gpio_set_level(intp, 0);
    vTaskDelay(pdMS_TO_TICKS(51)); // controller boot
}

// --- INT ISR ---------------------------------------------------------------
static void IRAM_ATTR touch_isr(void* arg)
{
    (void)arg;
    BaseType_t hpw = pdFALSE;
    if (s_reader != NULL) { vTaskNotifyGiveFromISR(s_reader, &hpw); }
    if (hpw == pdTRUE) { portYIELD_FROM_ISR(); }
}

static void dispatch(ss_input_kind_t kind, int16_t x, int16_t y, ss_mono_us_t at, int16_t delta)
{
    if (s_cb == NULL) { return; }
    ss_input_event_t ev;
    memset(&ev, 0, sizeof(ev));
    ev.kind = kind;
    ev.at = at;
    ev.x = x;
    ev.y = y;
    ev.delta = delta;
    s_cb(&ev, s_user);
}

// Read one GT911 point. Returns true and fills *x/*y iff a finger is present;
// always clears the status buffer-ready flag so the next INT can fire.
static bool read_point(int16_t* x, int16_t* y)
{
    uint8_t status = 0;
    if (gt911_read(GT911_REG_STATUS, &status, 1) != ESP_OK) { return false; }
    if ((status & GT911_STATUS_READY) == 0) { return false; }

    const uint8_t count = status & GT911_POINT_MASK;
    bool have = false;
    if (count > 0) {
        uint8_t p[8];
        if (gt911_read(GT911_REG_POINT0, p, sizeof(p)) == ESP_OK) {
            *x = (int16_t)((uint16_t)p[1] | ((uint16_t)p[2] << 8));
            *y = (int16_t)((uint16_t)p[3] | ((uint16_t)p[4] << 8));
            have = true;
        }
    }
    (void)gt911_write_u8(GT911_REG_STATUS, 0); // clear ready flag
    return have;
}

// Reader task: wakes on the INT notify, emits the low-latency raw TOUCH_DOWN,
// then tracks the contact (polling briefly for MOVE/UP) and feeds the gesture
// core for derived TAP/LONG_PRESS/SWIPE events.
static void reader_task(void* arg)
{
    (void)arg;
    bool down = false;
    for (;;) {
        // While a finger is down, poll at ~10 ms so MOVE/UP and long-press are
        // observed; when idle, block until the next INT.
        const TickType_t wait = down ? pdMS_TO_TICKS(10) : portMAX_DELAY;
        (void)ulTaskNotifyTake(pdTRUE, wait);

        int16_t x = 0, y = 0;
        const bool present = read_point(&x, &y);
        const ss_mono_us_t now = (ss_mono_us_t)esp_timer_get_time();
        ss_input_event_t out;

        if (present && !down) {
            down = true;
            dispatch(SS_INPUT_TOUCH_DOWN, x, y, now, 0); // low-latency PTT path
            (void)ss_gesture_step(&s_gesture, SS_TOUCH_SAMPLE_DOWN, x, y, now, &out);
        } else if (present && down) {
            dispatch(SS_INPUT_TOUCH_MOVE, x, y, now, 0);
            if (ss_gesture_step(&s_gesture, SS_TOUCH_SAMPLE_MOVE, x, y, now, &out)) {
                if (s_cb != NULL) { s_cb(&out, s_user); }
            }
        } else if (!present && down) {
            down = false;
            dispatch(SS_INPUT_TOUCH_UP, x, y, now, 0);
            if (ss_gesture_step(&s_gesture, SS_TOUCH_SAMPLE_UP, x, y, now, &out)) {
                if (s_cb != NULL) { s_cb(&out, s_user); }
            }
        }
    }
}

// --- contract implementation ----------------------------------------------

esp_err_t ss_touch_init(void)
{
    if (!ss_hal_has_cap(SS_CAP_TOUCH)) {
        ESP_LOGI(TAG, "no SS_CAP_TOUCH on this board: touch disabled (degraded)");
        return ESP_ERR_NOT_SUPPORTED;
    }

    ss_gesture_init(&s_gesture);
    gt911_reset_sequence();

    const i2c_master_bus_config_t bus_cfg = {
        .i2c_port = SS_TOUCH_I2C_PORT,
        .sda_io_num = (gpio_num_t)SS_TOUCH_PIN_SDA,
        .scl_io_num = (gpio_num_t)SS_TOUCH_PIN_SCL,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags = {.enable_internal_pullup = true},
    };
    esp_err_t err = i2c_new_master_bus(&bus_cfg, &s_bus);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "i2c bus init failed: 0x%x", err);
        return err;
    }
    const i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = SS_TOUCH_I2C_ADDR,
        .scl_speed_hz = SS_TOUCH_I2C_FREQ_HZ,
    };
    err = i2c_master_bus_add_device(s_bus, &dev_cfg, &s_dev);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "i2c device add failed: 0x%x", err);
        return err;
    }

    if (xTaskCreate(reader_task, "ss_touch", 4096, NULL, 10, &s_reader) != pdPASS) {
        ESP_LOGE(TAG, "reader task create failed");
        return ESP_ERR_NO_MEM;
    }

    // INT (GPIO47) as input, falling-edge interrupt -> touch_isr.
    const gpio_config_t int_cfg = {
        .pin_bit_mask = 1ULL << SS_TOUCH_PIN_INT,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE,
    };
    gpio_config(&int_cfg);
    err = gpio_install_isr_service(0);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) { // already installed is fine
        ESP_LOGE(TAG, "isr service install failed: 0x%x", err);
        return err;
    }
    err = gpio_isr_handler_add((gpio_num_t)SS_TOUCH_PIN_INT, touch_isr, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "isr handler add failed: 0x%x", err);
        return err;
    }

    s_ready = true;
    ESP_LOGI(TAG, "GT911 touch up (INT-driven, addr 0x%02x @ %d Hz)", SS_TOUCH_I2C_ADDR,
             SS_TOUCH_I2C_FREQ_HZ);
    return ESP_OK;
}

esp_err_t ss_touch_register(ss_touch_cb_t cb, void* user)
{
    s_cb = cb;
    s_user = user;
    return ESP_OK; // safe no-op semantics when touch is absent
}

esp_err_t ss_touch_poll_once(ss_input_event_t* out)
{
    if (out == NULL) { return ESP_ERR_INVALID_ARG; }
    if (!s_ready) { return ESP_ERR_NOT_FOUND; }
    int16_t x = 0, y = 0;
    if (!read_point(&x, &y)) { return ESP_ERR_NOT_FOUND; }
    memset(out, 0, sizeof(*out));
    out->kind = SS_INPUT_TOUCH_DOWN;
    out->at = (ss_mono_us_t)esp_timer_get_time();
    out->x = x;
    out->y = y;
    return ESP_OK;
}

esp_err_t ss_touch_sleep(bool on)
{
    if (!s_ready) { return ESP_OK; } // no-op when touch is absent
    if (on) {
        return gt911_write_u8(GT911_REG_COMMAND, GT911_CMD_SLEEP);
    }
    // Wake: pulse INT high briefly, then re-arm as input (GT911 wake protocol).
    gpio_set_direction((gpio_num_t)SS_TOUCH_PIN_INT, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)SS_TOUCH_PIN_INT, 1);
    esp_rom_delay_us(5);
    gpio_set_direction((gpio_num_t)SS_TOUCH_PIN_INT, GPIO_MODE_INPUT);
    return ESP_OK;
}
