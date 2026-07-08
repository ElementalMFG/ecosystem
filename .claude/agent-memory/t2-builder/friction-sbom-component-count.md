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

**How to apply:** the generator's enumeration logic (dirs with CMakeLists) is
correct as written — it will grow with the tree. The blocker is only the
hardcoded test threshold + the "18" prose. Resolution needs the designer:
either drop the threshold to reality (>= 2) or, better, make the test compute
the expected component set dynamically from the tree (matches the contract's own
"NOT a hardcoded list... must track the tree" intent). Don't improvise the
threshold — it's a mandatory test-case value = contract-level decision.

Also confirmed while investigating: firmware-build.yml DOES already export
SOURCE_DATE_EPOCH (step "Deterministic build metadata", from `git log -1
--format=%ct`), so the contract §7 claim holds — no need to add it.
