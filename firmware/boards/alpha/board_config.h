// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// board_config.h — SS-SP ALPHA 1.0 (ESP32-P4 flagship tactical pager)
//
// PLACEHOLDER PORT (S-02-004). This file exists so the Alpha board port
// compiles and satisfies the board-parity contract (tools/board-parity.py):
// it declares EXACTLY the same set of SS_* macros as the Lite reference port
// (firmware/boards/lite/board_config.h). It is NOT yet an authoritative pin
// map. Only values locked by 00_MASTER_SOFTWARE_PLAN.md §1.4 and
// models/CATALOG/ are filled; every hardware detail still open (pins, I2C
// addresses, bus frequencies, peripheral bindings) carries a conservative,
// compilable placeholder marked `// TODO(models/CATALOG): ...`.
//
// Locked facts used here (00_MASTER_SOFTWARE_PLAN.md §1.4):
//   MCU ESP32-P4NRW32X (RISC-V, 32 MB PSRAM); 2.4" 320x240 IPS display;
//   12x SK6805 addressable bezel. Radio (MM8108 HaLow), GNSS (MIA-M10Q),
//   magnetometer (BMM350), codec (ES8311) etc. are known from the BOM but
//   their pin maps are NOT locked, so no capability is claimed for them yet.
//
// NOTE ON BUILD TARGET: the canonical build still targets the DEFAULT
// firmware target (esp32s3, per firmware/sdkconfig.defaults). ESP32-P4 is
// recorded as the eventual MCU (SS_BOARD_MCU) but the target is NOT changed
// here; retargeting to esp32p4 is a separate, later story once the pin map
// and Kconfig/partitions for Alpha are locked.
//
// DO NOT promote this file to authoritative without also creating:
//   - the Alpha hardware reference doc (mirror of 01_SS-SP_LITE_...)
//   - the corresponding HW-consistency CI test under tests/hw/alpha/

#pragma once

// ============================================================================
// Board identity
// ============================================================================
#define SS_BOARD_ID_STR         "ss-sp-alpha-v1"
#define SS_BOARD_ID_INT         0x0002
#define SS_BOARD_HW_VARIANT     "ss_sp_alpha_1v0"
#define SS_BOARD_MCU            "ESP32-P4NRW32X"   // locked: doc 00 §1.4
#define SS_BOARD_FLASH_MB       0                  // TODO(models/CATALOG): confirm external flash size (P4 has no internal flash)
#define SS_BOARD_PSRAM_MB       32                 // locked: doc 00 §1.4
#define SS_BOARD_SRAM_KB        0                  // TODO(models/CATALOG): confirm on-chip SRAM figure to record

// Meshtastic-compat identifier (Lite-only concept; kept for board parity).
#define SS_MESHTASTIC_VARIANT   ""                 // TODO(models/CATALOG): N/A for Alpha unless a Meshtastic-compat variant is defined
#define SS_CROW_SELECT          0                  // not a CrowPanel board; kept for parity

// ============================================================================
// Capability mask (see ss_hal_caps.h)
//
// SS_BOARD_CAPS = onboard hardware (base) OR-ed with any plug-in expansion
// modules selected at build time. Apps never test CONFIG_* directly — they
// query ss_hal_has_cap() so the same application binary logic works on any
// board (Universal Test, HW ref §10).
//
// BASE is deliberately minimal: only ESP32-P4 SoC-intrinsic capabilities
// (crypto accelerators) are claimed. Every pin-dependent peripheral stays
// unclaimed until the Alpha pin map locks — see per-group TODOs below.
// ============================================================================
#include "ss_hal_caps.h"

#define SS_BOARD_CAPS_BASE  ( \
      SS_CAP_HW_RNG            \
    | SS_CAP_HW_AES            \
    | SS_CAP_HW_SHA            \
    /* TODO(models/CATALOG): add DISPLAY/TOUCH/BACKLIGHT/LEDS_BEZEL/MIC/     \
       SPEAKER/HAPTIC/SD/FUEL_GAUGE/PMIC/USB once the Alpha pin map locks */ \
    )

// ---- Primary mesh radio (MM8108 HaLow per doc 00 §1.4) --------------------
#define SS_BOARD_CAPS_RADIO   0   // TODO(models/CATALOG): claim SS_CAP_RADIO_HALOW | SS_CAP_1W_TX once MM8108/SKY66423 pin map locks

// ---- GNSS + compass (MIA-M10Q L1/L5 + BMM350 per doc 00 §1.4) -------------
#define SS_BOARD_CAPS_GNSS    0   // TODO(models/CATALOG): claim SS_CAP_GNSS_L1 | SS_CAP_GNSS_L5 | SS_CAP_MAGNETOMETER once pin map locks

