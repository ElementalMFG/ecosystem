<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# EPIC-22 — Stories

Format per `../../00_METHODOLOGY.md` §2.7. Meta lines are machine-parsed.

### S-22-001 — Host gtest harness in monorepo
As a firmware developer I want a host-run gtest harness in the monorepo so that unit tests execute fast without any device hardware.
- AC: `make test` runs the full host unit-test suite in < 10 min (epic exit criterion 1); firmware modules build against host HAL mocks; test failures annotate PRs with file and line
- Meta: Shard=A | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-00

### S-22-002 — On-target Unity harness
As a firmware developer I want a Unity test harness running on real hardware so that HAL and timing behaviour is validated where mocks cannot reach.
- AC: Unity tests flash and execute on a Lite dev board with results streamed back to the host; a selected subset runs in the nightly HIL job; conventions shared with the host harness (naming, assertions) are documented
- Meta: Shard=A | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-00

### S-22-003 — HIL rack design + BoM
As an SRE I want a Hardware-in-Loop rack design with a priced bill of materials so that a second rack can be built from the document alone.
- AC: design covers ≥ 8 device slots with RF isolation, per-slot power control, and USB/JTAG access; BoM is fully priced and orderable; a reviewer not on the design team confirms the rack is reproducible from the doc
- Meta: Shard=B | Type=Task | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-00

### S-22-004 — HIL orchestrator (Python)
As an SRE I want a Python orchestrator scheduling jobs onto the HIL rack so that on-target tests run unattended every night.
- AC: orchestrator flashes targets, runs suites, and collects logs and results per slot; a retry/quarantine policy contains flaky tests (risk R22-01); results are posted to CI nightly (supports epic exit criterion 2)
- Meta: Shard=B | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-00

### S-22-005 — RF-isolated chamber inventory
As an SRE I want the RF-isolated chambers inventoried and characterised so that radio tests are repeatable and simulator calibration has ground truth.
- AC: each chamber's attenuation is measured and recorded; chamber/slot assignments are tracked in the rack inventory; a calibration schedule is defined to counter simulator drift (risk R22-02)
- Meta: Shard=B | Type=Task | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-00

### S-22-006 — Mesh simulator core (Rust)
As a firmware developer I want a Rust mesh-simulator core hosting thousands of virtual nodes so that protocol behaviour is testable at scales no lab rack can reach.
- AC: simulator instantiates virtual nodes with pluggable radio models; seeded runs are deterministic and reproduce identical traces; node lifecycle and scenarios are scriptable via Python bindings
- Meta: Shard=C | Type=Feature | Size=XL | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-00

### S-22-007 — Simulator virtual LoRa radio
As a firmware developer I want a virtual LoRa radio model in the simulator so that mesh routing behaves realistically under contention and duty-cycle limits.
- AC: model implements path loss, collisions, and EU 868 duty-cycle limiting; model output is validated against real-device RF measurements within a documented tolerance; Meshtastic-compat framing traffic runs through the model
- Meta: Shard=C | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=NF-REG-05 | Const=C-00

### S-22-008 — Simulator virtual HaLow radio
As a firmware developer I want a virtual HaLow radio model in the simulator so that Alpha/Omega bearer behaviour can be tested before hardware is plentiful.
- AC: model implements HaLow range/rate trade-offs and association behaviour; validated against real MM8108 measurements within documented tolerance; bearer-selection scenarios run mixed LoRa + HaLow topologies
- Meta: Shard=C | Type=Feature | Size=L | Prio=P1 | Status=DRAFT | SKU=★ | PRD=— | Const=C-00

### S-22-009 — LXMF fuzzer harness
As a firmware developer I want an AFL/libFuzzer harness over LXMF parse and assembly paths so that malformed messages cannot crash or exploit the device.
- AC: harness covers LXMF parsing and reassembly entry points with corpus persisted between runs; runs continuously in CI; a 24 h run with zero new crashes is required on ACCEPTED code (epic exit criterion 4)
- Meta: Shard=D | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-00, C-02

### S-22-010 — RNS fuzzer harness
As a firmware developer I want a fuzzer harness over RNS packet handling so that hostile mesh traffic cannot compromise the transport layer.
- AC: harness covers RNS announce, link, and packet-parse paths with corpus persisted; runs continuously in CI; 24 h zero-new-crash bar enforced on ACCEPTED code
- Meta: Shard=D | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-00, C-02

### S-22-011 — SS-Link fuzzer harness
As a firmware developer I want a fuzzer harness over SS-Link framing so that bearer-layer parsing is hardened against arbitrary radio input.
- AC: harness covers SS-Link frame parse and bearer-negotiation paths with corpus persisted; runs continuously in CI; 24 h zero-new-crash bar enforced on ACCEPTED code
- Meta: Shard=D | Type=Feature | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-00, C-02

