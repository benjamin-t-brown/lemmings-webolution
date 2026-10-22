#!/bin/sh
# Resume an existing facilitator session for one turn with a prompt file, logging to a file.
# Usage: facilitate-mac.sh <facilitator-session-id> <prompt-file> <log-file>
root=$(cd "$(dirname "$0")/../.." && pwd)
cd "$root" || exit 1
git pull --ff-only || exit 1
opencode2 run --session "$1" --agent facilitator-mac \
  --model 'llmgw/gpt-6.1-sol#high' "$(cat "$2")" 2>&1 | tee "$3"
echo "=== opencode exited $? at $(date)" | tee -a "$3"
