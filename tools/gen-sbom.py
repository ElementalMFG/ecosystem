#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2026 SS-SP Project Contributors
"""Generate a CycloneDX 1.6 JSON SBOM for one firmware artifact.

Contract of record: docs/dev/contracts/S-02-019-sbom-contract.md (FROZEN).
One SBOM per board (`lite`/`alpha`/`omega`) is produced from the build
output directory plus the repo tree. Stdlib only — no pip dependencies, no
network, no ESP-IDF installation required. See the contract for the exact
document structure (§3), determinism rules (§7), and validation rules (§8).

Usage:
  gen-sbom.py --board {lite,alpha,omega} --build-dir DIR
              [--output PATH] [--version-source SPEC] [--repo-root DIR]

Exit codes (contract §2): 0 success; 2 usage (argparse); 3 missing/unreadable
input or unresolvable version under git|env; 4 internal invariant violation
(duplicate bom-ref, unresolvable ss_* dependency token). No other codes.
"""

import argparse
import hashlib
import json
import os
import re
import subprocess
import sys
import uuid
from datetime import datetime, timezone
from pathlib import Path

# Repo root default matches gen-stories-index.py (tools/ -> repo root).
DEFAULT_REPO_ROOT = Path(__file__).resolve().parent.parent

# Frozen SBOM namespace (contract §3.1). Never uuid4.
SS_SBOM_NS = uuid.uuid5(uuid.NAMESPACE_DNS, "sbom.ss-sp.elementalmfg")

# Repo slug used in purls / attestation URLs (contract-wide constant).
REPO_SLUG = "ElementalMFG/ecosystem"

# Documented placeholder for unresolved version fields (contract §6.4).
UNKNOWN = "UNKNOWN"

BOARDS = ("lite", "alpha", "omega")
FIRMWARE_BIN = "ss_sp_firmware.bin"
BOOTLOADER_BIN = "bootloader/bootloader.bin"
PARTTABLE_BIN = "partition_table/partition-table.bin"

_SHA40_RE = re.compile(r"^[0-9a-f]{40}$")
_UUID_RE = re.compile(r"^urn:uuid:[0-9a-f]{8}(-[0-9a-f]{4}){3}-[0-9a-f]{12}$")
_RFC3339_Z_RE = re.compile(r"^\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}Z$")

RESERVED_PREFIXES = (
    "ss-sp:firmware:",
    "ss-sp:component:",
    "ss-sp:artifact:",
    "ss-sp:dep:",
)


class InputError(Exception):
    """Missing/unreadable input or unresolvable version (exit 3)."""


class InvariantError(Exception):
    """Internal invariant violation such as a duplicate bom-ref (exit 4)."""


# --------------------------------------------------------------------------
# Pure helpers (importable / host-tested directly)
# --------------------------------------------------------------------------

def sbom_filename(board: str, version: str) -> str:
    """Return the canonical SBOM filename (contract §5).

    Pre: board and version are strings. Post: `<version>` is sanitized so
    every char outside [A-Za-z0-9._-] becomes '-' (so '/', '+', space cannot
    appear); the returned name has no path separators.
    """
    safe = re.sub(r"[^A-Za-z0-9._-]", "-", version)
    return f"ss_sp_firmware-{board}-{safe}.cdx.json"


def enumerate_components(repo_root: Path) -> list[str]:
    """Return sorted first-party component dir names (contract §3.4).

    A first-party component is any directory matching
    `firmware/components/ss_*` that contains a `CMakeLists.txt`. Enumerated
    at runtime — never hardcoded — so the set tracks the tree.

    Raises InputError (exit 3) if the components directory is missing.
    """
    comp_dir = Path(repo_root) / "firmware" / "components"
    if not comp_dir.is_dir():
        raise InputError(f"components dir not found: {comp_dir}")
    names = [
        p.name
        for p in comp_dir.iterdir()
        if p.is_dir()
        and p.name.startswith("ss_")
        and (p / "CMakeLists.txt").is_file()
    ]
    return sorted(names)


