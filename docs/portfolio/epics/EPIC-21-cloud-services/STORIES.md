<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# EPIC-21 — Stories

Format per `../../00_METHODOLOGY.md` §2.7. Meta lines are machine-parsed.

### S-21-001 — Fleet Console backend skeleton (Go/Rust + Postgres)
As a fleet admin I want a Fleet Console backend service with Postgres persistence so that enterprise device management has a reliable API foundation.
- AC: API service boots with schema migrations and a health endpoint; CI runs unit and integration tests against an ephemeral Postgres; every source file carries the BSL 1.1 header with the Apache-2.0 change date per C-04
- Meta: Shard=A | Type=Feature | Size=L | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-CL-01 | Const=C-07, C-04

### S-21-002 — Fleet Console frontend skeleton (React)
As a fleet admin I want a React SPA shell for the Fleet Console so that tenant operators have a usable management surface.
- AC: SPA authenticates against the backend and renders a tenant dashboard shell; production build meets the NF-PERF-06 p95 page-load budget in Lighthouse CI; component test harness runs in CI
- Meta: Shard=B | Type=Feature | Size=L | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-CL-01, NF-PERF-06 | Const=C-07, C-04

### S-21-003 — Multi-tenant model + RBAC roles
As a fleet admin I want strict tenant isolation with owner/admin/operator/viewer roles so that no tenant can ever see another tenant's devices.
- AC: tenant isolation is enforced at the data layer with tests proving no cross-tenant reads or writes; every API route enforces role checks; role grants and revocations are written to an audit log
- Meta: Shard=C | Type=Feature | Size=L | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-CL-01 | Const=C-07, C-04

### S-21-004 — Device enrollment API
As a fleet admin I want a device enrollment API so that factory-provisioned devices join my tenant securely.
- AC: a device enrols using its factory-provisioned identity and appears in the tenant inventory; enrolment with a duplicate or foreign identity is rejected; every enrolment attempt is recorded in the audit trail
- Meta: Shard=A | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-CL-01 | Const=C-07, C-04, C-05

### S-21-005 — Device inventory + telemetry view
As a fleet admin I want an inventory and telemetry view so that I can see fleet health at a glance.
- AC: inventory lists devices with firmware version, battery, and last-seen; telemetry views show only opt-in data and never message content, verified by test; views meet the NF-PERF-06 p95 load budget
- Meta: Shard=B | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-CL-01, NF-PERF-06 | Const=C-07, C-04

### S-21-006 — Fleet OTA control (cohort)
As a fleet admin I want staged cohort OTA rollouts so that a bad firmware update never hits the whole fleet at once.
- AC: rollout stages 5 % / 25 % / 100 % with automatic halt on regression signal; per-cohort rollback is issuable and verified in staging; only dual-signed images (F-SEC-08) are accepted for distribution
- Meta: Shard=A | Type=Feature | Size=L | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-CL-01, F-SEC-08 | Const=C-07, C-04, C-05

### S-21-007 — Fleet policy engine (crypto suite, radios enabled)
As a fleet admin I want a policy engine controlling crypto suite and enabled radios per device group so that fleet devices comply with site rules.
- AC: policies are schema-validated before push and applied per device group; device-side application is acknowledged and visible per device in the console; conflicting policies resolve deterministically with the resolution order documented
- Meta: Shard=A | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-CL-01, F-SEC-03 | Const=C-07, C-04

### S-21-008 — Relay service RNS transport node
As an SRE I want a deployable RNS relay service so that field devices get global mesh backhaul through commercial relays.
- AC: relay carries RNS/LXMF traffic between two field devices in a staging deployment; relay handles only encrypted payloads with no plaintext access, verified by design review and test; horizontal scale-out procedure is documented
- Meta: Shard=D | Type=Feature | Size=L | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-CL-04 | Const=C-07, C-04, C-08

### S-21-009 — Relay geo-anycast + health checks
As an SRE I want geo-anycast routing with health checks for the relay fleet so that clients always reach the nearest healthy relay.
- AC: clients are routed to the nearest healthy relay in multi-region test; a failed relay is drained within the health-check interval; measured uptime is reported against the ≥ 99.5 % monthly target (epic exit criterion 2)
- Meta: Shard=D | Type=Ops | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-CL-04 | Const=C-07, C-04, C-08

