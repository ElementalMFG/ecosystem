<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# EPIC-13 — Stories

Format per `../../00_METHODOLOGY.md` §2.7. Meta lines are machine-parsed.

### S-13-001 — Clean-room process documentation + reviewer isolation
As a protocol engineer I want a documented clean-room process with reviewer isolation rules so that no contributor who has seen GPL-3.0 Meshtastic firmware source writes SS-SP compat code and the legal firewall is provable.
- AC: Process doc lists spec-writer vs implementer roles and forbids implementers from viewing GPL-3.0 Meshtastic firmware source; contributor attestation form exists and is required in every `ss_meshtastic_compat` PR; dev-log template captures which public docs/observed traffic each spec section derives from
- Meta: Shard=I | Type=Task | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-BR-07 | Const=C-04

### S-13-002 — Regenerate protos from Apache-2.0 `.proto` files
As a firmware engineer I want protobuf bindings regenerated from the Apache-2.0 licensed Meshtastic `.proto` definitions so that wire message structures are available without touching GPL-3.0 firmware code.
- AC: Generated bindings build from upstream `.proto` files with pinned commit hash recorded; only Apache-2.0 licensed inputs appear in the generation pipeline and licences are recorded in the SBOM; round-trip encode/decode unit tests pass for every message type used by EPIC-13
- Meta: Shard=B | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-BR-07 | Const=C-04, C-02

### S-13-003 — Wire header + LoRa packet parser
As a firmware engineer I want a parser/serialiser for the Meshtastic LoRa packet header derived only from public wire documentation so that SS-SP radios can frame and deframe Meshtastic v2.x packets.
- AC: Parser handles packet header fields (dest, sender, packet-id, flags, hop-limit) per the extracted wire spec; malformed/truncated packets are rejected without crash under fuzz testing; serialised packets are byte-identical to captured reference vectors from real Meshtastic traffic
- Meta: Shard=A | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=L | PRD=F-BR-07 | Const=C-04, C-02

### S-13-004 — Channel + PSK derivation (AES-CTR / 128-bit)
As a firmware engineer I want Meshtastic channel-key derivation and AES-CTR payload encryption so that SS-SP devices can join a named Meshtastic channel with a shared PSK.
- AC: Channel hash and key derivation match published test vectors for default and custom PSKs; AES-CTR encrypt/decrypt interoperates with a commercial Meshtastic device on the same channel; SS-SP identity keys are never used or exposed on the Meshtastic side (verified by code review + test)
- Meta: Shard=C | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=L | PRD=F-BR-07 | Const=C-04, C-02, C-05

### S-13-005 — Node-info message
As a Meshtastic community user I want SS-SP devices to send and parse NodeInfo messages so that my Meshtastic client shows the SS-SP device with a name in its node list.
- AC: SS-SP broadcasts NodeInfo (long/short name, hardware id per compat mapping) at the documented cadence; received NodeInfo from Meshtastic peers populates the SS-SP neighbour table; SS-SP node appears named in a stock Meshtastic client node list
- Meta: Shard=D | Type=Feature | Size=S | Prio=P0 | Status=DRAFT | SKU=L | PRD=F-BR-07 | Const=C-04, C-02

### S-13-006 — Text message send/receive
As a Meshtastic community user I want to exchange text messages between an SS-SP device and my Meshtastic device so that both meshes can communicate on a shared channel.
- AC: Text sent from SS-SP renders on a commercial Meshtastic device on the same channel, and vice versa; UTF-8 payloads up to the Meshtastic size limit are handled and over-limit sends are rejected with a user-visible error; received Meshtastic texts appear in the SS-SP Messages app marked as Meshtastic-origin
- Meta: Shard=E | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=L | PRD=F-BR-07, F-MSG-01 | Const=C-04, C-02

### S-13-007 — Position report
As a Meshtastic community user I want SS-SP position reports on the Meshtastic channel so that SS-SP devices appear on the Meshtastic map.
- AC: Position packets encoded from SS-SP fix data are displayed at the correct coordinates in a Meshtastic client; incoming Meshtastic positions are decoded and shown for that peer on SS-SP; position broadcast respects the SS-SP location-privacy toggle (off by default)
- Meta: Shard=F | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=L | PRD=F-BR-07, F-MSG-07 | Const=C-04, C-02

### S-13-008 — Telemetry (battery, env)
As a Meshtastic community user I want SS-SP devices to emit Meshtastic telemetry packets so that battery and environment metrics show up in Meshtastic dashboards.
- AC: Device-metrics telemetry (battery level, voltage) encodes per the compat spec and renders in a Meshtastic client; telemetry cadence is configurable and defaults to the documented Meshtastic interval; unsupported sensor fields are omitted rather than zero-filled
- Meta: Shard=F | Type=Feature | Size=S | Prio=P1 | Status=DRAFT | SKU=L | PRD=F-BR-07 | Const=C-04, C-02

