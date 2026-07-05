<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# 05 — Infrastructure, Scale & Revenue Architecture

Status: DRAFT for wg-arch + wg-biz review
Binding inputs: `01_BRIEF.md`, `02_PRD.md` (§2.9 F-CL, §3.10 NF-SCALE), root
`07_BUSINESS_MODEL_AND_OPEN_SOURCE.md` (§3 revenue streams, §4 anti-rug-pull),
root `08_UNIVERSAL_CONNECTIVITY.md` (§9 decentralization guarantees).
This document is the **decision record** for the question: *can SS-SP run as a
fully decentralized ecosystem, or does it require centralized/cloud
infrastructure — and how do we generate revenue either way?*

---

## 1. The determination (decision record)

**Decision: hybrid — decentralized core is mandatory and sufficient;
centralized cloud is an optional accelerant and the revenue engine.**

### 1.1 What MUST be decentralized (non-negotiable, already codified)

Per 08 §9 and the Open Assurance covenant, the following work with **zero**
SS-SP-operated services, forever:

| Function | Decentralized mechanism | Requirement |
|---|---|---|
| Identity | Ed25519 keys generated on-device, never escrowed | NF-SEC, J5 |
| Messaging | LXMF over RNS, peer-to-peer + store-and-forward | F-MSG, NF-REL-01 |
| Routing | Reticulum transport nodes — any user can run one | F-NODE-05 |
| Voice | ss_link bearer-direct, codec on-device | F-VOICE |
| SOS | floods ALL bearers, no server in the path | F-SOS, C-05 |
| Pairing | local bearers (BLE/QR/NFC), bridge optional | F-CL-06 (opt-in) |
| Updates | community OTA channel, mirrorable, reproducible builds | F-CL-05, NF-SUS-03 |
| Gateways | Home Gateway Mode on any owner device or router | 08 §7, F-NODE-05 |

**Proof obligation:** S-22-023 (30-day cloudless soak) plus S-22-024
(mega-scale simulation, NF-SCALE-01). The claim is not marketing until both
pass.

### 1.2 What is centralized (optional, commercial, BSL 1.1)

| Service | Why centralization helps | Revenue stream (07 §3) | Self-host escape hatch |
|---|---|---|---|
| Fleet Console | enterprises want one pane of glass, SLAs, audit | subscriptions | Helm/compose (S-21-014/015) |
| Relay Federation | guaranteed-capacity geo-anycast transport beats best-effort volunteers for paying fleets | subscriptions + reservations | anyone can run RNS transport free |
| Provisioning | HSM ceremony, tenant onboarding at 1 000 dev/day | bundled with hardware/enterprise | bench programmer (F-MFG-02) |
| Plugin Registry | signed review pipeline, revocation, marketplace | marketplace cut | side-load signed plugins offline |
| Billing & entitlements | subscriptions, payouts, cert fees need commerce plumbing | ALL paid streams | N/A — entitlements only ADD, never gate (07 §4) |
| Bootstrap directory | first-contact hint list for internet-attached nodes | none (goodwill/funnel) | signed, mirrorable, overridable, ignorable |

### 1.3 Why hybrid and not the alternatives

- **Pure-decentralized rejected as a business:** it forfeits every recurring
  revenue stream in 07 §3 except hardware margin; enterprise buyers (J4)
  demand SLAs, tenancy, and support contracts that require operated services.
- **Cloud-required rejected as an architecture:** it violates the Open
  Assurance covenant, C-05 (SOS must never depend on infrastructure we could
  fail to operate), NF-REL-01 (indefinite offline), and journey J5 (hostile
  telecom). It is also the single biggest trust differentiator vs. Garmin
  inReach/Zoleo, which brick without their subscription backends.
- **Hybrid is self-reinforcing:** the free decentralized layer grows the node
  population that makes the mesh valuable; the paid layer sells convenience,
  capacity, and accountability on top — never access. "The user's device
  outlives the company" (07 §10) stays literally true.

---

## 2. Infrastructure tier model

Three concentric tiers. Everything in an outer tier keeps working if all
inner tiers vanish.