def parse_component_requires(cmake_text: str) -> list[str]:
    """Return REQUIRES + PRIV_REQUIRES tokens from a CMakeLists (contract §3.6).

    Parses the `idf_component_register(...)` call body (whitespace/newline
    tolerant). Bareword tokens following a REQUIRES or PRIV_REQUIRES keyword
    are collected until the next all-caps keyword. Order preserved; caller
    dedups.
    """
    m = re.search(
        r"idf_component_register\s*\((.*?)\)", cmake_text, re.DOTALL
    )
    if not m:
        return []
    body = m.group(1).replace('"', " ")
    tokens = body.split()
    kw_re = re.compile(r"^[A-Z][A-Z0-9_]*$")
    out: list[str] = []
    collecting = False
    for tok in tokens:
        if kw_re.match(tok):
            collecting = tok in ("REQUIRES", "PRIV_REQUIRES")
            continue
        if collecting:
            out.append(tok)
    return out


def _sha256_hex(path: Path) -> str:
    h = hashlib.sha256()
    with open(path, "rb") as fh:
        for chunk in iter(lambda: fh.read(65536), b""):
            h.update(chunk)
    return h.hexdigest()


def _rfc3339_utc(epoch: int) -> str:
    return datetime.fromtimestamp(int(epoch), timezone.utc).strftime(
        "%Y-%m-%dT%H:%M:%SZ"
    )


def _mark_unresolved(comp: dict) -> dict:
    """Flag a component whose version/purl carries an UNKNOWN placeholder.

    Adds `ss:version-unresolved=true` (contract §6.4) and keeps `properties`
    sorted by name (contract §7).
    """
    fields = [comp.get("version", "")]
    if "purl" in comp:
        fields.append(comp["purl"])
    if any(UNKNOWN in v for v in fields):
        comp.setdefault("properties", []).append(
            {"name": "ss:version-unresolved", "value": "true"}
        )
    if "properties" in comp:
        comp["properties"] = sorted(comp["properties"], key=lambda p: p["name"])
    return comp


# --------------------------------------------------------------------------
# Version sourcing (contract §6)
# --------------------------------------------------------------------------

def _git(repo_root: Path, *args: str) -> str:
    return subprocess.run(
        ["git", "-C", str(repo_root), *args],
        check=True,
        capture_output=True,
        text=True,
    ).stdout.strip()


def resolve_fw_version(source: str, repo_root: Path) -> tuple[str, str, bool]:
    """Resolve (fw_version, git_sha, unresolved) per contract §6.1.

    `unresolved` is True when placeholders were used (auto path only).
    Raises InputError (exit 3) for git/env failures.
    """
    env = os.environ

    def _from_env() -> tuple[str, str] | None:
        v, s = env.get("SS_FW_VERSION"), env.get("SS_FW_GIT_SHA")
        if not v or not s:
            return None
        if not _SHA40_RE.match(s):
            raise InputError("SS_FW_GIT_SHA must be a 40-hex commit SHA")
        return v, s

    def _from_git() -> tuple[str, str]:
        try:
            v = _git(repo_root, "describe", "--tags", "--always", "--dirty")
            s = _git(repo_root, "rev-parse", "HEAD")
        except (subprocess.CalledProcessError, OSError) as exc:
            raise InputError(f"git version resolution failed: {exc}") from exc
        return v, s

    if source == "git":
        v, s = _from_git()
        return v, s, False

    if source == "env":
        got = _from_env()
        if got is None:
            raise InputError(
                "version-source env requires SS_FW_VERSION and SS_FW_GIT_SHA"
            )
        return got[0], got[1], False

    if source.startswith("literal:"):
        v = source[len("literal:"):]
        s = env.get("SS_FW_GIT_SHA")
        if s:
            if not _SHA40_RE.match(s):
                raise InputError("SS_FW_GIT_SHA must be a 40-hex commit SHA")
            return v, s, False
        return v, UNKNOWN, True

    # auto (default): env, else git, else placeholders (exit 0 + warning).
    got = _from_env()
    if got is not None:
        return got[0], got[1], False
    try:
        v, s = _from_git()
        return v, s, False
    except InputError:
        print(
            "warning: fw_version unresolved (no env, no git) — using UNKNOWN",
            file=sys.stderr,
        )
        return UNKNOWN, UNKNOWN, True


