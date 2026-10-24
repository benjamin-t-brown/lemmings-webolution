# C1af: the paused attach saw no shift

Queue `c1af-shift-count-live`. One stock start. Worker [Read live shift counts](fb422a87-05cd-40db-8d86-7d396433c98e). `prediction.md` 17:14:31, `Popen` 17:15:44. REA was the Frida attach only. Dispatched 17:07 MDT.

## Result

One `Popen`, pid 32972, session 1. The click into `MM_CLIMB_002`, the pause, and `ready.json` are all logged at 17:16:16. The attach started when `ready.json` appeared and exited 0. `C1AE_HOOK` reported `hits` 0, `distinct` 0, `triples` []. The mask and both counts were not read. `hook-done.flag` was seen at 17:16:23, and the probe closed the process in that same run.

The six seconds after the hook was installed did not call `0x473897`. The upload during the click at 17:16:16 was not observed. Install hashes match. No game process afterwards.

## Facilitator check

Read `hook-live.json`: `hits` 0, `distinct` 0, `triples` []. `run.log` has one `Popen:` line. The click, pause, and `ready.json` share the timestamp 17:16:16; `hook-done.flag` is 17:16:23. Re-hashed the exe, `SavedGame.dat`, and `dgVoodoo.conf`; they match `cleanup-report.json`. No `*Lemming*` process. No source change. The suite on this tree was already 34/34.

Narrowing: zero hits means the `sar` did not run while the hook was installed. It does not mean the upload never runs. The hook was not shown to be in place before the click.

## Stop

Round 4. The start was spent. Another start needs an offline repair that arms the hook before the level click. The counts stay unread.
