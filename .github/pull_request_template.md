<!-- SPDX-License-Identifier: CC-BY-4.0 -->
## Summary

<!-- What and why, 1-3 sentences. -->

## Portfolio traceability

- Story ID(s): <!-- S-NN-MMM; also add a `Story: S-NN-MMM` commit trailer (CONTRIBUTING.md §8) -->
- RFC (if wire-format/public-API/crypto/governance): <!-- rfcs/… or n/a -->

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

## Security-sensitive paths

<!-- Does this touch ss_crypto/ss_hal/bootloader/protocol/ota/provisioning/
     firmware/security? If yes: 2 CODEOWNER approvals + wg-security sign-off
     required (CONTRIBUTING.md §9). -->

- [ ] No security-sensitive paths touched, **or** wg-security sign-off requested

## AI assistance disclosure

<!-- Tool(s) used, or "none". -->
