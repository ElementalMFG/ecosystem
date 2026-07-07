#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2025 SS-SP Project Contributors
#
# boot-budget-check.py — boot-time budget gate (S-02-010).
#
# The firmware emits one machine-parseable line at the end of boot (see
# firmware/main/ss_bootmark.cpp):
#
#   boot-report: gate=<us> nvs=<us> ... app_ready=<us> total=<us>
#
# This tool scans a captured boot log for the LAST such line, parses the
# space-separated name=value (µs) pairs, and asserts that the chosen milestone
# (default "app_ready") landed under the budget. The last-line rule means a log
# containing several boots is judged on the most recent one.
#
# Stdlib only. Exit 0 = under budget, 1 = over budget / milestone missing,
# 2 = no boot-report line or parse error.

import argparse
import sys
from pathlib import Path

BOOT_REPORT_PREFIX = "boot-report:"


def find_last_report(text: str) -> str | None:
    """Return the last 'boot-report:' payload (text after the prefix), or None.

    pre: `text` is the full log contents.
    post: returns the substring following the LAST occurrence of the prefix,
          stripped; None if no such line exists.
    """
    last: str | None = None
    for line in text.splitlines():
        idx = line.find(BOOT_REPORT_PREFIX)
        if idx != -1:
            last = line[idx + len(BOOT_REPORT_PREFIX) :].strip()
    return last


def parse_marks(payload: str) -> dict[str, int]:
    """Parse space-separated name=value (µs) pairs into a dict.

    pre: `payload` is the text after the 'boot-report:' prefix.
    post: returns an ordered dict of name -> microseconds (int); raises
          ValueError on a malformed token.
    """
    marks: dict[str, int] = {}
    for tok in payload.split():
        name, sep, value = tok.partition("=")
        if not sep or not name:
            raise ValueError(f"malformed token: {tok!r}")
        marks[name] = int(value)
    return marks


def write_report(path: Path, marks: dict[str, int], milestone: str, budget_ms: float, ok: bool) -> None:
    """Write a small human-readable report (all milestones in ms + verdict)."""
    lines = ["boot-budget report", f"milestone: {milestone}", f"budget-ms: {budget_ms}", ""]
    for name, us in marks.items():
        lines.append(f"  {name} = {us / 1000:.3f} ms")
    lines.append("")
    lines.append(f"verdict: {'PASS' if ok else 'FAIL'}")
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Assert a firmware boot milestone landed under budget from a captured log."
    )
    parser.add_argument("logfile", type=Path, help="path to a captured boot log")
    parser.add_argument(
        "--budget-ms", type=float, default=400.0, help="milestone budget in ms (default: 400)"
    )
    parser.add_argument(
        "--milestone", default="app_ready", help="milestone name to assert (default: app_ready)"
    )
    parser.add_argument(
        "--report-out", type=Path, default=None, help="optional path to write a text report"
    )
    args = parser.parse_args()

    try:
        text = args.logfile.read_text(encoding="utf-8", errors="replace")
    except OSError as exc:
        print(f"boot-budget: cannot read {args.logfile}: {exc}", file=sys.stderr)
        return 2

    payload = find_last_report(text)
    if payload is None:
        print(f"boot-budget: no '{BOOT_REPORT_PREFIX}' line in {args.logfile}", file=sys.stderr)
        return 2

    try:
        marks = parse_marks(payload)
    except ValueError as exc:
        print(f"boot-budget: parse error: {exc}", file=sys.stderr)
        return 2

    if args.milestone not in marks:
        print(
            f"boot-budget: milestone '{args.milestone}' not in report "
            f"(have: {', '.join(marks) or 'none'})",
            file=sys.stderr,
        )
        if args.report_out is not None:
            write_report(args.report_out, marks, args.milestone, args.budget_ms, ok=False)
        return 1

    value_ms = marks[args.milestone] / 1000
    ok = value_ms < args.budget_ms

    if args.report_out is not None:
        write_report(args.report_out, marks, args.milestone, args.budget_ms, ok=ok)

    verdict = "PASS" if ok else "FAIL"
    op = "<" if ok else ">="
    print(f"boot-budget: {args.milestone} {value_ms:.3f}ms {op} {args.budget_ms}ms {verdict}")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
