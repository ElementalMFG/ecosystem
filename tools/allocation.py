#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""Allocation-automation layer: resolve each portfolio story's model/effort tier.

Source of truth for tiering is the binding allocation strategy:
  - doc 10 (`10_MODEL_ALLOCATION_STRATEGY.md`) §5 per-epic floors, §7 task-type
    view, §11 near-term concrete assignments;
  - doc 11 (`11_TOKEN_ECONOMY.md`) §3 recipe table (model @ effort per tier).

Story blocks are parsed with the same grammar as `tools/gen-stories-index.py`
(`### S-NN-MMM — title` heading followed by a `- Meta:` line).

Tier resolution, in precedence order:
  a) EXPLICIT override (this file, seeded from doc 10 §5/§11) — basis `override`.
  b) KEYWORD flag on heading+user-story+AC text — basis `keyword-flag`, tier
     `T1?` (SUSPECTED T1; must be confirmed at elaboration, never auto-T1).
  c) EPIC floor otherwise — basis `epic-floor`; a T3 floor may drop to T4 for
     small lintable Task/Docs stories whose title names a mechanical artifact.

Usage:
  python3 tools/allocation.py --story S-NN-MMM   # compact recipe block
  python3 tools/allocation.py --generate         # write ALLOCATION_MAP.md
  python3 tools/allocation.py --check            # verify the map is current
  python3 tools/allocation.py --audit            # tier counts + per-epic table
