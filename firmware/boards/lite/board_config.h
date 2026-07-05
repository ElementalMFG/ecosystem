// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// board_config.h — SS-SP LITE (Elecrow CrowPanel Advance 3.5" HMI, "Meshtastic edition")
//
// AUTHORITATIVE PIN MAP. This file is the single source of truth for the Lite
// board and is derived directly from Meshtastic firmware
//   variants/esp32s3/elecrow_panel/variant.h
// combined with the Elecrow wiki and the Elecrow-RD schematic in
// docs/hardware/CROWPANEL-ADVANCE-3.5-HMI/. All cross-references are documented
// in 01_SS-SP_LITE_HARDWARE_REFERENCE.md.
//
// DO NOT MODIFY THIS FILE WITHOUT ALSO UPDATING:
//   - 01_SS-SP_LITE_HARDWARE_REFERENCE.md
//   - the corresponding CI HW-consistency test in tests/hw/lite/pin_map_test.c

#pragma once

// ============================================================================
// Board identity
// ============================================================================
#define SS_BOARD_ID_STR         "ss-sp-lite-v1"
#define SS_BOARD_ID_INT         0x0001
#define SS_BOARD_HW_VARIANT     "elecrow_crowpanel_advance_3v5_meshtastic"
#define SS_BOARD_MCU            "ESP32-S3-WROOM-1-N16R8"
#define SS_BOARD_FLASH_MB       16
#define SS_BOARD_PSRAM_MB       8
#define SS_BOARD_SRAM_KB        512

// Meshtastic-compat identifier (for variant selection at build time).
#define SS_MESHTASTIC_VARIANT   "elecrow_panel"
#define SS_CROW_SELECT          1

// ============================================================================
// Capability mask (see ss_hal_caps.h)
//
// SS_BOARD_CAPS = onboard hardware (base) OR-ed with any plug-in expansion
// modules selected at build time via Kconfig (CONFIG_SS_LITE_MOD_*). Apps
// never test CONFIG_* directly — they query ss_hal_has_cap() so the same
// application binary logic works on any board (Universal Test, HW ref §10).
// ============================================================================
#include "ss_hal_caps.h"

#define SS_BOARD_CAPS_BASE  ( \
      SS_CAP_DISPLAY            \
    | SS_CAP_TOUCH              \
    | SS_CAP_BACKLIGHT_PWM      \
    | SS_CAP_LED_INDICATOR      \
    | SS_CAP_MIC                \
    | SS_CAP_SPEAKER            \
    | SS_CAP_BUZZER             \
    | SS_CAP_SD                 \
    | SS_CAP_INTERNAL_FLASH_FS  \
    | SS_CAP_RADIO_BLE          \
    | SS_CAP_RADIO_WIFI4        \
    | SS_CAP_MUX_MIC_RADIO      \
    | SS_CAP_HW_RNG             \
    | SS_CAP_HW_AES             \
    | SS_CAP_HW_SHA             \
    | SS_CAP_USB_CDC            \
    /* NOT onboard: BATTERY_SENSE, FUEL_GAUGE, PMIC, IMU */ \
    )

// ---- Wireless-header module (exactly one; header carries SPI 0/10/9/3 +
//      RST 2 / DIO1 1 / DIO2 46). Default = SX1262 (Meshtastic edition). ----
#if defined(CONFIG_SS_LITE_MOD_HALOW)
  #define SS_BOARD_CAPS_RADIO   SS_CAP_RADIO_HALOW
#else
  #define SS_BOARD_CAPS_RADIO   SS_CAP_RADIO_LORA
#endif

// ---- Optional GNSS + compass module (BN-880: UART1 + HMC5883L on I2C0) ----
#if defined(CONFIG_SS_LITE_MOD_GNSS_BN880)
  #define SS_BOARD_CAPS_GNSS    (SS_CAP_GNSS_L1 | SS_CAP_MAGNETOMETER)
#else
  #define SS_BOARD_CAPS_GNSS    0
#endif

// ---- Optional mesh coprocessor (ESP32-C6/H2 on UART2 = pins 44/43) --------
#if defined(CONFIG_SS_LITE_MOD_COPROC_C6)
  #define SS_BOARD_CAPS_COPROC  SS_CAP_RADIO_WIFI6   // C6 adds Wi-Fi 6 + 802.15.4
#elif defined(CONFIG_SS_LITE_MOD_COPROC_H2)
  #define SS_BOARD_CAPS_COPROC  0                    // H2 adds 802.15.4/Thread only
#else
  #define SS_BOARD_CAPS_COPROC  0
#endif

