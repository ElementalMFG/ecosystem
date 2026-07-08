# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2026 SS-SP Project Contributors
"""Host tests for tools/gen-sbom.py (contract §10).

Pure pytest, stdlib + pytest only. No network/container/hardware. The real
repo tree is used for component enumeration; fake build dirs are synthesized
under tmp_path. The generator is exercised both by import (pure functions)
and as a subprocess (CLI / exit-code behavior).
"""

import hashlib
import importlib.util
import json
import os
import subprocess
import sys
from pathlib import Path

import pytest

REPO = Path(__file__).resolve().parents[2]
GEN = REPO / "tools" / "gen-sbom.py"

_spec = importlib.util.spec_from_file_location("gen_sbom", GEN)
g = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(g)

FW_BYTES = b"SS-SP-FAKE-FIRMWARE-IMAGE-0001"
FW_SHA = hashlib.sha256(FW_BYTES).hexdigest()
BOOT_BYTES = b"FAKE-BOOTLOADER"
PART_BYTES = b"FAKE-PARTITION-TABLE"


# --------------------------------------------------------------------------
# Fixtures
# --------------------------------------------------------------------------

def make_build_dir(root, *, fw=FW_BYTES, bootloader=False, parttable=False,
                   project_description=None):
    root.mkdir(parents=True, exist_ok=True)
    (root / "ss_sp_firmware.bin").write_bytes(fw)
    if bootloader:
        (root / "bootloader").mkdir(exist_ok=True)
        (root / "bootloader" / "bootloader.bin").write_bytes(BOOT_BYTES)
    if parttable:
        (root / "partition_table").mkdir(exist_ok=True)
        (root / "partition_table" / "partition-table.bin").write_bytes(PART_BYTES)
    if project_description is not None:
        (root / "project_description.json").write_text(
            json.dumps(project_description), encoding="utf-8"
        )
    return root


def make_fake_repo(root, components):
    """components: {name: cmake_text}. Creates firmware/components/<name>/."""
    cdir = root / "firmware" / "components"
    cdir.mkdir(parents=True, exist_ok=True)
    for name, text in components.items():
        (cdir / name).mkdir(parents=True, exist_ok=True)
        if text is not None:
            (cdir / name / "CMakeLists.txt").write_text(text, encoding="utf-8")
    return root


def run_cli(*args, env=None):
    e = dict(os.environ)
    # Isolate from the ambient CI environment unless a test opts in.
    for k in ("SS_FW_VERSION", "SS_FW_GIT_SHA", "IDF_VERSION",
              "SS_TOOLCHAIN_VERSION", "SOURCE_DATE_EPOCH"):
        e.pop(k, None)
    if env:
        e.update(env)
    return subprocess.run(
        [sys.executable, str(GEN), *args],
        capture_output=True, text=True, env=e,
    )


def build(board, build_dir, repo_root=REPO, source="literal:v1.2.3",
          env=None):
    old = {}
    keys = ("SS_FW_VERSION", "SS_FW_GIT_SHA", "IDF_VERSION",
            "SS_TOOLCHAIN_VERSION", "SOURCE_DATE_EPOCH")
    for k in keys:
        old[k] = os.environ.pop(k, None)
    try:
        if env:
            os.environ.update(env)
        return g.build_document(board, build_dir, Path(repo_root), source)
    finally:
        for k in keys:
            os.environ.pop(k, None)
            if old[k] is not None:
                os.environ[k] = old[k]


# --------------------------------------------------------------------------
# §10.1 Structure
# --------------------------------------------------------------------------

def test_structure_passes_validator(tmp_path):
    bd = make_build_dir(tmp_path / "b", bootloader=True, parttable=True)
    doc = build("lite", bd, env={"SS_FW_GIT_SHA": "a" * 40})
    assert g.validate_sbom(doc, REPO) == []
    assert doc["bomFormat"] == "CycloneDX"
    assert doc["specVersion"] == "1.6"
    assert doc["version"] == 1


# --------------------------------------------------------------------------
# §10.2 Determinism
# --------------------------------------------------------------------------

def test_determinism_byte_identical(tmp_path):
    bd = make_build_dir(tmp_path / "b")
    out1 = tmp_path / "one.cdx.json"
    out2 = tmp_path / "two.cdx.json"
    r1 = run_cli("--board", "lite", "--build-dir", str(bd),
                 "--version-source", "literal:v1", "--output", str(out1),
                 env={"PYTHONHASHSEED": "1"})
    r2 = run_cli("--board", "lite", "--build-dir", str(bd),
                 "--version-source", "literal:v1", "--output", str(out2),
                 env={"PYTHONHASHSEED": "999"})
    assert r1.returncode == 0 and r2.returncode == 0
    assert out1.read_bytes() == out2.read_bytes()


def test_timestamp_from_source_date_epoch(tmp_path):
    bd = make_build_dir(tmp_path / "b")
    doc = build("lite", bd, env={"SOURCE_DATE_EPOCH": "1700000000"})
    assert doc["metadata"]["timestamp"] == "2023-11-14T22:13:20Z"


