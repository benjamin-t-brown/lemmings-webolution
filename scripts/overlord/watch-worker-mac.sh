#!/bin/sh
# Wait until a worker session is no longer active (3 h deadline), then open a Terminal
# that resumes the facilitator with facilitate-mac.sh.
# Usage: watch-worker-mac.sh <worker-session-id> <facilitator-session-id> <prompt-file> <log-file>
here=$(cd "$(dirname "$0")" && pwd)
end=$(( $(date +%s) + 10800 ))
while [ "$(date +%s)" -lt "$end" ]; do
  if ! opencode2 api get /api/session/active 2>/dev/null | grep -q "\"$1\""; then
    echo "worker $1 idle at $(date)"
    osascript -e "tell application \"Terminal\" to do script \"'$here/facilitate-mac.sh' '$2' '$3' '$4'\"" \
      -e 'tell application "Terminal" to activate' >/dev/null
    exit 0
  fi
  sleep 60
done
echo "deadline reached; worker $1 still active at $(date)"; exit 1
