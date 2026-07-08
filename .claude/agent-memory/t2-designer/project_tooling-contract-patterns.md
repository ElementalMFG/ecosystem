<!-- SPDX-License-Identifier: Apache-2.0 -->
---
name: tooling-contract-patterns
description: T2 contract patterns for host tooling / CI stories (SBOM S-02-019 precedent)
metadata:
  type: project
---

Patterns proven in the S-02-019 SBOM contract (`docs/dev/contracts/S-02-019-sbom-contract.md`):

- Supply-chain "signing" stories stay T2 only if keyless (GitHub OIDC/Sigstore attest actions); state the no-private-keys boundary explicitly in the contract or builders drift into EPIC-08/09 T1 territory.
- Determinism recipe for generated artifacts: SOURCE_DATE_EPOCH-or-omit timestamps, uuid5 (never uuid4) serials derived from artifact hash, sort_keys + sorted arrays, byte-identical re-run as a CI `cmp` gate — satisfies C-OA reproducibility.
- `tools/*.py` contracts: stdlib-only, argparse exit 2 kept as-is, custom codes 3 (input) / 4 (invariant), atomic temp-file+rename so failures leave no partial output; expose pure functions for direct pytest import.
- Enumerate repo state at runtime (glob `ss_*` dirs) rather than freezing counts ("18 components") into the contract; tests assert count >= N and that additions appear.
- Split AC verifiability explicitly: push-CI-verifiable clauses vs tag-only clauses → story parks at IN_REVIEW until a real release tag.
- Repo GitHub slug is `ElementalMFG/ecosystem` (from .git/config), not the directory name — matters for purls/URLs.

**Why:** first Ops/CI-type T2 contract; these choices survived lint + review framing.
**How to apply:** reuse for any future host-tool + workflow contract (verifier, VEX, release automation). See also [[contract-patterns]].
