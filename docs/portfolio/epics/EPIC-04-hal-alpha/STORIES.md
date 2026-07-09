<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# EPIC-04 — Stories

Format per `../../00_METHODOLOGY.md` §2.7. Meta lines are machine-parsed.

---

### S-04-001 — ESP32-P4 DDR PSRAM init + cache tuning
As a firmware engineer I want ESP32-P4 DDR PSRAM init and cache tuning so that Alpha has the memory bandwidth its UI and radios need.
- AC: PSRAM initialised at target frequency; cache configuration benchmarked and documented; full-range memory test passes at temperature extremes
- Meta: Shard=A | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=A | PRD=— | Const=C-00

### S-04-002 — Dual-core RV32 SMP FreeRTOS
As a firmware engineer I want SMP FreeRTOS on the dual-core RV32 so that workloads spread across both cores.
- AC: SMP scheduler runs tasks on both cores; core-affinity API verified by test; EPIC-02 baseline tests pass under SMP
- Meta: Shard=A | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=A | PRD=— | Const=C-00

### S-04-003 — MM8108 HaLow bring-up over SDIO
As a firmware engineer I want MM8108 HaLow bring-up over SDIO so that HaLow becomes a usable bearer on Alpha.
- AC: SDIO enumeration and radio firmware load succeed; Morse OSAL glue passes the vendor sanity suite; TX/RX round-trip with a reference AP verified
- Meta: Shard=B | Type=Feature | Size=XL | Prio=P0 | Status=DRAFT | SKU=A | PRD=F-BR-04 | Const=C-00,C-08

### S-04-004 — HaLow WPA3-SAE association
As a device owner I want HaLow WPA3-SAE association so that long-range links are authenticated and encrypted.
- AC: SAE handshake completes with a reference AP; wrong-password and retry paths handled without lockup; reassociation after AP reboot verified
- Meta: Shard=B | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=A | PRD=F-BR-04 | Const=C-00,C-08

### S-04-005 — HaLow scan + channel-plan enforcement
As a compliance engineer I want HaLow scan with channel-plan enforcement so that the radio never operates outside its region plan.
- AC: scan restricted to the region channel plan; out-of-plan channels rejected at the API level; region mismatch disables HaLow per the first-boot rule
- Meta: Shard=B,H | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=A | PRD=F-BR-04,NF-REG-04 | Const=C-08

### S-04-006 — SKY66423 FEM gain / mode control GPIOs
As a firmware engineer I want SKY66423 FEM gain and mode control so that HaLow TX/RX performance meets link-budget targets.
- AC: TX gain and RX LNA modes controlled via GPIO per datasheet timing; T/R switch timing validated on scope; FEM state tied to the radio state machine
- Meta: Shard=C | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=A | PRD=F-BR-04 | Const=C-00,C-08
- Deps: external: corrected 2026-07-09 (D-0021, re-verified per HW-repo EXECUTION_READINESS_AUDIT BIT-1) — on the released Omega v69 board the SKY66423 FEM (U3) IS routed (control paths), but **no `pa_pdet`/VSWR sense net exists**; Omega FEM work is owned by EPIC-05 (S-05-029, rescoped to a composite VSWR proxy). This Alpha story waits only on the Alpha board itself, which is not release-verified (v14 "DO NOT FABRICATE", v15 unverified) — see `docs/dev/OMEGA_HW_BASELINE.md`

### S-04-007 — RTL8852BE Wi-Fi 5 driver integration
As a firmware engineer I want RTL8852BE Wi-Fi 5 driver integration so that Alpha gets high-throughput Wi-Fi.
- AC: driver probes over PCIe/SDIO; STA achieves ≥ 100 Mbps to a laptop AP; driver survives repeated suspend/resume cycles
- Meta: Shard=D | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=A | PRD=F-BR-05 | Const=C-00,C-08

### S-04-008 — Wi-Fi 5 concurrent STA + soft-AP
As a device owner I want concurrent Wi-Fi 5 STA + soft-AP so that the device can extend my network while staying online itself.
- AC: STA and soft-AP run concurrently without dropouts; per-SSID opt-in enforced; throughput split under load documented
- Meta: Shard=D | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=A | PRD=F-BR-05,F-HGW-06 | Const=C-08

### S-04-009 — u-blox GNSS UART driver + NMEA parse
As a device owner I want the u-blox GNSS driver with NMEA parsing so that location share and SOS carry a real fix.
- AC: UART NMEA parsed into position/velocity/time; cold fix < 60 s in open sky; fix quality and satellite count exposed to applications
- Meta: Shard=E | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=A | PRD=F-MSG-07 | Const=C-00

### S-04-010 — UBX binary protocol for RAWX (post-processing)
As a firmware engineer I want UBX RAWX binary support so that raw GNSS measurements are available for post-processing use cases.
- AC: UBX protocol framing implemented; RAWX messages logged to storage on demand; logs decode correctly in u-center
- Meta: Shard=E | Type=Feature | Size=M | Prio=P2 | Status=DRAFT | SKU=A | PRD=— | Const=C-00