### S-22-012 — Chaos bearer-flap test
As a firmware developer I want a chaos test that flaps Wi-Fi/LoRa/BLE bearers under live traffic so that bearer failover is proven, not assumed.
- AC: bearers are flapped in randomized sequences under message load; bearer re-selection completes within the 3 s NF-PERF-02 budget in every iteration; queued messages survive the flaps with zero loss; D-0021: per-SKU bearer set: L=Wi-Fi/LoRa/BLE, A/O=HaLow/Wi-Fi/BLE
- Meta: Shard=E | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=NF-PERF-02 | Const=C-00

### S-22-013 — Chaos brownout test
As a firmware developer I want brownout injection during flash writes and radio TX so that power loss can never corrupt device state.
- AC: brownouts injected at randomized points during flash writes and radio transmission; device recovers to a functional state with no filesystem or message-store corruption; key material and duress state verify intact after recovery
- Meta: Shard=E | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-00

### S-22-014 — Long-run 30-day soak test
As an SRE I want a 30-day soak test on the HIL rack so that slow leaks and clock/battery drift are caught before customers find them.
- AC: soak runs 30 days with periodic messaging, OTA, and sleep/wake cycles; memory-leak and drift thresholds alert automatically during the run; a per-run report is archived and compared against the previous run
- Meta: Shard=F | Type=Ops | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-00

### S-22-015 — kcov integration
As a release manager I want kcov coverage collection wired into the host test suite so that the coverage floor is measurable per PR.
- AC: line and branch coverage collected on every PR and published as a check; main-branch trend tracked against the ≥ 80 % line / ≥ 60 % branch bar (epic exit criterion 5); uncovered-line annotations appear in PR review
- Meta: Shard=G | Type=Ops | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-00

### S-22-016 — Mutation testing (Stryker/Mull) for crypto
As a release manager I want mutation testing over security-critical modules so that coverage numbers cannot be gamed on code that matters (risk R22-03).
- AC: Stryker/Mull runs over the wg-security-maintained list of security-critical modules; a surviving-mutant threshold is enforced per methodology DoD; mutation reports attach to security-relevant PRs
- Meta: Shard=G | Type=Ops | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-00, C-05

### S-22-017 — Field-trial log capture (opt-in, redacted)
As a firmware developer I want opt-in, redacted field-trial log capture so that real-world mesh behaviour informs tuning without violating user privacy.
- AC: capture is opt-in and off by default per NF-PRIV-01; message content and precise location are redacted on-device before anything leaves it (NF-PRIV-02), verified by test; the log format is versioned and parseable by the analysis tooling
- Meta: Shard=H | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=NF-PRIV-01, NF-PRIV-02 | Const=C-00

### S-22-018 — CI nightly job (HIL + simulator)
As an SRE I want a nightly CI job running the HIL suite and simulator scenarios so that regressions surface within one day (epic exit criterion 2).
- AC: nightly job executes HIL and simulator suites and posts results to the team channel and dashboard; failures auto-file issues with logs attached; total job duration stays within a documented budget
- Meta: Shard=I | Type=Ops | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-00, C-06

### S-22-019 — Coverage & mutation gate on PRs
As a release manager I want coverage and mutation floors enforced as PR gates so that quality bars are policy, not convention.
- AC: PRs below the coverage or mutation floor are blocked from merge; floor values are configured centrally and changes require wg-ops review; gate status is visible as a required PR check
- Meta: Shard=I | Type=Ops | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-00, C-06

### S-22-020 — Simulator scaling to 5 000 nodes on single box
As a firmware developer I want the simulator profiled and optimised to 5 000 virtual nodes on one dev machine so that city-scale mesh scenarios are testable by anyone (epic exit criterion 3).
- AC: 5 000-node scenario runs on a single reference dev machine within documented CPU/memory budget; per-node resource cost is profiled and published; a scaling regression test in CI fails if the budget regresses
- Meta: Shard=C | Type=Ops | Size=L | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-00

### S-22-021 — Test-vector library (shared with SDKs)
As a firmware developer I want a single versioned test-vector library shared between firmware tests and the SDK wire testkit so that firmware and SDKs cannot silently diverge on the wire.
- AC: vectors live in one repo location consumed by both firmware suites and the SDK testkit (EPIC-20 S-20-011); adding a vector automatically propagates to all runners; vector coverage includes framing, crypto handshake, and error paths
- Meta: Shard=— | Type=Feature | Size=M | Prio=P0 | Status=DRAFT | SKU=★ | PRD=— | Const=C-00, C-02

### S-22-022 — LoRa TX latency benchmark (NF-PERF-03)
As a firmware engineer I want an automated queue-to-on-air latency benchmark for LoRa so that the < 250 ms non-SOS budget is continuously enforced.
- AC: hardware-in-loop rig measures enqueue → preamble-start latency across payload sizes and duty-cycle states; CI perf job fails when P95 exceeds 250 ms outside SOS priority; results are trended per commit so regressions are attributable; D-0021: HaLow-equivalent benchmark for A/O at elaboration
- Meta: Shard=— | Type=Ops | Size=S | Prio=P1 | Status=DRAFT | SKU=L | PRD=NF-PERF-03 | Const=C-00, C-08

