// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_compass.cpp — tilt-compensated compass thread (directive goal C).
//
// Standard tilt-compensation:
//   pitch = atan2(-ax, sqrt(ay² + az²))
//   roll  = atan2( ay, az)
//   Xh    = mx·cos(pitch) + mz·sin(pitch)
//   Yh    = mx·sin(roll)·sin(pitch) + my·cos(roll) − mz·sin(roll)·cos(pitch)
//   heading = atan2(−Yh, Xh)  → 0..360°, + declination
//
// The IMU is register-compatible with the MPU-6050 class (WHO_AM_I 0x75,
// PWR_MGMT_1 0x6B, ACCEL_XOUT_H 0x3B); the real HAL driver (EPIC-03,
// hal_imu.c) will replace the raw register access here.

#include "ss_compass.h"

#include <math.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "esp_check.h"

#include "board_config.h"
#include "ss_tasks.h"

static const char* TAG = "ss.compass";

static ss_compass_reading_t s_reading;
static bool s_ever = false;
static float s_decl_deg = 0.0f;
static SemaphoreHandle_t s_lock;

static i2c_master_bus_handle_t s_bus = nullptr;
static i2c_master_dev_handle_t s_mag = nullptr; // HMC5883L @0x1E
static i2c_master_dev_handle_t s_imu = nullptr; // MPU-6050 class @0x68

// ---------------------------------------------------------------------------
// I2C helpers (new i2c_master API; 50 ms timeout keeps the task non-blocking
// enough — the shared-bus mutex is held only per transaction)
// ---------------------------------------------------------------------------
static esp_err_t reg_write(i2c_master_dev_handle_t dev, uint8_t reg, uint8_t val)
{
    const uint8_t buf[2] = {reg, val};
    return i2c_master_transmit(dev, buf, 2, 50);
}

static esp_err_t reg_read(i2c_master_dev_handle_t dev, uint8_t reg, uint8_t* out, size_t n)
{
    return i2c_master_transmit_receive(dev, &reg, 1, out, n, 50);
}

// ---------------------------------------------------------------------------
// Sensor bring-up
// ---------------------------------------------------------------------------
static esp_err_t bus_init(void)
{
    i2c_master_bus_config_t cfg = {};
    cfg.i2c_port = SS_MAG_I2C_PORT;                // I2C0, shared with GT911
    cfg.sda_io_num = gpio_num_t(SS_TOUCH_PIN_SDA); // 15
    cfg.scl_io_num = gpio_num_t(SS_TOUCH_PIN_SCL); // 16
    cfg.clk_source = I2C_CLK_SRC_DEFAULT;
    cfg.glitch_ignore_cnt = 7;
    cfg.flags.enable_internal_pullup = true; // board has external pullups too
    return i2c_new_master_bus(&cfg, &s_bus);
}

#if CONFIG_SS_LITE_MOD_GNSS_BN880
static bool mag_init(void)
{
    if (i2c_master_probe(s_bus, SS_MAG_I2C_ADDR, 50) != ESP_OK) return false;

    i2c_device_config_t dc = {};
    dc.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    dc.device_address = SS_MAG_I2C_ADDR;
    dc.scl_speed_hz = SS_TOUCH_I2C_FREQ_HZ;
    if (i2c_master_bus_add_device(s_bus, &dc, &s_mag) != ESP_OK) return false;

    // CRA: 8-sample avg, 15 Hz ODR, normal measurement (0x70)
    // CRB: gain ±1.3 Ga (0x20) — Earth field is ~0.25–0.65 Ga
    // MODE: continuous (0x00)
    bool ok = reg_write(s_mag, 0x00, 0x70) == ESP_OK && reg_write(s_mag, 0x01, 0x20) == ESP_OK &&
              reg_write(s_mag, 0x02, 0x00) == ESP_OK;
    if (!ok) {
        i2c_master_bus_rm_device(s_mag);
        s_mag = nullptr;
    }
    return ok;
}

// HMC5883L data registers 0x03..0x08 are X, Z, Y (big-endian, in that order!)
static bool mag_read(float* mx, float* my, float* mz)
{
    uint8_t d[6];
    if (reg_read(s_mag, 0x03, d, 6) != ESP_OK) return false;
    const int16_t x = int16_t((d[0] << 8) | d[1]);
    const int16_t z = int16_t((d[2] << 8) | d[3]);
    const int16_t y = int16_t((d[4] << 8) | d[5]);
    if (x == -4096 || y == -4096 || z == -4096) return false; // ADC overflow
    *mx = float(x);
    *my = float(y);
    *mz = float(z); // raw LSB is fine
    return true;    //  (ratio only)
}
#endif // CONFIG_SS_LITE_MOD_GNSS_BN880

#if CONFIG_SS_LITE_MOD_IMU
static bool imu_init(void)
{
    if (i2c_master_probe(s_bus, SS_IMU_I2C_ADDR, 50) != ESP_OK) return false;

    i2c_device_config_t dc = {};
    dc.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    dc.device_address = SS_IMU_I2C_ADDR;
    dc.scl_speed_hz = SS_TOUCH_I2C_FREQ_HZ;
    if (i2c_master_bus_add_device(s_bus, &dc, &s_imu) != ESP_OK) return false;

    // Wake from sleep, clock = PLL (PWR_MGMT_1 = 0x01); accel ±2 g default.
    if (reg_write(s_imu, 0x6B, 0x01) != ESP_OK) {
        i2c_master_bus_rm_device(s_imu);
        s_imu = nullptr;
        return false;
    }
    return true;
}

