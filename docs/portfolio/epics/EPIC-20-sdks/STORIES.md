<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# EPIC-20 — Stories

Format per `../../00_METHODOLOGY.md` §2.7. Meta lines are machine-parsed.

### S-20-001 — C SDK skeleton + CMake
As an SDK consumer I want a C SDK skeleton with proper CMake packaging so that embedded projects can link SS-SP protocol clients via `find_package`.
- AC: builds with GCC and Clang on Linux and cross-compiles for ARM in CI; installs CMake config and pkg-config files; a hello-world example compiles against the installed package
- Meta: Shard=A | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-SDK-01 | Const=C-00

### S-20-002 — C SDK LXMF client
As an SDK consumer I want an LXMF client in the C SDK so that constrained embedded devices can send and receive SS-SP messages.
- AC: sends and receives an LXMF message against a reference node in an integration test; passes all wire-testkit LXMF vectors; no dynamic allocation outside documented init paths, suitable for embedded targets
- Meta: Shard=A | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-SDK-01 | Const=C-00, C-02

### S-20-003 — Rust SDK skeleton + tokio
As an SDK consumer I want a Rust crate skeleton with an async tokio API so that high-assurance clients get an idiomatic foundation.
- AC: crate compiles on stable Rust with docs.rs-buildable metadata; async API contains no blocking calls in async paths (enforced by clippy lints and tests); example compiles in CI
- Meta: Shard=B | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-SDK-02 | Const=C-00

### S-20-004 — Rust SDK RNS client
As an SDK consumer I want an RNS client in the Rust SDK so that Rust applications can establish Reticulum links and transport packets.
- AC: establishes an RNS link and exchanges packets with a reference Reticulum node in integration tests; passes all wire-testkit RNS vectors; the pinned upstream Reticulum interop version is documented
- Meta: Shard=B | Type=Feature | Size=L | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-SDK-02 | Const=C-00, C-02

### S-20-005 — Python SDK skeleton + asyncio
As an SDK consumer I want a pip-installable Python SDK skeleton with an asyncio API so that scripts and tools can integrate quickly.
- AC: `pip install ss-sp` works from the built wheel (epic exit criterion 2); API is fully type-hinted and mypy-clean; CI tests run on all supported Python versions
- Meta: Shard=C | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-SDK-03 | Const=C-00

### S-20-006 — Python SDK LXMF client
As an SDK consumer I want an LXMF client in the Python SDK so that scripted tooling can send and receive SS-SP messages.
- AC: LXMF send/receive works against a reference node in integration tests; passes all wire-testkit LXMF vectors; a runnable example script ships in the docs
- Meta: Shard=C | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-SDK-03 | Const=C-00, C-02

### S-20-007 — TypeScript SDK skeleton (npm)
As an SDK consumer I want an npm-published TypeScript SDK skeleton so that web and Node developers get typed SS-SP access.
- AC: `npm i @ss-sp/sdk` hello-world passes (epic exit criterion 1); package ships ESM and CJS builds with published type declarations; unit tests run on Node LTS in CI
- Meta: Shard=D | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-SDK-05 | Const=C-00

### S-20-008 — TypeScript SDK browser build (WebBLE)
As an SDK consumer I want a browser build of the TypeScript SDK using Web-BLE so that web apps can talk to a SS-SP device directly.
- AC: connects to a device over Web-BLE from a Chromium browser in test; browser bundle requires no Node polyfills; Web-BLE availability is feature-detected and reported through a documented capability API
- Meta: Shard=D | Type=Feature | Size=L | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-SDK-05 | Const=C-00

### S-20-009 — Dart SDK skeleton
As an SDK consumer I want a pub.dev-ready Dart SDK skeleton so that Flutter developers can build SS-SP apps.
- AC: `dart pub publish --dry-run` passes clean; API is null-safe; unit tests run on stable Dart in CI
- Meta: Shard=E | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-SDK-04 | Const=C-00

### S-20-010 — Dart SDK Flutter integration example
As an SDK consumer I want a Flutter example app using the Dart SDK so that mobile developers have a working starting point.
- AC: example app pairs with a device and sends a message; runs on both iOS and Android; the example is built in CI so it never rots
- Meta: Shard=E | Type=Feature | Size=M | Prio=P2 | Status=DRAFT | SKU=★ | PRD=F-SDK-04 | Const=C-00