"""

import argparse
import difflib
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
PORTFOLIO = ROOT / "docs" / "portfolio"
EPICS_DIR = PORTFOLIO / "epics"
MAP = PORTFOLIO / "ALLOCATION_MAP.md"

HEADING_RE = re.compile(r"^### (S-(\d{2})-(\d{3})) — (.+)$")
META_RE = re.compile(r"^- Meta:\s*(.+)$")

# --- Model / effort recipe table (doc 11 §3) ------------------------------
FABLE = "claude-fable-5"
OPUS = "claude-opus-4-8"

# Per-tier recipe: orchestrator model@effort, delegated agent, worker skill.
RECIPE = {
    "T1": {"orch": (FABLE, "xhigh"), "agent": "t1-review + t1-cross-review",
           "agent_short": "t1-pipeline", "skill": "/t1-pipeline"},
    "T2": {"orch": (FABLE, "high"), "agent": "t2-builder (opus@high)",
           "agent_short": "t2-builder", "skill": "/story-run"},
    "T3": {"orch": (OPUS, "medium"), "agent": "t3-standard (opus@high)",
           "agent_short": "t3-standard", "skill": "/story-run"},
    "T4": {"orch": (OPUS, "medium"), "agent": "t4-mechanical (opus@low)",
           "agent_short": "t4-mechanical", "skill": "/story-run"},
}

# What tools/claude/work.sh should actually launch (the LAUNCH: machine line).
# T1? starts cheap on the floor launch (opus@medium) and escalates on confirm.
LAUNCH = {
    "T1": (FABLE, "xhigh"),
    "T2": (FABLE, "high"),
    "T3": (OPUS, "medium"),
    "T4": (OPUS, "medium"),
    "T1?": (OPUS, "medium"),
}

# --- (a) explicit overrides (doc 10 §5 / §11) -----------------------------
# Whole-epic T1 floors: EPIC-06/07/08 are T1 in full (doc 10 §5 / §11).
EPIC_ALL_T1 = {"06", "07", "08"}

# story-id -> (tier, citation)
EXPLICIT = {
    # doc 10 §11 near-term queue, named T1
    "S-02-008": ("T1", "doc 10 §11 (panic handler / crash-loop guard)"),
    "S-02-012": ("T1", "doc 10 §11 (brown-out save-state atomicity)"),
    "S-02-021": ("T1", "doc 10 §11 (pool-allocator contract; T1 contract/T2 impl)"),
    # task-seed named T1 cores (wire/frame format + protocol state machines)
    "S-10-008": ("T1", "doc 10 §5 EPIC-10 (SS-Link frame format v1 — wire format)"),
    "S-06-016": ("T1", "doc 10 §5/§11 (EPIC-06 crypto-core; redundant w/ epic floor)"),
    "S-12-014": ("T1", "doc 10 §5 EPIC-12 (LXMF version negotiation — protocol seam)"),
    # doc 10 §11 named T2 (override the EPIC-02 T3 floor)
    "S-02-014": ("T2", "doc 10 §11/§5 (host test-framework design)"),
    "S-02-016": ("T2", "doc 10 §11/§5 (safe-mode / recovery boot path)"),
    "S-02-017": ("T2", "doc 10 §11/§5 (NVS versioning / migrations)"),
    "S-02-019": ("T2", "doc 10 §11 (SBOM attestation — EPIC-23 lineage)"),
    # doc 10 §5 named T2 (override their T3 floor)
    "S-01-017": ("T2", "doc 10 §5 EPIC-01 (deprecation policy — irreversibility)"),
    "S-04-023": ("T2", "doc 10 §5 EPIC-04 (SDIO-vs-USB HaLow bring-up spike)"),
    "S-04-024": ("T2", "doc 10 §5 EPIC-04 (TWT/RAW power-save profile)"),
}

# --- (b) keyword flags (SUSPECTED T1, confirm at elaboration) -------------
KEYWORDS = [
    ("crypto", r"crypto"),
    ("key(s)", r"\bkeys?\b"),
    ("secure boot", r"secure.?boot"),
    ("efuse", r"efuse"),
    ("wire format", r"wire.?format"),
    ("frame format", r"frame.?format"),
    ("sandbox", r"sandbox"),
    ("fuzzer", r"fuzz"),
    ("vector", r"\bvectors?\b"),
    ("ratchet", r"ratchet"),
    ("provisioning", r"provision"),
    ("signing", r"\bsigning\b"),
    ("signature", r"signatur"),
    ("duress", r"duress"),
    ("panic persistence", r"panic[ -]persistence"),
    ("bootloader", r"bootloader"),
]
KEYWORD_RES = [(label, re.compile(pat, re.IGNORECASE)) for label, pat in KEYWORDS]

# --- (c) epic floors ------------------------------------------------------
T2_EPICS = {"09", "10", "11", "12", "13", "17", "18", "22", "23"}
T4_TITLE_KW = ("template", "index", "changelog", "docs", "toc",
               "scaffold", "rename", "format")


def short(model: str) -> str:
    """Short display name for a model id."""
    return "fable" if model == FABLE else "opus"


def keyword_match(text: str) -> str | None:
    """Return the first matching keyword label, or None."""
    for label, rex in KEYWORD_RES:
        if rex.search(text):
            return label
    return None


def parse_meta(raw: str) -> dict:
    """Parse a `- Meta:` payload into a key->value dict (best effort)."""
    meta = {}
    for part in raw.split("|"):
        key, _, val = part.partition("=")
        meta[key.strip()] = val.strip()
    return meta


def parse_epic(path: Path, epic_num: str) -> list[dict]:
    """Parse one STORIES.md into a list of story dicts (deterministic order)."""
    stories = []
    lines = path.read_text(encoding="utf-8").splitlines()
    cur = None  # (id, epic, title)
    buf: list[str] = []
    for line in lines:
        h = HEADING_RE.match(line)
        if h:
            cur = (h.group(1), h.group(2), h.group(4).strip())
            buf = [h.group(4).strip()]
            continue
        if cur is None:
            continue
        m = META_RE.match(line)
        if m:
            meta = parse_meta(m.group(1))
            stories.append({
                "id": cur[0], "epic": cur[1], "num": int(cur[0][-3:]),
                "title": cur[2], "text": "\n".join(buf),
                "Type": meta.get("Type", ""), "Size": meta.get("Size", ""),
                "Status": meta.get("Status", ""),
            })
            cur = None
            buf = []
            continue
        buf.append(line)
    return stories


def load_stories() -> list[dict]:
    """Parse every epic's STORIES.md, sorted by (epic, story number)."""
    out = []
    for d in sorted(p for p in EPICS_DIR.iterdir()
                    if p.is_dir() and p.name.startswith("EPIC-")):
        epic_num = d.name.split("-")[1]
        stories_md = d / "STORIES.md"
        if stories_md.exists():
            out.extend(parse_epic(stories_md, epic_num))
    out.sort(key=lambda s: (s["epic"], s["num"]))
    return out


