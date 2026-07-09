<!-- SPDX-License-Identifier: Apache-2.0 -->
---
name: contract-patterns
description: Reusable T2 contract-design patterns proven in firmware/main (ss_recovery, ss_nvs)
metadata:
  type: project
---

Proven T2 contract patterns in this repo:
- Single public header holds the WHOLE contract: rich `// CONTRACT` prose block (scheme + rationale + citations) above `#pragma once`; then "Pure decision core (no IDF)" section, then "Target glue (implemented in X.cpp; not host-buildable)" section. Includes limited to stdbool/stdint (use uint32_t lengths, not size_t).
- House policy for risky transitions is refuse-with-reason, never force (ss_recovery_rollback_eval, ss_nvs DOWNGRADE_REFUSE). Reserve `__` name prefix + validator rejection to make key collisions structural, not conventional.
- Persistence-adjacency guardrail: settings APIs must explicitly scope OUT keys/ratchet state/anti-rollback counter (doc 05 §3/§6.4/§11) — cite it in the header so builders don't drift into T1.
- Fixed static registries (no heap) with a documented SS_*_MAX capacity; hooks must be idempotent so failures retry safely.
- Host+target dual-run conformance (S-03-022): pure core never calls the HAL — an env vtable (reset/exec/emit) executes steps; freeze the observable state-word encodings AND the diff-line grammar byte-for-byte in the header, and mirror esp_err numerics as SS_*_RET_ macros so the core stays stdint-only.
- Never hardcode repo-state counts ("currently N components") in contracts or test thresholds — verify against the tree first, and prefer dynamic assertions (test computes expected set from tree at runtime). An S-02-019 amendment was forced by an unverified "18 components" premise (reality: 2 with CMakeLists.txt).

**Why:** builders implement verbatim; anything not frozen in the header gets improvised.
**How to apply:** mirror ss_recovery.h / ss_nvs.h when designing any new firmware/main contract.