// ---- Optional 6-axis IMU module on the I2C Grove header --------------------
#if defined(CONFIG_SS_LITE_MOD_IMU)
  #define SS_BOARD_CAPS_IMU     SS_CAP_IMU
#else
  #define SS_BOARD_CAPS_IMU     0
#endif

#define SS_BOARD_CAPS ( SS_BOARD_CAPS_BASE   \
                      | SS_BOARD_CAPS_RADIO  \
                      | SS_BOARD_CAPS_GNSS   \
                      | SS_BOARD_CAPS_COPROC \
                      | SS_BOARD_CAPS_IMU )

// ============================================================================
// Display (ILI9488, 4-wire SPI, 480x320 IPS)
// ============================================================================
#define SS_LCD_DRIVER           ILI9488
#define SS_LCD_W_PX             480
#define SS_LCD_H_PX             320
#define SS_LCD_NATIVE_ORIENT    SS_ORIENT_90       // landscape by default
#define SS_LCD_FMT              SS_PIXFMT_RGB565
#define SS_LCD_SPI_HOST         SPI3_HOST
#define SS_LCD_SPI_FREQ_HZ      40000000
#define SS_LCD_PIN_SCLK         42
#define SS_LCD_PIN_MOSI         39
#define SS_LCD_PIN_DC           41
#define SS_LCD_PIN_CS           40
#define SS_LCD_PIN_BL           38                 // PWM LEDC
#define SS_LCD_BL_LEDC_CH       0
#define SS_LCD_BL_LEDC_TIMER    0

// ============================================================================
// Touch (GT911 capacitive over I2C)
// ============================================================================
#define SS_TOUCH_DRIVER         GT911
#define SS_TOUCH_I2C_ADDR       0x5D
#define SS_TOUCH_I2C_PORT       I2C_NUM_0
#define SS_TOUCH_I2C_FREQ_HZ    400000
#define SS_TOUCH_PIN_SDA        15
#define SS_TOUCH_PIN_SCL        16
#define SS_TOUCH_PIN_INT        47
#define SS_TOUCH_PIN_RST        48

// ============================================================================
// SD card (soft-SPI)
// ============================================================================
#define SS_SD_SOFT_SPI          1
#define SS_SD_PIN_MOSI          6
#define SS_SD_PIN_MISO          4
#define SS_SD_PIN_SCK           5
#define SS_SD_PIN_CS            7

// ============================================================================
// LoRa SX1262
// ============================================================================
#define SS_LORA_DRIVER          SX1262
#define SS_LORA_SPI_HOST        SPI2_HOST
#define SS_LORA_PIN_CS          0
#define SS_LORA_PIN_SCK         10
#define SS_LORA_PIN_MISO        9
#define SS_LORA_PIN_MOSI        3
#define SS_LORA_PIN_RST         2
#define SS_LORA_PIN_DIO1        1
#define SS_LORA_PIN_DIO2        46
#define SS_LORA_DEFAULT_FREQ_HZ 915000000          // US ISM; user-configurable

// ============================================================================
// I2S Microphone (INMP441 or similar)
//   NOTE: pins collide with LoRa SPI; arbitration via GPIO 45 mux (see below).
// ============================================================================
#define SS_MIC_I2S_PORT         I2S_NUM_0
#define SS_MIC_PIN_BCLK         9
#define SS_MIC_PIN_WS           3
#define SS_MIC_PIN_DIN          10
#define SS_MIC_SAMPLE_RATE_HZ   16000
#define SS_MIC_BITS_PER_SAMPLE  16

// ============================================================================
// I2S Speaker (MAX98357A or similar)
// ============================================================================
#define SS_SPK_I2S_PORT         I2S_NUM_1
#define SS_SPK_PIN_BCLK         13
#define SS_SPK_PIN_WS           11
#define SS_SPK_PIN_DOUT         12
#define SS_SPK_PIN_MUTE         21                 // V1.2+ hardware
#define SS_SPK_HAS_MUTE_GPIO    1
#define SS_SPK_SAMPLE_RATE_HZ   16000

// ============================================================================
// Buzzer
// ============================================================================
#define SS_BUZZER_PIN           8
#define SS_BUZZER_LEDC_CH       1
#define SS_BUZZER_LEDC_TIMER    1

