<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# EPIC-07 — Stories

Format per `../../00_METHODOLOGY.md` §2.7. Meta lines are machine-parsed.

### S-07-001 — Root-key ceremony script + air-gap procedure
As a security engineer I want a scripted, rehearsable RelKey_A generation ceremony that runs on an air-gapped machine so that the root of trust is created without network exposure and every step is auditable.
- AC: ceremony script completes on a machine with all network interfaces verified down; every step appends a hash-chained transcript entry; a full dry-run with throwaway keys passes before the live ceremony; transcript containing only the public part is publishable
- Meta: Shard=A | Type=Ops | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-MFG-01 | Const=C-05, C-OA

### S-07-002 — M-of-N (3-of-5) Shamir shard split for RelKey_A
As a security engineer I want RelKey_A split into 5 Shamir shards with a 3-of-5 reconstruction threshold so that no single custodian can use or lose the root key.
- AC: split produces 5 shards and any 3 reconstruct the exact private key in a test vector run; any 2 shards reconstruct nothing (verified against reference implementation); shard custody assignment and geographic distribution recorded in the ceremony transcript
- Meta: Shard=A | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-05, C-OA

### S-07-003 — Publish RelKey_A public part in constitution
As a release manager I want the RelKey_A public key published in the repository's constitution documents so that anyone can independently verify the device trust chain.
- AC: RelKey_A public key and its fingerprint committed to `05_SECURITY_MODEL.md`; fingerprint matches the ceremony transcript; publication PR carries wg-security sign-off
- Meta: Shard=A | Type=Task | Size=XS | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-05, C-OA

### S-07-004 — HSM-backed RelKey_B signing service
As a release manager I want RelKey_B held in an HSM and exposed only through a signing service so that release firmware can be signed without the private key ever existing in software.
- AC: RelKey_B generated inside the HSM and marked non-exportable; signing service signs a test firmware image and the signature verifies against the published RelKey_B public key; every signing operation is logged with requester identity and artifact hash
- Meta: Shard=B | Type=Ops | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-SEC-08 | Const=C-05

### S-07-005 — HSM-backed RelKey_C signing (community channel)
As a release manager I want RelKey_C held in an HSM with a separate signing path for the community channel so that community builds can be signed independently of the commercial release pipeline.
- AC: RelKey_C generated inside an HSM and marked non-exportable; community-channel test manifest signed and verified against the RelKey_C public key; signing path is access-controlled separately from RelKey_B; escrow copy prepared per S-07-007
- Meta: Shard=C | Type=Ops | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-SEC-09 | Const=C-05, C-OA

### S-07-006 — Key rotation RFC (RFC-0004)
As a security engineer I want an accepted RFC defining rotation cadence, procedure, and revocation for RelKey_B and RelKey_C so that key compromise or scheduled rotation has a pre-agreed, reviewable playbook.
- AC: RFC-0004 covers rotation cadence, emergency rotation trigger, revocation distribution, and device-side key-update behaviour; RFC passes the governance review process to ACCEPTED; procedure references the escrowed RelKey_C cold-rotation path
- Meta: Shard=B | Type=RFC | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-SEC-09 | Const=C-05, C-06

### S-07-007 — Escrow contract with foundation (RFC-0005)
As a device owner I want RelKey_C legally escrowed with a named foundation custodian so that devices remain updatable even if the founding vendor disappears.
- AC: RFC-0005 names the escrow custodian, release conditions, and transfer clause to the foundation; contract terms reviewed and signed off by wg-legal; escrow verification procedure lets a third party confirm the escrowed material matches the published RelKey_C public key
- Meta: Shard=C | Type=RFC | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-SEC-09 | Const=C-OA, C-06

### S-07-008 — DevCert format spec + ASN.1 profile
As a firmware engineer I want a specified SS-SP DevCert format with a fixed ASN.1 profile so that device certificates parse identically on-device, on the provisioning line, and in cloud services.
- AC: spec defines fields (device serial, Ed25519 public key, X25519 public key, SKU, issuance metadata) and chain to RelKey_A; ASN.1 profile round-trips encode/decode in the reference parser with byte-exact output; negative tests reject malformed and mis-chained certificates
- Meta: Shard=E | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-SEC-01, F-SEC-02 | Const=C-05

