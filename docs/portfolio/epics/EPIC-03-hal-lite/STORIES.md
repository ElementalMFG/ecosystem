<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# EPIC-03 — Stories

Format per `../../00_METHODOLOGY.md` §2.7. Meta lines are machine-parsed.

---

### S-03-001 — `ss_power` for Lite — no-gauge contract + power state machine
As a firmware engineer I want the Lite power HAL implemented against the frozen `ss_hal_power.h` contract so that power state is truthful on hardware that has no fuel gauge.
- AC: `ss_power_init`/`ss_power_status` return the contract-mandated no-gauge values (`battery_sense_valid=false`, `v_mv=0`, C-01 §Meshtastic-#7993 rationale cited at the impl site); the power state machine (`ss_power_enter`, `ss_power_wake_source_add`, `ss_power_reboot`, `ss_power_shutdown`) is implemented per the frozen header with a pure host-tested decision core; compiles green on all three boards
- Meta: Shard=A | Type=Feature | Size=M | Prio=P0 | Status=DONE | SKU=L | PRD=NF-PWR-01 | Const=C-00,C-01
- Tasks: spec — pure decision-core contract (transition table + wake-source table + no-gauge status) split from IDF glue · design — `ss_power_core.[ch]` (host-testable, no esp_err) + `ss_power.c` IDF impl of the frozen `ss_hal_power.h` · impl — new `firmware/components/ss_power/` component (CMake, core, glue) · test — host gtest `test_ss_power_core` for transitions/wake-sources/no-gauge status · docs — impl-site C-01 §Meshtastic-#7993 citation + ENGINEERING_LOG
- Deps: — (retargeted 2026-07-08: gauge intent preserved in the Alpha epic — see ENGINEERING_LOG + the dropped-story pointers)

### S-03-002 — USB-C PD source detection + charge state machine
As a device owner I want USB-C source detection and a charge state machine so that charging is safe and its status is always accurate.
- AC: source capability detected on plug-in; charge states (pre-charge/CC/CV/full/fault) transition correctly; fault states surfaced to log and UI
- Meta: Shard=A | Type=Feature | Size=M | Prio=P0 | Status=DROPPED | SKU=L | PRD=— | Const=C-00,C-01
- Deps: DROPPED for Lite 2026-07-08 (triage): no charger IC/PMIC on Lite (C-01, `board_config.h` NOT-onboard list) — capability moved intact to S-04-026 (Alpha)

### S-03-003 — Sleep entry/exit with the canonical Lite wake set (touch, LoRa IRQ, RTC timer)
As a firmware engineer I want sleep entry/exit wired to Lite's real wake sources so that the Lite meets its standby battery target.
- AC: the canonical wake trio is wired through `ss_power` — touch INT (GPIO47, light-sleep wake only per S3 RTC-capability rules), LoRa DIO1 (GPIO1, light+deep), and RTC timer wake via the S-03-030 contract surface; wake-table decisions host-tested; the bench-measurement procedure (sleep current ≤ 0.5 mA target, wake-to-responsive latency) is written and executed at the first hardware session
- Meta: Shard=A | Type=Feature | Size=L | Prio=P0 | Status=IN_REVIEW | SKU=L | PRD=NF-PWR-01 | Const=C-00,C-01
- Tasks: spec canonical Lite wake set (touch INT 47 light-only, LoRa DIO1 1 light+deep, RTC timer via S-03-030) reconciled to C-01 §4.3 / NF-PWR-01 · design host-testable light-vs-deep partition (pure S3 RTC-capability predicate) + canonical wiring API · impl `ss_power_wake_lite_defaults()` from board_config macros + core RTC-capability predicate feeding the deep-wake filter · test host coverage for the wake-table light/deep partition decisions · docs bench-measurement procedure (sleep ≤ 0.5 mA, wake-to-responsive latency) for the D-0013 session
- Deps: S-03-001, S-03-030; bench measurements need the D-0013 hardware session — story parks at IN_REVIEW until then (retargeted 2026-07-08: "button" wake was drift — board_config declares touch INT + LoRa DIO1; C-01 §4.3 reconciliation lands with S-03-030)

### S-03-004 — Input events — GT911 touch gestures + BOOT button
As a device owner I want debounced touch and BOOT-button input events so that PTT and navigation feel instant and reliable on Lite's actual input hardware.
- AC: GT911 touch tap/long-press/swipe events emitted through the HAL input contract; BOOT button (GPIO0) debounced with press/long-press/release events, deferring to the S-02-016 recovery-window ownership; on-screen PTT press reaches the audio pipeline < 25 ms; behaviour degrades per capability flags when hardware is absent
- Meta: Shard=B | Type=Feature | Size=S | Prio=P0 | Status=IN_REVIEW | SKU=L | PRD=F-MSG-04,F-UI-06 | Const=C-00,C-01
- Tasks: spec input event mapping (GT911 tap/long-press/swipe thresholds → `ss_input_kind_t`; BOOT press/long-press/release; GPIO0 pin-share + recovery-window deferral honouring `CONFIG_SS_RECOVERY_ENTRY_WINDOW_MS`; PTT latency budget < 25 ms) · design pure IDF-free cores (gesture recogniser FSM + button debounce FSM) in new `ss_input` component, mirroring `ss_power` core/glue split · impl `firmware/components/ss_input/{include/ss_input_core.h,src/ss_input_core.c,src/ss_touch.c,src/ss_buttons.c,CMakeLists.txt}` implementing `ss_hal_touch.h`/`ss_hal_buttons.h`, capability-gated via `ss_hal_has_cap(SS_CAP_TOUCH)` · test host gtests for gesture + debounce + recovery-deferral cores, lite CI build; on-hardware GT911 tap/swipe emission + measured PTT < 25 ms pending an attached board · docs contract doc-block + changelog
- Deps: S-03-001 (DONE — ss_power core/glue pattern), S-02-016 (IN_REVIEW — `ss_recovery.h` contract frozen; BOOT input defers GPIO0 to the recovery entry window), ss_hal input contracts (`ss_hal_touch.h`/`ss_hal_buttons.h`/`ss_input_event_t` — already frozen in ss_hal); on-hardware ACs need an attached Lite board — story parks at IN_REVIEW until then (retargeted 2026-07-08: Lite has no button matrix — touch + BOOT only per C-01; matrix intent preserved in the Alpha epic)

### S-03-005 — Haptic driver (DRV2605L or PWM)
As a device owner I want haptic feedback so that interactions are confirmed without looking at the screen.
- AC: DRV2605L (or PWM fallback) plays a basic effect set; per-event haptic hooks exposed to `ss_ui`; degrades gracefully when hardware is absent
- Meta: Shard=B | Type=Feature | Size=S | Prio=P1 | Status=DROPPED | SKU=L | PRD=F-UI-03 | Const=C-00,C-01
- Deps: DROPPED for Lite 2026-07-08 (triage): no haptic IC on Lite (buzzer GPIO8 covers audible feedback via ss_diag) — capability moved intact to S-04-028 (Alpha)

### S-03-006 — TFT display driver (ILI9488 480×320) SPI DMA
As a firmware engineer I want an SPI-DMA driver for Lite's actual panel (ILI9488, 480×320) so that the UI renders smoothly within the Lite performance budget.
- AC: 60 FPS partial redraw of a 32×32 tile; controller/geometry taken from `board_config.h` `SS_LCD_*` (ILI9488 per C-01 §3), never hardcoded; DMA transfers tear-free with sync; builds on (and retires) the `ss_display_boot` bring-up scaffolding
- Meta: Shard=C | Type=Feature | Size=M | Prio=P0 | Status=IN_REVIEW | SKU=L | PRD=NF-PERF-01 | Const=C-00,C-01
- Tasks: spec — geometry/window/clipping math + RGB565→RGB666 conversion + 32×32-tile SPI transfer-time budget (bytes·8 ÷ `SS_LCD_SPI_FREQ_HZ` < 16.67 ms for 60 FPS) + tear-free flush sync (block on DMA completion before scratch reuse); all params from `board_config.h` `SS_LCD_*`, never hardcoded · design — pure IDF-free `ss_display_core.[ch]` (window bounds, clip, 565→666, per-tile xfer-µs budget; no `esp_err`) + thin IDF glue implementing frozen `ss_hal_display.h`, mirroring `ss_power`/`ss_input` core/glue split · impl — new `firmware/components/ss_display/{include/ss_display_core.h,src/ss_display_core.c,src/ss_display.cpp,CMakeLists.txt}` implementing `ss_display_init/info/flush/backlight/sleep/set_orient`, capability-gated via `ss_hal_has_cap(SS_CAP_DISPLAY|SS_CAP_BACKLIGHT_PWM)`; migrate the ILI9488 init sequence + chunked convert-blit + LEDC backlight out of `main/ss_display_boot.*` and **retire** it (delete `main/ss_display_boot.{h,cpp}`, rewire `main.cpp` + `main/CMakeLists.txt` to `ss_display_init` + boot self-test) · test — host gtest `test_ss_display_core` (clip vectors, 565→666 conversion vectors, 32×32 tile budget < 16.67 ms, geometry sourced from mock `SS_LCD_*`), lite CI build; on-hardware 60 FPS partial-redraw + tear-free observation pending an attached Lite board · docs — impl-site C-01 §3 citation + changelog + ENGINEERING_LOG
- Deps: S-03-001 (DONE — `ss_power` core/glue pattern reused), `ss_hal_display.h` contract (already frozen in `ss_hal`), `ss_display_boot` scaffolding (`main/ss_display_boot.*` — migrated then retired); on-hardware ACs (60 FPS empirical, tear-free) need an attached Lite board — story parks at IN_REVIEW until then (retargeted 2026-07-08: story predated the Lite panel lock; ILI9341/ST7789 were never Lite parts per C-01)

### S-03-007 — Backlight PWM + auto-dim on inactivity
As a device owner I want backlight PWM with auto-dim on inactivity so that battery is conserved without hurting readability.
- AC: PWM dimming flicker-free across the full range; auto-dim after configurable inactivity timeout; any input restores brightness immediately
- Meta: Shard=C | Type=Feature | Size=S | Prio=P1 | Status=DRAFT | SKU=L | PRD=NF-PWR-01 | Const=C-00,C-01

### S-03-008 — Framebuffer allocation policy (single/double)
As a firmware engineer I want a documented single/double framebuffer allocation policy so that RAM use and tearing are controlled deliberately.
- AC: policy selectable at build time; RAM budget per mode documented; no visible tearing in double-buffer mode
- Meta: Shard=C | Type=Feature | Size=S | Prio=P1 | Status=DRAFT | SKU=L | PRD=NF-PERF-01 | Const=C-00,C-01

### S-03-009 — I²S microphone capture (16 kHz mono)
As a firmware engineer I want I²S microphone capture at 16 kHz mono so that voice messages and on-device STT get clean input.
- AC: 16 kHz mono PCM stream delivered with bounded jitter; capture starts within the PTT latency budget; SNR measured and documented on reference hardware
- Meta: Shard=D | Type=Feature | Size=M | Prio=P0 | Status=IN_REVIEW | SKU=L | PRD=F-MSG-03,F-VOX-01 | Const=C-00,C-01
- Tasks: spec mic capture params (16 kHz mono 16-bit locked per `SS_MIC_*`; INMP441 24-in-32 slot → 16-bit PCM; DMA ring sizing for bounded read jitter; mux acquire as `SS_MUX_OWNER_AUDIO_MIC`/`SS_MUX_MODE_MIC` inside the PTT <25 ms budget) · design pure IDF-free core (format validation + DMA/jitter sizing + mux-plan decision) in new `ss_audio` component, mirroring the `ss_muxctl` core/glue split · impl `firmware/components/ss_audio/{include/ss_audio_core.h,src/ss_audio_core.c,src/ss_mic.c,CMakeLists.txt}` implementing the mic half of `ss_hal_audio.h`, capability-gated via `ss_hal_has_cap(SS_CAP_MIC)`, mux via `ss_mux_acquire/release` · test host harness for the core (format accept/reject, jitter-buffer math, mux plan), lite CI build; on-hardware 16 kHz capture + measured SNR + measured capture-start latency pending an attached board · docs contract doc-block + changelog + engineering-log
- Deps: S-03-033 (mic shares GPIO 9/3/10 with LoRa behind the GPIO45 mux — capture must acquire SS_MUX_MODE_MIC) DONE

### S-03-010 — I²S speaker output + class-D amp enable
As a device owner I want I²S speaker output with class-D amp control so that received voice is audible and power is not wasted when idle.
- AC: playback verified at target sample rates; amp enable/disable sequenced pop-free; sidetone path available during PTT capture
- Meta: Shard=D | Type=Feature | Size=M | Prio=P0 | Status=IN_REVIEW | SKU=L | PRD=F-MSG-03,F-VOX-02 | Const=C-00,C-01
- Tasks: spec speaker output params (16 kHz mono 16-bit locked per `SS_SPK_SAMPLE_RATE_HZ`; MAX98357A class-D SD/enable on `SS_SPK_PIN_MUTE` GPIO21; pop-free amp on/off ordering — clocks stable before enable, amp muted before clocks stop; software Q15 volume since the class-D amp has no gain register; speaker is mux-independent so it runs concurrently with mic capture = sidetone) · design pure IDF-free `ss_spk_core.[ch]` (format validation + TX DMA/latency plan + pop-free open/close step sequence + Q15 gain), mirroring the `ss_audio_core`/`ss_mic.c` split · impl `firmware/components/ss_audio/src/{ss_spk_core.c,ss_spk.c}` implementing `ss_spk_open/write/close/mute/volume` from `ss_hal_audio.h`, capability-gated via `ss_hal_has_cap(SS_CAP_SPEAKER)`, no mux acquire (keeps the sidetone path open) · test host harness `test_ss_spk_core` (format accept/reject, plan math, open/close pop-free ordering, gain mapping), lite CI build; on-hardware playback + pop-free + sidetone pending a board · docs impl-site notes + changelog + engineering-log
- Deps: S-03-009 (ss_audio component + core/glue pattern + `ss_hal_audio.h` mic half — code merged in e7089e8; story IN_REVIEW pending bench, but the merged component is what the speaker half builds on); `ss_hal_audio.h` speaker contract (frozen in ss_hal); `SS_SPK_*` + `SS_CAP_SPEAKER` board macros; on-hardware ACs (playback, pop-free, sidetone) need an attached Lite board — story parks at IN_REVIEW until then

### S-03-011 — SX1262 SPI driver — init, config, TX/RX
As a firmware engineer I want the SX1262 SPI driver with init, config, and TX/RX so that LoRa is a working bearer on Lite.
- AC: passes vendor eval-board round-trip test at SF7..SF12; IRQ-driven TX/RX FIFO handling with no busy-wait; driver conforms to the HAL radio contract
- Meta: Shard=E | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=L | PRD=F-BR-01 | Const=C-00,C-01,C-08
- Deps: S-03-031 (GPIO0 CS arbitration), S-03-033 (GPIO45 radio-mode mux acquisition)

### S-03-012 — LoRa region PA table (US915, EU868, AU915, AS923)
As a compliance engineer I want the LoRa region PA table so that TX power and channels are legal in every shipped region.
- AC: US915, EU868, AU915, AS923 tables present; region selected at provisioning locks the table; TX power never exceeds regional limit in a test sweep
- Meta: Shard=E | Type=Feature | Size=M | Prio=P0 | Status=IN_REVIEW | SKU=L | PRD=NF-REG-01,NF-REG-02,NF-REG-05 | Const=C-00,C-08
- Tasks: spec per-region max-EIRP + frequency-plan bounds for US915/EU868/AU915/AS923 (LoRaWAN RP002 + FCC 47 CFR §15.247 / ETSI EN 300 220 / ARIB STD-T108) + one-shot region-latch semantics · design pure host-testable contract `ss_lora_pa_core.h` (no ESP-IDF/HAL) · impl region-table lookup + never-exceed EIRP clamp + in-band frequency validation + one-shot region latch in `ss_lora_pa_core.c` · test host harness `test_ss_lora_pa_core` (ASan/UBSan) with a TX-power sweep across all regions proving the clamp never exceeds the regional limit · docs contract doc-block + ENGINEERING_LOG + CHANGELOG + host-tests CI wiring
- Deps: S-03-011 (SX1262 driver — on-target consumer that applies the clamped EIRP to the PA register and runs the physical RF sweep; NOT DONE, so the on-hardware sweep AC parks with it — the host sweep proves the policy now); region persistence deferred to sealed NVS (S-03-043, EPIC-08); external: LoRaWAN Regional Parameters RP002-1.0.4, FCC 47 CFR §15.247, ETSI EN 300 220-2, ARIB STD-T108 (AS923)

### S-03-013 — LBT (Listen-Before-Talk) + duty-cycle guard
As a compliance engineer I want listen-before-talk and a duty-cycle guard so that EU 868 duty limits and channel etiquette are enforced in software.
- AC: LBT backs off on a busy channel; EU868 duty-cycle accounting blocks over-limit TX; SOS override path documented and bounded
- Meta: Shard=E | Type=Feature | Size=M | Prio=P0 | Status=IN_REVIEW | SKU=L | PRD=NF-REG-05,F-BR-01 | Const=C-00,C-08
- Tasks: spec EU868 sub-band duty limits (ETSI EN 300 220) + LBT/CAD backoff + bounded SOS override · design pure host-testable contract `ss_lora_lbt_core.h` · impl duty accounting + busy-channel backoff + SOS override in `ss_lora_lbt_core.c` · test host harness `test_ss_lora_lbt_core` (ASan/UBSan) · docs contract doc-block + ENGINEERING_LOG + CHANGELOG + host-tests CI wiring
- Deps: S-03-011 (LoRa driver — on-target integration consumer that feeds RSSI/CAD samples and gates ss_lora_tx; NOT DONE, so on-target ACs land with it); external: ETSI EN 300 220 duty-cycle sub-band table

### S-03-014 — Wi-Fi STA scan + connect + WPA2/3
As a device owner I want Wi-Fi STA scan/connect with WPA2/3 so that my pager uses home Wi-Fi whenever it is available.
- AC: scan returns RSSI-sorted results; connect succeeds to WPA2 and WPA3 APs; disconnect/reconnect handled with bounded retry
- Meta: Shard=F | Type=Feature | Size=M | Prio=P0 | Status=IN_REVIEW | SKU=L | PRD=F-BR-02 | Const=C-00,C-08
- Tasks: spec RSSI-sorted scan ordering + WPA2/WPA3 auth-mode selection policy + bounded disconnect/reconnect backoff · design pure host-testable contract `ss_wifi_sta_core.h` (no ESP-IDF) · impl stable RSSI-descending sort + auth-mode selection + bounded-retry backoff/give-up state machine in `ss_wifi_sta_core.c`, plus ESP-IDF glue `ss_wifi.c` implementing the frozen `ss_hal_radio_wifi.h` lifecycle and wiring esp_wifi scan/connect · test host harness `test_ss_wifi_sta_core` (ASan/UBSan) · docs contract doc-block + ENGINEERING_LOG + CHANGELOG + host-tests CI wiring
- Deps: `ss_hal_radio_wifi.h` lifecycle contract (already frozen in ss_hal — init/config/start/stop/sleep); scan + bounded-retry live in the new `ss_wifi` component's own core so `ss_hal/**` stays untouched (keeps the story T3, not the T1 HAL path); on-hardware ACs (WPA2/WPA3 connect to a live AP) need an attached Lite board + Wi-Fi HIL rack (EPIC-03 exit criterion 2) — story parks at IN_REVIEW until then; external: IEEE 802.11 + WPA3-SAE via the esp_wifi supplicant (ESP-IDF v5.3.5)

### S-03-015 — Wi-Fi soft-AP for captive provisioning
As a device owner I want a Wi-Fi soft-AP captive provisioning mode so that first-boot setup works without any companion infrastructure.
- AC: soft-AP with captive portal reachable from a phone; provisioning completes credential handoff; soft-AP shuts down after provisioning completes
- Meta: Shard=F | Type=Feature | Size=M | Prio=P0 | Status=IN_REVIEW | SKU=L | PRD=F-BR-02,F-APP-07 | Const=C-00,C-08
- Tasks: spec never-open soft-AP + per-session-passphrase credential-handoff security contract (doc 05 §10.3 standalone path, this story's T1 deliverable) · design pure host-testable session core `ss_wifi_prov_core.h` (no ESP-IDF) · impl passphrase/verify-code generation (fail-closed TRNG, rejection sampling) + credential validation + bounded session state machine in `ss_wifi_prov_core.c`, plus ESP-IDF portal glue `ss_wifi_prov.c` (AP-mode soft-AP, DNS catch-all, HTTP portal, teardown-on-terminal) · test host harness `test_ss_wifi_prov_core` (ASan/UBSan) · docs doc-05 §10.3 amendment + contract doc-block + ENGINEERING_LOG + CHANGELOG + host-tests CI wiring
- Deps: S-03-014 (STA consumer of the handed-off `ss_wifi_cfg_t`); sealed at-rest credential persistence deferred to S-03-043 (FS_key, EPIC-08); on-target ACs (phone reachability, captive-portal detection, WPA2/WPA3-SAE soft-AP association) need the Wi-Fi HIL rack (EPIC-03 exit criterion 2) — story parks at IN_REVIEW; external: IEEE 802.11 WPA2/WPA3 transition soft-AP via esp_wifi + esp_http_server (ESP-IDF v5.3.5)

### S-03-016 — BLE 5 GATT server + advertising profile
As a companion-app developer I want a BLE 5 GATT server and advertising profile so that phones can pair with and talk to the device.
- AC: GATT services for pairing, provisioning, and companion link exposed; advertising intervals meet the power budget; MTU negotiation ≥ 247 bytes verified
- Meta: Shard=G | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=L | PRD=F-BR-03,F-APP-07 | Const=C-00,C-08

### S-03-017 — BLE bonding + LTK storage in secure NVS
As a security engineer I want BLE bonding with LTK storage in secure NVS so that paired-phone keys never rest in plaintext.
- AC: bonding survives reboot; LTKs stored only in encrypted NVS; re-pair flow wipes stale bonds
- Meta: Shard=G | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=L | PRD=F-BR-03,NF-SEC-02 | Const=C-05,C-08

### S-03-018 — Flash partition layout reconciliation (frozen map + keys/fs homes)
As a firmware engineer I want the frozen partition map reconciled with the remaining storage decisions so that keys, logs, and user data have safe, fixed homes without re-inventing the layout.
- AC: the frozen `firmware/partitions.csv` (nvs/otadata/phy_init/ota_0/ota_1/storage/coredump — S-02-008, RFC-0003 freeze note) is affirmed as the single source of truth; the keys home (dedicated partition vs NVS-encrypted namespace, EPIC-08 input) is decided and recorded by decision entry; logs/FS mapping onto `storage` (LittleFS, S-03-019) is documented; any layout change follows the partitions.csv freeze procedure
- Meta: Shard=H | Type=Feature | Size=S | Prio=P0 | Status=DRAFT | SKU=L | PRD=— | Const=C-00,C-05
- Deps: retargeted 2026-07-08 (triage): original text predated the EPIC-02 partition freeze; S-02-008, S-03-019

### S-03-019 — LittleFS on user partition
As a firmware engineer I want LittleFS on the user partition so that file storage is power-fail safe.
- AC: mount/format/self-heal verified; power-cut torture test passes without corruption; wear-levelling stats exposed via diagnostics
- Meta: Shard=H | Type=Feature | Size=S | Prio=P0 | Status=IN_REVIEW | SKU=L | PRD=— | Const=C-00
- Tasks: spec ss_storage INTERNAL_FS mount/self-heal FSM + wear-stat surface against ss_hal_storage.h (partition label "storage") · design pure host-testable core (mount→format→remount self-heal FSM + wear-stat aggregation) split from esp_littlefs glue · impl new ss_storage component: core + joltwallet/littlefs glue registering on label "storage", plaintext (TODO EPIC-08 encryption) · test gcc host harness for FSM + wear-stat formatting under ASan/UBSan; on-target power-cut torture deferred to S-02-015 hardware gate · docs diagnostics wear-stat surface + engineering-log + changelog
- Deps: S-02-008 (storage partition frozen, DONE); S-02-015 (on-target Unity harness — for the deferred power-cut torture test; IN_PROGRESS)

### S-03-020 — RGB LED driver + status patterns
As a device owner I want RGB status LED patterns so that link, activity, and SOS states are visible at a glance.
- AC: link, activity, and SOS-breathe patterns implemented; higher-priority patterns preempt lower ones; brightness respects the active power profile
- Meta: Shard=I | Type=Feature | Size=S | Prio=P1 | Status=DROPPED | SKU=L | PRD=F-MSG-08 | Const=C-00,C-01
- Deps: DROPPED for Lite 2026-07-08 (triage): LED footprint unpopulated on Lite (`SS_LED_PIN -1`, C-01); display/buzzer carry status — capability moved intact to S-04-029 (Alpha bezel LEDs)

### S-03-021 — Internal temperature sensor + thermal throttle policy
As a firmware engineer I want the internal temperature sensor with a thermal-throttle policy so that the device protects itself under heat.
- AC: temperature readable via the HAL sensor API; throttle thresholds reduce TX power and backlight; hysteresis prevents oscillation
- Meta: Shard=J | Type=Feature | Size=S | Prio=P1 | Status=DRAFT | SKU=L | PRD=— | Const=C-00,C-01

### S-03-022 — HAL conformance test vectors (host-run)
As a firmware engineer I want host-run HAL conformance test vectors so that any board implementation can be verified against the contracts.
- AC: vectors cover power, audio, LoRa, Wi-Fi, and BLE HAL contracts; runnable on host against mocks and on target; failures produce actionable diffs
- Meta: Shard=— | Type=Feature | Size=L | Prio=P0 | Status=IN_REVIEW | SKU=L | PRD=— | Const=C-00
- Tasks: spec conformance vector-set binding to the frozen HAL contract headers (power, audio, LoRa, Wi-Fi, BLE) — each vector = a HAL call sequence + expected observable state/return runnable in the S-02-014 `firmware/test/host` gtest harness against HAL mocks · design vector-set shape as a t2 frozen contract (per-domain data-driven case struct, mock-observation interface, actionable-diff formatter, on-target reuse shape) · impl mock HAL backends for the five domains + vector tables + a diff-printing conformance runner, wired into `firmware/test/host` CMake and `host-tests.yml` CI · test the harness self-check (a deliberately non-conforming mock produces the expected actionable diff) under gtest/ASan · docs conformance-vector authoring guide + engineering-log + changelog
- Deps: S-02-014 (host gtest baseline + mocked HAL headers, DONE); S-02-015 (on-target Unity harness — carries the "runnable on target" AC clause, itself IN_REVIEW pending a Lite board, so that clause defers with it); HAL contract headers frozen for the five domains (power S-03-001 DONE; audio S-03-009/S-03-010; LoRa S-03-011; Wi-Fi S-03-014/S-03-015; BLE `ss_hal_radio_ble.h`) — vectors bind to the headers and run against mocks, so real-driver completion is not required for host runs

### S-03-023 — HIL rack test-plan for Lite
As a test engineer I want a HIL rack test-plan for Lite so that every merge is validated on real hardware.
- AC: rack topology and fixtures documented; test matrix maps to EPIC-03 exit criteria; CI trigger and result reporting defined
- Meta: Shard=— | Type=Ops | Size=M | Prio=P0 | Status=DONE | SKU=L | PRD=— | Const=C-00
- Tasks: spec enumerate the EPIC-03 exit criteria (HAL impls · 5-domain conformance vectors · FCC 15.247 duty cycle · idle ≤25 mA / sleep ≤0.5 mA) and the observable each requires on real hardware · design rack topology (DUT fixture, host controller, instrumentation: RF-shield/attenuator path for LoRa, µA current path, Wi-Fi/BLE peer, serial-JTAG console) + a fixture inventory + a test-matrix schema binding each exit criterion and each of the 5 conformance domains (power/audio/LoRa/Wi-Fi/BLE) to a measurement/vector-run · impl author `docs/dev/HIL_TEST_PLAN_LITE.md` — topology diagram, fixture table, exit-criteria→test matrix, CI trigger (self-hosted runner label + `workflow_dispatch`/merge-gate), result-reporting format (JUnit/artifact + PASS/FAIL surfaced to the merge) · test lint-docs passes, all relative links + anchors resolve, matrix covers all 4 exit criteria + 5 domains (self-check) · docs engineering-log + changelog
- Deps: S-03-022 (host-run conformance vectors the rack executes on-target — IN_REVIEW; plan references the frozen vector set but does not require it DONE to author); S-03-003/BENCH_POWER_LITE.md (sibling µA bench procedure the current-draw fixture reuses — IN_REVIEW, itself pending the D-0013 session); external: D-0013 Lite hardware fleet (not yet built — on-hardware execution of the matrix defers to that session, so this story lands IN_REVIEW not DONE)

### S-03-024 — Power-consumption measurement + budget report
As a firmware engineer I want a power-consumption measurement and budget report so that the Lite battery targets are provable.
- AC: sleep ≤ 0.5 mA; RX-idle ≤ 25 mA; TX peak documented per bearer
- Meta: Shard=A | Type=Feature | Size=M | Prio=P0 | Status=IN_REVIEW | SKU=L | PRD=NF-PWR-01 | Const=C-00,C-01
- Tasks: spec — full Lite power model (per-state draw: sleep/RX-idle/per-bearer TX/active) + the NF-PWR-01 ≥24 h standby roll-up · design — `POWER_BUDGET_LITE.md`: budget table, per-bearer TX-peak table (datasheet + measured columns), RX-idle measurement procedure reusing the PPK2 series fixture, standby-hours estimate from a duty-cycle model · impl — author `docs/dev/POWER_BUDGET_LITE.md` with datasheet TX figures (SX1262 / ESP32-S3 Wi-Fi / ESP32-S3 BLE), cross-refs to `BENCH_POWER_LITE.md` (sleep) + `HIL_TEST_PLAN_LITE.md` (rack) · test — docs-lint + index --check green; internal links resolve · docs — CHANGELOG + ENGINEERING_LOG
- Deps: S-03-001, S-03-003, S-03-030 (S-03-003 is IN_REVIEW — it parks on the same D-0013 hardware session; the budget report + datasheet TX figures are authorable now, and the measured sleep/RX-idle cells fill alongside the sleep bench at that session)

### S-03-025 — Wi-Fi/BLE coexistence stress test
As a firmware engineer I want a Wi-Fi/BLE coexistence stress test so that shared-antenna scheduling never starves either radio.
- AC: concurrent BLE link + Wi-Fi throughput soak passes; BLE connection maintained under sustained Wi-Fi load; packet-loss thresholds defined and met
- Meta: Shard=F,G | Type=Ops | Size=M | Prio=P0 | Status=IN_REVIEW | SKU=L | PRD=F-BR-02,F-BR-03 | Const=C-00,C-08
- Tasks: spec enumerate the shared-2.4 GHz coexistence observables (BLE link stays connected — no unexpected GAP disconnect — under sustained Wi-Fi TX/RX; Wi-Fi throughput holds a floor; bidirectional packet-loss stays under threshold for each radio) and the ESP-IDF software-coex config the single antenna requires (`CONFIG_ESP_COEX_SW_COEXIST_ENABLE`, coex preference/priority) · design the soak topology (DUT + co-located Wi-Fi AP/iperf peer + BLE central peer) reusing the S-03-023 HIL rack fixtures, the load profile (Wi-Fi iperf TCP/UDP soak duration + BLE notify/keep-alive cadence), and the pass/fail schema (BLE disconnect count, Wi-Fi throughput floor, per-radio packet-loss %) · impl author `docs/dev/COEX_STRESS_LITE.md` — coex config, soak topology, load profile, threshold table, empty results table (fills at D-0013), non-blocking HIL CI wiring · test lint-docs passes, all relative links + anchors resolve, thresholds cover both radios (self-check) · docs engineering-log + changelog
- Deps: S-03-023/`HIL_TEST_PLAN_LITE.md` (reuses the rack topology + Wi-Fi AP/BLE central peer fixtures — DONE); S-03-014/S-03-015 (Wi-Fi STA connect + reconnect the soak drives — IN_REVIEW); S-03-016/S-03-017 (BLE GATT link-under-load observable — DRAFT, referenced not required to author the plan); external: D-0013 Lite hardware fleet (soak execution + measured packet-loss/throughput defer to that session, so this story lands IN_REVIEW not DONE)

### S-03-026 — HaLow module bring-up on the Lite wireless header (SPI)
As a firmware engineer I want the Elecrow HaLow module brought up on Lite's wireless header so that the D-0013 dev fleet's primary long-range bearer works on the shipping board.
- AC: HaLow host driver initializes over the wireless-header SPI set (0/10/9/3, RST 2, IRQ 1, mux GPIO45=LOW per C-01 §3 table) with `SS_CAP_RADIO_HALOW` replacing `SS_CAP_RADIO_LORA` under `CONFIG_SS_LITE_MOD_HALOW`; SPI init defers to the S-02-016 recovery-window ownership of GPIO0; association + data-path smoke passes between the two D-0013 dev units; module part/stack pin recorded (Lite SPI path is distinct from Alpha's MM8108-SDIO stories S-04-003/023)
- Meta: Shard=— | Type=Feature | Size=L | Prio=P0 | Status=BLOCKED | SKU=L | PRD=— | Const=C-00,C-01,C-08
- Tasks: spec (blocked on contract decision) · design/impl/test/docs pending S-03-046 outcome
- Deps: S-03-046 (HaLow HAL contract reconciliation — BLOCKED 2026-07-09: frozen `ss_hal_radio_halow.h` says "absent on Lite"/MM8108-only, contradicting this story's AC; T3 worker correctly stopped per doc 10 §8), S-02-016 (recovery-window GPIO0 ownership — soft dep, IN_REVIEW); smoke clause additionally needs the two D-0013 dev units

### S-03-027 — GNSS HAL driver (`hal_gnss`) — UART1 NMEA behind the HAL contract
As a firmware engineer I want the GNSS module driven through `hal_gnss` so that location leaves the bring-up scaffolding and binds to the HAL contract.
- AC: `hal_gnss` implements the `ss_hal_gnss.h` contract on UART1 (18/17 @9600, DMA ring buffer, NMEA-0183 parse per C-01 §HAL map); the `ss_uart_engine` GNSS path migrates behind it with no fix-loss regression; exact module part number confirmed at attachment and recorded per the D-0013 clause (any deviation updates doc 01 + pin map first); fix acquisition verified on a dev-fleet unit
- Meta: Shard=— | Type=Feature | Size=M | Prio=P0 | Status=IN_REVIEW | SKU=L | PRD=— | Const=C-00,C-01
- Tasks: spec — NMEA-0183 line framer + minimal RMC/GGA field parse (`$…*hh` XOR checksum, `ddmm.mmmm`→signed decimal degrees, knots→m/s, in-place comma-field split) yielding `ss_gnss_fix_t`; UART1 config sourced from `board_config.h` `SS_UART_GNSS_*` (RX=18/TX=17 @9600, 2048-byte DMA ring), part-agnostic across any NMEA-0183 module per the D-0013 clause; `ss_hal_gnss.h` init/start/stop/get/sleep semantics · design — pure IDF-free `ss_gnss_core.[ch]` (sentence→fix decode, no `esp_err`) + thin IDF glue `ss_gnss.c` implementing frozen `ss_hal_gnss.h`, mirroring the `ss_power` core/glue split · impl — new `firmware/components/ss_gnss/{include/ss_gnss_core.h,src/ss_gnss_core.c,src/ss_gnss.c,CMakeLists.txt}`, capability-gated via `ss_hal_has_cap(SS_CAP_GNSS_L1)`; migrate `main/ss_uart_engine.cpp` GNSS parse helpers (`nmea_checksum_ok`/`nmea_coord`/`split_fields`/`gnss_handle_sentence`) to the shared core so a single parser feeds both paths with no fix-loss regression · test — host gtest `test_ss_gnss_core` (RMC/GGA vectors, checksum accept/reject, N/S/E/W coord conversion, knots→m/s, malformed/short sentences), lite CI build; on-hardware fix acquisition pending an attached Lite dev-fleet unit · docs — impl-site C-01 §HAL map citation + changelog + ENGINEERING_LOG; exact part number recorded per the D-0013 clause at attachment
- Deps: S-03-001 (DONE — `ss_power` core/glue pattern reused), `ss_hal_gnss.h` contract (already frozen in `ss_hal`), `ss_uart_engine` GNSS scaffolding (`main/ss_uart_engine.cpp` — parse migrated behind the shared core); on-hardware ACs (exact part number confirmed at attachment per the D-0013 clause; fix acquisition on a dev-fleet unit) need an attached Lite board / the D-0013 session — story parks at IN_REVIEW until then

### S-03-028 — Magnetometer/compass HAL driver (`hal_imu` mag path)
As a firmware engineer I want the 3-axis compass driven through the HAL so that heading is a contract-backed capability, not bring-up code.
- AC: mag driver implements the HAL contract on I²C0 (15/16) at the C-01 roster address (HMC5883L @0x1E as documented — **actual Elecrow module part verified at attachment**: a QMC5883L variant sits at 0x0D and changes the driver; any deviation updates doc 01 + pin map FIRST per D-0013); `ss_compass` migrates behind the HAL preserving the three documented behaviours (tilt-compensated with IMU, flat-hold mag-only, phone-fed with neither); heading sanity-checked on a dev-fleet unit
- Meta: Shard=— | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=L | PRD=— | Const=C-00,C-01

### S-03-029 — ESP32-C6 coprocessor link bring-up (UART2 framed transport)
As a firmware engineer I want the optional C6 mesh-coprocessor link brought up behind a HAL contract so that the coproc becomes a usable bearer/offload path.
- AC: framed CRC transport on UART2 (44/43 @115200 per C-01; `CONFIG_SS_LITE_MOD_COPROC_C6`, `SS_CAP_COPROC`) formalized behind a HAL contract with version negotiation per RFC-0003; `ss_uart_engine` coproc pump migrates behind it; link-up/echo smoke passes against a C6 dev module; physical port assignment confirmed at attachment per the D-0013 clause
- Meta: Shard=— | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=L | PRD=— | Const=C-00,C-01,C-02

### S-03-030 — `ss_hal_power.h` timer/periodic-wake contract surface
As a firmware engineer I want the frozen power contract extended with a timer-wake surface so that RTC-timer wakeups (NF-PWR-01 periodic LoRa duty) are a contract capability, not an ad-hoc syscall.
- AC: `ss_hal_power.h` gains a timer-wake API (e.g. `ss_power_wake_timer_set(uint64_t us)` + clear semantics for light vs deep sleep) designed and double-reviewed per the T1 pipeline; `ss_power` implements it (`esp_sleep_enable_timer_wakeup` glue) with the pure decision core host-tested; C-01 §4.3's wake-source row is reconciled with `board_config.h` (touch INT / LoRa DIO1 / RTC timer) in the same change; 3-board CI green
- Meta: Shard=A | Type=Feature | Size=S | Prio=P0 | Status=DONE | SKU=L | PRD=NF-PWR-01 | Const=C-00,C-01
- Tasks: spec timer-wake semantics vs S3 sleep modes (light/deep/hibernate/shutdown) · design set/clear surface — sleep-entry countdown, sticky-until-clear, 30-day cap · impl ss_hal_power.h + ss_power core/glue · test 9 PowerTimer host cases, 3-board CI · docs C-01 §4.3 wake row + board_config comments + log
- Deps: S-03-001


### S-03-031 — GPIO0 runtime arbitration (recovery watcher / BOOT input / LoRa CS)
As a firmware engineer I want GPIO0's three consumers arbitrated explicitly so that the BOOT input line and LoRa SPI chip-select never fight at runtime.
- AC: an ownership policy for GPIO0 is designed and documented (sequence today: S-02-016 recovery watcher during the entry window → ss_input BOOT button → SX1262 CS when LoRa is active) resolving the runtime conflict by explicit ruling (mux strategy, LoRa-active-drops-BOOT-input, or strap-only BOOT role); ss_input and the S-03-011 LoRa driver both honor the ruling with a host-tested arbitration decision core; C-01 pin-sharing table updated in lockstep; 3-board CI green
- Meta: Shard=— | Type=Feature | Size=S | Prio=P0 | Status=DRAFT | SKU=L | PRD=— | Const=C-00,C-01

### S-03-032 — ss_hal ABI baseline: extern-C guards + 64-bit caps accessors
As a firmware engineer I want the HAL headers C/C++-ABI-safe and the capability accessors implemented so that HAL contracts link correctly from every consumer.
- AC: all ss_hal public headers carry extern "C" guards; `ss_hal_has_cap` takes `uint64_t` (caps reach bit 38 — the uint32_t signature truncated and mis-evaluated high caps); `ss_hal.c` implements `ss_hal_has_cap`/`ss_hal_caps_mask`/`ss_hal_board_id` from board_config; both T1 review passes approve; 3-board CI green (fixes the f68e7d2 link failure)
- Meta: Shard=— | Type=Feature | Size=S | Prio=P0 | Status=DONE | SKU=★ | PRD=— | Const=C-00,C-01
- Tasks: spec defect pair from the link failure · design guards pattern + widened signature + header-driven impl · impl 24 headers + ss_hal.c + CMakeLists · test host suite green, 3-board CI green · docs rules/log
- Deps: —

### S-03-033 — GPIO45 mic/radio mux implementation (`ss_muxctl`)
As a firmware engineer I want the declared mux-arbitration contract implemented so that the mic and the wireless-header radio can never drive the shared GPIO 9/3/10 set simultaneously.
- AC: `ss_mux_init/acquire/release` (frozen `ss_hal_muxctl.h`) implemented in a new component-local `ss_muxctl` (core/glue split; `ss_hal/**` untouched): GPIO45 driven per C-01 (LOW = radio, mic mode per mux table), owner-token + mutex arbitration with busy rejection and documented owner semantics; pure decision core host-tested (acquire/release/contention/double-release matrix); portable no-op behaviour on boards without the mux (per the header's contract); 3-board CI green
- Meta: Shard=D | Type=Feature | Size=S | Prio=P0 | Status=DONE | SKU=L | PRD=— | Const=C-00,C-01
- Tasks: spec core state-machine + owner semantics · design core/glue split (pure decision core + FreeRTOS token/mutex glue) · impl `ss_muxctl_core.c` + `ss_muxctl.c` glue implementing frozen `ss_hal_muxctl.h` · test host gtest acquire/release/contention/double-release matrix · docs eng-log learnings
- Deps: — (frozen `ss_hal_muxctl.h` + `ss_hal_types.h` mux enums + `ss_hal_caps.h` `SS_CAP_MUX_MIC_RADIO` + board_config `SS_MUX_*` macros already in tree)


### S-03-034 — `ss_hal_init`/`ss_hal_shutdown` boot sequencer
As a firmware engineer I want the HAL lifecycle aggregator implemented so that main boots subsystems through one contract call instead of direct module calls.
- AC: `ss_hal_init` sequences every enabled subsystem per the C-01 §7 Lite order (capability-gated, per-driver errors reported not swallowed); `ss_hal_shutdown` reverses for deep-sleep prep; main.cpp's TODO(EPIC-03) direct calls migrate behind it incrementally as drivers land; pure sequencing core host-tested; 3-board CI green
- Meta: Shard=— | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=— | Const=C-00,C-01

### S-03-035 — Buzzer behind the HAL (`ss_buzzer_beep`)
As a firmware engineer I want the buzzer migrated behind its declared HAL surface so that audio feedback is contract-driven, not ss_diag-local.
- AC: `ss_buzzer_beep` (frozen ss_hal_audio.h) implemented over the GPIO8 LEDC path; ss_diag migrates to the HAL call; degrades via caps on boards without a buzzer; 3-board CI green
- Meta: Shard=D | Type=Feature | Size=XS | Prio=P2 | Status=DRAFT | SKU=L | PRD=— | Const=C-00,C-01

### S-03-036 — Lite 6-axis IMU HAL path (`ss_imu_init/read/sleep`)
As a firmware engineer I want the optional 6-axis IMU driven through the HAL so that tilt-compensated heading gets contract-backed accel data.
- AC: `ss_imu_init/read/sleep` implemented for the C-01 optional IMU module (I²C0 @0x68, `CONFIG_SS_LITE_MOD_IMU`), accel+gyro fields populated; ss_compass consumes the HAL read for tilt compensation; exact module part confirmed at attachment per D-0013; host-tested core; 3-board CI green
- Meta: Shard=— | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=L | PRD=— | Const=C-00,C-01

### S-03-037 — Storage HAL aggregator + SD + MODELS kinds
As a firmware engineer I want the remaining ss_hal_storage.h surface implemented so that every declared storage kind has a real path.
- AC: `ss_storage_init`/`ss_storage_stat` aggregate the kinds; SS_STORAGE_SD implemented over the soft-SPI microSD (6/4/5/7, hot-remove tolerant); SS_STORAGE_MODELS maps its partition/mount point; INTERNAL_FS/NVS kinds delegate to the shipped S-03-019/S-02-017 paths; host-tested dispatch core; 3-board CI green
- Meta: Shard=H | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=L | PRD=— | Const=C-00,C-01
- Deps: S-02-017, S-03-019

### S-03-038 — USB CDC data channel (`ss_usb_cdc_*`)
As a firmware engineer I want the declared USB CDC surface implemented so that the companion tether has a wired data path distinct from the debug console.
- AC: `ss_usb_cdc_*` (frozen ss_hal_usb.h) implemented coexisting with the USB-Serial-JTAG console; framed read/write with host-tested framing core; enumeration smoke on a dev unit at the D-0013 hardware session; 3-board CI green
- Meta: Shard=— | Type=Feature | Size=M | Prio=P2 | Status=DRAFT | SKU=L | PRD=— | Const=C-00,C-01

### S-03-039 — Watchdog HAL surface reconciliation (`ss_wdt_*` vs `ss_task_wdt_*`)
As a firmware engineer I want the two watchdog surfaces reconciled so that exactly one contract exists.
- AC: decision recorded (adopt the shipped `ss_task_wdt_*` semantics into the frozen header, or implement `ss_wdt_*` as the canonical wrapper) via the T1 pipeline since ss_hal_watchdog.h changes are a T1 path; the losing surface is deprecated with a migration note, never silently dropped; ss_tasks call sites updated if the contract wins; 3-board CI green
- Meta: Shard=F | Type=Feature | Size=S | Prio=P1 | Status=DONE | SKU=★ | PRD=— | Const=C-00
- Tasks: spec decision record — shipped `ss_task_wdt_*` semantics win; no runtime init by design (TWDT deadline/panic are sdkconfig policy constants) · design contract-of-record header layout: canonical decls + `ss_wdt_*` tombstones (`deprecated`+GCC `error` attrs — hard compile error on any call; IDF defaults soften deprecated-only to a warning) with per-symbol migration notes · impl adopt semantics into `ss_hal_watchdog.h`, move wrappers unchanged to `ss_wdt.c` (contract-audit sees `*.c` only), `ss_tasks.h` re-points at the contract, ownership wildcard → 4 tombstone entries · test behavior unchanged (on-target crash tests stay `test_ss_wdt.c`); gates contract-audit/task-policy/board-parity + 3-board CI · docs DEPRECATIONS.md row + CHANGELOG (Changed/Deprecated) + ENGINEERING_LOG decision entry
- Deps: S-02-006 (ss_tasks home), S-02-009 (shipped semantics adopted), S-02-025 (contract-audit gate)

### S-03-040 — `ss_rng` base plumbing (Lite)
As a firmware engineer I want the base RNG surface implemented so that entropy consumers have a contract path before the EPIC-06 DRBG work.
- AC: `ss_rng_init`/`ss_rng_bytes` implemented over the SoC TRNG (`esp_fill_random`), documented as raw-TRNG-until-S-06-008 (which owns healthcheck + DRBG reseed and supersedes nothing here); never returns predictable output on RNG failure — fails loudly; 3-board CI green
- Meta: Shard=— | Type=Feature | Size=XS | Prio=P1 | Status=DRAFT | SKU=L | PRD=— | Const=C-00,C-05
- Deps: S-06-008 owns the DRBG/healthcheck layer above this

### S-03-041 — Cross-reboot duty-cycle persistence determination
As a compliance owner I want a determination on reboot-durable duty-cycle budgets so that EU868 accounting is regulator-correct, not merely session-correct.
- AC: a written determination (with EN 300 220 citation) on whether the rolling 1 h duty window must survive reboot; if yes, the S-03-013 core gains a persistence hook via `ss_nvs` (S-02-017) designed to never let persistence failure block SOS; if no, the in-RAM limitation note in the LBT header is marked compliance-approved
- Meta: Shard=E | Type=Task | Size=S | Prio=P2 | Status=DRAFT | SKU=L | PRD=— | Const=C-00,C-08
- Deps: S-03-013

### S-03-042 — Wi-Fi reconnect off the event-loop task
As a firmware engineer I want the reconnect backoff moved off the default event loop so that a long backoff never stalls unrelated event delivery.
- AC: the S-03-014 disconnect handler arms an esp_timer (or dedicated task) instead of blocking the event loop with vTaskDelay; backoff/give-up semantics unchanged (host suite still green); verified before/with the Wi-Fi HIL rack (S-03-023)
- Meta: Shard=F | Type=Feature | Size=XS | Prio=P2 | Status=DRAFT | SKU=L | PRD=— | Const=C-00
- Deps: S-03-014

### S-03-043 — Sealed at-rest Wi-Fi credential persistence
As a security engineer I want handed-off Wi-Fi credentials persisted sealed under `FS_key` so that home-network PSKs survive reboot without ever resting in plaintext (doc 05 §10.3).
- AC: a credential store seals the S-03-015 handoff output under `FS_key` (EPIC-08) and feeds STA auto-join on boot; plaintext never touches flash (doc 05 §10.3 "never stores Wi-Fi PSKs plaintext" holds end-to-end); store failure degrades to re-provisioning, never to plaintext fallback; wipe-on-command supported
- Meta: Shard=F | Type=Feature | Size=S | Prio=P2 | Status=DRAFT | SKU=L | PRD=F-APP-07 | Const=C-00,C-05
- Deps: S-03-015 (handoff producer); EPIC-08 FS_key store (blocking — no sealed storage exists before it)

### S-03-044 — HIL CI workflow wiring (self-hosted `[hil, lite]` runner)
As a test engineer I want the HIL CI workflow implemented so that the plan defined by S-03-023 is executed automatically once the fleet is online.
- AC: `.github/workflows/hil-lite.yml` created with `workflow_dispatch` + merges touching `firmware/**` triggers on the self-hosted `[hil, lite]` runner (per `HIL_TEST_PLAN_LITE.md` §5); runs the S-03-022 conformance suite against real HAL drivers via the S-02-015 Unity adapter; emits per-domain JUnit XML + evidence artifacts; empty-diff-across-5-domains is the merge-gate pass condition; job is a no-op (documented) until the fleet is registered
- Meta: Shard=I | Type=Ops | Size=S | Prio=P0 | Status=DRAFT | SKU=L | PRD=— | Const=C-00
- Deps: S-03-022 (vectors), S-03-023 (plan), S-02-015 (Unity target adapter); blocked in practice on D-0013 fleet registration before the runner produces useful signal

### S-03-045 — HIL matrix execution on D-0013 fleet (fills `HIL_TEST_PLAN_LITE.md` §4.1)
As a test engineer I want the full HIL matrix executed on D-0013 boards so that EPIC-03's on-hardware exit criteria are objectively met, not just documented.
- AC: the full 4-exit-criteria × 5-conformance-domain matrix from `HIL_TEST_PLAN_LITE.md` §4 is run on D-0013 fleet Lite units; §4.1 results table populated with real measurements + timestamps + firmware revs; per-domain JUnit XML archived as evidence; any exit-criterion failure blocks EPIC-03 exit (does not silently downgrade)
- Meta: Shard=I | Type=Ops | Size=M | Prio=P0 | Status=DRAFT | SKU=L | PRD=— | Const=C-00
- Deps: S-03-022 (vectors), S-03-023 (plan), S-03-044 (workflow), S-02-015 (Unity rig); blocking hardware event: D-0013 fleet online

### S-03-046 — HaLow HAL contract reconciliation (Lite Elecrow-SPI vs MM8108-only framing)
As a platform engineer I want `ss_hal_radio_halow.h` reconciled with the ratified Lite hardware so that the Lite HaLow driver (S-03-026) builds against a contract that admits it exists.
- AC: header prose ("via Morse Micro MM8108", "absent on Lite") reconciled with C-01 §3 (Elecrow HaLow module on Lite's wireless-header SPI); decision recorded whether the header stays the single silicon-neutral HaLow contract (transport = implementation detail) or Lite gets a distinct contract; silicon-specific field framing (MCS 0..10, BW up to 16 MHz comments) generalized or explicitly bounded per-silicon via caps query; ranges that depend on the unconfirmed Elecrow part are marked TBD-D-0013, not guessed; decision + rationale in ENGINEERING_LOG (+ DEPRECATIONS/CHANGELOG if any symbol changes)
- Meta: Shard=— | Type=Task | Size=M | Prio=P0 | Status=DRAFT | SKU=L | PRD=— | Const=C-00,C-01,C-08
- Deps: — (part/stack confirmation at D-0013 may refine numeric ranges; the reconciliation itself must not wait on it). Filed 2026-07-09 from the S-03-026 stop-and-escalate (frozen-contract contradiction, doc 10 §8).
