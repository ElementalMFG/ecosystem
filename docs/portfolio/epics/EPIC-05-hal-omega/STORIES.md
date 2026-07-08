<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# EPIC-05 — Stories

Format per `../../00_METHODOLOGY.md` §2.7. Meta lines are machine-parsed.

---

### S-05-001 — Cellular modem driver (AT + MUX) skeleton
As a firmware engineer I want the cellular modem AT+MUX driver skeleton so that higher layers get a stable command and data channel.
- AC: CMUX channels for AT and data established; AT command queue with timeouts and retries implemented; modem power-on/reset sequencing verified on hardware
- Meta: Shard=A | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=O | PRD=F-BR-06 | Const=C-00,C-08

### S-05-002 — PDP context activation + IP passthrough
As a firmware engineer I want PDP context activation with IP passthrough so that IP traffic flows over LTE-M.
- AC: PDP context activates on a supported carrier; IP passthrough delivers packets to the network stack; teardown and reattach handled without reboot
- Meta: Shard=A | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=O | PRD=F-BR-06 | Const=C-08

### S-05-003 — LTE-M band-lock + preferred RAT
As a fleet admin I want LTE-M band-lock and preferred-RAT configuration so that devices attach quickly on known networks.
- AC: band-lock persists across reboot; preferred RAT (LTE-M vs NB-IoT) selectable by policy; attach + IP data path within 30 s in coverage verified
- Meta: Shard=A | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=O | PRD=F-BR-06 | Const=C-08

### S-05-004 — NB-IoT band table + regional profiles
As a fleet admin I want the NB-IoT band table with regional roaming profiles so that devices work across global deployments.
- AC: regional profiles cover target markets; profile selected by region code; roaming behaviour validated on at least two carriers
- Meta: Shard=D | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=O | PRD=F-BR-06 | Const=C-08

### S-05-005 — PSM entry / eDRX config
As a device owner I want PSM and eDRX configuration so that cellular standby meets the Omega battery target.
- AC: PSM sleep current ≤ 15 µA measured; eDRX cycles configurable by policy; wake-on-page verified within the eDRX window
- Meta: Shard=C | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=O | PRD=NF-PWR-03 | Const=C-00,C-08

### S-05-006 — eSIM (SGP.32) profile install
As a fleet admin I want eSIM (SGP.32) profile install so that connectivity is provisioned without physical SIM handling.
- AC: profile install and delete succeed over BIP; profile state survives reboot; failed installs roll back cleanly
- Meta: Shard=B | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=O | PRD=F-BR-06 | Const=C-08

### S-05-007 — Fallback SIM slot (nano-SIM)
As a device owner I want a fallback nano-SIM slot so that I can use a local carrier when eSIM is not viable.
- AC: physical SIM detected and preferred per policy; SIM removal/insert handled safely at runtime; SIM PIN flow supported
- Meta: Shard=B | Type=Feature | Size=S | Prio=P1 | Status=DRAFT | SKU=O | PRD=F-BR-06 | Const=C-00,C-08

### S-05-008 — Satellite modem driver (Iridium SBD)
As a device owner I want the Iridium SBD satellite modem driver so that SOS and short messages get out with no terrestrial coverage.
- AC: SBD round-trip succeeds in a field trial; message queue respects the airtime cost policy; signal/pointing status surfaced to the UI
- Meta: Shard=E | Type=Feature | Size=L | Prio=P1 | Status=DRAFT | SKU=O | PRD=F-MSG-08 | Const=C-08

### S-05-009 — Satellite modem driver (Skylo NTN)
As a device owner I want the Skylo NTN modem driver so that 3GPP Release-17 satellite messaging is available where supported.
- AC: NTN attach succeeds on the Skylo test network; message round-trip verified; fallback to terrestrial RAT is automatic
- Meta: Shard=E | Type=Feature | Size=L | Prio=P2 | Status=DRAFT | SKU=O | PRD=F-MSG-08 | Const=C-08

### S-05-010 — BMP390 barometer driver
As a firmware engineer I want the BMP390 barometer driver so that altitude data enriches location and SAR use cases.
- AC: pressure and derived altitude readable via the HAL sensor API; sampling rates configurable per power profile; calibration offset persisted
- Meta: Shard=G | Type=Feature | Size=S | Prio=P1 | Status=DRAFT | SKU=O | PRD=— | Const=C-00

### S-05-011 — MMC5983MA magnetometer driver
As a device owner I want the MMC5983MA magnetometer driver so that the Seekie compass has a true heading.
- AC: calibrated heading output with hard/soft-iron compensation; self-test runs on boot; magnetic-interference flag exposed
- Meta: Shard=G | Type=Feature | Size=S | Prio=P1 | Status=DRAFT | SKU=O | PRD=F-APP-04 | Const=C-00

