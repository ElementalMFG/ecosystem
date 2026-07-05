#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
# PostToolUse hook: auto-format C/C++ files edited by agents (OWNER_DECISIONS F2a).
# Skips hand-formatted T1 contract trees (pin maps / HAL headers use deliberate
# column alignment that .clang-format cannot reproduce — see docs/CHANGELOG.md):
#   firmware/boards/**, firmware/components/ss_hal/**
set -euo pipefail
input=$(cat)
file=$(printf '%s' "$input" | jq -r '.tool_input.file_path? // empty')
case "$file" in
  */firmware/boards/*|*/firmware/components/ss_hal/*)
    ;;
  *.c|*.h|*.cpp)
    if command -v clang-format >/dev/null 2>&1 && [ -f "$file" ]; then
      clang-format -i "$file"
    fi
    ;;
esac
exit 0
