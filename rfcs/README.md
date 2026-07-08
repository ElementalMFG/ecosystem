<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# SS-SP RFCs

Formal Request-for-Comments documents for changes that require broader review.

## Index

| RFC | Title | Status |
|-----|-------|--------|
| [0001](0001-rfc-process-adoption.md) | Adoption of the RFC process | ACCEPTED |
| [0002](0002-toolchain-pinning-policy.md) | Toolchain pinning policy (digest-pinned build containers) | IMPLEMENTED |
| [0003](0003-compat-deprecation-policy.md) | Protocol compatibility & deprecation policy (cross-layer) | ACCEPTED |
| [0004](0004-scope-lock.md) | v1.0 scope lock — device identity, feature truth, compatibility guards | ACCEPTED |
| [DRAFT](DRAFT-foundation-transfer.md) | Foundation transfer plan (Phase-2 neutrality) | DRAFT |

## When to open an RFC

- Any change to the wire format (protocol layer).
- Any change to a public HAL / SDK API.
- Any new capability flag or cryptographic primitive.
- Any change to release cadence or LTS commitments.
- Any charter-level change (governance, licensing, trademark).

Small bug fixes, doc typos, board-specific tweaks that don't affect the shared HAL, and community-edition feature additions following existing patterns do **not** need an RFC.

## Process

See [`../06_GOVERNANCE.md`](../06_GOVERNANCE.md) §4.

Lifecycle:

```
IDEA → DRAFT → REVIEW → FINAL CALL → ACCEPTED/REJECTED → IMPLEMENTED → OBSOLETED
```

## Filename convention

`rfcs/<NNNN>-<short-slug>.md`, numbered sequentially at acceptance. Draft PRs use `rfcs/DRAFT-<slug>.md` and are renumbered on merge.

## Template

Copy `rfcs/TEMPLATE.md` to start.
