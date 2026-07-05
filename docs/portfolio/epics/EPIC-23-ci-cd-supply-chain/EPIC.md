<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# EPIC-23 — CI/CD & Supply Chain

**Primary WG:** wg-ops · **Contributing:** wg-security, wg-legal
**Priority:** P0 · **SKU:** ★ · **Milestone:** M0

## Outcome
Every commit builds reproducibly, every artifact is signed and has SLSA-3 provenance + SBOM, secrets never leave a HSM, PR gates enforce DCO+CLA+lint+coverage+mutation+wire-conformance, container images are pinned by digest, third-party dependencies are pinned and audited.

## Constitution
C-06 `06_GOVERNANCE.md` §CI; C-OA `governance/OPEN_ASSURANCE.md` §reproducible builds; C-05 `05_SECURITY_MODEL.md` §release signing.

## Dependencies
EPIC-01, EPIC-22.

## Shards
- **S-23.A GitHub Actions matrix** — firmware × board × arch × cloud × apps.
- **S-23.B Reproducible container images (digest-pinned).**
- **S-23.C SLSA-3 provenance emission.**
- **S-23.D SBOM (CycloneDX) per artifact.**
- **S-23.E Signed release artifacts (cosign + RelKey_B).**
- **S-23.F PR gate: DCO, CLA, lint, coverage, mutation, conformance.**
- **S-23.G Dependency-pin registry (lockfiles).**
- **S-23.H Supply-chain vuln scanning (osv-scanner, Trivy).**
- **S-23.I Release automation (semver, changelog gen).**

## Exit criteria
1. Any commit on `main` yields byte-identical artifact under `ss-verify`.
2. Every release ships SBOM + SLSA provenance signed by RelKey_B.
3. PR merge blocked on any gate failure.
4. Vuln scan runs weekly and files issues automatically.

## Risks
| # | Risk | Mitigation |
|---|------|-----------|
| R23-01 | CI compromise | Least-priv tokens + HSM signing |
| R23-02 | Dep pin drift | Renovate PRs + audit |
| R23-03 | Signing outage | Backup RelKey_B on second HSM |
