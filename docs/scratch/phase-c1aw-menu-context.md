# C1aw: the menu-attach start exited before ^tab

Queue `c1aw-menu-context`. The one live round. Worker [Read tab context byte](635c6c8f-4682-4ab9-ab0f-53701ae71e2a). `prediction.md` 00:46:55, before the dry check and before `Popen` 00:49:28. REA was not used. Dispatched 00:36 MDT.

## Result

Pid 35216, one normal `Popen`, session 1. The attach armed at the menu before the click. The process was already dead at cleanup, exit code 3221225477 (`0xC0000005`). It was not started again.

`hits.log` has one line, texture `Sheets\$LemSheetHi8`, dword `0`, context `+4` `0`, `+5` `1`, `+6` `1`, `+7` `0`, flags `2`. `^tab` did not reach `0x473146`.

The exe, `SavedGame.dat`, and `dgVoodoo.conf` hashes match the pre-launch copies. The level-table slot is process memory and was not written back because the process was already gone.

## Facilitator check

Recounted `hits.log`: one line, and the texture is not `^tab`. `run.log` has one `Popen:` line. `drive.log` has one attach, pid 35216, and `lemmings after drive: []`. `probe.py` has no `frida.spawn`. `poll_before_cleanup` and `exit_code` are 3221225477 in `cleanup-report.json` and `state.json`. `closed_by_cleanup` is false.

Re-hashed the exe, `SavedGame.dat`, and `dgVoodoo.conf`. All three equal the cleanup report and the copied originals, including `SavedGame.dat` byte for byte. No Lemmings process and no `LemWeb-*` task. `python build/tmp/c1aw-menu-context/dry_check.py` exited 0. Changing only `ptr('0x473146')` to `ptr('0x473147')` exited 1 with `missing attach ptr('0x473146')`. Restored; SHA-256 `6b8a3a6867048c251c224b706714139445535b42a2bbdc937ea8a62b675365b1`; the same command exited 0. No source change, so the suite was not rebuilt.

Narrowing: the one logged upload has flags `2`, so `+4` `0` and `+5` `1` match the c1au formula for that upload. It is not a `^tab` measurement, and `+7` `0` on this line does not fill the unwritten dword for `^tab`. The in-process level slot could not be restored. The install files match. The gate is not scored.

## Stop

The directive's one start crashed. No repair and no further start. The park-or-amend choice is left for the overlord.