static bool imu_read_accel(float* ax, float* ay, float* az)
{
    uint8_t d[6];
    if (reg_read(s_imu, 0x3B, d, 6) != ESP_OK) return false; // ACCEL_XOUT_H
    *ax = float(int16_t((d[0] << 8) | d[1])) / 16384.0f;     // ±2 g scale
    *ay = float(int16_t((d[2] << 8) | d[3])) / 16384.0f;
    *az = float(int16_t((d[4] << 8) | d[5])) / 16384.0f;
    return true;
}
#endif // CONFIG_SS_LITE_MOD_IMU

// ---------------------------------------------------------------------------
// Compass task — 15 Hz (matches HMC5883L ODR), prio below the UART pumps.
// ---------------------------------------------------------------------------
static void compass_task(void*)
{
    const TickType_t period = pdMS_TO_TICKS(1000 / SS_MAG_ODR_HZ);
    TickType_t wake = xTaskGetTickCount();

    // TWDT policy (S-02-009): the loop period is 1000/SS_MAG_ODR_HZ ms plus a
    // few 50 ms-timeout I2C transactions — worst case ~1 s (SS_MAG_ODR_HZ is 15
    // on lite and 1 on alpha/omega), provably far under the 5 s TWDT deadline on
    // every board. So this task subscribes and feeds once per iteration.
    if (ss_task_wdt_register() != ESP_OK)
        ESP_LOGW(TAG, "compass: TWDT subscribe failed (task runs unmonitored)");

    for (;;) {
        vTaskDelayUntil(&wake, period);
        ss_task_wdt_feed();

        ss_compass_reading_t r = {};
        r.src = SS_COMPASS_SRC_NONE;

#if CONFIG_SS_LITE_MOD_GNSS_BN880
        float mx, my, mz;
        if (s_mag && mag_read(&mx, &my, &mz)) {
            float pitch = 0.0f, roll = 0.0f;
            r.src = SS_COMPASS_SRC_MAG_ONLY;

#if CONFIG_SS_LITE_MOD_IMU
            float ax, ay, az;
            if (s_imu && imu_read_accel(&ax, &ay, &az)) {
                pitch = atan2f(-ax, sqrtf(ay * ay + az * az));
                roll = atan2f(ay, az);
                r.src = SS_COMPASS_SRC_MAG_TILTCOMP;
            }
#endif
            const float xh = mx * cosf(pitch) + mz * sinf(pitch);
            const float yh =
                mx * sinf(roll) * sinf(pitch) + my * cosf(roll) - mz * sinf(roll) * cosf(pitch);
            float hdg = atan2f(-yh, xh) * 180.0f / float(M_PI) + s_decl_deg;
            if (hdg < 0) hdg += 360.0f;
            if (hdg >= 360.0f) hdg -= 360.0f;

            r.heading_deg = hdg;
            r.pitch_deg = pitch * 180.0f / float(M_PI);
            r.roll_deg = roll * 180.0f / float(M_PI);
            r.valid = true;
            r.sample_ms = uint32_t(xTaskGetTickCount() * portTICK_PERIOD_MS);
        }
#endif // CONFIG_SS_LITE_MOD_GNSS_BN880

        if (r.valid) {
            xSemaphoreTake(s_lock, portMAX_DELAY);
            s_reading = r;
            s_ever = true;
            xSemaphoreGive(s_lock);
        }
    }
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------
esp_err_t ss_compass_start(void)
{
    s_lock = xSemaphoreCreateMutex();
    if (!s_lock) return ESP_ERR_NO_MEM;

    bool have_mag = false, have_imu = false;

#if CONFIG_SS_LITE_MOD_GNSS_BN880 || CONFIG_SS_LITE_MOD_IMU
    esp_err_t err = bus_init();
    if (err != ESP_OK) {
        // GT911 bring-up (EPIC-03) may own the bus already; that path will
        // hand us the bus handle instead. For boot scaffolding, just report.
        ESP_LOGW(TAG, "i2c bus init: %s (compass disabled)", esp_err_to_name(err));
        return ESP_OK;
    }
#if CONFIG_SS_LITE_MOD_GNSS_BN880
    have_mag = mag_init();
#endif
#if CONFIG_SS_LITE_MOD_IMU
    have_imu = imu_init();
#endif
#endif

    if (!have_mag) {
        ESP_LOGI(TAG, "no magnetometer found — compass source = NONE "
                      "(GNSS course / phone BLE fallback at app layer)");
        return ESP_OK; // idle at zero cost, no task
    }
    ESP_LOGI(TAG, "compass: HMC5883L @0x%02X%s", SS_MAG_I2C_ADDR,
             have_imu ? " + IMU tilt compensation @0x68" : " (mag-only, hold level)");

    const BaseType_t ok = ss_task_create_pinned(compass_task, "ss_compass", 3072, nullptr,
                                                SS_PRIO_SENSOR, nullptr, tskNO_AFFINITY);
    return ok == pdPASS ? ESP_OK : ESP_ERR_NO_MEM;
}

bool ss_compass_get(ss_compass_reading_t* out)
{
    if (!out || !s_lock) return false;
    xSemaphoreTake(s_lock, portMAX_DELAY);
    *out = s_reading;
    const bool ever = s_ever;
    xSemaphoreGive(s_lock);
    return ever;
}

void ss_compass_set_declination(float decl_deg)
{
    s_decl_deg = decl_deg;
}
