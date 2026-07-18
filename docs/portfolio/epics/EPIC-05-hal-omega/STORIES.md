<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# EPIC-05 — Stories

Format per `../../00_METHODOLOGY.md` §2.7. Meta lines are machine-parsed.

---

### S-05-001 — Cellular modem driver (AT + MUX) skeleton
As a firmware engineer I want the cellular modem AT+MUX driver skeleton so that higher layers get a stable command and data channel.
- AC: CMUX channels for AT and data established; AT command queue with timeouts and retries implemented; modem power-on/reset sequencing verified on hardware
- Meta: Shard=A | Type=Feature | Size=L | Prio=P0 | Status=BLOCKED | SKU=O | PRD=F-BR-06 | Const=C-00,C-08
- Tasks: spec · design · impl · test · docs — all pending Omega rev-2 hardware
- Deps: external: BLOCKED 2026-07-09 (D-0020): cellular modem absent from the signed-off Omega v1.0 board (PCB release v69 — no expansion interface, respin required); capability preserved as an UNSCHEDULED roadmap option (D-0024: cellular is not in current product intent; revisit only if a future board rev adopts a modem) — see `docs/dev/OMEGA_HW_BASELINE.md`

### S-05-002 — PDP context activation + IP passthrough
As a firmware engineer I want PDP context activation with IP passthrough so that IP traffic flows over LTE-M.
- AC: PDP context activates on a supported carrier; IP passthrough delivers packets to the network stack; teardown and reattach handled without reboot
- Meta: Shard=A | Type=Feature | Size=M | Prio=P0 | Status=BLOCKED | SKU=O | PRD=F-BR-06 | Const=C-08
- Tasks: spec · design · impl · test · docs — all pending Omega rev-2 hardware
- Deps: external: BLOCKED 2026-07-09 (D-0020): cellular modem absent from the signed-off Omega v1.0 board (PCB release v69 — no expansion interface, respin required); capability preserved as an UNSCHEDULED roadmap option (D-0024: cellular is not in current product intent; revisit only if a future board rev adopts a modem) — see `docs/dev/OMEGA_HW_BASELINE.md`

### S-05-003 — LTE-M band-lock + preferred RAT
As a fleet admin I want LTE-M band-lock and preferred-RAT configuration so that devices attach quickly on known networks.
- AC: band-lock persists across reboot; preferred RAT (LTE-M vs NB-IoT) selectable by policy; attach + IP data path within 30 s in coverage verified
- Meta: Shard=A | Type=Feature | Size=M | Prio=P0 | Status=BLOCKED | SKU=O | PRD=F-BR-06 | Const=C-08
- Tasks: spec · design · impl · test · docs — all pending Omega rev-2 hardware
- Deps: external: BLOCKED 2026-07-09 (D-0020): cellular modem absent from the signed-off Omega v1.0 board (PCB release v69 — no expansion interface, respin required); capability preserved as an UNSCHEDULED roadmap option (D-0024: cellular is not in current product intent; revisit only if a future board rev adopts a modem) — see `docs/dev/OMEGA_HW_BASELINE.md`

### S-05-004 — NB-IoT band table + regional profiles
As a fleet admin I want the NB-IoT band table with regional roaming profiles so that devices work across global deployments.
- AC: regional profiles cover target markets; profile selected by region code; roaming behaviour validated on at least two carriers
- Meta: Shard=D | Type=Feature | Size=M | Prio=P0 | Status=BLOCKED | SKU=O | PRD=F-BR-06 | Const=C-08
- Tasks: spec · design · impl · test · docs — all pending Omega rev-2 hardware
- Deps: external: BLOCKED 2026-07-09 (D-0020): cellular modem absent from the signed-off Omega v1.0 board (PCB release v69 — no expansion interface, respin required); capability preserved as an UNSCHEDULED roadmap option (D-0024: cellular is not in current product intent; revisit only if a future board rev adopts a modem) — see `docs/dev/OMEGA_HW_BASELINE.md`

### S-05-005 — PSM entry / eDRX config
As a device owner I want PSM and eDRX configuration so that cellular standby meets the Omega battery target.
- AC: PSM sleep current ≤ 15 µA measured; eDRX cycles configurable by policy; wake-on-page verified within the eDRX window
- Meta: Shard=C | Type=Feature | Size=M | Prio=P0 | Status=BLOCKED | SKU=O | PRD=NF-PWR-03 | Const=C-00,C-08
- Tasks: spec · design · impl · test · docs — all pending Omega rev-2 hardware
- Deps: external: BLOCKED 2026-07-09 (D-0020): cellular modem absent from the signed-off Omega v1.0 board (PCB release v69 — no expansion interface, respin required); capability preserved as an UNSCHEDULED roadmap option (D-0024: cellular is not in current product intent; revisit only if a future board rev adopts a modem) — see `docs/dev/OMEGA_HW_BASELINE.md`

### S-05-006 — eSIM (SGP.32) profile install
As a fleet admin I want eSIM (SGP.32) profile install so that connectivity is provisioned without physical SIM handling.
- AC: profile install and delete succeed over BIP; profile state survives reboot; failed installs roll back cleanly
- Meta: Shard=B | Type=Feature | Size=L | Prio=P0 | Status=BLOCKED | SKU=O | PRD=F-BR-06 | Const=C-08
- Tasks: spec · design · impl · test · docs — all pending Omega rev-2 hardware
- Deps: external: BLOCKED 2026-07-09 (D-0020): cellular modem (eSIM) absent from the signed-off Omega v1.0 board (PCB release v69 — no expansion interface, respin required); capability preserved as an UNSCHEDULED roadmap option (D-0024: cellular is not in current product intent; revisit only if a future board rev adopts a modem) — see `docs/dev/OMEGA_HW_BASELINE.md`

