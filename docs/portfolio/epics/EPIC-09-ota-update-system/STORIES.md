<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# EPIC-09 — Stories

Format per `../../00_METHODOLOGY.md` §2.7. Meta lines are machine-parsed.

### S-09-001 — OTA manifest v1 schema (JSON + Ed25519 signatures)
As a firmware engineer I want a versioned JSON OTA manifest schema carrying version, image hash, delta info, and Ed25519 signature blocks so that every update is described by a single verifiable document.
- AC: schema validates all fields required by shard S-09.A (version, hash, delta info, signature blocks for RelKey_B and/or RelKey_C); schema includes anti-rollback security-version and a nonce field (per R09-02 replay mitigation); malformed and unknown-version manifests are rejected by the reference parser with distinct error codes
- Meta: Shard=A | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-SEC-08 | Const=C-05

### S-09-002 — Dual-signature verification on device
As a security engineer I want the device to require valid RelKey_A-chained dual signatures (RelKey_A + RelKey_B) on release manifests before installing so that a single compromised signing key cannot push firmware to the fleet.
- AC: manifest with both required valid signatures installs; manifest with only one valid signature is rejected; RelKey_C single-signature path is accepted only when the community-channel opt-in is enabled; verification failures are logged with the offending key slot
- Meta: Shard=A | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-SEC-08 | Const=C-05

### S-09-003 — bsdiff/hdiff delta patcher on device
As a firmware engineer I want an on-device bsdiff/hdiff delta patcher so that updates download a small patch instead of a full image.
- AC: delta update reduces payload ≥ 60 % vs the full image on a representative release pair (exit criterion 3); patched image hash matches the manifest full-image hash before boot switch; patcher operates within the device RAM budget and resumes after power loss mid-patch
- Meta: Shard=B | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=NF-REL-02 | Const=C-05

### S-09-004 — A/B partition switch after successful boot
As a firmware engineer I want updates written to the inactive partition and the boot slot switched only after verification so that a bad update never leaves the device unbootable.
- AC: update installs to the inactive slot while the active slot keeps running; boot switch happens only after image hash and signature verification; power loss at any point during install leaves the device booting the previous slot
- Meta: Shard=C | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=NF-REL-03 | Const=C-05

### S-09-005 — First-boot success flag + rollback watchdog
As a firmware engineer I want new firmware to confirm a healthy first boot before being marked valid, with a watchdog forcing rollback otherwise so that failed updates self-heal within one boot cycle.
- AC: new firmware that fails to set the success flag is rolled back automatically within one boot cycle (exit criterion 2); watchdog covers hangs and crash loops during first boot; success flag is set only after core services (mesh, UI, storage) report healthy
- Meta: Shard=E | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=NF-REL-02, NF-REL-03 | Const=C-05

### S-09-006 — Wi-Fi download w/ resume + integrity
As a device owner I want OTA downloads over Wi-Fi to resume after interruption and verify integrity so that updates complete on flaky home networks without corruption.
- AC: interrupted download resumes from the last verified byte range instead of restarting; completed download hash matches the manifest hash before install begins; a Lite unit completes the full update-over-Wi-Fi flow with signature verification (exit criterion 1)
- Meta: Shard=F | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-BR-02, NF-REL-02 | Const=C-08, C-05

### S-09-007 — Cellular download w/ backoff & data-cap awareness
As a device owner I want OTA over cellular to respect data caps and back off politely so that an Omega update never causes billing runaway.
- AC: download pauses when the user-configured data cap would be exceeded and resumes on Wi-Fi or user override; retry uses exponential backoff with jitter on cellular errors; non-critical updates default to deferring until a non-metered bearer is available
- Meta: Shard=F | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=O | PRD=F-BR-06 | Const=C-08

### S-09-008 — LoRa critical-patch mode (< 32 KB, FEC)
As a security engineer I want security-critical patches under 32 KB deliverable over LoRa with forward error correction so that off-grid devices can still receive urgent fixes.
- AC: a < 32 KB signed patch delivers over LoRa to a device with no other bearer, with FEC recovering from injected packet loss; patch retains full manifest signature verification (no reduced security on LoRa path); transfer respects LoRa duty-cycle limits per NF-REG-05
- Meta: Shard=F | Type=Feature | Size=L | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-BR-01, NF-REG-05 | Const=C-08, C-05

