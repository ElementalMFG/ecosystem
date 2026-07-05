<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# EPIC-03 — HAL for Lite (ESP32-S3 + SX1262 + Wi-Fi/BLE)

**Primary WG:** wg-firmware · **Contributing:** wg-hardware
**Priority:** P0 · **SKU:** L · **Milestone:** M1

## Outcome
Every HAL contract declared in `firmware/components/ss_hal/include/*.h` is implemented for the Lite board (ESP32-S3-WROOM-1U-N16R8 + SX1262 LoRa + on-die Wi-Fi 2.4 GHz + BLE 5.0), tested on real hardware, and passes conformance vectors.

## Constitution
C-00 `00_MASTER_SOFTWARE_PLAN.md` §HAL contracts; C-01 `01_SS-SP_LITE_HARDWARE_REFERENCE.md` §Lite board (ESP32-S3, SX1262, display, audio, power); C-08 `08_UNIVERSAL_CONNECTIVITY.md` §bearers (LoRa, Wi-Fi 2.4 GHz, BLE).

## Dependencies
EPIC-02.

## Shards
- **S-03.A Power** — battery gauge, USB-C charge, sleep/wake, load switches.
- **S-03.B Buttons & haptics** — 4-button matrix (PTT, up, down, back), debounce, vibe motor.
- **S-03.C Display** — 240×320 IPS SPI TFT, backlight PWM, framebuffer, tearing sync.
- **S-03.D Audio** — I2S mic + speaker amp, PTT capture chain, sidetone.
- **S-03.E LoRa driver** — SX1262 SPI, IRQ, TX/RX FIFOs, LBT, region PA table, duty-cycle guard.
- **S-03.F Wi-Fi 2.4 GHz** — STA + soft-AP, scan, connect, roaming stub.
- **S-03.G BLE 5** — GATT server for pairing + provisioning + companion-app link.
- **S-03.H Storage** — SPI flash partitions, W25Q128 external optional, wear-levelling.
- **S-03.I LEDs** — RGB status LED (link, activity, SOS breathe).
- **S-03.J Sensor stubs** — temperature (internal), no-GNSS/IMU tag.

## Exit criteria
1. All HAL headers have a Lite-specific `.c` implementation.
2. Hardware-in-loop rack passes power/audio/lora/wifi/ble conformance vectors.
3. LoRa TX meets FCC 15.247 duty cycle in software.
4. Idle current ≤ 25 mA @ 3.7 V; sleep current ≤ 0.5 mA.

## RACI
- R: wg-firmware · A: wg-firmware chair · C: wg-hardware · I: wg-security.

## Risks
| # | Risk | Mitigation |
|---|------|-----------|
| R03-01 | SX1262 driver bugs on high SF | Vendor errata sweep + long soak |
| R03-02 | Wi-Fi/BLE coexistence on 2.4 GHz | ESP-IDF coex config + scheduling test |
| R03-03 | Audio ground loop | Board rev-B ground plane review |