### S-13-009 — Store-and-forward relay
As a Meshtastic community user I want SS-SP nodes to participate in Meshtastic store-and-forward so that messages reach devices that were temporarily out of range.
- AC: SS-SP honours hop-limit and rebroadcast rules per the extracted wire spec without packet storms in a 5-node testbed; stored messages are bounded by a configurable RAM/flash quota and evicted oldest-first; relay behaviour is off by default and toggleable in Settings
- Meta: Shard=G | Type=Feature | Size=L | Prio=P1 | Status=DRAFT | SKU=L | PRD=F-BR-07, F-MSG-09 | Const=C-04, C-02

### S-13-010 — Admin message subset (channel set, node-info request)
As a Meshtastic community user I want SS-SP to answer the documented admin-message subset so that channel configuration and node-info requests from Meshtastic tools work.
- AC: Channel-set and node-info-request admin messages are handled and acknowledged per the compat spec; admin messages outside the documented subset are ignored and logged, never crash; admin actions that would alter SS-SP security material are refused by design (test included)
- Meta: Shard=H | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=L | PRD=F-BR-07 | Const=C-04, C-02, C-05

### S-13-011 — Capability advertisement flag
As a protocol engineer I want SS-SP nodes to advertise an SS-SP capability flag on the Meshtastic mesh so that two SS-SP devices can negotiate native features instead of staying in compat mode.
- AC: SS-SP flag is carried in a spec-documented extension field that stock Meshtastic firmware ignores harmlessly (verified against a commercial device); two SS-SP devices detecting each other's flag negotiate up to native SS-SP protocol; flag semantics are documented in the compat wire spec
- Meta: Shard=J | Type=Feature | Size=S | Prio=P0 | Status=DRAFT | SKU=L | PRD=F-BR-07 | Const=C-04, C-02

### S-13-012 — Legal audit + attestation
As a device owner I want an independent legal audit of the clean-room process so that the Meshtastic compat layer is defensible against copyright challenge.
- AC: wg-legal review confirms no file in `firmware/components/ss_meshtastic_compat/` derives from GPL-3.0 code; contributor attestations and dev logs are complete for every merged compat PR; signed audit attestation is filed in the governance record
- Meta: Shard=I | Type=Task | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-04

### S-13-013 — Compatibility matrix vs Meshtastic firmware versions
As a protocol engineer I want an automated compatibility matrix across Meshtastic firmware versions so that regressions in interop are caught when either side changes.
- AC: CI job exercises text, node-info, and position flows against at least the two most recent Meshtastic v2.x minor releases; matrix results are published as a versioned artifact; a failing cell blocks release of the compat component
- Meta: Shard=— | Type=Ops | Size=M | Prio=P0 | Status=DRAFT | SKU=L | PRD=F-BR-07 | Const=C-04, C-02

### S-13-014 — Field trial w/ commercial Meshtastic hardware
As a Meshtastic community user I want a field trial of SS-SP against off-the-shelf Meshtastic hardware so that interop is proven over real RF, not just bench captures.
- AC: SS-SP Lite exchanges text bidirectionally with a commercial Meshtastic device on the same channel outdoors; SS-SP position report is visible on the Meshtastic client map; trial report logs RSSI/SNR, packet loss, and any interop defects as filed issues
- Meta: Shard=— | Type=Ops | Size=M | Prio=P0 | Status=DRAFT | SKU=L | PRD=F-BR-07 | Const=C-04, C-02

### S-13-015 — Meshtastic compat RFC (RFC-0009)
As a protocol engineer I want the Meshtastic compat wire behaviour ratified as RFC-0009 so that the supported surface is governed and future changes go through formal review.
- AC: RFC-0009 specifies supported message types, versioning/feature-gating strategy, and the crypto-isolation boundary; RFC passes the `06_GOVERNANCE.md` review process to ACCEPTED; implementation stories reference RFC-0009 section numbers
- Meta: Shard=A | Type=RFC | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-BR-07 | Const=C-04, C-02, C-06

### S-13-016 — Scope statement in user docs (what works, what doesn't)
As a device owner I want user documentation stating exactly which Meshtastic features work with SS-SP so that I have accurate expectations and the community sees honest scoping.
- AC: Docs page lists supported features (text, node-info, position, telemetry, admin subset) and explicitly lists unsupported ones; wording follows the neutral-marketing mitigation (no denigration of Meshtastic); page is linked from the device quick-start
- Meta: Shard=— | Type=Docs | Size=S | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-BR-07 | Const=C-04
