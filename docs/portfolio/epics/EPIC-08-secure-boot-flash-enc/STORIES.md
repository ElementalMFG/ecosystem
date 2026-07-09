<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# EPIC-08 — Stories

Format per `../../00_METHODOLOGY.md` §2.7. Meta lines are machine-parsed.

### S-08-001 — Secure Boot v2 pubkey burn (RelKey_B)
As a firmware engineer I want the RelKey_B RSA-3072-PSS public-key digest burned to eFuse and Secure Boot v2 enabled so that the device only executes bootloaders signed by the release key.
- AC: RelKey_B-signed bootloader boots on a fused unit; an unsigned or differently-signed bootloader is refused; eFuse burn procedure includes a line-side dry-run check before permanent write (per R08-02); D-0022: RSA-3072-PSS is the ratified silicon-locked SBv2 algorithm; application manifests are Ed25519 — never unify the two
- Meta: Shard=A | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-SEC-06 | Const=C-05

### S-08-002 — Secondary pubkey slot for RelKey_C (opt-in)
As a device owner I want a secondary Secure Boot key slot carrying RelKey_C so that I can opt my device into community-channel firmware without weakening the default chain of trust.
- AC: RelKey_C slot is provisioned but inactive by default; when the owner opts in, RelKey_C-signed firmware boots; with opt-in disabled, RelKey_C-signed firmware is refused; opt-in state survives OTA updates
- Meta: Shard=A | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-SEC-06, F-SEC-09 | Const=C-05

### S-08-003 — Bootloader signing job in CI (HSM)
As a release manager I want the bootloader signed in CI via the HSM-backed signing service so that release bootloaders are produced only through the audited pipeline.
- AC: CI job requests signatures from the HSM service and never handles a raw private key (verified by pipeline audit); signed bootloader artifact verifies against the published RelKey_B public key; signing job records artifact hash and requester in the signing log
- Meta: Shard=C | Type=Ops | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-SEC-06 | Const=C-05, C-06

### S-08-004 — Flash encryption enable + release-mode
As a security engineer I want AES-256-XTS flash encryption enabled in release mode with a device-unique eFuse key so that flash contents are unreadable off-device.
- AC: flash read of an encrypted unit without the device key yields only ciphertext (exit criterion 3); device boots and runs normally with encryption enabled; release-mode fuses prevent re-flashing plaintext firmware; encryption key is device-unique and never leaves eFuse
- Meta: Shard=B | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-SEC-07, NF-SEC-02 | Const=C-05

### S-08-005 — Partition-level encryption policy
As a firmware engineer I want a documented per-partition encryption policy so that every partition holding user data or keys is encrypted and only explicitly justified partitions remain plaintext.
- AC: partition table annotates each partition as encrypted or plaintext with a written justification for every plaintext entry; automated check fails the build if a user-data or key partition is marked plaintext; policy verified on-device by reading each partition raw
- Meta: Shard=B | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-SEC-07, NF-SEC-02 | Const=C-05

### S-08-006 — eFuse anti-rollback counter integration
As a security engineer I want firmware versions bound to a monotonic eFuse anti-rollback counter so that downgrade attacks to vulnerable firmware are impossible.
- AC: firmware with a security-version lower than the eFuse counter refuses to boot; counter bump happens only after a successful boot of the new version; counter exhaustion budget is documented against the expected device lifetime
- Meta: Shard=D | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-SEC-08, NF-SEC-04 | Const=C-05

### S-08-007 — Boot chain runtime verifier
As a firmware engineer I want a runtime component that verifies the full bootloader → app chain and exposes the result so that later stages (attestation, diagnostics) can rely on a measured boot state.
- AC: verifier reports signature status and key slot used (RelKey_B vs RelKey_C) for each chain stage; a deliberately corrupted app partition is detected and reported before application start; verification result is available via an internal API for the attestation service
- Meta: Shard=F | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-SEC-06 | Const=C-05

### S-08-008 — Community-channel opt-in UX flow
As a device owner I want an explicit on-device consent flow to enable the community firmware channel so that switching trust to RelKey_C is deliberate and informed.
- AC: enabling requires a physical button hold plus a typed confirmation string (exit criterion 4); flow displays a plain-language warning about warranty and trust implications; opt-in and opt-out events are logged locally; flow cannot be triggered remotely or by an app alone
- Meta: Shard=E | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-SEC-09 | Const=C-05, C-03

