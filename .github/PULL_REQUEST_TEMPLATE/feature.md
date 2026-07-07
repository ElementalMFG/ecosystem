<!-- SPDX-License-Identifier: CC-BY-4.0 -->
<!-- Feature / fix PR -->
## Summary

<!-- What and why, 1-3 sentences. -->

## Portfolio traceability

- Story ID(s): <!-- S-NN-MMM; also add a `Story: S-NN-MMM` commit trailer (CONTRIBUTING.md §8) -->
- RFC (if wire-format/public-API/crypto/governance): <!-- rfcs/… or n/a -->

## Testing

<!-- How was this tested? Include `make lite` / pinned container build output
     if `firmware/**` is touched (docs/dev/BUILDING.md). -->

## Required sign-offs

- [ ] DCO sign-off on every commit (`git commit -s`, CONTRIBUTING.md §4)
- [ ] CLA signed on first PR (CONTRIBUTING.md §5)
- [ ] At least 1 CODEOWNER approval (CONTRIBUTING.md §9)
- [ ] If any security-sensitive path is touched (`components/ss_hal/**`, `firmware/security/**`, `bootloader/**`, `protocol/**`, `ota/**`, `provisioning/**`, `components/ss_crypto/**`): 2 CODEOWNER approvals + wg-security sign-off — consider using the security template instead
- [ ] Squash-merge is the default (CONTRIBUTING.md §8)

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