def test_timestamp_omitted_when_unset(tmp_path):
    bd = make_build_dir(tmp_path / "b")
    doc = build("lite", bd)
    assert "timestamp" not in doc["metadata"]


# --------------------------------------------------------------------------
# §10.3 Component completeness
# --------------------------------------------------------------------------

def test_component_set_equals_real_tree(tmp_path):
    bd = make_build_dir(tmp_path / "b")
    doc = build("lite", bd)
    expected = {
        f"ss-sp:component:{n}" for n in g.enumerate_components(REPO)
    }
    got = {
        c["bom-ref"] for c in doc["components"]
        if c["bom-ref"].startswith("ss-sp:component:")
    }
    assert got == expected
    assert expected  # tree must yield at least one component today


def test_added_component_appears_without_code_change(tmp_path):
    fake = make_fake_repo(tmp_path / "repo", {
        "ss_aaa": "idf_component_register(REQUIRES esp_timer)",
        "ss_zz_test": "idf_component_register(REQUIRES esp_timer)",
        "ss_skeleton": None,          # no CMakeLists -> excluded
        "not_ss": "idf_component_register()",  # wrong prefix -> excluded
    })
    bd = make_build_dir(tmp_path / "b")
    doc = build("lite", bd, repo_root=fake)
    got = {
        c["bom-ref"] for c in doc["components"]
        if c["bom-ref"].startswith("ss-sp:component:")
    }
    assert got == {"ss-sp:component:ss_aaa", "ss-sp:component:ss_zz_test"}


# --------------------------------------------------------------------------
# §10.4 Board variance
# --------------------------------------------------------------------------

def test_board_variance(tmp_path):
    bd_l = make_build_dir(tmp_path / "l")
    bd_a = make_build_dir(tmp_path / "a")
    lite = build("lite", bd_l)
    alpha = build("alpha", bd_a)

    # Component list identical.
    assert [c["bom-ref"] for c in lite["components"]] == \
           [c["bom-ref"] for c in alpha["components"]]
    # Firmware node differs.
    assert lite["serialNumber"] != alpha["serialNumber"]
    assert lite["metadata"]["component"]["bom-ref"] != \
        alpha["metadata"]["component"]["bom-ref"]
    assert lite["metadata"]["component"]["purl"] != \
        alpha["metadata"]["component"]["purl"]
    lite_board = [p["value"] for p in lite["metadata"]["component"]["properties"]
                  if p["name"] == "ss:board"][0]
    assert lite_board == "lite"
    # Dependencies differ only in the firmware ref key.
    lite_deps = {d["ref"] for d in lite["dependencies"]}
    alpha_deps = {d["ref"] for d in alpha["dependencies"]}
    assert lite_deps ^ alpha_deps == {
        "ss-sp:firmware:lite", "ss-sp:firmware:alpha"
    }


# --------------------------------------------------------------------------
# §10.5 Hashes
# --------------------------------------------------------------------------

def test_firmware_hash_matches(tmp_path):
    bd = make_build_dir(tmp_path / "b")
    doc = build("lite", bd)
    assert doc["metadata"]["component"]["hashes"][0]["content"] == FW_SHA


def test_secondary_artifacts_present_and_absent(tmp_path):
    bd = make_build_dir(tmp_path / "b", bootloader=True, parttable=True)
    doc = build("lite", bd)
    refs = {c["bom-ref"]: c for c in doc["components"]}
    boot = refs["ss-sp:artifact:bootloader"]
    part = refs["ss-sp:artifact:partition-table"]
    assert boot["hashes"][0]["content"] == hashlib.sha256(BOOT_BYTES).hexdigest()
    assert part["hashes"][0]["content"] == hashlib.sha256(PART_BYTES).hexdigest()

    bd2 = make_build_dir(tmp_path / "b2")
    r = run_cli("--board", "lite", "--build-dir", str(bd2),
                "--version-source", "literal:v1", "--output", "-")
    assert r.returncode == 0
    doc2 = json.loads(r.stdout)
    refs2 = {c["bom-ref"] for c in doc2["components"]}
    assert "ss-sp:artifact:bootloader" not in refs2
    assert "ss-sp:artifact:partition-table" not in refs2


# --------------------------------------------------------------------------
# §10.6 Version sourcing
# --------------------------------------------------------------------------

def test_env_version_source(tmp_path):
    bd = make_build_dir(tmp_path / "b")
    r = run_cli("--board", "lite", "--build-dir", str(bd),
                "--version-source", "env", "--output", "-",
                env={"SS_FW_VERSION": "v9.9.9", "SS_FW_GIT_SHA": "b" * 40})
    assert r.returncode == 0
    doc = json.loads(r.stdout)
    assert doc["metadata"]["component"]["version"] == "v9.9.9"
    assert ("b" * 40) in doc["metadata"]["component"]["purl"]


def test_env_version_missing_exits_3(tmp_path):
    bd = make_build_dir(tmp_path / "b")
    r = run_cli("--board", "lite", "--build-dir", str(bd),
                "--version-source", "env", "--output", "-")
    assert r.returncode == 3


