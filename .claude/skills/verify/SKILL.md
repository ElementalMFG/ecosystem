---
# SPDX-License-Identifier: Apache-2.0
name: verify
description: Run all applicable SS-SP repo gates (docs lint, stories index, firmware build, DCO check) and report a pass/fail table. Use before any commit or PR.
allowed-tools: Bash, Read, Grep, Glob
# Gate-running is mechanical (doc 11 §3): auto-drop to low while active.
effort: low
---

# Repo verification gates

Run from the repo root, in this order. Report a pass/fail table at the end; fix nothing beyond mechanical staging unless asked.

1. **Stage first**: `git add` the intended files — `tools/lint-docs.py` cannot see untracked files.
2. **Docs lint**: `python3 tools/lint-docs.py` (SPDX first-3-lines, links, anchors, constitution TOCs).
3. **Stories index + allocation map**: `python3 tools/gen-stories-index.py --check` and `python3 tools/allocation.py --check` (both generated; the map carries story STATUS, so any status flip needs `allocation.py --generate` first — CI enforces this in docs-lint). Cheap — always run both.
3a. **Other repo gates** (cheap, always): `python3 tools/covenant-check.py`, `python3 tools/task-policy-check.py`, `python3 tools/board-parity.py`, `python3 tools/contract-audit.py`.
4. **Firmware build** (only if `firmware/**` or `ci/containers/firmware/**` changed): `make lite` with local ESP-IDF, else the pinned-container command from `docs/dev/BUILDING.md`. On WSL2, Docker may need `sudo -n service docker start` first.
5. **DCO**: `git log --format='%(trailers:key=Signed-off-by)' <range>` — every commit to be pushed needs a sign-off (`git commit -s`).
6. **Report**: table of gate | result | evidence line. Any FAIL blocks commit/push until resolved.
