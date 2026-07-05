<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# Changelog

All notable changes to SS-SP. Format: [Keep a Changelog](https://keepachangelog.com/en/1.1.0/);
versioning: SemVer per released artifact once releases begin. Update this file
in every user-visible PR (CONTRIBUTING.md §3).

## [Unreleased]

### Added

- Constitution set docs 00–08, methodology + portfolio (24 epics / 533
  stories), model-allocation strategy (doc 10), venture execution map (doc 09).
- Governance baseline: CODE_OF_CONDUCT, SECURITY.md, CONTRIBUTING with DCO
  gate, RFC template + RFC-0001/0002, decisions log D-0001..D-0010.
- Firmware scaffold: digest-pinned ESP-IDF v5.3.5 container, `make
  lite|alpha|omega` board wrapper, Lite board config, `ss_hal` contracts,
  boot display / UART engine / compass / diag bring-up code.
- Tooling: `tools/lint-docs.py`, `tools/gen-stories-index.py` (now enforcing
  §2.7 story elaboration), Claude Code tier agents + rules + skills.
- Project governance files: `.clang-format`, CODEOWNERS, issue + PR
  templates, dependabot (GitHub Actions), this changelog.

### Fixed

- HAL component shadowing (`hal` → `ss_hal`) that broke the mbedtls port on
  the first containerized build.
- Statusline effort parsing when the statusline JSON carries a plain-string
  `effort`.