// ---- Bridge coprocessor (ESP32-C6-MINI-1U per doc 00 §1.4) ----------------
#define SS_BOARD_CAPS_COPROC  0   // TODO(models/CATALOG): claim SS_CAP_RADIO_WIFI6 | SS_CAP_RADIO_BLE once C6 bridge link locks

// ---- Inertial sensor ------------------------------------------------------
#define SS_BOARD_CAPS_IMU     0   // TODO(models/CATALOG): claim SS_CAP_IMU once an IMU part + bus is locked

#define SS_BOARD_CAPS ( SS_BOARD_CAPS_BASE   \
                      | SS_BOARD_CAPS_RADIO  \
                      | SS_BOARD_CAPS_GNSS   \
                      | SS_BOARD_CAPS_COPROC \
                      | SS_BOARD_CAPS_IMU )

// ============================================================================
// Display (ER-TFT024IPS-3, 2.4" 320x240 IPS — doc 00 §1.4)
//   NOTE: the P4 drives a 16-bit parallel LCD; the SPI host/pins below are
//   compilable placeholders only, kept for parity with the Lite SPI port.
// ============================================================================
#define SS_LCD_DRIVER           SS_DRIVER_TBD      // TODO(models/CATALOG): confirm ER-TFT024IPS-3 controller IC
#define SS_LCD_W_PX             320                // locked: doc 00 §1.4 (320x240 landscape)
#define SS_LCD_H_PX             240                // locked: doc 00 §1.4
#define SS_LCD_NATIVE_ORIENT    SS_ORIENT_0        // TODO(models/CATALOG): confirm panel mount orientation
#define SS_LCD_FMT              SS_PIXFMT_RGB565   // TODO(models/CATALOG): confirm pixel format
#define SS_LCD_SPI_HOST         SPI2_HOST          // TODO(models/CATALOG): P4 uses parallel LCD; bus TBD
#define SS_LCD_SPI_FREQ_HZ      0                  // TODO(models/CATALOG): confirm pixel clock
#define SS_LCD_PIN_SCLK         -1                 // TODO(models/CATALOG): confirm
#define SS_LCD_PIN_MOSI         -1                 // TODO(models/CATALOG): confirm
#define SS_LCD_PIN_DC           -1                 // TODO(models/CATALOG): confirm
#define SS_LCD_PIN_CS           -1                 // TODO(models/CATALOG): confirm
#define SS_LCD_PIN_BL           -1                 // TODO(models/CATALOG): confirm backlight pin
#define SS_LCD_BL_LEDC_CH       0                  // TODO(models/CATALOG): confirm LEDC channel
#define SS_LCD_BL_LEDC_TIMER    0                  // TODO(models/CATALOG): confirm LEDC timer

// ============================================================================
// Touch (capacitive, on the ER-TFT024 panel — doc 00 §1.4)
// ============================================================================
#define SS_TOUCH_DRIVER         SS_DRIVER_TBD      // TODO(models/CATALOG): confirm touch controller
#define SS_TOUCH_I2C_ADDR       0                  // TODO(models/CATALOG): confirm I2C address
#define SS_TOUCH_I2C_PORT       I2C_NUM_0          // TODO(models/CATALOG): confirm I2C port
#define SS_TOUCH_I2C_FREQ_HZ    0                  // TODO(models/CATALOG): confirm bus frequency
#define SS_TOUCH_PIN_SDA        -1                 // TODO(models/CATALOG): confirm
#define SS_TOUCH_PIN_SCL        -1                 // TODO(models/CATALOG): confirm
#define SS_TOUCH_PIN_INT        -1                 // TODO(models/CATALOG): confirm
#define SS_TOUCH_PIN_RST        -1                 // TODO(models/CATALOG): confirm

// ============================================================================
// SD card (microSD, SDIO 2.0 per doc 00 §1.4)
//   NOTE: doc 00 specifies SDIO, not soft-SPI; the SPI pins below are
//   compilable placeholders kept for parity with the Lite soft-SPI port.
// ============================================================================
#define SS_SD_SOFT_SPI          0                  // TODO(models/CATALOG): Alpha is SDIO 2.0; confirm interface
#define SS_SD_PIN_MOSI          -1                 // TODO(models/CATALOG): confirm
#define SS_SD_PIN_MISO          -1                 // TODO(models/CATALOG): confirm
#define SS_SD_PIN_SCK           -1                 // TODO(models/CATALOG): confirm
#define SS_SD_PIN_CS            -1                 // TODO(models/CATALOG): confirm