```
┌──────────────────────────────────────────────────────────────┐
│ TIER 0 — SOVEREIGN MESH (no infrastructure at all)           │
│   devices ↔ devices over LoRa/HaLow/BLE/Wi-Fi-Direct;        │
│   app-nodes (phones/PCs) as full RNS peers; SOS; pairing     │
├──────────────────────────────────────────────────────────────┤
│ TIER 1 — COMMUNITY INFRASTRUCTURE (anyone-operated)          │
│   user RNS transport nodes; Home Gateways; OpenWrt routers;  │
│   LXMF propagation nodes; OTA mirrors; bootstrap mirrors;    │
│   volunteer relays. Funded by goodwill; enabled by us.       │
├──────────────────────────────────────────────────────────────┤
│ TIER 2 — COMMERCIAL CLOUD (SS-SP-operated OR self-hosted)    │
│   Fleet Console · Provisioning · Plugin Registry/Marketplace │
│   Relay Federation (guaranteed capacity) · Billing &         │
│   entitlements · signed bootstrap directory · support/SLA    │
└──────────────────────────────────────────────────────────────┘
```

Tier-boundary rules:
1. No Tier-0/1 code path may hard-depend on a Tier-2 endpoint (CI-enforced by
   the S-22-023 blackhole soak).
2. Every Tier-2 service ships a self-host artifact (Helm + compose) and
   converts to Apache-2.0 at its BSL change date.
3. Tier-1 operators are first-class: the same relay/gateway/propagation
   software we run commercially is what the community runs, minus the SLA.

---

## 3. Scale engineering — how millions actually work

"Millions of users" fails at four specific choke points. Each has an
engineering answer, a requirement ID, and a story.

### 3.1 Announce & path-table load on constrained bearers (NF-SCALE-02)

Reticulum destinations announce themselves; announces propagate through
transport nodes. On a 500 kbps HaLow link this is trivia; on LoRa at ~1 kbps
with 1–10 % regulatory duty cycle it is **the** bottleneck. Naive math: 1 000
regional nodes × 1 announce/10 min × ~50 B ≈ steady-state load that alone can
eat a duty-cycle budget before any user traffic flows.

Mitigations (S-11-023):
- **Transport-tier profile:** constrained-bearer nodes default to non-transport
  (leaf) mode; only mains-powered/gateway nodes carry regional path state.
- **Announce ingress/egress budgets** per bearer with priority eviction
  (paths in active use are never evicted for fresh announces).
- **Path-table caps** sized to Lite/Alpha RAM with LRU + pin-active policy.
- **Cross-bearer announce damping:** an announce learned on a fast bearer is
  re-emitted on LoRa only when the destination is plausibly reachable there.
- Builds on the storm-suppression work already in EPIC-11 (announce
  rate-limiting story) — this story adds *budgets and tiers*, not duplicate
  suppression.

### 3.2 Relay & transport capacity (NF-SCALE-03)

RNS transport is connectionless with per-link state only at endpoints and
transports on the path; horizontal scale-out is architecturally natural but
must be **proven**, not assumed. Targets: ≥ 10 k concurrent links per
commodity relay; add-node = add-capacity; geo-anycast steering; zero shared
mutable state between relays. Federation-operator program (S-21-024) turns
community relays into a capacity multiplier: certified third-party operators
peer with the commercial federation under quota contracts — the network
outgrows what we alone could fund.

### 3.3 Message persistence & spam economics (NF-SCALE-06)

LXMF propagation nodes sync opportunistically. At millions of users,
unbounded peering = storage-flood attacks and sync storms. S-12-020 adds:
- per-peer storage/bandwidth quotas with negotiated sync windows,
- **LXMF stamps** (sender proof-of-work) so unsolicited bulk delivery costs
  the spammer, not the network — aligned with upstream LXMF stamp design so
  interop with Sideband/Nomad Network is preserved,
- retention tiers (paid fleets can buy longer retention on commercial
  propagation nodes — a natural F-CL-04/07 upsell that never degrades the
  community default).

### 3.4 Release-day OTA & artifact distribution (NF-SCALE-05)

1 M devices × ~6 MB slot image = ~6 TB per release wave. Answer (S-09-021):
CDN in front of the community channel, binary-delta updates (cuts the wave
~10×), staged rollout percentages already specified in EPIC-09, and
first-class community mirrors with signed manifests so no single distribution
point exists (Open Assurance). Mirror list itself is signed + mirrorable.

### 3.5 Control-plane scale (NF-SCALE-04)

Fleet Console at 100 k devices/tenant: check-in fan-in is queue-buffered
(ingest → durable queue → workers → Postgres partitioned per tenant),
dashboards read from materialized rollups (holding NF-PERF-06), policy pushes
fan out through the relay layer, and 10× burst degrades to increased check-in
latency — never dropped state (S-21-026).

### 3.6 Sequenced scale gates

