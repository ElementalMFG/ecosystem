#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
# run-queue.sh — headless story-queue runner.
#
# Runs a list of stories in order through `/story-run`, one headless `claude`
# session each, with per-story post-checks (clean tree, Story: trailer, push)
# and a CI wait on main. T1 (or suspected T1) stories are never run headless:
# they trigger a loud STOP banner directing you to the interactive path.
#
# Usage: run-queue.sh [--dry-run] [--dangerous] S-AA-XXX [S-BB-YYY ...]
#   --dry-run    print the resolved launch line / T1 stop per story; run nothing
#   --dangerous  use --dangerously-skip-permissions instead of acceptEdits
set -euo pipefail

DRY_RUN=0
DANGEROUS=0
while [[ "${1:-}" == --* ]]; do
  case "$1" in
    --dry-run)   DRY_RUN=1; shift ;;
    --dangerous) DANGEROUS=1; shift ;;
    *) echo "run-queue.sh: unknown option '$1'" >&2; exit 64 ;;
  esac
done

if [[ $# -eq 0 ]]; then
  echo "usage: run-queue.sh [--dry-run] [--dangerous] S-AA-XXX [S-BB-YYY ...]" >&2
  exit 64
fi

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
STORIES=("$@")

t1_stop() { # $1=story $2=tier
  echo "############################################################" >&2
  echo "STOP: $1 is $2 — never run T1 headless." >&2
  echo "Run it interactively instead:" >&2
  echo "    tools/claude/work.sh $1" >&2
  echo "############################################################" >&2
}

abort() { # $1=story $2=message
  echo "############################################################" >&2
  echo "ABORT [$1]: $2" >&2
  echo "############################################################" >&2
}

declare -a SUM_STORY SUM_COMMIT SUM_CI

for STORY in "${STORIES[@]}"; do
  OUT="$(python3 "$ROOT/tools/allocation.py" --story "$STORY")"
  TIER="$(printf '%s\n' "$OUT" | sed -n 's/^TIER: //p')"
  MODEL="$(printf '%s\n' "$OUT" | sed -n 's/^LAUNCH: model=\([^ ]*\).*/\1/p')"
  EFFORT="$(printf '%s\n' "$OUT" | sed -n 's/^LAUNCH: .*effort=\([^ ]*\).*/\1/p')"

  if [[ -z "$TIER" || -z "$MODEL" || -z "$EFFORT" ]]; then
    abort "$STORY" "could not resolve tier/model/effort"
    printf '%s\n' "$OUT" >&2
    exit 1
  fi

  if [[ "$TIER" == "T1" || "$TIER" == "T1?" ]]; then
    t1_stop "$STORY" "$TIER"
    exit 3
  fi

  # ORDER-GUARD: refuse to launch a story whose dependencies are unsatisfied.
  # STATUS-GUARD (exit 5): refuse a story that is not DRAFT/READY — already
  # executed, in flight, or blocked. Re-execution is structurally impossible.
  ELIG="$(python3 "$ROOT/tools/allocation.py" --eligible "$STORY" 2>&1)" && ERC=0 || ERC=$?
  if [[ $ERC -eq 4 || $ERC -eq 5 ]]; then
    echo "############################################################" >&2
    if [[ $ERC -eq 5 ]]; then
      echo "STATUS-GUARD ABORT [$STORY]: story is not runnable" >&2
    else
      echo "ORDER-GUARD ABORT [$STORY]: dependencies not satisfied" >&2
    fi
    printf '%s\n' "$ELIG" >&2
    echo "Run 'python3 tools/allocation.py --next' to see the eligible frontier." >&2
    echo "############################################################" >&2
    exit $ERC
  fi

  if [[ $DANGEROUS -eq 1 ]]; then
    PERM=(--dangerously-skip-permissions)
  else
    PERM=(--permission-mode acceptEdits)
  fi
  LAUNCH="claude -p \"/story-run $STORY\" --model $MODEL --effort $EFFORT ${PERM[*]}"

  if [[ $DRY_RUN -eq 1 ]]; then
    echo "LAUNCH [$STORY $TIER]: $LAUNCH"
    continue
  fi

  before="$(git -C "$ROOT" rev-parse HEAD)"

  LOGDIR="$ROOT/logs/queue"
  mkdir -p "$LOGDIR"
  LOG="$LOGDIR/$(date -u +%Y-%m-%d)-$STORY.log"
  echo ">>> [$STORY $TIER] $LAUNCH"
  if ! claude -p "/story-run $STORY" --model "$MODEL" --effort "$EFFORT" "${PERM[@]}" 2>&1 | tee "$LOG"; then
    abort "$STORY" "headless claude exited nonzero (see $LOG)"
    exit 1
  fi

  # Post-checks.
  if [[ -n "$(git -C "$ROOT" status --porcelain)" ]]; then
    abort "$STORY" "working tree not clean after story-run"
    exit 1
  fi
  after="$(git -C "$ROOT" rev-parse HEAD)"
  if [[ "$after" == "$before" ]]; then
    abort "$STORY" "no new commit (HEAD unchanged)"
    exit 1
  fi
  if ! git -C "$ROOT" log "$before..$after" --format=%B | grep -q "^Story: $STORY$"; then
    abort "$STORY" "no commit carries trailer 'Story: $STORY'"
    exit 1
  fi

  git -C "$ROOT" fetch -q
  if [[ "$(git -C "$ROOT" rev-parse HEAD)" != "$(git -C "$ROOT" rev-parse origin/main)" ]]; then
    if ! git -C "$ROOT" push -q origin HEAD:main; then
      abort "$STORY" "HEAD not on origin/main and push failed"
      exit 1
    fi
    git -C "$ROOT" fetch -q
    if [[ "$(git -C "$ROOT" rev-parse HEAD)" != "$(git -C "$ROOT" rev-parse origin/main)" ]]; then
      abort "$STORY" "HEAD still != origin/main after push"
      exit 1
    fi
  fi
  sha="$(git -C "$ROOT" rev-parse HEAD)"

  # CI wait: up to 12 min (24 * 30s).
  ci="TIMEOUT"
  for _ in $(seq 1 24); do
    runs="$(gh run list --branch main --limit 8 \
              --json headSha,status,conclusion 2>/dev/null || echo '[]')"
    status="$(printf '%s' "$runs" | python3 -c '
import json,sys
sha=sys.argv[1]
try: runs=json.load(sys.stdin)
except Exception: runs=[]
mine=[r for r in runs if r.get("headSha")==sha]
if not mine: print("PENDING"); sys.exit()
if any(r.get("status")!="completed" for r in mine): print("PENDING"); sys.exit()
if all(r.get("conclusion")=="success" for r in mine): print("SUCCESS")
else: print("FAIL:"+",".join(r.get("name",r.get("workflowName","?")) for r in mine if r.get("conclusion")!="success"))
' "$sha")"
    case "$status" in
      SUCCESS) ci="success"; break ;;
      FAIL:*)  abort "$STORY" "CI failed: ${status#FAIL:}"; exit 2 ;;
      *)       sleep 30 ;;
    esac
  done
  if [[ "$ci" != "success" ]]; then
    abort "$STORY" "CI did not complete within 12 min for $sha"
    exit 2
  fi

  SUM_STORY+=("$STORY"); SUM_COMMIT+=("${sha:0:12}"); SUM_CI+=("$ci")
done

if [[ $DRY_RUN -eq 0 && ${#SUM_STORY[@]} -gt 0 ]]; then
  echo
  printf '%-12s | %-12s | %s\n' "story" "commit" "CI"
  printf '%-12s-+-%-12s-+-%s\n' "------------" "------------" "-------"
  for i in "${!SUM_STORY[@]}"; do
    printf '%-12s | %-12s | %s\n' "${SUM_STORY[$i]}" "${SUM_COMMIT[$i]}" "${SUM_CI[$i]}"
  done
fi
exit 0
