// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// board_config.h — SS-SP OMEGA (next-gen enterprise / heavy-duty base-station-class pager)
//
// PLACEHOLDER PORT (S-02-005). This file exists so the Omega board port
// compiles and satisfies the board-parity contract (tools/board-parity.py):
// it declares EXACTLY the same set of SS_* macros as the Lite reference port
// (firmware/boards/lite/board_config.h). It is NOT yet an authoritative pin
// map. Only values locked by 00_MASTER_SOFTWARE_PLAN.md §1.2 / §11 and
// models/CATALOG/ are filled; every hardware detail still open (MCU, pins,
// I2C addresses, bus frequencies, display geometry, peripheral bindings)
// carries a conservative, compilable placeholder marked
// `// TODO(models/CATALOG): ...`.
//
// Locked facts used here:
//   Class: next-gen "Enterprise / heavy-duty" flagship (doc 00 §1.2 tier table).
//   Board of record: Omega v1.0 = PCB release v69 (2026-07-08, D-0020) —
//     see docs/dev/OMEGA_HW_BASELINE.md for the authoritative part list.
//   Radios: MM8108 HaLow + ESP32-C6 bridge (Wi-Fi/BLE). NO LoRa, NO
//     cellular, NO satellite on v1.0 (rev-2-gated; doc 00 §1.2 baseline note).
//   MCU: ESP32-P4NRW32X SoM (U1) + ESP32-C6-MINI-1 (U12) — locked by v69.
//   Display: ER-TFT3.92-1 3.92" IPS, ST7796S 8-bit parallel via 40-pin FPC.
//   On-device models: Omega is a target for whisper-tiny STT, Piper TTS, and
//     (Omega-only) llama-3.2-1b LLM assist (models/CATALOG/README.md); this
//     drives memory sizing later but locks no pin here.
//   Because no MCU part, pin map, or display geometry is locked, NO
//   pin-dependent capability is claimed yet — see per-group TODOs below.
//
// NOTE ON BUILD TARGET: the canonical build still targets the DEFAULT
// firmware target (esp32s3, per firmware/sdkconfig.defaults). Omega's MCU is
// TBD (SS_BOARD_MCU records that); the target is NOT changed here. Retargeting
// is a separate, later story once the Omega SoC, pin map, and Kconfig/
// partitions are locked.
//
// DO NOT promote this file to authoritative without also creating:
//   - the Omega hardware reference doc (mirror of 01_SS-SP_LITE_...)
//   - the corresponding HW-consistency CI test under tests/hw/omega/

#pragma once

// ============================================================================
// Board identity
// ============================================================================
#define SS_BOARD_ID_STR         "ss-sp-omega-v1"
#define SS_BOARD_ID_INT         0x0003
#define SS_BOARD_HW_VARIANT     "ss_sp_omega_1v0"
#define SS_BOARD_MCU            "TBD"              // locked-as-TBD: doc 00 §1.2 / §11 (RISC-V + Linux SoM likely)
#define SS_BOARD_FLASH_MB       0                  // TODO(models/CATALOG): confirm flash size once Omega SoC/module is selected
#define SS_BOARD_PSRAM_MB       0                  // TODO(models/CATALOG): confirm PSRAM/DRAM size once Omega SoC is selected
#define SS_BOARD_SRAM_KB        0                  // TODO(models/CATALOG): confirm on-chip SRAM figure to record

// Meshtastic-compat identifier (Lite-only concept; kept for board parity).
#define SS_MESHTASTIC_VARIANT   ""                 // TODO(models/CATALOG): N/A for Omega unless a Meshtastic-compat variant is defined
#define SS_CROW_SELECT          0                  // not a CrowPanel board; kept for parity

// ============================================================================
// Capability mask (see ss_hal_caps.h)
//
// SS_BOARD_CAPS = onboard hardware (base) OR-ed with any plug-in expansion
// modules selected at build time. Apps never test CONFIG_* directly — they
// query ss_hal_has_cap() so the same application binary logic works on any
// board (Universal Test, HW ref §10).
//
// BASE is deliberately minimal: only SoC-intrinsic crypto capabilities are
// claimed (any candidate Omega SoC — RISC-V or Linux SoM — provides HW RNG/
// AES/SHA). Every pin-dependent peripheral stays unclaimed until the Omega
// SoC and pin map lock — see per-group TODOs below.
// ============================================================================
#include "ss_hal_caps.h"