### S-20-011 — Wire testkit YAML vector format
As an SDK maintainer I want a YAML test-vector format covering SS-Link/RNS/LXMF frames so that all five SDKs prove wire conformance against one source of truth.
- AC: vector schema is documented and schema-validated in CI; vectors cover framing, crypto handshake, and error cases for SS-Link, RNS, and LXMF; vectors are shared with the firmware test-vector library (EPIC-22 S-22-021)
- Meta: Shard=F | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-SDK-01, F-SDK-02, F-SDK-03, F-SDK-04, F-SDK-05 | Const=C-00, C-02

### S-20-012 — Testkit runners per language
As an SDK maintainer I want a testkit runner in each of C, Rust, Python, TypeScript, and Dart so that cross-language drift (risk R20-01) is caught automatically.
- AC: runners in all five languages execute the full vector set and pass (epic exit criterion 3); a failing vector reports vector ID plus expected/actual diff; runners are wired into each SDK's CI as a required check
- Meta: Shard=F | Type=Feature | Size=L | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-SDK-01, F-SDK-02, F-SDK-03, F-SDK-04, F-SDK-05 | Const=C-00, C-02

### S-20-013 — Plugin dev-kit CLI (`ss-plugin new`)
As a plugin developer I want an `ss-plugin new` CLI that scaffolds, builds, and signs a WASM plugin so that going from idea to installable plugin takes minutes.
- AC: `ss-plugin new` scaffolds a buildable WASM plugin with a valid manifest; manifest signer signs with a dev key and validates the manifest schema; scaffold-to-signed-artifact completes in ≤ 5 min (epic exit criterion 4)
- Meta: Shard=G | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-SEC-11 | Const=C-00, C-05

### S-20-014 — Plugin dev-kit emulator
As a plugin developer I want a local sandbox emulator so that I can test capability-token behaviour without device hardware.
- AC: emulator runs a WASM plugin under the same capability-token model as the on-device sandbox; capability violations are rejected identically to device behaviour, verified against a shared conformance suite; an example plugin's test suite runs in the emulator in CI
- Meta: Shard=G | Type=Feature | Size=L | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-SEC-11 | Const=C-00, C-05

### S-20-015 — Fleet Console REST client (TS, Py)
As a fleet admin I want TypeScript and Python client libraries for the Fleet Console API so that fleet automation scripts do not hand-roll HTTP calls.
- AC: TS and Python clients are generated from / verified against the Fleet Console OpenAPI spec in CI so they cannot drift; tenant-token authentication is supported; a smoke test passes against a staging console
- Meta: Shard=H | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-CL-01 | Const=C-00

### S-20-016 — SDK docs site scaffolding (mkdocs / docusaurus)
As an SDK consumer I want a docs site with a section per language so that I can find an install guide and quick-start for my stack.
- AC: site builds in CI with versioned docs; every language has an install plus hello-world quick-start; API reference is auto-generated from source on each release
- Meta: Shard=I | Type=Docs | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=— | Const=C-00

### S-20-017 — Semver + LTS branch policy
As a release manager I want a published semver and LTS-branch policy for all SDKs so that pinned users are not broken by ABI churn (risk R20-03).
- AC: policy document defines breaking-change rules, LTS branch lifetime, and deprecation windows; a CI check blocks API-breaking changes that lack a major-version bump; the policy is linked from every SDK README
- Meta: Shard=— | Type=Task | Size=S | Prio=P1 | Status=DRAFT | SKU=★ | PRD=— | Const=C-00, C-06
- Deps: RFC-0003 (compat/deprecation policy — SDK deprecation windows inherit from it, not restated here)

### S-20-018 — Publish first releases to npm/pypi/crates.io/pub.dev
As a release manager I want first public releases on npm, PyPI, crates.io, and pub.dev so that developers can install the SDKs from standard registries.
- AC: packages publish to all four registries via the release pipeline with signed provenance; installation from each public registry is verified in a clean container; the publish process is documented and repeatable by a second person
- Meta: Shard=— | Type=Ops | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-SDK-01, F-SDK-02, F-SDK-03, F-SDK-04, F-SDK-05 | Const=C-00, C-06