// ============================================================================
// LoRa
//   Alpha has NO onboard LoRa (HaLow is the primary mesh radio, doc 00 §1.4).
//   These macros are retained for board parity only.
// ============================================================================
#define SS_LORA_DRIVER          SS_DRIVER_TBD      // TODO(models/CATALOG): Alpha has no onboard LoRa; kept for parity
#define SS_LORA_SPI_HOST        SPI2_HOST          // TODO(models/CATALOG): n/a; placeholder host
#define SS_LORA_PIN_CS          -1                 // TODO(models/CATALOG): n/a
#define SS_LORA_PIN_SCK         -1                 // TODO(models/CATALOG): n/a
#define SS_LORA_PIN_MISO        -1                 // TODO(models/CATALOG): n/a
#define SS_LORA_PIN_MOSI        -1                 // TODO(models/CATALOG): n/a
#define SS_LORA_PIN_RST         -1                 // TODO(models/CATALOG): n/a
#define SS_LORA_PIN_DIO1        -1                 // TODO(models/CATALOG): n/a
#define SS_LORA_PIN_DIO2        -1                 // TODO(models/CATALOG): n/a
#define SS_LORA_DEFAULT_FREQ_HZ 0                  // TODO(models/CATALOG): n/a

// ============================================================================
// I2S Microphone (MSM381A digital MEMS mic per doc 00 §1.4)
// ============================================================================
#define SS_MIC_I2S_PORT         I2S_NUM_0          // TODO(models/CATALOG): confirm I2S port
#define SS_MIC_PIN_BCLK         -1                 // TODO(models/CATALOG): confirm
#define SS_MIC_PIN_WS           -1                 // TODO(models/CATALOG): confirm
#define SS_MIC_PIN_DIN          -1                 // TODO(models/CATALOG): confirm
#define SS_MIC_SAMPLE_RATE_HZ   0                  // TODO(models/CATALOG): confirm sample rate
#define SS_MIC_BITS_PER_SAMPLE  0                  // TODO(models/CATALOG): confirm bit depth

// ============================================================================
// I2S Speaker (ES8311 codec + NS4150B amp per doc 00 §1.4)
// ============================================================================
#define SS_SPK_I2S_PORT         I2S_NUM_0          // TODO(models/CATALOG): confirm I2S port
#define SS_SPK_PIN_BCLK         -1                 // TODO(models/CATALOG): confirm
#define SS_SPK_PIN_WS           -1                 // TODO(models/CATALOG): confirm
#define SS_SPK_PIN_DOUT         -1                 // TODO(models/CATALOG): confirm
#define SS_SPK_PIN_MUTE         -1                 // TODO(models/CATALOG): confirm amp shutdown/mute pin
#define SS_SPK_HAS_MUTE_GPIO    0                  // TODO(models/CATALOG): confirm mute GPIO presence
#define SS_SPK_SAMPLE_RATE_HZ   0                  // TODO(models/CATALOG): confirm sample rate

// ============================================================================
// Buzzer
//   No dedicated buzzer in the Alpha BOM (haptics via DRV2625 LRA instead).
//   Retained for board parity only.
// ============================================================================
#define SS_BUZZER_PIN           -1                 // TODO(models/CATALOG): no discrete buzzer on Alpha; kept for parity
#define SS_BUZZER_LEDC_CH       0                  // TODO(models/CATALOG): n/a
#define SS_BUZZER_LEDC_TIMER    0                  // TODO(models/CATALOG): n/a

// ============================================================================
// UART
//   Console runs on the P4 native USB-Serial-JTAG; the aux UARTs below are
//   compilable placeholders (GNSS + C6 bridge links) pending pin lock.
// ============================================================================
#define SS_UART_GNSS_PORT       UART_NUM_1         // TODO(models/CATALOG): confirm GNSS UART
#define SS_UART_GNSS_PIN_RX     -1                 // TODO(models/CATALOG): confirm
#define SS_UART_GNSS_PIN_TX     -1                 // TODO(models/CATALOG): confirm
#define SS_UART_GNSS_BAUD       0                  // TODO(models/CATALOG): confirm baud
#define SS_UART_GNSS_RX_BUF     0                  // TODO(models/CATALOG): confirm RX buffer size

#define SS_UART_COPROC_PORT     UART_NUM_2         // TODO(models/CATALOG): confirm C6-bridge UART (may be SDIO/ESP-Hosted)
#define SS_UART_COPROC_PIN_RX   -1                 // TODO(models/CATALOG): confirm
#define SS_UART_COPROC_PIN_TX   -1                 // TODO(models/CATALOG): confirm
#define SS_UART_COPROC_BAUD     0                  // TODO(models/CATALOG): confirm baud
#define SS_UART_COPROC_RX_BUF   0                  // TODO(models/CATALOG): confirm RX buffer size