#define SS_BOARD_CAPS_BASE  ( \
      SS_CAP_HW_RNG            \
    | SS_CAP_HW_AES            \
    | SS_CAP_HW_SHA            \
    /* TODO(models/CATALOG): add DISPLAY/TOUCH/BACKLIGHT/LEDS_BEZEL/MIC/     \
       SPEAKER/HAPTIC/SD/FUEL_GAUGE/PMIC/USB once the Omega SoC + pin map    \
       lock */                                                               \
    )

// ---- Mesh radio (v69: MM8108 HaLow only — no LoRa on this board, D-0020) --
#define SS_BOARD_CAPS_RADIO   0   // TODO(S-05-020): claim SS_CAP_RADIO_HALOW once the v69 pin map lands here (NEVER SS_CAP_RADIO_LORA on v1.0)

// ---- GNSS + compass -------------------------------------------------------
#define SS_BOARD_CAPS_GNSS    0   // TODO(S-05-020): claim SS_CAP_GNSS_L1 | SS_CAP_MAGNETOMETER once the v69 pin map lands (MIA-M10Q I2C + BMM350, D-0020)

// ---- Bridge / connectivity coprocessor (v69: ESP32-C6-MINI-1, U12) --------
#define SS_BOARD_CAPS_COPROC  0   // TODO(models/CATALOG): claim SS_CAP_RADIO_WIFI6 | SS_CAP_RADIO_BLE once the coproc/link locks

// ---- Inertial sensor ------------------------------------------------------
#define SS_BOARD_CAPS_IMU     0   // TODO(models/CATALOG): claim SS_CAP_IMU once an IMU part + bus is locked

#define SS_BOARD_CAPS ( SS_BOARD_CAPS_BASE   \
                      | SS_BOARD_CAPS_RADIO  \
                      | SS_BOARD_CAPS_GNSS   \
                      | SS_BOARD_CAPS_COPROC \
                      | SS_BOARD_CAPS_IMU )

// ============================================================================
// Display ("larger IPS / OLED" per doc 00 §1.2 — geometry NOT locked)
//   The SPI host/pins below are compilable placeholders only, kept for parity
//   with the Lite SPI port.
// ============================================================================
#define SS_LCD_DRIVER           SS_DRIVER_TBD      // TODO(models/CATALOG): confirm Omega display controller IC
#define SS_LCD_W_PX             0                  // TODO(models/CATALOG): confirm display width (larger IPS/OLED, doc 00 §1.2)
#define SS_LCD_H_PX             0                  // TODO(models/CATALOG): confirm display height
#define SS_LCD_NATIVE_ORIENT    SS_ORIENT_0        // TODO(models/CATALOG): confirm panel mount orientation
#define SS_LCD_FMT              SS_PIXFMT_RGB565   // TODO(models/CATALOG): confirm pixel format
#define SS_LCD_SPI_HOST         SPI2_HOST          // TODO(models/CATALOG): confirm display bus/host
#define SS_LCD_SPI_FREQ_HZ      0                  // TODO(models/CATALOG): confirm pixel clock
#define SS_LCD_PIN_SCLK         -1                 // TODO(models/CATALOG): confirm
#define SS_LCD_PIN_MOSI         -1                 // TODO(models/CATALOG): confirm
#define SS_LCD_PIN_DC           -1                 // TODO(models/CATALOG): confirm
#define SS_LCD_PIN_CS           -1                 // TODO(models/CATALOG): confirm
#define SS_LCD_PIN_BL           -1                 // TODO(models/CATALOG): confirm backlight pin
#define SS_LCD_BL_LEDC_CH       0                  // TODO(models/CATALOG): confirm LEDC channel
#define SS_LCD_BL_LEDC_TIMER    0                  // TODO(models/CATALOG): confirm LEDC timer

// ============================================================================
// Touch (capacitive on the Omega panel — TBD)
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
// SD card
//   Interface TBD; the SPI pins below are compilable placeholders kept for
//   parity with the Lite soft-SPI port.
// ============================================================================
#define SS_SD_SOFT_SPI          0                  // TODO(models/CATALOG): confirm Omega SD interface
#define SS_SD_PIN_MOSI          -1                 // TODO(models/CATALOG): confirm
#define SS_SD_PIN_MISO          -1                 // TODO(models/CATALOG): confirm
#define SS_SD_PIN_SCK           -1                 // TODO(models/CATALOG): confirm
#define SS_SD_PIN_CS            -1                 // TODO(models/CATALOG): confirm