def _load_project_description(build_dir: Path) -> dict:
    pd = build_dir / "project_description.json"
    if not pd.is_file():
        return {}
    try:
        with open(pd, encoding="utf-8") as fh:
            return json.load(fh)
    except (OSError, ValueError):
        return {}


def resolve_idf_version(build_dir: Path) -> str:
    """Resolve the ESP-IDF version (contract §6.2).

    Precedence: $IDF_VERSION env; then project_description.json. IDF v5.3.5
    emits the version under the `git_revision` key (verified against a real
    v5.3.5 build: git_revision == "v5.3.5"; there is no `idf_version` key).
    `idf_version` is retained first only as forward-compat for future IDF
    releases; `git_revision` is the key that actually applies today.
    """
    env = os.environ.get("IDF_VERSION")
    if env:
        return env
    pd = _load_project_description(build_dir)
    for key in ("idf_version", "git_revision"):
        val = pd.get(key)
        if val:
            return str(val)
    return UNKNOWN


def resolve_toolchain_version(build_dir: Path) -> str:
    """Resolve the xtensa toolchain version (contract §6.3).

    Precedence: $SS_TOOLCHAIN_VERSION; then parse the compiler path in
    project_description.json (`.../xtensa-esp-elf/esp-<ver>/...`).
    """
    env = os.environ.get("SS_TOOLCHAIN_VERSION")
    if env:
        return env
    pd = _load_project_description(build_dir)
    compiler = pd.get("c_compiler") or pd.get("cxx_compiler") or ""
    m = re.search(r"xtensa-esp-elf/esp-([^/]+)", str(compiler))
    if m:
        return m.group(1)
    return UNKNOWN


# --------------------------------------------------------------------------
# Document builder (contract §3)
# --------------------------------------------------------------------------