### S-21-010 — Provisioning service REST API
As a fleet admin I want a provisioning REST API for factory-line and reprovisioning flows so that devices are onboarded at production scale.
- AC: API sustains 1 000 devices/day in staging (epic exit criterion 3); every issued device manifest is written to the transparency log per F-MFG-04; factory clients authenticate with mutual auth and least-privilege credentials
- Meta: Shard=E | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-CL-02, F-MFG-01, F-MFG-04 | Const=C-07, C-04, C-05

### S-21-011 — Plugin Registry backend
As a plugin developer I want a Plugin Registry backend so that signed plugins can be published, rated, and distributed to devices.
- AC: registry stores versioned plugins with ratings metadata; uploads without a valid signature are rejected; registry API serves plugin metadata to both devices and the dev-kit CLI
- Meta: Shard=F | Type=Feature | Size=L | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-CL-03 | Const=C-07, C-04, C-05

### S-21-012 — Plugin Registry signing (HSM)
As an SRE I want registry publishing signatures performed inside an HSM so that the plugin signing key can never be exfiltrated.
- AC: publish-time signing executes inside the HSM with the key marked non-exportable; the signing ceremony and operator roles are documented; the device sandbox loader verifies a registry-signed plugin in an end-to-end test
- Meta: Shard=F | Type=Ops | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-CL-03, F-SEC-11 | Const=C-07, C-04, C-05

### S-21-013 — Revocation-list distribution
As a fleet admin I want plugin revocation lists distributed to devices so that a malicious plugin can be neutralised fleet-wide.
- AC: a revoked plugin is disabled on online devices within a defined propagation SLA in test; the revocation list is signed and strictly monotonic so it cannot be rolled back; offline devices re-validate installed plugins on next contact
- Meta: Shard=F | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-CL-03 | Const=C-07, C-04, C-05

### S-21-014 — OpenTelemetry instrumentation
As an SRE I want OpenTelemetry traces, metrics, and logs across all cloud services so that incidents are diagnosable end-to-end.
- AC: all services emit OTel signals with a common resource-attribute convention; a scrub test proves no PII or message payloads appear in telemetry; sampling rates are configurable per deployment
- Meta: Shard=G | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=NF-REL-04 | Const=C-07, C-04

### S-21-015 — Prometheus + Loki stack
As an SRE I want a Prometheus + Loki observability stack with SLO alerting so that the 99.9 % Fleet Console SLA is measurable and defended.
- AC: dashboards and burn-rate alerts exist for the NF-REL-04 SLO; log retention obeys the data-protection policy (S-21-018); every alert links to a runbook
- Meta: Shard=G | Type=Ops | Size=S | Prio=P1 | Status=DRAFT | SKU=★ | PRD=NF-REL-04 | Const=C-07, C-04

### S-21-016 — Helm chart for self-host
As an SRE I want a Helm chart deploying the full cloud stack so that self-hosting customers can run everything on their own cluster.
- AC: chart deploys the full stack on a kind cluster (epic exit criterion 4); values are documented for self-host including air-gapped image registries; upgrade and rollback paths are tested
- Meta: Shard=H | Type=Ops | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-CL-01 | Const=C-07, C-04

### S-21-017 — docker-compose for dev
As a mobile app developer I want a one-command docker-compose stack so that I can develop against local cloud services without Kubernetes.
- AC: `docker compose up` yields a working local stack in under 10 minutes on a clean machine; a demo tenant with seed data is created automatically; parity gaps versus the Helm deployment are documented
- Meta: Shard=H | Type=Ops | Size=S | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-CL-01 | Const=C-07, C-04

