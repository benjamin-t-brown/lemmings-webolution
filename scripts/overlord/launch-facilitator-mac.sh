#!/bin/sh
# Open a visible Terminal running a new Mac facilitator TUI in this clone.
# Usage: launch-facilitator-mac.sh ["optional scope text"]
root=$(cd "$(dirname "$0")/../.." && pwd)
cmd="cd '$root' && git pull --ff-only && opencode2 --prompt '/facilitator-mac $1'"
osascript -e "tell application \"Terminal\" to do script \"$cmd\"" -e 'tell application "Terminal" to activate'
