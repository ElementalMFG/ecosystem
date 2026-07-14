<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# EPIC-18 — Stories

Format per `../../00_METHODOLOGY.md` §2.7. Meta lines are machine-parsed.

### S-18-001 — WAMR vs Wasm3 evaluation spike
As a firmware engineer I want a timeboxed evaluation of WAMR vs Wasm3 so that the sandbox runtime choice is made on measured footprint, speed, and security posture rather than assumption.
- AC: Decision doc benchmarks both runtimes on target hardware for flash/RAM footprint, execution speed, and interpreter-vs-AOT trade-offs; security posture (sandboxing model, CVE history, memory-isolation guarantees) is compared per `05_SECURITY_MODEL.md` criteria; a recommendation with rationale is logged as an ADR and accepted by wg-firmware and wg-security
- Meta: Shard=A | Type=Spike | Size=M | Prio=P1 | Status=DRAFT | SKU=A,O | PRD=F-SEC-11 | Const=C-00, C-05

### S-18-002 — Integrate chosen runtime
As a firmware engineer I want the chosen WASM runtime integrated behind a stable embedding API so that plugins execute in an isolated interpreter with no direct access to firmware memory.
- AC: Runtime loads and executes a test module with linear memory fully isolated from firmware address space (negative tests attempt out-of-bounds access); runtime flash/RAM footprint stays within the budget set in the S-18-001 ADR; a runtime trap (invalid opcode, OOM) terminates only the plugin instance, never the firmware
- Meta: Shard=A | Type=Feature | Size=L | Prio=P1 | Status=DRAFT | SKU=A,O | PRD=F-SEC-11 | Const=C-00, C-05

### S-18-003 — Plugin manifest v1 schema + signer
As a plugin developer I want a v1 manifest schema and signing tool so that my plugin declares its identity, version, and required capabilities in a verifiable way.
- AC: Manifest schema covers id, version, required caps, publisher, and signature fields with a published JSON-schema validator; signer tool produces signatures the device verifies before any plugin code is loaded; a manifest requesting an unknown capability is rejected at install time with a clear error
- Meta: Shard=B | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=A,O | PRD=F-SEC-11, F-CL-03 | Const=C-00, C-05

### S-18-004 — Capability grant model
As a firmware engineer I want a capability-token grant model so that a plugin can touch messages, location, UI, storage, or network only when the user has granted that specific capability.
- AC: Every host function checks a capability token minted at grant time — calls without the token fail closed; grants are per-plugin, persisted, and revocable at runtime with revocation taking effect on the next call; location and message capabilities require explicit user grant and are never default-on
- Meta: Shard=C | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=A,O | PRD=F-SEC-11 | Const=C-00, C-05

### S-18-005 — Host syscall table (narrow)
As a firmware engineer I want a narrow, audited host syscall table so that the sandbox's attack surface is minimal and every crossing is reviewable.
- AC: Syscall surface is limited to the documented set (messaging send/receive, location read, ss_ui screen calls, scoped storage, scoped network) with each entry documented and capability-gated; all syscall inputs are validated and fuzzed from the WASM side without a firmware crash; adding a syscall requires wg-security review recorded in the PR (enforced by CODEOWNERS)
- Meta: Shard=H | Type=Feature | Size=L | Prio=P1 | Status=DRAFT | SKU=A,O | PRD=F-SEC-11 | Const=C-00, C-05

### S-18-006 — Quota enforcement (CPU tick, heap cap, net bytes)
As a firmware engineer I want per-plugin CPU, heap, and network quotas so that a runaway or malicious plugin cannot starve the pager's core functions.
- AC: CPU tick budget, heap cap, and network byte quota are enforced per plugin instance with limits set from the manifest and platform policy; a quota breach terminates the plugin cleanly and reports the reason to the user (epic exit criterion 3); core messaging and SOS latency remain within budget while a plugin runs at its quota limit (stress test)
- Meta: Shard=D | Type=Feature | Size=L | Prio=P1 | Status=DRAFT | SKU=A,O | PRD=F-SEC-11 | Const=C-00, C-05

### S-18-007 — Install / uninstall lifecycle
As a device owner I want reliable plugin install, run, suspend, and uninstall so that I can manage plugins without leftover state or reboots.
- AC: Install verifies manifest signature and capability set before any plugin code executes; suspend/resume preserves plugin state across the lifecycle without leaking resources; uninstall removes code, grants, and quotas atomically — a mid-uninstall power loss leaves no half-installed plugin
- Meta: Shard=E | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=A,O | PRD=F-SEC-11 | Const=C-00, C-05