### S-21-018 — GDPR DPA + privacy notice
As legal counsel I want a DPA and privacy notice covering the cloud services so that enterprise customers can sign compliantly and GDPR risk (R21-01) is contained.
- AC: DPA and privacy notice cover data categories, sub-processors, and retention including location-data lifetime ≤ 30 days per NF-PRIV-03; the data-subject-request path meets the ≤ 30 day NF-PRIV-04 deadline in a dry run; wg-legal sign-off is recorded
- Meta: Shard=I | Type=Task | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=NF-PRIV-03, NF-PRIV-04 | Const=C-07

### S-21-019 — BSL 1.1 licence banner + FAQ
As legal counsel I want a BSL 1.1 banner and FAQ with the pre-committed 4-year Apache-2.0 change date so that self-hosters are not confused about their rights (risk R21-02).
- AC: every cloud-service repo displays the BSL 1.1 grant and its Apache-2.0 change date; the FAQ explains permitted use, the additional-use grant, and the automatic conversion; wg-legal sign-off is recorded
- Meta: Shard=— | Type=Docs | Size=S | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-04, C-07

### S-21-020 — First pilot customer onboarding runbook
As a support lead I want a pilot-customer onboarding runbook so that the first paying Fleet Console pilot (epic exit criterion 1) is repeatable, not heroic.
- AC: runbook covers tenant creation, device enrolment, policy setup, and first OTA end-to-end; a dry run against staging is executed successfully by someone other than the author; escalation contacts and response expectations are stated
- Meta: Shard=— | Type=Ops | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-CL-01 | Const=C-07, C-04

### S-21-021 — Community pairing bridge
As a device owner I want an opt-in cloud pairing bridge that passes only encrypted blobs so that two parties who have never met on-mesh can exchange pairing material without the service learning anything.
- AC: bridge relays fixed-size encrypted pairing blobs matched by short-lived rendezvous codes; service stores nothing after exchange completes and logs no identities (verified in design review + retention test); bridge is opt-in, fully skippable, and all pairing flows work without it on local bearers; abuse controls (rate limits, code expiry) are in place
- Meta: Shard=— | Type=Feature | Size=M | Prio=P2 | Status=DRAFT | SKU=★ | PRD=F-CL-06 | Const=C-02, C-05, C-07

### S-21-022 — Billing & entitlements service
As a commercial lead I want a billing and entitlements service handling subscriptions, marketplace payouts, and certification fees so that every revenue stream in Business Model §3 has working commerce plumbing without ever gating core device function.
- AC: subscriptions (Fleet Console tiers, relay reservations, retention tiers) and one-off charges (certification fees) are created, invoiced, and reconciled through a PSP adapter that is swappable for self-hosters; entitlement tokens are signed capability grants cached on tenant/device, and an expired entitlement never disables an already-functioning offline capability (verified against the S-22-023 blackhole soak — lapsed subscription means no new premium provisioning, zero degradation); PCI scope is isolated to the billing service and documented; entitlement checks are absent from all Tier-0/1 code paths (CI lint)
- Meta: Shard=— | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-CL-07, NF-COST-03 | Const=C-04, C-07

### S-21-023 — Plugin marketplace commerce & developer payouts
As a plugin developer I want marketplace listings, paid purchases, and transparent payouts so that premium plugins are a real revenue stream for me and for SS-SP while the registry's ADD-only rule stays intact.
- AC: developers list free or paid plugins through the existing signed-review pipeline with a published fee split; purchases grant entitlements via S-21-022 and install through the standard Plugin Registry flow; a developer dashboard shows sales and payouts against an open payout-ledger format; paid plugins may only ADD capability — a review-pipeline gate rejects any plugin that disables or degrades core function when unlicensed
- Meta: Shard=— | Type=Feature | Size=L | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-CL-03, F-CL-07 | Const=C-04, C-06, C-07

### S-21-024 — Relay federation operator program
As an infrastructure operator I want to peer my relay with the commercial federation under a quota contract so that the federation's capacity grows beyond what SS-SP alone can fund while paying fleets still get guaranteed service.
- AC: third-party operators onboard via a published program (technical requirements, quota contract, revenue share) using the same relay software SS-SP operates; peered relays are steered by the geo-anycast layer with health checks and automatic de-peering on SLA breach; a load test demonstrates add-relay = add-capacity with no shared-state bottleneck per NF-SCALE-03; community (non-contracted) relays continue to interoperate at best-effort with zero program obligations
- Meta: Shard=— | Type=Feature | Size=L | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-CL-04, NF-SCALE-03 | Const=C-04, C-07, C-OA

