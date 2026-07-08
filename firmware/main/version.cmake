# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2025 SS-SP Project Contributors
#
# version.cmake — capture firmware provenance (SHA + tag + build id) at
# configure time and inject it into the `main` component as quoted string
# macros.
#
# Resolution precedence MIRRORS tools/gen-sbom.py resolve_fw_version() in
# "auto" mode (S-02-019) so the compiled-in values MATCH the release artifact
# SBOM metadata (S-02-020 AC):
#   1. env  — SS_FW_VERSION (tag) + SS_FW_GIT_SHA (sha), if BOTH are set.
#             This is the source release-sbom.yml uses (`--version-source env`),
#             so a tagged release build embeds exactly what the release SBOM
#             records.
#   2. git  — `git describe --tags --always --dirty` (tag) + `git rev-parse
#             HEAD` (sha), byte-identical to gen-sbom.py's git path. Invoked
#             with `-c safe.directory=<repo_root>` so it also works inside the
#             root-owned pinned build container (foreign-owned /workspace would
#             otherwise trip git's "dubious ownership" guard → "unknown").
#   3. else — "0.0.0-unknown" / "unknown" placeholders.
# A host test (tools/tests/test_fw_version_matches_sbom.py) asserts the match.
#
# Included from main/CMakeLists.txt AFTER idf_component_register(). Values are
# captured into plain (non-cache) variables so every configure re-runs the
# cheap resolution — a rebuild after a new commit never serves a stale SHA.

# Repo root = two levels up from firmware/main/.
get_filename_component(_ss_repo_root "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)

# 1. env precedence — both must be present (matches gen-sbom.py _from_env()).
if(DEFINED ENV{SS_FW_VERSION} AND DEFINED ENV{SS_FW_GIT_SHA}
   AND NOT "$ENV{SS_FW_VERSION}" STREQUAL "" AND NOT "$ENV{SS_FW_GIT_SHA}" STREQUAL "")
    set(SS_FW_TAG "$ENV{SS_FW_VERSION}")
    set(SS_FW_GIT_SHA "$ENV{SS_FW_GIT_SHA}")
else()
    # 2. git — safe.directory guard lets this succeed in the build container.
    execute_process(
        COMMAND git -c "safe.directory=${_ss_repo_root}" describe --tags --always --dirty
        WORKING_DIRECTORY "${_ss_repo_root}"
        OUTPUT_VARIABLE SS_FW_TAG
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE _ss_tag_rc
        ERROR_QUIET
    )
    execute_process(
        COMMAND git -c "safe.directory=${_ss_repo_root}" rev-parse HEAD
        WORKING_DIRECTORY "${_ss_repo_root}"
        OUTPUT_VARIABLE SS_FW_GIT_SHA
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE _ss_sha_rc
        ERROR_QUIET
    )
    # 3. placeholders on any failure.
    if(NOT _ss_tag_rc EQUAL 0 OR SS_FW_TAG STREQUAL "")
        set(SS_FW_TAG "0.0.0-unknown")
    endif()
    if(NOT _ss_sha_rc EQUAL 0 OR SS_FW_GIT_SHA STREQUAL "")
        set(SS_FW_GIT_SHA "unknown")
    endif()
endif()

# build id = <board>-<sha12>  (sha12 = first 12 chars, or "unknown").
if(SS_FW_GIT_SHA STREQUAL "unknown")
    set(_ss_sha12 "unknown")
else()
    string(SUBSTRING "${SS_FW_GIT_SHA}" 0 12 _ss_sha12)
endif()
set(SS_FW_BUILD_ID "${SS_BOARD}-${_ss_sha12}")

message(STATUS "SS-SP: fw provenance tag=${SS_FW_TAG} sha=${SS_FW_GIT_SHA} "
               "build_id=${SS_FW_BUILD_ID}")

# Quoted string macros on the main component ONLY (never global).
target_compile_definitions(${COMPONENT_LIB} PRIVATE
    "SS_FW_GIT_SHA=\"${SS_FW_GIT_SHA}\""
    "SS_FW_TAG=\"${SS_FW_TAG}\""
    "SS_FW_BUILD_ID=\"${SS_FW_BUILD_ID}\""
)