### S-18-008 — Cloud plugin registry (signing service)
As a plugin developer I want a cloud registry with a signing service so that approved plugins are distributed signed and can be revoked if they turn malicious.
- AC: Registry accepts submissions, runs the review pipeline, and signs approved plugins per F-CL-03; devices verify registry signatures at install and refuse revoked plugin versions on the next sync; registry publishes a revocation feed devices can poll without exposing per-device install lists
- Meta: Shard=F | Type=Feature | Size=L | Prio=P1 | Status=DRAFT | SKU=A,O | PRD=F-CL-03 | Const=C-00, C-05

### S-18-009 — Registry HSM integration
As a firmware engineer I want registry signing keys held in an HSM so that the plugin certification supply chain resists key theft (risk R18-03).
- AC: All registry signing operations execute inside the HSM — private keys are never exportable; signing requests are authenticated, rate-limited, and produce an append-only audit log; a documented key-rotation and compromise-response procedure exists and is tested in a drill
- Meta: Shard=F | Type=Ops | Size=M | Prio=P1 | Status=DRAFT | SKU=A,O | PRD=F-CL-03 | Const=C-05, C-00

### S-18-010 — Plugin review workflow doc
As a plugin developer I want the review workflow documented for each trust tier so that I know exactly what SS-SP-Certified, community-approved, and sideload each require.
- AC: Doc defines criteria, review steps, and turnaround expectations for all three tiers in shard S-18.G; security-review requirements (capability audit, static checks) are specified per tier; doc is published in the developer portal and versioned under governance change control
- Meta: Shard=G | Type=Docs | Size=M | Prio=P1 | Status=DRAFT | SKU=A,O | PRD=F-CL-03 | Const=C-00, C-05, C-06

### S-18-011 — Sideload mode UX (developer)
As a plugin developer I want a developer sideload mode so that I can iterate on unsigned builds on my own device without the registry loop.
- AC: Unsigned plugins refuse to load unless sideload mode is explicitly enabled (epic exit criterion 2); enabling sideload requires a deliberate multi-step confirmation and shows a persistent on-device indicator while active; sideloaded plugins run under the same capability and quota enforcement as signed ones
- Meta: Shard=G | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=A,O | PRD=F-SEC-11 | Const=C-00, C-05

### S-18-012 — Reference plugin: hello-world
As a plugin developer I want a minimal hello-world reference plugin so that I have a known-good starting template for manifest, build, and UI basics.
- AC: Plugin builds from documented toolchain steps and renders a screen via the ss_ui host functions; source demonstrates manifest declaration, one capability request, and clean lifecycle handling; a signed build installs and runs on-device, and the walkthrough is linked from the developer docs
- Meta: Shard=I | Type=Feature | Size=S | Prio=P1 | Status=DRAFT | SKU=A,O | PRD=F-SEC-11 | Const=C-00, C-05

### S-18-013 — Reference plugin: weather
As a plugin developer I want a weather reference plugin so that scoped network access and quota-friendly fetching are demonstrated end-to-end.
- AC: Plugin fetches a forecast via the network capability and renders it with ss_ui list/text widgets; it degrades gracefully (cached/last-known data) when no bearer has Internet reachability; network usage stays within its manifest byte quota in a metered test
- Meta: Shard=I | Type=Feature | Size=M | Prio=P2 | Status=DRAFT | SKU=A,O | PRD=F-SEC-11 | Const=C-00, C-05

### S-18-014 — Reference plugin: RSS reader
As a plugin developer I want an RSS reader reference plugin so that scoped storage plus periodic fetching patterns are demonstrated.
- AC: Plugin fetches, parses, and lists feed items with per-plugin scoped storage for read-state; malformed feed input is handled without crashing the plugin or tripping the sandbox; storage stays within the plugin's quota and is fully removed on uninstall
- Meta: Shard=I | Type=Feature | Size=M | Prio=P2 | Status=DRAFT | SKU=A,O | PRD=F-SEC-11 | Const=C-00, C-05