### S-05-007 — Fallback SIM slot (nano-SIM)
As a device owner I want a fallback nano-SIM slot so that I can use a local carrier when eSIM is not viable.
- AC: physical SIM detected and preferred per policy; SIM removal/insert handled safely at runtime; SIM PIN flow supported
- Meta: Shard=B | Type=Feature | Size=S | Prio=P1 | Status=BLOCKED | SKU=O | PRD=F-BR-06 | Const=C-00,C-08
- Tasks: spec · design · impl · test · docs — all pending Omega rev-2 hardware
- Deps: external: BLOCKED 2026-07-09 (D-0020): SIM slot absent from the signed-off Omega v1.0 board (PCB release v69 — no expansion interface, respin required); capability preserved as an UNSCHEDULED roadmap option (D-0024: cellular is not in current product intent; revisit only if a future board rev adopts a modem) — see `docs/dev/OMEGA_HW_BASELINE.md`

### S-05-008 — Satellite modem driver (Iridium SBD)
As a device owner I want the Iridium SBD satellite modem driver so that SOS and short messages get out with no terrestrial coverage.
- AC: SBD round-trip succeeds in a field trial; message queue respects the airtime cost policy; signal/pointing status surfaced to the UI
- Meta: Shard=E | Type=Feature | Size=L | Prio=P1 | Status=BLOCKED | SKU=O | PRD=F-MSG-08 | Const=C-08
- Tasks: spec · design · impl · test · docs — all pending Omega rev-2 hardware
- Deps: external: BLOCKED 2026-07-09 (D-0020): satellite modem absent from the signed-off Omega v1.0 board (PCB release v69 — no expansion interface, respin required); capability preserved for Omega rev-2 — see `docs/dev/OMEGA_HW_BASELINE.md`

### S-05-009 — Satellite modem driver (Skylo NTN)
As a device owner I want the Skylo NTN modem driver so that 3GPP Release-17 satellite messaging is available where supported.
- AC: NTN attach succeeds on the Skylo test network; message round-trip verified; fallback to terrestrial RAT is automatic
- Meta: Shard=E | Type=Feature | Size=L | Prio=P2 | Status=BLOCKED | SKU=O | PRD=F-MSG-08 | Const=C-08
- Tasks: spec · design · impl · test · docs — all pending Omega rev-2 hardware
- Deps: external: BLOCKED 2026-07-09 (D-0020): satellite modem absent from the signed-off Omega v1.0 board (PCB release v69 — no expansion interface, respin required); capability preserved for Omega rev-2 — see `docs/dev/OMEGA_HW_BASELINE.md`

### S-05-010 — BMP390 barometer driver
As a firmware engineer I want the BMP390 barometer driver so that altitude data enriches location and SAR use cases.
- AC: pressure and derived altitude readable via the HAL sensor API; sampling rates configurable per power profile; calibration offset persisted
- Meta: Shard=G | Type=Feature | Size=S | Prio=P1 | Status=BLOCKED | SKU=O | PRD=— | Const=C-00
- Tasks: spec · design · impl · test · docs — all pending Omega rev-2 hardware
- Deps: external: BLOCKED 2026-07-09 (D-0020): BMP390 barometer absent from the signed-off Omega v1.0 board (PCB release v69 — no expansion interface, respin required); capability preserved for Omega rev-2 — see `docs/dev/OMEGA_HW_BASELINE.md`

### S-05-011 — Magnetometer driver (BMM350)
As a device owner I want the BMM350 magnetometer driver so that the Seekie compass has a true heading.
- AC: calibrated heading output with hard/soft-iron compensation; self-test runs on boot; magnetic-interference flag exposed
- Deps: retargeted 2026-07-09 (D-0020): the v69 board fits a Bosch BMM350 (U79, I²C, WLCSP-9), not the planned MMC5983MA — see `docs/dev/OMEGA_HW_BASELINE.md`
- Meta: Shard=G | Type=Feature | Size=S | Prio=P1 | Status=DRAFT | SKU=O | PRD=F-APP-04 | Const=C-00

### S-05-012 — Multi-LED status ring driver
As a fleet admin I want the multi-LED status ring driver so that radio state is readable at a distance in industrial settings.
- AC: ring patterns map to radio/bearer states; patterns configurable by fleet policy; brightness respects the active power profile
- Meta: Shard=H | Type=Feature | Size=S | Prio=P2 | Status=DRAFT | SKU=O | PRD=— | Const=C-00
- Deps: part confirmed 2026-07-09 (D-0020): 12× SK6805-EC15 addressable LEDs on v69 (U24–U35) — story is buildable against real hardware; see `docs/dev/OMEGA_HW_BASELINE.md`

### S-05-013 — ATECC608 / SE050 secure-element driver
As a security engineer I want the ATECC608/SE050 secure-element driver so that Omega key operations are hardware-isolated.
- AC: SE provisioned and locked in the factory flow; sign/verify and key-storage operations via SE verified; SE absence detected and handled per policy
- Meta: Shard=I | Type=Feature | Size=L | Prio=P0 | Status=BLOCKED | SKU=O | PRD=F-SEC-01,F-SEC-02,NF-SEC-01 | Const=C-05
- Tasks: spec · design · impl · test · docs — all pending Omega rev-2 hardware
- Deps: external: BLOCKED 2026-07-09 (D-0020): secure element absent from the signed-off Omega v1.0 board (PCB release v69 — no expansion interface, respin required); capability preserved for Omega rev-2 — see `docs/dev/OMEGA_HW_BASELINE.md`