### S-05-012 — Multi-LED status ring driver
As a fleet admin I want the multi-LED status ring driver so that radio state is readable at a distance in industrial settings.
- AC: ring patterns map to radio/bearer states; patterns configurable by fleet policy; brightness respects the active power profile
- Meta: Shard=H | Type=Feature | Size=S | Prio=P2 | Status=DRAFT | SKU=O | PRD=— | Const=C-00

### S-05-013 — ATECC608 / SE050 secure-element driver
As a security engineer I want the ATECC608/SE050 secure-element driver so that Omega key operations are hardware-isolated.
- AC: SE provisioned and locked in the factory flow; sign/verify and key-storage operations via SE verified; SE absence detected and handled per policy
- Meta: Shard=I | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=O | PRD=F-SEC-01,F-SEC-02,NF-SEC-01 | Const=C-05

### S-05-014 — Tamper switch + key-wipe policy
As a security engineer I want the tamper switch wired to a key-wipe policy so that physical intrusion destroys secrets.
- AC: armed tamper event wipes designated keys; wipe event logged and beaconed per policy; false-trigger rate assessed in shake/drop testing
- Meta: Shard=I | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=O | PRD=F-SEC-10,NF-SEC-02 | Const=C-05

### S-05-015 — Enhanced power path (boost + supercap) validation
As a firmware engineer I want the enhanced power path validated so that cellular TX bursts never brown out the system.
- AC: boost + supercap sustain the maximum TX burst without brown-out; brown-out tolerance margin measured; charge/discharge behaviour documented
- Meta: Shard=F | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=O | PRD=NF-PWR-03 | Const=C-00

### S-05-016 — Omega HIL rack test-plan
As a test engineer I want an Omega HIL rack test-plan so that cellular, satellite, and security hardware are exercised per merge.
- AC: rack includes a cell simulator or live-SIM fixture; tamper and secure-element tests automated; test matrix maps to EPIC-05 exit criteria
- Meta: Shard=— | Type=Ops | Size=M | Prio=P0 | Status=DRAFT | SKU=O | PRD=— | Const=C-00

### S-05-017 — Cellular network compatibility matrix (top 20 carriers)
As a fleet admin I want a cellular compatibility matrix for the top 20 carriers so that deployments can be planned with confidence.
- AC: attach/data/PSM behaviour recorded per carrier; matrix published and versioned; regressions tracked per firmware release
- Meta: Shard=D | Type=Ops | Size=M | Prio=P0 | Status=DRAFT | SKU=O | PRD=F-BR-06 | Const=C-08

### S-05-018 — Modem OTA (FOTA) plumbing
As a release manager I want modem FOTA plumbing so that modem firmware can be updated securely in the field.
- AC: modem firmware applied via delta or full image; update authenticated and resumable after interruption; failed update rolls back to the prior modem firmware
- Meta: Shard=A | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=O | PRD=NF-REL-02 | Const=C-05,C-08

### S-05-019 — Cellular modem vendor selection spike (PRD Q-04)
As a hardware/firmware lead I want a scoping spike comparing candidate cellular modems (Quectel BG95-class, Nordic nRF9160-class, u-blox SARA-class) on cost, LTE-M/NB-IoT coverage, power, AT/driver maturity, FOTA support, and radio-certification burden so that the Omega modem choice is an evidenced ADR instead of an open question.
- AC: a decision matrix covering unit cost at target volume, regional band/coverage fit, sleep/active power against the Omega battery budget, driver and CMUX maturity against S-05-001, modem-FOTA capability against S-05-018/S-09-019, and per-region certification scope (FCC/CE/ISED/ACMA) is produced; supply-chain risk (second source, longevity commitment) is assessed per vendor; the selection is recorded as an ADR closing PRD Q-04 with wg-firmware + wg-legal (certification) sign-off; the Omega BOM (S-24-031) is updated with the selected part
- Meta: Shard=A | Type=Spike | Size=S | Prio=P1 | Status=DRAFT | SKU=O | PRD=F-BR-06, NF-COST-02 | Const=C-01, C-08

### S-05-020 — Omega v1.x spec-lock (SoM/BOM, app-platform scope)
As the product owner I want an Omega v1.x spec-lock — mirroring the Lite D-0013 pattern — so that the deferred Omega commitments in RFC-0004 (cellular/LEO, Linux SoM, full on-device browser and expanded smartphone-class apps) are pinned to real part selections before any Omega story starts.
- AC: SoM/module part numbers selected and recorded by decision entry; the RFC-0004 SL-4/SL-5 deferrals (on-device browser, expanded app platform) are scoped in/out explicitly for v1.x; board_config.h TODO(models/CATALOG) placeholders for Omega resolve against the locked BOM; EPIC-05 story priorities re-sequenced against the lock
- Meta: Shard=— | Type=Task | Size=M | Prio=P2 | Status=DRAFT | SKU=O | PRD=— | Const=C-00
