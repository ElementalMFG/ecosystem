<!-- SPDX-License-Identifier: CC-BY-4.0 -->
- Title: Adoption of the RFC process as the change-control mechanism for protocol- and security-relevant changes
- Author(s): SS-SP program lead
- Shepherd: wg-community (bootstrap: program lead)
- Status: ACCEPTED
- Start Date: 2026-07-04
- Feature area (WG): wg-community / governance
- Requires: —
- Supersedes: —

# Summary

This RFC ratifies the RFC process itself. From this point forward, every change that touches a wire format, a public HAL/SDK API, capability flags or cryptographic primitives, release cadence/LTS policy, or a charter-level governance rule MUST go through the RFC lifecycle defined in `../06_GOVERNANCE.md` (§4, "RFC Process") before implementation. This document exists so that the process has a citable, versioned anchor inside the `rfcs/` directory and so that contributors have a minimal worked example of the template in use.

# Motivation

`06_GOVERNANCE.md` §4 defines when an RFC is required and the lifecycle states, and `rfcs/TEMPLATE.md` defines the document shape — but until a first RFC is merged, the process is untested and the directory is empty. Bootstrapping the process with an RFC *about* the process (a) exercises the template end-to-end, (b) gives future authors a concrete example to copy, and (c) makes the adoption of the process itself an auditable decision rather than an implicit convention. Decision D-0001 in `../governance/decisions.md` launched the program and its licensing regime; this RFC extends that foundation to change control.

# Detailed design

1. **Scope of mandatory RFCs.** An RFC is required for the trigger list in `06_GOVERNANCE.md` §4.1: wire-format changes (SS-Link frames, RNS/LXMF profile), public HAL/SDK API surface, capability flags and crypto-primitive selection, release cadence/LTS commitments, and charter-level governance changes. Anything else may use the normal PR flow.
2. **Lifecycle.** `IDEA → DRAFT → REVIEW → FINAL CALL → ACCEPTED / REJECTED → IMPLEMENTED → OBSOLETED`, exactly as in `06_GOVERNANCE.md` §4.2. FINAL CALL lasts a minimum of 14 calendar days once the community is larger than the bootstrap maintainership.
3. **Filenames and numbering.** Drafts live at `rfcs/DRAFT-<slug>.md`. A number `NNNN` is assigned only at ACCEPTED, at which point the file is renamed `rfcs/<NNNN>-<slug>.md`. Numbers are never reused.
4. **Record of decision.** Every ACCEPTED or REJECTED RFC gets a corresponding entry in `../governance/decisions.md` citing the RFC number.
5. **Bootstrap quorum.** Until working groups are staffed (S-01-009), the program lead acts as author, shepherd, and approver; every bootstrap acceptance is flagged as such in the decisions log and is subject to re-ratification at the first quarterly constitutional review (S-01-016).

# Wire-format changes

None. This RFC changes process only; no bytes on the wire change.

# Security considerations

The RFC process is itself a security control: it forces cryptographic and wire-format changes through a mandatory `# Security considerations` review referencing `../05_SECURITY_MODEL.md`. The bootstrap-quorum clause (single approver) is a known temporary weakness; it is mitigated by the re-ratification requirement and by DCO sign-off on every merge.

# Interoperability impact

None directly. Indirectly positive: Meshtastic wire-compat and RNS-transport changes can no longer land without an RFC documenting interop impact.

# Backward compatibility & migration

Nothing breaks. Changes merged before this RFC (the constitution set, the firmware scaffold) are grandfathered; any future modification to them that hits a §4.1 trigger requires an RFC.

# Rejected alternatives

- **GitHub issues/discussions as the only change record** — rejected: not versioned alongside the code they govern, poor for archival citation.
- **Adopting an external process wholesale (Rust RFCs, PEPs) without adaptation** — rejected: those assume large staffed teams; ours must degrade gracefully to a bootstrap maintainership while remaining honest about quorum.
- **No formal process until Phase 2 (foundation transfer)** — rejected: retrofitting change control after wire formats ship is how compatibility promises get broken.

# Unresolved questions

- Exact FINAL CALL announcement channels (mailing list vs forum) — deferred until VA-02/VA-05 (org + domains) exist.

# Prior art

Rust RFCs, Python PEPs, IETF Internet-Drafts → RFCs (numbering-at-acceptance is borrowed from IETF practice), Zephyr Project RFC label flow.

# Implementation plan

Already implemented by the merge of this file plus `rfcs/README.md` and `rfcs/TEMPLATE.md`. Enforcement of the trigger list in CI (wire-format PRs must cite an RFC) is tracked by S-01-017.