def build_document(
    board: str,
    build_dir: Path,
    repo_root: Path,
    version_source: str,
) -> dict:
    """Build the CycloneDX 1.6 document (contract §3). Raises InputError /
    InvariantError for exit 3 / exit 4 conditions."""
    if not build_dir.is_dir():
        raise InputError(f"build dir not found: {build_dir}")
    bin_path = build_dir / FIRMWARE_BIN
    if not bin_path.is_file():
        raise InputError(f"firmware image not found: {bin_path}")

    bin_sha = _sha256_hex(bin_path)
    fw_version, git_sha, _unresolved = resolve_fw_version(
        version_source, repo_root
    )
    idf_version = resolve_idf_version(build_dir)
    toolchain_version = resolve_toolchain_version(build_dir)

    fw_purl = f"pkg:github/{REPO_SLUG}@{git_sha}?board={board}"
    fw_ref = f"ss-sp:firmware:{board}"

    components: list[dict] = []

    # First-party components (contract §3.4).
    comp_names = enumerate_components(repo_root)
    comp_dir = repo_root / "firmware" / "components"
    comp_deps: dict[str, list[str]] = {}
    for name in comp_names:
        components.append(
            _mark_unresolved({
                "type": "library",
                "bom-ref": f"ss-sp:component:{name}",
                "name": name,
                "version": fw_version,
                "supplier": {"name": "SS-SP Project"},
                "purl": (
                    f"pkg:github/{REPO_SLUG}@{git_sha}"
                    f"#firmware/components/{name}"
                ),
            })
        )
        cmake = (comp_dir / name / "CMakeLists.txt").read_text(encoding="utf-8")
        comp_deps[name] = parse_component_requires(cmake)

    # Third-party (contract §3.4).
    components.append(
        _mark_unresolved({
            "type": "framework",
            "bom-ref": "ss-sp:dep:esp-idf",
            "name": "esp-idf",
            "version": idf_version,
            "supplier": {"name": "Espressif Systems"},
            "purl": f"pkg:github/espressif/esp-idf@{idf_version}",
        })
    )
    components.append(
        _mark_unresolved({
            "type": "application",
            "bom-ref": "ss-sp:dep:xtensa-toolchain",
            "name": "xtensa-esp-elf-gcc",
            "version": toolchain_version,
            "supplier": {"name": "Espressif Systems"},
            "purl": (
                "pkg:generic/espressif/xtensa-esp-elf@"
                f"{toolchain_version}"
            ),
        })
    )

    # Secondary artifacts (contract §3.5) — warn (not error) when absent.
    boot_path = build_dir / BOOTLOADER_BIN
    if boot_path.is_file():
        components.append(
            _mark_unresolved({
                "type": "firmware",
                "bom-ref": "ss-sp:artifact:bootloader",
                "name": "bootloader",
                "version": idf_version,
                "supplier": {"name": "Espressif Systems"},
                "purl": (
                    f"pkg:github/espressif/esp-idf@{idf_version}"
                    "#components/bootloader"
                ),
                "hashes": [
                    {"alg": "SHA-256", "content": _sha256_hex(boot_path)}
                ],
            })
        )
    else:
        print(f"warning: {BOOTLOADER_BIN} absent — omitting", file=sys.stderr)

    part_path = build_dir / PARTTABLE_BIN
    if part_path.is_file():
        components.append(
            _mark_unresolved({
                "type": "firmware",
                "bom-ref": "ss-sp:artifact:partition-table",
                "name": "partition-table",
                "version": fw_version,
                "supplier": {"name": "SS-SP Project"},
                "purl": f"{fw_purl}#firmware/partitions",
                "hashes": [
                    {"alg": "SHA-256", "content": _sha256_hex(part_path)}
                ],
            })
        )
    else:
        print(f"warning: {PARTTABLE_BIN} absent — omitting", file=sys.stderr)

    # metadata.component — the firmware artifact (contract §3.3).
    meta_component = _mark_unresolved({
        "type": "firmware",
        "bom-ref": fw_ref,
        "name": "ss_sp_firmware",
        "version": fw_version,
        "supplier": {"name": "SS-SP Project"},
        "purl": fw_purl,
        "hashes": [{"alg": "SHA-256", "content": bin_sha}],
        "properties": [
            {"name": "ss:board", "value": board},
            {"name": "ss:artifact", "value": FIRMWARE_BIN},
        ],
    })

    # bom-ref uniqueness (contract §3.5 invariant → exit 4).
    all_refs = [fw_ref] + [c["bom-ref"] for c in components]
    if len(all_refs) != len(set(all_refs)):
        raise InvariantError("duplicate bom-ref detected")
    ref_set = set(all_refs)

    # dependencies graph (contract §3.6).
    dependencies: list[dict] = []
    other_refs = sorted(r for r in all_refs if r != fw_ref)
    dependencies.append({"ref": fw_ref, "dependsOn": other_refs})

    for name in comp_names:
        resolved: set[str] = set()
        for tok in comp_deps[name]:
            if tok.startswith("ss_"):
                dep_ref = f"ss-sp:component:{tok}"
                if dep_ref not in ref_set:
                    raise InvariantError(
                        f"component {name} REQUIRES unknown ss_* '{tok}'"
                    )
                resolved.add(dep_ref)
            else:
                resolved.add("ss-sp:dep:esp-idf")
        dependencies.append({
            "ref": f"ss-sp:component:{name}",
            "dependsOn": sorted(resolved),
        })

    for ref in all_refs:
        if ref == fw_ref or ref.startswith("ss-sp:component:"):
            continue
        dependencies.append({"ref": ref, "dependsOn": []})

    dependencies.sort(key=lambda d: d["ref"])

    # metadata (contract §3.2). Timestamp only when SOURCE_DATE_EPOCH set.
    metadata: dict = {
        "tools": {
            "components": [{
                "type": "application",
                "name": "gen-sbom.py",
                "version": fw_version,
                "supplier": {"name": "SS-SP Project"},
            }]
        },
        "component": meta_component,
    }
    sde = os.environ.get("SOURCE_DATE_EPOCH")
    if sde:
        try:
            metadata["timestamp"] = _rfc3339_utc(int(sde))
        except (ValueError, OverflowError, OSError) as exc:
            raise InputError(f"invalid SOURCE_DATE_EPOCH: {exc}") from exc

    serial = "urn:uuid:" + str(
        uuid.uuid5(SS_SBOM_NS, f"{board}:{fw_version}:{bin_sha}")
    )

    return {
        "bomFormat": "CycloneDX",
        "specVersion": "1.6",
        "serialNumber": serial,
        "version": 1,
        "metadata": metadata,
        "components": sorted(components, key=lambda c: c["bom-ref"]),
        "dependencies": dependencies,
    }