### S-09-009 — Cohort tag storage + server-driven staging
As a release manager I want each device to carry a persistent cohort tag honoured by the OTA server so that rollouts can stage 1 % → 10 % → 50 % → 100 % across the fleet.
- AC: cohort tag persists across updates and factory data preservation rules; server offers a manifest only to devices in currently-enabled cohorts (verified in the sim); cohort assignment distribution across a test fleet matches the configured percentages within tolerance
- Meta: Shard=D | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=NF-REL-02 | Const=C-05

### S-09-010 — OTA server REST API
As a release manager I want a REST API for publishing manifests, controlling cohorts, and reading rollout status so that releases are operated programmatically rather than by hand.
- AC: API supports manifest upload, cohort enable/pause, and per-cohort status query with authenticated access; API refuses manifests whose signatures do not verify; API is exercised end-to-end by the community OTA channel deployment
- Meta: Shard=H | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-CL-05, NF-REL-05 | Const=C-05

### S-09-011 — Cohort scheduling policy engine
As a release manager I want a policy engine that advances a rollout through cohorts A (1 %) → B (10 %) → C (50 %) → D (100 %) based on health signals so that staging is systematic instead of manual.
- AC: engine advances to the next cohort only when the configured health criteria (success rate, error budget) are met; engine supports manual hold and manual abort at any stage; every cohort transition is logged with the triggering metrics
- Meta: Shard=D | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=NF-REL-02 | Const=C-05

### S-09-012 — Automatic pause on error-rate spike
As a release manager I want rollouts to pause automatically when the update error rate spikes so that a bad release cannot brick the wider fleet (risk R09-03).
- AC: rollout pauses automatically when error rate crosses the configured threshold in the current cohort; pause fires an alert to the release operators; no further devices are offered the manifest until a human explicitly resumes or aborts
- Meta: Shard=D | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=NF-REL-02 | Const=C-05

### S-09-013 — Force-update policy (security-critical)
As a security engineer I want security-critical updates to be non-indefinitely-deferrable by policy so that known-vulnerable firmware does not persist in the field.
- AC: manifests flagged security-critical enforce a bounded defer window after which install is scheduled automatically; user is clearly notified before any forced install and SOS availability is never interrupted mid-update; force-update behaviour is documented and consistent with the CRA vulnerability-handling process per NF-REG-03
- Meta: Shard=I | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=NF-REG-03, NF-SEC-04 | Const=C-05

### S-09-014 — User UX: notify, defer, install
As a device owner I want a clear on-device update flow — notification, release notes, defer, install now — so that updates happen on my terms within policy limits.
- AC: notification shows version, size, and summary before any install; defer option works up to the policy limit and re-prompts afterwards; install-now path shows progress and completes into the new version or a clean rollback; flow is operable one-handed per F-UI-06
- Meta: Shard=I | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-03, C-05

### S-09-015 — SLSA-3 provenance emission per artifact
As a release manager I want every release artifact to carry SLSA-3 provenance so that consumers can verify what source and build produced each binary.
- AC: CI emits a SLSA-3 provenance file for every published firmware artifact; provenance links source commit, builder identity, and artifact hash; provenance publication is part of the release gate and its absence fails the release
- Meta: Shard=G | Type=Ops | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=NF-SEC-05, NF-SUS-03 | Const=C-OA, C-05

### S-09-016 — `ss-verify` CLI reproduces build hash
As a sovereignty-conscious device owner I want an `ss-verify` CLI that rebuilds firmware from source and compares hashes so that I can independently confirm my device runs the published code.
- AC: `ss-verify` reproduces the same hash from the source tree for a tagged release (exit criterion 5); tool verifies the SLSA provenance chain and the manifest signature in the same run; mismatch output clearly identifies which artifact differs; tool builds with open toolchains only per NF-SUS-03
- Meta: Shard=G | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=NF-SUS-03, NF-SEC-05 | Const=C-OA, C-05

### S-09-017 — Community-channel OTA (RelKey_C) opt-in flow
As a device owner I want my opted-in device to fetch and install RelKey_C-signed manifests from the community channel so that I can run community firmware through the same safe OTA machinery.
- AC: community-channel manifest signed by RelKey_C installs on an opted-in device (exit criterion 4); the same manifest is refused on a device without opt-in; channel switch preserves A/B rollback and anti-rollback protections unchanged
- Meta: Shard=A | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-CL-05, F-SEC-09 | Const=C-05, C-OA

