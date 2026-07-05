<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# EPIC-17 — Home Gateway Mode

**Primary WG:** wg-firmware, wg-cloud · **Contributing:** wg-ui-ux, wg-security
**Priority:** P0 · **SKU:** A, O · **Milestone:** M5

## Outcome
When docked (or user-enabled), the device switches into Home Gateway (HGW) mode: it bridges HaLow ↔ LoRa ↔ Wi-Fi ↔ Internet, acts as a Wi-Fi extender / AP for existing home Wi-Fi, hosts a small Reticulum/LXMF propagation node, and exposes a captive-portal setup UX. When the user takes the device out, HGW mode gracefully shuts down and mesh-mode resumes.

## Constitution
C-08 `08_UNIVERSAL_CONNECTIVITY.md` §Home Gateway; C-07 `07_BUSINESS_MODEL_AND_OPEN_SOURCE.md` §Home Mode.

## Dependencies
EPIC-04 (HaLow HAL), EPIC-10, EPIC-11.

## Shards
- **S-17.A Dock detection** — physical dock contacts + power state signal.
- **S-17.B Mode-switch state machine** — mesh ↔ HGW transitions.
- **S-17.C Wi-Fi client to home AP** (uplink).
- **S-17.D Wi-Fi extender / soft-AP (downlink).**
- **S-17.E Bridge LoRa ↔ Internet (LXMF propagation via cloud relay).**
- **S-17.F Bridge HaLow ↔ Internet.**
- **S-17.G Captive portal setup UX.**
- **S-17.H Local admin API (companion app).**
- **S-17.I Traffic accounting + fair use.**
- **S-17.J Off-line graceful degrade — HGW keeps propagating on mesh even without WAN.**

## Exit criteria
1. Docked Alpha bridges LoRa mesh to Internet (Reticulum server reachable).
2. Alpha operates as Wi-Fi extender for a phone client.
3. Un-docking within 3 s reverts to mobile mesh mode without user-visible fault.
4. Captive-portal setup completes in ≤ 2 min.

## Risks
| # | Risk | Mitigation |
|---|------|-----------|
| R17-01 | Home ISP terms violation (extender) | User consent + docs |
| R17-02 | Thermal budget in HGW mode | Fan-less thermal test, throttle |
| R17-03 | Security regressions in AP mode | Threat model review |
