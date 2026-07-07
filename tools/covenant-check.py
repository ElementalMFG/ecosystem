#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2025 SS-SP Project Contributors
#
# covenant-check.py — anti-rug-pull covenant checker (S-01-015).
#
# Detects retroactive license changes to published artifacts, enforcing the
# anti-rug-pull covenant (governance/OPEN_ASSURANCE.md and
# 04_LICENSING_AND_FORK_STRATEGY.md §7).
#
# The ledger (governance/covenant-anchors.json) records, per anchored file,
# its sha256, declared SPDX id, and whether the content is immutable. The
# checker verifies the working tree still honours those anchors and, with a
# base ledger, that the ledger itself is append-only (no anchor is ever
# removed or mutated).
#
# Stdlib only. Exit 0 = all pass, 1 = violation, 2 = usage/internal error.

from __future__ import annotations

import argparse
import datetime
import hashlib
import json
import subprocess
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
LEDGER = REPO / "governance" / "covenant-anchors.json"
VIOL = "COVENANT VIOLATION:"
SPDX_TAG = "SPDX-License-Identifier:"
SPDX_SCAN_LINES = 5


def read_spdx(path: Path) -> str | None:
    """Return the SPDX id declared in the first 5 lines, or None.

    Pre: `path` is a readable text file.
    Post: returns the whitespace-delimited token following the SPDX tag, or
          None if no such tag appears in the leading lines.
    """
    try:
        with path.open(encoding="utf-8") as fh:
            for _ in range(SPDX_SCAN_LINES):
                line = fh.readline()
                if not line:
                    break
                if SPDX_TAG in line:
                    rest = line.split(SPDX_TAG, 1)[1].strip()
                    return rest.split()[0] if rest.split() else None
    except OSError:
        return None
    return None


def sha256_of(path: Path) -> str:
    """Return the hex sha256 digest of `path`'s bytes.

    Pre: `path` exists and is readable.
    Post: returns a 64-character lowercase hex string.
    Error: raises OSError if the file cannot be read.
    """
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(65536), b""):
            h.update(chunk)
    return h.hexdigest()


def last_commit_date(rel: str) -> str:
    """Return the last-commit date (YYYY-MM-DD) for `rel`, or today's date.

    Pre: `rel` is a repo-relative path.
    Post: returns the committer date of the last commit touching `rel`, or
          today's date if the path is not yet committed.
    """
    try:
        out = subprocess.run(
            ["git", "log", "-1", "--format=%cs", "--", rel],
            cwd=REPO, capture_output=True, text=True, check=False,
        )
        date = out.stdout.strip()
        if date:
            return date
    except OSError:
        pass
    return datetime.date.today().isoformat()


def load_ledger(path: Path) -> dict:
    """Parse a ledger JSON file into a dict.

    Pre: `path` refers to a JSON covenant ledger.
    Post: returns the parsed object.
    Error: raises ValueError on malformed JSON or a non-object root.
    """
    try:
        data = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise ValueError(f"cannot read ledger {path}: {exc}") from exc
    if not isinstance(data, dict):
        raise ValueError(f"ledger {path} is not a JSON object")
    return data


def dump_ledger(path: Path, ledger: dict) -> None:
    """Write `ledger` deterministically (anchors sorted, 2-space, newline).

    Pre: `ledger` has "version" and "anchors" keys.
    Post: the file is rewritten with anchors sorted by path and a trailing
          newline.
    """
    anchors = sorted(ledger.get("anchors", []), key=lambda a: a["path"])
    ordered = {"version": ledger.get("version", 1), "anchors": anchors}
    path.write_text(json.dumps(ordered, indent=2) + "\n", encoding="utf-8")


def make_anchor(rel: str, immutable: bool) -> dict:
    """Build a new anchor record for repo-relative path `rel`.

    Pre: `rel` names an existing file in the working tree.
    Post: returns an anchor dict with deterministic field ordering.
    Error: raises ValueError if the file does not exist.
    """
    target = REPO / rel
    if not target.is_file():
        raise ValueError(f"file not found: {rel}")
    return {
        "path": rel,
        "sha256": sha256_of(target),
        "spdx": read_spdx(target),
        "immutable": immutable,
        "anchored": last_commit_date(rel),
    }


def cmd_anchor(paths: list[str], immutable: bool) -> int:
    """Append new anchors for `paths` and rewrite the ledger.

    Pre: each path is repo-relative and not already anchored.
    Post: the ledger gains one anchor per path; returns 0.
    Error: returns 2 if a path is already anchored or missing.
    """
    ledger = load_ledger(LEDGER) if LEDGER.exists() else {"version": 1, "anchors": []}
    existing = {a["path"] for a in ledger.get("anchors", [])}
    ledger.setdefault("anchors", [])
    for rel in paths:
        if rel in existing:
            print(f"error: already anchored: {rel}", file=sys.stderr)
            return 2
        try:
            anchor = make_anchor(rel, immutable)
        except ValueError as exc:
            print(f"error: {exc}", file=sys.stderr)
            return 2
        ledger["anchors"].append(anchor)
        existing.add(rel)
        print(f"anchored: {rel} (spdx={anchor['spdx']}, immutable={immutable})")
    dump_ledger(LEDGER, ledger)
    return 0


