<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# EPIC-11 — Stories

Format per `../../00_METHODOLOGY.md` §2.7. Meta lines are machine-parsed.

### S-11-001 — `ss_rns` component skeleton + build glue
As a firmware engineer I want an `ss_rns` component skeleton (C port / bindings) wired into the firmware build so that Reticulum work has a compiling, testable home in the tree.
- AC: component builds as part of the firmware image with CI coverage; public headers expose init/deinit and the interface-registration entry points used by later shards; component links against the crypto core (EPIC-06) without duplicating primitives
- Meta: Shard=A | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-02, C-00

### S-11-002 — RNS Identity struct + persistence
As a protocol engineer I want the RNS Identity mapped to the SS-SP device identity and persisted across reboots so that a device keeps a stable mesh identity for its lifetime.
- AC: RNS Identity is derived from / bound to the device Ed25519 and X25519 keys per shard S-11.B; identity survives reboot and power loss with no regeneration; persisted identity material is stored only in encrypted storage per NF-SEC-02
- Meta: Shard=B | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-SEC-01, F-SEC-02, NF-SEC-02 | Const=C-02, C-05

### S-11-003 — Destination registration API
As a protocol engineer I want an API for registering and unregistering RNS destinations so that upper layers (LXMF, apps) can claim addressable endpoints on the mesh.
- AC: API supports registering multiple destinations with distinct aspects and callbacks; inbound packets are dispatched to the correct destination callback in unit tests; unregistration releases resources with no leaks under repeated register/unregister cycles
- Meta: Shard=B | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-02

### S-11-004 — `ss_rns_iface_lora` interface plugin
As a protocol engineer I want an RNS interface bridging to the SS-Link LoRa bearer so that Reticulum traffic flows over LoRa on every SKU.
- AC: RNS packets transit the SS-Link LoRa bearer end-to-end between two devices; interface respects LoRa MTU by fragmenting per the RNS interface contract; a Lite device announces over LoRa and is discovered by a Python RNS node (exit criterion 1)
- Meta: Shard=C | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-BR-01 | Const=C-02, C-08

### S-11-005 — `ss_rns_iface_halow` interface plugin
As a protocol engineer I want an RNS interface over the HaLow bearer so that Alpha/Omega devices run Reticulum at HaLow range and throughput.
- AC: RNS packets transit the HaLow bearer between two HaLow-equipped devices; interface advertises HaLow's larger MTU so RNS avoids unnecessary fragmentation; path establishment succeeds over a HaLow-only link
- Meta: Shard=D | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=A | PRD=F-BR-04 | Const=C-02, C-08

### S-11-006 — `ss_rns_iface_wifi` interface plugin
As a protocol engineer I want an RNS interface over the Wi-Fi bearer so that devices on the same network or the Internet reach the global RNS mesh.
- AC: RNS packets transit the Wi-Fi bearer to a LAN-attached Python RNS node; interface recovers automatically when Wi-Fi drops and returns; throughput is sufficient for resource transfer tests in shard S-11.H
- Meta: Shard=E | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-BR-02 | Const=C-02, C-08

### S-11-007 — `ss_rns_iface_cellular` interface plugin
As a protocol engineer I want an RNS interface over the cellular bearer so that Omega devices stay on the mesh with no local peers in range.
- AC: RNS packets transit the cellular bearer to a remote transport node; interface marks itself metered and defers bulk resource transfers per bearer policy; interface tolerates cellular latency without spurious path timeouts; D-0021: path timeouts must be per-interface, not global
- Meta: Shard=F | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=O | PRD=F-BR-06 | Const=C-02, C-08

### S-11-008 — Announce packet handling
As a protocol engineer I want RNS announce emission and processing so that devices advertise their destinations and learn about peers.
- AC: device emits valid announces that a Python RNS node accepts and records; received announces populate the local destination knowledge used by path-finding; announce processing is wire-compatible with the pinned upstream RNS version
- Meta: Shard=G | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-02

### S-11-009 — Path-finding + cache
As a protocol engineer I want RNS path request/response handling with a path cache so that devices route to non-adjacent destinations efficiently.
- AC: device resolves a path to a destination two hops away in the test topology; path re-computation on link failure completes < 30 s (exit criterion 3); path cache entries expire and refresh per the pinned RNS semantics
- Meta: Shard=G | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-02