### S-22-023 — Indefinite-offline soak test (NF-REL-01)
As a firmware engineer I want a long-duration cloudless soak test so that field devices are proven to operate indefinitely without cloud contact.
- AC: a device farm runs ≥ 30 days with all cloud endpoints blackholed while exercising messaging, voice notes, OTA-deferral, and key rotation on-mesh; zero feature degradation attributable to missing cloud (no license checks, no expiring tokens, no forced phone-home) is asserted; clock-drift and certificate-expiry edge cases are covered in the scenario matrix
- Meta: Shard=— | Type=Ops | Size=M | Prio=P1 | Status=DRAFT | SKU=★ | PRD=NF-REL-01 | Const=C-00, C-05

### S-22-024 — Mega-scale mesh simulation (1 M nodes)
As a network architect I want a ≥ 1 M-node mixed-bearer simulation so that every "millions of users" claim is backed by reproducible evidence instead of hope.
- AC: the simulator models ≥ 1 M concurrent nodes across realistic bearer mixes (LoRa leaf regions, HaLow neighbourhoods, internet transports, app nodes) with announce propagation, path-table dynamics, and LXMF store-and-forward; runs assert no protocol-level collapse — routing convergence, bounded announce load per NF-SCALE-02, and ≥ 99 % delivery for in-coverage peers per NF-SCALE-01; scale gates are re-runnable per order of magnitude (G1 1 k → G3 1 M) with results published openly so the community can reproduce them; failures produce actionable per-choke-point diagnostics (announce load, relay saturation, propagation quota exhaustion)
- Meta: Shard=— | Type=Ops | Size=XL | Prio=P1 | Status=DRAFT | SKU=★ | PRD=NF-SCALE-01, NF-SCALE-02 | Const=C-00, C-08

### S-22-025 — Public conformance suite & versioned capability profiles
As an ecosystem partner I want a publicly runnable conformance suite tied to versioned capability profiles so that "SS-SP-Certified" means provable interoperability instead of paper compliance — the inversion of Matter's fragmentation failure (`06_COMPETITIVE_LANDSCAPE.md` §4.2).
- AC: capability profiles (e.g., core, voice, gateway) are defined as versioned, additive sets of testable behaviors ratified through the RFC process; the suite exercises wire-level RNS/LXMF behavior, profile advertisement/negotiation, and cross-version coexistence (a core-v1 node must fully interoperate on a network of newer nodes); the suite is open-source, runnable by anyone against real hardware or the simulator, and is the executable definition of the F-CERT-01 badge — certification requires a full pass with results archived; partial exposure of a claimed profile is an automatic failure (anti lowest-common-denominator rule); official firmware, companion apps, the headless daemon, and the OpenWrt package all pass in CI on every release
- Meta: Shard=— | Type=Feature | Size=L | Prio=P1 | Status=DRAFT | SKU=★ | PRD=F-CERT-01, F-CERT-02 | Const=C-02, C-OA, C-07

### S-22-026 — Cross-vendor HaLow interop bench (Morse + Newracom reference targets)
As a QA engineer I want a cross-vendor HaLow interoperability bench in the HIL rack so that SS-SP devices verifiably associate and exchange traffic with the certified third-party HaLow ecosystem on every release.
- AC: bench includes a Morse Micro HaLowLink 2 (MM8108) and one Newracom NRC7394-based reference AP; per-release automated runs cover STA association to third-party APs, third-party STA association to SS-SP AP/HGW mode, WPA3-SAE handshake, and 802.11s mesh peering where supported, each with documented throughput floors; regressions block release per the release-gate policy; TaiXin TXW8301 interop is explicitly out of scope (non-certified, per D-HALOW-06 in `docs/portfolio/08_HALOW_TECHNOLOGY_DOSSIER.md`); results feed the S-22-008 simulator model validation
- Meta: Shard=— | Type=Ops | Size=M | Prio=P1 | Status=DRAFT | SKU=A,O | PRD=NF-REL-04 | Const=C-08

### S-22-027 — On-target test harness on ss-sp omega (ESP32-P4, v69)
As a test engineer I want the on-target Unity harness running on the Omega v69 board so that HAL stories S-05-021..039 verify on real hardware, not only the Lite dev board.
- AC: harness boots + runs the existing suite on P4/v69 · per-story HAL test groups selectable · results land in the same CI artifact format as the Lite harness · flash/monitor flow documented for the v69 board
- Meta: Shard=A | Type=Feature | Size=M | Prio=P1 | Status=DRAFT | SKU=O | PRD=— | Const=C-00
