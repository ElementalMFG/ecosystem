<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# EPIC-24 — Stories

Format per `../../00_METHODOLOGY.md` §2.7. Meta lines are machine-parsed.

### S-24-001 — FCC 15.247 test plan (Lite)
As a compliance officer I want an FCC 15.247 test plan for the Lite SKU so that its LoRa and 2.4 GHz radios are certified before US sale.
- AC: test plan covers 15.247 for the SX1262 LoRa path and 2.4 GHz Wi-Fi/BLE; an accredited lab is selected with EMC pre-scan scheduled at ID-freeze (risk R24-01); FCC approval is on file before the M2 Lite ship gate (epic exit criterion 1)
- Meta: Shard=A | Type=Compliance | Size=L | Prio=P0 | Status=DRAFT | SKU=L | PRD=NF-REG-01 | Const=C-07

### S-24-002 — CE / ETSI EN 300 220 test plan
As a compliance officer I want a CE RED test plan under ETSI EN 300 220 so that the 868 MHz LoRa radio is certifiable for EU sale.
- AC: plan covers EN 300 220 including EU 868 duty-cycle limits per NF-REG-05; RED essential requirements are mapped to concrete evidence items; lab slots are booked with pre-test on EVT hardware
- Meta: Shard=A | Type=Compliance | Size=L | Prio=P0 | Status=DRAFT | SKU=L | PRD=NF-REG-02, NF-REG-05 | Const=C-07

### S-24-003 — ETSI EN 300 328 (Wi-Fi 2.4)
As a compliance officer I want EN 300 328 testing planned for the 2.4 GHz Wi-Fi/BLE radios so that the EU declaration covers every bearer on every SKU.
- AC: adaptivity and occupied-spectrum tests are planned for 2.4 GHz Wi-Fi and BLE; the evidence pack integrates into the overall RED declaration of conformity; pre-testing is run on EVT hardware before formal submission
- Meta: Shard=A | Type=Compliance | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=NF-REG-02 | Const=C-07

### S-24-004 — HaLow region certs (Alpha)
As a compliance officer I want per-region HaLow certification mapped for the Alpha SKU so that HaLow ships only where it is legal.
- AC: region matrix enumerates HaLow cert requirements (FCC 15.247/15.407, ETSI, and per-market equivalents) for each launch region; dependency on first-boot region-code enforcement (NF-REG-04) is documented for firmware; lab plan and cost per region are approved
- Meta: Shard=A | Type=Compliance | Size=L | Prio=P0 | Status=DRAFT | SKU=A | PRD=NF-REG-01, NF-REG-02, NF-REG-04 | Const=C-07

### S-24-005 — LTE-M modem cert reuse (Omega)
As a compliance officer I want the LTE-M modem module's existing certifications inventoried and reused so that Omega avoids redundant cellular testing.
- AC: module pre-certifications (FCC, PTCRB, carrier) are inventoried with reuse conditions confirmed in writing; the end-product delta-testing scope is defined; the carrier-approval path is documented per target market
- Meta: Shard=A | Type=Compliance | Size=M | Prio=P0 | Status=BLOCKED | SKU=O | PRD=NF-REG-01, NF-REG-02 | Const=C-07
- Tasks: spec · design · impl · test · docs — all pending Omega rev-2 hardware
- Deps: external: BLOCKED 2026-07-09 (D-0020): cellular/satellite modem absent from the signed-off Omega v1.0 board (PCB release v69 — no expansion interface, respin required); capability preserved for Omega rev-2 — see `docs/dev/OMEGA_HW_BASELINE.md`

### S-24-006 — Iridium / NTN cert plan (Omega optional)
As a compliance officer I want a certification plan scoped for the optional Iridium/NTN bearer so that a satellite go/no-go decision is grounded in real cost and schedule.
- AC: regulatory and operator certification path is scoped per target market; cost and lead-time estimate supports a documented go/no-go decision; RF co-existence test plan with the other Omega radios is drafted
- Meta: Shard=A | Type=Compliance | Size=L | Prio=P1 | Status=BLOCKED | SKU=O | PRD=NF-REG-01 | Const=C-07
- Tasks: spec · design · impl · test · docs — all pending Omega rev-2 hardware
- Deps: external: BLOCKED 2026-07-09 (D-0020): cellular/satellite modem absent from the signed-off Omega v1.0 board (PCB release v69 — no expansion interface, respin required); capability preserved for Omega rev-2 — see `docs/dev/OMEGA_HW_BASELINE.md`

