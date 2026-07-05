<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# EPIC-23 — Stories

Format per `../../00_METHODOLOGY.md` §2.7. Meta lines are machine-parsed.

### S-23-001 — GitHub Actions matrix skeleton
As a release manager I want a GitHub Actions matrix skeleton spanning firmware × board × arch × cloud × apps so that every product surface builds from one consistent pipeline.
- AC: matrix defines all five axes using reusable workflows; path filters run only the legs a PR actually touches; required status checks are registered on `main`
- Meta: Shard=A | Type=Ops | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-06, C-OA

### S-23-002 — Firmware build job per board
As a release manager I want per-board firmware build jobs so that every supported board compiles on every PR.
- AC: each supported board builds inside the pinned reproducible builder container; artifacts upload with checksums; build cache is keyed by lockfile digest so cache poisoning across dependency changes is impossible
- Meta: Shard=A | Type=Ops | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-06, C-OA

### S-23-003 — Companion app build matrix (iOS/Android/desktop/web)
As a release manager I want the companion apps built for iOS, Android, desktop, and web in CI so that app regressions block merge, not release day.
- AC: all app targets build on PR (aligned with EPIC-19 S-19-016); signing uses least-privilege ephemeral credentials, never long-lived secrets in workflow files; failure of any platform blocks merge
- Meta: Shard=A | Type=Ops | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-CA-01, F-CA-02, F-CA-03, F-CA-04 | Const=C-06, C-OA

### S-23-004 — Cloud services build & test
As a release manager I want cloud services built and tested in CI so that backend changes are gated identically to firmware.
- AC: cloud services build with unit and integration tests against ephemeral dependencies; container images are built and vulnerability-scanned before publish; results gate merge as required checks
- Meta: Shard=A | Type=Ops | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-CL-01 | Const=C-06, C-OA

### S-23-005 — Reproducible builder container (digest-pinned)
As a release manager I want a digest-pinned reproducible builder container so that two independent builds of the same commit produce byte-identical artifacts.
- AC: builder image is referenced only by digest, never floating tags; two independent builds of the same commit yield byte-identical firmware (supports epic exit criterion 1); the image's own build procedure is documented and reproducible
- Meta: Shard=B | Type=Ops | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=NF-SUS-03 | Const=C-06, C-OA

### S-23-006 — SLSA-3 provenance action
As a release manager I want SLSA-3 provenance emitted for every release artifact so that consumers can verify what built it, from what, and where.
- AC: every release artifact carries a SLSA-3 provenance attestation; provenance verifies with standard public tooling; attestation records source digest and builder-image digest
- Meta: Shard=C | Type=Ops | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-06, C-OA

### S-23-007 — CycloneDX SBOM emission
As a compliance officer I want a CycloneDX SBOM emitted per artifact so that the published-SBOM and CRA obligations are met automatically.
- AC: SBOM in CycloneDX format is generated for every build artifact; SBOM publishes with each release per NF-SEC-05 and NF-REG-03; SBOM completeness is checked against lockfiles in CI
- Meta: Shard=D | Type=Ops | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=NF-SEC-05, NF-REG-03 | Const=C-06, C-OA

### S-23-008 — cosign signing pipeline
As a release manager I want release artifacts signed via cosign backed by RelKey_B in an HSM so that only HSM-held keys can produce shippable artifacts.
- AC: signing executes against the HSM-held RelKey_B with the key non-exportable; a pipeline verification step rejects unsigned or wrong-key artifacts; public signature-verification instructions are published with each release
- Meta: Shard=E | Type=Ops | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-SEC-08 | Const=C-06, C-OA, C-05

### S-23-009 — DCO check bot
As a community manager I want a DCO check on every PR so that contribution provenance is legally clean.
- AC: PRs missing DCO sign-off are blocked as a required check; the bot comment explains how to remediate (amend/sign-off); enforcement matches the governance contribution rules
- Meta: Shard=F | Type=Ops | Size=S | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-06

### S-23-010 — CLA-assistant bot
As legal counsel I want a CLA-assistant flow for first-time contributors so that relicensing rights (including the BSL/Apache-2.0 strategy) stay intact.
- AC: first-time contributors are prompted and cannot merge until the CLA is signed; CLA signature state is stored durably and auditable; the CLA text has recorded wg-legal approval
- Meta: Shard=F | Type=Ops | Size=S | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-06, C-04

