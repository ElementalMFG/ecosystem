<!-- SPDX-License-Identifier: CC-BY-4.0 -->
<!-- Docs-only PR -->
## Summary

<!-- What and why, 1-3 sentences. -->

## Portfolio traceability

- Story ID(s): <!-- S-NN-MMM; also add a `Story: S-NN-MMM` commit trailer (CONTRIBUTING.md §8) -->
- RFC (if wire-format/public-API/crypto/governance): <!-- rfcs/… or n/a -->

## Required sign-offs

- [ ] DCO sign-off on every commit (`git commit -s`, CONTRIBUTING.md §4)
- [ ] CLA signed on first PR (CONTRIBUTING.md §5)
- [ ] 1 CODEOWNER approval (CONTRIBUTING.md §9)

## Checklist

- [ ] Every commit is signed off (`git commit -s`, DCO — CONTRIBUTING.md §4)
- [ ] Commit messages follow Conventional Commits (CONTRIBUTING.md §8)
- [ ] `python3 tools/lint-docs.py` passes after `git add`
- [ ] `docs/portfolio/epics/**` touched → `python3 tools/gen-stories-index.py --check` passes
- [ ] `docs/CHANGELOG.md` updated if user-visible (CONTRIBUTING.md §3)
- [ ] New files carry SPDX + copyright headers (CONTRIBUTING.md §6)
- [ ] AI assistance disclosed below if used (CONTRIBUTING.md §11)

<!-- Note: docs-only PRs still report the `build (lite)` required check via the
     always-run gate job (no firmware build runs). -->

## AI assistance disclosure

<!-- Tool(s) used, or "none". -->