### S-24-007 — UL 62368-1 safety cert
As a compliance officer I want UL/IEC 62368-1 safety certification planned so that the battery-powered hardware is legally sellable as consumer electronics.
- AC: safety test plan covers 62368-1 for battery, charging, and thermal cases; critical-components list uses certified parts with certificates collected; lab quote and schedule are approved before DVT; binds to S-05-028 rolling-60s TX duty ≤40% + soak protocol (43 °C plastic / 51 °C metal skin-temp ceiling)
- Meta: Shard=B | Type=Compliance | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-07

### S-24-008 — RoHS 3 / REACH declaration
As a compliance officer I want RoHS 3 and REACH declarations assembled from the full BoM so that EU environmental obligations are met at launch.
- AC: supplier declarations cover the full BoM for RoHS 3 and REACH SVHC; a declaration of conformity is drafted and reviewed; a re-check process is defined for any BoM change
- Meta: Shard=C | Type=Compliance | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-07

### S-24-009 — WEEE registration per region
As a compliance officer I want WEEE producer registration completed per launch EU region so that devices can be legally placed on those markets.
- AC: WEEE registration completed for each launch EU country with producer numbers recorded; recycling-marking requirements are passed to packaging/ID; annual reporting obligations are calendared
- Meta: Shard=C | Type=Compliance | Size=S | Prio=P1 | Status=DRAFT | SKU=★ | PRD=— | Const=C-07

### S-24-010 — GDPR DPA template
As legal counsel I want a GDPR DPA template for enterprise customers so that Fleet Console deals can close without bespoke privacy negotiation.
- AC: template covers processor obligations, sub-processor list, and international-transfer mechanism (SCCs); template aligns with the cloud data flows documented in EPIC-21 (S-21-018); wg-legal approval is recorded
- Meta: Shard=D | Type=Compliance | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=NF-PRIV-04 | Const=C-07

### S-24-011 — CCPA notice
As legal counsel I want a CCPA/CPRA privacy notice so that California users receive legally required disclosures.
- AC: notice covers categories collected, purposes, and opt-out rights; notice is published on the website and linked from the companion apps; wg-legal sign-off is recorded
- Meta: Shard=D | Type=Compliance | Size=S | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-07

### S-24-012 — DSR fulfillment runbook (access/delete/port)
As a support lead I want a data-subject-request runbook so that access, deletion, and portability requests are fulfilled inside the legal deadline every time.
- AC: a staging drill fulfils access, delete, and port requests end-to-end within the ≤ 30 day NF-PRIV-04 deadline; a requester identity-verification step is defined; every request and outcome is logged auditable
- Meta: Shard=D | Type=Compliance | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=NF-PRIV-04 | Const=C-07

### S-24-013 — CRA SBOM & vuln SLA policy
As a compliance officer I want a CRA-readiness policy tying per-artifact SBOMs to a published vulnerability-handling SLA so that EU Cyber Resilience Act obligations are demonstrably met.
- AC: SBOM obligation maps to the EPIC-23 pipeline output (S-23-007) for every shipped artifact; vuln SLA of 24 h ack / 7 d triage / 30 d fix-or-mitigation is published; the CRA incident-reporting procedure (including ENISA notification duty) is documented
- Meta: Shard=E | Type=Compliance | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=NF-REG-03, NF-SEC-05 | Const=C-SEC, C-OA

### S-24-014 — EoL / end-of-support policy
As a compliance officer I want a published end-of-life and end-of-support policy so that the 5-year security-fix commitment is contractual, not aspirational.
- AC: policy commits to security fixes for 5 years post last shipment per NF-SUS-01 and the Open Assurance pledge; EoL announcement process and post-EoS device behaviour are documented; policy is published on the support portal
- Meta: Shard=E | Type=Compliance | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=NF-SUS-01 | Const=C-07, C-OA