def test_literal_passthrough(tmp_path):
    bd = make_build_dir(tmp_path / "b")
    doc = build("lite", bd, source="literal:v0.0.0-test",
                env={"SS_FW_GIT_SHA": "c" * 40})
    assert doc["metadata"]["component"]["version"] == "v0.0.0-test"


def test_auto_no_git_no_env_marks_unresolved(tmp_path):
    bd = make_build_dir(tmp_path / "b")
    # repo_root without .git -> git fails; no env -> UNKNOWN placeholders.
    fake = make_fake_repo(tmp_path / "repo", {
        "ss_aaa": "idf_component_register(REQUIRES esp_timer)"
    })
    r = run_cli("--board", "lite", "--build-dir", str(bd),
                "--version-source", "auto", "--repo-root", str(fake),
                "--output", "-")
    assert r.returncode == 0
    doc = json.loads(r.stdout)
    mc = doc["metadata"]["component"]
    assert mc["version"] == "UNKNOWN"
    assert any(p["name"] == "ss:version-unresolved" and p["value"] == "true"
               for p in mc["properties"])


# --------------------------------------------------------------------------
# §10.7 CLI errors
# --------------------------------------------------------------------------

def test_missing_build_dir_exits_3(tmp_path):
    r = run_cli("--board", "lite", "--build-dir", str(tmp_path / "nope"),
                "--output", "-")
    assert r.returncode == 3


def test_missing_bin_exits_3_no_output(tmp_path):
    bd = tmp_path / "b"
    bd.mkdir()
    out = tmp_path / "x.cdx.json"
    r = run_cli("--board", "lite", "--build-dir", str(bd), "--output", str(out))
    assert r.returncode == 3
    assert not out.exists()


def test_bad_board_exits_2(tmp_path):
    bd = make_build_dir(tmp_path / "b")
    r = run_cli("--board", "banana", "--build-dir", str(bd), "--output", "-")
    assert r.returncode == 2


def test_stdout_mode_json_only(tmp_path):
    bd = make_build_dir(tmp_path / "b")
    r = run_cli("--board", "lite", "--build-dir", str(bd),
                "--version-source", "literal:v1", "--output", "-")
    assert r.returncode == 0
    json.loads(r.stdout)  # stdout is pure JSON
    assert "warning" in r.stderr  # diagnostics go to stderr


def test_missing_output_parent_exits_3(tmp_path):
    bd = make_build_dir(tmp_path / "b")
    out = tmp_path / "nodir" / "x.cdx.json"
    r = run_cli("--board", "lite", "--build-dir", str(bd),
                "--version-source", "literal:v1", "--output", str(out))
    assert r.returncode == 3
    assert not out.exists()


# --------------------------------------------------------------------------
# §10.8 Filename
# --------------------------------------------------------------------------

def test_filename_sanitization():
    assert g.sbom_filename("lite", "v0.3.0-12-gabc1234") == \
        "ss_sp_firmware-lite-v0.3.0-12-gabc1234.cdx.json"
    assert g.sbom_filename("alpha", "v1.0+build/2 rc") == \
        "ss_sp_firmware-alpha-v1.0-build-2-rc.cdx.json"


# --------------------------------------------------------------------------
# §10.9 Dependency graph
# --------------------------------------------------------------------------

def test_requires_parsing_multiline():
    cmake = """
    idf_component_register(
        SRCS "src/a.c" "src/b.c"
        INCLUDE_DIRS "include"
        REQUIRES ss_hal
                 esp_timer
        PRIV_REQUIRES
            nvs_flash ss_log
    )
    """
    toks = g.parse_component_requires(cmake)
    assert toks == ["ss_hal", "esp_timer", "nvs_flash", "ss_log"]


def test_dependency_mapping_and_collapse(tmp_path):
    fake = make_fake_repo(tmp_path / "repo", {
        "ss_hal": "idf_component_register(REQUIRES driver esp_timer)",
        "ss_log": (
            "idf_component_register(REQUIRES ss_hal\n"
            "  PRIV_REQUIRES nvs_flash esp_timer)"
        ),
    })
    bd = make_build_dir(tmp_path / "b")
    doc = build("lite", bd, repo_root=fake)
    deps = {d["ref"]: d["dependsOn"] for d in doc["dependencies"]}
    assert deps["ss-sp:component:ss_hal"] == ["ss-sp:dep:esp-idf"]
    assert deps["ss-sp:component:ss_log"] == [
        "ss-sp:component:ss_hal", "ss-sp:dep:esp-idf"
    ]


def test_unknown_ss_token_exits_4(tmp_path):
    fake = make_fake_repo(tmp_path / "repo", {
        "ss_aaa": "idf_component_register(REQUIRES ss_ghost)",
    })
    bd = make_build_dir(tmp_path / "b")
    r = run_cli("--board", "lite", "--build-dir", str(bd),
                "--version-source", "literal:v1", "--repo-root", str(fake),
                "--output", "-")
    assert r.returncode == 4
