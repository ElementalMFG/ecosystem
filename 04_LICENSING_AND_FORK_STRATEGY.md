<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# 04 — SS-SP LICENSING & FORK STRATEGY (Commercial-Sale Grade)

> **Purpose.** This is the binding decision document that determines *what open-source components the SS-SP program will and will not adopt, fork, or link against*, given that **we are commercially selling hardware devices** containing this software. It supersedes any earlier informal statements in `00_MASTER_SOFTWARE_PLAN.md` §9.
>
> **Governing constraint.** Every SS-SP device (Lite, Alpha, Omega, and future OEM variants) is a **commercial product** sold to end-users, enterprises, and (potentially) public-safety agencies. The firmware, companion apps, cloud services, and SDKs must therefore be *licensed in a way that*:
>
> 1. Legally permits sale of physical devices that contain them.
> 2. Does not virally re-license our own proprietary work (e.g. custom AI models, enterprise fleet-management, silicon-specific tuning, DRM-adjacent features, paid cloud APIs).
> 3. Does not put us in breach of any upstream license when we ship a device to a customer.
> 4. Preserves our ability to offer *paid tiers, closed extensions, and OEM/whitelabel programs* alongside the open community edition.
> 5. Does not entangle us in patent traps, field-of-use restrictions, or "ethical use" clauses that a large fleet customer's legal team will refuse to accept.

---

## Table of contents