### S-07-009 — Per-device Ed25519 + X25519 keygen on line
As a factory operator I want each device to generate its Ed25519 signing and X25519 KEX keypair during line provisioning so that every unit ships with a unique identity whose private keys never leave the device.
- AC: keygen runs on-device seeded per NF-SEC-01 (HW-RNG / eFuse seed); private keys are never transmitted over the line interface (verified by bus capture on the test rig); 1 000-unit simulated run produces zero duplicate public keys
- Meta: Shard=D | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-SEC-01, F-SEC-02, NF-SEC-01 | Const=C-05

### S-07-010 — Secure-element sealing of device keys
As a security engineer I want device private keys sealed to the secure element / eFuse-protected storage so that keys cannot be extracted from a captured device.
- AC: sealed keys are unreadable via flash dump of a provisioned unit (ciphertext only); signing and KEX operations succeed using sealed keys after reboot and power loss; sealing failure on the line marks the unit as scrap with an audit-log entry
- Meta: Shard=D | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-SEC-01, F-SEC-02, NF-SEC-01 | Const=C-05

### S-07-011 — Provisioning-line firmware image
As a factory operator I want a dedicated, signed provisioning firmware image driven over USB/JTAG so that the line can provision units deterministically and a tampered line image is detected.
- AC: provisioning image is signed and its hash verified by the line tool before each session; image executes keygen, sealing, DevCert install, and eFuse configuration in one pass; provisioning session talks to the manufacturing HSM for DevCert issuance; any step failure leaves the unit in a defined re-runnable state
- Meta: Shard=F, G | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-MFG-01 | Const=C-05

### S-07-012 — Provisioning-line audit log (signed entries)
As a fleet admin I want every provisioning action recorded as a signed, append-only log entry so that per-device manufacturing history is verifiable offline.
- AC: each provisioned unit produces a signed log entry containing serial, DevCert hash, timestamp, and line-station ID; offline verifier validates a day's log with no network access; log entries feed the per-device transparency-log manifest
- Meta: Shard=H | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-MFG-04 | Const=C-05, C-OA

### S-07-013 — Line-side operator UI (Electron or web)
As a factory operator I want a line-side UI showing per-station progress, pass/fail, and error causes so that I can run the line without reading serial consoles.
- AC: UI shows live per-unit state for each provisioning step; failed units display an actionable error code and are flagged for rework; framework choice (Electron vs web) recorded in an ADR; UI cannot bypass or mute audit logging
- Meta: Shard=F | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-MFG-01 | Const=C-05

### S-07-014 — Line throughput target — 100 units/hour
As a factory operator I want the provisioning line to sustain at least 100 units/hour on the test rig so that manufacturing volume targets are achievable.
- AC: test rig sustains ≥ 100 provisioned units/hour over a continuous one-hour run; per-step timing breakdown published to identify bottlenecks; throughput run performed with audit logging and HSM signing enabled (no shortcuts)
- Meta: Shard=F | Type=Ops | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-MFG-01 | Const=C-05

### S-07-015 — On-device DevCert verification at boot
As a firmware engineer I want the device to verify its DevCert against the embedded RelKey_A root at boot so that a unit with a missing, corrupt, or mis-chained certificate is detected before joining the mesh.
- AC: valid DevCert verifies against the embedded root on every boot within the boot-time budget; corrupt or wrong-chain DevCert puts the device into a defined unprovisioned state with a user-visible indication; verification failure is logged locally without leaking key material
- Meta: Shard=E | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-SEC-01 | Const=C-05

### S-07-016 — Factory reset flow — wipe + re-attest
As a device owner I want a factory reset that cryptographically wipes all user data and keys and returns the device to first-boot state so that I can resell or dispose of the device safely.
- AC: reset destroys all user data, session state, and device-generated key material (verified by post-reset flash inspection showing no recoverable plaintext); device re-attests to first-boot state and re-runs first-boot key derivation; reset is user-initiated with an explicit confirmation step
- Meta: Shard=I | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-MFG-03, NF-SEC-02 | Const=C-05