def serialize(doc: dict) -> str:
    """Deterministic serialization (contract §7): sorted keys, 2-space indent,
    UTF-8, LF, single trailing newline."""
    return json.dumps(doc, indent=2, sort_keys=True, ensure_ascii=False) + "\n"


# --------------------------------------------------------------------------
# Validation (contract §8)
# --------------------------------------------------------------------------

def validate_sbom(doc: dict, repo_root: Path | None = None) -> list[str]:
    """Return a list of validation error strings (empty == valid); contract §8.

    Pre: `doc` is a parsed SBOM dict. `repo_root` (default DEFAULT_REPO_ROOT)
    supplies the tree used for the V4 component-completeness cross-check.
    """
    root = Path(repo_root) if repo_root is not None else DEFAULT_REPO_ROOT
    errs: list[str] = []

    # V1: top-level shape.
    expect_keys = {
        "bomFormat", "specVersion", "serialNumber", "version",
        "metadata", "components", "dependencies",
    }
    if set(doc) != expect_keys:
        errs.append(f"V1: top-level keys {sorted(doc)} != {sorted(expect_keys)}")
        return errs
    if doc["bomFormat"] != "CycloneDX":
        errs.append("V1: bomFormat != CycloneDX")
    if doc["specVersion"] != "1.6":
        errs.append("V1: specVersion != 1.6")
    if doc["version"] != 1:
        errs.append("V1: version != 1")

    mc = doc["metadata"].get("component", {})

    # V2: serialNumber regex + §3.1 recomputation from the doc's own data.
    serial = doc["serialNumber"]
    if not _UUID_RE.match(serial):
        errs.append("V2: serialNumber malformed")
    else:
        board = next(
            (p["value"] for p in mc.get("properties", [])
             if p["name"] == "ss:board"),
            None,
        )
        hashes = mc.get("hashes", [])
        bin_sha = hashes[0]["content"] if hashes else None
        if board and bin_sha:
            expect = "urn:uuid:" + str(
                uuid.uuid5(SS_SBOM_NS, f"{board}:{mc.get('version')}:{bin_sha}")
            )
            if serial != expect:
                errs.append("V2: serialNumber != recomputed value")

    # V3: metadata.component shape.
    if mc.get("type") != "firmware":
        errs.append("V3: metadata.component.type != firmware")
    for f in ("name", "version", "purl"):
        if not mc.get(f):
            errs.append(f"V3: metadata.component.{f} empty")
    h = mc.get("hashes", [])
    if len(h) != 1 or h[0].get("alg") != "SHA-256" or not re.match(
        r"^[0-9a-f]{64}$", h[0].get("content", "")
    ):
        errs.append("V3: metadata.component.hashes not exactly one SHA-256")

    refs = [c["bom-ref"] for c in doc["components"]]
    ref_set = set(refs)

    # V4: component completeness against the tree.
    try:
        for name in enumerate_components(root):
            if f"ss-sp:component:{name}" not in ref_set:
                errs.append(f"V4: missing component entry for {name}")
    except InputError as exc:
        errs.append(f"V4: {exc}")
    for dep in ("ss-sp:dep:esp-idf", "ss-sp:dep:xtensa-toolchain"):
        if dep not in ref_set:
            errs.append(f"V4: missing {dep}")

    # V5: bom-ref uniqueness + dependency-graph closure.
    if len(refs) != len(ref_set):
        errs.append("V5: duplicate bom-ref in components")
    fw_ref = mc.get("bom-ref")
    all_refs = ref_set | ({fw_ref} if fw_ref else set())
    dep_refs = [d["ref"] for d in doc["dependencies"]]
    for d in doc["dependencies"]:
        if d["ref"] not in all_refs:
            errs.append(f"V5: dependencies ref {d['ref']} unknown")
        for on in d["dependsOn"]:
            if on not in all_refs:
                errs.append(f"V5: dependsOn {on} unresolved")
    if set(dep_refs) != all_refs:
        errs.append("V5: not every bom-ref has a dependencies entry")
    if len(dep_refs) != len(set(dep_refs)):
        errs.append("V5: duplicate dependencies ref")

    # V6: sortedness + timestamp form.
    if refs != sorted(refs):
        errs.append("V6: components[] not sorted by bom-ref")
    if dep_refs != sorted(dep_refs):
        errs.append("V6: dependencies[] not sorted by ref")
    for d in doc["dependencies"]:
        if d["dependsOn"] != sorted(d["dependsOn"]):
            errs.append(f"V6: dependsOn of {d['ref']} not sorted")
    for c in doc["components"] + [mc]:
        props = c.get("properties")
        if props and [p["name"] for p in props] != sorted(
            p["name"] for p in props
        ):
            errs.append(f"V6: properties of {c.get('bom-ref')} not sorted")
    ts = doc["metadata"].get("timestamp")
    if ts is not None and not _RFC3339_Z_RE.match(ts):
        errs.append("V6: timestamp not RFC3339 UTC Z form")

    # V7: purl prefix + component type domain.
    valid_types = {"firmware", "library", "framework", "application"}
    for c in doc["components"] + [mc]:
        if c.get("type") not in valid_types:
            errs.append(f"V7: bad type on {c.get('bom-ref')}")
        purl = c.get("purl")
        if purl is not None and not purl.startswith("pkg:"):
            errs.append(f"V7: purl on {c.get('bom-ref')} missing pkg: prefix")

    return errs


