# Contributing to SS-SP

Thank you for your interest in contributing. SS-SP is a governance-driven, standards-track project; the rules below exist to keep the codebase legally safe, secure, and welcoming.

Please also read: [`CODE_OF_CONDUCT.md`](./CODE_OF_CONDUCT.md), [`SECURITY.md`](./SECURITY.md), and [`06_GOVERNANCE.md`](./06_GOVERNANCE.md).

---

## 0. TL;DR

- Every commit: `git commit -s` (DCO sign-off).
- First PR: sign the CLA (bot will prompt you).
- Every file: `// SPDX-License-Identifier: Apache-2.0` on line 1 or 2.
- Do not paste GPL/AGPL/SSPL/proprietary code into the repo. See `04_LICENSING_AND_FORK_STRATEGY.md`.
- Wire-format or public-API changes: open an [RFC](./rfcs/).
- Security-sensitive paths need 2 reviewer approvals + wg-security sign-off.
- Follow the coding standard for the language you touch.

---

## 1. Getting your environment ready

See [`docs/dev/BUILDING.md`](./docs/dev/BUILDING.md) for full build instructions.

Quick start (Lite firmware):

```bash
# ESP-IDF v5.3+
. $IDF_PATH/export.sh
idf.py -B build/lite -DBOARD=lite build
```

Companion apps: see `companion/README.md`.
Cloud: see `cloud/README.md`.

## 2. Filing an issue

- Search first — duplicates get closed.
- Use the appropriate template (`bug`, `feature`, `security`, `docs`, `rfc-request`).
- Include: board, firmware version, reproduction, expected vs actual.
- Do NOT file security issues publicly. See [`SECURITY.md`](./SECURITY.md).

## 3. Making a change

1. Fork or branch. Branch names: `feat/…`, `fix/…`, `docs/…`, `security/…`, `chore/…`.
2. Add tests.
3. Add docs (update `docs/` and inline).
4. Update the changelog under `docs/CHANGELOG.md`.
5. Push and open a PR against `main`.

## 4. DCO — required on every commit

Add `Signed-off-by: Your Name <your@email>` to every commit message. `git commit -s` does this automatically. By signing, you certify the [Developer Certificate of Origin 1.1](https://developercertificate.org).

The `dco` CI check (`.github/workflows/dco.yml`) fails any PR containing an unsigned commit, and it is a required status check on `main` — unsigned commits cannot merge.

**Fixing an unsigned commit** (the check told you which ones):

```bash
# Only the latest commit missing?
git commit --amend -s --no-edit && git push --force-with-lease

# Several commits in the branch missing? Re-sign them all:
git rebase --signoff origin/main && git push --force-with-lease
```

`--force-with-lease` is safe here because you are only rewriting your own PR branch, never `main`.

## 5. CLA — required on your first PR

We use an Apache-style Individual/Corporate CLA. A bot will prompt you on your first PR. This CLA:

- Does **not** transfer copyright to us; you retain it.
- Grants a broad license and a patent grant.
- Ensures the project can transition to a foundation later without asking each contributor again.

## 6. License hygiene

- New files: `// SPDX-License-Identifier: Apache-2.0` + `// Copyright (c) 2025 SS-SP Project Contributors` at the top.
- New dependencies must be on the pre-approved license allowlist (see `04_LICENSING_AND_FORK_STRATEGY.md` §2). Adding a dep with a new license requires a governance-review issue.
- Never copy code from GPL/AGPL/SSPL/proprietary projects.
- Never copy from the Meshtastic firmware or `.proto` files. Use only public documentation of the wire format for clean-room compat work (`protocol/foreign/meshtastic/`).

## 7. Coding standards

- **Firmware C**: `clang-format` (config in `.clang-format`), `-Wall -Wextra -Wshadow -Wconversion -Werror`, MISRA-C:2012 where practical.
- **Rust**: `rustfmt`, `clippy -- -D warnings`.
- **TypeScript / Dart**: project linters, strict mode.
- **Python** (tools only): `ruff`, type-hinted, tested.
- Every public function has a docstring/comment stating pre-conditions, post-conditions, and error behavior.

## 8. Commit style

Conventional Commits:

```
feat(ss_ui): add round-shape template
fix(hal/lite): correct GT911 INT edge polarity
security(ss_crypto): switch to constant-time ratchet lookup
breaking(protocol): drop unused sos.rate_hint field (see rfc-0012)
```

Keep commits small and focused. Squash-merge is default; multi-commit merges are allowed for RFC-driven changes with meaningful history.

## 9. Reviews

- Every PR needs at least 1 CODEOWNER approval.
- Security-sensitive paths (`components/hal/**`, `firmware/security/**`, `bootloader/**`, `protocol/**`, `ota/**`, `provisioning/**`, `components/ss_crypto/**`) require **2 CODEOWNER approvals + wg-security sign-off**.
- Reviewers: focus on correctness, safety, license, tests, docs — in that order.
- Maintainers: never merge your own PR unless it is a trivial doc/typo fix.

## 10. RFC process

Wire-format changes, breaking API changes, cryptographic-primitive changes, or governance changes require an RFC. See `rfcs/README.md` and `06_GOVERNANCE.md` §4.

## 11. AI-assisted contributions

If you use AI code assistants:

- Disclose the tool used in the PR description.
- Confirm the output is not verbatim from a copyleft training-set memorized snippet.
- You remain responsible for correctness, safety, and license compliance.

## 12. Getting help

- Chat: (to be published) Matrix room `#ss-sp:matrix.org`, IRC `#ss-sp` on Libera.
- Forum: (to be published) `discuss.ss-sp.org`.
- Mailing list: `dev@lists.ss-sp.org`.

We are a friendly and neurodivergent-welcoming project. Ask questions; there are no stupid ones.