### S-11-010 — Resource GET client API
As a firmware engineer I want a client API for RNS resource GET so that on-device software can fetch resources from remote mesh nodes.
- AC: device completes a resource GET from a Python RNS node with hash-verified payload; interrupted transfer resumes or fails cleanly with a distinct error code; API delivers progress callbacks for large resources
- Meta: Shard=H | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-02

### S-11-011 — Resource PUT server API
As a firmware engineer I want the device to serve RNS resources (PUT/serve side) so that peers and gateways can retrieve data the device offers.
- AC: a Python RNS node completes a resource GET from the device (exit criterion 2); concurrent transfers are bounded by a configurable limit without crashing; serving respects the flash-cache quota from S-11-012
- Meta: Shard=H | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-02

### S-11-012 — Flash-backed resource cache
As a firmware engineer I want a flash-backed resource cache with size cap and TTL so that resource transfers survive reboots without wearing out flash (risk R11-03).
- AC: cache enforces its configured size cap by evicting expired/oldest entries; entries expire on TTL and are reaped without device restart; write pattern is wear-aware (measured write amplification documented); cached content is stored encrypted per NF-SEC-02
- Meta: Shard=H | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=NF-SEC-02 | Const=C-02

### S-11-013 — Interop test w/ Python RNS reference
As a protocol engineer I want an automated interop suite against the Python reference RNS so that wire compatibility with upstream is proven continuously (risk R11-02).
- AC: suite covers announce, path-finding, link establishment, and resource GET/PUT against the pinned reference version; suite runs in CI on every change to `ss_rns`; version pin and upgrade procedure are documented
- Meta: Shard=I | Type=Ops | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-02

### S-11-014 — Interop test w/ Rust rusty-reticulum
As a protocol engineer I want interop tests against the Rust rusty-reticulum implementation so that compatibility is proven against a second independent implementation (toward exit criterion 4).
- AC: announce and link establishment succeed bidirectionally against rusty-reticulum; any incompatibilities are filed with a minimal reproduction and tracked to closure; suite is runnable in CI on demand
- Meta: Shard=I | Type=Ops | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=— | Const=C-02

### S-11-015 — Interop test w/ another SS-SP
As a protocol engineer I want device-to-device interop tests between two SS-SP units so that the C port is proven against itself on real radios, not just against desktop implementations.
- AC: two Lite units complete announce, path establishment, and resource transfer over LoRa on the hardware rack; test also passes over the Wi-Fi interface; test runs against release candidates as part of the hardware-in-loop gate
- Meta: Shard=I | Type=Ops | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-02

### S-11-016 — Licence-compliance review + notice bundling
As a release manager I want a completed legal review of the Reticulum licence terms (including no-harm and no-AI-training conditions) with required notices bundled so that shipping `ss_rns` creates no licence exposure (risk R11-01).
- AC: wg-legal review of the Reticulum licence is recorded with distribution constraints and the module-boundary decision; all required licence and notice texts ship in the firmware notice bundle; SBOM entry for Reticulum-derived code is correct per NF-SEC-05
- Meta: Shard=J | Type=Task | Size=S | Prio=P0 | Status=DRAFT | SKU=★ | PRD=NF-SEC-05 | Const=C-04

### S-11-017 — Announce rate-limit + storm protection
As a firmware engineer I want announce rate-limiting and storm suppression so that a misbehaving or hostile node cannot flood the mesh or drain device batteries.
- AC: inbound announce processing is rate-limited per source with excess dropped and counted; own-announce emission respects the configured minimum interval even under rapid destination changes; synthetic announce-storm test shows bounded CPU and airtime use
- Meta: Shard=G | Type=Feature | Size=S | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-02

### S-11-018 — RNS transport metrics + counters
As a firmware engineer I want RNS-layer metrics (packets in/out, path hits/misses, announce counts, resource bytes) so that diagnostics and the network log can show mesh health.
- AC: counters are exposed through the same metrics surface consumed by diagnostics per F-APP-05; counters survive being read concurrently with traffic (no torn reads in the stress test); counter overhead is negligible at P95 packet-processing latency
- Meta: Shard=A | Type=Feature | Size=S | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-APP-05 | Const=C-02

### S-11-019 — RNS integration RFC (RFC-0007)
As a protocol engineer I want RFC-0007 specifying how RNS integrates with SS-SP (identity mapping, interface model, version pinning, upgrade policy) so that the integration is reviewable and stable across releases.
- AC: RFC-0007 covers identity ↔ destination mapping, the SS-Link interface model, the upstream version-pin and upgrade policy, and interop requirements; RFC reaches ACCEPTED via the governance process; accepted RFC is referenced by the `ss_rns` component docs
- Meta: Shard=— | Type=RFC | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-02, C-06

