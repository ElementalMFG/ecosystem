<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# EPIC-17 — Stories

Format per `../../00_METHODOLOGY.md` §2.7. Meta lines are machine-parsed.

### S-17-001 — Dock detection GPIO + debounce
As a firmware engineer I want debounced dock-contact detection so that dock/undock events are reliable triggers for Home Gateway activation.
- AC: Dock and undock events are detected via the dock contacts with debounce so that contact chatter produces exactly one event; dock state combines with power/charge signal to satisfy the F-HGW-01 activation preconditions (dock + trusted Wi-Fi + battery ≥ 50%); event is published on the system bus for the mode-switch state machine to consume
- Meta: Shard=A | Type=Feature | Size=S | Prio=P0 | Status=DRAFT | SKU=A,O | PRD=F-HGW-01 | Const=C-08

### S-17-002 — Mode-switch state machine
As a firmware engineer I want a mesh ↔ HGW mode-switch state machine so that the device transitions between roles deterministically with no stuck intermediate states.
- AC: State machine covers mesh, HGW-activating, HGW-active, and HGW-shutdown states with all transitions unit-tested; auto-activation fires only when all F-HGW-01 conditions hold and user override (enable/disable) is respected; undock triggers reversion to mesh mode within 3 s without user-visible fault (epic exit criterion 3)
- Meta: Shard=B | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=A,O | PRD=F-HGW-01 | Const=C-08

### S-17-003 — Wi-Fi client uplink to home AP
As a home user I want the docked device to join my home Wi-Fi as the WAN uplink so that mesh traffic can reach the Internet through my existing router.
- AC: Device associates to the configured trusted SSID and validates Internet reachability before advertising WAN to bridge services; uplink loss is detected and triggers the offline-degrade path rather than a mode fault; credentials are stored encrypted at rest (NF-SEC-02)
- Meta: Shard=C | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=A,O | PRD=F-HGW-02, F-HGW-03 | Const=C-08

### S-17-004 — Wi-Fi soft-AP downlink for clients
As a home user I want the gateway to run a soft-AP so that my phone and other devices can use it as a Wi-Fi extender.
- AC: Soft-AP starts in HGW mode with user-configured SSID/passphrase and is opt-in per SSID (F-HGW-06); a phone client associates and reaches the Internet via the uplink; soft-AP is WPA2/WPA3-protected and isolated from gateway admin surfaces by default
- Meta: Shard=D | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=A,O | PRD=F-HGW-06 | Const=C-08

### S-17-005 — NAT + DHCP server on soft-AP
As a firmware engineer I want NAT and a DHCP server on the soft-AP segment so that downlink clients get addresses and routed Internet access without manual configuration.
- AC: DHCP leases addresses to at least 8 concurrent soft-AP clients with correct gateway/DNS options; NAT forwards TCP/UDP between soft-AP clients and the uplink with no leakage into gateway-internal services; lease table survives a soft-AP restart without duplicate-address conflicts
- Meta: Shard=D | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=A,O | PRD=F-HGW-06 | Const=C-08

### S-17-006 — LoRa ↔ RNS gateway (mesh to Internet)
As a home user I want my docked device to bridge the local LoRa mesh to the global Reticulum network so that neighbourhood mesh traffic can reach distant peers over the Internet.
- AC: A LoRa-only device in range exchanges LXMF messages with an Internet-side Reticulum peer through the gateway (epic exit criterion 1); bridging preserves end-to-end encryption — the gateway never decrypts user payloads; bridge role is visible and can be disabled from the admin surfaces
- Meta: Shard=E | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=A,O | PRD=F-HGW-03 | Const=C-08, C-05

### S-17-007 — HaLow ↔ RNS gateway
As a home user I want the gateway to bridge HaLow devices to the Internet so that neighbourhood devices 1–2 km away get global reach through my house.
- AC: A HaLow device in range exchanges LXMF traffic with an Internet-side Reticulum peer via the gateway; bridge sustains concurrent HaLow clients without starving LoRa bridging (fairness test); HaLow bridging honours the region-code enforcement rules (NF-REG-04)
- Meta: Shard=F | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=A,O | PRD=F-HGW-02 | Const=C-08

