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
DEPS_RE = re.compile(r"^- Deps:\s*(.+)$")
DEP_ID_RE = re.compile(r"S-\d{2}-\d{3}")

# --- dependency-ordering vocabulary --------------------------------------
# A dep is SATISFIED if its status is DONE, or IN_REVIEW (satisfied-with-note:
# implementation exists, only live evidence is pending). Anything else
# (DRAFT/IN_PROGRESS/BLOCKED/READY) or a nonexistent id is UNSATISFIED.
SATISFIED = {"DONE", "IN_REVIEW"}
FRONTIER_STATUS = {"DRAFT", "READY"}
PRIO_ORDER = {"P0": 0, "P1": 1, "P2": 2, "P3": 3}

# --- Model / effort recipe table (doc 11 §3) ------------------------------
FABLE = "claude-fable-5"
OPUS = "claude-opus-4-8"

# Per-tier recipe: orchestrator model@effort, delegated agent, worker skill.
RECIPE = {
    "T1": {"orch": (FABLE, "xhigh"), "agent": "t1-review + t1-cross-review",
           "agent_short": "t1-pipeline", "skill": "/t1-pipeline"},
    # T2 re-architecture (doc 11 §6f, 2026-07-07): orchestration on Opus like
    # T3; only the frozen-contract design runs on Fable, via the t2-designer
    # agent (fable@medium per doc 10 §1.3 parity), then t2-builder implements.
    "T2": {"orch": (OPUS, "medium"), "agent": "t2-designer (fable@medium) + t2-builder (opus@high)",
           "agent_short": "t2-designer+builder", "skill": "/story-run"},
    # T3 pin dropped high->medium (doc 11 §6f): verification-saturated tier;
    # two failed attempts escalate to t2-builder (opus@high) per doc 10 §8.
    "T3": {"orch": (OPUS, "medium"), "agent": "t3-standard (opus@medium)",
           "agent_short": "t3-standard", "skill": "/story-run"},
    "T4": {"orch": (OPUS, "medium"), "agent": "t4-mechanical (opus@low)",
           "agent_short": "t4-mechanical", "skill": "/story-run"},
}

