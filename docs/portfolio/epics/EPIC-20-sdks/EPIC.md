<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# EPIC-20 — SDKs (C, Rust, Python, TypeScript, Dart)

**Primary WG:** wg-sdk · **Contributing:** wg-protocol, wg-docs
**Priority:** P1 · **SKU:** ★ · **Milestone:** M6

## Outcome
Public SDKs let third-party developers speak SS-SP wire protocols (SS-Link, RNS, LXMF) from five languages, build plugins for the device sandbox, and integrate with Fleet Console/Relay APIs. Includes a wire testkit that generates cross-lang conformance suites.

## Constitution
C-00 `00_MASTER_SOFTWARE_PLAN.md` §SDK; C-02 `02_PROTOCOL_STACK.md` §wire conformance.

## Dependencies
EPIC-10, EPIC-11, EPIC-12, EPIC-18.

## Shards
- **S-20.A C SDK** — thin binding of firmware protocol clients.
- **S-20.B Rust SDK** — idiomatic, tokio.
- **S-20.C Python SDK** — pip, asyncio.
- **S-20.D TypeScript SDK** — npm, browser + node.
- **S-20.E Dart SDK** — pub, Flutter-friendly.
- **S-20.F Wire testkit** — YAML test-vectors + generators.
- **S-20.G Plugin dev kit** — WASM toolchain, manifest signer, emulator.
- **S-20.H Cloud API client libs** — Fleet Console REST/gRPC.
- **S-20.I SDK docs site** — one per lang.

## Exit criteria
1. `npm i @ss-sp/sdk` publishes a passing hello-world.
2. `pip install ss-sp` publishes.
3. Wire testkit vectors pass in all 5 languages.
4. Plugin dev-kit builds and signs a plugin locally in ≤ 5 min.

## Risks
| # | Risk | Mitigation |
|---|------|-----------|
| R20-01 | Cross-lang drift | Shared wire testkit conformance |
| R20-02 | SDK ownership burnout | Community maintainer model |
| R20-03 | ABI churn breaks pinned users | Semver + long-term support branches |
