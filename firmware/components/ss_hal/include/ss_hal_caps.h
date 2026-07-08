// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_hal_caps.h — Capability flags exposed by the HAL for a given board.
//
// Upper layers query these to decide whether to enable a feature ("Do we have
// a mag? A HaLow radio? A GNSS?"). The board's board_config.h sets
// SS_BOARD_CAPS to a bitwise OR of these flags at compile time; ss_hal_has_cap
// reads that value at runtime.

#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SS_CAP_DISPLAY           (1ULL << 0)
#define SS_CAP_TOUCH             (1ULL << 1)
#define SS_CAP_BACKLIGHT_PWM     (1ULL << 2)
#define SS_CAP_LEDS_BEZEL        (1ULL << 3)  // multi-LED bezel (Alpha 12x SK6805)
#define SS_CAP_LED_INDICATOR     (1ULL << 4)  // single status LED
#define SS_CAP_HAPTIC            (1ULL << 5)
#define SS_CAP_MIC               (1ULL << 6)
#define SS_CAP_SPEAKER           (1ULL << 7)
#define SS_CAP_BUZZER            (1ULL << 8)  // Lite has GPIO 8 buzzer
#define SS_CAP_SD                (1ULL << 9)
#define SS_CAP_INTERNAL_FLASH_FS (1ULL << 10)
#define SS_CAP_BATTERY_SENSE     (1ULL << 11) // Lite: NO (per Meshtastic #7993)
#define SS_CAP_CHARGER_STATUS    (1ULL << 12)
#define SS_CAP_FUEL_GAUGE        (1ULL << 13) // Alpha: MAX17048
#define SS_CAP_PMIC              (1ULL << 14) // Alpha: EA3059QDR
#define SS_CAP_RADIO_LORA        (1ULL << 15)
#define SS_CAP_RADIO_HALOW       (1ULL << 16)
#define SS_CAP_RADIO_BLE         (1ULL << 17)
#define SS_CAP_RADIO_WIFI4       (1ULL << 18)
#define SS_CAP_RADIO_WIFI6       (1ULL << 19)
#define SS_CAP_RADIO_LR11XX      (1ULL << 20) // future
#define SS_CAP_GNSS_L1           (1ULL << 21)
#define SS_CAP_GNSS_L5           (1ULL << 22)
#define SS_CAP_IMU               (1ULL << 23)
#define SS_CAP_MAGNETOMETER      (1ULL << 24)
#define SS_CAP_BAROMETER         (1ULL << 25)
#define SS_CAP_MUX_MIC_RADIO     (1ULL << 26) // Lite mic-vs-LoRa mux (GPIO45)
#define SS_CAP_SECURE_ELEM       (1ULL << 27) // ATECC608 or similar
#define SS_CAP_HW_RNG            (1ULL << 28)
#define SS_CAP_HW_AES            (1ULL << 29)
#define SS_CAP_HW_SHA            (1ULL << 30)
#define SS_CAP_USB_CDC           (1ULL << 31)
#define SS_CAP_USB_HOST          (1ULL << 32)
#define SS_CAP_ETHERNET          (1ULL << 33)
#define SS_CAP_CELLULAR          (1ULL << 34)
#define SS_CAP_SATCOM            (1ULL << 35)
#define SS_CAP_FEM_PA            (1ULL << 36)
#define SS_CAP_1W_TX             (1ULL << 37) // Alpha SKY66423
#define SS_CAP_HEADLESS          (1ULL << 38) // no display, gateway build

// Reserved bits 39..63 for future.

#ifdef __cplusplus
} // extern "C"
#endif