// ============================================================================
// UART
//   Console runs on the ESP32-S3 native USB-Serial-JTAG (not a UART), so both
//   UART peripherals are free for expansion modules:
//     UART1 (pins 18/17, "UART-IN" header)  → GNSS module (BN-880, NMEA-0183)
//     UART2 (pins 44/43, "UART-OUT" header) → mesh coprocessor (ESP32-C6/H2)
//   Pins 44/43 are the chip's default UART0 pins; we deliberately map the
//   UART2 peripheral onto them via the GPIO matrix and keep UART0 unused.
// ============================================================================
#define SS_UART_GNSS_PORT       UART_NUM_1
#define SS_UART_GNSS_PIN_RX     18
#define SS_UART_GNSS_PIN_TX     17
#define SS_UART_GNSS_BAUD       9600               // BN-880 factory default
#define SS_UART_GNSS_RX_BUF     2048               // DMA ring buffer (NMEA burst)

#define SS_UART_COPROC_PORT     UART_NUM_2
#define SS_UART_COPROC_PIN_RX   44
#define SS_UART_COPROC_PIN_TX   43
#define SS_UART_COPROC_BAUD     115200
#define SS_UART_COPROC_RX_BUF   4096               // DMA ring buffer (framed link)

// Back-compat aliases (older code referenced UART0/UART1 by number).
#define SS_UART1_PIN_RX         SS_UART_GNSS_PIN_RX
#define SS_UART1_PIN_TX         SS_UART_GNSS_PIN_TX

// ============================================================================
// Optional module: BN-880 GNSS + HMC5883L compass (CONFIG_SS_LITE_MOD_GNSS_BN880)
//   GNSS half  → UART1 above (NMEA-0183 out, UBX config in)
//   Compass    → HMC5883L on I2C0 (shared with GT911 touch, pins 15/16)
// ============================================================================
#define SS_MAG_DRIVER           HMC5883L
#define SS_MAG_I2C_PORT         I2C_NUM_0          // shared bus with touch
#define SS_MAG_I2C_ADDR         0x1E               // HMC5883L fixed address
#define SS_MAG_ODR_HZ           15                 // default output data rate

// ============================================================================
// Optional module: 6-axis IMU on I2C Grove header (CONFIG_SS_LITE_MOD_IMU)
//   Used for tilt-compensated compass (pitch/roll) and motion wake.
// ============================================================================
#define SS_IMU_I2C_PORT         I2C_NUM_0
#define SS_IMU_I2C_ADDR         0x68               // MPU-6050/ICM-426xx class

// ============================================================================
// Optional module: HaLow radio on the wireless header (CONFIG_SS_LITE_MOD_HALOW)
//   Replaces the SX1262 (one wireless-header slot). Reuses the header SPI +
//   control pins; mux GPIO 45 must be LOW (radio path), same as LoRa.
// ============================================================================
#define SS_HALOW_SPI_HOST       SS_LORA_SPI_HOST
#define SS_HALOW_PIN_CS         SS_LORA_PIN_CS
#define SS_HALOW_PIN_SCK        SS_LORA_PIN_SCK
#define SS_HALOW_PIN_MISO       SS_LORA_PIN_MISO
#define SS_HALOW_PIN_MOSI       SS_LORA_PIN_MOSI
#define SS_HALOW_PIN_RST        SS_LORA_PIN_RST
#define SS_HALOW_PIN_IRQ        SS_LORA_PIN_DIO1

// ============================================================================
// Mode select mux (mic vs LoRa) — GPIO 45
//   HIGH = mic path active
//   LOW  = LoRa/wireless path active
// The mic and LoRa share pins 9/3/10; only one may operate at a time.
// This is the reason SS_CAP_MUX_MIC_RADIO exists.
// ============================================================================
#define SS_MUX_MIC_RADIO_PIN    45
#define SS_MUX_MIC_LEVEL        1
#define SS_MUX_RADIO_LEVEL      0
#define SS_MUX_DEFAULT_MODE     SS_MUX_MODE_RADIO

// ============================================================================
// Battery / power
//   Lite has NO dedicated battery-voltage sense line (per Meshtastic issue
//   #7993). ss_power_status() will report battery_sense_valid=false.
// ============================================================================
#define SS_BATTERY_SENSE_PRESENT 0

// ============================================================================
// LED indicator (single status LED, if fitted; else compile out)
// ============================================================================
#define SS_LED_INDICATOR_PIN    -1                 // unpopulated on stock unit
#define SS_LED_BEZEL_COUNT      0                  // Lite has no bezel LED ring

// ============================================================================
// Deep-sleep wake sources
// ============================================================================
#define SS_WAKE_GPIO_TOUCH_INT  SS_TOUCH_PIN_INT
#define SS_WAKE_GPIO_LORA_DIO1  SS_LORA_PIN_DIO1

// ============================================================================
// Boot / provisioning
// ============================================================================
#define SS_HAS_SECURE_ELEMENT   0                  // ESP32-S3 eFuse-sealed seed only
#define SS_HAS_HW_RNG           1
