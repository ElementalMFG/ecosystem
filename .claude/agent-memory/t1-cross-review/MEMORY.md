<!-- SPDX-License-Identifier: Apache-2.0 -->
# T1 cross-review memory index

Consult these at the start of every review; append one line per durable finding.

## Defect classes (recurring)
- [Coredump / panic-guard gotchas](defects_coredump_panic.md) — espcoredump format default, reset-reason enum gaps, DRAM-capture pinning, app-level breaker scope.

## Per-component / repo gotchas
- [Repo doc & config gotchas](gotchas_repo.md) — 05_SECURITY_MODEL.md is cited everywhere but does not exist yet; sdkconfig self-documents defaults selectively.

## Workflow
- [Cross-review workflow](workflow_cross_review.md) — host tests are runnable/expected green; hardware AC parts park at IN_REVIEW by design.