### S-17-008 — LXMF propagation-node service
As a home user I want my gateway to run an opt-in LXMF propagation node so that messages for offline neighbours are stored and delivered when they come back into range.
- AC: Propagation node is opt-in and off by default (F-HGW-07); stored messages respect a configurable disk quota with oldest-first eviction and survive reboot; a message posted while the recipient is offline is delivered when the recipient reappears (store-and-forward test)
- Meta: Shard=E | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=A,O | PRD=F-HGW-07, F-HGW-05, F-MSG-09 | Const=C-08

### S-17-009 — Captive portal setup UX (device)
As a home user I want a captive-portal setup flow when I first dock the device so that I can configure gateway roles from my phone browser without installing anything.
- AC: Un-configured HGW exposes a setup AP whose captive portal walks through uplink Wi-Fi, soft-AP, and role opt-ins (bridge, propagation, extender); setup completes in ≤ 2 min (epic exit criterion 4); portal is served only on the setup network and disables itself after configuration
- Meta: Shard=G | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=A,O | PRD=F-HGW-01, F-HGW-06 | Const=C-08

### S-17-010 — Local admin API (mDNS + REST)
As a firmware engineer I want a local mDNS-discoverable REST admin API so that companion apps can read gateway status and change configuration on the LAN.
- AC: Gateway advertises via mDNS and serves a versioned REST API covering status, role toggles, and traffic counters; every request is authenticated against the paired-device trust store — unauthenticated requests are rejected; API is reachable only from local networks, never exposed on the WAN uplink
- Meta: Shard=H | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=A,O | PRD=F-HGW-05, F-HGW-07 | Const=C-08, C-05

### S-17-011 — Fair-use traffic accounting
As a home user I want per-role traffic accounting with fair-use caps so that guest mesh relaying never swamps my home broadband.
- AC: Byte counters per role (bridge, propagation, extender) are visible in the admin API and companion pane; user-set fair-use caps throttle or pause a role when exceeded and notify the owner; accounting counts volumes only — no per-user content or destination logging (NF-PRIV)
- Meta: Shard=I | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=A,O | PRD=F-HGW-05, F-HGW-06 | Const=C-08, C-07

### S-17-012 — HGW → mesh graceful shutdown
As a device owner I want gateway mode to shut down gracefully when I grab the device off the dock so that I can walk out the door and use it immediately as a pager.
- AC: On undock, in-flight bridge transfers are flushed or handed off within the 3 s reversion budget; soft-AP clients receive a deauth and DHCP releases are cleaned up; mesh mode resumes with radios in the pre-dock configuration and no user-visible fault
- Meta: Shard=B | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=A,O | PRD=F-HGW-01 | Const=C-08

### S-17-013 — Thermal throttling policy in HGW mode
As a firmware engineer I want a thermal throttling policy for sustained gateway operation so that a fan-less docked device stays within its thermal budget (risk R17-02).
- AC: Temperature monitoring throttles bridge throughput and soft-AP activity in defined steps before any emergency shutdown; sustained max-load soak test on the dock stays below the component thermal limits; throttle state is reported in diagnostics and the admin API
- Meta: Shard=— | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=A,O | PRD=F-HGW-01 | Const=C-08

### S-17-014 — HGW admin pane in companion app
As a home user I want a Home Gateway pane in the companion app so that I can see gateway status and toggle roles without opening a browser.
- AC: Pane discovers the gateway via the local admin API and shows mode, uplink health, connected clients, and traffic counters; role opt-ins (bridge, propagation node, extender) can be toggled from the pane and take effect without reboot; pane clearly surfaces fair-use cap state and throttling
- Meta: Shard=H | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=A,O | PRD=F-HGW-05, F-HGW-06, F-HGW-07 | Const=C-08, C-07

### S-17-015 — Offline propagation (WAN down)
As a home user I want the gateway to keep serving the local mesh when my Internet goes down so that neighbourhood messaging continues through an outage.
- AC: With WAN unreachable, LoRa/HaLow local relaying and the propagation node keep operating (shard S-17.J behaviour); queued WAN-bound traffic is stored within quota and drains automatically when the uplink returns; WAN-down state is signalled in the admin pane without any mode flapping
- Meta: Shard=J | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=A,O | PRD=F-HGW-07, F-MSG-09, NF-REL | Const=C-08

