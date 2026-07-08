#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""Contract-ownership gate (S-02-025): every HAL declaration is accounted for.

Extracts declared functions from every
firmware/components/ss_hal/include/ss_hal_*.h contract header (plus the
umbrella ss_hal.h init/shutdown sequencer) and asserts
each is either (a) implemented in-tree by a column-0 definition in some
firmware/**/*.c, or (b) claimed in the checked-in ownership map
tools/contract-ownership.txt by a valid owner token (S-NN-MMM or EPIC-NN).

An UNOWNED declaration means a contract exists with neither an
implementation nor an owning story — exactly the orphaned-artifact class
the 2026-07-08 audit found. The gate fails CI with a filing instruction so
the gap is turned into a tracked story (docs/portfolio/00_METHODOLOGY.md
§2.7) rather than silently rotting.

Stdlib only; no third-party parsers. Runs from repo root.

Usage:  python3 tools/contract-audit.py [--check | --list]
  (default)  run as the gate: exit 1 on any unowned declaration.
  --check    alias for the default gate behaviour.
  --list     print every declaration + status (impl/owned/UNOWNED), exit 0.
"""

import argparse
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
HAL_INCLUDE = ROOT / "firmware" / "components" / "ss_hal" / "include"
FIRMWARE = ROOT / "firmware"
OWNERSHIP = ROOT / "tools" / "contract-ownership.txt"
METHODOLOGY = "docs/portfolio/00_METHODOLOGY.md §2.7"

# A valid owner token: a story ID (S-NN-MMM) or an epic (EPIC-NN).
OWNER_RE = re.compile(r"^(S-\d{2}-\d{3}|EPIC-\d{2})$")

# A file-scope prototype: <return type on one line> <name>( <params> ) ;
# The return-type class excludes newline/;/{/} so it stays anchored to the
# statement's own line; the param class excludes ;/{/} so function bodies
# ({...}) and typedef function pointers ((*name)(...)) never match.
PROTO_RE = re.compile(
    r"[A-Za-z_][A-Za-z0-9_ \t*]*?\b(ss_[a-z0-9_]+)\s*\([^;{}]*\)\s*;"
)

# A column-0 function definition: return type at column 0, a body follows
# (no trailing ';'). Calls are indented and never match.
NAME_RE = re.compile(r"\b(ss_[a-z0-9_]+)\s*\(")


def strip_comments(text):
    """Remove /* block */ and // line comments so they can't yield matches."""
    text = re.sub(r"/\*.*?\*/", " ", text, flags=re.DOTALL)
    text = re.sub(r"//[^\n]*", " ", text)
    return text


def declared_functions():
    """Return {funcname: header_filename} for every ss_hal contract header.

    Covers ``ss_hal_*.h`` plus the umbrella ``ss_hal.h`` (init/shutdown
    sequencer) so no declared HAL contract function escapes the gate.
    """
    decls = {}
    headers = sorted(HAL_INCLUDE.glob("ss_hal_*.h"))
    umbrella = HAL_INCLUDE / "ss_hal.h"
    if umbrella.exists():
        headers.append(umbrella)
    for header in headers:
        text = strip_comments(header.read_text())
        for match in PROTO_RE.finditer(text):
            decls[match.group(1)] = header.name
    return decls


def implemented_functions():
    """Return the set of ss_* names with a column-0 definition in firmware/**/*.c."""
    impl = set()
    for src in FIRMWARE.rglob("*.c"):
        for line in src.read_text(errors="replace").splitlines():
            if not line or line[0].isspace():
                continue  # indented -> a call, not a definition
            if line.rstrip().endswith(";"):
                continue  # a prototype/forward decl, not a definition
            for match in NAME_RE.finditer(line):
                impl.add(match.group(1))
    return impl


def load_ownership():
    """Parse the ownership map.

    Returns (by_func, by_header, errors) where by_func maps funcname->owner
    and by_header maps 'ss_hal_x.h'->owner (a header-scoped wildcard).
    """
    by_func, by_header, errors = {}, {}, []
    if not OWNERSHIP.exists():
        errors.append(f"missing ownership map: {OWNERSHIP.relative_to(ROOT)}")
        return by_func, by_header, errors
    for lineno, raw in enumerate(OWNERSHIP.read_text().splitlines(), 1):
        line = raw.split("#", 1)[0].strip()
        if not line:
            continue
        if "=" not in line:
            errors.append(f"{OWNERSHIP.name}:{lineno}: no '=' in '{raw.strip()}'")
            continue
        key, _, owner = line.partition("=")
        key, owner = key.strip(), owner.strip()
        if not OWNER_RE.match(owner):
            errors.append(f"{OWNERSHIP.name}:{lineno}: invalid owner token '{owner}'")
            continue
        if key.endswith(".h"):
            by_header[key] = owner
        else:
            by_func[key] = owner
    return by_func, by_header, errors


def audit():
    """Classify every declaration. Returns (rows, errors).

    rows: list of (funcname, header, status, detail) sorted by header/name.
    status is one of 'impl', 'owned', 'UNOWNED'.
    """
    decls = declared_functions()
    impl = implemented_functions()
    by_func, by_header, errors = load_ownership()

    rows = []
    for name, header in sorted(decls.items(), key=lambda kv: (kv[1], kv[0])):
        if name in impl:
            rows.append((name, header, "impl", ""))
        elif name in by_func:
            rows.append((name, header, "owned", by_func[name]))
        elif header in by_header:
            rows.append((name, header, "owned", by_header[header]))
        else:
            rows.append((name, header, "UNOWNED", ""))
    return rows, errors


def main():
    parser = argparse.ArgumentParser(description="HAL contract-ownership gate.")
    group = parser.add_mutually_exclusive_group()
    group.add_argument("--check", action="store_true", help="run the gate (default).")
    group.add_argument("--list", action="store_true", help="list every declaration + status.")
    args = parser.parse_args()

    rows, errors = audit()

    if args.list:
        for name, header, status, detail in rows:
            suffix = f" [{detail}]" if detail else ""
            print(f"{status:8} {name} ({header}){suffix}")

    for msg in errors:
        print(f"contract-audit: error: {msg}", file=sys.stderr)

    unowned = [(n, h) for n, h, s, _ in rows if s == "UNOWNED"]
    for name, header in unowned:
        print(
            f"UNOWNED: {name} ({header}) — implement it, or add "
            f"'{name} = S-NN-MMM' to tools/contract-ownership.txt and "
            f"file/own that story (see {METHODOLOGY}).",
            file=sys.stderr,
        )

    if errors or unowned:
        sys.exit(1)

    n = len(rows)
    n_impl = sum(1 for _, _, s, _ in rows if s == "impl")
    n_owned = sum(1 for _, _, s, _ in rows if s == "owned")
    print(f"contract-audit: {n} declarations, {n_impl} implemented, {n_owned} owned, 0 unowned")


if __name__ == "__main__":
    main()
