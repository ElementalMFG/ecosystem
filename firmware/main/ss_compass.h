// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_compass.h — tilt-compensated compass thread (directive goal C).
//
// Sensors (both optional plug-in modules, HW ref §3.15):
//   HMC5883L magnetometer  I2C0 @0x1E  (part of the BN-880 GNSS module)
//   6-axis IMU             I2C0 @0x68  (Grove header; accel used for tilt)
//
// I2C0 (pins 15/16 @400 kHz) is shared with the GT911 touch controller —
// this module creates the bus master if nobody has yet, or attaches to it.
//
// Fallback ladder (HW ref §3.15 compass note):
//   mag + IMU      → tilt-compensated heading (full accuracy)
//   mag only       → hold-level heading (UI shows "hold flat" hint)
//   neither        → SS_COMPASS_SRC_NONE; heading may come from GNSS course
//                    or a paired phone via BLE at the application layer.

#pragma once
#include "esp_err.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SS_COMPASS_SRC_NONE = 0,     // no usable sensor fitted
    SS_COMPASS_SRC_MAG_ONLY,     // HMC5883L, no tilt compensation
    SS_COMPASS_SRC_MAG_TILTCOMP, // HMC5883L + IMU accel
} ss_compass_src_t;

typedef struct {
    float heading_deg; // 0..360, magnetic north, tilt-compensated if src allows
    float pitch_deg;   // from IMU accel (0 if mag-only)
    float roll_deg;
    ss_compass_src_t src;
    bool valid;         // false until first good sample
    uint32_t sample_ms; // tick time of last sample
} ss_compass_reading_t;

// Probe sensors on I2C0 and spawn the 15 Hz compass task. Safe to call when
// no module is fitted: returns ESP_OK with src=NONE and zero runtime cost.
esp_err_t ss_compass_start(void);

// Copy the latest reading. Returns true if a reading has ever been produced.
bool ss_compass_get(ss_compass_reading_t* out);

// Apply magnetic declination (deg, east-positive) so heading_deg is true north.
// Application layer sets this from the GNSS fix (world magnetic model, S-16-xxx).
void ss_compass_set_declination(float decl_deg);

#ifdef __cplusplus
}
#endif