### S-21-025 — Signed bootstrap directory + community mirrors
As a new node operator I want a signed, mirrorable bootstrap directory of first-contact RNS transports so that internet-attached nodes find the mesh in seconds while nothing forces anyone to use — or trust — our list.
- AC: the directory is a signed document listing community and commercial transport endpoints, fetched over HTTPS and verified against a published key; clients treat it as a hint only — user-configured interfaces always take precedence and the directory is fully overridable and ignorable; the document and its signing key are mirrorable by third parties with mirror instructions published; stale or unreachable entries age out via client-side health scoring, not central revocation
- Meta: Shard=— | Type=Feature | Size=S | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-CL-05 | Const=C-05, C-OA

### S-21-026 — Fleet Console scale hardening (100 k devices/tenant)
As a platform engineer I want the Fleet Console control plane proven at 100 k devices per tenant so that our largest prospective fleets onboard without a re-architecture.
- AC: device check-in fan-in is queue-buffered (ingest → durable queue → workers → partitioned Postgres) and a synthetic 100 k-device tenant holds NF-PERF-06 dashboard latency using materialized rollups; a 10× check-in burst degrades to increased check-in latency with zero dropped state; policy/OTA fan-out to the full synthetic tenant completes within its published SLO; capacity model (devices per Console node) is documented for self-hosters
- Meta: Shard=— | Type=Ops | Size=L | Prio=P1 | Status=DRAFT | SKU=★ | PRD=NF-SCALE-04 | Const=C-07, C-08

### S-21-027 — Hosted SMS/MMS reachability bridge — "SS-SP Number" (Rung 0)
As a subscriber I want a turnkey hosted dedicated phone number that bridges SMS/MMS between the normal phone network and my LXMF identity so that people without SS-SP can still reach me off-grid without my running any infrastructure — the reachability differentiator proven by Zoleo's dedicated-number offering (`06_COMPETITIVE_LANDSCAPE.md` §2.5) and benchmarked to JMP.chat (~$4.99/mo).
- AC: the hosted Rung-0 "SS-SP Number" service provisions a dedicated E.164 via the S-21-030 TPI and relays SMS/MMS↔LXMF bidirectionally (text-first, async store-and-forward so it reaches every bearer including LoRa) with delivery states surfaced on-device; it is a named Tier-2 paid line billed through S-21-022 entitlements; the service is strictly ADD-only and never-degrades — all mesh function is untouched without it and entitlement lapse only stops the bridge (verified per the F-CL-07 never-degrade rule); the hosted offering carries NO user-originated PSTN voice (text-first regulatory posture — SS-SP holds the A2P 10DLC brand/campaign and never becomes an interconnected-VoIP provider), with a prominent "not for emergency calling" notice (SOS remains bearer-flood per C-05); bridge traffic is encrypted mesh-side end-to-end to the subscriber identity, and the documented plaintext boundary at the PSTN edge plus retention policy pass wg-security review; per-country availability is gated by wg-legal sign-off
- Meta: Shard=— | Type=Feature | Size=L | Prio=P2 | Status=DRAFT | SKU=★ | PRD=F-TEL-02, F-TEL-03, F-CL-07 | Const=C-04, C-05, C-07, C-08
- Tasks: spec hosted Rung-0 service contract (E.164↔LXMF map, SMS/MMS relay, delivery states) against S-21-030 TPI · design entitlement + billing integration via S-21-022 and never-degrade enforcement · impl hosted bridge + on-device delivery-state surfacing · test bidirectional SMS/MMS relay, entitlement-lapse degrades-to-mesh-only, LoRa store-and-forward path · docs pricing/positioning, plaintext-boundary + retention note for wg-security, per-country availability matrix for wg-legal
- Deps: D-0025, `12_TELEPHONY_AND_UNIVERSAL_INTEROP_DOSSIER.md`, S-21-030, S-21-022

