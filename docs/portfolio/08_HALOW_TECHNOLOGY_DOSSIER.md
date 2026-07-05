<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# 08 — Wi-Fi HaLow Technology Dossier (Decision Record)

*Deep-research decision record: the current state of Wi-Fi HaLow (IEEE 802.11ah) — standard, silicon tiers, hardware, software stacks, regulatory nuances — and the binding decisions that keep SS-SP on the most capable, current, and universally interoperable HaLow technology. Research current as of 2026-07. Companion to `06_COMPETITIVE_LANDSCAPE.md` (market) and `07_FINAL_READINESS_TRIAGE.md` (audit ledger — this dossier closes T-18..T-20).*

---

## 1. The standard — what "latest HaLow" actually means

| Fact | Detail | Consequence for SS-SP |
|---|---|---|
| Base standard | IEEE 802.11ah-2016 ("Amendment 2: Sub-1 GHz License-Exempt Operation"), published 2017 | Stable, mature target — no moving spec risk |
| Current status | **Absorbed into IEEE 802.11-2020 and 802.11-2024 base standards.** There is **no successor amendment** ("HaLow 2" does not exist as a standard) | "Latest HaLow" = latest **silicon generation + certification**, not a new protocol rev |
| Certification | **Wi-Fi CERTIFIED HaLow™** (Wi-Fi Alliance, since 2021-11). Test-bed vendors: Morse Micro, Newracom, Methods2Business | Interop guarantee across vendors flows from WFA certification → we certify (S-24-037) |
| PHY | OFDM, 31.25 kHz tone spacing; BPSK/QPSK/16/64/**256-QAM**; **MCS0–10**; channels **1/2/4/8 MHz** (16 MHz optional, region-dependent) | Peak 43.33 Mbps @ 8 MHz/256-QAM; 1 MHz MCS10 duplication mode for max range |
| MAC scale | 13-bit AID → **8,191 stations per AP**; hierarchical TIM | One HGW/AP cell covers an entire neighbourhood — no AP sharding needed |
| Power save | **TWT** (target wake time — negotiated doze, later adopted by Wi-Fi 6), **RAW** (restricted access window — contention grouping), long DTIM/snooze | These are the mechanisms behind NF-PWR-02; must be explicitly exercised (S-04-024). Studies: RAW up to +76 % energy efficiency under load; TWT >2× better than RAW for sparse traffic |
| Security | WPA3 **SAE** + **GCMP** ciphers; OWE possible | Matches our WPA3-SAE stories (S-04-004); no legacy WPA2 mode shipped |
| Mesh | **802.11s** supported on HaLow (S1G hostapd/wpa_supplicant stack; batman-adv proven on top) | Our preferred infrastructure-free mode (`02_PROTOCOL_STACK.md` §3.1) is ecosystem-proven |
| Range reality | 15.9 km record @ 26 dBm class silicon (Morse, Joshua Tree, 2 Mbps sustained); 106 km @ 30 dBm (Newracom/Teledatics); mesh field studies: ~814 m single-hop LoS, ~1.1 km with 3 relays, ~120 m NLoS urban | Our "16 km ground / 100+ km LoS" claims are validated; NLoS planning number is ~120 m–1 km — the HGW window-mount guidance in `08_UNIVERSAL_CONNECTIVITY.md` stands |

## 2. Silicon tiers — the full vendor landscape

**Tier 1 — high-throughput flagship (our primary):**
- **Morse Micro MM8108** — 2nd gen, **mass production since 2025-09**. Up to **43.33 Mbps** (256-QAM/8 MHz, world-first sub-GHz 256-QAM), MCS0–10, 1/2/4/8 MHz. Integrated 26 dBm PA @ 325 mA/3.3 V (claimed 4× TX efficiency vs CMOS OFDM peers). Host: **SDIO 2.0, SPI, USB 2.0 HS** (new), MIPI RFFE. 5×5 mm BGA. WPA3 SAE + GCMP. Global sub-GHz coverage (850–950 MHz) in one SKU.
- **MM8108-M20 module** (2026-06) — up to **28.5 dBm** TX, SAW-filtered for US 902–928 MHz. Candidate for Omega/HGW high-power US variant (FCC 15.247 allows 30 dBm peak).

**Tier 1-legacy:** Morse Micro MM6108 (32.3 Mbps, 2022) — superseded; we already migrated all references to MM8108 (triage T-01).

**Tier 2 — value / standalone SoC:** Newracom **NRC7394** — 15 Mbps, +17 dBm PA, 750–950 MHz, on-die Cortex-M3 (hostless/standalone or hosted), 802.11s, 8K stations, 6×6 mm. The cross-vendor interop reference (S-22-026).

**Tier 3 — ultra-low-power:** Newracom **NRC5294** — coin-cell-class 802.11ah for massive sensor IoT. Not for SS-SP devices; relevant as *peer traffic* our HGW cells must serve.

**IP / specialty:** Methods2Business — WFA-certified HaLow IP cores; 2026: first HaLow chip with integrated edge-AI. Watch item.

**Avoid:** TaiXin **TXW8301** (Zhuhai Huge-IC) — cheap, common on AliExpress bridges, but **not Wi-Fi CERTIFIED**, runs a proprietary "WNB" mode by default, and community testing reports HaLow-compat mode works only "minus encryption". Prohibited for any SS-SP SKU or certified accessory; interop with it is explicitly out of scope.

**Ecosystem hardware (test/interop/accessory relevance):** Morse **HaLowLink 2** router (MM8108, GA 2026-01, US $129 — our reference third-party AP for HIL/interop); Gateworks GW16167 M.2 (MM8108, 43.3 Mbps); AzureWave AW-HM677 + Vantron VT-MOB-AH-8108 modules (Morse Approved Module Partner program); Quectel FGH-series; AsiaRF/Vantron USB dongles; Seeed WM6108; Heltec HT-HC32.

## 3. Software stacks — current, supported paths

| Host class | Current path | Notes |
|---|---|---|
| **ESP32 family (our firmware)** | **`morsemicro/halow` component on the ESP Component Registry** (components.espressif.com), Apache-2.0, supports ESP-IDF ≥ v5.1.1 (≥ v5.2.2 for C6) | **The old `MorseMicro/mm-iot-esp32` repo is deprecated — archived 2026-07.** S-04-023 pins the registry component. MM8108 needs explicit config (MM6108 is the component default) — CI must assert the MM8108 configuration |
| ARM Cortex-M (M4F/M7F/M33F) | MM-IoT-SDK (morselib: driver + UMAC + WPA supplicant; FreeRTOS + LwIP/mbedTLS) | Fallback path if Alpha host strategy ever changes; also the EKH05 (STM32U585) EVK stack |
| Linux (Omega-class / gateways) | Morse open Linux driver + firmware + S1G hostapd (GitHub, actively released — 1.17.x line through 2026-05), official OpenWrt fork + `morse-feed` | Enables Omega Linux-SoM option and third-party OpenWrt HaLow routers as transports (S-19 OpenWrt package story) |

**Integration nuances (bind into S-04-023/024 ACs):** WAKE/RESET/BUSY pin handshake required for chip power-save; FreeRTOS time-slicing must stay enabled (command timeouts otherwise); host deep-sleep causes 1–4 % clock drift (re-sync from GNSS/NTP per S-02-018); board-specific BCF file is mandatory per module.

## 4. Regulatory / regional nuances (binding constraints)

| Region | Band | Nuances |
|---|---|---|
| US/CA (+ most of Americas) | 902–928 MHz | FCC 15.247: up to 30 dBm peak (≥ 500 kHz BW) → 8 MHz channels + M20-class 28.5 dBm viable; best-case HaLow region |
| EU (+ most of ME) | 863–868 MHz | Strict ERP + duty limits; 863–865 MHz sub-segment unusable for HaLow (narrowband-only); practical plan = 1 MHz channels, lower power → **EU range/throughput materially below US** — set customer expectations per region (marketing + `08_UNIVERSAL_CONNECTIVITY.md` figures are US-band) |
| Japan | 916.5–927.5 MHz | Own channel plan |
| Australia/NZ | 915–928 MHz | Near-US performance |
| China | 755–787 MHz | Separate plan; TXW8301-dominated market; not a launch region |
| KR / IN / SG | national sub-GHz plans | Already enumerated in S-04-015 table |

Existing enforcement machinery is correct and sufficient: NF-REG-04 first-boot region lock, S-04-005 channel-plan enforcement, S-04-015 PA/channel tables (US/EU/JP/KR/IN/AU), S-04-016 duty-cycle + TPC, S-24-004 region cert matrix. **One SKU covers all regions** (MM8108 is 850–950 MHz worldwide; China excluded).

## 5. Decisions (D-HALOW-01..07)

1. **D-HALOW-01 — MM8108 confirmed as the sole HaLow silicon for Alpha/Omega.** Fastest shipping silicon (43.33 Mbps/256-QAM), best integrated PA (26 dBm), mass production, WFA-certified vendor, one global SKU. Re-evaluate only at next silicon generation.
2. **D-HALOW-02 — Adopt the `morsemicro/halow` ESP Component Registry package** as the only supported ESP-IDF integration path; the deprecated `mm-iot-esp32` repo is banned from the dep-pin registry. (Story S-04-023.)
3. **D-HALOW-03 — Host-interface selection spike on ESP32-P4: SDIO 2.0 vs USB 2.0 HS.** MM8108 newly offers USB HS; P4 has native USB HS. Decide by measured throughput/power/co-existence, record as ADR. (Story S-04-023.)
4. **D-HALOW-04 — Explicit TWT + DTIM/snooze power-save engineering, RAW grouping in AP/HGW mode.** "HaLow doze" (NF-PWR-02) is now pinned to named 802.11ah mechanisms with measured targets. (Story S-04-024.)
5. **D-HALOW-05 — Wi-Fi CERTIFIED HaLow certification for every HaLow-bearing SKU.** Interop with the Morse/Newracom/M2B ecosystem becomes a certified guarantee, not a claim. (Story S-24-037.)
6. **D-HALOW-06 — Cross-vendor interop is a release gate:** HaLowLink 2 (Morse) + one Newracom NRC7394 reference AP join the HIL rack; STA/AP/802.11s association, WPA3-SAE, and throughput floors verified per release. TXW8301 interop explicitly out of scope. (Story S-22-026.)
7. **D-HALOW-07 — High-power US variant path noted for Omega/HGW** (M20-class 28.5 dBm module or external FEM via MIPI RFFE) — captured as an Omega catalog option, not an Alpha change; S-04-022 antenna-safety interlock and S-04-016 TPC apply unchanged.

## 6. What this changes in the portfolio

| Artifact | Change |
|---|---|
| `08_UNIVERSAL_CONNECTIVITY.md` bearer table | HaLow throughput corrected from "150 kbps – 15 Mbps" to MCS-honest "150 kbps – 43 Mbps (region/BW dependent)" |
| `00_MASTER_SOFTWARE_PLAN.md` §4.1 | Driver path updated to ESP Component Registry + SDIO/USB decision reference |
| EPIC-04 | + S-04-023 (component pin + host-interface spike), + S-04-024 (TWT/RAW/DTIM power-save profile) |
| EPIC-22 | + S-22-026 (cross-vendor HaLow interop bench) |
| EPIC-24 | + S-24-037 (Wi-Fi CERTIFIED HaLow certification) |
| `07_FINAL_READINESS_TRIAGE.md` §4 | + T-18 (bearer-table drift), T-19 (deprecated driver path), T-20 (WFA cert absent) — all FIXED |

Everything else already in the portfolio (region enforcement, PA tables, duty cycle, WPA3-SAE, 802.11s mesh mode, HGW AP mode, simulator radio model, HIL rack, FCC/ETSI cert plans) was audited against this research and found **current and sufficient**.

## 7. Sources

Morse Micro MM8108 launch + mass-production announcements and datasheets (morsemicro.com; CES 2025/2026; The Things Conference 2025-09), MM8108-M20 module (cnx-software.com 2026-06), HaLowLink 2 GA (prnewswire/morsemicro 2026-01), ESP Component Registry `morsemicro/halow` (components.espressif.com) + `mm-iot-esp32` deprecation notice (github.com/MorseMicro), Morse OpenWrt fork + morse-feed + 1.17.x Linux releases (community.morsemicro.com 2026-05), Newracom NRC7394/NRC5294 product pages (newracom.com), Wi-Fi Alliance CERTIFIED HaLow program (wi-fi.org 2021-11), IEEE 802.11ah-2016 → 802.11-2020/2024 rollup (IEEE/Wikipedia), range records (Tom's Hardware, ia.acs.org.au, wifinowglobal.com 2025-08), HaLow mesh field characterization (arXiv 2605.17349), RAW/TWT energy studies (PMC6603570), TXW8301 analyses (TechInsights; DEFCON32; Reticulum discussion #421), regional band data (Wi-Fi Alliance HaLow Technology Overview; GAO Tek guide), 650 Group CES 2026 HaLow assessment, Gateworks GW16167 (prnewswire 2026-01).

---

*This dossier is CC-BY-4.0. It follows the portfolio non-timeline rule — ordering and gates only, no calendar commitments.*
