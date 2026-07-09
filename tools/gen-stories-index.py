#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""Regenerate docs/portfolio/STORIES_INDEX.md from every epic's STORIES.md.

Source of truth: the machine-parsed `- Meta:` line of each story, per
docs/portfolio/00_METHODOLOGY.md §2.7. Field order is mandatory:

  - Meta: Shard=.. | Type=.. | Size=.. | Prio=.. | Status=.. | SKU=.. | PRD=.. | Const=..

Everything above the HAND-MAINTAINED marker in STORIES_INDEX.md is
regenerated; everything below it is preserved verbatim.

§2.7 elaboration rule (enforced here): any story whose Status is not
DRAFT or DROPPED must carry `- Tasks:` and `- Deps:` lines; every
`S-NN-MMM` reference in a Deps line must resolve to an existing story.

Usage:  python3 tools/gen-stories-index.py [--check]
  --check   validate only; exit 1 on any problem, write nothing.
"""

import argparse
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
PORTFOLIO = ROOT / "docs" / "portfolio"
EPICS_DIR = PORTFOLIO / "epics"
INDEX = PORTFOLIO / "STORIES_INDEX.md"

MARKER = "<!-- GENERATED ABOVE — do not edit by hand. Hand-maintained sections below. -->"

META_FIELDS = ["Shard", "Type", "Size", "Prio", "Status", "SKU", "PRD", "Const"]
SIZES = {"XS", "S", "M", "L", "XL"}
PRIOS = {"P0", "P1", "P2", "P3"}
STATUSES = {"DRAFT", "READY", "IN_PROGRESS", "IN_REVIEW", "DONE", "BLOCKED", "DROPPED"}
TYPES = {"Feature", "Task", "Spike", "Bug", "RFC", "Compliance", "Ops", "Docs"}
CLAUSE_RE = re.compile(r"^(C-0[0-8]|C-OA|C-TM|C-CoC|C-SEC)$")
HEADING_RE = re.compile(r"^### (S-(\d{2})-(\d{3})) — (.+)$")
META_RE = re.compile(r"^- Meta:\s*(.+)$")
TASKS_RE = re.compile(r"^- Tasks:\s*(.+)$")
DEPS_RE = re.compile(r"^- Deps:\s*(.+)$")
STORY_REF_RE = re.compile(r"S-\d{2}-\d{3}")
ELABORATION_EXEMPT = {"DRAFT", "DROPPED"}


def parse_meta(line, errors, ctx):
    parts = [p.strip() for p in line.split("|")]
    meta = {}
    if len(parts) != len(META_FIELDS):
        errors.append(f"{ctx}: Meta line has {len(parts)} fields, expected {len(META_FIELDS)}")
        return None
    for i, part in enumerate(parts):
        key, _, val = part.partition("=")
        key, val = key.strip(), val.strip()
        if key != META_FIELDS[i]:
            errors.append(f"{ctx}: field {i + 1} is '{key}', expected '{META_FIELDS[i]}' (order is mandatory)")
            return None
        meta[key] = val
    if meta["Size"] not in SIZES:
        errors.append(f"{ctx}: invalid Size '{meta['Size']}'")
    if meta["Prio"] not in PRIOS:
        errors.append(f"{ctx}: invalid Prio '{meta['Prio']}'")
    if meta["Status"] not in STATUSES:
        errors.append(f"{ctx}: invalid Status '{meta['Status']}'")
    if meta["Type"] not in TYPES:
        errors.append(f"{ctx}: invalid Type '{meta['Type']}'")
    if meta["Const"] != "—":
        for clause in re.split(r"[,\s]+", meta["Const"]):
            if clause and not CLAUSE_RE.match(clause):
                errors.append(f"{ctx}: invalid constitution clause key '{clause}' (see §2.9)")
    return meta


def parse_stories(path, epic_num, errors):
    stories = []
    lines = path.read_text(encoding="utf-8").splitlines()
    current = None  # (id, num, title, heading_lineno)
    last_story = None  # most recent story dict (Meta already parsed)
    for lineno, line in enumerate(lines, 1):
        h = HEADING_RE.match(line)
        if h:
            if current is not None:
                errors.append(f"{path}:{current[3]}: story {current[0]} has no Meta line")
            sid, s_epic, s_num, title = h.group(1), h.group(2), h.group(3), h.group(4).strip()
            if s_epic != epic_num:
                errors.append(f"{path}:{lineno}: story {sid} epic prefix != {epic_num}")
            current = (sid, int(s_num), title, lineno)
            last_story = None
            continue
        m = META_RE.match(line)
        if m and current is not None:
            meta = parse_meta(m.group(1), errors, f"{path}:{lineno} ({current[0]})")
            if meta is not None:
                story = {"id": current[0], "num": current[1], "title": current[2],
                         "path": str(path), "line": current[3], "Tasks": None, "Deps": None,
                         **meta}
                stories.append(story)
                last_story = story
            current = None
            continue
        t = TASKS_RE.match(line)
        if t and last_story is not None:
            last_story["Tasks"] = t.group(1).strip()
            continue
        dp = DEPS_RE.match(line)
        if dp and last_story is not None:
            last_story["Deps"] = dp.group(1).strip()
    if current is not None:
        errors.append(f"{path}:{current[3]}: story {current[0]} has no Meta line")
    # contiguity
    nums = sorted(s["num"] for s in stories)
    for want, got in enumerate(nums, 1):
        if want != got:
            errors.append(f"{path}: story numbering not contiguous (expected {want:03d}, found {got:03d})")
            break
    return stories


def epic_title(epic_dir):
    epic_md = epic_dir / "EPIC.md"
    if epic_md.exists():
        for line in epic_md.read_text(encoding="utf-8").splitlines():
            m = re.match(r"^# EPIC-\d{2}\s+—\s+(.+)$", line)
            if m:
                return m.group(1).strip()
    return epic_dir.name.split("-", 2)[-1].replace("-", " ")


def build(epics):
    out = []
    out.append("<!-- SPDX-License-Identifier: CC-BY-4.0 -->")
    out.append("# STORIES_INDEX — Traceability Matrix")
    out.append("")
    out.append("*Master ledger. Rows = stories. Generated by `tools/gen-stories-index.py` from the")
    out.append("`- Meta:` line of every story in `epics/EPIC-NN-*/STORIES.md` (format:")
    out.append("`00_METHODOLOGY.md` §2.7; clause keys: §2.9). Do not edit generated rows by hand —")
    out.append("edit the epic's `STORIES.md` and re-run the generator.*")
    out.append("")
    out.append("**How this file is used:**")
    out.append("- On PR: cite the story ID; CI verifies the row exists and re-runs the generator (`--check`).")
    out.append("- On release: verify every P0 story in the milestone has status `DONE`.")
    out.append("- On audit: every constitution clause key must appear in at least one P0 row (see coverage table).")
    out.append("")
    out.append("Story text lives in the epic's `STORIES.md`; it is not duplicated here.")
    out.append("")

    total = 0
    clause_cover = {}   # clause -> set of story ids
    clause_p0 = {}      # clause -> bool (has P0 row)
    status_counts = {}
    for epic_num, title, stories in epics:
        total += len(stories)
        out.append("---")
        out.append("")
        out.append(f"## Epic-{epic_num} — {title} ({len(stories)} stories)")
        out.append("")
        out.append("| Story | Shard | Type | Size | Prio | Status | SKU | PRD | Const |")
        out.append("|---|---|---|---|---|---|---|---|---|")
        for s in sorted(stories, key=lambda x: x["num"]):
            out.append(
                f"| {s['id']} | {s['Shard']} | {s['Type']} | {s['Size']} | {s['Prio']} "
                f"| {s['Status']} | {s['SKU']} | {s['PRD']} | {s['Const']} |"
            )
            status_counts[s["Status"]] = status_counts.get(s["Status"], 0) + 1
            if s["Const"] != "—":
                for clause in re.split(r"[,\s]+", s["Const"]):
                    if clause:
                        clause_cover.setdefault(clause, set()).add(s["id"])
                        if s["Prio"] == "P0":
                            clause_p0[clause] = True
        out.append("")

    out.append("---")
    out.append("")
    out.append(f"## Totals — {total} stories across {len(epics)} epics")
    out.append("")
    out.append("| Status | Count |")
    out.append("|---|---|")
    for st in ["DRAFT", "READY", "IN_PROGRESS", "IN_REVIEW", "DONE", "BLOCKED", "DROPPED"]:
        if st in status_counts:
            out.append(f"| {st} | {status_counts[st]} |")
    out.append("")
    out.append("---")
    out.append("")
    out.append("## Constitution-clause coverage (auto-computed)")
    out.append("")
    out.append("Every clause key must be covered by at least one P0 story; a `no` in the P0 column is a CI failure.")
    out.append("")
    out.append("| Clause key | Stories | P0 coverage |")
    out.append("|---|---|---|")
    all_clauses = ["C-00", "C-01", "C-02", "C-03", "C-04", "C-05", "C-06", "C-07", "C-08",
                   "C-OA", "C-TM", "C-CoC", "C-SEC"]
    for clause in all_clauses:
        ids = clause_cover.get(clause, set())
        p0 = "yes" if clause_p0.get(clause) else "no"
        out.append(f"| {clause} | {len(ids)} | {p0} |")
    out.append("")
    out.append(MARKER)
    out.append("")
    return "\n".join(out) + "\n", total, clause_cover, clause_p0


DEFAULT_HAND = """## Working-group ownership summary (hand-maintained)

