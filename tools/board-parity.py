#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2025 SS-SP Project Contributors
#
# board-parity.py — board_config.h macro-parity gate (S-02-004).
#
# Every board port under firmware/boards/<name>/board_config.h must declare
# EXACTLY the same set of `#define SS_*` macro names as the reference port
# (lite). This keeps the HAL's compile-time contract identical across boards
# so the same application logic (which only queries ss_hal_has_cap()) builds
# on every board. This tool does NOT check values — only the set of names —
# plus an SPDX header on each anchored board header.
#
# Checks:
#   1. Reference board (lite) supplies the authoritative SS_* define set.
#   2. Each other board WITH a board_config.h must define the same set:
#        - names in lite but missing from the board  -> violation
#        - names in the board but not in lite (extra) -> violation
#   3. Boards without a board_config.h are reported "skipped" (not a failure).
#   4. Every anchored board header carries an SPDX line in its first 3 lines.
#
# Stdlib only. Exit 0 = clean, 1 = parity/SPDX violations, 2 = usage/setup error.

import argparse
import re
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
BOARDS_DIR = REPO / "firmware" / "boards"
REFERENCE_BOARD = "lite"

# object-like:  #define SS_FOO ...
# function-like: #define SS_FOO(a, b) ...   -> name is "SS_FOO" (drop params)
DEFINE_RE = re.compile(r"^\s*#\s*define\s+(SS_[A-Z0-9_]+)")


def extract_defines(header: Path) -> set[str]:
    """Return the set of SS_* macro names defined in a board header.

    pre: `header` exists and is readable UTF-8.
    post: returns every first-token SS_* name after a `#define`, with any
          function-like parameter list stripped (regex stops at '(' or space).
    """
    names: set[str] = set()
    for line in header.read_text(encoding="utf-8").splitlines():
        m = DEFINE_RE.match(line)
        if m:
            names.add(m.group(1))
    return names


def has_spdx(header: Path) -> bool:
    """True if an SPDX-License-Identifier line is in the first 3 lines."""
    head = header.read_text(encoding="utf-8").splitlines()[:3]
    return any("SPDX-License-Identifier:" in ln for ln in head)


def board_dirs() -> list[Path]:
    """All board port directories, sorted by name."""
    return sorted((p for p in BOARDS_DIR.iterdir() if p.is_dir()), key=lambda p: p.name)


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Verify board_config.h macro parity against the reference board."
    )
    parser.add_argument(
        "--reference",
        default=REFERENCE_BOARD,
        help=f"reference board name (default: {REFERENCE_BOARD})",
    )
    args = parser.parse_args()

    if not BOARDS_DIR.is_dir():
        print(f"board-parity: no boards directory at {BOARDS_DIR}", file=sys.stderr)
        return 2

    ref_header = BOARDS_DIR / args.reference / "board_config.h"
    if not ref_header.is_file():
        print(
            f"board-parity: reference board '{args.reference}' has no board_config.h",
            file=sys.stderr,
        )
        return 2

    ref_defines = extract_defines(ref_header)
    if not ref_defines:
        print(
            f"board-parity: reference board '{args.reference}' defines no SS_* macros",
            file=sys.stderr,
        )
        return 2

    violations: list[str] = []
    checked = 0

    # SPDX check applies to the reference header too.
    if not has_spdx(ref_header):
        violations.append(
            f"{ref_header.relative_to(REPO)}: missing SPDX-License-Identifier in first 3 lines"
        )

    for board in board_dirs():
        name = board.name
        if name == args.reference:
            continue
        header = board / "board_config.h"
        if not header.is_file():
            print(f"board-parity: {name}: skipped (no board_config.h)")
            continue

        checked += 1
        if not has_spdx(header):
            violations.append(
                f"{header.relative_to(REPO)}: missing SPDX-License-Identifier in first 3 lines"
            )

        defines = extract_defines(header)
        missing = sorted(ref_defines - defines)
        extra = sorted(defines - ref_defines)
        for name_missing in missing:
            violations.append(
                f"{name}: missing #define {name_missing} (present in {args.reference})"
            )
        for name_extra in extra:
            violations.append(
                f"{name}: extra #define {name_extra} (not in {args.reference})"
            )

    for v in violations:
        print(v)

    print(
        f"board-parity: {checked} boards checked against {args.reference} "
        f"({len(ref_defines)} defines), {len(violations)} violation(s)"
    )
    return 1 if violations else 0


if __name__ == "__main__":
    sys.exit(main())