### S-17-016 — HGW security threat-model review
As a firmware engineer I want a threat-model review of the gateway attack surface so that AP mode and the bridges do not regress device security (risk R17-03).
- AC: Threat model covers soft-AP, captive portal, admin API, and both bridge paths with mitigations mapped to `05_SECURITY_MODEL.md`; every identified high-risk finding has a fix story or accepted-risk record before HGW ships; wg-security sign-off is recorded
- Meta: Shard=— | Type=Task | Size=M | Prio=P0 | Status=DRAFT | SKU=A,O | PRD=NF-SEC | Const=C-05, C-08

### S-17-017 — HGW mode integration test
As a firmware engineer I want an end-to-end HGW integration test rig so that the epic exit criteria are verified on real hardware in CI.
- AC: Rig asserts docked Alpha bridges LoRa mesh to a reachable Reticulum server (exit criterion 1) and serves a phone client as Wi-Fi extender (exit criterion 2); rig asserts undock reversion within 3 s and captive-portal setup within 2 min; test suite runs on every HGW-touching change
- Meta: Shard=— | Type=Ops | Size=M | Prio=P0 | Status=DRAFT | SKU=A,O | PRD=F-HGW-02, F-HGW-03, F-HGW-06 | Const=C-08

### S-17-018 — HGW mode user docs
As a home user I want clear Home Gateway documentation so that I understand what each role shares from my connection and can set it up confidently.
- AC: Docs cover setup, each opt-in role, fair-use caps, and troubleshooting; the extender section explains home-ISP terms considerations and records user consent language per risk R17-01; docs are linked from the captive portal and the companion pane
- Meta: Shard=— | Type=Docs | Size=S | Prio=P0 | Status=DRAFT | SKU=A,O | PRD=F-HGW-06 | Const=C-08, C-07

### S-17-019 — EUD IP tether / gateway mode
As a field operator I want to tether an EUD (phone, laptop, tablet) to my device over USB-ECM or Wi-Fi and reach IP services across the mesh so that ordinary IP applications work through the SS-SP with zero custom software on the EUD.
- AC: an unmodified EUD obtains an address via standard DHCP/SLAAC and routes IP through the device's `ss_rns_iface_tun` interface per the S-11-020 spec; both USB-ECM and Wi-Fi client attachment paths are covered by integration tests; per-EUD traffic is policed by bearer-aware QoS so tethered bulk traffic never starves mesh-critical classes; tether mode is opt-in with a clear on-device indicator while active
- Meta: Shard=— | Type=Feature | Size=L | Prio=P1 | Status=DRAFT | SKU=A,O | PRD=F-EUD-02 | Const=C-08, C-02

### S-17-020 — On-device web admin console (API parity)
As a device owner I want a local web admin console served by the device so that I can configure and monitor it from any browser with no app install and no cloud dependency.
- AC: console is served locally over HTTPS with first-connect trust bootstrap documented; every console action goes through the same public device API used by the companion apps (API parity per F-EUD-03) with zero private endpoints; console is off by default, requires authenticated session per the security model, and its static assets ship in firmware with a bounded size budget
- Meta: Shard=— | Type=Feature | Size=M | Prio=P2 | Status=DRAFT | SKU=A,O | PRD=F-EUD-03 | Const=C-08, C-05

### S-17-021 — Meshtastic-compat gateway on LoRa
As a home-gateway owner I want the gateway to bridge Meshtastic LoRa traffic so that legacy Meshtastic nodes around my home reach the SS-SP mesh and (policy-permitting) the Internet.
- AC: gateway joins a configured Meshtastic channel and relays text/position packets to/from the SS-SP side using the EPIC-13 compat layer; bridging is opt-in per channel with rate limits and duty-cycle enforcement per NF-REG-05; gateway never leaks SS-SP-encrypted payloads onto the Meshtastic channel; bridge status is visible in the gateway admin surface
- Meta: Shard=— | Type=Feature | Size=M | Prio=P2 | Status=DRAFT | SKU=★ | PRD=F-HGW-04, F-BR-07 | Const=C-00, C-08
