# C1i: live read of the skill-panel icon inputs, entry failed, parked (Q2, the one stock start)

Queue `c1i-icon-live-read`. Worker packet `build/tmp/c1i-icon-live-read/round-note.md`, `prediction.md`, `run.log`,
`cleanup-report.json`, `patch-applied.md`. Dispatched 2026-10-24 01:31:26 MDT, blocked at 01:36:44, landed as parked 01:39 MDT.

## Result (MEASURED, negative)

One stock start: `prediction.md` mtime 01:33:35, `Popen` 01:34:58 (pid 36708), no retry. The c3m `fix.patch` (retry
x3, per-press logging in `find_menu_index`) was applied unchanged to mouv's `tools/harness/goto_level.py` after
`git apply --check`. The entry still failed: `enter MM_CLIMB_002` at 01:35:03, `RuntimeError: no menu highlight` at
01:35:47 (probe.py:224), after about 44 s of scanning. `state.json` records `entry_focus` "already foreground" for the
game pid, so acquisition reported the game as foreground. **None of the five items was read: `[levelScreen+0x10]`, the
per-icon fields and frame records, the `0x4E335C` entries, `[0x4CBF7C]` and the count text, and the hook triples are all
UNREAD.** The Frida hook never attached.

The patch's per-press lines (key polls, foreground pid, mod-5 counts) went to stdout, which the scheduled task does
not capture and the probe did not tee into `run.log`. They are lost, so why DOWN did not register is **not named**: this
is the second consecutive failure of the same entry (c3j, c1i) and the second time the cause stays unnamed. The c3m
patch did not fix it.

## Cleanup (worker, then my separate check)

Process closed; exe, `SavedGame.dat`, `dgVoodoo.conf` hashes before equal after (`32675bdb...`, `f7f86039...`,
`d639e220...`); no memory write happened (the level-table slot write was never reached); the worker left task
`LemWeb-c1i-1` registered and deleted it. **Left in place:** `tools/harness/goto_level.py` on mouv carries the c3m patch,
uncommitted (sha256 `c28335b5...31c8`); mouv's tree is dirty for that one file.

## Facilitator check

From my own ssh call after the report: no Lemmings process; no `LemWeb` task; `git status -uno` on mouv shows only
`tools/harness/goto_level.py`; `certutil` SHA-256 of the three install files equals the pre-launch hashes in
`cleanup-report.json`. `prediction.md` mtime 01:33:35 precedes `Popen` 01:34:58 in `run.log`. No icon value claim is
made, so there is nothing to narrow in C1g/C1h. I removed one `._round-note.md` left in the round directory.

## Resume (parked)

Do not spend another start on the same entry. A next live attempt first needs the entry understood offline: route
`find_menu_index`'s per-press output into `run.log` (it is stdout-only) and decide from a no-input 15 s snapshot whether
the menu is interactive. Then the read list is unchanged: `[levelScreen+0x10]`, icon fields at
`0x4C91D8 + 0x1150 + i*0x208`, `0x4E335C..`, `[0x4CBF7C]`/`[0x4CBF78]` and the count text objects, one hook on `0x47A75D`.
`probe.py` and `hook.js` in the round directory are ready for it.