### S-21-028 — Cloud backend stack ADR (Rust axum vs Node TS)
As a cloud lead I want the backend stack decision (Rust axum vs Node/TypeScript, per `03_ARCHITECTURE.md` §stack table) made and recorded as an ADR before cloud implementation starts so that all EPIC-21 services build on one deliberate choice instead of drifting into a mixed stack.
- AC: decision matrix covers performance against NF-SCALE-03/NF-SCALE-04 targets, team velocity, crypto-library maturity for the relay path, self-host packaging weight (Helm/compose), and hiring pool; the ADR is recorded in the decisions log with wg-cloud sign-off and referenced from `03_ARCHITECTURE.md` (TBD removed); the choice is applied uniformly across Fleet Console, Relay, Provisioning, Plugin Registry, and Billing unless a per-service exception is justified in the same ADR
- Meta: Shard=— | Type=Spike | Size=XS | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-CL-01, F-CL-02 | Const=C-07

### S-21-029 — Published OpenAPI specifications for all cloud services
As an SDK developer I want versioned OpenAPI 3.1 specifications published for the Fleet Console, Provisioning, Plugin Registry, and Relay-admin APIs so that clients (S-20-015) are generated from a contract instead of reverse-engineered.
- AC: each service's OpenAPI document lives in-repo, is versioned per S-01-017 policy, and is published with the docs site; CI fails if a service's implemented routes drift from its spec (schema-diff check); auth schemes, tenancy headers, rate-limit and error envelopes are specified once and shared across services; the S-20-015 generated clients build from these specs unmodified
- Meta: Shard=— | Type=Feature | Size=S | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-CL-01 | Const=C-04

### S-21-030 — Telephony Provider Interface (TPI) contract + reference adapter
As a platform architect I want a carrier-neutral Telephony Provider Interface contract with one CI-maintained reference adapter so that any CPaaS or SIP trunk is pluggable behind one abstraction and every telephony tier builds on the same code path regardless of which tier ships.
- AC: the TPI contract defines `send_message`/`receive_message` (SMS, MMS), `originate_call`/`answer_call`/`bridge_media`, `provision_number`/`port_in`/`port_out`/`release_number`, `send_dtmf`, `delivery_receipt`, and `presence_hint`, plus the E.164↔LXMF-identity addressing model and `tel:` destination scheme; at least one reference adapter (`tpi-cpaas` against a CPaaS REST backend) is implemented and CI-maintained regardless of which tier is activated (capability-ready per D-0023/D-0024); a capability-flag/negotiation surface lets a deployment advertise which TPI operations a given adapter supports; the contract is documented with pre/post-conditions and error behaviour so BYO/self-host bridges (S-21-031, S-17-022) and voice bridges (S-14-021) target it unmodified
- Meta: Shard=— | Type=Feature | Size=L | Prio=P0 | Status=BLOCKED | SKU=★ | PRD=F-TEL-01 | Const=C-05, C-08
- Tasks: spec TPI operations, addressing model, `tel:` scheme, capability flags [done] · design carrier-neutral interface + adapter plug points [done] · impl `tpi-cpaas` reference adapter [blocked] · test contract conformance suite + adapter integration against a CPaaS sandbox [blocked] · docs interface reference (pre/post-conditions, errors) + adapter-authoring guide [done → `cloud/telephony-bridge/TPI_CONTRACT.md`]
- Deps: D-0025, `12_TELEPHONY_AND_UNIVERSAL_INTEROP_DOSSIER.md`, S-21-033, S-23-004; BLOCKED 2026-07-13: frozen contract delivered (`cloud/telephony-bridge/TPI_CONTRACT.md`); the `tpi-cpaas` reference-adapter impl+CI and CPaaS-sandbox conformance AC are unblocked by S-21-033 (telephony-bridge service scaffold + build) + S-23-004 (cloud CI, exists) + VA-29 (CPaaS test sandbox) — resume implementation once those are DONE.

