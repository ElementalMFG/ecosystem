---
# SPDX-License-Identifier: Apache-2.0
paths:
  - "**/*.md"
---

# Markdown conventions (lint-enforced)

- SPDX in the **first 3 lines** of every `.md`: `<!-- SPDX-License-Identifier: CC-BY-4.0 -->` for prose docs; `Apache-2.0` where the file/dir already uses it (e.g. `.claude/`, YAML-frontmatter files use a `# SPDX…` comment on line 2 inside the frontmatter).
- Root constitution docs `0[0-8]_*.md` require a `## Table of contents` whose anchors match GitHub slugs; all relative links and `#anchors` must resolve (`tools/lint-docs.py` checks all of this).
- Constitution docs of record at root: 00 master plan, 01 Lite hardware reference, 02 protocol stack, 03 UI layout spec, 04 licensing & fork strategy, 05 security model, 06 governance, 07 business model & open source, 08 universal connectivity.
- Commits: Conventional Commits (`feat|fix|docs|security|chore(scope): …`), signed (`git commit -s`); changelog per `CONTRIBUTING.md` §3.
- Gate before pushing doc changes: `git add` the files, then `python3 tools/lint-docs.py` from repo root.
