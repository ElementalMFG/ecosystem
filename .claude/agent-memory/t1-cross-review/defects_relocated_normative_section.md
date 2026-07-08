<!-- SPDX-License-Identifier: Apache-2.0 -->
---
name: defects-relocated-normative-section
description: Moving a "normative home" doc/policy section between headers leaves stale external cross-references that no gate catches
metadata:
  type: feedback
---

When a diff relocates a normative section (e.g. "WATCHDOG POLICY") from one header to another and marks the new location "normative home", grep the whole repo for the old pointer string — external citations in .c/.cpp/sdkconfig/docs go stale silently.

**Why:** S-03-039 moved the WATCHDOG POLICY block from `ss_tasks.h` to `ss_hal_watchdog.h`. The author updated `ss_tasks.h`'s own wording but left two dangling pointers: `firmware/sdkconfig.defaults:37` ("see ss_tasks.h WATCHDOG POLICY") and `firmware/main/ss_memwatch.cpp:180` (a policy-mandated TWDT-EXEMPT comment citing "ss_tasks.h WATCHDOG POLICY"). lint-docs / contract-audit / gen-stories all pass — prose comment pointers are invisible to every gate.

**How to apply:** On any "contract of record moved here" / "normative home is this header" diff, `grep -rn "<OLD SECTION NAME>"` across firmware+docs+tools and flag every hit that still names the old file. Same-model authors reliably miss the cross-file citations while diligently fixing the header they are editing.
