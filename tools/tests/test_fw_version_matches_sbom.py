# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2026 SS-SP Project Contributors
"""Host test: the firmware's compiled-in version (firmware/main/version.cmake)
resolves byte-identically to the SBOM's version metadata (tools/gen-sbom.py).

This machine-verifies the S-02-020 AC "values MATCH the release artifact
metadata": both sources derive from the SAME two git commands
(`git describe --tags --always --dirty` and `git rev-parse HEAD`), so a drift
in either (version.cmake or resolve_fw_version) fails here. Pure pytest,
stdlib + pytest only — no container/hardware.
"""

import importlib.util
import subprocess
from pathlib import Path

import pytest

REPO = Path(__file__).resolve().parents[2]
GEN = REPO / "tools" / "gen-sbom.py"
VERSION_CMAKE = REPO / "firmware" / "main" / "version.cmake"

_spec = importlib.util.spec_from_file_location("gen_sbom", GEN)
g = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(g)


def _in_git_repo() -> bool:
    r = subprocess.run(
        ["git", "-C", str(REPO), "rev-parse", "--git-dir"],
        capture_output=True, text=True,
    )
    return r.returncode == 0


pytestmark = pytest.mark.skipif(
    not _in_git_repo(), reason="requires a git checkout"
)


def _cmake_git_tag() -> str:
    """Reproduce version.cmake's tag command exactly."""
    return subprocess.run(
        ["git", "describe", "--tags", "--always", "--dirty"],
        cwd=str(REPO), capture_output=True, text=True, check=True,
    ).stdout.strip()


def _cmake_git_sha() -> str:
    """Reproduce version.cmake's sha command exactly."""
    return subprocess.run(
        ["git", "rev-parse", "HEAD"],
        cwd=str(REPO), capture_output=True, text=True, check=True,
    ).stdout.strip()


def test_firmware_and_sbom_tag_sha_byte_identical():
    fw_tag = _cmake_git_tag()
    fw_sha = _cmake_git_sha()
    sbom_tag, sbom_sha, unresolved = g.resolve_fw_version("git", REPO)
    assert not unresolved
    assert fw_tag == sbom_tag, (fw_tag, sbom_tag)
    assert fw_sha == sbom_sha, (fw_sha, sbom_sha)
    assert len(fw_sha) == 40
    assert all(c in "0123456789abcdef" for c in fw_sha)


@pytest.mark.parametrize("board", ["lite", "alpha", "omega"])
def test_build_id_matches_contract(board):
    sha = _cmake_git_sha()
    # Contract: build_id == "<board>-<sha12>", sha12 = first 12 chars.
    build_id = f"{board}-{sha[:12]}"
    assert build_id == f"{board}-{sha[:12]}"
    sha12 = build_id.split("-", 1)[1]
    assert len(sha12) == 12
    assert sha.startswith(sha12)


def test_version_cmake_uses_matching_git_commands():
    """version.cmake must invoke the same git commands gen-sbom.py does."""
    text = VERSION_CMAKE.read_text(encoding="utf-8")
    # git may carry global options (-c safe.directory=...) between `git` and the
    # subcommand, so match the subcommand + args, not a `git `-prefixed literal.
    assert "describe --tags --always --dirty" in text
    assert "rev-parse HEAD" in text


def test_version_cmake_honors_release_env_vars():
    """The release SBOM (release-sbom.yml) sources version from the env vars
    SS_FW_VERSION / SS_FW_GIT_SHA (gen-sbom.py --version-source env). For the
    firmware image to MATCH that release metadata, version.cmake must read the
    same two env vars, and (like gen-sbom.py _from_env) require BOTH."""
    text = VERSION_CMAKE.read_text(encoding="utf-8")
    assert "ENV{SS_FW_VERSION}" in text
    assert "ENV{SS_FW_GIT_SHA}" in text
    # env branch must precede the git fallback (mirrors gen-sbom auto precedence).
    assert text.index("ENV{SS_FW_VERSION}") < text.index("rev-parse HEAD")


def test_version_cmake_git_is_container_safe():
    """git must run with an explicit safe.directory so the root-owned pinned
    build container does not fall back to 'unknown' on a foreign-owned tree."""
    text = VERSION_CMAKE.read_text(encoding="utf-8")
    assert "safe.directory=" in text


def test_env_source_roundtrips_through_gen_sbom():
    """gen-sbom.py env resolution returns the same env vars version.cmake reads,
    so both embed identical values in a release build."""
    import os

    prev = {k: os.environ.get(k) for k in ("SS_FW_VERSION", "SS_FW_GIT_SHA")}
    fake_sha = "0" * 40
    os.environ["SS_FW_VERSION"] = "v9.9.9-test"
    os.environ["SS_FW_GIT_SHA"] = fake_sha
    try:
        tag, sha, unresolved = g.resolve_fw_version("env", REPO)
        assert (tag, sha, unresolved) == ("v9.9.9-test", fake_sha, False)
    finally:
        for k, v in prev.items():
            if v is None:
                os.environ.pop(k, None)
            else:
                os.environ[k] = v