### S-24-015 — Warranty policy (12/24 months)
As legal counsel I want a 12/24-month warranty policy drafted per region so that consumer-law obligations are met and support knows what is covered.
- AC: warranty terms drafted for each launch region reflecting local consumer law minimums; policy defines treatment of user-flashed firmware without voiding statutory rights; policy is published before first sale
- Meta: Shard=F | Type=Task | Size=S | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-07

### S-24-016 — RMA process + return portal
As a support lead I want an RMA process with a return portal so that defective devices flow back, get re-provisioned, and return to customers traceably.
- AC: portal issues RMA numbers and tracks return state end-to-end; returned units go through the field re-provisioning ceremony per F-MFG-03 with user-data handling defined; RMA turnaround time is measured and reported
- Meta: Shard=F | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-MFG-03 | Const=C-07

### S-24-017 — Support portal launch (docs.ss-sp / support.ss-sp)
As a support lead I want a docs + ticketing support portal live so that customers self-serve first and reach humans within the published SLAs.
- AC: docs and ticketing are live at the support domains; ticket routing enforces the tier SLAs defined in S-24-018; knowledge base is seeded with the top provisioning, pairing, and OTA issues (epic exit criterion 4)
- Meta: Shard=G | Type=Ops | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-07

### S-24-018 — SLA definitions: community 72 h, enterprise 4 h
As a support lead I want published support SLAs (community 72 h, enterprise 4 h first response) so that expectations are explicit for every tier.
- AC: SLA document defines first-response and escalation targets per tier; SLA attainment measurement and reporting are defined; the escalation path to engineering WGs is documented
- Meta: Shard=I | Type=Docs | Size=S | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-07

### S-24-019 — Marketing site + brand kit
As a community manager I want a marketing site and brand kit so that the product story is public and third parties use the brand correctly.
- AC: marketing site is live with per-SKU pages and honest capability claims reviewed by wg-legal; brand kit usage terms match the trademark policy (risk R24-03); site analytics are privacy-respecting with no third-party trackers by default
- Meta: Shard=H | Type=Task | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-07, C-TM

### S-24-020 — Sales enablement (deck, pricing sheet, order form)
As a support lead I want sales-enablement materials (deck, pricing sheet, order form) so that enterprise deals proceed without founder involvement.
- AC: deck, pricing sheet, and order form exist with claims and pricing approved by wg-legal; pricing is consistent with the business-model tiers in C-07; assets are versioned in the repo with an owner
- Meta: Shard=H | Type=Task | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-07

### S-24-021 — Foundation shortlist RFC (RFC-0002 execution)
As a community manager I want the foundation-shortlist RFC executed so that the Phase-2 neutral home for trademarks and infrastructure is decided in the open.
- AC: RFC lists candidate foundations scored against published criteria; a community comment period runs per the C-06 RFC process; the decision is recorded as ACCEPTED with rationale
- Meta: Shard=J | Type=RFC | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-06, C-07

### S-24-022 — IP escrow contract execution
As legal counsel I want the IP escrow contract executed so that users are protected even if the founding vendor disappears before foundation transfer.
- AC: escrow instrument is executed per the Open Assurance commitments in `governance/OPEN_ASSURANCE.md`; the deposit includes source, build instructions, and key-ceremony documentation; release triggers are legally reviewed and tested in a paper exercise
- Meta: Shard=J | Type=Task | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-OA, C-06

### S-24-023 — Foundation transfer legal package
As legal counsel I want the complete foundation-transfer legal package so that trademarks, domains, infrastructure, and key custody can move to the neutral foundation at the Phase-2 gate.
- AC: package covers trademarks, domains, infrastructure accounts, and signing-key custody transfer; contract terms are reviewed by outside counsel; an execution checklist is gated on the Phase-2 gate (epic exit criterion 5)
- Meta: Shard=J | Type=Task | Size=XL | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-06, C-TM, C-OA