# --------------------------------------------------------------------------
# CLI
# --------------------------------------------------------------------------

def _write_atomic(path: Path, text: str) -> None:
    parent = path.parent
    if not parent.is_dir():
        raise InputError(f"output parent dir does not exist: {parent}")
    tmp = parent / (path.name + f".tmp.{os.getpid()}")
    try:
        with open(tmp, "w", encoding="utf-8", newline="\n") as fh:
            fh.write(text)
        os.replace(tmp, path)
    except BaseException:
        if tmp.exists():
            tmp.unlink()
        raise


def _version_source(spec: str) -> str:
    if spec in ("auto", "env", "git") or spec.startswith("literal:"):
        return spec
    raise argparse.ArgumentTypeError(
        "must be auto|env|git|literal:<string>"
    )


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(prog="gen-sbom.py")
    ap.add_argument("--board", required=True, choices=BOARDS)
    ap.add_argument("--build-dir", required=True)
    ap.add_argument("--output", default=None)
    ap.add_argument(
        "--version-source", default="auto", type=_version_source
    )
    ap.add_argument("--repo-root", default=None)
    args = ap.parse_args(argv)

    repo_root = (
        Path(args.repo_root).resolve()
        if args.repo_root
        else DEFAULT_REPO_ROOT
    )
    build_dir = Path(args.build_dir)

    try:
        doc = build_document(
            args.board, build_dir, repo_root, args.version_source
        )
        text = serialize(doc)
        if args.output == "-":
            sys.stdout.write(text)
        else:
            out = (
                Path(args.output)
                if args.output
                else build_dir / sbom_filename(args.board, doc["metadata"]["component"]["version"])
            )
            _write_atomic(out, text)
            print(f"wrote {out}")
    except InputError as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 3
    except InvariantError as exc:
        print(f"internal error: {exc}", file=sys.stderr)
        return 4
    return 0


if __name__ == "__main__":
    sys.exit(main())
