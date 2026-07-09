<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# EPIC-10 — Stories

Format per `../../00_METHODOLOGY.md` §2.7. Meta lines are machine-parsed.

### S-10-001 — `ss_bearer_ops` header + reference doc
As a firmware engineer I want a frozen `ss_bearer_ops` plugin contract (send, receive, peer_scan, link_stats, set_power_mode, caps) with a reference document so that every bearer implements one identical interface.
- AC: header defines all six operations plus a capability descriptor and is semver-versioned (per R10-01); reference doc specifies threading, ownership, and error-code semantics for each operation; a stub bearer built only from the header and doc links and passes the contract smoke test
- Meta: Shard=A | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-08

### S-10-002 — LoRa bearer plugin
As a firmware engineer I want a LoRa (SX1262) bearer plugin implementing `ss_bearer_ops` so that the mandatory long-range bearer is available to the scheduler on every SKU.
- AC: plugin implements all `ss_bearer_ops` operations and reports correct capabilities (small-frame, high-latency, low-power); two devices exchange frames over LoRa through the plugin; plugin passes the same upper-layer functional tests as the Wi-Fi plugin (exit criterion 1); EU 868 MHz duty-cycle limits enforced per NF-REG-05
- Meta: Shard=B | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=L | PRD=F-BR-01, NF-REG-05 | Const=C-08

### S-10-003 — Wi-Fi STA bearer plugin
As a firmware engineer I want a Wi-Fi station-mode bearer plugin so that devices on an access point get a high-throughput bearer through the same `ss_link` API.
- AC: plugin implements all `ss_bearer_ops` operations including link_stats (RSSI, throughput); large-frame transfer succeeds over Wi-Fi STA; plugin passes the shared upper-layer functional test suite (exit criterion 1)
- Meta: Shard=B | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-BR-02 | Const=C-08

### S-10-004 — Wi-Fi AP bearer plugin (for HGW)
As a firmware engineer I want a Wi-Fi access-point bearer plugin so that a Home Gateway device can serve nearby peers and hotspot clients directly.
- AC: plugin brings up AP mode and exchanges `ss_link` frames with an associated peer device; AP mode is opt-in per SSID configuration consistent with F-HGW-06; concurrent operation with the STA plugin (or documented exclusivity) is verified on target hardware
- Meta: Shard=B | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-BR-02, F-HGW-06 | Const=C-08

### S-10-005 — BLE bearer plugin
As a firmware engineer I want a BLE 5.x bearer plugin so that companion apps and nearby devices can exchange frames over the mandatory short-range bearer.
- AC: plugin implements `ss_bearer_ops` over BLE with MTU-aware fragmentation; a companion-app peer completes a frame exchange through the plugin; plugin reports correct capabilities (small-frame, short-range, low-power) to the scheduler
- Meta: Shard=B | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-BR-03 | Const=C-08

### S-10-006 — HaLow bearer plugin (Alpha)
As a firmware engineer I want a Wi-Fi HaLow (MM8108) bearer plugin so that Alpha and Omega devices get a long-range, higher-throughput bearer than LoRa.
- AC: plugin implements all `ss_bearer_ops` operations on MM8108 hardware; frame exchange between two HaLow devices succeeds at a range where 2.4 GHz Wi-Fi has dropped; HaLow region-code enforcement disables the bearer on wrong-region configuration per NF-REG-04
- Meta: Shard=B | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=A | PRD=F-BR-04, NF-REG-04 | Const=C-08

### S-10-007 — Cellular bearer plugin (Omega)
As a firmware engineer I want an LTE-M / NB-IoT cellular bearer plugin so that Omega devices reach the wider network where no local mesh exists.
- AC: plugin implements all `ss_bearer_ops` operations over the cellular modem; capability descriptor marks the bearer as metered so the scheduler can respect data caps; plugin honours modem power-saving (PSM) states consistent with NF-PWR-03
- Meta: Shard=B | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=O | PRD=F-BR-06, NF-PWR-03 | Const=C-08

