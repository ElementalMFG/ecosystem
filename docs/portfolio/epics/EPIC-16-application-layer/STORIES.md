<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# EPIC-16 — Stories

Format per `../../00_METHODOLOGY.md` §2.7. Meta lines are machine-parsed.

### S-16-001 — Messages app thread list
As a device owner I want a thread list showing my conversations with unread counts and last-message previews so that I can find and resume a conversation quickly.
- AC: Threads sort by most-recent activity with unread badge, contact name, and truncated preview; list handles 200+ threads at 60 FPS via the `ss_ui` virtualised list; opening a thread from the list transitions in < 300 ms on Lite
- Meta: Shard=A | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-APP-01, F-MSG-01 | Const=C-00, C-03

### S-16-002 — Messages app thread view + compose
As a device owner I want a thread view with a composer so that I can read a conversation history and send new messages over the mesh.
- AC: Thread view renders sent/received bubbles with delivery and read-receipt states per F-MSG-01; composed messages enqueue to LXMF and show queued → sent → delivered progression; long histories scroll without loading the entire thread into RAM
- Meta: Shard=A | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-APP-01, F-MSG-01, F-MSG-02 | Const=C-00, C-03

### S-16-003 — Priority selector UI (SOS/high/normal/bulk)
As a device owner I want to set a message's priority when composing so that urgent traffic is scheduled ahead of bulk traffic on constrained bearers.
- AC: Composer exposes high/normal/bulk selection with normal as default; selected priority is carried into the LXMF queue and observable in the network log; SOS priority is reserved for the SOS app and not selectable from the composer
- Meta: Shard=A | Type=Feature | Size=S | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-APP-01, F-MSG-08 | Const=C-00, C-03

### S-16-004 — Contacts app add/edit + key-verify UI
As a device owner I want to add and edit contacts and see each contact's key-verification state so that I know whether I am talking to the identity I expect.
- AC: Add/edit flow captures name and destination identity and persists across reboot; each contact shows verified/unverified state driven by the fingerprint-verify flow; deleting a contact removes it from rosters without deleting message history unless the user confirms
- Meta: Shard=B | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-APP-02 | Const=C-00, C-03, C-05

### S-16-005 — Contacts key-fingerprint side-by-side compare
As a device owner I want a side-by-side fingerprint comparison screen so that I can verify a contact's identity in person before trusting the channel.
- AC: Both devices can display their fingerprint in the same grouped short-form encoding for visual comparison; marking a contact verified requires an explicit confirm action and updates the verified badge everywhere the contact appears; a later key change for a verified contact clears the badge and warns the user
- Meta: Shard=B | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-APP-02 | Const=C-00, C-03, C-05

### S-16-006 — Presence broadcast + roster view
As a device owner I want to set my presence and see my contacts' presence in a roster so that I know who is reachable before I message them.
- AC: User can set online/away/DND and the state broadcasts per F-MSG-06 (SOS state is set only by the SOS app); roster shows last-known presence with staleness indication when a peer has not been heard from; DND suppresses non-SOS notification sounds/haptics locally
- Meta: Shard=C | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-MSG-06 | Const=C-00, C-03

### S-16-007 — Location share (periodic + on-demand)
As a device owner I want to share my location once or live for a chosen duration so that companions can follow my position while I am off-grid.
- AC: On-demand share sends the current fix to a chosen contact or group; live share broadcasts periodically for a user-selected duration and stops automatically when it expires; an active live share is always visible via a persistent indicator with one-tap stop
- Meta: Shard=D | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-MSG-07 | Const=C-00, C-03

### S-16-008 — Location privacy toggles (per-contact allow)
As a device owner I want per-contact location permissions so that only people I explicitly allow can ever see my position.
- AC: Location sharing is denied by default and per-contact allow lists gate every share path including live share; revoking a contact's permission immediately stops any active share to them; no location data leaves the device for non-allowed contacts (verified by network-log test), consistent with NF-PRIV-02
- Meta: Shard=D | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-MSG-07, NF-PRIV | Const=C-00, C-03, C-05

### S-16-009 — SOS hold-to-arm + cancel window
As a device owner I want SOS to require a deliberate hold and offer a cancel window so that a real emergency is fast to trigger but a pocket press never sends a false alarm.
- AC: SOS arms only after the 3 s hold with distinct haptic feedback per R16-01; a visible cancel window lets the user abort before broadcast without any packet leaving the device; once past the window the beacon transmits on every available bearer within 2 s (NF-PERF-04)
- Meta: Shard=E | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-MSG-08, F-APP-06, NF-PERF-04 | Const=C-00, C-03

### S-16-010 — SOS broadcast escalation policy
As a device owner I want the SOS beacon to escalate and repeat across bearers so that my alert keeps propagating until it is acknowledged or I stand down.
- AC: Active SOS rebroadcasts on every available bearer at the policy-defined interval, taking priority over all queued traffic; escalation includes position (when a fix exists) and presence flips to SOS per F-MSG-06; stand-down requires deliberate confirmation and sends an explicit cancellation to prior recipients
- Meta: Shard=E | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-MSG-08, F-APP-06 | Const=C-00, C-02