# What tools/claude/work.sh should actually launch (the LAUNCH: machine line).
# T1? starts cheap on the floor launch (opus@medium) and escalates on confirm.
LAUNCH = {
    "T1": (FABLE, "xhigh"),
    "T2": (OPUS, "medium"),
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
    "S-03-030": ("T1", "frozen ss_hal contract change (components/ss_hal/** is a T1 path, doc 10 §8.3)"),
    "S-03-032": ("T1", "ss_hal ABI/signature repair (components/ss_hal/** is a T1 path, doc 10 §8.3)"),
    "S-03-039": ("T1", "watchdog HAL surface reconciliation (ss_hal_watchdog.h change is a T1 path)"),
    "S-03-015": ("T1", "confirmed 2026-07-08: AC carries credential handoff over an open soft-AP — onboarding security surface (doc 05); portal plumbing rides the same story under T1 review"),
    "S-03-016": ("T1", "confirmed 2026-07-08: AC exposes GATT pairing + provisioning services — BLE pairing-mode/MITM policy and the provisioning characteristic are the onboarding security surface (doc 05); GATT plumbing rides the same story under T1 review"),
    "S-03-017": ("T1", "confirmed 2026-07-08: LTK storage, encrypted-NVS-only rest state, bond wipe lifecycle — keys at rest (doc 10 §2 T1 domain)"),
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


def parse_deps(block: list[str], self_id: str) -> list[str]:
    """Extract S-NN-MMM ids from a story block's `- Deps:` line (if any).

    Free text (RFC/decision refs, prose, `—`) is ignored; ids are de-duped in
    first-seen order and the story's own id is never treated as a self-dep.
    """
    deps: list[str] = []
    for line in block:
        dm = DEPS_RE.match(line)
        if not dm:
            continue
        for did in DEP_ID_RE.findall(dm.group(1)):
            if did != self_id and did not in deps:
                deps.append(did)
    return deps


def parse_epic(path: Path, epic_num: str) -> list[dict]:
    """Parse one STORIES.md into a list of story dicts (deterministic order)."""
    stories: list[dict] = []
    lines = path.read_text(encoding="utf-8").splitlines()
    cur = None            # (id, epic, title)
    block: list[str] = []  # lines after the heading (exclusive of heading)

    def flush() -> None:
        if cur is None:
            return
        meta_idx = next((i for i, ln in enumerate(block)
                         if META_RE.match(ln)), None)
        if meta_idx is None:  # no Meta line -> not a real story block
            return
        meta = parse_meta(META_RE.match(block[meta_idx]).group(1))
        # `text` is byte-identical to the pre-Deps parser: title + the lines
        # between the heading and the Meta line (keyword matching relies on it).
        text = "\n".join([cur[2]] + block[:meta_idx])
        stories.append({
            "id": cur[0], "epic": cur[1], "num": int(cur[0][-3:]),
            "title": cur[2], "text": text,
            "Type": meta.get("Type", ""), "Size": meta.get("Size", ""),
            "Status": meta.get("Status", ""), "Prio": meta.get("Prio", ""),
            "deps": parse_deps(block, cur[0]),
        })

    for line in lines:
        h = HEADING_RE.match(line)
        if h:
            flush()
            cur = (h.group(1), h.group(2), h.group(4).strip())
            block = []
            continue
        if cur is not None:
            block.append(line)
    flush()
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


def dep_report(story: dict, by_id: dict, warn: bool = True) -> tuple:
    """Classify a story's deps against the satisfaction rule.

    Returns (blocking, in_review, statuses): `blocking` is the list of
    `S-..=STATUS` strings preventing eligibility (unsatisfied or MISSING);
    `in_review` is the list of dep ids satisfied-with-note (IN_REVIEW);
    `statuses` is the display list of every dep's `S-..=STATUS`. Nonexistent
    dep ids warn on stderr and count as blocking.
    """
    blocking: list[str] = []
    in_review: list[str] = []
    statuses: list[str] = []
    for d in story["deps"]:
        s = by_id.get(d)
        if s is None:
            if warn:
                print(f"allocation: warning: {story['id']} depends on unknown "
                      f"story {d}", file=sys.stderr)
            blocking.append(f"{d}=MISSING")
            statuses.append(f"{d}=MISSING")
            continue
        st = s["Status"] or "?"
        statuses.append(f"{d}={st}")
        if st in SATISFIED:
            if st == "IN_REVIEW":
                in_review.append(d)
        else:
            blocking.append(f"{d}={st}")
    return blocking, in_review, statuses


def cmd_next(stories: list[dict], n: int) -> int:
    """Print the eligible frontier: DRAFT/READY stories with all deps satisfied."""
    by_id = {s["id"]: s for s in stories}
    rows = []
    for s in stories:
        if s["Status"] not in FRONTIER_STATUS:
            continue
        blocking, in_review, _ = dep_report(s, by_id)
        if blocking:
            continue
        tier, basis, _ = resolve(s)
        rows.append((s, tier, basis, in_review))
    rows.sort(key=lambda r: (PRIO_ORDER.get(r[0]["Prio"], 9), r[0]["id"]))
    rows = rows[:n]

    print(f"eligible frontier — {len(rows)} shown "
          f"(DRAFT/READY, all deps satisfied)")
    print(f"{'ID':11} {'Prio':4} {'tier(basis)':20} {'orchestrator':13} title")
    print("-" * 92)
    for s, tier, basis, in_review in rows:
        r = RECIPE[recipe_tier(tier, s)]
        o_model, o_effort = r["orch"]
        orch = f"{short(o_model)}@{o_effort}"
        title = s["title"] if len(s["title"]) <= 50 else s["title"][:47] + "..."
        note = ""
        if in_review:
            note += f"  (dep in review: {', '.join(in_review)})"
        marker = ""
        if tier in ("T1", "T1?"):
            marker = ">> "
            note += "  [T1 — interactive only, never headless]"
        idcell = marker + s["id"]
        print(f"{idcell:11} {s['Prio']:4} {tier + ' (' + basis + ')':20} "
              f"{orch:13} {title}{note}")

    q = [s["id"] for s, tier, _, _ in rows if tier not in ("T1", "T1?")][:4]
    print()
    print(f'queue suggestion: make queue Q="{" ".join(q)}"')
    return 0


def cmd_eligible(stories: list[dict], sid: str) -> int:
    """Check a story is runnable: its own status AND its deps.

    Own-status guard (doc 11 §6e): only DRAFT/READY stories may launch —
    DONE means already executed (nothing to do), IN_PROGRESS/IN_REVIEW means
    in flight elsewhere, BLOCKED means explicitly parked. Exit 5.
    Dep guard: unsatisfied dependencies. Exit 4.
    """
    by_id = {s["id"]: s for s in stories}
    s = by_id.get(sid)
    if s is None:
        print(f"allocation: story {sid} not found", file=sys.stderr)
        return 2
    status = s.get("Status", "")
    if status not in FRONTIER_STATUS:
        reason = {
            "DONE": "already executed — nothing to do",
            "IN_PROGRESS": "in flight (another session/worker owns it)",
            "IN_REVIEW": "executed and parked pending review evidence",
            "BLOCKED": "explicitly parked (see its Deps line)",
        }.get(status, f"status {status} is not runnable")
        print(f"eligible: no — {sid} is {status}: {reason}", file=sys.stderr)
        return 5
    blocking, in_review, statuses = dep_report(s, by_id)
    depstr = ", ".join(statuses) if statuses else "none"
    if not blocking:
        note = (f" [satisfied-with-note: in review {', '.join(in_review)}]"
                if in_review else "")
        print(f"eligible: yes (deps: {depstr}){note}")
        return 0
    print(f"eligible: no — blocked by: {', '.join(blocking)} (deps: {depstr})",
          file=sys.stderr)
    print("run `python3 tools/allocation.py --next` for the eligible frontier",
          file=sys.stderr)
    return 4


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    g = ap.add_mutually_exclusive_group(required=True)
    g.add_argument("--story", metavar="S-NN-MMM", help="print a recipe block")
    g.add_argument("--generate", action="store_true", help="write ALLOCATION_MAP.md")
    g.add_argument("--check", action="store_true", help="verify the map is current")
    g.add_argument("--audit", action="store_true", help="print tier counts")
    g.add_argument("--next", nargs="?", type=int, const=10, metavar="N",
                   dest="next_n", help="print the eligible dependency frontier")
    g.add_argument("--eligible", metavar="S-NN-MMM",
                   help="check whether a story's deps are satisfied")
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
    if args.next_n is not None:
        return cmd_next(stories, args.next_n)
    if args.eligible:
        return cmd_eligible(stories, args.eligible)
    return 0


if __name__ == "__main__":
    sys.exit(main())