def epic_floor(story: dict) -> str:
    """Epic-floor tier (precedence c), with the T3->T4 heuristic applied."""
    base = "T2" if story["epic"] in T2_EPICS else "T3"
    if base == "T3":
        small = story["Type"] in ("Task", "Docs") or story["Size"] == "XS"
        titled = any(kw in story["title"].lower() for kw in T4_TITLE_KW)
        if small and titled:
            return "T4"
    return base


def resolve(story: dict) -> tuple[str, str, str]:
    """Resolve (tier, basis, detail) for a story in precedence a -> b -> c."""
    sid, epic = story["id"], story["epic"]
    if epic in EPIC_ALL_T1:
        return "T1", "override", f"EPIC-{epic} is T1 in full (doc 10 §5/§11)"
    if sid in EXPLICIT:
        tier, cite = EXPLICIT[sid]
        return tier, "override", cite
    label = keyword_match(story["text"])
    if label:
        return "T1?", "keyword-flag", f"matched '{label}'"
    return epic_floor(story), "epic-floor", f"EPIC-{epic} default"


def recipe_tier(tier: str, story: dict) -> str:
    """The recipe to apply: T1? borrows its epic floor's recipe."""
    return epic_floor(story) if tier == "T1?" else tier


def print_story(story: dict) -> int:
    """Print the compact recipe block + machine lines for one story."""
    tier, basis, detail = resolve(story)
    rt = recipe_tier(tier, story)
    r = RECIPE[rt]
    o_model, o_effort = r["orch"]
    l_model, l_effort = LAUNCH[tier]

    print(f"Story:        {story['id']} — {story['title']}")
    if tier == "T1?":
        print("Tier:         T1? (SUSPECTED T1 — confirm at elaboration)")
    else:
        print(f"Tier:         {tier}")
    print(f"Basis:        {basis} ({detail})")
    if tier == "T1?":
        print(f"Floor recipe: use unless elaboration confirms a T1 domain "
              f"(floor {rt})")
    print(f"Orchestrator: {short(o_model)} @ {o_effort}")
    print(f"Agent:        {r['agent']}")
    print(f"Skill:        {r['skill']} {story['id']}")
    print(f"Launch hint:  tools/claude/work.sh {story['id']}")
    if tier == "T1?":
        print("CONFIRM:      if a T1 domain is confirmed, add an EXPLICIT "
              "T1 override and run /t1-pipeline on Fable @ xhigh instead.")
    print(f"TIER: {tier}")
    print(f"LAUNCH: model={l_model} effort={l_effort}")
    return 0


HEADER = [
    "<!-- SPDX-License-Identifier: CC-BY-4.0 -->",
    "<!-- GENERATED by tools/allocation.py --generate — do not edit by hand. -->",
    "<!-- Regenerate: python3 tools/allocation.py --generate ; "
    "verify: python3 tools/allocation.py --check -->",
    "# ALLOCATION_MAP — model/effort tier per story",
    "",
    "*Generated from `10_MODEL_ALLOCATION_STRATEGY.md` §5/§7/§11 and",
    "`11_TOKEN_ECONOMY.md` §3 by `tools/allocation.py`. Tier basis is one of",
    "`override` (doc 10 §5/§11), `keyword-flag` (`T1?` — SUSPECTED T1, confirm",
    "at elaboration), or `epic-floor`. Do not edit by hand — change the resolver",
    "or the EXPLICIT override table in `tools/allocation.py` and regenerate.*",
    "",
]


