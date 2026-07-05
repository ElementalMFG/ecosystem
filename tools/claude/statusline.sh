#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2025 SS-SP Project Contributors
#
# Claude Code statusline: makes (model @ effort) drift visible at all times,
# per docs/portfolio/10_MODEL_ALLOCATION_STRATEGY.md §2.2. Reads the statusline
# JSON on stdin; prints one line.
set -euo pipefail

input=$(cat)

model=$(printf '%s' "$input" | jq -r '.model.display_name // .model.id // "?"')
effort=$(printf '%s' "$input" | jq -r '.effort.level // "default"')
pct=$(printf '%s' "$input" | jq -r '.context_window.used_percentage // 0' | cut -d. -f1)

printf '[%s @ %s] ctx %s%%\n' "$model" "$effort" "$pct"
