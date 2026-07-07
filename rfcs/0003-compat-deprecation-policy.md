<!-- SPDX-License-Identifier: CC-BY-4.0 -->
- Title: Protocol compatibility & deprecation policy (cross-layer)
- Author(s): SS-SP program lead
- Shepherd: wg-protocol (bootstrap: program lead)
- Status: ACCEPTED
- Start Date: 2026-07-07
- Feature area (WG): wg-protocol / cross-layer governance
- Requires: 0001
- Supersedes: —

# Summary

One ratified rule for every versioned surface of the SS-SP ecosystem
answering "how long must old versions keep working": the minimum
supported-version window, the deprecation announcement channel, the lead
time expressed in release counts, and the sunset procedure. Teams cite this
RFC instead of inventing per-surface policies.

# Motivation

Seven independently versioned surfaces already exist or are planned
(SS-Link frames, RNS/LXMF profile, crypto suites, pairing schema, plugin
ABI, node-profile schema, cloud APIs). Without a single governed rule,
each version-negotiation story would restate its own window and the
guarantees in `06_GOVERNANCE.md` §4.4 (12-month wire-format deprecation,
LTS backports) would be interpreted differently per team. Downstream
integrators need one answer they can engineer against.

# Detailed design

## Definitions

- **Release**: a tagged minor or major release of the reference
  implementation (this repo). Patch releases do not count toward lead
  time.
- **Deprecation lead time**: the interval between the release in which a
  deprecation is announced and the earliest release that may remove the
  deprecated version, expressed as *whichever is longer* of a release
  count and a calendar floor.
- **Window**: the set of versions an implementation must accept.

## Per-surface policy

| Surface | Version signal | Minimum supported window | Deprecation lead time | Removal earliest in |
|---|---|---|---|---|
| SS-Link frames | frame `version` field | current + previous major | ≥ 2 minor releases and ≥ 12 months | next major release |
| RNS/LXMF profile | profile version (negotiated) | current + previous profile version | ≥ 2 minor releases and ≥ 12 months | next major release |
| Crypto suites | suite ID (handshake) | current + previous suite; deprecated suites: no new sessions, decrypt-only for stored data | ≥ 2 minor releases and ≥ 12 months (security exception below) | next major release |
| Pairing schema | schema version | current + previous major | ≥ 2 minor releases and ≥ 12 months | next major release |
| Plugin ABI | ABI major number | current + previous ABI major | ≥ 2 minor releases and ≥ 12 months | next major release |
| Node-profile schema | schema version | current + previous major | ≥ 2 minor releases and ≥ 12 months | next major release |
| Cloud APIs | URL major version (`/v1/`) | previous major serves ≥ 12 months after successor GA | announcement at successor GA | after window elapses |

Additive, backward-compatible changes (new optional fields, new suite
IDs, new endpoints) are always allowed without deprecation, per
`06_GOVERNANCE.md` §4.4 (unknown fields ignored).

## Announcement channel

A deprecation exists once, and only once, it is recorded in
[`docs/DEPRECATIONS.md`](../docs/DEPRECATIONS.md) — the single sunset
list required by `06_GOVERNANCE.md` §6.3 — and cross-linked from the
release notes of the announcing release. The entry names: the surface,
the version(s) deprecated, the announcing release, and the earliest
removal release computed from the lead time.

## Sunset procedure

1. An RFC (or, for non-wire surfaces, an accepted governance issue)
   declares the deprecation and cites the surface row above.
2. The entry is added to `docs/DEPRECATIONS.md` in the same PR.
3. Implementations emit a diagnosable warning when the deprecated
   version is used, where the surface makes that feasible.
4. Removal ships only in a major release after the lead time has fully
   elapsed, and the removal PR cites the `DEPRECATIONS.md` entry.

## Security exception

wg-security may shorten the crypto-suite lead time for a suite with a
practical break (accepted advisory required). The shortened schedule is
still announced via `docs/DEPRECATIONS.md`, and decrypt-only support for
stored data is preserved wherever it does not prolong the vulnerability.

## CI enforcement

A CI check (`.github/workflows/wire-format-policy.yml`) requires any PR
touching wire-format paths (`protocol/**`) to reference the policy
section it complies with by citing `RFC-0003` in the PR description.

# Wire-format changes

None. This RFC constrains the lifecycle of versioned surfaces; it does
not alter any encoding.

# Security considerations

The security exception above is the only path that shortens a window,
and it can only shorten — never skip — the announcement step. Keeping
deprecated crypto decrypt-only limits downgrade exposure while
preserving user data access.

# Interoperability impact

Downstream integrators can rely on: any version they ship against
remains accepted for at least one further major cycle and 12 months
after its deprecation is announced. LTS releases additionally receive
security backports for 5 years (`06_GOVERNANCE.md` §4.4).

# Backward compatibility & migration

This policy binds surfaces from their first published version; no
existing published surface predates it, so no migration is needed.

# Rejected alternatives

- **Per-surface policies owned by each WG** — rejected: exactly the
  per-team guessing this RFC exists to eliminate.
- **Calendar-only lead time** — rejected: a quiet year with no releases
  would let a deprecation expire without any release in which users
  could have seen the warning; the release-count floor prevents that.
- **N-2 support windows** — rejected as a blanket rule: doubles the
  negotiation test matrix for marginal benefit on embedded targets;
  individual surfaces may adopt wider windows via their own RFCs.

# Unresolved questions

None blocking. Per-surface conformance-test requirements are defined by
each surface's own RFC (e.g. S-10-008 frame format).

# Prior art

Rust editions (trailing-edition support), Kubernetes API deprecation
policy (release-count-based), Debian LTS windows,
`06_GOVERNANCE.md` §4.4/§6.3 which this RFC operationalizes.

# Implementation plan

Implemented with this RFC: `docs/DEPRECATIONS.md` scaffold, the
wire-format CI check, citations from version-negotiation stories
(S-06-016, S-12-014, S-10-008) and the SDK policy story (S-20-017), and
the link from `02_PROTOCOL_STACK.md` §11. Acceptance recorded as D-0018.
