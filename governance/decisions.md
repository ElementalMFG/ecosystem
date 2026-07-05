# Steering Committee Decisions Log

Chronological, append-only. Each entry is signed by the Chair at close.

---

## D-0001 — Program launch

- Date: 2025-Q?  (bootstrap)
- Decision: Establish SS-SP program under founding vendor entity. Adopt Apache-2.0 for firmware/apps, CC-BY 4.0 for spec docs.
- Status: RECORDED
- Reference: `04_LICENSING_AND_FORK_STRATEGY.md`, `06_GOVERNANCE.md`

## D-0002 — No Meshtastic firmware fork

- Date: 2025-Q?
- Decision: SS-SP does not fork or link against the Meshtastic firmware or `.proto` files. Interoperability with Meshtastic is achieved via clean-room wire-format reimplementation under Apache-2.0.
- Rationale: GPL-3.0 viral licensing incompatible with commercial-sale device shipping with secure boot, and with paid enterprise extensions.
- Reference: `04_LICENSING_AND_FORK_STRATEGY.md` §3

## D-0003 — Adopt Reticulum / LXMF under Reticulum License

- Date: 2025-Q?
- Decision: Adopt Reticulum Network Stack and LXMF as core mesh substrate. Honour the "no-harm" and "no-AI-training" clauses in TOS, AUP, and data-processing architecture.
- Reference: `04_LICENSING_AND_FORK_STRATEGY.md` §4

## D-0004 — First target device: Lite (Elecrow CrowPanel Advance 3.5" HMI)

- Date: 2025-Q?
- Decision: Lite v1 is the first commercial SKU. Pin map derived from Meshtastic firmware `variants/esp32s3/elecrow_panel/` and Elecrow-RD schematic.
- Reference: `01_SS-SP_LITE_HARDWARE_REFERENCE.md`

## D-0005 — Founding security posture

- Date: 2025-Q?
- Decision: Adopt security-model baseline in `05_SECURITY_MODEL.md`. Dual-signed OTA, per-device seed, no key escrow that decrypts user messages, duress PIN with beacon.
- Reference: `05_SECURITY_MODEL.md`
