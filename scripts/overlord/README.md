# Overlord launchers

Used by the [overlord](../../docs/OVERLORD.md) to start, wake and show facilitator sessions.
They only launch sessions; queue state stays with the facilitator.

| Script | Machine | Does |
| --- | --- | --- |
| `launch-facilitator-mac.sh ["scope"]` | Mac | Pulls, then opens a Terminal running a new `/facilitator-mac` TUI. |
| `facilitate-mac.sh <session> <prompt> <log>` | Mac | Pulls, then resumes the facilitator session for one turn with the prompt file, logging with `tee`. |
| `watch-worker-mac.sh <worker> <session> <prompt> <log>` | Mac | Polls `opencode2 api get /api/session/active` each minute for up to 3 h. When the worker is idle, it opens a Terminal running `facilitate-mac.sh`. |
| `Start-Visible.ps1 -Script <ps1> -Title <name>` | mouv | Over SSH, opens a visible Windows Terminal in desktop session 1 through a one-shot `LemWeb-overlord-*` scheduled task. |

Run `watch-worker-mac.sh` in the background. To stop a worker that is over budget, after
the user agrees, run `opencode2 api post /api/session/<worker>/interrupt`. Put prompt files
and logs under `build/tmp/overlord-<date>/`.
