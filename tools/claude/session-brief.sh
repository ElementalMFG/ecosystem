#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
# session-brief.sh — SessionStart hook: inject a compact program brief into
# every fresh session so cold starts never miss in-flight state (doc 11 §6d).
# Output budget: ~25 lines. Read-only; must never fail the session (exit 0).
set -uo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
cd "$ROOT" 2>/dev/null || exit 0

echo "=== SS-SP session brief (auto-injected; generated $(date -u +%F)) ==="
echo "--- last 3 commits:"
git log --oneline -3 2>/dev/null || true

echo "--- stories in flight (IN_PROGRESS / IN_REVIEW / BLOCKED):"
grep -rh -B3 "Status=IN_PROGRESS\|Status=IN_REVIEW\|Status=BLOCKED" \
    docs/portfolio/epics/*/STORIES.md 2>/dev/null |
    grep "^### " | sed 's/^### /  /' | head -10 || true

echo "--- recent engineering-log entries (grep it for your component):"
grep "^- " docs/dev/ENGINEERING_LOG.md 2>/dev/null | tail -4 || true

echo "--- reminders: allocation via 'python3 tools/allocation.py --story ID';"
echo "    binding policies: CLAUDE.md + docs/portfolio/11_TOKEN_ECONOMY.md;"
echo "    IN_REVIEW exits (evidence-linked only): docs/dev/EVIDENCE_RUNSHEET.md"
exit 0