| Gate | Population | Validation |
|---|---|---|
| G1 | 1 k regional (LoRa-heavy) | S-22 sim + field pilot; NF-SCALE-02 airtime ≤ 5 % |
| G2 | 100 k global | relay load test (NF-SCALE-03); Console tenant test (NF-SCALE-04) |
| G3 | 1 M mixed | S-22-024 mega-sim (NF-SCALE-01); OTA wave drill (NF-SCALE-05) |
| G4 | 10 M+ | re-run G3 suite per order of magnitude; publish results |

No scale claim ships in marketing until its gate passes. Simulation results
are published (C-00) so the community can reproduce them.

---

## 4. Revenue architecture — streams → components → work

All eight streams from 07 §3, mapped to the components and stories that
implement them. Constraint on every row: **entitlements ADD, never gate**
(07 §4); core communication is never behind any of these.

| # | Stream (07 §3) | Component(s) | Epic/stories | Notes |
|---|---|---|---|---|
| 1 | Hardware margin | devices + accessories | EPIC-02..05, 24 | primary at launch; NF-COST-01/02 protect margin |
| 2 | Cloud subscriptions | Fleet Console, Relay Federation, retention tiers | EPIC-21 S-21-001..013, **S-21-022, S-21-024, S-21-026** | NF-COST-03 margin ≥ 40 % @10 k enrolled |
| 3 | Enterprise support/SLA | support org + runbooks | S-21-020, EPIC-24 | attach to stream 2 contracts |
| 4 | Premium plugins (marketplace) | Plugin Registry + commerce | EPIC-18, S-21-011..013, **S-21-023** | marketplace cut; WASM-sandboxed; ADD-only |
| 5 | Certification/trademark fees | SS-SP-Certified program | S-24-025, **S-21-022** (fee invoicing) | third-party hardware pays for the mark, not the protocol |
| 6 | OEM / white-label | firmware + provisioning licensing | EPIC-07, F-MFG-02, EPIC-24 | per-unit or flat licence via billing service |
| 7 | Professional services | deployment/integration consulting | runbooks + SDKs (EPIC-20) | high-touch, low-tooling |
| 8 | Grants / sponsorships | open-core public-interest work | EPIC-01 governance | funds Tier-0/1 development |

**The one missing load-bearing component was commerce plumbing.** F-CL-07 +
S-21-022 (billing & entitlements) and S-21-023 (marketplace payouts) close
it. Design constraints:
- Entitlement tokens are signed capability grants cached on device/tenant;
  expiry **never disables** anything already functioning offline (NF-REL-01)
  — lapsed subscription = no *new* premium provisioning, zero degradation of
  the existing fleet's core function.
- Billing is its own bounded service (PCI scope isolation; likely
  Stripe-class PSP behind an adapter so self-hosters can swap providers).
- Marketplace payouts are transparent: published fee split, developer
  dashboard, open payout ledger format.

Funnel logic: Tier 0/1 free forever → grows mesh population → enterprises and
power users arrive where the mesh already works → they buy Tier 2 convenience
(streams 2–5) → revenue funds Tier-0/1 engineering. Decentralization is not
in tension with revenue; it is the customer-acquisition engine.

---

## 5. Build-out phases (sequence, not calendar)

- **P1 — Prove the core:** Lite bring-up (firmware §), ss_link + RNS/LXMF,
  cloudless soak (S-22-023). Revenue: pre-orders/hardware.
- **P2 — Minimum commercial cloud:** Fleet Console MVP, provisioning,
  community OTA channel + pilot customer (EPIC-21 exit), billing MVP
  (S-21-022). Revenue: first subscriptions.
- **P3 — Ecosystem opening:** SDKs, plugin registry + marketplace commerce
  (S-21-023), companion app nodes, OpenWrt package (S-19-028), bootstrap
  directory (S-21-025). Revenue: marketplace + certification.
- **P4 — Scale hardening:** relay federation + operator program (S-21-024),
  announce budgets (S-11-023), propagation quotas/stamps (S-12-020), OTA CDN
  (S-09-021), Console scale (S-21-026), gates G2–G3 (S-22-024). Revenue:
  reservations, retention tiers, OEM.

---

## 6. Traceability

New requirement IDs introduced by this document: **F-CL-07**,
**NF-SCALE-01..06** (PRD §2.9, §3.10). New stories: S-09-021, S-11-023,
S-12-020, S-19-028, S-21-022..026, S-22-024. All other rows reference
pre-existing artifacts. Amendments to this document follow the RFC process
(EPIC-01); §1 (the determination) additionally requires wg-biz + wg-legal
sign-off to change, because the Open Assurance covenant depends on it.