### S-08-009 — Tamper switch → chip-erase policy
As a security engineer I want a defined tamper-response policy (brownout and tamper-switch events → configurable chip-erase) so that physical attacks on a device cannot recover protected material.
- AC: tamper policy is configurable per deployment profile (consumer default vs high-security); triggering the tamper condition on a test unit executes the configured response and the unit re-enters a defined recoverable state; false-trigger behaviour under brownout is characterised on the rack
- Meta: Shard=G | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-05

### S-08-010 — JTAG disable via eFuse
As a security engineer I want JTAG permanently disabled via eFuse on production units so that debug ports cannot be used to extract keys or firmware.
- AC: JTAG connection attempts fail on a production-fused unit (exit criterion 5); disable step is part of the provisioning-line sequence and recorded in the audit log; development units remain distinguishable from production units by fuse state
- Meta: Shard=H | Type=Feature | Size=S | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-05

### S-08-011 — Debug re-enable ceremony (RMA)
As a factory operator I want a documented, authorised ceremony for regaining debug access on RMA units so that failure analysis is possible without creating a fleet-wide backdoor.
- AC: ceremony requires the RMA unit to be factory-reset (keys wiped) before any debug re-enable step; procedure is documented and reviewed by wg-security (per R08-03 external audit of debug paths); each ceremony execution is logged with unit serial and operator identity
- Meta: Shard=H | Type=Ops | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-MFG-03 | Const=C-05

### S-08-012 — Remote-attestation quote generator
As a fleet admin I want devices to produce signed attestation quotes of their boot state so that cloud enrollment can verify a device runs unmodified, current firmware.
- AC: quote includes firmware version, boot key slot, secure-boot/flash-encryption fuse state, and a challenge nonce, signed with the device identity key; a replayed quote (stale nonce) is rejected by the verifier; quote generation works offline and is transmitted only during enrollment
- Meta: Shard=I | Type=Feature | Size=L | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-CL-02 | Const=C-05

### S-08-013 — Reproducible bootloader build verification
As an external auditor I want the bootloader build to be bit-for-bit reproducible from source so that anyone can confirm the shipped bootloader matches the published code.
- AC: two independent clean-room builds from the same tag produce identical binaries; CI job diffs the reproduced binary against the signed release artifact (pre-signature payload) and fails on mismatch; reproduction instructions use open toolchains only per NF-SUS-03
- Meta: Shard=C | Type=Ops | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=NF-SUS-03, NF-SEC-05 | Const=C-05, C-OA

### S-08-014 — Modified-firmware refusal test on rack
As a security engineer I want an automated rack test proving that modified firmware refuses to boot so that exit criterion 1 is continuously verified, not a one-off demo.
- AC: rack test flashes a bit-flipped app image and a re-signed-with-wrong-key image and both are refused; test runs in CI against hardware on every release candidate; refusal path leaves the device bootable into its previous valid firmware
- Meta: Shard=F | Type=Ops | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-SEC-06 | Const=C-05

### S-08-015 — Rollback-refusal test on rack
As a security engineer I want an automated rack test proving that older firmware refuses to boot after an eFuse counter bump so that anti-rollback is continuously verified.
- AC: rack test installs version N+1, bumps the counter, then attempts to boot version N and the boot is refused (exit criterion 2); test verifies the counter is bumped only after a successful N+1 boot; test runs in CI against hardware on every release candidate
- Meta: Shard=D | Type=Ops | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=NF-SEC-04 | Const=C-05

### S-08-016 — Chip-erase brick-recovery documentation
As a device owner I want clear documentation on what a tamper-triggered chip-erase means and which recovery paths exist so that I know whether a device is recoverable and how.
- AC: doc explains the erase trigger conditions, what data is destroyed, and the recovery-mode path requiring a signed bootloader (per R08-01); doc distinguishes recoverable states from permanent-scrap states; doc is published in the user documentation set and reviewed by wg-security
- Meta: Shard=G | Type=Docs | Size=S | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-05

### S-08-017 — Lite-optional ATECC608 secure-element HAL (`ss_se_*`)
As a firmware engineer I want the declared secure-element surface implemented for the Lite-optional ATECC608 so that key-storage consumers have a contract path on fitted units.
- AC: `ss_se_*` (frozen ss_hal_secure_elem.h) implemented for the I²C0 ATECC608 optional module, capability-gated (absent module degrades per caps, never fakes success); key material never leaves the SE unwrapped per doc 05; part/address confirmed at attachment per D-0013; distinct from the Omega S-05-013 path; 3-board CI green
- Meta: Shard=— | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=L | PRD=— | Const=C-00,C-05