def build_map(stories: list[dict]) -> str:
    """Render the full ALLOCATION_MAP.md text."""
    out = list(HEADER)
    out.append("| Story | Tier (basis) | Orchestrator | Agent | Status |")
    out.append("|---|---|---|---|---|")
    for s in stories:
        tier, basis, _ = resolve(s)
        r = RECIPE[recipe_tier(tier, s)]
        o_model, o_effort = r["orch"]
        out.append(
            f"| {s['id']} | {tier} ({basis}) | {short(o_model)}@{o_effort} "
            f"| {r['agent_short']} | {s['Status']} |"
        )
    out.append("")
    return "\n".join(out) + "\n"


def cmd_generate(stories: list[dict]) -> int:
    MAP.write_text(build_map(stories), encoding="utf-8")
    print(f"allocation: wrote {MAP} — {len(stories)} stories")
    return 0


def cmd_check(stories: list[dict]) -> int:
    generated = build_map(stories)
    if not MAP.exists():
        print("allocation: ALLOCATION_MAP.md missing; run --generate",
              file=sys.stderr)
        return 1
    on_disk = MAP.read_text(encoding="utf-8")
    if on_disk == generated:
        print(f"allocation: OK — {len(stories)} stories, map up to date")
        return 0
    diff = list(difflib.unified_diff(
        on_disk.splitlines(), generated.splitlines(),
        fromfile="ALLOCATION_MAP.md (on disk)", tofile="regenerated", lineterm=""))
    print("allocation: ALLOCATION_MAP.md is stale; re-run --generate",
          file=sys.stderr)
    changed = sum(1 for line in diff
                  if line[:1] in "+-" and line[:2] not in ("++", "--"))
    print(f"allocation: {changed} changed line(s):", file=sys.stderr)
    for line in diff[:40]:
        print(f"  {line}", file=sys.stderr)
    return 1


def cmd_audit(stories: list[dict]) -> int:
    order = ["T1", "T1?", "T2", "T3", "T4"]
    totals = {t: 0 for t in order}
    per_epic: dict[str, dict[str, int]] = {}
    for s in stories:
        tier, _, _ = resolve(s)
        totals[tier] += 1
        per_epic.setdefault(s["epic"], {t: 0 for t in order})[tier] += 1

    print(f"allocation audit — {len(stories)} stories")
    print()
    print("Tier totals:")
    for t in order:
        print(f"  {t:4} {totals[t]}")
    print()
    print("Per-epic breakdown:")
    print(f"  {'Epic':6} " + " ".join(f"{t:>4}" for t in order))
    for epic in sorted(per_epic):
        counts = per_epic[epic]
        print(f"  {epic:6} " + " ".join(f"{counts[t]:>4}" for t in order))
    return 0


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    g = ap.add_mutually_exclusive_group(required=True)
    g.add_argument("--story", metavar="S-NN-MMM", help="print a recipe block")
    g.add_argument("--generate", action="store_true", help="write ALLOCATION_MAP.md")
    g.add_argument("--check", action="store_true", help="verify the map is current")
    g.add_argument("--audit", action="store_true", help="print tier counts")
    args = ap.parse_args()

    stories = load_stories()

    if args.story:
        for s in stories:
            if s["id"] == args.story:
                return print_story(s)
        print(f"allocation: story {args.story} not found", file=sys.stderr)
        return 2
    if args.generate:
        return cmd_generate(stories)
    if args.check:
        return cmd_check(stories)
    if args.audit:
        return cmd_audit(stories)
    return 0


if __name__ == "__main__":
    sys.exit(main())
