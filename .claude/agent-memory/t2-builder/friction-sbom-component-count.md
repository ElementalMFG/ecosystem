<!-- SPDX-License-Identifier: Apache-2.0 -->
---
name: friction-sbom-component-count
description: S-02-019 SBOM contract assumed 18 ss_* components with CMakeLists; repo has only 2 — stale premise, escalated
metadata:
  type: project
---

S-02-019 SBOM contract (`docs/dev/contracts/S-02-019-sbom-contract.md`, FROZEN
2026-07-07) §3.4 states "the list is currently 18" first-party components and
mandatory host-test §10.3 asserts `count >= 18`. Reality (2026-07): only
`firmware/components/ss_hal` and `ss_log` carry a `CMakeLists.txt`; the other 15
`ss_*` dirs are empty skeletons (alpha/omega are skeletons too — only `lite`
builds today). Enumeration rule "ss_* dir WITH CMakeLists.txt" yields 2, not 18.

**Why:** contract was drafted against a planned/future tree; the "18" is a stale
factual premise, not an implementable spec. `count >= 18` cannot pass now.

**RESOLVED 2026-07-07:** designer amended the contract (§3.4 dynamic
enumeration, §10.3 tree-equality assertion, no hardcoded count). Implemented in
`tools/gen-sbom.py` + `tools/tests/test_gen_sbom.py`; the completeness test
computes the expected set from the tree at runtime, so it needs no edit as
`ss_*` dirs gain CMakeLists.

**How to apply:** enumeration = dirs matching `firmware/components/ss_*` WITH a
`CMakeLists.txt`. Never hardcode the count. Also confirmed: firmware-build.yml
already exports SOURCE_DATE_EPOCH (step "Deterministic build metadata", `git
log -1 --format=%ct`) — consume it, don't re-add.