// Back-compat aliases (older code referenced UART0/UART1 by number).
#define SS_UART1_PIN_RX         SS_UART_GNSS_PIN_RX
#define SS_UART1_PIN_TX         SS_UART_GNSS_PIN_TX

// ============================================================================
// Magnetometer (Bosch BMM350 per doc 00 §1.4)
// ============================================================================
#define SS_MAG_DRIVER           SS_DRIVER_TBD      // TODO(models/CATALOG): BMM350 bindings unconfirmed
#define SS_MAG_I2C_PORT         I2C_NUM_0          // TODO(models/CATALOG): confirm I2C port
#define SS_MAG_I2C_ADDR         0                  // TODO(models/CATALOG): confirm I2C address
#define SS_MAG_ODR_HZ           1                  // TODO(models/CATALOG): confirm output data rate (nonzero placeholder: divisor in compass task)

// ============================================================================
// IMU
//   No IMU is locked in the Alpha BOM yet; retained for board parity only.
// ============================================================================
#define SS_IMU_I2C_PORT         I2C_NUM_0          // TODO(models/CATALOG): no IMU locked; kept for parity
#define SS_IMU_I2C_ADDR         0                  // TODO(models/CATALOG): n/a

// ============================================================================
// HaLow radio (MM8108 per doc 00 §1.4)
//   Retained for parity with the Lite HaLow-module alias block; the real
//   Alpha HaLow binding will be defined once its interface/pin map locks.
// ============================================================================
#define SS_HALOW_SPI_HOST       SS_LORA_SPI_HOST   // TODO(models/CATALOG): confirm MM8108 host/interface
#define SS_HALOW_PIN_CS         SS_LORA_PIN_CS     // TODO(models/CATALOG): confirm
#define SS_HALOW_PIN_SCK        SS_LORA_PIN_SCK    // TODO(models/CATALOG): confirm
#define SS_HALOW_PIN_MISO       SS_LORA_PIN_MISO   // TODO(models/CATALOG): confirm
#define SS_HALOW_PIN_MOSI       SS_LORA_PIN_MOSI   // TODO(models/CATALOG): confirm
#define SS_HALOW_PIN_RST        SS_LORA_PIN_RST    // TODO(models/CATALOG): confirm
#define SS_HALOW_PIN_IRQ        SS_LORA_PIN_DIO1   // TODO(models/CATALOG): confirm

// ============================================================================
// Mode select mux (mic vs radio)
//   Alpha does NOT share mic/radio pins (no mux constraint, doc 00 §1.3).
//   Retained for board parity only.
// ============================================================================
#define SS_MUX_MIC_RADIO_PIN    -1                 // TODO(models/CATALOG): Alpha has no mic/radio mux; kept for parity
#define SS_MUX_MIC_LEVEL        1                  // n/a
#define SS_MUX_RADIO_LEVEL      0                  // n/a
#define SS_MUX_DEFAULT_MODE     SS_MUX_MODE_RADIO  // n/a

// ============================================================================
// Battery / power (EA3059 PMIC + MAX17048 fuel gauge per doc 00 §1.4)
//   Fuel-gauge based; no raw ADC battery-sense line is assumed here.
// ============================================================================
#define SS_BATTERY_SENSE_PRESENT 0                 // TODO(models/CATALOG): confirm (Alpha uses MAX17048 fuel gauge, not ADC sense)

// ============================================================================
// LED indicator / bezel (12x SK6805-EC15 addressable RGB per doc 00 §1.4)
// ============================================================================
#define SS_LED_INDICATOR_PIN    -1                 // TODO(models/CATALOG): confirm status LED pin
#define SS_LED_BEZEL_COUNT      12                 // locked: doc 00 §1.4 (12x SK6805)

// ============================================================================
// Wake sources (Lite roles per C-01 §4.3 — S3-specific; this board's wake
// roles TBD per models/CATALOG; RTC timer wake via ss_power_wake_timer_set)
// ============================================================================
#define SS_WAKE_GPIO_TOUCH_INT  SS_TOUCH_PIN_INT   // TODO(models/CATALOG): confirm wake source pin
#define SS_WAKE_GPIO_LORA_DIO1  SS_LORA_PIN_DIO1   // TODO(models/CATALOG): confirm radio wake source pin

// ============================================================================
// Boot / provisioning
// ============================================================================
#define SS_HAS_SECURE_ELEMENT   0                  // TODO(models/CATALOG): confirm whether Alpha fits a secure element
#define SS_HAS_HW_RNG           1                  // ESP32-P4 SoC RNG (doc 00 §1.4 MCU)