### S-11-020 — IP-over-RNS tunnel interface spec (`ss_rns_iface_tun`)
As a protocol engineer I want a TUN-semantics IP-over-RNS interface specified by RFC before any consumer ships so that EUD tethering, HGW Ethernet, and IP-mesh bridging all consume one stable spec instead of inventing three.
- AC: RFC specifies MTU discipline, fragmentation policy, and addressing derived from RNS identity per `03_ARCHITECTURE.md` §4.6; a reference implementation round-trips IPv4/IPv6 packets between two devices over the mesh in an integration test; spec reaches ACCEPTED via the governance process before F-EUD-02 implementation starts; D-0021: HGW deployment only, not a handheld-SKU claim
- Meta: Shard=— | Type=RFC | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-EUD-02 | Const=C-02, C-08, C-06

### S-11-021 — Host-portable `ss_node_core` library build
As a protocol engineer I want `ss_rns` + `ss_lxmf` + the crypto core building as one host-portable library from the same sources as firmware so that companion apps and the headless daemon run the identical protocol code instead of a re-implementation.
- AC: `ss_node_core` builds for Linux, macOS, Windows, Android NDK, and iOS from the firmware sources behind a narrow documented OS-abstraction layer; CI builds and runs the full protocol test suite on at least Linux x86-64 and Android arm64 on every change; public headers contain no ESP-IDF/FreeRTOS types, enforced by a CI header lint; host and firmware builds pass the same cross-implementation interop vectors
- Meta: Shard=A | Type=Feature | Size=L | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-NODE-01 | Const=C-02, C-00

### S-11-022 — `ss_rns_iface_ble` phone ↔ device RNS interface
As a protocol engineer I want BLE (and USB serial) as a first-class RNS interface between a device and a paired app node so that phone and device form one routing domain with either side's bearers carrying traffic for both.
- AC: RNS packets transit the BLE GATT link between device and app node with MTU discipline and fragmentation per the RNS interface contract; in an integration test, traffic originated on the phone reaches a LoRa-only peer via the device and vice versa (one routing domain); the interface admits only bonded, fingerprint-verified peers per the pairing trust model; link drop and reconnect re-converge routes without message loss
- Meta: Shard=— | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-NODE-04 | Const=C-02, C-08

### S-11-023 — Announce budgets + transport-tier profile for constrained bearers
As a protocol engineer I want per-bearer announce budgets and a leaf/transport tier profile so that a 1 000-node LoRa region spends its duty-cycle airtime on user traffic instead of drowning in path-table chatter.
- AC: constrained-bearer nodes default to leaf (non-transport) mode with transport role reserved for mains-powered/gateway profiles, user-overridable; announce ingress and egress budgets are enforced per bearer with priority eviction that never evicts paths in active use; cross-bearer announce damping re-emits fast-bearer announces onto LoRa only when the destination is plausibly reachable there; simulation of a 1 000-node region shows combined announce airtime ≤ 5 % of the regional duty-cycle budget per NF-SCALE-02; path-table caps fit Lite/Alpha RAM budgets with LRU + pin-active policy and are covered by unit tests; D-0021: HaLow-scenario equivalent for A/O tracked at story elaboration
- Meta: Shard=— | Type=Feature | Size=L | Prio=P1 | Status=DRAFT | SKU=L | PRD=NF-SCALE-02 | Const=C-02, C-08

### S-11-024 — Hostile-network resilience for IP transports (captive portals, TLS interception)
As a device owner I want IP-based transports to survive captive portals, TLS-intercepting middleboxes, and DNS tampering so that hotel/airport/corporate Wi-Fi degrades gracefully instead of silently breaking connectivity.
- AC: captive-portal state is detected and surfaced to the user (with soft-AP hand-off to a phone browser where applicable) instead of the bearer reporting a false healthy link; connections to Tier-2 cloud endpoints (relay, OTA, provisioning) pin expected certificate authorities and fail closed with a clear user-visible reason on interception, never falling back to unverified TLS; RNS-over-IP interfaces treat middlebox tampering as link failure and trigger normal bearer failover per NF-PERF-02; no hostile-network condition ever degrades Tier-0 off-grid operation (device-to-device bearers are unaffected by IP-side failures, verified by test)
- Meta: Shard=— | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=NF-REL-01, NF-PERF-02 | Const=C-02, C-05, C-08