### S-23-011 — Lint job (clang-format, ruff, prettier, cargo-fmt, golangci-lint)
As a release manager I want per-language lint jobs so that style debates are settled by machines.
- AC: clang-format, ruff, prettier, cargo-fmt, and golangci-lint run against their respective paths; any violation blocks merge; lint configuration is centralised at repo root, not per-package
- Meta: Shard=F | Type=Ops | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-06

### S-23-012 — Coverage floor gate
As a release manager I want a coverage floor enforced as a PR gate so that test debt cannot accumulate silently.
- AC: PRs below the configured coverage floor are blocked, consuming EPIC-22 kcov output; floor changes require review, not drive-by edits; gate result is a visible required check
- Meta: Shard=F | Type=Ops | Size=S | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-06

### S-23-013 — Mutation floor gate (security-critical only)
As a release manager I want a mutation-score gate scoped to security-critical modules so that crypto and boot code cannot pass on shallow tests.
- AC: mutation gate applies only to the security-critical scope list maintained with wg-security; failing the mutation floor blocks merge; scope-list changes require wg-security review
- Meta: Shard=F | Type=Ops | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-06, C-05

### S-23-014 — Wire conformance gate (testkit)
As a release manager I want the wire testkit run as a PR gate on protocol-touching changes so that wire compatibility cannot break without an RFC.
- AC: wire testkit vectors (EPIC-20 S-20-011) run automatically on PRs touching protocol paths; a failing vector blocks merge; wire-format changes require a linked ACCEPTED RFC per methodology DoD
- Meta: Shard=F | Type=Ops | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-06, C-02

### S-23-015 — Renovate for dep pins
As an SRE I want Renovate managing pinned dependencies across all lockfiles so that pins stay current without drifting unaudited (risk R23-02).
- AC: Renovate opens PRs for every lockfile ecosystem in the monorepo; automerge is allowed only for patch-level updates that pass full CI; a periodic pin-drift audit report is generated
- Meta: Shard=G | Type=Ops | Size=S | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-06, C-OA

### S-23-016 — osv-scanner + Trivy weekly job
As an SRE I want weekly osv-scanner and Trivy scans of dependencies and container images so that known vulnerabilities are found and filed automatically (epic exit criterion 4).
- AC: weekly scheduled scan covers all lockfiles and published container images; findings auto-file issues with severity labels; critical findings page wg-security per the security policy
- Meta: Shard=H | Type=Ops | Size=S | Prio=P0 | Status=DRAFT | SKU=★ | PRD=NF-SEC-05 | Const=C-06, C-05

### S-23-017 — Release-please for changelog + tag
As a release manager I want release-please automating semver tags and changelogs so that releases are consistent and the DoD changelog requirement is automatic.
- AC: releases are tagged with correct semver derived from conventional commits; changelog is generated and included in the release PR; the release flow is documented end-to-end
- Meta: Shard=I | Type=Ops | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-06

### S-23-018 — Backup HSM for RelKey_B
As an SRE I want RelKey_B backed up on a second HSM so that a signing outage cannot stall releases (risk R23-03).
- AC: second HSM holds RelKey_B via a documented, witnessed ceremony; failover signing is exercised in a drill and timed; access to either HSM requires quorum per the key-management policy
- Meta: Shard=E | Type=Ops | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-SEC-08 | Const=C-06, C-05

### S-23-019 — `ss-verify` verifier tool
As a sovereignty-conscious user I want an `ss-verify` tool so that I can independently confirm a firmware binary matches the published source.
- AC: `ss-verify` rebuilds and byte-compares any `main` artifact (epic exit criterion 1); it verifies signature, SLSA provenance, and SBOM digest chain in one command; it runs on stock Linux using open toolchains only per NF-SUS-03
- Meta: Shard=B | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=NF-SUS-03 | Const=C-OA, C-05, C-06

### S-23-020 — Signing-outage runbook
As an SRE I want a signing-outage runbook so that HSM failure or suspected key compromise has a rehearsed response, including escrowed RelKey_C escalation.
- AC: runbook covers HSM failure, suspected RelKey_B compromise, and the escrowed RelKey_C cold-rotation path per F-SEC-09; a tabletop exercise has been completed against the runbook; an owner and review cadence are assigned
- Meta: Shard=E | Type=Docs | Size=S | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-SEC-09 | Const=C-06, C-05
