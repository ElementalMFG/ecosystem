// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_hal_types.h — Shared types used across HAL headers.

#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// ----- Pixel formats -----------------------------------------------------
typedef enum {
    SS_PIXFMT_MONO1,          // 1 bpp
    SS_PIXFMT_GRAY4,          // 4 bpp
    SS_PIXFMT_GRAY8,          // 8 bpp
    SS_PIXFMT_RGB565,         // 16 bpp
    SS_PIXFMT_RGB888,         // 24 bpp
    SS_PIXFMT_ARGB8888,       // 32 bpp
} ss_pixfmt_t;

// ----- Display shape ----------------------------------------------------
typedef enum {
    SS_SHAPE_RECT,
    SS_SHAPE_SQUARE,
    SS_SHAPE_ROUND,
    SS_SHAPE_OCTAGON,
    SS_SHAPE_EINK_RECT,
    SS_SHAPE_HEADLESS,
} ss_shape_t;

typedef enum {
    SS_ORIENT_0,
    SS_ORIENT_90,
    SS_ORIENT_180,
    SS_ORIENT_270,
} ss_orient_t;

// ----- Rect / point -----------------------------------------------------
typedef struct { int16_t x, y; } ss_point_t;
typedef struct { int16_t x, y, w, h; } ss_rect_t;

// ----- Color helpers ----------------------------------------------------
typedef struct { uint8_t r, g, b, a; } ss_color_t;

// ----- Mux mode (Lite mic-vs-LoRa; portable no-op elsewhere) -----------
typedef enum {
    SS_MUX_MODE_RADIO = 0,    // GPIO45 LOW: wireless/LoRa/BLE path
    SS_MUX_MODE_MIC   = 1,    // GPIO45 HIGH: microphone I2S path
} ss_mux_mode_t;

typedef enum {
    SS_MUX_OWNER_NONE = 0,
    SS_MUX_OWNER_RADIO_LORA,
    SS_MUX_OWNER_RADIO_BLE,
    SS_MUX_OWNER_RADIO_WIFI,
    SS_MUX_OWNER_AUDIO_MIC,
    SS_MUX_OWNER_TEST,
} ss_mux_owner_t;

// ----- Frequency / power ------------------------------------------------
typedef struct {
    uint32_t freq_hz_min;
    uint32_t freq_hz_max;
    int8_t   tx_pwr_dbm_min;
    int8_t   tx_pwr_dbm_max;
} ss_rf_limits_t;

// ----- Time -------------------------------------------------------------
typedef uint64_t ss_mono_us_t;     // monotonic microseconds since boot
typedef int64_t  ss_unix_ms_t;     // milliseconds since Unix epoch

// ----- Input event ------------------------------------------------------
typedef enum {
    SS_INPUT_TOUCH_DOWN,
    SS_INPUT_TOUCH_MOVE,
    SS_INPUT_TOUCH_UP,
    SS_INPUT_TAP,
    SS_INPUT_LONG_PRESS,
    SS_INPUT_SWIPE_L, SS_INPUT_SWIPE_R,
    SS_INPUT_SWIPE_U, SS_INPUT_SWIPE_D,
    SS_INPUT_ROTARY_CW, SS_INPUT_ROTARY_CCW,
    SS_INPUT_BUTTON_DOWN,
    SS_INPUT_BUTTON_UP,
    SS_INPUT_PTT_DOWN,
    SS_INPUT_PTT_UP,
    SS_INPUT_VOICE_CMD,
    SS_INPUT_ACCEL_SHAKE,
    SS_INPUT_ACCEL_FALL,
    SS_INPUT_TILT,
} ss_input_kind_t;

typedef struct {
    ss_input_kind_t kind;
    ss_mono_us_t    at;
    int16_t         x, y;
    uint16_t        code;      // button code or vk
    int16_t         delta;     // rotary delta or swipe strength
    void*           extra;
} ss_input_event_t;

#ifdef __cplusplus
} // extern "C"
#endif
