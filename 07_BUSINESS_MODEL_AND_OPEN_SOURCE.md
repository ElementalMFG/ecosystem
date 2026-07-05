<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# 07 — Business Model & Open-Source Strategy

*Binding founding document. Supersedes all prior informal statements on monetization. Companion to `04_LICENSING_AND_FORK_STRATEGY.md` and `governance/OPEN_ASSURANCE.md`.*

---

## Table of contents

- [0. TL;DR](#0-tldr)
- [1. Why "open from day one" instead of "open later"](#1-why-open-from-day-one-instead-of-open-later)
- [2. What is community edition vs. commercial edition](#2-what-is-community-edition-vs-commercial-edition)
- [3. Revenue streams — concrete list](#3-revenue-streams--concrete-list)
- [4. Anti-rug-pull binding rules](#4-anti-rug-pull-binding-rules)
- [5. Fair-source practical notes](#5-fair-source-practical-notes)
- [6. Certification program (`SS-SP-Certified`)](#6-certification-program-ss-sp-certified)
- [7. Data protection & user trust as a business asset](#7-data-protection--user-trust-as-a-business-asset)
- [8. Transparency deliverables (published on every release)](#8-transparency-deliverables-published-on-every-release)
- [9. Governance-money interaction rules](#9-governance-money-interaction-rules)
- [10. If we run out of money](#10-if-we-run-out-of-money)
- [11. What "success" means](#11-what-success-means)

## 0. TL;DR

- **We are an open-source hardware company, not a closed-source software company.**
- **Firmware, apps, protocol, SDK are Apache-2.0 / CC-BY-4.0 from day one.** No "open source later." No takebacks.
- **We make money on:** hardware margin, cloud services (BSL 1.1 → Apache-2.0 after 4 years), enterprise support, premium plugins (BSL), certification program, professional services.
- **We do NOT make money on:** locking users out of features on the device they bought, ads, telemetry, or selling data.
- **Every commercial capability we add must strengthen the community edition, never gate it.**

---

## 1. Why "open from day one" instead of "open later"

The "closed now, open later" model is the single most common way open-hardware projects fail. It looks appealing (protect the crown jewels while you sell hardware) but destroys the three things that actually make the business work:

1. **Trust.** Once a user or reviewer sees closed firmware on a "secure communications" device, no amount of later opening restores confidence. The window for building trust is at launch, not year three.
2. **Contributions.** Contributors show up when the source is open and the project is early. Wait two years and the pool of interested contributors evaporates — everyone assumes it's a solo shop.
3. **Head start.** The moment you finally open the source, clones race ahead using two years of your engineering as a free head-start. Meanwhile your reputation for being closed lingers.

**Open source from day one is a moat, not a leak.** The moat is:

- **Hardware.** Nobody spins up an ESP32-P4 + HaLow + LoRa PCB, gets it through FCC/CE, and produces it in the thousands without serious capital and time.
- **Brand and cryptographic identity.** `RelKey_A`/`B` for signing OTAs are ours. `SS-SP-Certified` is a trademark we own. A clone can copy the code; it cannot ship a device that our official infrastructure trusts.
- **Cloud services and operations.** Fleet console, plugin registry, provisioning service — running these at scale, with SLAs, with security response, is a real business that a clone won't casually stand up.
- **Community.** A healthy contributor base flowing through us means we set the direction, and clones fork away from the mainstream, not toward it.

---

## 2. What is community edition vs. commercial edition

### 2.1 Community edition (Apache-2.0, forever)

Ships on every device we sell. Available in source. Available in binary from our OTA channel. Buildable by anyone from source.

**Includes, non-negotiable per `OPEN_ASSURANCE.md`:**

- One-to-one text messaging.
- Group text messaging.
- Voice messaging and PTT (subject to hardware).
- Presence, location sharing.
- SOS and emergency beacons.
- Full Reticulum + LXMF connectivity.
- LoRa, Wi-Fi (2.4/5), Bluetooth LE, Wi-Fi HaLow bearers (subject to hardware).
- Meshtastic wire compatibility on LoRa (clean-room, see `04_LICENSING_AND_FORK_STRATEGY.md` §3).
- Home Gateway mode (device acts as mesh↔internet bridge and Wi-Fi extender when docked).
- OTA updates (community channel).
- All companion apps: mobile (iOS, Android), desktop (Linux, macOS, Windows), web.
- All SDKs (C, Rust, Python, Dart, TypeScript).

**Community edition never phones home for anything the user did not opt into.** Anonymous crash reports and update checks are opt-in at first-boot, and the user may run their own OTA mirror.

### 2.2 Commercial editions (BSL 1.1 → Apache-2.0 after 4 years)

Additional capabilities we sell to enterprises, deployments at scale, or power users. Source is public. Anyone may read it, audit it, run it privately, contribute patches. What the BSL restricts is: **you may not stand up a competing hosted commercial service using our code, until the 4-year clock expires.** After that, everything becomes Apache-2.0.

Candidates (each requires its own product decision):

- **Fleet Console** — multi-device management, remote configuration, over-the-air fleet updates, per-device compliance dashboards. For humanitarian, industrial, public-safety, and enterprise deployments.
- **Provisioning Service** — factory-line and post-sale bulk provisioning of device identities, key ceremonies, tenant onboarding.
- **Plugin Registry** — signed plugin distribution with dependency tracking, review pipeline, revocation.
- **Relay Federation** — commercial relay operator tooling: reservations, billing, abuse response, high-availability.
- **Fleet-specific features on device** — capabilities activated only when a device is enrolled in a paid fleet: e.g. centralised policy enforcement, mandatory audit logging, hardware-attested location reports for regulated deployments. Community-edition devices retain full personal use.

### 2.3 Premium plugins (BSL 1.1 or proprietary, per plugin)

Optional add-ons that run in the WASM plugin sandbox (`05_SECURITY_MODEL.md` §10). Users choose to install and pay for them. Examples:

- Advanced mapping and offline satellite tiles.
- Marine or aviation-grade tracking integrations.
- Corporate messaging bridges (Slack, MS Teams, Signal-Enterprise).
- Language packs and premium TTS voices.

**Rule:** a premium plugin **may not** disable, degrade, or paywall any community-edition capability. It can only add.

---

## 3. Revenue streams — concrete list

1. **Hardware sales.** Lite, Alpha, Omega SKUs plus optional accessories (dock, external antenna, ruggedised case, extended battery). Direct-to-consumer and B2B channels.
2. **Certification licence fees.** Third-party device makers may license the `SS-SP-Certified` trademark and pass the certification test suite. Annual fee scales with SKU volume.
3. **Cloud services subscriptions.** Fleet console tiers (free personal, team, enterprise). Provisioning service per-device fees for third-party manufacturers.
4. **Enterprise support contracts.** Signed SLA, guaranteed response times, security fix prioritization, private threat feed, private incident response.
5. **Premium plugin marketplace.** Revenue share with plugin authors. First-party plugins keep 100%.
6. **OEM / white-label.** Customized SS-SP builds for other brands, on our hardware or theirs, under commercial licence separate from community.
7. **Professional services.** Custom integrations, on-site deployment consulting, threat-model reviews, penetration testing engagements for large customers.
8. **Grants and government contracts.** Humanitarian, disaster-response, public-safety agencies. Community edition is often the sellable point.

**Explicitly NOT revenue streams:**

- Advertising.
- Selling any user data, contact graph, message content, or location history.
- Paywalling any capability that shipped in community edition or its documented roadmap.
- Feature ransoming (locking devices after non-payment).
- Coerced telemetry.

---

## 4. Anti-rug-pull binding rules

These are enforceable per `OPEN_ASSURANCE.md` and the governance-transition escrow described in `06_GOVERNANCE.md`.

1. **No retroactive relicensing** of already-published community-edition source.
2. **No essentials paywall** on shipped hardware.
3. **Five-year minimum security-fix window** per SKU from last commercial shipment date, delivered on the community OTA channel.
4. **BSL 1.1 auto-converts to Apache-2.0** at 4 years for every published BSL commit. No shell-game where the clock resets on trivial refactors.
5. **Trademark transfer** to the SS-SP Foundation at Phase 2 (see `06_GOVERNANCE.md` §2). Backup transfer to independent open-source steward if the Foundation does not form.
6. **Vendor-loss escrow** covers `RelKey_C`, trademarks, and infrastructure. See `05_SECURITY_MODEL.md` §13.4.

---

## 5. Fair-source practical notes

- BSL 1.1 source is **published in the same monorepo** under `cloud/` with clear `LICENSE.BSL` headers. Anyone can read, build, run privately, and contribute patches. What they cannot do until the 4-year clock: sell it as a competing service.
- Contributors to BSL 1.1 code sign the same CLA as Apache-2.0 code. The CLA grants us the right to relicense the resulting composite work, which is what makes the auto-conversion clean.
- Every BSL 1.1 file must carry `Change Date: YYYY-MM-DD` in the header. That is the date the file becomes Apache-2.0. No `Change Date: (never)`.
- Third-party dependencies of BSL modules must themselves be compatible with both BSL and Apache-2.0 for the eventual conversion to be lawful. A CI lint enforces this.

---

## 6. Certification program (`SS-SP-Certified`)

Purpose: allow third-party hardware to interoperate reliably with SS-SP devices, and give users a badge to trust.

- **Test suite** in `tools/protocol-fuzzer/` and `protocol/testvectors/` — mandatory pass to earn the badge.
- **Interop matrix** covers wire-format conformance on every bearer (LoRa Meshtastic-compat, HaLow, Wi-Fi, BLE).
- **Security floor**: secure boot, dual-signed OTA (may use vendor's own keys), no bundled backdoors.
- **Update commitment**: certified device manufacturers commit to a documented security-fix window (minimum 3 years).
- **Revocation**: repeated security disclosures ignored or safety violations trigger badge revocation and public notice.
- **Fee model**: per-SKU annual licence fee with volume tiers; below a small-volume threshold the licence is free (encourages hobby / research builds).

---

## 7. Data protection & user trust as a business asset

Every product decision in commercial editions must pass the following filters. If any answer is "no" we do not ship the feature.

1. Would we be comfortable if this data appeared, verbatim, in a leaked press exposé about our company?
2. Is user consent explicit, granular, revocable, and default-off for anything beyond core function?
3. Is any collected data useless to an attacker who compromises our servers (i.e. is it end-to-end encrypted or aggregated to the point of anonymity)?
4. Have we minimised what is collected (e.g. can we do it on-device instead)?
5. Is there a documented retention window, and an auto-deletion job that enforces it?

This is not just ethics; it is durable competitive advantage. Every scandal by a competitor becomes a case for our design.

---

## 8. Transparency deliverables (published on every release)

- **SBOM** for firmware, apps, cloud services.
- **Reproducible-build receipts** (Sigstore attestation) so anyone can verify the binary matches the source.
- **Signed release notes** including changes to security-relevant code paths.
- **Public transparency log** entries for release signings, key rotations, plugin registry additions and revocations.
- **Warrant canary** on the project's public site, updated per quarter.
- **Annual audit report** by an independent security firm from year 2 onward, funded from cloud-services revenue.

---

## 9. Governance-money interaction rules

Cross-refers `06_GOVERNANCE.md` §10.

- The commercial vendor entity may pay Working Group members, Steering Committee members, and Security Response Team members — but must disclose it in `governance/decisions.md`.
- No decision that materially affects the community edition may be taken by paid-vendor votes alone; independent seats must concur.
- Financial pressure on the vendor is not a valid reason to weaken the Open Assurance commitments. The Steering Committee has explicit standing to reject vendor requests that would.

---

## 10. If we run out of money

The escrow instrument fires:

1. `RelKey_C` and the trademark portfolio pass to the SS-SP Foundation.
2. Devices in the field continue to function on the last shipped community firmware, indefinitely.
3. The community OTA channel is preserved for security fixes contributed by whoever picks up maintenance.
4. Cloud-service source (BSL) is on-disk in the public monorepo, so anyone can stand up a successor operator.
5. No customer data is auctioned. Data-controller status transfers to a fiduciary who honours the existing privacy policy or deletes.

The point is: **the user's device outlives the company.**

---

## 11. What "success" means

Not exit valuation. The scoring rubric for the project:

1. Devices in the field number in the hundreds of thousands within five years.
2. At least three independent hardware manufacturers ship `SS-SP-Certified` devices.
3. Community edition is the default firmware on 100 % of vendor-shipped devices.
4. Cloud-service revenue funds the security response team and reproducible-build infrastructure indefinitely.
5. The trademark transfer to Foundation is executed on schedule.
6. Zero rug-pull events. Zero secret backdoors. Zero customer-data commercialization incidents.

---

*End of 07.*