### S-24-024 — Trademark policing runbook + templates
As legal counsel I want a trademark-policing runbook with response templates so that dilution and misuse (risk R24-03) get a fast, proportionate response.
- AC: runbook defines monitoring sources, an escalation ladder, and cease-and-desist templates; permitted community uses are cross-referenced to the C-TM policy so legitimate forks are not harassed; a response SLA per infringement class is defined
- Meta: Shard=K | Type=Task | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-TM

### S-24-025 — Certified-partner cert program launch
As a compliance officer I want the SS-SP-Certified partner program launched so that third-party manufacturers can ship interoperable, badge-carrying devices.
- AC: public interop test suite is published per F-CERT-01; the security floor gate (secure boot, dual-sig OTA, no backdoors) is enforced per F-CERT-02; certification portal accepts applications (F-CERT-03) and the revocation process is documented (F-CERT-04)
- Meta: Shard=K | Type=Feature | Size=L | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-CERT-01, F-CERT-02, F-CERT-03, F-CERT-04 | Const=C-TM, C-07

### S-24-026 — Customer advisory board (formation)
As a community manager I want a customer advisory board formed so that roadmap decisions hear directly from real deployments.
- AC: charter defines membership, meeting cadence, and confidentiality terms; founding members span at least three personas (e.g. SAR, enterprise fleet, neighbourhood enthusiast); a feedback loop into the PRD amendment process is documented
- Meta: Shard=— | Type=Task | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=— | Const=C-07

### S-24-027 — Vuln disclosure portal (hackerone or self-host)
As a compliance officer I want a vulnerability-disclosure portal live so that researchers have a working intake matching our published security policy.
- AC: disclosure intake (HackerOne or self-hosted) is live and linked from `SECURITY.md`; the 24 h acknowledgement KPI is wired to an on-call rotation; safe-harbor language for good-faith research is published
- Meta: Shard=E | Type=Ops | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=NF-REG-03 | Const=C-SEC

### S-24-028 — Security bug bounty program
As a compliance officer I want a paid bug-bounty program so that external researchers are incentivised to report rather than hoard vulnerabilities.
- AC: scope and reward tiers are published and consistent with the disclosure policy in `SECURITY.md`; the payout process is legal-reviewed for sanctions/tax constraints; program metrics (reports, time-to-fix, payouts) are reported quarterly
- Meta: Shard=E | Type=Task | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=— | Const=C-SEC, C-07

### S-24-029 — Tactical/prosumer MANET segment positioning
As a product manager I want a positioning analysis against tactical/prosumer MANET offerings (MorosX-class nodes, Doodle Labs, Meshtastic gateways) so that SS-SP's open-standards interop (TAK/CoT, NMEA, IP tether) is marketed accurately and priced correctly for that segment.
- AC: analysis maps SS-SP capabilities and gaps against at least three named MANET/EUD products with sourced specs; positioning states the open-assurance differentiators (auditable firmware, RFC-governed interop, no proprietary clients) without overclaiming radio performance; recommendations feed the pricing/SKU review and are recorded with wg-business sign-off
- Meta: Shard=— | Type=Task | Size=S | Prio=P2 | Status=DRAFT | SKU=★ | PRD=— | Const=C-07

### S-24-030 — Ecosystem programme + store-policy review (node modes, Auto/CarPlay, Connect IQ, vehicle OEM)
As a product manager I want a legal/policy review of every ecosystem channel the app-node strategy touches so that F-NODE and F-ECO features sequence against real programme constraints instead of assumptions.
- AC: review covers Play/App Store policies for background transport and gateway roles (foreground services, VoIP entitlements, background BLE), Android Auto/CarPlay programme rules, Garmin Connect IQ terms, and at least one vehicle-OEM integration option (Ride Command-class) with a go/no-go and constraints per channel; findings are recorded with wg-legal and wg-business sign-off; F-NODE/F-ECO story sequencing is updated to reflect the findings
- Meta: Shard=— | Type=Task | Size=S | Prio=P2 | Status=DRAFT | SKU=★ | PRD=F-ECO-01, F-ECO-02 | Const=C-07