// ============================================================================
// LoRa
//   Omega carries onboard LoRa as part of the dual-radio set (doc 00 §11
//   Phase 4 item 2), but the pin map is NOT locked.
// ============================================================================
#define SS_LORA_DRIVER          SS_DRIVER_TBD      // TODO(models/CATALOG): confirm Omega LoRa part
#define SS_LORA_SPI_HOST        SPI2_HOST          // TODO(models/CATALOG): confirm host
#define SS_LORA_PIN_CS          -1                 // TODO(models/CATALOG): confirm
#define SS_LORA_PIN_SCK         -1                 // TODO(models/CATALOG): confirm
#define SS_LORA_PIN_MISO        -1                 // TODO(models/CATALOG): confirm
#define SS_LORA_PIN_MOSI        -1                 // TODO(models/CATALOG): confirm
#define SS_LORA_PIN_RST         -1                 // TODO(models/CATALOG): confirm
#define SS_LORA_PIN_DIO1        -1                 // TODO(models/CATALOG): confirm
#define SS_LORA_PIN_DIO2        -1                 // TODO(models/CATALOG): confirm
#define SS_LORA_DEFAULT_FREQ_HZ 0                  // TODO(models/CATALOG): confirm default frequency

// ============================================================================
// I2S Microphone
// ============================================================================
#define SS_MIC_I2S_PORT         I2S_NUM_0          // TODO(models/CATALOG): confirm I2S port
#define SS_MIC_PIN_BCLK         -1                 // TODO(models/CATALOG): confirm
#define SS_MIC_PIN_WS           -1                 // TODO(models/CATALOG): confirm
#define SS_MIC_PIN_DIN          -1                 // TODO(models/CATALOG): confirm
#define SS_MIC_SAMPLE_RATE_HZ   0                  // TODO(models/CATALOG): confirm sample rate
#define SS_MIC_BITS_PER_SAMPLE  0                  // TODO(models/CATALOG): confirm bit depth

// ============================================================================
// I2S Speaker
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
//   No discrete buzzer locked in the Omega BOM. Retained for board parity.
// ============================================================================
#define SS_BUZZER_PIN           -1                 // TODO(models/CATALOG): confirm discrete buzzer presence; kept for parity
#define SS_BUZZER_LEDC_CH       0                  // TODO(models/CATALOG): n/a
#define SS_BUZZER_LEDC_TIMER    0                  // TODO(models/CATALOG): n/a

// ============================================================================
// UART
//   Console interface TBD; the aux UARTs below are compilable placeholders
//   (GNSS + connectivity coproc links) pending pin lock.
// ============================================================================
#define SS_UART_GNSS_PORT       UART_NUM_1         // TODO(models/CATALOG): confirm GNSS UART
#define SS_UART_GNSS_PIN_RX     -1                 // TODO(models/CATALOG): confirm
#define SS_UART_GNSS_PIN_TX     -1                 // TODO(models/CATALOG): confirm
#define SS_UART_GNSS_BAUD       0                  // TODO(models/CATALOG): confirm baud
#define SS_UART_GNSS_RX_BUF     0                  // TODO(models/CATALOG): confirm RX buffer size

#define SS_UART_COPROC_PORT     UART_NUM_2         // TODO(S-05-020): confirm C6-bridge link (UART vs SDIO/ESP-Hosted); no cellular/LEO on v1.0 (D-0020)
#define SS_UART_COPROC_PIN_RX   -1                 // TODO(models/CATALOG): confirm
#define SS_UART_COPROC_PIN_TX   -1                 // TODO(models/CATALOG): confirm
#define SS_UART_COPROC_BAUD     0                  // TODO(models/CATALOG): confirm baud
#define SS_UART_COPROC_RX_BUF   0                  // TODO(models/CATALOG): confirm RX buffer size

// Back-compat aliases (older code referenced UART0/UART1 by number).
#define SS_UART1_PIN_RX         SS_UART_GNSS_PIN_RX
#define SS_UART1_PIN_TX         SS_UART_GNSS_PIN_TX

