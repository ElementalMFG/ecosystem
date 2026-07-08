// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_version.h — compiled-in firmware provenance (git SHA + tag + build id).
//
// Values are injected at configure time by main/version.cmake using the same
// resolution precedence as tools/gen-sbom.py (env SS_FW_VERSION/SS_FW_GIT_SHA,
// else git), so these accessors (and the boot banner that prints them) are
// byte-identical to the release artifact's SBOM metadata (S-02-020). This
// header IS the firmware's version query surface.

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Full 40-hex commit SHA, or "unknown" when built outside a git tree.
const char* ss_fw_git_sha(void);

// `git describe --tags --always --dirty` output
// (e.g. "v1.2.3", "v1.2.3-4-gabc1234", "...-dirty", or "0.0.0-unknown").
const char* ss_fw_tag(void);

// Build identifier "<board>-<sha12>" (first 12 SHA chars), or "<board>-unknown".
const char* ss_fw_build_id(void);

#ifdef __cplusplus
}
#endif