### S-09-018 — OTA e2e integration test in EPIC-22 sim
As a firmware engineer I want an end-to-end OTA scenario in the EPIC-22 simulation so that the full pipeline — manifest, download, patch, switch, rollback — is regression-tested on every merge.
- AC: sim scenario covers success path, mid-download power loss, corrupted image, and failed-first-boot rollback; scenario runs in CI and gates merges to the OTA components; scenario exercises staged cohort progression against the OTA server API
- Meta: Shard=— | Type=Ops | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=NF-REL-02, NF-REL-03 | Const=C-00, C-05

### S-09-019 — Modem FOTA integration for Omega
As a firmware engineer I want the Omega cellular modem's own firmware updatable through the OTA system so that modem vulnerabilities are patchable through the same signed pipeline.
- AC: modem firmware package is distributed inside a signed OTA manifest and verified before being handed to the modem; failed modem FOTA leaves the modem on its previous firmware and reports the failure; modem update never interrupts an active SOS
- Meta: Shard=F | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=O | PRD=F-BR-06 | Const=C-08, C-05

### S-09-020 — Documentation for community-channel builders
As a community firmware builder I want documentation covering how to build, sign (RelKey_C pipeline), and publish to the community OTA channel so that community releases follow the same safety rails as official ones.
- AC: doc walks through reproducible build, manifest creation, RelKey_C signing submission, and channel publication end-to-end; a community tester following only the doc successfully publishes to a staging channel; doc states the anti-rollback and opt-in constraints that apply to community builds
- Meta: Shard=G | Type=Docs | Size=S | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-CL-05, NF-SUS-03 | Const=C-OA, C-05

### S-09-021 — Release-day OTA distribution at 1 M-device scale
As a release engineer I want CDN-fronted, delta-capable, mirror-friendly artifact distribution so that a release wave reaches one million devices within 72 hours without a single distribution point existing.
- AC: binary-delta updates are produced alongside full images and verified against the same signed manifest (deltas cut wave volume ≥ 5× on a representative version step); the community channel sits behind a CDN while remaining byte-identical to origin so third-party mirrors serve the same signed artifacts; a load drill demonstrates a simulated 1 M-device wave completing ≤ 72 h within the staged-rollout percentages and automatic-rollback rails; the mirror list is itself signed and mirrorable, and a client with origin + CDN blackholed still updates from a community mirror
- Meta: Shard=G | Type=Ops | Size=L | Prio=P1 | Status=DRAFT | SKU=★ | PRD=NF-SCALE-05, NF-REL-05 | Const=C-OA, C-08

### S-09-022 — Web-based flasher & zero-install onboarding
As a new owner or community builder I want to flash and first-configure any SS-SP device from a browser (WebSerial/WebUSB) with no toolchain installed so that going from unboxing or bare board to a working node takes minutes, matching the strongest onboarding in the category (Meshtastic/RNode web flashers per `06_COMPETITIVE_LANDSCAPE.md` §4.3).
- AC: a static, self-hostable web app flashes official and community channel images to all three SKUs over WebSerial/WebUSB, verifying the same signed OTA manifests as the device updater (no unsigned side channel); first-boot essentials (region/band plan, device name, initial profile per S-16-024) are configurable in the same flow; the flasher works fully offline from a local copy with pre-downloaded artifacts; browser support matrix and a CLI fallback are documented; the tool is published under the firmware license and mirrorable per NF-SUS-03
- Meta: Shard=G | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-CL-05, NF-SUS-03 | Const=C-OA, C-04

### S-09-023 — Signed asset/model pack updates (voice models, fonts, tiles)
As a release manager I want large non-firmware assets — STT/TTS models, font bundles, map-tile packs — delivered as independently versioned, signed packs so that a 40 MB Whisper model update never requires a firmware release.
- AC: an asset-pack manifest format (pack id, semver, target `fs_models`/`fs_user` location, hash tree, Ed25519 signature chained to the release keys per `05_SECURITY_MODEL.md` §6.7) is specified and verified on device before activation; packs install resumably over any bearer or SD side-load, with atomic swap and rollback to the prior pack on verification failure; firmware↔pack compatibility is declared in the manifest (min/max firmware version) and enforced; voice pipeline (EPIC-14) loads models exclusively through this mechanism, verified by integration test
- Meta: Shard=— | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=NF-REL-02, F-VOX-01 | Const=C-05