### S-07-017 — Reprovisioning (post-repair) mode
As a factory operator I want a field/RMA reprovisioning ceremony so that a repaired device can receive a fresh identity and DevCert without factory-line hardware.
- AC: reprovisioning issues a new keypair and DevCert and revokes association with the old identity; ceremony works on the bench-programmer variant; reprovisioning event is recorded in the signed audit log with an RMA reference
- Meta: Shard=I | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-MFG-03, F-MFG-02 | Const=C-05

### S-07-018 — Post-line QC image (self-test)
As a factory operator I want a post-provisioning QC self-test image so that every unit proves its keys, certificate, and radios work before it leaves the line.
- AC: self-test exercises DevCert chain verification, a sign/verify round-trip with the sealed keys, and basic radio TX/RX; QC result (pass/fail with failure code) is written to the signed audit log; failed units are automatically routed to rework, not shipping
- Meta: Shard=F | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-MFG-01 | Const=C-00, C-05

### S-07-019 — Bulk key export to Fleet Console for enterprise buyers
As a fleet admin I want the public identities and DevCerts of a purchased batch exportable in bulk to my Fleet Console tenant so that hundreds of devices enrol without per-unit manual pairing.
- AC: export contains only public material (public keys, DevCerts, serials) — verified by schema check that rejects private-key fields; export format imports cleanly into a Fleet Console tenant in a batch test of ≥ 100 units; export generation is recorded in the provisioning audit log
- Meta: Shard=H | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-CL-01, F-CL-02 | Const=C-05

### S-07-020 — Lost/stolen device: identity revocation + contact notification
As a device owner I want to revoke a lost or stolen device's identity and have my contacts warned so that whoever holds the device cannot impersonate me on the mesh.
- AC: a revocation statement signed by the owner's recovery material (or fleet admin per policy) propagates over LXMF, marks the identity compromised in receiving rosters, and blocks new sessions to/from it with a visible warning; revocation works with the lost device fully offline (it is the peers that enforce, not the device); a replacement identity restored from backup can be cryptographically linked to the revoked one so existing contacts re-bind conversations without manual re-verification; revocation interacts safely with the duress-wipe compromise beacon (F-SEC-10) — the two paths converge on the same compromised-identity state
- Meta: Shard=— | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-SEC-10, NF-SEC-02 | Const=C-05

### S-07-021 — Duress-unlock engine: silent wipe + compromise beacon
As a device owner I want entering my duress PIN to silently wipe designated secrets and emit a compromise beacon so that coerced unlock cannot expose my history or my contacts (F-SEC-10).
- AC: duress unlock is timing- and UI-indistinguishable from normal unlock while atomically destroying the designated key set (ratchet states, message store keys per `05_SECURITY_MODEL.md` §8.3) such that interrupted wipes never leave recoverable material (power-cut test); the wipe set includes erasing the `coredump` partition, since panic dumps can persist RAM-resident key state (S-02-008 hygiene contract, `firmware/main/ss_panic_guard.h`); a signed compromise beacon is queued for transmission per policy and converges on the same compromised-identity state as S-07-020 revocation; the exact wrap cipher, nonce derivation, and wipe order are specified in the story design doc and reviewed by wg-security; behaviour composes with tamper wipe (S-05-014) without double-wipe faults
- Meta: Shard=— | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=F-SEC-10, NF-SEC-02 | Const=C-05

### S-07-022 — Dual-eFuse identity anchor (P4 UID + C6 MAC signed manifest)
As a security engineer I want each device's identity anchored to the P4 eFuse UID with the C6 MAC bound via a signed device manifest so that a swapped or spoofed radio co-processor is detected and quarantined.
- AC: device identity anchor = the P4 eFuse UID; the C6 MAC is bound to it via a signed device manifest; provisioning verifies the binding before the unit passes; a binding mismatch drives the device into a quarantine state (SPEC-5)
- Meta: Shard=D | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=A,O | PRD=F-SEC-01, F-SEC-02 | Const=C-05
