#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2025 SS-SP Project Contributors
#
# lint-docs.py — repository documentation linter (S-01-001).
#
# Checks, for every git-tracked *.md file:
#   1. SPDX-License-Identifier header within the first 3 lines.
#   2. Every relative markdown link resolves to an existing file/dir.
#   3. Every anchor link (#fragment, in-file or cross-file) resolves to a
#      real heading, using the GitHub heading-slug algorithm.
#   4. Each constitution doc (00_*.md .. 08_*.md at repo root) contains a
#      "## Table of contents" section.
#
# Stdlib only. Exit 0 = clean, exit 1 = findings (printed as file:line: msg).

import re
import subprocess
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent

LINK_RE = re.compile(r'!?\[([^\]]*)\]\(([^)\s]+)(?:\s+"[^"]*")?\)')
HEADING_RE = re.compile(r'^(#{1,6})\s+(.*)$')
FENCE_RE = re.compile(r'^\s*(```|~~~)')
INLINE_CODE_RE = re.compile(r'`[^`]*`')
TOC_HEADING_RE = re.compile(r'^##\s+table of contents\s*$', re.IGNORECASE)
CONSTITUTION_RE = re.compile(r'^0[0-8]_[^/]*\.md$')


def gh_slug(text: str) -> str:
    """Approximate github-slugger for a heading's rendered text."""
    text = re.sub(r'\[([^\]]*)\]\([^)]*\)', r'\1', text)  # links -> text
    text = text.replace('`', '').replace('*', '')          # code/emphasis
    text = text.strip().lower()
    out = []
    for ch in text:
        if ch.isalnum() or ch in '-_':
            out.append(ch)
        elif ch == ' ':
            out.append('-')
        # all other punctuation is dropped
    return ''.join(out)


class Doc:
    def __init__(self, relpath: str, lines: list[str]):
        self.relpath = relpath
        self.lines = lines
        self.anchors: set[str] = set()
        self.links: list[tuple[int, str]] = []  # (lineno, target)
        self.has_toc = False
        self._parse()

    def _parse(self):
        seen: dict[str, int] = {}
        in_fence = False
        for i, line in enumerate(self.lines, start=1):
            if FENCE_RE.match(line):
                in_fence = not in_fence
                continue
            if in_fence:
                continue
            m = HEADING_RE.match(line)
            if m:
                slug = gh_slug(m.group(2))
                n = seen.get(slug, 0)
                seen[slug] = n + 1
                self.anchors.add(slug if n == 0 else f'{slug}-{n}')
                if TOC_HEADING_RE.match(line):
                    self.has_toc = True
            scrubbed = INLINE_CODE_RE.sub('', line)
            for lm in LINK_RE.finditer(scrubbed):
                self.links.append((i, lm.group(2)))


def tracked_md() -> list[str]:
    out = subprocess.run(
        ['git', 'ls-files', '*.md'], cwd=REPO,
        capture_output=True, text=True, check=True)
    return out.stdout.split()


def main() -> int:
    errors: list[str] = []
    docs: dict[str, Doc] = {}
    for rel in tracked_md():
        text = (REPO / rel).read_text(encoding='utf-8')
        docs[rel] = Doc(rel, text.splitlines())

    for rel, doc in docs.items():
        # 1. SPDX header
        head = '\n'.join(doc.lines[:3])
        if 'SPDX-License-Identifier:' not in head:
            errors.append(f'{rel}:1: missing SPDX-License-Identifier in first 3 lines')

        # 4. constitution TOC
        if CONSTITUTION_RE.match(rel) and not doc.has_toc:
            errors.append(f'{rel}:1: constitution doc missing "## Table of contents" section')

        # 2 + 3. links and anchors
        for lineno, target in doc.links:
            if re.match(r'^[a-z][a-z0-9+.-]*:', target):   # http:, https:, mailto:, ...
                continue
            path_part, _, frag = target.partition('#')
            if path_part == '':
                # in-file anchor
                if frag and frag not in doc.anchors:
                    errors.append(f'{rel}:{lineno}: broken anchor "#{frag}"')
                continue
            base = REPO if path_part.startswith('/') else (REPO / rel).parent
            resolved = (base / path_part.lstrip('/')).resolve()
            if not resolved.exists():
                errors.append(f'{rel}:{lineno}: broken link "{target}"')
                continue
            if frag:
                try:
                    target_rel = str(resolved.relative_to(REPO))
                except ValueError:
                    continue
                tdoc = docs.get(target_rel)
                if tdoc is not None and frag not in tdoc.anchors:
                    errors.append(f'{rel}:{lineno}: broken anchor "{target}" '
                                  f'(no heading "#{frag}" in {target_rel})')

    for e in errors:
        print(e)
    n = len(errors)
    print(f'lint-docs: {len(docs)} files checked, {n} finding(s)')
    return 1 if n else 0


if __name__ == '__main__':
    sys.exit(main())
