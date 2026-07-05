---
# SPDX-License-Identifier: Apache-2.0
paths:
  - "protocol/**"
---

# Protocol domain facts (verified 2026-07)

- **Everything under `protocol/` is T1** (doc 10 §2): wire bytes freeze at first ship. Never work here on a demoted model/effort; the §10 double review (`t1-review` + `t1-cross-review` agents) is mandatory.
- Tree (all directories are empty scaffolds today — do not assume specs exist yet): `protocol/ss/` native wire spec, `protocol/schemas/`, `protocol/testvectors/` (the contract T2 builds test against), `protocol/foreign/{rns,lxmf,meshtastic}/` interop notes.
- Any wire-format, public-API, or crypto-primitive change requires an **RFC first** (`rfcs/TEMPLATE.md`, process in `06_GOVERNANCE.md` §4) and merges only with 2 CODEOWNER approvals + wg-security sign-off (`CONTRIBUTING.md` §9).
- Specs of record: `02_PROTOCOL_STACK.md` (root) and `05_SECURITY_MODEL.md`. Trace every design decision to one of them or to an RFC.
- **Clean-room rule (legal)**: Meshtastic interop derives from public documentation only, recorded in `protocol/foreign/meshtastic/`. Never open, quote, or paraphrase Meshtastic source or `.proto` files. Reticulum/LXMF interop likewise documents its upstream sources.
- New vectors: deterministic, byte-exact, with provenance comments; a vector set is an irreversible published artifact once released.