// ============================================================================
// Magnetometer
// ============================================================================
#define SS_MAG_DRIVER           SS_DRIVER_TBD      // TODO(models/CATALOG): magnetometer part/bindings unconfirmed
#define SS_MAG_I2C_PORT         I2C_NUM_0          // TODO(models/CATALOG): confirm I2C port
#define SS_MAG_I2C_ADDR         0                  // TODO(models/CATALOG): confirm I2C address
#define SS_MAG_ODR_HZ           1                  // TODO(models/CATALOG): confirm output data rate (nonzero placeholder: divisor in compass task)

// ============================================================================
// IMU
//   No IMU locked in the Omega BOM yet; retained for board parity only.
// ============================================================================
#define SS_IMU_I2C_PORT         I2C_NUM_0          // TODO(models/CATALOG): no IMU locked; kept for parity
#define SS_IMU_I2C_ADDR         0                  // TODO(models/CATALOG): n/a

// ============================================================================
// HaLow radio (onboard per doc 00 §11 Phase 4 item 2)
//   Retained for parity with the Lite HaLow-module alias block; the real
//   Omega HaLow binding will be defined once its interface/pin map locks.
// ============================================================================
#define SS_HALOW_SPI_HOST       SS_LORA_SPI_HOST   // TODO(models/CATALOG): confirm HaLow host/interface
#define SS_HALOW_PIN_CS         SS_LORA_PIN_CS     // TODO(models/CATALOG): confirm
#define SS_HALOW_PIN_SCK        SS_LORA_PIN_SCK    // TODO(models/CATALOG): confirm
#define SS_HALOW_PIN_MISO       SS_LORA_PIN_MISO   // TODO(models/CATALOG): confirm
#define SS_HALOW_PIN_MOSI       SS_LORA_PIN_MOSI   // TODO(models/CATALOG): confirm
#define SS_HALOW_PIN_RST        SS_LORA_PIN_RST    // TODO(models/CATALOG): confirm
#define SS_HALOW_PIN_IRQ        SS_LORA_PIN_DIO1   // TODO(models/CATALOG): confirm

// ============================================================================
// Mode select mux (mic vs radio)
//   Omega carries dual onboard radios (no shared mic/radio pin mux assumed).
//   Retained for board parity only.
// ============================================================================
#define SS_MUX_MIC_RADIO_PIN    -1                 // TODO(models/CATALOG): Omega mux constraint TBD; kept for parity
#define SS_MUX_MIC_LEVEL        1                  // n/a
#define SS_MUX_RADIO_LEVEL      0                  // n/a
#define SS_MUX_DEFAULT_MODE     SS_MUX_MODE_RADIO  // n/a

// ============================================================================
// Battery / power
//   Fuel-gauge / PMIC design TBD; no raw ADC battery-sense line is assumed.
// ============================================================================
#define SS_BATTERY_SENSE_PRESENT 0                 // TODO(models/CATALOG): confirm power-monitoring interface (fuel gauge vs ADC sense)

// ============================================================================
// LED indicator / bezel
// ============================================================================
#define SS_LED_INDICATOR_PIN    -1                 // TODO(models/CATALOG): confirm status LED pin
#define SS_LED_BEZEL_COUNT      0                  // TODO(models/CATALOG): confirm addressable bezel LED count

// ============================================================================
// Wake sources (Lite roles per C-01 §4.3 — S3-specific; this board's wake
// roles TBD per models/CATALOG; RTC timer wake via ss_power_wake_timer_set)
// ============================================================================
#define SS_WAKE_GPIO_TOUCH_INT  SS_TOUCH_PIN_INT   // TODO(models/CATALOG): confirm wake source pin
#define SS_WAKE_GPIO_LORA_DIO1  SS_LORA_PIN_DIO1   // TODO(models/CATALOG): confirm radio wake source pin

// ============================================================================
// Boot / provisioning
// ============================================================================
#define SS_HAS_SECURE_ELEMENT   0                  // TODO(models/CATALOG): confirm whether Omega fits a secure element
#define SS_HAS_HW_RNG           1                  // base-crypto assumption: candidate Omega SoC (doc 00 §1.2) provides a HW RNG
