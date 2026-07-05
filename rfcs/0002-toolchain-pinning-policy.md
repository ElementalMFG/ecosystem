<!-- SPDX-License-Identifier: CC-BY-4.0 -->
- Title: Toolchain pinning policy — digest-pinned reproducible build containers
- Author(s): SS-SP program lead
- Shepherd: wg-firmware (bootstrap: program lead)
- Status: IMPLEMENTED
- Start Date: 2026-07-04
- Feature area (WG): wg-firmware / build & release
- Requires: 0001
- Supersedes: —

# Summary

All official firmware artifacts MUST be built inside a container image pinned by **digest** (not tag) to a specific ESP-IDF release. The current pin is `espressif/idf@sha256:114a9f8cde8bdc5a7e95809745b492663f7c5afe9c2821a4127ff133754fafd2` (ESP-IDF v5.3.5). Developers, CI, and release builds all use the same digest, so every party builds identical bits from identical sources. Upgrading the toolchain is a reviewed, single-commit change to one Dockerfile line plus the CI workflow, never an ambient drift.

# Motivation

Firmware supply-chain integrity (CRA disclosure duties, covenant #8 in `../governance/OPEN_ASSURANCE.md`, PRD ids NF-SEC-05/NF-SUS-03) requires that a given git SHA always produces the same binary. Tag-pinned images (`espressif/idf:v5.3.5`) can be silently re-pushed; digest pins cannot. The policy was proven necessary empirically: the first real containerized build of the scaffold exposed a component-shadowing defect (triage ledger T-22, `../docs/portfolio/07_FINAL_READINESS_TRIAGE.md`) that untested "compile-ready" claims had hidden. Story S-02-001 implements this policy.

# Detailed design

1. **Single source of truth.** The digest lives in `../ci/containers/firmware/Dockerfile` (`FROM espressif/idf@sha256:…`). The CI workflow `../.github/workflows/firmware-build.yml` repeats the same digest (and must be updated atomically with the Dockerfile); `../.devcontainer/devcontainer.json` builds from that Dockerfile and therefore inherits the pin without duplicating it.
2. **Build invocation.** Canonical build: run `idf.py -B build/<board> -DSS_BOARD=<board> build` inside the pinned container with the repo mounted at `/workspace` (see `../docs/dev/BUILDING.md`).
3. **Upgrade procedure.** (a) Open a PR changing the digest in both reference points (Dockerfile + CI workflow) atomically; (b) CI must produce a green build for every board in the matrix; (c) the PR description records old digest, new digest, and upstream release notes; (d) merge requires wg-firmware approval; (e) a decisions-log entry is appended if the major/minor IDF version changes.
4. **Reproducibility knobs.** Release builds set `SOURCE_DATE_EPOCH` from the commit timestamp; build paths are containerized and therefore stable; the firmware version resource (S-02-020) embeds git SHA + tag + build id so artifacts are traceable back to the exact toolchain digest via CI logs and the SBOM (S-02-019).
5. **Escape hatch.** Local un-containerized builds (`. $IDF_PATH/export.sh`) remain allowed for development, but no artifact built outside the pinned container may be signed or released.

# Wire-format changes

None.

# Security considerations

Digest pinning defeats tag-repushing and registry-side substitution for the toolchain layer (see `../05_SECURITY_MODEL.md`, supply-chain section). Residual risks: (a) upstream compromise *before* the pin is taken — mitigated by pinning to a version already vetted publicly for weeks; (b) Docker Hub availability — mitigated by the option to mirror the image to the project registry at VA-02; (c) the base image is not built from source by us — accepted at this phase, revisit at first LTS release.

# Interoperability impact

None on the wire. Positive for third-party firmware forks: they can reproduce official binaries bit-for-bit, which supports the certification program (`../docs/TRADEMARK.md`).

# Backward compatibility & migration

No prior releases exist; the policy applies from the first artifact. Future digest upgrades follow §Detailed design step 3, and old digests remain in git history so any historical release can be rebuilt.

# Rejected alternatives

- **Tag-pinned image (`espressif/idf:v5.3.5`)** — rejected: tags are mutable.
- **Vendoring the full toolchain into the repo** — rejected: multi-GB repo bloat, no security gain over a digest pin.
- **Nix/Bazel hermetic toolchain** — deferred: strongest reproducibility story, but ESP-IDF's CMake integration makes the container approach far cheaper today; may be revisited via a future RFC.
- **Unpinned "latest stable" with a version check script** — rejected: drift is exactly the failure mode we are excluding.

# Unresolved questions

- Whether to mirror the pinned image to a project-controlled registry (depends on VA-02 GitHub org / infra decisions).

# Prior art

Debian reproducible-builds project; Zephyr SDK version pinning; Meshtastic's PlatformIO version pinning; SLSA provenance guidance on pinned build environments.

# Implementation plan

Implemented by S-02-001 (`../.devcontainer/devcontainer.json`, `../ci/containers/firmware/Dockerfile`, `../.github/workflows/firmware-build.yml`) and verified by the first clean containerized build (T-22 closure). SBOM emission per artifact is S-02-019; `SOURCE_DATE_EPOCH` wiring lands with the release pipeline epic.
