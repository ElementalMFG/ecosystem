<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# EPIC-22 — Testing & Simulation

**Primary WG:** wg-ops, wg-firmware · **Contributing:** wg-security, wg-protocol
**Priority:** P0 · **SKU:** ★ · **Milestone:** M0

## Outcome
End-to-end testing infrastructure: unit tests (host-run), on-target tests (Unity), Hardware-in-Loop rack, protocol fuzzer, mesh simulator (thousands of virtual nodes), chaos & soak tests, and continuously-enforced coverage/mutation gates.

## Constitution
C-00 `00_MASTER_SOFTWARE_PLAN.md` §test strategy.

## Dependencies
EPIC-02.

## Shards
- **S-22.A Unit-test harness** — host gtest + on-target Unity.
- **S-22.B HIL rack architecture** — devices, RF isolation, control PC.
- **S-22.C Mesh simulator** — Rust/Python, thousands of nodes, virtual radios.
- **S-22.D Protocol fuzzer** — AFL/libFuzzer harnesses for LXMF/RNS/SS-Link.
- **S-22.E Chaos suite** — bearer flap, brownout, memory pressure.
- **S-22.F Long-run soak** — 30-day drift/leak tests.
- **S-22.G Coverage & mutation** — kcov + Stryker/Mull.
- **S-22.H Field-trial framework** — real-world logs, opt-in telemetry (redacted).
- **S-22.I CI wiring — everything above runs in CI.**

## Exit criteria
1. `make test` runs all unit tests in < 10 min.
2. HIL rack executes nightly and posts results.
3. Simulator can run 5 000 virtual nodes on a single dev machine.
4. Fuzzer runs 24 h with zero new crashes on ACCEPTED code.
5. Coverage ≥ 80 % line, ≥ 60 % branch on `main`.

## Risks
| # | Risk | Mitigation |
|---|------|-----------|
| R22-01 | Flaky HIL tests | Retry policy + isolated racks |
| R22-02 | Simulator drift from real radios | Regular calibration |
| R22-03 | Coverage gaming | Mutation on security-critical code |