### S-04-011 — LSM6DSV IMU init + motion IRQ
As a firmware engineer I want LSM6DSV IMU init with motion IRQ so that motion can wake the device cheaply.
- AC: I²C init and WHO_AM_I verified; motion IRQ wakes from sleep < 200 ms; sample rates configurable per power profile
- Meta: Shard=F | Type=Feature | Size=S | Prio=P0 | Status=DRAFT | SKU=A | PRD=— | Const=C-00

### S-04-012 — Sensor fusion (Madgwick) for orientation
As a device owner I want Madgwick sensor fusion so that the Seekie peer-pointer shows a stable heading.
- AC: orientation output stable within documented drift bounds; fusion runs within CPU budget; calibration flow documented
- Meta: Shard=F | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=A | PRD=F-APP-04 | Const=C-00

### S-04-013 — MIPI-DSI display driver
As a firmware engineer I want the MIPI-DSI display driver so that the larger Alpha panel renders `ss_ui` at full rate.
- AC: DSI link up at target resolution (320×480 or 480×640); frame rate and latency measured against NF-PERF budgets; `ss_ui` renders identical semantics as on Lite
- Meta: Shard=G | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=A | PRD=F-UI-01 | Const=C-00

### S-04-014 — Backlight PWM + ambient-light auto brightness
As a device owner I want ambient-light auto brightness so that the display is readable outdoors and frugal indoors.
- AC: ambient sensor sampled at bounded rate; brightness curve tuned and flicker-free; manual override persists across reboots
- Meta: Shard=G | Type=Feature | Size=S | Prio=P1 | Status=DRAFT | SKU=A | PRD=NF-PWR-02 | Const=C-00

### S-04-015 — HaLow region PA + channel table
As a compliance engineer I want the HaLow region PA and channel table so that TX stays within each region's rules.
- AC: US, EU, JP, KR, IN, AU tables present; wrong region code disables HaLow at first boot; table auto-fetched from the wg-legal repo with a drift check
- Meta: Shard=H | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=A | PRD=NF-REG-04 | Const=C-08

### S-04-016 — HaLow TX duty-cycle & TPC
As a compliance engineer I want HaLow TX duty-cycle limits and transmit power control so that emissions remain within certification limits.
- AC: duty-cycle accounting enforced per region; TPC reduces power on strong links; compliance sweep logged as certification evidence
- Meta: Shard=H | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=A | PRD=NF-REG-01,NF-REG-02,NF-REG-04 | Const=C-08

### S-04-017 — HaLow RSSI/LQI reporting to ss_link
As a firmware engineer I want HaLow RSSI/LQI reporting into `ss_link` so that bearer selection can rank HaLow accurately.
- AC: RSSI/LQI sampled per link and exposed via the HAL contract; values feed `ss_link` bearer scoring; update rate meets the bearer-switch latency budget
- Meta: Shard=B | Type=Feature | Size=S | Prio=P0 | Status=DRAFT | SKU=A | PRD=F-BR-04,NF-PERF-02 | Const=C-00,C-08

### S-04-018 — DSP wake-word inference (INT8) using P4 accelerator
As a device owner I want on-device wake-word inference on the P4 accelerator so that hands-free voice commands work offline.
- AC: INT8 wake-word model runs on the accelerator within the power budget; false-accept/false-reject rates measured and documented; inference latency documented in the design note
- Meta: Shard=I | Type=Feature | Size=L | Prio=P1 | Status=DRAFT | SKU=A | PRD=F-VOX-01 | Const=C-00

### S-04-019 — Alpha HIL rack test-plan
As a test engineer I want an Alpha HIL rack test-plan so that Alpha-specific radios and sensors are exercised on real hardware per merge.
- AC: rack covers HaLow, Wi-Fi 5, GNSS (record/replay), and IMU fixtures; test matrix maps to EPIC-04 exit criteria; CI trigger and result reporting defined
- Meta: Shard=— | Type=Ops | Size=M | Prio=P0 | Status=DRAFT | SKU=A | PRD=— | Const=C-00

### S-04-020 — Power-budget validation for Alpha (target ≤ 40 mA idle)
As a firmware engineer I want Alpha power-budget validation so that the 72 h standby target is provable.
- AC: idle ≤ 40 mA measured; HaLow doze and LoRa periodic profiles measured; power report published per firmware release
- Meta: Shard=— | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=A | PRD=NF-PWR-02 | Const=C-00

### S-04-021 — Antenna path S-parameter documentation
As a hardware engineer I want antenna path S-parameter documentation so that RF debugging and certification have a measured baseline.
- AC: S-parameters captured for each antenna path; measurement setup documented for reproducibility; files stored beside the board documentation
- Meta: Shard=C | Type=Task | Size=S | Prio=P1 | Status=DRAFT | SKU=A | PRD=— | Const=C-00