### S-16-011 — Settings — Radios pane
As a device owner I want a Radios settings pane so that I can see bearer status and enable or disable individual radios.
- AC: Pane lists each bearer present on the SKU with live state (on/off, connected, signal); toggling a bearer takes effect without reboot and persists; regulatory-locked parameters (e.g. HaLow region code) display read-only per NF-REG-04
- Meta: Shard=F | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-APP-05 | Const=C-00, C-03, C-08

### S-16-012 — Settings — Security pane (community-channel opt-in)
As a device owner I want a Security settings pane so that I can manage PIN, duress PIN, PQ-hybrid policy, and community-channel opt-ins in one place.
- AC: Pane exposes device PIN and duress-PIN setup with the duress flow never distinguishable on-screen from normal unlock; community pairing-bridge and telemetry opt-ins default to off per NF-PRIV-01 and state their effect before enabling; hybrid-PQ policy toggle (F-SEC-03) shows current mode and applies without reboot
- Meta: Shard=F | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-APP-05, F-SEC-03, F-SEC-10 | Const=C-00, C-03, C-05

### S-16-013 — Settings — Display pane (theme, brightness)
As a device owner I want a Display settings pane so that I can pick a theme, adjust brightness, and enable accessibility display modes.
- AC: Theme selection (light/dark/high-contrast/night plus colour-blind variants) applies live without reboot; brightness slider changes backlight immediately and persists; large-font toggle is reachable here and applies per S-15-014
- Meta: Shard=F | Type=Feature | Size=S | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-APP-05, F-UI-05 | Const=C-00, C-03

### S-16-014 — Settings — Updates pane
As a device owner I want an Updates settings pane so that I can check the installed firmware version, fetch available updates, and control when they install.
- AC: Pane shows current version, channel, and signature/attestation status of the running firmware; user can check for and defer or apply an update, with dual-signature verification outcome surfaced before install; a failed or rolled-back update is reported clearly on next boot
- Meta: Shard=F | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-APP-05, F-SEC-08 | Const=C-00, C-03, C-05

### S-16-015 — About / Diagnostics screen
As a device owner I want an About/Diagnostics screen so that I can read versions, radio signal detail, and my signed device identity when troubleshooting.
- AC: Screen shows firmware/component versions, per-bearer signal diagnostics, and the device's public identity fingerprint; network log view lists recent link events without exposing message contents (NF-SEC-03); diagnostics can be exported as a shareable text blob with an explicit user action
- Meta: Shard=G | Type=Feature | Size=S | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-APP-05 | Const=C-00, C-03

### S-16-016 — First-run onboarding flow
As a device owner I want a guided first-run flow covering language, region, key generation, backup, and first contact so that the device is usable minutes after unboxing.
- AC: Flow completes language, region (with HaLow region-code enforcement per NF-REG-04 where applicable), key generation, and key-backup prompt in ≤ 3 min for a first-time user; onboarding works fully standalone with no cloud contact, with companion-app-guided setup as an optional path (F-APP-07); interrupting onboarding resumes at the last completed step after reboot
- Meta: Shard=H | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-APP-07 | Const=C-00, C-03, C-05

### S-16-017 — Encrypted backup export (QR + file)
As a device owner I want to export an encrypted backup of my identity and contacts as a QR sequence or file so that I can recover onto a new device if this one is lost.
- AC: Backup blob contains identity keys, contacts, and settings, encrypted under a key derived from a user seed passphrase; export works as both QR-code sequence and file transfer, and the guided flow offers the printed seed-card option per R16-02; no plaintext key material is written to flash or shown after the flow completes (NF-SEC-02)
- Meta: Shard=I | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-APP-07, NF-SEC | Const=C-00, C-05

### S-16-018 — Restore-from-backup flow
As a device owner I want to restore a backup onto a fresh device so that my identity, contacts, and settings survive device loss or replacement.
- AC: Restore from QR sequence or file onto a factory-fresh device reproduces identity, contacts, and settings (epic exit criterion 3); a wrong passphrase fails cleanly with no partial state left on the device; restored identity sends and receives messages with pre-existing contacts without re-verification of the user's own key
- Meta: Shard=I | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-APP-07 | Const=C-00, C-05

### S-16-019 — Notifications routing (haptic, LED, sound)
As a device owner I want notification routing across haptic, LED, and sound so that I notice events in the way that suits my situation and hardware.
- AC: Per-event-class routing (message, presence, SOS, system) is configurable in Settings; SOS alerts always fire on all channels present and cannot be muted; on hardware lacking a channel (e.g. no haptic motor) routing degrades gracefully to the remaining channels per F-UI-03
- Meta: Shard=J | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-UI-03, F-APP-05 | Const=C-00, C-03

