<!-- SPDX-License-Identifier: CC-BY-4.0 -->

# 12 — Telephony & Universal Interop Dossier

Status: DRAFT (research-complete; ratified in principle by D-0025; implementing stories to be filed). Owner: program lead + wg-legal + wg-security. Last updated: 2026-07-13.

This dossier is the single source of truth for how SS-SP connects to the real-world phone network **and** to the wider open-communications world (Matrix, XMPP, Nostr, SIP, RCS, cellular), how a consumer can be their **own sovereign provider**, and how the paid "dedicated number" service fits the revenue model. It is the design substrate that decision **D-0025** ratifies and that the implementing stories across EPIC-10/12/14/17/18/20/21/24 draw from. It is written to the same "capability-ready even if unshipped" standard as the HaLow dossier (`08_HALOW_TECHNOLOGY_DOSSIER.md`) and the bearer-readiness principle (D-0023).

## Table of contents

- [1. Scope and non-negotiables](#1-scope-and-non-negotiables)
- [2. The one primitive: the Universal Communications Abstraction](#2-the-one-primitive-the-universal-communications-abstraction)
- [3. The Sovereignty Ladder](#3-the-sovereignty-ladder)
- [4. Capability planes](#4-capability-planes)
- [5. The AI/ML plane and division of labor](#5-the-aiml-plane-and-division-of-labor)
- [6. Security and crypto posture](#6-security-and-crypto-posture)
- [7. Regulatory posture](#7-regulatory-posture)
- [8. Revenue and business-model alignment](#8-revenue-and-business-model-alignment)
- [9. Interop verdict matrix](#9-interop-verdict-matrix)
- [10. Phasing and epic mapping](#10-phasing-and-epic-mapping)
- [11. Open questions and uncertainties](#11-open-questions-and-uncertainties)
- [12. Sources](#12-sources)

## 1. Scope and non-negotiables

Everything here is **additive and optional**. The mesh (RNS + LXMF over LoRa/HaLow/Wi-Fi/BLE) is the mandatory, sufficient core; phone-number reachability, cloud, and every external bridge are pure ADD-capability that never degrades mesh function (the `F-CL-07` never-degrade rule). These principles bind the whole dossier:

- **Optionality.** The device is fully useful with zero phone number, zero cloud, zero internet. Every feature below has a defined "works-without-it" fallback.
- **Open-source first, revenue second.** The complete sovereign path (self-host everything) ships under Apache-2.0. Paid tiers sell **convenience and operations**, never capability. "Every commercial capability strengthens the community edition, never gates it" (`07_BUSINESS_MODEL_AND_OPEN_SOURCE.md` §0).
- **The device outlives the company** (`07` §10): clean number port-out, self-host escape hatch for every hosted service, no feature ransom.
- **Sovereignty ladder, not a switch.** Users choose how much they operate themselves, from turnkey to their own ITSP — one code path, escalating control (§3).
- **One abstraction, many adapters.** We do not build N features; we build one interface with N pluggable backends and N deployment locations (§2). This is the primary defense against overcomplication.
- **Interop where standards are open, partner where regulation forces it, build native where incumbents keep it closed** (§9).

## 2. The one primitive: the Universal Communications Abstraction

To avoid a sprawl of disconnected features, all external connectivity resolves to **one abstraction** with three sub-contracts. Everything in §4–§5 is an adapter or a deployment location behind this.

### 2.1 Telephony Provider Interface (TPI)

A carrier-neutral contract every telephony backend implements:

- `send_message` / `receive_message` (SMS, MMS)
- `originate_call` / `answer_call` / `bridge_media` (voice)
- `provision_number` / `port_in` / `port_out` / `release_number`
- `send_dtmf`, `delivery_receipt`, `presence_hint`

Reference adapters (built and CI-maintained regardless of which tier ships — capability-ready per D-0023/D-0024):

| Adapter | Backend | Primary use |
|---|---|---|
| `tpi-cpaas` | Telnyx / SignalWire / Twilio / Plivo REST | Hosted number, BYO-CPaaS |
| `tpi-sip` | SIP trunk (VoIP.ms / Telnyx / Flowroute) via embedded baresip/PJSIP | Self-host voice + SMS-on-trunk |
| `tpi-xmpp` | JMP.chat / Soprani.ca (XMPP↔PSTN, open, self-hostable) | Sovereign turnkey number backend |
| `tpi-webrtc` | WHIP/WHEP + WebRTC media at gateway | Voice ingest/egress transcode |
| `tpi-esim` | MVNE / eSIM-aaS (Gigs / 1GLOBAL) + certified eUICC | Real MSISDN (partner-gated) |

The default recommended stack is **SignalWire or Telnyx** for plug-and-play CPaaS (SignalWire is the most open-adjacent — FreeSWITCH lineage, Twilio-compatible API), **VoIP.ms** for the individual self-hoster, and **JMP.chat/Soprani.ca** as the sovereignty-aligned open bridge blueprint.

### 2.2 Identity and addressing

- Native identity is the **RNS/LXMF cryptographic identity** (Ed25519+X25519, 16-byte destination hash) — the thing the user actually *owns*. A PSTN number is a *leased, portable alias*, never the root identity.
- The bridge owns an **E.164 ↔ LXMF-identity map**; mesh users reach the phone network via a `tel:` destination scheme; inbound calls/texts route to the mapped identity's LXMF inbox.
- The same alias table carries external-network addresses (`xmpp:`, `nostr:`npub, `matrix:`, `sip:`) so one contact can be reached over whichever path is available — the "universal contact" model.

### 2.3 On-device bearer manager (client-owns-handoff)

Google Fi proves the handoff intelligence lives in the **client**, not the network — carrier ATSSS/SRVCC is closed in the 5G core. So SS-SP owns a local bearer manager that selects and fails over across mesh ↔ Wi-Fi ↔ (future) cellular, modeled on the open **Multipath-QUIC** per-flow steer/switch/split concept rather than any carrier core. This gives Fi-like "one identity, many bearers" without depending on any carrier.

## 3. The Sovereignty Ladder

The organizing model for "hosted-default **and** be-your-own-provider." Same TPI code path; the user picks a rung. Rungs 2–4 are the open bridge pointed at different backends; Rung 0 is the paid convenience layer.

| Rung | Who runs the bridge | Who owns number / carrier acct | Sovereignty | User effort | Regulatory holder |
|---|---|---|---|---|---|
| **0 — SS-SP Number** (turnkey) | SS-SP cloud (EPIC-21) | SS-SP CPaaS acct | Low | Buy subscription | **SS-SP** (10DLC per user) → **text-first only** |
| **1 — BYO-number, our bridge** | SS-SP cloud | **User** (own CPaaS acct) | Medium | Paste API key | CPaaS + user |
| **2 — Self-hosted bridge** | User (Home Gateway EPIC-17 / WASM plugin EPIC-18) | User | High | Run a plugin | CPaaS + user; SS-SP = "just software" |
| **3 — Self-hosted full stack** | User (Asterisk/FreeSWITCH or Soprani.ca/JMP) + own SIP trunk | User | Very high | Run a PBX | User's carrier + user |
| **4 — Own ITSP / direct numbering** | Org/community (CLEC, MVNE deal) | Org | Maximal | Telecom operation | The org is the carrier |

The number is always **portable** (FCC-mandated LNP; choose DID providers with clean no-fee port-out such as VoIP.ms/Telnyx) so it is never hostage — the concrete form of "device outlives the company."

## 4. Capability planes

### 4.1 Messaging — native first, adopt selectively, bridge last

Verdict from the protocol landscape: **COMPETE → ADOPT → BRIDGE**, in that priority.

- **COMPETE (build first).** The native RNS+LXMF stack already beats Element/Signal/RCS on the target axes — zero infrastructure, no phone number, works at ≥5 bps, blackout/censorship-resilient — and Sideband/NomadNet prove the core. The gap vs Element is **UX, not transport**: invest in verified-identity onboarding, key backup/recovery, graceful group chat over the propagation-node store, presence-by-announce (last-seen beacons), bandwidth-adaptive media, and polished companion-app UX. This is the moat: *be the thing that works when Matrix/Signal/RCS cannot.*
- **ADOPT (R&D, not a near-term dependency).** Borrow proven ideas, not whole wire protocols: TreeKEM-style logarithmic group key agreement (from **MLS / RFC 9420**) for larger groups — but only with a partition-tolerant delivery model and **Quarantined-TreeKEM**-style offline-member handling, because raw MLS's single-Commit-per-epoch and latency-bound commit rate assume too much online coordination for LoRa. Also adopt Signal-style **sealed-sender** metadata protection and a **PQXDH/SPQR hybrid PQ ratchet** (§6). Run MLS on the **gateway**, push derived keys to nodes; there is no MCU-class MLS stack.
- **BRIDGE (opt-in reach feature, clearly bounded).** Any bridge reintroduces an always-on server and a **plaintext trust point** (crypto mismatch forces decrypt/re-encrypt), so ship each bridge as an explicit "gateway node" the operator runs **inside their own trust boundary**, labeled *not end-to-end*. Priority order by effort/precedent: **Nostr** (simplest — stateless relays, single keypair) → **XMPP** (strongest precedent: XEP-0114 components, Slidge analogue) → **Matrix** (richest but heaviest; needs a homeserver — a Rust `continuwuity` server fits an SBC/gateway, never an MCU). **Do not** market "run stock Element on the mesh" — delay-tolerance, opaque hex addressing, absent presence, and a *false E2E shield* (real crypto boundary ends at the bridge) make it misleading.

Fallback-without-it: native LXMF messaging is the baseline; every bridge is removable with zero effect on mesh messaging.

### 4.2 Voice

- **On device (MCU, all bearers):** **Codec2** (700–3200 bps, DSP-only) for LoRa-class links — the proven FreeDV/LoRa voice codec already shipping on ESP32 handhelds; **Opus 1.5/1.6** (6–24 kbps, with the LACE neural enhancer at ~0.15% CPU) as the interop anchor on Wi-Fi/cellular and the PSTN bridge.
- **Mesh-native voice:** full-duplex encrypted voice over RNS via **LXST/rnphone** interop (EPIC-14; interop tracked via S-19-027; note F-INT-04 is the *video*-call profile, not telephony — a dedicated F-TEL-* PRD requirement is to be filed) — no carrier involved.
- **Gateway (IP bearers only):** terminate voice on the Linux gateway with **WebRTC + WHIP (RFC 9725)**; transcode Opus ↔ G.711 for the PSTN leg; neural sub-kbps codecs (Mimi/EnCodec/Lyra) stay on the gateway, never the MCU.
- Fallback-without-it: text and voice-*messages* (store-and-forward Codec2/Opus clips) work with no live-voice capability and no gateway.

### 4.3 Telephony / PSTN interconnect

- **Text-first (universal, works over LoRa):** SMS/MMS ↔ LXMF, async store-and-forward. This is the hosted default and where ~90% of the "compatibility with existing infrastructure" value lives.
- **Voicemail-to-text (inbound, still LoRa-friendly):** a call to the number is answered by the gateway, transcribed on-device/edge (§5), and delivered as an LXMF message — a big capability jump with a small regulatory jump (stays off the interconnected-VoIP voice line).
- **Live voice ↔ PSTN (IP bearers, self-host/BYO only):** SIP↔LXST bridge; gated behind the operator's own carrier account for regulatory reasons (§7).
- **Provider matrix:** CPaaS (Telnyx/SignalWire/Twilio/Plivo) for turnkey; SIP trunk (VoIP.ms/Telnyx/Flowroute) for self-host; JMP/Soprani.ca as the open XMPP↔PSTN blueprint.

### 4.4 Cellular and number identity

- **Cellular = IP transport, not a number source.** Data-only IoT eSIMs (Twilio Super SIM, Hologram) give the gateway an IP uplink; the number rides VoIP on top. This matches D-0024 (cellular = costless architectural compatibility only; no modem work scheduled).
- **A real, portable MSISDN is partner-gated.** The clean route is an **MVNE / eSIM-as-a-service** deal (**Gigs** or **1GLOBAL**) plus a **GSMA-certified eUICC** (a discrete secure element, not software — feasibility on ESP32-class hardware is an open question, §11). This is a Rung-4 / roadmap option, not a v1.0 dependency.
- **VoWiFi / Wi-Fi Calling:** the protocols (IKEv2/IPsec, SIP, RTP) are open and reusable to build our own IMS-like core, but riding a *carrier's* number over VoWiFi needs that carrier's SIM/ISIM credentials — PARTNER-GATED. Build native, don't expect to bolt on.
- **RCS:** consumer P2P RCS is CLOSED to third parties (Google Messages / Apple hold the only client APIs). The E2EE layer (MLS) is worth reusing; RCS-for-Business via a certified aggregator is the only outsider path and is one-way brand→consumer. Treat RCS as a lesson, not a near-term integration.
- **Google Fi:** CLOSED product; borrow the client-owns-handoff pattern (§2.3), do not attempt integration.

## 5. The AI/ML plane and division of labor

Clean division of labor keeps this powerful without overcomplicating the constrained device. **MCU = radio + Codec2/Opus + hybrid-PQC + wakeword/VAD only. Linux gateway = the AI brain. Phone = optional NPU accelerator. Cloud = optional, never required.** A real LLM on an ESP32 is infeasible today — state that plainly and do not design around it.

Top capabilities to be next-gen while staying open-source and self-hostable:

1. **Voice codecs:** Codec2 (LoRa) + Opus 1.5/1.6 + LACE (Wi-Fi/cellular/bridge). ON-DEVICE. Non-negotiable baseline.
2. **Hybrid PQC handshake:** X25519 + ML-KEM-768, now. ON-DEVICE, MCU-benchmarked (ML-KEM-512 KEX ~36 ms on Cortex-M0+). Harvest-now-decrypt-later resistance ahead of most comms products.
3. **MLS group key agreement** on the gateway (openmls/mls-rs). EDGE-GATEWAY.
4. **On-gateway STT + TTS** via `sherpa-onnx`: Moonshine or whisper.cpp base/tiny for voicemail-to-text and live captions; Kokoro-82M (Apache-2.0) + Piper for AI-receptionist greetings and read-aloud — fully offline. EDGE-GATEWAY.
5. **Local small-LLM assistant** on the SoM (Gemma-3-4B for reliable tool/JSON output, or Qwen3) + optional phone-NPU offload: message triage, smart replies, call screening, summarization. EDGE-GATEWAY / ON-PHONE.
6. **Spam/robocall defense:** STIR/SHAKEN verification (libstirshaken + Asterisk `res_stir_shaken`) as a *weighted feature* (attestation proves origination integrity, not legitimacy — never a gate) feeding a small on-device SMS/call classifier (TF-IDF+SVM or TinyBERT). Local blocklists. Private, no cloud-reputation dependency.
7. **MASQUE (CONNECT-IP/UDP) tunnel** on the bridge — QUIC-based, HTTPS-indistinguishable, censorship-resistant transport. EDGE-GATEWAY. A genuine sovereignty differentiator.
8. **WebRTC/WHIP voice bridge** with neural-codec transcode kept on the gateway. EDGE-GATEWAY.

Hype to avoid building on: any real LLM or neural sub-kbps codec on the MCU; on-device end-to-end live speech translation (Seamless-grade is GPU/cloud — a cascaded whisper→NLLB-distilled→Piper pipeline on the gateway is the sovereignty-viable route); MLS-PQ ciphersuites and WHEP (both still IETF drafts — track, don't depend); cloud-scale robocall reputation as a "self-hosted" feature (inherently centralized).

## 6. Security and crypto posture

- **Post-quantum now:** hybrid X25519+ML-KEM-768 key exchange on device (the industry interim default, shipping in Signal PQXDH/SPQR and iMessage PQ3). ML-DSA for signatures where bandwidth allows; **SLH-DSA reserved for firmware/root signing only** (7.8–49.8 KB sigs are bandwidth-hostile) — consistent with the dual-signature-chain decision D-0022.
- **Metadata protection:** Signal-style sealed-sender / Nostr NIP-17 gift-wrap addressing for the native stack.
- **Group crypto:** MLS/TreeKEM ideas on the gateway with Quarantined-TreeKEM offline handling (§4.1).
- **The plaintext boundary is real and must be documented.** PSTN and any cross-protocol bridge are cleartext at the edge by physics/regulation — E2E encryption holds mesh-side to the subscriber identity and *ends at the bridge*. Every bridge UX must show this honestly (no false E2E shield). Retention policy and the documented plaintext boundary pass wg-security review before any bridge ships. SOS remains bearer-flood (C-05), independent of any telephony feature.

## 7. Regulatory posture

The single bright line that shapes the product: **user-originated, two-way PSTN voice = "interconnected VoIP" (FCC)** → triggers non-waivable **E911**, plus **CALEA, USF, CPNI, STIR/SHAKEN/RMD, LNP**. Text, MMS, and inbound-only / voicemail-to-text stay off that line (they attract only **A2P 10DLC**, a lighter carrier-registration regime).

Therefore:

- **The hosted offering (Rung 0) is text-first, permanently.** SS-SP is the CPaaS customer there, so hosting user-originated PSTN voice would pull E911/CALEA onto SS-SP — so we never do. Hosted = SMS/MMS + inbound voicemail-to-text.
- **Live two-way PSTN voice exists only on Rungs 1–4 (BYO/self-host).** The operator brings their own carrier account, so carrier obligations attach to the **CPaaS/SIP provider** and secondary compliance (10DLC brand, TCPA consent, E911 address entry, AUP) to the **operator** — the standard "PBX/CPE vendor is not a carrier" framing. SS-SP is neither carrier nor provider *provided* it does not hold the carrier account, provision numbers, resell connectivity, or sit as a paid media intermediary.
- **"Not for emergency calling"** is a classification/UX signal only — never a legal shield. If two-way PSTN voice is offered, E911 cannot be disclaimed away; that is precisely why hosted voice is excluded and self-host voice rides the operator's carrier E911.
- **Operational cost to flag:** hosted 10DLC is per-brand and non-portable — as an ISV, SS-SP must register each hosted-number customer's brand/campaign (one-time ~$4 brand + ~$15 campaign + ~$1.50–$10/mo). This is the main reason to offer Rung 1 (BYO, user is their own 10DLC brand) alongside Rung 0.
- **International:** UK Ofcom KYC (proof of address; UK geographic numbers need a UK address) since 27 May 2024; EU per-country geographic-number presence rules + GDPR; treat India/UAE/China/Russia and similar restricted-VoIP markets as no-go without local licensing. Per-country availability is gated by wg-legal.
- **STIR/SHAKEN:** all small-provider extensions expired in 2025; a compliant CPaaS signs calls and holds the RMD filing — staying a customer (not a provider) keeps this off SS-SP and off self-hosters.

Least-regulated posture, stated plainly: **ship open bridge/client software; text-first for anything hosted; put every voice/BYO number on the operator's own carrier account.** Maximum universality (text reaches every bearer), maximum decentralization (anyone runs a bridge), minimum regulatory exposure for the project.

## 8. Revenue and business-model alignment

Add **"Reachability / Dedicated Number"** as a *named* Tier-2 cloud revenue line (today it is buried inside "cloud subscriptions"; it deserves parity with Fleet Console / Relay):

- **SS-SP Number** — hosted text bridge + dedicated E.164, billed through the existing entitlements story (S-21-022). Price envelope benchmarked to JMP.chat (~$4.99/mo, itself open/self-hostable) and Zoleo's proven dedicated-number differentiator (`06_COMPETITIVE_LANDSCAPE.md` §2.5). Covers DID lease + carrier fees + 10DLC + margin.
- **BYO-number managed bridge (Rung 1)** — lower "compute-only" tier (we host, you supply the carrier account) — sidesteps our 10DLC burden.
- **Voice minutes / voicemail-to-text** — metered add-ons where offered.
- **MVNE / branded-cellular** — a future partner line (Gigs/1GLOBAL) if a real MSISDN + cellular bearer is pursued (Rung 4 / roadmap).
- **Professional services / grants** — help orgs stand up Rung 3/4 (fits existing lines).
- **Self-host (Rungs 2–4)** — free, Apache-2.0. This *is* the moat: it makes the paid tiers trustworthy (no lock-in) and honors "device outlives the company," including mandatory clean port-out.

Licensing follows the existing split: firmware/apps/protocol/SDKs and the self-host bridge are **Apache-2.0**; hosted cloud services are **BSL 1.1 → Apache-2.0 after 4 years** with mandatory self-host packaging — same treatment as Fleet Console / Relay / Provisioning.

## 9. Interop verdict matrix

OPEN = integrate/reuse freely · PARTNER-GATED = needs MVNE/carrier/aggregator deal · CLOSED = interop infeasible, treat as competitor/lesson.

| System / standard | Verdict | Posture |
|---|---|---|
| SIP / SIP trunking | OPEN | Register mesh gateway as a SIP UA; core interconnect path |
| XMPP (+ JMP/Soprani.ca) | OPEN | Sovereign PSTN-bridge blueprint; second bridge target |
| Nostr | OPEN | Simplest bridge; first bridge target |
| MLS (RFC 9420) | OPEN (standard) | Adopt TreeKEM ideas on gateway; track MLS-PQ drafts |
| Multipath-QUIC / MASQUE | OPEN | On-device bearer manager + censorship-resistant tunnel |
| eUICC / SGP.22–32 | PARTNER-GATED | Real MSISDN via MVNE + certified eUICC |
| WebRTC / WHIP | OPEN | Gateway voice ingest/egress |
| Matrix / Element | OPEN (heavy) | Optional bridge (last); homeserver on SBC only; never "stock Element on mesh" |
| RCS (P2P) | CLOSED | Reuse MLS lesson; RCS-for-Business only via aggregator |
| Carrier VoWiFi / IMS | PARTNER-GATED | Reuse protocol pattern; build native IMS-like core |
| Google Fi | CLOSED | Borrow client-owns-handoff pattern only |
| Emergency 911/112 | PARTNER-GATED + regulated | Never claim native without carrier/MVNO; SOS = bearer-flood |

## 10. Phasing and epic mapping

None of these gate Alpha/Omega v1.0. Each phase is independently shippable and independently removable. Story IDs below are the filed DRAFT stories (per D-0025); the TPI contract (S-21-030) is frozen, and the P1 adapter implementation is blocked on the cloud scaffold (S-21-033) + cloud CI (S-23-004) + a CPaaS sandbox (VA-29).

- **P0 — abstraction (design):** TPI contract + E.164↔LXMF addressing + `tel:` scheme + capability flags. **S-21-030** (contract frozen at `cloud/telephony-bridge/TPI_CONTRACT.md`; PRD F-TEL-01). Filed under EPIC-21 (the bridge's home) rather than EPIC-10/12/20 since the contract governs the cloud/gateway bridge; the SDK bindings remain a later EPIC-20 concern.
- **P1 — text bridge, hosted + self-host:** hosted SS-SP Number **S-21-027** (Rung 0); BYO-number managed bridge **S-21-031** (Rung 1); self-host bridge on Home Gateway **S-17-022** (Rung 2); federated messaging bridge gateway **S-18-019** (F-INT-05); number portability **S-21-032**; regulatory & compliance module **S-24-038** (10DLC/E911/per-country). Prerequisite scaffold: **S-21-033**.
- **P2 — inbound voicemail-to-text:** **S-14-020** — gateway answer + on-device/edge STT (F-TEL-05).
- **P3 — live voice ↔ PSTN (IP bearers, BYO/self-host):** **S-14-021** — SIP↔LXST bridge, Opus↔G.711 transcode (F-TEL-06); STIR/SHAKEN verify + E911-address flow carried by S-24-038.
- **Parallel — native messaging + bridges:** close the LXMF/companion-app UX gap first (EPIC-15/16/19); the Nostr → XMPP → (optional) Matrix bridge gateway is **S-18-019**.
- **Roadmap — real MSISDN / cellular:** MVNE + eUICC partner track, revisited only per D-0024's "unscheduled unless a board rev adopts a modem at no meaningful added complexity."

## 11. Open questions and uncertainties

- **eUICC on SS-SP hardware:** a GSMA-certified eUICC likely needs a discrete certified secure element, not software — feasibility on ESP32-P4-class hardware is unconfirmed. Blocks the real-MSISDN path; confirm before promising branded cellular numbers.
- **Jurisdictional 911/E112:** obligations vary by country; never market an emergency feature without a carrier/MVNO relationship and per-country legal review.
- **MLS over mesh:** raw RFC 9420's online-coordination assumptions need the Quarantined-TreeKEM adaptation; treat as R&D and validate on the mesh simulator (EPIC-22) before any dependency.
- **Draft standards:** MLS-PQ ciphersuites and WHEP are IETF drafts — track, do not build hard dependencies.
- **E2EE-RCS device rollout dates** beyond mid-2026 are provider-announced and may slip; do not plan around them.

## 12. Sources

Provider landscape: telnyx.com/pricing · signalwire.com/signalwire-vs-twilio · plivo.com/pricing · voip.ms · github.com/baresip/baresip · pjsip.org · jmp.chat · github.com/markqvist/LXST · github.com/mautrix/twilio.
Regulation: fcc.gov/consumers/guides/voip-and-911-service · legalclarity.org/fcc-voip-rules-classification-e911-and-fcc-compliance · usac.org (USF de minimis) · ndcac.fbi.gov/calea · signalwire.com A2P-10DLC guide · fcc.gov/call-authentication · twilio.com/docs/glossary/what-is-pbx · help.justcall.io (UK KYC) · voiso.com (intl VoIP legality).
Consumer telephony: en.wikipedia.org/wiki/Google_Fi_Wireless · tech-academy.amarisoft.com/ePDG_VoWiFi · gsma.com VoWiFi · thehackernews.com (RCS E2EE MLS) · developers.google.com/business-communications/rcs-business-messaging · gigs.com/api · 1global.com · bandwidth.com SIP Link · blog.3g4g.co.uk ATSSS.
AI/ML + codecs/protocols: opus-codec.org (1.5/1.6, LACE) · github.com/drowe67/codec2 · huggingface.co/kyutai/mimi · github.com/ggml-org/whisper.cpp · arxiv.org/abs/2410.15608 (Moonshine) · github.com/k2-fsa/sherpa-onnx · huggingface.co/hexgrad/Kokoro-82M · github.com/rhasspy/piper · csrc.nist.gov/pubs/fips/205/final (PQC) · datatracker.ietf.org draft-ietf-tls-ecdhe-mlkem · rfc-editor.org/info/rfc9420 (MLS) · github.com/openmls/openmls · github.com/signalwire/libstirshaken · datatracker.ietf.org/doc/rfc9725 (WHIP) · blog.cloudflare.com (MASQUE).
Messaging landscape: matrix.org · arxiv.org/html/2408.12743v2 · pistack.xyz (homeserver footprints) · reticulum.network/manual · github.com/markqvist/LXMF · github.com/markqvist/Sideband · signal.org/blog/spqr · briarproject.org/how-it-works · nips.nostr.com/17 · eprint.iacr.org/2023/1903 (Quarantined-TreeKEM).
