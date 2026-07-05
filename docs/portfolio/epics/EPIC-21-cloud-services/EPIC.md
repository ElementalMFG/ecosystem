<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# EPIC-21 — Cloud Services (Fleet Console, Relay, Provisioning, Registry)

**Primary WG:** wg-cloud · **Contributing:** wg-ops, wg-security, wg-legal
**Priority:** P1 · **SKU:** ★ · **Milestone:** M6

## Outcome
Optional cloud services support the SS-SP ecosystem:
- **Fleet Console** — enterprise device fleet mgmt (self-host or SaaS).
- **Relay** — Reticulum/LXMF relays for global mesh backhaul.
- **Provisioning Service** — factory-line + reprovisioning API.
- **Plugin Registry** — signed plugins, ratings, revocations.
Licensed under BSL 1.1 with a 4-year Apache-2.0 conversion (per C-04 `04_LICENSING_AND_FORK_STRATEGY.md`).

## Constitution
C-07 `07_BUSINESS_MODEL_AND_OPEN_SOURCE.md` §cloud tiers; C-04 `04_LICENSING_AND_FORK_STRATEGY.md` §BSL.

## Dependencies
EPIC-06 (crypto), EPIC-07 (provisioning), EPIC-11/12 (relay protocols), EPIC-18 (plugins).

## Shards
- **S-21.A Fleet Console backend** — Postgres + Go/Rust API.
- **S-21.B Fleet Console frontend** — React SPA.
- **S-21.C Multi-tenant identity + RBAC.**
- **S-21.D Relay service** — RNS relay, geo-anycast.
- **S-21.E Provisioning service API.**
- **S-21.F Plugin Registry** — HSM-signed, revocation lists.
- **S-21.G Observability** — OpenTelemetry, Prometheus, Loki.
- **S-21.H Self-host packaging** — Helm chart, docker-compose.
- **S-21.I Data protection & GDPR posture.**

## Exit criteria
1. Fleet Console beta shipped to one paying pilot customer.
2. Relay achieves ≥ 99.5 % monthly uptime in test rig.
3. Provisioning service processes 1 000 devices/day in staging.
4. Self-host Helm chart deploys on kind cluster.

## Risks
| # | Risk | Mitigation |
|---|------|-----------|
| R21-01 | GDPR breach | DPA + data-min posture |
| R21-02 | BSL confusion | FAQ + auto-convert clarity |
| R21-03 | Cloud-vendor lock-in | Cloud-agnostic infra |