- [0. TL;DR — Executive Decision](#0-tldr--executive-decision)
- [1. Why This Matters (Legal Primer for the Program)](#1-why-this-matters-legal-primer-for-the-program)
- [2. Verified License Inventory of Candidate Components](#2-verified-license-inventory-of-candidate-components)
- [3. The Meshtastic Question — Full Decision Matrix](#3-the-meshtastic-question--full-decision-matrix)
- [4. The Reticulum Question](#4-the-reticulum-question)
- [5. SS-SP's Own Licensing Structure](#5-ss-sps-own-licensing-structure)
- [6. The "Do we build from scratch or reuse?" — Practical Answer](#6-the-do-we-build-from-scratch-or-reuse--practical-answer)
- [7. Contribution & IP Hygiene (mandatory for the repo)](#7-contribution--ip-hygiene-mandatory-for-the-repo)
- [8. Failure Modes We Are Explicitly Avoiding](#8-failure-modes-we-are-explicitly-avoiding)
- [9. Renewal & Review](#9-renewal--review)
- [10. Summary of Bindings for the Rest of the Program](#10-summary-of-bindings-for-the-rest-of-the-program)

## 0. TL;DR — Executive Decision

| Question | Decision |
|---|---|
| Do we **fork** Meshtastic firmware? | **NO.** GPL-3.0 would virally re-license our entire firmware and forbid proprietary extensions. |
| Do we **use** Meshtastic's `.proto` files directly? | **NO.** Those files are GPL-3.0 too. We **clean-room reimplement** the wire format under Apache-2.0. |
| Do we **interoperate** with Meshtastic devices on the air? | **YES.** Wire-protocol interoperability is not a derivative work; well-established in law and industry practice. |
| Do we **fork** the Meshtastic Android / iOS / Web apps? | **NO.** GPL-3.0 apps would prevent us from shipping a proprietary companion or a paid enterprise console. |
| Do we **use** Reticulum (RNS) and LXMF? | **YES**, with two caveats: our marketing and TOS must exclude "systems purposefully designed to do harm to human beings," and we must not use RNS/LXMF-touching data for AI/ML training. |
| Do we **fork** Sideband / Nomad (Mark Qvist's apps)? | **NO fork; may reference.** Same Reticulum License applies; build our own companion apps from scratch under Apache-2.0. |
| Firmware license | **Apache-2.0** (patent-grant, permissive, enterprise-friendly). |
| Protocol spec license | **CC-BY 4.0** (public interop; anyone can implement). |
| Companion apps license | **Apache-2.0** (open community edition) + optional **proprietary enterprise fork**. |
| Cloud / fleet console | **Business Source License 1.1 (BSL-1.1)** converting to Apache-2.0 after 4 years, *or* fully proprietary. |
| Codec2 (LGPL-2.1) | **Isolate in loadable component**, or **replace with Opus at low bitrate + our own DSP** to avoid LGPL entanglement on embedded flash. |
| Non-negotiable "kill-list" | Any GPL-3.0, AGPL-3.0, SSPL, or "ethical source" (Hippocratic, Anti-996, Do-No-Harm-2+) licensed code in the *firmware or companion apps we ship*. |

---

## 1. Why This Matters (Legal Primer for the Program)

### 1.1 What "shipping a device" means under copyleft
When we sell a device that contains software, we **distribute** that software under copyright law. Distribution triggers the outbound obligations of the software's license:

- **Permissive licenses (MIT, BSD-2/3, Apache-2.0, ISC, Zlib)** → obligation is essentially "keep the notice in the docs/about-page." No source disclosure. No re-licensing of our other code. Fine for commercial sale.
- **Weak copyleft (LGPL-2.1, LGPL-3.0, MPL-2.0)** → the *specific library* must remain user-replaceable and its source must be provided; the rest of our firmware/app is unaffected *if we respect the linkage boundary*. Doable, but constrains build system.
- **Strong copyleft (GPL-2.0, GPL-3.0)** → *any work that includes or links this code* must also be released under the same license, with **complete corresponding source code** offered to every device purchaser for at least 3 years. Our proprietary extensions become GPL. **Fatal for a commercial device with paid tiers.**
- **Network copyleft (AGPL-3.0, SSPL)** → the above, plus obligations trigger when users interact with the software *over a network*, e.g. our cloud console.  **Fatal for SaaS.**
- **"Ethical" / field-of-use licenses (Hippocratic, Do-No-Harm, JSON License "shall be used for Good, not Evil")** → uncertain enforceability, but *every* enterprise procurement will red-flag them and refuse the deal. **Effectively poison** for a B2B product.

### 1.2 GPL-3.0's specific traps for embedded
GPL-3.0 goes further than GPL-2.0 in three ways that matter to us:

1. **Anti-Tivoization (§6).** If the device restricts users from installing modified versions of the GPL software, we must ship "Installation Information" allowing them to do so. That is fundamentally at odds with **secure boot** and **signed OTA**, which are non-negotiable requirements for an enterprise pager (see `05_SECURITY_MODEL.md`).
2. **Patent grant / retaliation (§11).** Broader than Apache-2.0's; can inadvertently cross-license our silicon-partner patents.
3. **Compatibility.** GPL-3.0 code cannot be linked with GPL-2.0-only code, and cannot be linked with Apache-2.0 code *going in* to GPL, only out. This locks us into a GPL-only universe forever.

For a device that must offer **secure boot, verified OTA, tamper-resistance, and enterprise SLAs**, GPL-3.0 firmware is a hard no.

### 1.3 Interoperability is legal
Reimplementing a **wire protocol** so that our devices can talk to Meshtastic devices is:

- Protected in the US under DMCA §1201(f) (interoperability exception).
- Protected in the EU under the Software Directive (Art. 6, decompilation for interoperability) and the CJEU's *SAS v WPL* (functional aspects, programming languages, and data formats are not copyrightable).
- The industry norm: Samba↔SMB, LibreOffice↔OOXML, WireGuard↔IPsec, Signal Protocol↔MTProto bridges, etc.

**But**: we must *not* copy the Meshtastic `.proto` files verbatim into our repo. We author our own `.proto` files (or CBOR schemas) that produce the same over-the-air bytes.

---

## 2. Verified License Inventory of Candidate Components

*(All entries verified against upstream `LICENSE` files at the time of writing; each carries the URL and observed SPDX tag. This table is normative — the build system's SBOM check MUST match it.)*

### 2.1 Mesh / messaging substrate

| Component | Version target | License (verified) | Verdict |
|---|---|---|---|
| **Reticulum Network Stack (RNS)** | ≥0.9.x | Reticulum License (custom, ~MIT-like + no-harm + no-AI-training) | **ADOPT** — as required library; carry license verbatim; add TOS clause. |
| **LXMF** | latest | Reticulum License | **ADOPT** — same treatment. |
| **Sideband / Nomad** (existing RNS apps) | — | Reticulum License | **DO NOT FORK.** Study for UX inspiration only. |
| **Meshtastic firmware** | any | **GPL-3.0** | **REJECT.** Would virally infect our firmware. |
| **Meshtastic protobufs** | any | **GPL-3.0** | **REJECT** as source. We publish our own `.proto` under Apache-2.0 producing the same wire format. |
| **Meshtastic Android app** | any | **GPL-3.0** | **REJECT.** Build our own from scratch. |
| **Meshtastic iOS app** | any | **GPL-3.0** | **REJECT.** Build our own from scratch. |
| **Meshtastic Web** | any | **GPL-3.0** | **REJECT.** Build our own from scratch. |
| **MeshCore** | latest | (verify — appears BSD/MIT-family) | **CONDITIONAL** — evaluate in Phase 2 as alternate transport. |
| **T-Deck / LILYGO firmware** | any | GPL-3.0 (Meshtastic-derived) | **REJECT** for same reasons. |

### 2.2 Firmware base

| Component | License | Verdict |
|---|---|---|
| ESP-IDF | Apache-2.0 | **ADOPT.** |
| FreeRTOS | MIT | **ADOPT.** |
| LVGL (v9.x) | **MIT** | **ADOPT.** |
| mbedTLS | Apache-2.0 | **ADOPT.** |
| libsodium | ISC | **ADOPT.** |
| BearSSL | MIT | Fallback crypto option. |
| nanopb | zlib | **ADOPT** for wire encoding. |
| tinycbor | MIT | **ADOPT** for SS-Ext records. |

### 2.3 Radio & DSP

| Component | License | Verdict |
|---|---|---|
| **RadioLib** | **MIT** | **ADOPT** for SX126x driver. |
| Semtech SX126x reference driver | BSD-3-Clause | Fallback. |
| Morse Micro MM8108 HaLow driver | Vendor SDK (proprietary EULA) | **ADOPT** under NDA/vendor agreement — isolate behind HAL. |
| **Opus** | BSD-3-Clause | **ADOPT** for HaLow/BLE voice. |
| **Codec2** | **LGPL-2.1** | **CONDITIONAL** — see §4.4. Prefer to load as separate ELF/PIC blob, or replace with a low-bit Opus profile + our own vocoder tuning. |
| Meshtastic-modem (BSD FSK preamble) | MIT/BSD (verify) | ADOPT if we need Meshtastic on-air compat at PHY. |

### 2.4 On-device AI/ML

| Component | License | Verdict |
|---|---|---|
| **whisper.cpp** | MIT | **ADOPT** for STT (tiny.en / distil variants). |
| Vosk (Kaldi) | Apache-2.0 | Alternate STT for languages not covered by whisper.cpp. |
| **Piper TTS** | MIT | **ADOPT** for TTS. |
| llama.cpp | MIT | **ADOPT** for Alpha/Omega-class on-device LLM assist. |
| ONNX Runtime | MIT | **ADOPT** for portable model execution. |
| ESP-DL / ESP-NN | Apache-2.0 | **ADOPT** — Espressif's NN kernels. |

### 2.5 UI / graphics

| Component | License | Verdict |
|---|---|---|
| LVGL | MIT | **ADOPT.** |
| FreeType (with FT license) | FTL or GPL-2.0 (dual) | **ADOPT under FTL.** |
| HarfBuzz | MIT | **ADOPT** for complex script shaping (Arabic, Devanagari). |
| Google Material Symbols | Apache-2.0 | **ADOPT** — icon font. |
| Inter / Noto fonts | OFL-1.1 | **ADOPT.** OFL is font-specific, does not viral onto our firmware. |

### 2.6 Cross-platform companion apps

| Component | License | Verdict |
|---|---|---|
| Flutter | BSD-3-Clause | **ADOPT** — recommended companion runtime. |
| React Native | MIT | Alternative. |
| Tauri | MIT/Apache-2.0 | **ADOPT** for desktop console. |
| RNS Python | Reticulum License | Bundled as separate process, not linked. |
| RNS Rust (crate `reticulum`) | (verify — community port) | Preferred for embedded/mobile if available. |

### 2.7 Cloud / fleet console (optional tier)

| Component | License | Verdict |
|---|---|---|
| PostgreSQL | PostgreSQL License (MIT-like) | **ADOPT.** |
| NATS.io | Apache-2.0 | **ADOPT** for backhaul messaging. |
| Redis (post-BSL) | BSD-3 (pre-2024) / SSPL (post) / RSAL / AGPL — depends on version | **PIN to a BSD-3 tag** or use **Valkey** (BSD-3, Linux Foundation fork). |
| Grafana / Loki / Tempo | AGPL-3.0 | **REJECT** on-prem embed; use as external observability only. |
| MinIO | AGPL-3.0 | **REJECT**; use SeaweedFS (Apache-2.0) or S3-compatible instead. |
| MongoDB | SSPL | **REJECT.** |
| Elasticsearch (post-7.11) | SSPL/Elastic | **REJECT.** Use OpenSearch (Apache-2.0). |

**Anti-pattern** to avoid: silently bundling an SSPL/AGPL server "for a quick demo" then discovering at Series A that our cloud has to be re-architected.

### 2.8 The blanket kill-list (must never appear in shipped code)

- Any **GPL-2.0/-3.0-only** or **-or-later** code inside firmware or the mobile app binary.
- **AGPL-3.0** in anything network-reachable that a customer touches.
- **SSPL** in anything.
- **CC-BY-NC**, **CC-BY-NC-SA**, and any Non-Commercial CC variant.
- **JSON License** ("shall be used for Good, not Evil") — old, but still shows up.
- **Hippocratic License**, **Do No Harm 2.x**, **Anti-996**, **Commons Clause**, **BUSL-1.1 (without a defined change date to a permissive license)**, **Elastic License 2.0**.
- Anything with a **field-of-use restriction naming a customer type** (e.g. "not for use by law enforcement").
- Anything with a **"dominant-use" clause** or **"contact us for commercial use"** wording.

The CI SBOM job (see `ci/sbom-check.yaml`) MUST fail the build if any dependency reports these SPDX identifiers.

---

## 3. The Meshtastic Question — Full Decision Matrix

We evaluated **six** possible relationships to Meshtastic. Each was scored on: legal safety for commercial sale, speed to first product, technical fit, community perception, and long-term optionality.

| Option | Description | Legal | Speed | Fit | Community | Optionality | **Score** |
|---|---|---|---|---|---|---|---|
| **A. Fork the firmware whole** | Take `meshtastic/firmware`, rebrand, extend. | ❌ GPL viral | ✅ | ⚠ constrained | 😐 mixed | ❌ locked to GPL | **REJECT** |
| **B. Rebase our own firmware onto Meshtastic protobufs** | Author our firmware; use their `.proto` files. | ❌ .proto files are GPL-3.0 | ✅ | ✅ | 😐 | ❌ | **REJECT** |
| **C. Fork mobile apps only, own firmware** | Firmware Apache-2.0, apps forked as GPL-3.0. | ⚠ apps stay GPL, no enterprise paid tier possible for app | ⚠ | ✅ | ✅ | ⚠ | **CONDITIONAL** |
| **D. Clean-room protocol compat, own everything** | Reimplement wire format; own firmware and apps. | ✅ | ⚠ | ✅ | ✅ | ✅ | **✅ RECOMMENDED** |
| **E. Ignore Meshtastic entirely** | Reticulum-only. | ✅ | ✅ | ⚠ interop gap | ⚠ | ✅ | **PARTIAL** |
| **F. Dual-radio bridging device only** | Never speak Meshtastic protocol in firmware; use a companion "bridge" node running Meshtastic FW on a separate MCU. | ✅ | ⚠ | ⚠ | ✅ | ✅ | **BACKUP** |

**Chosen path: D, with F as a "gateway product" fallback for regions where clean-room compat isn't yet ready.**

### 3.1 What "clean-room" concretely means for SS-SP

1. **Wire format documentation.** We derive the byte layout from:
   - The public Meshtastic docs (meshtastic.org).
   - Passive on-air captures (SDR + firmware in monitor mode).
   - The Meshtastic user manual, blog posts, and video demos.
   - **NOT** by reading or copying the `.proto` files in `meshtastic/protobufs`.
2. **Two-team clean-room** (best practice, though not legally required for wire-format compat):
   - **Spec team** reads only Meshtastic docs + external analyses, produces `docs/wire/meshtastic-compat.md` in our own words.
   - **Impl team** implements from that doc without ever opening the GPL source.
   - Record both teams in `docs/wire/CLEANROOM_LOG.md`.
3. **Our own `.proto` and CBOR files** in `protocol/ss/`, licensed Apache-2.0.
4. **Test corpus**: a directory of hex byte-strings observed on-air, with expected decoded fields, so we prove compat empirically without ever executing GPL code.
5. **No copy-paste from Meshtastic issue trackers.** Contributor guide (see §7) forbids pasting GPL code snippets into PRs.

### 3.2 What we *can* freely do
- Talk to Meshtastic devices over LoRa, BLE, and Wi-Fi.
- Publish our compat documentation for others to use.
- Contribute *back* upstream to Meshtastic where useful (individual contributions are subject to their GPL/CLA when contributed there, but that does not backflow to us).
- Cite Meshtastic in our marketing as an interoperability partner (respecting their trademark).

### 3.3 Trademark
"Meshtastic" is a **registered trademark of Meshtastic LLC**. We must **not**:
- Call our device a "Meshtastic device."
- Use the Meshtastic logo on packaging or firmware boot screens.
- Advertise "Meshtastic-certified" without their written permission.

We **may** say: "SS-SP interoperates with Meshtastic (v2.x) LoRa mesh networks." That is factual, comparative, and protected as nominative fair use.

---

## 4. The Reticulum Question

### 4.1 The two clauses that matter
The Reticulum License permits use, sale, modification, sublicensing, distribution — but adds:

1. **"The Software shall not be used in any kind of system which includes amongst its functions the ability to purposefully do harm to human beings."**
2. **"The Software shall not be used, directly or indirectly, in the creation of an artificial intelligence, machine learning or language model training dataset."**

### 4.2 Impact on our business plan
- **Civilian, enterprise, industrial, humanitarian, first-responder, outdoor/adventure, event, campus** markets: **✅ fully compatible.**
- **Public-safety agencies whose function is *response* (fire, EMS, SAR)**: ✅ compatible — response is not "purposefully doing harm."
- **Law enforcement (patrol, community policing)**: ⚠ **case-by-case.** We interpret "purposefully do harm" as *offensive systems whose primary intended output is human harm*, not incidental use by any organization that may in extreme cases use force. Best practice: get a written interpretation from Mark Qvist (upstream author) or from independent counsel before pursuing LE contracts.
- **Weapons systems, military targeting, autonomous lethal systems**: ❌ prohibited. Do not pursue.
- **Border enforcement, ICE-style operations, private-military**: ❌ prohibited or high-risk; do not pursue.
- **Feeding user chat/voice/location into AI training pipelines**: ❌ prohibited. This constrains us but *aligns with our own privacy posture* — we would not do it anyway.

### 4.3 Concrete controls we will implement
- **Acceptable Use Policy** in the End-User License Agreement forbidding use to purposefully harm humans.
- **Sales-qualification checklist** for any B2G/defense inquiry; escalate to counsel.
- **Data-processing policy** stating no user data crosses into AI training corpora; enforce by architecture (no shipping of user messages to any training pipeline; logs anonymized to on-device counters only).
- **Third-party model catalog** for on-device AI marks each model with training-data provenance so we can attest none were trained on RNS-derived data.

### 4.4 Reticulum runtime licensing on device
RNS is currently primarily distributed as a Python library. On ESP32-S3/P4 we will either:

- (a) Port a **Rust or C reimplementation** of the RNS packet layer (there is a community `reticulum-rs` in progress; our SDK team will evaluate/contribute). If we author it clean-room from the RNS protocol spec, we can license our port Apache-2.0 while still being wire-compatible.
- (b) Run a stripped **MicroPython + RNS** on Alpha/Omega where the P4 has resources; keep the "no-harm/no-AI-training" clause satisfied by TOS.

Either way, our device firmware code we write ourselves is Apache-2.0; the RNS runtime is credited with its license notice.

### 4.5 Codec2 (LGPL-2.1)
LGPL-2.1 requires that the user be able to re-link a modified codec2 against our binary. On a locked-down secure-boot ESP32, this is **problematic**. Options:

- **(i)** Compile Codec2 as a **dynamically-loadable component** (ESP-IDF component-load or an ELF loader) so the user can swap it; **preferred**.
- **(ii)** Ship the **compiled object files** for Codec2 separately with instructions for re-linking against our public firmware ABI; acceptable per LGPL.
- **(iii)** Skip Codec2 entirely for Lite; use a very-low-bitrate Opus profile (~6 kbps) for HaLow/BLE voice; for LoRa voice on Lite, disable it (Lite has the mic-vs-LoRa mux constraint anyway, see `01_...HARDWARE_REFERENCE.md`).

We choose **(iii) for Lite v1** (skip Codec2), **(i) for Alpha/Omega** (loadable module, so LGPL stays clean).

---

## 5. SS-SP's Own Licensing Structure

### 5.1 The community edition (all shipped devices boot into this)
- **Firmware:** Apache-2.0.
- **HAL, SDK, sample apps, board configs:** Apache-2.0.
- **Protocol specs and `.proto`/CBOR files:** **CC-BY 4.0** (documents) + **Apache-2.0** (code).
- **Companion mobile/desktop apps (community edition):** Apache-2.0.
- **Fonts/artwork:** OFL-1.1 (fonts), CC-BY 4.0 (default themes).
- **Documentation site:** CC-BY 4.0.
- **Trademarks:** "SS-SP", "Seekie-Speakie", the shield/dot logo — **retained** by the vendor entity (see `06_GOVERNANCE.md`). Users get an OSS-usage grant for unmodified builds only; any modified or third-party build must remove trademarks.
- **Contributor License Agreement:** Apache Individual CLA + Corporate CLA (based on ASF template). Ensures we can relicense contributions in the future (e.g. into a foundation) and grants patent protection.
- **DCO (Developer Certificate of Origin)** required on every commit as a lightweight parallel signal.

### 5.2 Optional proprietary tiers (do not ship on device by default)
- **SS-SP Enterprise Fleet Console** (cloud + on-prem): **BSL 1.1** with change date to Apache-2.0 after 4 years; or, for the closed premium build, fully proprietary.
- **Vendor-signed enterprise plugins**: proprietary, distributed as signed images loaded through the plugin sandbox (see `05_SECURITY_MODEL.md`).
- **OEM/whitelabel firmware images**: proprietary customization on top of the Apache-2.0 base, permitted because the base is permissive.

### 5.3 Why Apache-2.0 specifically (not MIT, BSD, or MPL)
- **Explicit patent grant** — critical when we accept contributions from silicon vendors (Espressif, Morse Micro, Semtech) or from enterprise contributors.
- **Patent-retaliation clause** — deters patent trolls; permissive licenses without it (MIT/BSD) leave us more exposed.
- **NOTICE file requirement** — machine-readable attribution scales to hundreds of dependencies.
- **Enterprise-familiar** — big customers' legal teams already have Apache-2.0 approved; MIT/BSD are also easy, but Apache is the strongest permissive.
- **Foundation-ready** — Apache Software Foundation, Linux Foundation, and Eclipse Foundation all accept Apache-2.0 projects without relicensing.

### 5.4 SPDX everywhere
Every source file starts with:
```c
// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
```
CI enforces via `reuse-tool` and `licensecheck`. SBOMs are generated per release using SPDX 2.3 + CycloneDX 1.5.

---

## 6. The "Do we build from scratch or reuse?" — Practical Answer

The user asked whether starting from scratch is realistic. Here is the honest engineering answer:

### 6.1 What we WILL reuse (large scope reduction)
- **Espressif ESP-IDF** — MCU RTOS, drivers, Wi-Fi/BLE stacks. Saves ~1.5 person-years vs. from-scratch. Apache-2.0.
- **LVGL** — battle-tested embedded GUI. Saves ~1 person-year vs. from-scratch. MIT.
- **mbedTLS + libsodium** — cryptography. Saves ~1 person-year and avoids self-rolled-crypto disasters. Apache-2.0 / ISC.
- **RadioLib** — SX126x driver. Saves months. MIT.
- **Morse Micro HaLow driver** — mandatory (vendor-supplied). Vendor EULA.
- **Opus** — audio codec. Saves years. BSD-3.
- **whisper.cpp, Piper, llama.cpp** — AI runtimes. Saves years. MIT.
- **Reticulum protocol** — as a *specification we implement against* (and possibly the reference Python runtime on Alpha/Omega where it fits). Reticulum License, with TOS controls.
- **Meshtastic protocol** — as a *wire format we interoperate with*, clean-room reimplemented.

### 6.2 What we WILL write from scratch
- **SS-SP firmware application layer** — power, state machines, UI, provisioning, fleet, mux control, feature routing. Apache-2.0.
- **SS-Ext CBOR record schemas** and encoder/decoder. Apache-2.0.
- **Meshtastic wire compat** implementation (clean-room). Apache-2.0.
- **HAL contracts** and per-board drivers (Lite / Alpha / Omega / OEM). Apache-2.0.
- **SS-SP UI framework** (`ss_ui`) — layout descriptors, themes, templates on top of LVGL. Apache-2.0.
- **Companion apps** — Flutter mobile (iOS/Android), Tauri desktop, React web console. Apache-2.0 for community edition.
- **Cloud fleet console** — Rust or Go services, Postgres/NATS/Valkey backing. BSL-1.1 or proprietary.
- **Provisioning & OTA infrastructure** — signing service, artifact CDN. Proprietary (release manifests are signed with our keys; software is Apache-2.0).

### 6.3 What we WILL NOT invent
- We will not invent our own crypto primitives.
- We will not invent our own bootloader (use ESP-IDF's + signed image chain).
- We will not invent our own filesystem (SPIFFS, LittleFS, FATFS).
- We will not invent our own font-shaping engine (HarfBuzz + FreeType).
- We will not invent our own compressed voice codec from scratch (Opus).
- We will not invent our own AI inference runtime (ONNX Runtime / llama.cpp / whisper.cpp).

### 6.4 Effective effort
By reusing the permissive stack in §6.1 and writing only §6.2, first-product effort for Lite v1 is bounded — the *net-new* code is on the order of tens of thousands of lines, not hundreds of thousands. This is the "practical, effective, realistic" answer to the user's question.

---

## 7. Contribution & IP Hygiene (mandatory for the repo)

To keep the codebase legally clean forever, we adopt these ground rules from day one:

1. **CLA + DCO required on every PR.** Enforced by the `cla-assistant` GitHub App.
2. **Automated license scan** on every PR: `reuse lint`, `licensecheck`, `scancode-toolkit`, `syft`. Any new dependency with a license not on the pre-approved allowlist fails the build and requires a governance-review issue.
3. **SBOM published with every release** (SPDX + CycloneDX). Signed with our release key. Kept publicly for at least 7 years.
4. **Header check.** Every source file must have `SPDX-License-Identifier:` on line 1 or 2. Pre-commit hook.
5. **"No-copy" rule.** Contributor guide explicitly forbids copying code from GPL/AGPL/SSPL sources into the repo. Reviewers instructed to challenge any unexplained large block.
6. **Vendor SDK isolation.** Proprietary vendor SDKs (Morse Micro, potentially u-blox, potentially Bosch) live in `vendor/<vendor>/` behind our HAL and are excluded from the community-edition release image unless the vendor grants us redistribution rights. If they do not, we ship binary blobs under the vendor's own EULA and document exactly what is proprietary.
7. **Font & asset provenance.** `assets/PROVENANCE.md` lists every font, icon, sound, image, with its license and source URL. No stock assets without traceable license.
8. **AI-generated code disclosure.** If a contributor uses AI-assisted coding for a PR, they must indicate the tool in the PR description and confirm the output is not verbatim from a copyleft training-set memorized snippet (`Copyright Notice: Contributor asserts this contribution is either original or reproduced from clearly permissible sources.`).
9. **Trademark policy.** Documented at `docs/TRADEMARK.md`. Third-party forks must rebrand.
10. **Export control.** Firmware includes strong crypto → the release notes and the source ship a NOTICE of ECCN classification (self-classified 5D002 mass-market crypto per Category 5 Part 2, EAR §740.17). We will file the appropriate self-classification / TSU notification before the first commercial shipment.

---

## 8. Failure Modes We Are Explicitly Avoiding

We list these so future contributors understand *why* certain shortcuts are forbidden:

- **Silent GPL contamination.** A contributor grabs a "cool little library" from GitHub, drops it in, ships firmware. Product is now GPL-3.0 by law. Cure: SBOM + CI license scan blocks it at PR.
- **The Codec2 static-link disaster.** LGPL requires re-linkability. If we statically link Codec2 into a secure-boot-locked image without providing object files, we are in breach. Cure: loadable module, or omit.
- **The AGPL cloud trap.** We spin up a "quick demo" on a MongoDB or Grafana Enterprise instance; someone points to it from the customer TOS; now we owe source. Cure: allowlist.
- **The "ethical license" B2B rejection.** A code path uses a Hippocratic-licensed library; enterprise legal refuses procurement. Cure: allowlist.
- **The Reticulum clause mismatch.** Sales pursues a defense-contractor lead; we breach RNS's no-harm clause. Cure: sales checklist + counsel review before any non-civilian pursuit.
- **Trademark drift.** Community fork ships as "SS-SP" and creates support-load / reputation damage. Cure: trademark policy + brand-guard tooling.
- **AI-model provenance.** Community contributor drops a model trained on scraped copyrighted content; we ship it. Cure: model manifest `models/CATALOG.md` with training-data provenance for every included model.

---

## 9. Renewal & Review

This document is reviewed:

- On every major upstream license change of a used component (auto-detected by SBOM diff CI).
- At each major release cadence (v1.0, v1.5, v2.0, …).
- Whenever we enter a new market segment (B2G, healthcare, financial, ...).
- Whenever a governance body (foundation, board) transition occurs.

Changes require: RFC → Steering Committee sign-off → legal review → merge (see `06_GOVERNANCE.md`).

---

## 10. Summary of Bindings for the Rest of the Program

| Downstream doc / artifact | Constraint from this doc |
|---|---|
| `firmware/**` | Apache-2.0. No GPL/AGPL deps. Loadable modules for weak-copyleft. |
| `protocol/ss/**` | Apache-2.0 code + CC-BY 4.0 spec. Clean-room compat only for foreign protocols. |
| `protocol/foreign/meshtastic/**` | Only the *wire spec* file + our own `.proto`. Never a `git subtree` from `meshtastic/*`. |
| `companion/mobile/**` | Apache-2.0 community edition. |
| `companion/desktop/**` | Apache-2.0 community edition. |
| `cloud/**` | BSL-1.1 or proprietary. Never AGPL/SSPL server deps. |
| `models/**` | Every model has a `PROVENANCE.md` entry and license tag. |
| `vendor/**` | Isolated proprietary SDKs; excluded from community image unless redistributable. |
| CI | Enforces SPDX headers, license scan, SBOM, DCO, CLA, no-copy checks. |
| Sales | Follows AUP + no-harm checklist for RNS/LXMF. |
| Marketing | Nominative-fair-use rules for Meshtastic references; no trademark misuse. |

**This is the licensing floor. Anything below it does not ship.**