### S-05-014 — Tamper switch + key-wipe policy
As a security engineer I want the tamper switch wired to a key-wipe policy so that physical intrusion destroys secrets.
- AC: armed tamper event wipes designated keys; wipe event logged and beaconed per policy; false-trigger rate assessed in shake/drop testing
- Meta: Shard=I | Type=Feature | Size=M | Prio=P0 | Status=BLOCKED | SKU=O | PRD=F-SEC-10,NF-SEC-02 | Const=C-05
- Tasks: spec · design · impl · test · docs — all pending Omega rev-2 hardware
- Deps: external: BLOCKED 2026-07-09 (D-0020): secure element (tamper-switch presence also unconfirmed in the v69 BOM) absent from the signed-off Omega v1.0 board (PCB release v69 — no expansion interface, respin required); capability preserved for Omega rev-2 — see `docs/dev/OMEGA_HW_BASELINE.md`

### S-05-015 — Enhanced power path (boost + supercap) validation
As a firmware engineer I want the enhanced power path validated so that cellular TX bursts never brown out the system.
- AC: boost + supercap sustain the maximum TX burst without brown-out; brown-out tolerance margin measured; charge/discharge behaviour documented
- Meta: Shard=F | Type=Feature | Size=M | Prio=P0 | Status=BLOCKED | SKU=O | PRD=NF-PWR-03 | Const=C-00
- Tasks: spec · design · impl · test · docs — all pending Omega rev-2 hardware
- Deps: external: BLOCKED 2026-07-09 (D-0020): supercap/boost TX power path (v69's SY7201 boost is backlight-only) absent from the signed-off Omega v1.0 board (PCB release v69 — no expansion interface, respin required); capability preserved for Omega rev-2 — see `docs/dev/OMEGA_HW_BASELINE.md`

### S-05-016 — Omega HIL rack test-plan
As a test engineer I want an Omega HIL rack test-plan so that cellular, satellite, and security hardware are exercised per merge.
- AC: rack includes a cell simulator or live-SIM fixture; tamper and secure-element tests automated; test matrix maps to EPIC-05 exit criteria; D-0021: covers the EVT matrix EVT-1..9 (FPC ESD, SDIO EMI, SK6805 low-vbat, I²C 400 kHz, HaLow range, skin temp, MM8108 mesh/AP SDK availability, USB CDC/ECM device-role) — see docs/dev/OMEGA_LEDGER_ALIGNMENT.md §6; numeric EVT pass bars are owned by HW-repo ST-M5 — execute against those, never invent local ones
- Meta: Shard=— | Type=Ops | Size=M | Prio=P0 | Status=DRAFT | SKU=O | PRD=— | Const=C-00

### S-05-017 — Cellular network compatibility matrix (top 20 carriers)
As a fleet admin I want a cellular compatibility matrix for the top 20 carriers so that deployments can be planned with confidence.
- AC: attach/data/PSM behaviour recorded per carrier; matrix published and versioned; regressions tracked per firmware release
- Meta: Shard=D | Type=Ops | Size=M | Prio=P0 | Status=BLOCKED | SKU=O | PRD=F-BR-06 | Const=C-08
- Tasks: spec · design · impl · test · docs — all pending Omega rev-2 hardware
- Deps: external: BLOCKED 2026-07-09 (D-0020): cellular modem absent from the signed-off Omega v1.0 board (PCB release v69 — no expansion interface, respin required); capability preserved as an UNSCHEDULED roadmap option (D-0024: cellular is not in current product intent; revisit only if a future board rev adopts a modem) — see `docs/dev/OMEGA_HW_BASELINE.md`

### S-05-018 — Modem OTA (FOTA) plumbing
As a release manager I want modem FOTA plumbing so that modem firmware can be updated securely in the field.
- AC: modem firmware applied via delta or full image; update authenticated and resumable after interruption; failed update rolls back to the prior modem firmware
- Meta: Shard=A | Type=Feature | Size=M | Prio=P0 | Status=BLOCKED | SKU=O | PRD=NF-REL-02 | Const=C-05,C-08
- Tasks: spec · design · impl · test · docs — all pending Omega rev-2 hardware
- Deps: external: BLOCKED 2026-07-09 (D-0020): cellular modem absent from the signed-off Omega v1.0 board (PCB release v69 — no expansion interface, respin required); capability preserved as an UNSCHEDULED roadmap option (D-0024: cellular is not in current product intent; revisit only if a future board rev adopts a modem) — see `docs/dev/OMEGA_HW_BASELINE.md`

### S-05-019 — Cellular modem vendor selection spike (PRD Q-04)
As a hardware/firmware lead I want a scoping spike comparing candidate cellular modems (Quectel BG95-class, Nordic nRF9160-class, u-blox SARA-class) on cost, LTE-M/NB-IoT coverage, power, AT/driver maturity, FOTA support, and radio-certification burden so that the Omega modem choice is an evidenced ADR instead of an open question.
- AC: a decision matrix covering unit cost at target volume, regional band/coverage fit, sleep/active power against the Omega battery budget, driver and CMUX maturity against S-05-001, modem-FOTA capability against S-05-018/S-09-019, and per-region certification scope (FCC/CE/ISED/ACMA) is produced; supply-chain risk (second source, longevity commitment) is assessed per vendor; the selection is recorded as an ADR closing PRD Q-04 with wg-firmware + wg-legal (certification) sign-off; the Omega BOM (S-24-031) is updated with the selected part
- Meta: Shard=A | Type=Spike | Size=S | Prio=P1 | Status=BLOCKED | SKU=O | PRD=F-BR-06, NF-COST-02 | Const=C-01, C-08
- Tasks: spec · design · impl · test · docs — all pending Omega rev-2 hardware
- Deps: external: BLOCKED 2026-07-09 (D-0020): cellular modem absent from the signed-off Omega v1.0 board (PCB release v69 — no expansion interface, respin required); capability preserved as an UNSCHEDULED roadmap option (D-0024: cellular is not in current product intent; revisit only if a future board rev adopts a modem) — see `docs/dev/OMEGA_HW_BASELINE.md`

### S-05-020 — Omega v1.x spec-lock (SoM/BOM, app-platform scope)
As the product owner I want an Omega v1.x spec-lock — mirroring the Lite D-0013 pattern — so that the deferred Omega commitments in RFC-0004 (cellular/LEO, Linux SoM, full on-device browser and expanded smartphone-class apps) are pinned to real part selections before any Omega story starts.
- AC: SoM/module part numbers selected and recorded by decision entry; the RFC-0004 SL-4/SL-5 deferrals (on-device browser, expanded app platform) are scoped in/out explicitly for v1.x; board_config.h TODO(models/CATALOG) placeholders for Omega resolve against the locked BOM; EPIC-05 story priorities re-sequenced against the lock
- Meta: Shard=— | Type=Task | Size=M | Prio=P2 | Status=DRAFT | SKU=O | PRD=— | Const=C-00
- Deps: input of record 2026-07-09 (D-0020): `docs/dev/OMEGA_HW_BASELINE.md` (PCB release v69, SHA 054eaa8b) — board_config claims flow from it; Alpha inherits nothing until an Alpha release package exists

### S-05-021 — Display driver: ST7796S 8-bit parallel I8080, 480×480
As a firmware engineer I want the ST7796S 8-bit parallel I8080 display driver so that the Omega 480×480 panel renders the UI reliably.
- AC: I8080 bus brought up per the v69 pinmap (ER-TFT3.92-1, 40-pin FPC / J6) · 480×480 framebuffer with tearing-safe refresh · backlight PWM ≥ 20 kHz on the SY7201 boost EN pin with no visible beat against panel refresh (FW-15) · boot splash rendered within the boot-time budget
- Meta: Shard=J | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=O | PRD=F-UI-01 | Const=C-00

### S-05-022 — GT911 touch driver + D-27 strap sequence
As a firmware engineer I want the GT911 touch driver with the D-27 reset-strap sequence so that touch works and the controller never collides with the BMM350 magnetometer on I²C.
- AC: tp_int held LOW while touch_rst is released so the GT911 straps to I²C 0x5D (never 0x14, which collides with BMM350 — FW-16) · boot-order dependency (touch strap before magnetometer probe) documented in board_config · probe confirms 0x5D on boot · multi-touch events delivered to ss_input
- Meta: Shard=J | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=O | PRD=— | Const=C-00

### S-05-023 — Audio path bring-up: ES8311 + NS4150B gating + PDM mic
As a firmware engineer I want the ES8311 codec, NS4150B gating, and PDM mic capture brought up so that Omega has clean playback and capture with no idle class-D noise.
- AC: SPH0641LU4H PDM mic captured, clocked from i2s_bck · ES8311 codec playback path functional · NS4150B CTRL/SD is hard-strapped to v3v3 (BIT-2) — there is NO mute GPIO, so muting is codec-side: ES8311 register mute + MCLK stop on RX/TX transitions · class-D idle noise floor measured at EVT (HW ST-M6.2, gate ≤ 30 dBA at 10 cm) (FW-9)
- Meta: Shard=J | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=O | PRD=— | Const=C-00

### S-05-024 — Battery policy: MAX17048 + two-tier low-battery shutdown
As a device owner I want a two-tier low-battery policy driven by the MAX17048 fuel gauge so that Omega shuts down cleanly instead of corrupting data on brown-out.
- AC: SoC/voltage polled every ≤ 5 s · current is MODELED from ΔV/Δt (MAX17048 has no shunt — FW-12) · warn at 3.3 V, save-state + deep-sleep at 3.0 V (FW-6) · SD writes quiesced before cutoff
- Meta: Shard=K | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=O | PRD=NF-PWR-01 | Const=C-00

### S-05-025 — Power-tree bring-up: EA3059 3-ch rails + TP4056 charger
As a firmware engineer I want the EA3059 3-channel power tree and TP4056 charger brought up so that all rails are sequenced within v69 tolerances and charge state is observable.
- AC: v3v3 / v1v8 / v3v6_rf rails sequenced and validated against v69 tolerances · TP4056 charger status surfaced to the power service · per-rail brownout thresholds set · boot aborts safely if a rail is out of tolerance
- Meta: Shard=K | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=O | PRD=— | Const=C-00

### S-05-026 — USB-C ↔ LCD pin arbitration
As a firmware engineer I want a VBUS-detect arbitration state machine for the shared GPIO26/27 pins so that plugging USB never garbles the UI or fails enumeration.
- AC: VBUS-detect state machine drives mux ownership · LCD de-init before GPIO26/27 are muxed to the USB FS device, and re-init on VBUS drop (FW-3) · no garbled UI or failed enumeration across 100 plug/unplug cycles · mux ownership is race-free (single-owner)
- Meta: Shard=J | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=O | PRD=— | Const=C-00

### S-05-027 — Boot-strap pin deferral guard
As a firmware engineer I want a compile/assert guard against early writes to the boot-strap pins so that random boot-mode misfires are eliminated.
- AC: any write to GPIO34/35/36 before `board_init_done` fails a compile-time check or asserts at runtime (FW-2) · reset-loop soak shows 0 boot-mode misfires · guard and rationale documented in board_config
- Meta: Shard=J | Type=Feature | Size=S | Prio=P0 | Status=DRAFT | SKU=O | PRD=— | Const=C-00

### S-05-028 — TX thermal duty cap + case-temp model
As a device owner I want a TX duty cap driven by a software case-temperature model so that Omega meets IEC 62368-1 skin-temp limits during sustained transmit.
- AC: rolling-60 s TX duty ≤ 40 % enforced in the TX task (FW-1) · software case-temp model derived from die temperature (no pack NTC — FW-8) feeds the cap · IEC 62368-1 43 °C skin-temp gate met in soak testing · the EPIC-24 certification story cites this cap (SPEC-2)
- Meta: Shard=K | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=O | PRD=— | Const=C-00

### S-05-029 — PA VSWR watchdog (composite proxy — no pa_pdet net)
As a device owner I want a PA VSWR watchdog on pa_pdet so that an antenna-disconnect fault inhibits TX before the PA degrades.
- AC: the `pa_pdet` net does NOT exist on the frozen v69 board (BIT-1) — a composite VSWR proxy per Path A (owner decision #4) correlates P4/MM8108 die-temp rise + v3v6_rf current spike + MM8108 PHY telemetry (RSSI/EVM/retry anomalies) to detect antenna-disconnect within 100 ms of TX start · proxy discrimination ≥ 3σ validated at EVT bench (HW ST-M6.1) before the watchdog arms · on fault: TX inhibited + user notified via LED/screen · escalation path documented (Path B rev-2 directional coupler) if no proxy signal discriminates (FW-5, antenna-disconnect protection; SPEC-1)
- Meta: Shard=L | Type=Feature | Size=S | Prio=P0 | Status=DRAFT | SKU=O | PRD=NF-REG-01,NF-REG-02 | Const=C-00,C-08

### S-05-030 — HaLow MM8108 SDIO bring-up + EU regional profile loader
As a device owner I want MM8108 HaLow bring-up over SDIO with a signed EU regional profile so that transmit is impossible outside the licensed 863–868 MHz allocation.
- AC: MM8108 brought up over SDIO per the v69 pinmap · EU 863–868 MHz channel + duty-cycle table loaded at first boot from a SIGNED region blob (FW-10; blob signing is a T1 sub-task) · TX impossible outside the loaded region's allocation · region enforcement survives reboot; region blob signature = Ed25519 per D-0022 (application-layer manifest chain); SDIO bus arbitration with S-05-036 microSD (shared bus) resolved in design
- Meta: Shard=L | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=O | PRD=F-BR-04,NF-REG-04 | Const=C-00,C-08

### S-05-031 — P4↔C6 ESP-Hosted-NG transport
As a firmware engineer I want the P4↔C6 ESP-Hosted-NG transport pinned so that Wi-Fi STA and BLE work through the C6 bridge with bounded wake latency.
- AC: framing, baud (≈2 Mbps), and flow control pinned in an ADR (FW-11) · Wi-Fi STA + BLE functional through the bridge · wake-from-sleep latency across the UART measured with a C6 doze protocol (FW-19) · transport recovers from C6 reset without a P4 reboot (SPEC-6)
- Meta: Shard=L | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=O | PRD=F-BR-05 | Const=C-00,C-08

### S-05-032 — BLE bonding key boundary: LTK confined to C6
As a security engineer I want BLE bonding LTKs confined to the C6 so that long-term key material never reaches P4 memory.
- AC: LTK generated and stored on the C6 only · any cross-UART provisioning uses a derived transport key, never the LTK itself (FW-17) · P4-side memory never holds the LTK (verified by test/memory inspection) · key boundary documented in a wg-security-reviewed ADR
- Meta: Shard=L | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=O | PRD=F-BR-05 | Const=C-05

### S-05-033 — JTAG / ROM-download lockdown policy
As a security engineer I want ROM-download and JTAG locked down after production programming so that a captured device cannot be dropped into download mode.
- AC: ROM download mode disabled post-production-programming via eFuse (FW-4) · lockdown decision and revocation path documented · dev-unit escape hatch defined and access-controlled · lockdown state verifiable in the QC self-test
- Meta: Shard=J | Type=Task | Size=S | Prio=P0 | Status=DRAFT | SKU=O | PRD=— | Const=C-05

### S-05-034 — I²C bus clock policy
As a firmware engineer I want a conservative I²C bus clock policy so that the 4.7 kΩ pull-ups on the v69 bus don't cause NACKs.
- AC: 100 kHz default (705 ns rise on the 4.7 kΩ pull-ups fails Fast-mode timing — FW-7) · 400 kHz enabled only after EVT-4 validation · per-slave NACK-rate telemetry exposed · policy documented in board_config
- Meta: Shard=J | Type=Feature | Size=S | Prio=P1 | Status=DRAFT | SKU=O | PRD=— | Const=C-00

### S-05-035 — Side-key input map
As a device owner I want the four side keys mapped with long-press semantics so that power/volume/reset work without any phantom D-pad paths.
- AC: power / vol+ / vol− / reset handled including long-press semantics · no phantom D-pad or rocker paths present in ss_input for Omega (DOC-24) · debounce + event delivery to ss_input verified · key map recorded in board_config
- Meta: Shard=J | Type=Feature | Size=S | Prio=P1 | Status=DRAFT | SKU=O | PRD=F-MSG-04,F-UI-06 | Const=C-00

### S-05-036 — microSD SDIO storage bring-up
As a firmware engineer I want robust microSD SDIO storage so that logs and data survive power loss.
- AC: mount/unmount robust across power loss (pairs with the S-05-024 write-quiesce) · filesystem integrity verified after an abrupt cut · EMI note: SD_CLK is the dominant emission — EVT-2 hook recorded · card-absent handled gracefully; SDIO contention with S-05-030 MM8108 resolved (see that story)
- Meta: Shard=J | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=O | PRD=— | Const=C-00

### S-05-037 — GNSS driver: u-blox MIA-M10Q over I²C, L1-only
As a device owner I want the u-blox MIA-M10Q GNSS driver over I²C so that the Seekie has a position fix.
- AC: I²C (DDC) transport, not UART (AL-5) · L1-only expectations encoded in the ACs (no L5 claims) · fix acquisition and TTFF targets taken from the MIA-M10Q datasheet · parsed fixes delivered to ss_gnss; backup-domain time policy per BIT-3 hybrid (GNSS time when locked → NTP via C6 → Y2 RTC freewheel), characterized at EVT (HW ST-M6.3)
- Meta: Shard=G | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=O | PRD=F-MSG-07 | Const=C-00

### S-05-038 — Haptics driver: DRV2625
As a device owner I want the DRV2625 haptics driver so that notifications and confirmations have tactile feedback.
- AC: I²C-only control (no DRV2605L legacy PWM/trigger assumptions — AL-8 / D-28) · LRA waveforms defined for notify and confirm events · licensed effects/waveform configuration documented · driver delivered through the HAL haptics API
- Meta: Shard=J | Type=Feature | Size=S | Prio=P2 | Status=DRAFT | SKU=O | PRD=F-UI-03 | Const=C-00

### S-05-039 — Omega UI layout descriptor + touch-primary focus nav
As a UI engineer I want a 480×480 square layout descriptor with touch-primary focus navigation so that the Omega UI is authored for its real panel (FW-13).
- AC: SQUARE 480×480 descriptor (dpi ≈ 175 TBD from the panel datasheet, bezel_led_count = 12) registered in doc 03 (DOC-25/26) · focus-nav works touch-primary · the 4 side keys act only as a system-level nav fallback · descriptor round-trips in the UI layout loader
- Meta: Shard=J | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=O | PRD=F-UI-01 | Const=C-00

### S-05-040 — LoRa bearer integration (SX1262) — Omega rev-2
As a firmware engineer I want the shared SX1262 driver + ss-link LoRa bearer integrated on Omega rev-2 so that Omega gains the LoRa mesh bearer the owner prioritized for the respin.
- AC: reuses the EPIC-03 SX1262 driver (S-03-011) unmodified behind the HAL · SS_CAP_RADIO_LORA claimed ONLY on rev-2 board configs (never v1.0/v69) · LoRa+HaLow dual-radio coexistence policy defined · bearer registered with ss-link scheduler
- Meta: Shard=L | Type=Feature | Size=M | Prio=P1 | Status=BLOCKED | SKU=O | PRD=F-BR-01 | Const=C-00,C-08
- Tasks: spec · design · impl · test · docs — all pending Omega rev-2 hardware
- Deps: S-03-011; external: BLOCKED 2026-07-09 (D-0023): no LoRa radio on the signed-off Omega v1.0 board (PCB release v69); owner-prioritized capability for the Omega rev-2 respin — see docs/dev/OMEGA_HW_BASELINE.md

---

## Round-1 `elecrow5` bring-up (D-0026 — Elecrow CrowPanel Advance 5″ ESP32-P4)

Per D-0026 (2026-07-14) the round-1 Omega-tier board is the off-the-shelf `elecrow5`; v69 is the round-2 / in-house realization. These stories bring up `elecrow5` as the round-1 target. Pins/parts carry bring-up UNKNOWNS (D-0026 §7) to verify at attachment — any deviation updates the hardware-truth docs first. See `docs/dev/OMEGA_HW_BASELINE.md` (Round framing).

### S-05-041 — `elecrow5` board_config + capability profile ("everything optional except HaLow")
As a firmware engineer I want the `elecrow5` board_config and capability profile so that the round-1 Omega-tier board builds with HaLow mandatory and every other peripheral optional and auto-detected.
- AC: firmware board id `elecrow5` (ESP32-P4 + onboard C6) added, keyed off hardware target + tier role, never a marketing name (D-0026 §3) · SS_CAP_RADIO_HALOW claimed mandatory (HaLow present on every V1 unit); Wi-Fi 6 + BLE (C6) always claimed · display capability = **16-bit parallel RGB565** (`esp_lcd` RGB path, not MIPI-DSI — D-0027) · **on-board audio (NS4168 ×2) + PDM MEMS mic are always-present capabilities** (not auto-detected — D-0027) · **NO fuel-gauge capability flag** (no on-board fuel gauge; battery is sensed via the STC8 co-MCU ADC over I²C1 — D-0027); **no on-board GNSS or magnetometer** (external-only) · 16 MB (W25Q128JV) flash + dual USB-C reflected in the profile · LoRa, GNSS, compass, camera are additive-optional capabilities gated on boot-time presence (S-03-048) — HaLow is never removed; the profile system supports any HaLow + {LoRa, GNSS, compass, camera} combination, and absent peripherals compile/flag to zero · app code queries `ss_hal_has_cap()` (never CONFIG_*/board macros) · no IP/rugged capability claimed (dev/functional-grade board, D-0026 §5)
- Meta: Shard=J | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=O | PRD=F-BR-04 | Const=C-00,C-08
- Tasks: spec board_config + capability table · design additive variant profiles (HaLow-always + optional LoRa/GNSS/compass/camera) · impl board port · test capability-flag matrix (host) · docs board_config rationale
- Deps: D-0026, D-0027, D-0023, S-03-047, S-03-048, S-05-048 (STC8 co-MCU gates backlight-EN / SPI-UART mux select / touch+camera reset / VBAT+charge sense — D-0027)

### S-05-042 — 16-bit parallel RGB565 800×480 display + on-panel touch driver
As a firmware engineer I want the `elecrow5` 800×480 16-bit parallel RGB565 display and on-panel capacitive touch driver so that the Omega-tier round-1 UI renders and takes input.
- AC: **16-bit parallel RGB565 800×480** panel brought up via the `esp_lcd` **RGB** panel path (NOT MIPI-DSI — D-0027), FPC JP1: PCLK=GPIO3, DE=GPIO2, HSYNC=GPIO40, VSYNC=GPIO41, R/G/B on GPIO4–19 · framebuffer with tearing-safe refresh · on-panel capacitive touch over **I²C1** (SCL=GPIO46, SDA=GPIO45, INT=GPIO42, RST via GPIO36 + the STC8 co-MCU — D-0027); controller is on the display flex and **not in the board BOM, so likely GT911 @ 0x5D/0x14 is UNVERIFIED — confirm at bring-up** · multi-touch events delivered to ss_input · boot splash within the boot-time budget · panel + touch presence surfaced through the HAL, not board macros
- Meta: Shard=J | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=O | PRD=F-UI-01 | Const=C-00
- Tasks: spec RGB565 parallel bus + on-panel I²C1 touch · design esp_lcd RGB panel abstraction · impl driver · test refresh + multi-touch · docs pinmap + UNKNOWNS (touch controller/addr)
- Deps: D-0026, D-0027, S-05-041, S-05-048

### S-05-043 — HaLow (MM6108) over SPI module-slot bring-up
As a device owner I want MM6108 HaLow brought up over the `elecrow5` SPI wireless-module slot so that the mandatory round-1 bearer transmits within its licensed allocation.
- AC: MM6108 brought up over the module-slot **SPI2** (SCK=GPIO26, MOSI=GPIO48, MISO=GPIO47 — D-0027) · the **SGM3005 analog mux (slide-switch S1 + STC8 co-MCU) must be set to the module slot before slot use** — MOSI/MISO are shared with UART1→Crowtail (D-0027); firmware sets the mux via the STC8 driver (S-05-048) · HaLow driver basis = MorseMicro `esp-halow` (Apache-2.0) · HaLow-over-SPI throughput ceiling characterized · signed regional profile loaded at first boot; TX impossible outside the loaded allocation (region blob signature = Ed25519 per D-0022; signing is a T1 sub-task) · bearer registered with ss-link · reuses the HaLow bearer abstraction shared with S-05-030 behind the HAL
- Meta: Shard=L | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=O | PRD=F-BR-04 | Const=C-00,C-08
- Tasks: spec SPI2 transport + mux-select + region loader · design shared HaLow bearer path · impl driver (esp-halow basis) · test throughput + region enforcement · docs pinmap + UNKNOWNS
- Deps: D-0026, D-0027, D-0023, S-05-030, S-05-041, S-05-048

### S-05-044 — ESP32-C6 Wi-Fi 6 / BLE via ESP-Hosted link
As a firmware engineer I want the P4↔C6 ESP-Hosted link on `elecrow5` so that Wi-Fi 6 STA and BLE work through the onboard C6 with bounded wake latency.
- AC: C6↔P4 bus **CONFIRMED = SDIO 4-bit** (CLK/CMD/D0–D3 = GPIO53/54/52/51/50/49; C6 as SDIO radio slave — D-0027; the D-0026 §7 "assume SDIO vs SPI" uncertainty is resolved) on **ESP-IDF ≥ v5.3 + `esp_hosted`/`esp_wifi_remote`**, with framing/flow-control pinned in an ADR · Wi-Fi 6 STA + BLE functional through the bridge · transport recovers from C6 reset without a P4 reboot · Wi-Fi/BLE capabilities surfaced through `ss_hal_has_cap()`
- Meta: Shard=L | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=O | PRD=F-BR-05 | Const=C-00,C-08
- Tasks: spec SDIO 4-bit / esp_hosted transport · design link + recovery · impl bridge · test STA + BLE + reset recovery · docs ADR + pinmap
- Deps: D-0026, D-0027, S-05-031, S-05-041

### S-05-045 — On-board audio (always-present) + external GNSS / compass / camera auto-detection
As a device owner I want the `elecrow5` on-board audio path always available and the external GNSS (UART), 3-axis compass (I²C), and camera (MIPI-CSI) auto-detected at boot so that fitted peripherals light up plug-and-play and absent ones never break the build.
- AC: **on-board audio is always present, not auto-detected** — 2× NS4168 I²S amps (shared BCLK=GPIO22) + PDM MEMS mic (D-0027) play/capture unconditionally · **GNSS, compass, and camera are external-only — there is NO on-board GNSS, magnetometer/IMU, or camera sensor (D-0027)**; boot-time presence probes (S-03-048) apply to these external modules and set the matching capability flag, absent peripherals flag to zero · external GNSS on a free UART (not the module-slot SPI2/UART1 mux-shared pins — D-0027) delivers parsed fixes to ss_gnss · external compass delivers calibrated heading · external camera over the 2-lane MIPI-CSI header (FPC3, SCCB on I²C2 = GPIO34/33 — D-0027); sensor model (OV5647/SC2336-class) confirmed at bring-up · every capability queried via `ss_hal_has_cap()`
- Meta: Shard=J | Type=Feature | Size=L | Prio=P1 | Status=DRAFT | SKU=O | PRD=F-MSG-07 | Const=C-00
- Tasks: spec on-board audio + external presence-probe map · design per-peripheral detection · impl drivers behind HAL · test detection matrix (present/absent) · docs peripheral table + UNKNOWNS
- Deps: D-0026, D-0027, D-0023, S-03-048, S-05-041

### S-05-046 — `elecrow5` CI build target + BUILDING.md entry
As a release engineer I want an `elecrow5` CI build target and BUILDING.md entry so that the round-1 Omega-tier firmware is built and gated per merge.
- AC: `elecrow5` added to the firmware build matrix (representative additive profiles build green: HaLow-only, HaLow+LoRa, HaLow+GNSS+compass, and full) · pinned ESP-IDF version for the P4 target recorded (D-0026 §7) · `docs/dev/BUILDING.md` documents the `elecrow5` build invocation · CI build is a required-status candidate alongside `build (lite)` · build produces artifacts with no capability-flag warnings
- Meta: Shard=— | Type=Ops | Size=M | Prio=P0 | Status=DRAFT | SKU=O | PRD=— | Const=C-00
- Tasks: spec build matrix entry · design additive-profile build split · impl CMake/CI target · test green build across representative profiles · docs BUILDING.md
- Deps: D-0026, S-05-041, S-05-047

### S-05-047 — `elecrow5` LoRa additive sub-variant (SX1262 on secondary bus)
As a device owner I want an optional LoRa (SX1262) bearer added alongside HaLow on `elecrow5` so that a V1 unit can carry HaLow and LoRa together without ever giving up HaLow.
- AC: LoRa (SX1262) brought up **in addition to** the mandatory HaLow, never replacing it · **the module slot exposes reset/BUSY on GPIO29/30 (SX126x-style) on the same SPI2 as the HaLow module (D-0027)** — LoRa is an in-slot alternative to HaLow on that slot (consistent with D-0026 §4); an additive secondary-bus attachment (Crowtail SPI/UART or GPIO) remains an option, confirmed at bring-up per D-0026 §4/§7 · presence auto-detected (S-03-048); when the LoRa module is absent, HaLow-only builds are unaffected · LoRa registered with ss-link as an *additive* bearer alongside HaLow, the scheduler choosing either per QoS/energy · reuses the SX1262 driver + LoRa bearer abstraction shared with Lite (EPIC-03) behind the HAL · signed regional profile enforced (TX impossible outside the loaded allocation)
- Meta: Shard=L | Type=Feature | Size=M | Prio=P2 | Status=DRAFT | SKU=O | PRD=F-BR-04 | Const=C-00,C-08
- Tasks: spec module-slot/secondary-bus LoRa attachment · design additive dual-bearer (HaLow+LoRa) path · impl SX1262 driver reuse · test dual-bearer + module-absent · docs pinmap + attachment feasibility
- Deps: D-0026, D-0027, D-0023, S-05-041, S-05-043

### S-05-048 — `elecrow5` STC8H1K08 companion-MCU driver (I²C1)
As a firmware engineer I want a P4↔STC8H1K08 companion-MCU driver over I²C1 so that the `elecrow5` power/peripheral co-MCU (U14) can be commanded to gate board resources and report battery state that the P4 cannot reach directly.
- AC: P4↔STC8 protocol over **I²C1** implemented and documented (STC8H1K08 U14 — D-0027) · driver gates **backlight-EN**, the **SPI/UART SGM3005 mux select** (required before HaLow/LoRa module-slot use — S-05-043), **touch-RST** (GPIO36 path — S-05-042), **camera-RST**, and **audio-shutdown** · **charge-status LEDs** driven per policy · **VBAT + charge status read over I²C1** from the STC8 ADC (no on-board fuel gauge — battery sense is via the STC8, D-0027) and surfaced to the power service · STC8 presence probed at boot; absence handled gracefully · exposed behind the HAL, app code queries via `ss_hal_has_cap()` (never board macros)
- Meta: Shard=J | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=O | PRD=F-BR-04 | Const=C-00,C-08
- Tasks: spec P4↔STC8 I²C1 protocol (gates + VBAT/charge sense) · design co-MCU driver + boot-order (mux/reset gating) · impl driver behind HAL · test gate commands + VBAT read (host/mock) · docs protocol + pinmap + UNKNOWNS
- Deps: D-0026, D-0027, S-05-041
