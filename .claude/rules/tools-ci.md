---
# SPDX-License-Identifier: Apache-2.0
paths:
  - "tools/**"
  - "ci/**"
  - ".github/**"
---

# Tooling & CI domain facts (verified 2026-07)

- Real tools (Python, `CONTRIBUTING.md` §7: ruff, type-hinted, tested):
  - `tools/lint-docs.py` — SPDX in first 3 lines of every git-tracked `.md`, relative links resolve, GitHub-slug anchors resolve, `## Table of contents` on root `0[0-8]_*.md`. **Only sees tracked/staged files — `git add` first.**
  - `tools/gen-stories-index.py [--check]` — regenerates/verifies `docs/portfolio/STORIES_INDEX.md`. Run from repo root.
  - `tools/claude/statusline.sh` — Claude Code statusline (`[model @ effort] ctx %`), doc 10 §2.2.
- `tools/{artwork,brand-guard,ota-signer,protocol-fuzzer,provisioning-line,sim}/` are empty scaffolds (epic-gated) — creating the first tool there is new work, not maintenance.
- Workflows (`.github/workflows/`, all required checks on `main`):
  - `firmware-build.yml` — builds `lite` in the pinned container on PRs touching `firmware/**` / `ci/containers/firmware/**` / workflows, and on push to main.
  - `docs-lint.yml` — runs `tools/lint-docs.py` on `**.md` changes.
  - `dco.yml` — every commit needs a `Signed-off-by:` trailer (`git commit -s`).
- **Toolchain pinning policy (RFC-0002)**: the single source of truth for the firmware image is the digest in `ci/containers/firmware/Dockerfile` (`espressif/idf` v5.3.5, sha256-pinned); `.devcontainer/devcontainer.json` and `firmware-build.yml` must carry the **same digest** — changing one means changing all three in one commit.
- Container build (canonical): `docker run --rm -v "$PWD":/workspace -w /workspace/firmware <pinned-image> idf.py -B build/lite -DSS_BOARD=lite build`. Container runs as root → `firmware/build/` artifacts may need `sudo rm -rf`.