### S-24-031 — BOM cost & cloud margin tracking
As a programme manager I want per-SKU BOM cost and cloud gross-margin tracked against the NF-COST targets so that design decisions are checked against Lite ≤ USD 60, Alpha ≤ USD 120, and cloud margin ≥ 40 %.
- AC: a costed BOM per SKU is maintained in-repo and updated at every hardware-affecting design change; CI or review checklist flags any change that pushes a SKU past its NF-COST ceiling at target volume; cloud unit-economics model (per 10 k enrolled devices) is reviewed each quarter and at pricing changes
- Meta: Shard=— | Type=Task | Size=S | Prio=P1 | Status=DRAFT | SKU=★ | PRD=NF-COST-01, NF-COST-02, NF-COST-03 | Const=C-07

### S-24-032 — Repairability guide & spares programme
As a device owner I want a published repair guide and a 3-year spare-parts commitment so that my device is serviceable rather than disposable.
- AC: repair guide covers battery, display, antenna, and enclosure replacement with torque/ESD notes and is published openly; spare-parts list with 3-year availability commitment is published and reflected in supplier agreements; disassembly does not void regulatory compliance (documented re-assembly verification steps); D-0021 SPEC-8: post-repair steps include button-gasket re-seat + IP65/66 re-verification + antenna-mount RF recheck
- Meta: Shard=— | Type=Task | Size=S | Prio=P2 | Status=DRAFT | SKU=★ | PRD=NF-SUS-02 | Const=C-07, C-03

