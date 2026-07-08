// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_version.cpp — return the compile-time provenance macros injected by
// main/version.cmake. The #ifndef fallbacks are defensive: if the macros are
// ever absent (e.g. an isolated host compile with no version.cmake), the
// accessors still return a well-defined "unknown"-style string.

#include "ss_version.h"

#ifndef SS_FW_GIT_SHA
#define SS_FW_GIT_SHA "unknown"
#endif
#ifndef SS_FW_TAG
#define SS_FW_TAG "0.0.0-unknown"
#endif
#ifndef SS_FW_BUILD_ID
#define SS_FW_BUILD_ID "unknown"
#endif

const char* ss_fw_git_sha(void)
{
    return SS_FW_GIT_SHA;
}
const char* ss_fw_tag(void)
{
    return SS_FW_TAG;
}
const char* ss_fw_build_id(void)
{
    return SS_FW_BUILD_ID;
}