### S-16-020 — In-app translation stub w/ crowdin export
As a UI engineer I want app strings exportable to and importable from a community translation platform so that locale coverage can grow via the community portal planned for M3+.
- AC: All application-layer strings are `ss_i18n`-extracted (NF-L10N-01) and export to the platform's file format losslessly; imported translations round-trip into device catalogues and render on-device without code changes; missing keys fall back to EN and are reported in the export summary
- Meta: Shard=— | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-UI-04, NF-L10N-01 | Const=C-00, C-03

### S-16-021 — Screen-transition perf harness (< 300 ms)
As a firmware engineer I want an automated screen-transition performance harness so that the epic's < 300 ms navigation exit criterion is enforced in CI.
- AC: Harness navigates every v1 app screen pair on Lite hardware and records transition times; any transition ≥ 300 ms fails the CI run with the offending screen pair identified; results are archived per build for trend analysis
- Meta: Shard=— | Type=Ops | Size=S | Prio=P0 | Status=DRAFT | SKU=★ | PRD=NF-PERF | Const=C-00, C-03

### S-16-022 — Network GPS service (NMEA-0183 over BLE / TCP)
As a device owner I want my SS-SP to serve its GNSS fix as standard NMEA-0183 sentences over BLE (all SKUs) and TCP (Alpha/Omega) so that any EUD, laptop, or mapping app can use the device as a network GPS with no proprietary client.
- AC: served sentences (GGA/RMC/GSA at minimum) parse in stock third-party NMEA consumers over both transports; the service is opt-in, off by default, and gated by the position-privacy setting per NF-PRIV; endpoint (BLE characteristic and TCP port) is documented in the public device-API reference per F-EUD-01; serving adds no measurable GNSS power penalty when no client is connected
- Meta: Shard=— | Type=Feature | Size=S | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-EUD-01 | Const=C-00, C-08

### S-16-023 — Map application (offline tiles)
As a device owner I want an on-device map app with pre-loaded offline tiles so that positioning, peers, and waypoints render on a real map with zero connectivity.
- AC: map renders offline raster/vector tiles from SD or flash with pan/zoom on `ss_ui`; own GNSS fix, peer positions, and waypoints overlay correctly with heading-up option; optional online tile enrichment is opt-in, clearly indicated, and never blocks offline rendering; tile-pack import is documented (companion app and SD side-load)
- Meta: Shard=— | Type=Feature | Size=L | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-APP-03 | Const=C-00, C-03

### S-16-024 — Unified declarative node profile (one config, every form factor)
As a node operator I want a single declarative, versioned configuration profile (interfaces, roles, radio params, app settings) that applies identically to a device, a phone app, a desktop daemon, and a router package so that setups are shareable, reproducible, and portable across every SS-SP form factor (`06_COMPETITIVE_LANDSCAPE.md` §4.1).
- AC: a published, versioned schema expresses node identity references, bearer/interface config, tier/role (leaf/transport per S-11-023), gateway options, and app-level settings; the same profile file imports and validates on Lite firmware, companion apps, the headless daemon (F-NODE-05), and the OpenWrt package with unsupported keys reported (not silently dropped) and hardware-inapplicable keys ignored by declared capability; profiles export from any node form factor and round-trip losslessly; secrets/keys are referenced, never embedded, and a profile can be shared safely by default; schema changes follow the RFC process with backward-compatible minor versions
- Meta: Shard=— | Type=Feature | Size=L | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-NODE-01, F-NODE-05 | Const=C-02, C-08

### S-16-025 — Low-battery graceful-degradation ladder + SOS power reserve
As a device owner I want a defined power-degradation ladder with a protected SOS reserve so that a dying battery sheds comfort features first and can still send an SOS at the very end.
- AC: a documented degradation ladder (e.g. brightness/LED → non-essential radios → sync intervals → display-off beacon-only) engages automatically at defined state-of-charge thresholds with user notification at each step; a protected energy reserve is enforced below which only SOS transmission and minimal receive remain available, sized so at least one full SOS beacon burst (NF-PERF-04) plus location fix succeeds from the reserve floor on Lite hardware; entering and leaving reserve mode is logged and visible in the UI; thresholds are policy-configurable per power profile and covered by HIL battery-drain tests
- Meta: Shard=— | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=NF-PWR-01, NF-PERF-04 | Const=C-00, C-05

### S-16-026 — Device-side fleet policy receipt, verification, and application
As a fleet admin I want devices to receive, verify, and apply signed fleet policies (crypto suite, enabled radios, update channel) and report compliance so that the S-21-007 policy engine actually lands on hardware.
- AC: policies arrive as signed `admin.command` records (type 12 per `02_PROTOCOL_STACK.md` §4.2), are verified against the tenant's fleet-admin key with anti-rollback sequence numbers, and apply without reboot where the setting allows; a policy can never disable SOS, core off-grid messaging, or user duress protections — a denylist of non-overridable settings is enforced and unit-tested per the C-05 covenant; the device reports applied-policy state via `admin.telemetry_report` respecting NF-PRIV-01 opt-ins; unenrolled personal devices ignore fleet policy entirely
- Meta: Shard=— | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-CL-01 | Const=C-02, C-05
