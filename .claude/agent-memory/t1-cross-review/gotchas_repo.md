<!-- SPDX-License-Identifier: Apache-2.0 -->
---
name: gotchas-repo
description: Repo-wide doc/config conditions that recur across T1 reviews
metadata:
  type: project
---

- **`05_SECURITY_MODEL.md` NOW EXISTS at repo root** (32 KB, present as of 2026-07; a prior review flagged it missing — that is stale). Verified sections that T1 contracts cite: §8.3 Duress PIN, §11.1 On-device storage (encryption-at-rest + ratchet overwrite-in-place), §12 Privacy Posture (line ~475: "No default telemetry… opt-in… crash reports"). **How to apply:** citations to §8.3/§11.1/§12 resolve and are topically correct — do NOT flag them as dangling. §11.1 is about at-rest encryption, so a citation for "zeroize live buffers after use" is a slightly loose (still defensible) match, not a defect.

- **sdkconfig.defaults self-documents defaults selectively.** The file deliberately pins some IDF defaults (e.g. stack-overflow canary) "so the baseline is self-documenting," but leaves other security-relevant defaults implicit (e.g. COREDUMP_CAPTURE_DRAM). Treat any *unpinned* security-relevant Kconfig as an editability hazard, not a settled decision.