### S-24-033 — Open accessory & expansion-port standard (Grove-compatible)
As an accessory maker I want a published SS-SP expansion-port standard — Grove-compatible connector and pinout, defined power budgets with per-port gating, I2C auto-discovery, and plugin-driver pairing — so that anyone can build plug-and-play accessories without permission, following the ecosystem playbook proven by Grove, M5Stack, and WisBlock (`06_COMPETITIVE_LANDSCAPE.md` §2.4).
- AC: the standard document specifies connector (HY2.0-4P Grove-compatible), pinout, electrical limits, per-port power-gating requirements, and bus variants (I2C primary; UART/GPIO profiles) and is published CC-BY with reference schematics; the existing 400+ Grove module ecosystem works unmodified on conforming ports (validated on the Lite's Grove header with a representative I2C/UART/GPIO module set); accessories carrying a standard I2C descriptor are auto-discovered and paired to a sandboxed plugin driver (EPIC-18 ABI) with user consent, and unknown modules degrade to safe manual configuration; adapter designs to QWIIC and mikroBUS are published rather than competing standards invented; accessory conformance is testable via a self-serve checklist feeding the F-CERT-01 certification track
- Meta: Shard=— | Type=Feature | Size=M | Prio=P2 | Status=DRAFT | SKU=★ | PRD=F-CERT-01 | Const=C-01, C-07, C-OA

### S-24-034 — Cryptographic export-control classification (EAR / Wassenaar)
As a compliance officer I want the product's cryptography classified under EAR (ECCN 5A002/5D002, licence exception ENC) and Wassenaar equivalents so that international sales and open-source publication are legally unambiguous.
- AC: an export-control assessment covers device firmware, companion apps, SDKs, and the open-source repositories, records the applicable ECCN and licence exception, and files any required notifications (e.g. BIS/NSA email notification for publicly available encryption source); per-country distribution restrictions (embargoed destinations) are documented and wired into the sales/fulfilment checklist; the assessment is repeated on crypto-suite changes (e.g. PQ rollout per Q-03) via a trigger noted in the RFC template; wg-legal sign-off recorded
- Meta: Shard=— | Type=Compliance | Size=S | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-04, C-05

### S-24-035 — UN 38.3 lithium-battery transport certification + shipping compliance
As a compliance officer I want UN 38.3 test certification and dangerous-goods shipping procedures for every battery-containing SKU so that devices can legally ship by air and sea worldwide.
- AC: UN 38.3 test summaries exist for each shipped battery/SKU combination (from cell vendor or commissioned lab) and are published in the compliance pack; packaging, labelling, and documentation procedures for lithium-ion shipments (IATA PI 966/967 as applicable) are documented for direct sales, distributors, and RMA return shipments; battery state-of-charge limits for air freight are reflected in the manufacturing/packout process; a battery change on any SKU triggers re-assessment via the change-control checklist; acknowledges the no-NTC pack design (RT1/RT2 jumpered, HW-6); thermal envelope is FW-owned (S-05-028) and its duty-cap evidence goes in the T.2 thermal submission
- Meta: Shard=— | Type=Compliance | Size=S | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-00

### S-24-036 — Child-user / COPPA-and-equivalents privacy posture
As a compliance officer I want a documented child-user privacy posture (COPPA, GDPR-K age provisions) so that family-oriented use cases (kids carrying pagers) do not create unmanaged legal exposure.
- AC: an assessment determines whether any SS-SP service (cloud accounts, telemetry, location sharing) collects personal data from children and under which conditions parental consent is required per COPPA/GDPR-K; the resulting posture (e.g. accounts 16+, child devices operated under a guardian's account with location sharing controlled by the guardian) is documented in the privacy policy and enforced in account-creation and Fleet/Family flows; marketing guidance for family use cases is aligned with the posture; wg-legal sign-off recorded
- Meta: Shard=— | Type=Compliance | Size=S | Prio=P2 | Status=DRAFT | SKU=★ | PRD=NF-PRIV-02, NF-PRIV-04 | Const=C-05

### S-24-037 — Wi-Fi CERTIFIED HaLow certification per HaLow-bearing SKU
As a compliance officer I want Wi-Fi CERTIFIED HaLow certification obtained for Alpha and Omega so that interoperability with the wider HaLow ecosystem is a certified guarantee rather than a marketing claim.
- AC: Wi-Fi Alliance membership and HaLow certification test plan established covering every HaLow-bearing SKU; certification passes against the WFA test bed (Morse Micro / Newracom / Methods2Business reference devices); certified status and logo usage rules are recorded and linked from the trademark/certification docs (C-TM interaction noted); recertification triggers on radio-affecting firmware changes are documented in the release checklist; per D-HALOW-05 in `docs/portfolio/08_HALOW_TECHNOLOGY_DOSSIER.md`
- Meta: Shard=A | Type=Compliance | Size=M | Prio=P1 | Status=DRAFT | SKU=A,O | PRD=NF-REG-04 | Const=C-08,C-TM

### S-24-038 — Telephony regulatory & compliance module
As a compliance officer I want a telephony regulatory module covering A2P 10DLC, self-host-voice E911 address entry, per-country gating, and emergency-exclusion notices so that every telephony tier ships with its regulatory obligations correctly placed and the interconnected-VoIP regime is kept off the project.
- AC: A2P 10DLC brand/campaign registration is handled for the hosted SS-SP Number service (SS-SP as the ISV brand) and documented as per-brand/non-portable, while Rung-1+ BYO/self-host places 10DLC on the operator; for self-host live voice (S-14-021) an E911-address entry flow is provided that rides the operator's own carrier E911 (never SS-SP's); per-country availability is gated by wg-legal (UK Ofcom KYC, EU geographic-number + GDPR, restricted-VoIP markets treated as no-go without local licensing); a prominent "not for emergency calling" notice appears on all telephony features and no 911/112 termination is offered or implied (SOS remains bearer-flood per C-05); the module documents that hosted stays text-first permanently and that E911/CALEA/USF/CPNI exposure is carried by each bridge operator's carrier account, with wg-legal sign-off
- Meta: Shard=— | Type=Compliance | Size=L | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-TEL-08 | Const=C-07
- Tasks: spec regulatory obligations per rung (10DLC placement, E911, per-country gating) · design A2P 10DLC registration workflow + self-host E911-address entry + emergency-exclusion notices · impl per-country gating config + notice surfacing hooks · test hosted-text-first enforcement, operator-carries-voice-liability boundary, emergency-notice presence · docs regulatory posture module + wg-legal per-country matrix
- Deps: D-0025, `12_TELEPHONY_AND_UNIVERSAL_INTEROP_DOSSIER.md`, S-21-027, S-14-021
