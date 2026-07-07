#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2025 SS-SP Project Contributors
#
# task-policy-check.py — FreeRTOS task-creation policy gate (S-02-006).
#
# The RTOS baseline (firmware/main/ss_tasks.h) defines a priority-ceiling
# policy: all application tasks must be created through ss_task_create() or
# ss_task_create_pinned() and must name one of the SS_PRIO_* bands rather than
# passing a bare numeric priority. This tool enforces both rules across
# firmware/main so the policy cannot silently rot.
#
# Checks (over firmware/main/**/*.{c,cpp,h}, EXCLUDING ss_tasks.{h,cpp} which is
# the one place the raw APIs are legitimately used):
#
#   1. Raw task-creation calls. Any `xTaskCreate(` or `xTaskCreatePinnedToCore(`
#      occurrence is a violation (reported file:line) — application code must go
#      through the wrappers.
#
#   2. Numeric-literal priorities. Any ss_task_create() / ss_task_create_pinned()
#      call whose priority argument is a bare integer literal is a violation —
#      the priority must be an SS_PRIO_* band.
#
#      Heuristic: the story specifies the seed regex
#          ss_task_create\w*\s*\([^;]*,\s*[0-9]+\s*,
#      but taken literally that also matches the (legitimate, always-numeric)
#      *stack-depth* argument, so it false-positives on every wrapped call.
#      We therefore tighten it: locate each ss_task_create* call, split its
#      top-level arguments, and flag only when the PRIORITY argument (arg index
#      4 by the fixed wrapper signature: task, name, stack, arg, prio, ...) is a
#      pure integer literal. Same intent, no stack-depth false positive.
#
# Stdlib only. Exit 0 = clean, 1 = violations, 2 = usage/setup error.

import argparse
import re
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
DEFAULT_ROOT = REPO / "firmware" / "main"
SOURCE_SUFFIXES = (".c", ".cpp", ".h")
EXCLUDED_NAMES = {"ss_tasks.h", "ss_tasks.cpp"}

# Raw API calls that application code must not use directly. `xTaskCreate\s*\(`
# does not match `xTaskCreatePinnedToCore(` (the char after "xTaskCreate" is
# "P"), so the two patterns are disjoint.
RAW_CALL_RE = re.compile(r"\b(xTaskCreate|xTaskCreatePinnedToCore)\s*\(")

# Start of a wrapper call; used to locate calls for the priority-slot check.
WRAPPER_CALL_RE = re.compile(r"\bss_task_create(?:_pinned)?\s*\(")

# A bare integer literal (decimal or hex, optional sign/suffix). SS_PRIO_* bands
# and expressions like UBaseType_t(5) do not match.
INT_LITERAL_RE = re.compile(r"^[+-]?(?:0[xX][0-9a-fA-F]+|[0-9]+)[uUlL]*$")

# Priority is the 5th positional argument of both wrappers (0-based index 4):
#   ss_task_create(task, name, stack, arg, prio, out)
#   ss_task_create_pinned(task, name, stack, arg, prio, out, core)
PRIO_ARG_INDEX = 4


def source_files(root: Path) -> list[Path]:
    """All firmware source files under `root`, sorted, excluding the wrappers.

    pre: `root` is an existing directory.
    post: returns every *.c/*.cpp/*.h file under `root` except ss_tasks.{h,cpp}.
    """
    return sorted(
        p
        for p in root.rglob("*")
        if p.is_file() and p.suffix in SOURCE_SUFFIXES and p.name not in EXCLUDED_NAMES
    )


def split_top_level_args(arglist: str) -> list[str]:
    """Split a C argument list on top-level commas.

    pre: `arglist` is the text between a call's outer parentheses (exclusive).
    post: returns each argument, commas inside nested (), [], {}, string or
          character literals ignored. Whitespace is not stripped.
    """
    args: list[str] = []
    depth = 0
    start = 0
    in_str: str | None = None
    i = 0
    while i < len(arglist):
        ch = arglist[i]
        if in_str is not None:
            if ch == "\\":
                i += 2
                continue
            if ch == in_str:
                in_str = None
        elif ch in "\"'":
            in_str = ch
        elif ch in "([{":
            depth += 1
        elif ch in ")]}":
            depth -= 1
        elif ch == "," and depth == 0:
            args.append(arglist[start:i])
            start = i + 1
        i += 1
    args.append(arglist[start:])
    return args


def extract_call_args(text: str, open_paren: int) -> str | None:
    """Return the argument text between the matching parentheses.

    pre: text[open_paren] == '('.
    post: returns the substring inside the balanced parentheses, or None if the
          parentheses are unbalanced (truncated / malformed source).
    """
    depth = 0
    in_str: str | None = None
    i = open_paren
    while i < len(text):
        ch = text[i]
        if in_str is not None:
            if ch == "\\":
                i += 2
                continue
            if ch == in_str:
                in_str = None
        elif ch in "\"'":
            in_str = ch
        elif ch == "(":
            depth += 1
        elif ch == ")":
            depth -= 1
            if depth == 0:
                return text[open_paren + 1 : i]
        i += 1
    return None


def check_file(path: Path) -> list[str]:
    """Return policy violations (as "file:line: msg") for one source file."""
    rel = path.relative_to(REPO)
    text = path.read_text(encoding="utf-8")
    violations: list[str] = []

    # 1. Raw task-creation calls (line-oriented).
    for lineno, line in enumerate(text.splitlines(), start=1):
        for m in RAW_CALL_RE.finditer(line):
            violations.append(
                f"{rel}:{lineno}: raw {m.group(1)}() — use ss_task_create*() (ss_tasks.h policy)"
            )

    # 2. Numeric-literal priority in a wrapper call (may span lines).
    for m in WRAPPER_CALL_RE.finditer(text):
        open_paren = m.end() - 1
        arglist = extract_call_args(text, open_paren)
        if arglist is None:
            continue
        args = split_top_level_args(arglist)
        if len(args) <= PRIO_ARG_INDEX:
            continue
        prio = args[PRIO_ARG_INDEX].strip()
        if INT_LITERAL_RE.match(prio):
            lineno = text.count("\n", 0, m.start()) + 1
            violations.append(
                f"{rel}:{lineno}: numeric priority '{prio}' in "
                f"{m.group(0)[:-1].strip()}() — name an SS_PRIO_* band"
            )
    return violations


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Enforce the FreeRTOS priority-ceiling task policy (S-02-006)."
    )
    parser.add_argument(
        "--root",
        type=Path,
        default=DEFAULT_ROOT,
        help=f"directory tree to scan (default: {DEFAULT_ROOT.relative_to(REPO)})",
    )
    args = parser.parse_args()

    root: Path = args.root
    if not root.is_dir():
        print(f"task-policy: no such directory {root}", file=sys.stderr)
        return 2

    files = source_files(root)
    violations: list[str] = []
    for path in files:
        violations.extend(check_file(path))

    for v in violations:
        print(v)

    print(f"task-policy: {len(files)} files scanned, {len(violations)} violation(s)")
    return 1 if violations else 0


if __name__ == "__main__":
    sys.exit(main())