def resolve_base(ref: str | None, base_ledger: str | None) -> list[dict] | None:
    """Return the base ledger's anchors, or None if base is empty/absent.

    Pre: at most one of `ref`/`base_ledger` is set.
    Post: returns the list of base anchors, or None when there is no usable
          base (prints an explanatory note in that case).
    """
    if base_ledger is not None:
        try:
            data = load_ledger(Path(base_ledger))
        except ValueError as exc:
            print(f"note: base ledger unusable ({exc}); treating base as empty")
            return None
        return list(data.get("anchors", []))
    if ref is not None:
        if set(ref) <= {"0"} and ref:
            print(f"note: base ref {ref} is all-zeros; treating base as empty")
            return None
        out = subprocess.run(
            ["git", "show", f"{ref}:governance/covenant-anchors.json"],
            cwd=REPO, capture_output=True, text=True, check=False,
        )
        if out.returncode != 0:
            print(f"note: no ledger at base ref {ref}; treating base as empty")
            return None
        try:
            data = json.loads(out.stdout)
        except json.JSONDecodeError as exc:
            print(f"note: base ledger at {ref} malformed ({exc}); treating base as empty")
            return None
        return list(data.get("anchors", [])) if isinstance(data, dict) else None
    return None


def check_ledger_shape(ledger: dict, violations: list[str]) -> list[dict]:
    """Validate ledger structure; append findings; return the anchor list.

    Pre: `ledger` is a parsed ledger object.
    Post: `violations` gains one entry per structural problem; returns the
          anchors (empty list if the shape is unusable).
    """
    if ledger.get("version") != 1:
        violations.append(f"{VIOL} ledger version is {ledger.get('version')!r}, expected 1")
    anchors = ledger.get("anchors", [])
    if not isinstance(anchors, list):
        violations.append(f"{VIOL} ledger 'anchors' is not a list")
        return []
    paths = [a.get("path") for a in anchors]
    if len(paths) != len(set(paths)):
        violations.append(f"{VIOL} ledger has duplicate anchor paths")
    if paths != sorted(paths):
        violations.append(f"{VIOL} ledger anchors are not sorted by path")
    for anchor in anchors:
        digest = anchor.get("sha256", "")
        if not (isinstance(digest, str) and len(digest) == 64
                and all(c in "0123456789abcdef" for c in digest)):
            violations.append(f"{VIOL} {anchor.get('path')}: malformed sha256")
    print(f"OK: ledger parses (version 1, {len(anchors)} anchors)")
    return anchors


def check_anchor(anchor: dict, violations: list[str]) -> None:
    """Check one anchor against the working tree; append any findings.

    Pre: `anchor` is a well-formed anchor record.
    Post: `violations` gains an entry for each covenant breach on this file.
    """
    rel = anchor.get("path", "")
    target = REPO / rel
    if not target.is_file():
        violations.append(f"{VIOL} {rel}: anchored path missing from working tree")
        return
    declared = anchor.get("spdx")
    if declared is not None:
        current = read_spdx(target)
        if current != declared:
            violations.append(
                f"{VIOL} {rel}: retroactive license change "
                f"(anchored {declared}, found {current})"
            )
    if anchor.get("immutable"):
        if sha256_of(target) != anchor.get("sha256"):
            violations.append(
                f"{VIOL} {rel}: immutable content changed (sha256 mismatch)"
            )


def check_append_only(base: list[dict], current: list[dict],
                      violations: list[str]) -> None:
    """Enforce append-only semantics of `current` versus `base`.

    Pre: both are anchor lists.
    Post: `violations` gains an entry for each base anchor that is missing or
          mutated in `current`; additions are permitted.
    """
    by_path = {a.get("path"): a for a in current}
    for anchor in base:
        rel = anchor.get("path")
        cur = by_path.get(rel)
        if cur is None:
            violations.append(f"{VIOL} {rel}: anchor removed (append-only violation)")
            continue
        if cur != anchor:
            violations.append(
                f"{VIOL} {rel}: anchor modified (append-only violation)"
            )
    print(f"OK: append-only base check ({len(base)} base anchors)")


def cmd_check(ref: str | None, base_ledger: str | None) -> int:
    """Run all covenant checks against the working tree.

    Pre: the ledger exists at governance/covenant-anchors.json.
    Post: returns 0 if all checks pass, 1 on any violation, 2 on error.
    """
    try:
        ledger = load_ledger(LEDGER)
    except ValueError as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2
    violations: list[str] = []
    anchors = check_ledger_shape(ledger, violations)
    for anchor in anchors:
        check_anchor(anchor, violations)
    if ref is not None or base_ledger is not None:
        base = resolve_base(ref, base_ledger)
        if base is not None:
            check_append_only(base, anchors, violations)
    for line in violations:
        print(line)
    if violations:
        print(f"covenant-check: {len(violations)} violation(s)")
        return 1
    print(f"covenant-check: {len(anchors)} anchors verified, 0 violations")
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Anti-rug-pull covenant checker (S-01-015)."
    )
    parser.add_argument("--base", metavar="REF",
                        help="enforce append-only against the ledger at git ref REF")
    parser.add_argument("--base-ledger", metavar="FILE",
                        help="enforce append-only against the ledger read from FILE")
    parser.add_argument("--anchor", nargs="+", metavar="PATH",
                        help="append new anchors for the given repo-relative paths")
    parser.add_argument("--immutable", action="store_true",
                        help="mark newly added anchors as immutable")
    return parser


def main(argv: list[str] | None = None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)
    if args.anchor is not None:
        if args.base is not None or args.base_ledger is not None:
            parser.error("--anchor cannot be combined with --base/--base-ledger")
        return cmd_anchor(args.anchor, args.immutable)
    if args.base is not None and args.base_ledger is not None:
        parser.error("--base and --base-ledger are mutually exclusive")
    return cmd_check(args.base, args.base_ledger)


if __name__ == "__main__":
    sys.exit(main())
