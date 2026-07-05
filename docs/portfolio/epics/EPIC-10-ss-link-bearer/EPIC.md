<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# EPIC-10 — SS-Link Multi-Bearer Transport

**Primary WG:** wg-firmware, wg-protocol · **Contributing:** wg-security
**Priority:** P0 · **SKU:** ★ · **Milestone:** M2

## Outcome
A single unified transport abstraction (`ss_link`) sits above every radio (LoRa, HaLow, Wi-Fi 2.4/5, BLE, cellular) and exposes one API to upper layers. Bearer plugins expose capabilities; scheduler picks best bearer for each frame; QoS queue prioritises SOS > presence > text > voice; encryption is bearer-agnostic.

## Constitution
C-08 `08_UNIVERSAL_CONNECTIVITY.md` §SS-Link; C-02 `02_PROTOCOL_STACK.md` §framing.

## Dependencies
EPIC-03 (Lite HAL) at minimum.

## Shards
- **S-10.A `ss_bearer_ops` plugin contract** — send, receive, peer_scan, link_stats, set_power_mode, caps.
- **S-10.B Bearer plugins** — LoRa, HaLow, Wi-Fi STA, Wi-Fi AP, BLE, cellular (each in own file).
- **S-10.C Scheduler & selection policy** — cost/latency/energy scoring.
- **S-10.D QoS queue + prioritisation.**
- **S-10.E Frame format** — small (LoRa) and large (Wi-Fi/HaLow) framing.
- **S-10.F Congestion control & backpressure.**
- **S-10.G Neighbour table** — peer discovery per bearer.
- **S-10.H Bearer-level metrics + telemetry hook.**
- **S-10.I Fault injection & bearer-failover tests.**

## Exit criteria
1. LoRa and Wi-Fi bearer plugins pass identical upper-layer functional tests.
2. SOS frame prioritised over all others under load.
3. Fail-over from Wi-Fi → LoRa within 2 s when bearer drops.
4. Scheduler chooses lowest-energy bearer that meets QoS hint.

## Risks
| # | Risk | Mitigation |
|---|------|-----------|
| R10-01 | Bearer plugin ABI churn | Semver + conformance suite |
| R10-02 | QoS starvation on LoRa | Time-slotted airtime accounting |
| R10-03 | Scheduler thrashing | Hysteresis + hold-down timers |