### S-10-008 — Frame format v1 (small + large variants)
As a protocol engineer I want an RFC-specified `ss_link` frame format with a small variant for LoRa and a large variant for Wi-Fi/HaLow so that every bearer carries the same semantics in a wire-verifiable shape.
- AC: RFC specifies both variants including header layout, priority bits, fragmentation, and version field; reference encoder/decoder round-trips published test vectors byte-exactly; small-variant overhead fits LoRa payload budgets; RFC reaches ACCEPTED via the governance process
- Meta: Shard=E | Type=RFC | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-02, C-08, C-06
- Deps: RFC-0003 (compat/deprecation policy — version-field window & sunset are governed there, not restated here)

### S-10-009 — Scheduler w/ cost/latency/energy scoring
As a firmware engineer I want a bearer scheduler that scores available bearers on cost, latency, and energy per frame so that each frame goes out on the best bearer for its QoS hint.
- AC: scheduler chooses the lowest-energy bearer that meets the frame's QoS hint (exit criterion 4); scoring inputs come from live link_stats, not static tables alone; bearer switch after loss completes within the NF-PERF-02 3 s budget in the failover test
- Meta: Shard=C | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=NF-PERF-02 | Const=C-08

### S-10-010 — QoS priority queue (SOS, presence, text, voice, bulk)
As a first responder I want SOS frames to preempt all other traffic in the transmit queue so that an emergency beacon is never stuck behind bulk transfers.
- AC: SOS frames preempt presence, text, voice, and bulk classes under sustained load (exit criterion 2); SOS reaches on-air within the NF-PERF-04 2 s budget with a full queue; per-class ordering is FIFO within a class and starvation of lower classes is bounded and measured
- Meta: Shard=D | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-MSG-08, NF-PERF-04 | Const=C-08

### S-10-011 — Airtime accounting for LoRa
As a firmware engineer I want per-device LoRa airtime accounting with time-slotted budgets so that regulatory duty-cycle limits are respected and QoS starvation on LoRa is prevented (risk R10-02).
- AC: accounting tracks cumulative on-air time per regulatory window and refuses TX that would exceed the EU 868 MHz duty-cycle limit per NF-REG-05; remaining airtime budget is exposed to the scheduler for scoring; accounting counters are observable via the metrics hook
- Meta: Shard=D | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=NF-REG-05, F-BR-01 | Const=C-08

### S-10-012 — Backpressure signal upstream to LXMF/RNS
As a protocol engineer I want `ss_link` to signal queue pressure upstream to RNS/LXMF so that upper layers throttle instead of overflowing the transmit queue.
- AC: backpressure signal fires at the configured queue-depth threshold and clears with hysteresis; upper-layer test shows send rate reduces on signal instead of dropping frames; SOS-class frames are exempt from backpressure rejection
- Meta: Shard=F | Type=Feature | Size=S | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-08

### S-10-013 — Neighbour table per bearer
As a firmware engineer I want a per-bearer neighbour table populated by peer_scan and passive reception so that the scheduler and upper layers know which peers are reachable on which bearer.
- AC: neighbour entries record bearer, last-seen time, and link quality, and expire on a configurable TTL; peer_scan on each implemented bearer populates the table on the two-device rig; table contents are queryable by upper layers through a stable API
- Meta: Shard=G | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-08

### S-10-014 — Bearer metrics hook (RSSI, SNR, PER)
As a firmware engineer I want a metrics hook exposing per-bearer RSSI, SNR, and packet-error-rate so that diagnostics and the network log can show real link health.
- AC: metrics update from live link_stats on every implemented bearer; diagnostics/network-log surface per F-APP-05 can read the metrics through the hook without touching bearer internals; metrics collection adds no measurable TX-path latency at P95
- Meta: Shard=H | Type=Feature | Size=S | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-APP-05 | Const=C-08

### S-10-015 — Fail-over test suite (bearer down → next-best)
As a firmware engineer I want an automated fail-over test suite that kills bearers and verifies re-selection so that fail-over behaviour is regression-tested rather than assumed.
- AC: Wi-Fi → LoRa fail-over completes within 2 s when Wi-Fi drops (exit criterion 3); suite covers each implemented bearer being killed and restored, verifying traffic resumes; suite runs in CI with fault injection per shard S-10.I
- Meta: Shard=I | Type=Ops | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=NF-PERF-02 | Const=C-08

