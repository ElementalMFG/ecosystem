#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
# work.sh S-NN-MMM — launch a tier-correct Claude session for one story.
#
# Resolves the story's tier and (model @ effort) via tools/allocation.py, then
# execs `claude` with the matching model, effort, and entry prompt:
#   T1   -> claude-fable-5 @ xhigh, /t1-pipeline
#   T2   -> claude-fable-5 @ high,  /story-run
#   T3/T4 -> claude-opus-4-8 @ medium, /story-run
#   T1?  -> claude-opus-4-8 @ medium, /story-run (prints a confirm warning first)
set -euo pipefail

STORY="${1:-}"
if [[ -z "$STORY" ]]; then
  echo "usage: work.sh S-NN-MMM" >&2
  exit 64
fi

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
OUT="$(python3 "$ROOT/tools/allocation.py" --story "$STORY")"

TIER="$(printf '%s\n' "$OUT" | sed -n 's/^TIER: //p')"
MODEL="$(printf '%s\n' "$OUT" | sed -n 's/^LAUNCH: model=\([^ ]*\).*/\1/p')"
EFFORT="$(printf '%s\n' "$OUT" | sed -n 's/^LAUNCH: .*effort=\([^ ]*\).*/\1/p')"

if [[ -z "$TIER" || -z "$MODEL" || -z "$EFFORT" ]]; then
  echo "work.sh: could not resolve tier/model/effort for $STORY" >&2
  printf '%s\n' "$OUT" >&2
  exit 1
fi

case "$TIER" in
  "T1?")
    echo "############################################################" >&2
    echo "WARNING: $STORY is T1? (SUSPECTED T1)." >&2
    echo "Confirm the tier at elaboration. If a T1 domain is confirmed" >&2
    echo "(keys, wire bytes, eFuse, persistence, sandbox), STOP and run" >&2
    echo "/t1-pipeline $STORY on Fable @ xhigh instead of this session." >&2
    echo "############################################################" >&2
    PROMPT="/story-run $STORY"
    ;;
  T1)
    PROMPT="/t1-pipeline $STORY"
    ;;
  T2|T3|T4)
    PROMPT="/story-run $STORY"
    ;;
  *)
    echo "work.sh: unknown tier '$TIER' for $STORY" >&2
    exit 1
    ;;
esac

if claude --help 2>/dev/null | grep -q -- --effort; then
  exec claude --model "$MODEL" --effort "$EFFORT" "$PROMPT"
else
  echo "note: installed claude has no --effort flag; run /effort $EFFORT in-session." >&2
  PROMPT="$PROMPT (run /effort $EFFORT)"
  exec claude --model "$MODEL" "$PROMPT"
fi
