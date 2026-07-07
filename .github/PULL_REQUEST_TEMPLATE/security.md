<!-- SPDX-License-Identifier: CC-BY-4.0 -->
<!-- Security PR -->
<!-- WARNING: Do NOT include exploit details or undisclosed vulnerability
     information in a public PR. Coordinate disclosure via SECURITY.md first. -->
## Summary

<!-- What and why, 1-3 sentences. No exploit detail. -->

## Portfolio traceability

- Story ID(s): <!-- S-NN-MMM; also add a `Story: S-NN-MMM` commit trailer (CONTRIBUTING.md §8) -->
- RFC (if wire-format/public-API/crypto/governance): <!-- rfcs/… or n/a -->

## Threat / issue addressed

<!-- Reference the advisory / report ID. No exploit detail in a public PR. -->

## Security-relevant testing

<!-- Vectors, fuzzing, constant-time checks as applicable. -->

## Required sign-offs

- [ ] DCO sign-off on every commit (`git commit -s`, CONTRIBUTING.md §4)
- [ ] CLA signed on first PR (CONTRIBUTING.md §5)
- [ ] 2 CODEOWNER approvals + wg-security sign-off (CONTRIBUTING.md §9, mandatory for security-sensitive paths)
- [ ] Disclosure timing follows SECURITY.md

## Checklist

- [ ] Every commit is signed off (`git commit -s`, DCO — CONTRIBUTING.md §4)
- [ ] Commit messages follow Conventional Commits (CONTRIBUTING.md §8)
- [ ] Tests added/updated
- [ ] Docs updated (including `docs/CHANGELOG.md` — CONTRIBUTING.md §3)
- [ ] New files carry SPDX + copyright headers (CONTRIBUTING.md §6)
- [ ] `firmware/**` touched → `make lite` passes locally
- [ ] `**/*.md` touched → `python3 tools/lint-docs.py` passes
- [ ] `docs/portfolio/epics/**` touched → `python3 tools/gen-stories-index.py --check` passes
- [ ] AI assistance disclosed below if used (CONTRIBUTING.md §11)

## AI assistance disclosure

<!-- Tool(s) used, or "none". -->