### S-10-016 — Bearer conformance-suite for external plugins
As an external plugin developer I want a public conformance suite for `ss_bearer_ops` so that I can prove my third-party bearer behaves identically to first-party plugins.
- AC: suite exercises every `ss_bearer_ops` operation, capability reporting, and error paths against a candidate plugin; first-party LoRa and Wi-Fi plugins pass the suite unmodified; suite reports a versioned pass/fail result usable as evidence for certification
- Meta: Shard=A | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-CERT-01 | Const=C-08

### S-10-017 — Hysteresis + hold-down anti-thrash
As a firmware engineer I want hysteresis and hold-down timers in bearer selection so that the scheduler does not thrash between bearers with similar scores (risk R10-03).
- AC: a synthetic scenario with two oscillating bearer scores produces no more than one switch per hold-down window; hold-down and hysteresis margins are configurable and documented defaults are justified; SOS frames may still override hold-down to use any available bearer
- Meta: Shard=C | Type=Feature | Size=S | Prio=P1 | Status=DRAFT | SKU=★ | PRD=NF-PERF-02 | Const=C-08

### S-10-018 — Power-mode plumbing (idle/tx/rx/sleep) per bearer
As a firmware engineer I want set_power_mode plumbed through every bearer plugin so that the power manager can put idle radios to sleep and meet standby targets.
- AC: each implemented bearer honours idle/tx/rx/sleep transitions and reports its current mode; measured standby draw with all bearers in sleep supports the NF-PWR-01/02/03 SKU budgets; wake-from-sleep latency per bearer is measured and published for scheduler use
- Meta: Shard=A | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=NF-PWR-01, NF-PWR-02, NF-PWR-03 | Const=C-08

### S-10-019 — Bearer selection RFC (RFC-0006)
As a protocol engineer I want the bearer-selection policy (scoring, QoS mapping, fail-over, hold-down) specified in RFC-0006 so that selection behaviour is reviewable and stable across implementations.
- AC: RFC-0006 specifies scoring inputs, QoS-hint-to-class mapping, fail-over triggers, and anti-thrash behaviour; RFC includes worked examples matching the shipped scheduler's observable behaviour; RFC reaches ACCEPTED via the governance process before the scheduler is declared stable
- Meta: Shard=C | Type=RFC | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=NF-PERF-02 | Const=C-08, C-06

### S-10-020 — Bearer ABI: reserved L2/IP bearer class + capability flags
As a protocol engineer I want the bearer plugin ABI to reserve `IP_NATIVE`, `L2_MESH`, and `CHANNEL_AGILE` capability flags and a generic bearer class from v1.0 so that future IP-native bearers (802.11s, Ethernet, future radios) slot in as plugins with no ABI break.
- AC: `caps` bitfield reserves the three flags with documented semantics per `03_ARCHITECTURE.md` §4.6; the scheduler handles an unknown/generic bearer class without special-casing (proven by a mock IP bearer in tests); ABI reservation is covered by the SS-Link RFC so third-party bearer authors can rely on it
- Meta: Shard=A | Type=Feature | Size=S | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-BR-09 | Const=C-08, C-02

### S-10-021 — Channel-agility hooks (interference metrics + in-plan channel move)
As a firmware engineer I want every bearer to expose `interference_metrics()` and an optional region-plan-constrained `move_channel()` callback so that channel-agility policy is a table change, not a driver change.
- AC: all v1.0 bearers report interference/degradation metrics on a bounded cadence; `move_channel()` rejects any target outside the region channel plan at the API level per NF-REG-04; a policy-table-driven agility demo moves a bearer off a jammed channel in the HIL rig and logs the agility event
- Meta: Shard=A | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-BR-08, NF-REG-04 | Const=C-08

### S-10-022 — 802.11s L2 IP-mesh interop bearer plugin
As a neighbourhood-mesh enthusiast I want an opt-in 802.11s mesh bearer so that my device can join or serve standard IP-mesh networks (OpenWRT and MANET-class gear) as one more SS-Link bearer.
- AC: device joins an OpenWRT-hosted 802.11s mesh and exchanges RNS traffic over it end-to-end; bearer implements the reserved `L2_MESH`/`IP_NATIVE` capability flags from S-10-020 with no scheduler special-casing; interop against at least one third-party 802.11s device is demonstrated and documented
- Meta: Shard=— | Type=Feature | Size=L | Prio=P2 | Status=DRAFT | SKU=A, O | PRD=F-BR-09 | Const=C-08