| WG | Epics owned |
|---|---|
| wg-community | 01, 24 |
| wg-firmware | 02, 03, 04, 05, 08, 09, 10, 11, 14, 15, 16, 17, 18 |
| wg-hardware | (contributes to 03, 04, 05) |
| wg-protocol | 10, 11, 12, 13 |
| wg-security | 06, 07, 08 (with wg-firmware) |
| wg-ui-ux | 14, 15, 16 |
| wg-apps | 19 |
| wg-cloud | 17, 21 |
| wg-sdk | 20 |
| wg-legal | 24 (with wg-community) |
| wg-docs | (contributes to all) |
| wg-ops | 09, 22, 23 |

---

*End of STORIES_INDEX.*
"""


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--check", action="store_true", help="validate only; write nothing")
    args = ap.parse_args()

    errors = []
    epic_dirs = sorted(d for d in EPICS_DIR.iterdir() if d.is_dir() and d.name.startswith("EPIC-"))
    if len(epic_dirs) != 24:
        errors.append(f"expected 24 epic directories, found {len(epic_dirs)}")

    epics = []
    for d in epic_dirs:
        epic_num = d.name.split("-")[1]
        stories_md = d / "STORIES.md"
        if not stories_md.exists():
            errors.append(f"{d}: missing STORIES.md")
            continue
        stories = parse_stories(stories_md, epic_num, errors)
        if not stories:
            errors.append(f"{stories_md}: no parseable stories found")
        epics.append((epic_num, epic_title(d), stories))

    # §2.7 elaboration enforcement: non-DRAFT/DROPPED stories need Tasks + Deps;
    # Deps story references must resolve and may not be self-referential.
    all_ids = {s["id"] for _, _, ss in epics for s in ss}
    for _, _, ss in epics:
        for s in ss:
            ctx = f"{s['path']}:{s['line']} ({s['id']})"
            if s["Status"] not in ELABORATION_EXEMPT:
                if not s["Tasks"]:
                    errors.append(f"{ctx}: Status={s['Status']} requires a '- Tasks:' elaboration line (§2.7)")
                if not s["Deps"]:
                    errors.append(f"{ctx}: Status={s['Status']} requires a '- Deps:' elaboration line (§2.7)")
            if s["Deps"]:
                for ref in STORY_REF_RE.findall(s["Deps"]):
                    if ref == s["id"]:
                        errors.append(f"{ctx}: story depends on itself")
                    elif ref not in all_ids:
                        errors.append(f"{ctx}: Deps references unknown story {ref}")
            # Park-reason rule (2026-07-09, from the S-02-019/S-03-009 ledger
            # audit): a parked/blocked status must say WHAT it waits on, in the
            # Deps line, or the story silently rots — nobody can drive it out.
            if s["Status"] == "IN_REVIEW" and s["Deps"]:
                if not re.search(r"park|pending|await|until|needs?\b", s["Deps"], re.I):
                    errors.append(f"{ctx}: Status=IN_REVIEW but the Deps line records no "
                                  f"park reason (what evidence is missing + what supplies it)")
            if s["Status"] == "BLOCKED" and s["Deps"]:
                if not re.search(r"block", s["Deps"], re.I):
                    errors.append(f"{ctx}: Status=BLOCKED but the Deps line does not "
                                  f"say what blocks it (annotate 'BLOCKED <date>: <reason>')")

    generated, total, clause_cover, clause_p0 = build(epics)

    for clause in ["C-00", "C-01", "C-02", "C-03", "C-04", "C-05", "C-06", "C-07", "C-08",
                   "C-OA", "C-TM", "C-CoC", "C-SEC"]:
        if clause not in clause_cover:
            errors.append(f"coverage: clause {clause} referenced by zero stories")
        elif not clause_p0.get(clause):
            errors.append(f"coverage: clause {clause} has no P0 story")

    if errors:
        print(f"gen-stories-index: {len(errors)} problem(s):", file=sys.stderr)
        for e in errors:
            print(f"  - {e}", file=sys.stderr)
        if args.check:
            return 1

    if args.check:
        # also verify the on-disk index matches what we would generate
        if INDEX.exists():
            on_disk = INDEX.read_text(encoding="utf-8")
            head = on_disk.split(MARKER)[0] + MARKER
            if head != generated.rstrip("\n") or MARKER not in on_disk:
                print("gen-stories-index: STORIES_INDEX.md is stale; re-run the generator", file=sys.stderr)
                return 1
        print(f"gen-stories-index: OK — {total} stories, index up to date")
        return 0

    hand = DEFAULT_HAND
    if INDEX.exists():
        on_disk = INDEX.read_text(encoding="utf-8")
        if MARKER in on_disk:
            hand = on_disk.split(MARKER, 1)[1].lstrip("\n")
    INDEX.write_text(generated + "\n" + hand, encoding="utf-8")
    print(f"gen-stories-index: wrote {INDEX} — {total} stories across {len(epics)} epics"
          + (f" ({len(errors)} warnings)" if errors else ""))
    return 0


if __name__ == "__main__":
    sys.exit(main())