### S-04-022 — External antenna port safety interlock (Rx-only if mis-tuned)
As a compliance engineer I want an external-antenna safety interlock so that a mis-tuned antenna cannot cause out-of-spec transmissions.
- AC: mis-tune detection (VSWR/return-loss proxy) drops the path to Rx-only; interlock event logged and surfaced in diagnostics; behaviour verified with a deliberately detuned load
- Meta: Shard=C | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=A | PRD=NF-REG-01,NF-REG-02 | Const=C-00,C-08

### S-04-023 — HaLow host stack currency: ESP Component Registry pin + SDIO-vs-USB selection spike
As a firmware engineer I want the HaLow driver integration pinned to the supported `morsemicro/halow` ESP Component Registry package and the P4 host interface (SDIO 2.0 vs USB 2.0 HS) selected by measurement so that Alpha ships on a maintained stack at the interface that maximises throughput-per-watt.
- AC: dep-pin registry references `morsemicro/halow` from components.espressif.com at a digest-pinned version and bans the deprecated `mm-iot-esp32` repo; CI asserts the MM8108 (not default MM6108) configuration is active; spike measures SDIO 2.0 vs USB 2.0 HS on ESP32-P4 for sustained throughput, latency, and sleep/wake power including the WAKE/RESET/BUSY handshake, and the decision is recorded as an ADR in `governance/decisions.md`; FreeRTOS time-slicing requirement and host deep-sleep clock-drift mitigation (re-sync per S-02-018) are documented in the driver README; per D-HALOW-02/03 in `docs/portfolio/08_HALOW_TECHNOLOGY_DOSSIER.md`
- Meta: Shard=B | Type=Spike | Size=M | Prio=P0 | Status=DRAFT | SKU=A | PRD=— | Const=C-00,C-08

### S-04-024 — HaLow power-save profile: TWT + DTIM/snooze tuning, RAW grouping in AP mode
As a firmware engineer I want the 802.11ah power-save mechanisms (TWT, DTIM/snooze, RAW) explicitly engineered and measured so that "HaLow doze" in NF-PWR-02 is a named, tested configuration instead of a vendor default.
- AC: STA mode negotiates TWT with measured average current at three traffic profiles (idle beacon-only, periodic telemetry, active chat) published in the per-release power report (S-04-020); DTIM/snooze settings are tuned per QoS class with the latency trade-off documented (SOS/ALERT wake budget still meets NF-PERF-04); AP/HGW mode enables RAW grouping when associated-station count crosses a documented threshold and a dense-cell test shows no starvation of low-priority stations; profile settings are exposed via the bearer-policy vocabulary so fleet policy (S-16-026) can tune them; per D-HALOW-04 in `docs/portfolio/08_HALOW_TECHNOLOGY_DOSSIER.md`
- Meta: Shard=B | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=A | PRD=NF-PWR-02 | Const=C-00,C-08

### S-04-025 — `hal_power` battery fuel gauge (MAX17048) I²C
As a firmware engineer I want `hal_power` backed by the MAX17048 fuel gauge over I²C so that Alpha battery state is accurately reported to the system.
- AC: SoC, voltage, current reported; low-battery IRQ wired to the power manager; readings validated against a bench meter within documented tolerance
- Meta: Shard=— | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=A | PRD=NF-PWR-01 | Const=C-00

### S-04-026 — USB-C PD source detection + charge state machine
As a device owner I want USB-C source detection and a charge state machine on Alpha so that charging is safe and its status is always accurate.
- AC: source capability detected on plug-in; charge states (pre-charge/CC/CV/full/fault) transition correctly; fault states surfaced to log and UI
- Meta: Shard=— | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=A | PRD=— | Const=C-00

### S-04-027 — Button matrix + debounce + long-press events (hard PTT)
As a device owner I want debounced button-matrix input with long-press events so that Alpha's hard PTT and navigation feel instant and reliable.
- AC: PTT press < 25 ms latency to audio pipeline; debounce rejects contact bounce without missing fast presses; long-press and release events emitted for all buttons
- Meta: Shard=— | Type=Feature | Size=S | Prio=P0 | Status=DRAFT | SKU=A | PRD=F-MSG-04,F-UI-06 | Const=C-00

### S-04-028 — Haptic driver (DRV2605L)
As a device owner I want haptic feedback on Alpha so that interactions are confirmed without looking at the screen.
- AC: DRV2605L plays a basic effect set; per-event haptic hooks exposed to `ss_ui`; degrades gracefully when hardware is absent
- Meta: Shard=— | Type=Feature | Size=S | Prio=P1 | Status=DRAFT | SKU=A | PRD=F-UI-03 | Const=C-00

### S-04-029 — Bezel LED driver (12× SK6805) + status patterns
As a device owner I want the Alpha bezel LEDs driven with status patterns so that link, activity, and SOS states are visible at a glance.
- AC: link, activity, and SOS-breathe patterns implemented; higher-priority patterns preempt lower ones; brightness respects the active power profile
- Meta: Shard=— | Type=Feature | Size=S | Prio=P1 | Status=DRAFT | SKU=A | PRD=F-MSG-08 | Const=C-00
