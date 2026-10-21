# A7o the walk never started

2026-10-21. One stock start of `MM_CLIMB_002`, pid 58432. Dispatch
`f9285bba-bafa-4589-bdb3-340043496d41`, Cursor `round-agent`, requested
`grok-4.7-high-fast`. The prediction file was written at 02:38:47 and the
launch log begins at 02:41:44. The prediction was a pass for the reference
camera hash `7e6e1a8a77c29f860f82dd8cde3f60c9f89ecf3f1352d3c2a6801cc9867d0fff`
and for every view. That prediction was not tested.

## What the run measured

The process reached `Compiled\MM_CLIMB_002.lvl` with `ui_state` 1. The level
slot at `0x4E35D4` (280 bytes) was restored; the write log's saved and
readback SHA-256 are both
`ff54ee2abe05d38c4231add0cc8af980323f7ea4d7edc605cd49d81afff40508`. Before any
unpause, the 360-byte block at `0x4CBFC8` was saved. Its SHA-256 is
`619ef313758bffa1630fe465d30df802d4447bab3c4caab27ea848d35a84702b`. That is
the entry-pause block. It is not the post-walk start check, and it is neither
the reference `7e6e1a8a…` nor A7n's `2b33a708…`.

`walk_14` then raised `unpause tap wait failed`. `run.log` stamps `02:42:03`
on the entry-pause line and on the traceback. `roi-table.json` and
`score-table.json` are `[]`. No screenshot was taken. No route prologue was
changed to `C3`. There is no caption-excluded wider-mask score.

## Why the walk did not run

`tools/harness/lrinput.py` `tap` keys the scan down, waits, keys it up, and
waits again. The function has no `return`, so the value is `None`.
`capture.py` `walk_14` does `if not inp.tap("P", hold_frames=2): raise`.
`not None` is true, so that raise follows every return from `tap`. The same
script's entry path calls `inp.tap(DIK["P"], hold_frames=4)` and does not
read the return; the log then records `paused ui 1`. Whether the walk's P
key changed `ui_state` was not read. The opening-camera hypothesis is
untested.

The log also records `timer_dword 300` at `0x4CBAAC`. That dword was not
saved as its own bytes, and the process is gone, so it was not re-read.
`tools/harness/measure_release.py` already labels `0x4CBAAC` as whole seconds
remaining, and `fresh_level.py` already says `MM_CLIMB_002` has a 300 second
limit. The worker's `300 / 60 = 5` arithmetic is not adopted.

## Restoration

Exe, `SavedGame.dat`, and `dgVoodoo.conf` match the pre-launch hashes and the
backups. `savedgame_restored` is false because the save already matched its
backup. File bytes at `0x4622FD`, `0x4623A5`, `0x462404`, `0x462465`, and
`0x43A4EC` are `0x55`. The write log contains the config swap and the level
slot only. Launcher stderr is empty and the launcher exit code is 1. Pid
58432 is gone. No stock or `webolution` process was left.

## Facilitator check

`build/tmp/a7o-facilitator/verify.py` rehashed the saved camera block
(360 bytes, `619ef313758bffa1630fe465d30df802d4447bab3c4caab27ea848d35a84702b`),
confirmed both score tables are `[]`, and confirmed `tap`'s body contains no
`return` while `walk_14` still branches on `not inp.tap(...)`. `capture.py`
refuses `Popen` when `prediction.md` is missing. Prediction mtime 02:38:47
is before `capture.py` 02:40:56 and before `run.log` 02:42:03.
`read_steppers.py` reads `55` (`push ebp`) at all five prologue addresses.
Install SHA-256 matches the A7n set: exe
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`,
`SavedGame.dat`
`f7f8603928890fb13d022d828c8a602cf7a7619ab24de82c7de4ae86ec337e6a`,
`dgVoodoo.conf`
`d639e220814d80b7956b96bad0ca708b768458bd348bfd607afbf1913598e72b`.
The save and conf backups match those install files. No production edit.
Ninja had no work. CTest 24/24.

## Left open

The settled-camera question is not parked. A7m, A7n, and A7o are three
acquisition rounds, and none of them scored a baseline taken from camera
`7e6e1a8a…`. A7o stopped on a return-value check, not on the hash falsifier.
A fourth round is expected to move the gate because the A6f walk calls
`tap` and then reads `ui_state`, and that sequence is what produced the
reference hash. The durable caption-excluded initial score is unchanged.