### S-21-031 — BYO-number managed bridge (Rung 1)
As a sovereignty-minded subscriber I want to run the reachability bridge on SS-SP's managed cloud but against my own CPaaS/carrier account so that the phone number and carrier relationship stay mine and SS-SP never holds the 10DLC brand for my number.
- AC: a Rung-1 tier hosts the bridge compute while the operator supplies their own CPaaS/carrier credentials (paste API key) provisioned through the S-21-030 TPI; the phone number and carrier account belong to the operator, so A2P 10DLC brand/campaign and TCPA/AUP obligations attach to the operator, not SS-SP; billed as a lower "compute-only" Tier-2 line via S-21-022 entitlements (no DID/carrier fees on SS-SP's bill); ADD-only/never-degrade per F-CL-07; the same open bridge code path as Rungs 2–4, only the deployment location differs; credential handling and the plaintext boundary pass wg-security review
- Meta: Shard=— | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-TEL-04 | Const=C-04, C-07
- Tasks: spec Rung-1 BYO-credential model + operator-carrier liability boundary · design credential intake/storage + compute-only billing tier · impl BYO adapter wiring over S-21-030 TPI · test BYO-CPaaS relay, credential isolation, entitlement lapse · docs Rung-1 setup guide + operator-carries-10DLC posture
- Deps: D-0025, `12_TELEPHONY_AND_UNIVERSAL_INTEROP_DOSSIER.md`, S-21-030, S-21-022, S-21-027

### S-21-032 — Phone-number portability (port-in / no-fee port-out)
As a subscriber I want clean port-in of an existing number and no-fee port-out of an SS-SP number so that my number is never hostage and the device outlives the company.
- AC: port-in (bring an existing E.164 onto the bridge) and port-out (release the number to another carrier with no fee) are implemented via the S-21-030 TPI `port_in`/`port_out`/`release_number` operations; DID providers are chosen for clean no-fee port-out (VoIP.ms/Telnyx per the dossier); port-out never requires an active subscription and is documented as a user right (the concrete "device outlives the company" guarantee, `07_BUSINESS_MODEL_AND_OPEN_SOURCE.md` §10); the E.164↔LXMF map is cleanly migrated/torn down on port events; port workflows and per-country LNP constraints carry wg-legal sign-off
- Meta: Shard=— | Type=Feature | Size=M | Prio=P2 | Status=DRAFT | SKU=★ | PRD=F-TEL-07 | Const=C-07
- Tasks: spec port-in/port-out flows + no-fee-port-out guarantee · design LNP workflow over TPI + identity-map migration · impl port operations against reference adapter · test port-in, port-out-without-subscription, map teardown · docs number-portability user right + provider selection guidance
- Deps: D-0025, `12_TELEPHONY_AND_UNIVERSAL_INTEROP_DOSSIER.md`, S-21-030

### S-21-033 — Telephony bridge service scaffold + build
As a platform engineer I want a runnable telephony-bridge service scaffold with a build and container packaging so that TPI adapters have a deployable home and can be CI-maintained (the prerequisite that unblocks the S-21-030 reference adapter).
- AC: a telephony-bridge service skeleton exists under `cloud/telephony-bridge/` (service framework, config, health/lifecycle per the TPI `start`/`stop`/`health` surface) with a reproducible build and a vulnerability-scanned container image; it loads TPI adapters behind the S-21-030 contract and exposes the capability-negotiation surface; build/test/scan are wired into the cloud CI lane (S-23-004); self-host packaging (Helm/compose per S-21-016/S-21-017) can deploy it; no mesh/core dependency — ADD-only, never-degrade (F-CL-07)
- Meta: Shard=— | Type=Feature | Size=L | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-TEL-01, F-TEL-02 | Const=C-04, C-08
- Tasks: spec service boundaries + config + adapter-loading · design scaffold against TPI contract + cloud CI + self-host packaging · impl service skeleton + build + container image · test health/lifecycle + adapter-load + CI integration · docs deploy + adapter-registration guide
- Deps: D-0025, `12_TELEPHONY_AND_UNIVERSAL_INTEROP_DOSSIER.md`, S-21-030, S-23-004, S-21-016, S-21-017