### S-18-015 — Sandbox external audit
As a device owner I want an independent security audit of the plugin sandbox so that sandbox-escape risk (R18-01) is assessed by someone outside the team before third-party code ships.
- AC: External auditors review the runtime integration, syscall table, and capability/quota enforcement with an escape-attempt test plan; all critical/high findings are fixed or formally risk-accepted before the plugin store opens; audit summary is published per the disclosure norms in `SECURITY.md`
- Meta: Shard=— | Type=Task | Size=L | Prio=P1 | Status=DRAFT | SKU=A,O | PRD=F-SEC-11, NF-SEC-06 | Const=C-05, C-SEC

### S-18-016 — Plugin data-wipe on uninstall
As a device owner I want uninstalling a plugin to wipe all of its data so that nothing a plugin stored or was granted survives its removal.
- AC: Uninstall removes plugin binary, scoped storage, capability grants, and cached state (epic exit criterion 4); a post-uninstall filesystem scan finds no residual plugin-owned data; wipe completes even if the plugin is running or crashed at uninstall time
- Meta: Shard=E | Type=Feature | Size=S | Prio=P1 | Status=DRAFT | SKU=A,O | PRD=F-SEC-11, NF-PRIV | Const=C-00, C-05

### S-18-017 — Plugin permission grant UX on device
As a device owner I want a clear on-device permission prompt when a plugin requests capabilities so that I understand and control what each plugin can access.
- AC: First run presents each requested capability in plain language with per-capability allow/deny; granted permissions are reviewable and revocable from Settings, with revocation effective without reboot; a plugin call to a denied capability fails gracefully with no silent re-prompt loop
- Meta: Shard=C | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=A,O | PRD=F-SEC-11 | Const=C-00, C-05, C-03

### S-18-018 — Video-over-mesh RTSP plugin exemplar
As a plugin developer I want a signed exemplar plugin that relays an RTSP/RTP camera stream across capable mesh links so that video-over-mesh is proven as an extension without ever entering core firmware.
- AC: plugin ingests a standard RTSP/RTP source and relays it to a standard player across a HaLow/Wi-Fi mesh path with documented resolution/bitrate limits; plugin requests only declared capabilities (network, no message-store access) and passes the S-18-017 permission flow; core firmware contains no video code paths — removal of the plugin leaves zero dead code; exemplar source is published under the open plugin SDK licence as a reference for third parties
- Meta: Shard=— | Type=Feature | Size=L | Prio=P3 | Status=DRAFT | SKU=A,O | PRD=F-INT-03 | Const=C-00, C-05

### S-18-019 — Federated messaging bridge gateway (Nostr → XMPP → Matrix, opt-in, not-E2E-labelled)
As an operator I want opt-in gateway plugins that bridge my LXMF messaging to the wider open-messaging world so that I can reach contacts on Nostr, XMPP, or Matrix while the native RNS/LXMF stack stays the first-class experience.
- AC: bridge gateways ship as sandboxed opt-in plugins in priority order Nostr → XMPP → Matrix (simplest-first per the dossier §4.1), each run inside the operator's own trust boundary; every bridge is prominently labelled *not end-to-end* — the crypto boundary ends at the bridge (a plaintext trust point by protocol mismatch) — and the delay-tolerance/opaque-addressing/absent-presence/false-E2E-shield limits are documented, never marketed away; each plugin requests only declared capabilities and passes the S-18-017 permission flow; the native LXMF stack + companion app remain first-class and every bridge is fully removable with zero effect on mesh messaging (never "stock Element on the mesh"); Matrix (heaviest) targets an SBC/gateway homeserver only, never the MCU; the alias table carries `nostr:`/`xmpp:`/`matrix:` addresses for the universal-contact model
- Meta: Shard=— | Type=Feature | Size=XL | Prio=P3 | Status=DRAFT | SKU=A,O | PRD=F-INT-05 | Const=C-04, C-08
- Tasks: spec bridge-gateway plugin contract + not-E2E labelling + universal-contact alias entries · design Nostr-first adapter then XMPP (XEP-0114 component) then Matrix (SBC homeserver) · impl Nostr bridge exemplar under permission flow · test opt-in enable/disable, removability/never-degrade, sandbox capability isolation, plaintext-boundary labelling · docs bridge limits (delay-tolerance, addressing, false-E2E-shield) honestly documented
- Deps: D-0025, `12_TELEPHONY_AND_UNIVERSAL_INTEROP_DOSSIER.md`, S-18-017
