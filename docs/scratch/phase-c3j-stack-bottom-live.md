# C3j - stack bottom `0x58D5A0` live read: the one start failed before any read (Q2 round 2 of 2)

Worker output: `build/tmp/c3j-stack-bottom-live/round-note.md`. Brief: `build/tmp/c3j-stack-bottom-live-brief.md`.

## Result

**Nothing was read.** The directive's one stock start (pid 40752, session 1, task `LemWeb-c3j-1`, launched 23:32:38 MDT) raised `RuntimeError: no menu highlight` at 23:32:58 from A7p `capture.py:678` inside `enter_level()` (unedited A7p code; `find_menu_index` in `tools/harness/goto_level.py:200` returned `None` while screen id 3 / ui_state 3). The reader opens after `enter_level`, so `polls` is `{}` and `judge` is `[]`: zero reads of `0x58D5A0`, `0x58D3D8` or `0x58CE24`. The A6 repair 4 pause was never reached.

P1-P8 (c3h table, copied verbatim into the run's `prediction.md` before the start) are **untested**, neither confirmed nor falsified. Element 0 = per-frame push of `[0x58D3D8]` (push `0x45DD7B`, pop `0x45E347`) stays a READ offline claim, not a live measurement.

## Cause: UNREAD

Only the traceback exists. The same A7p entry reached the level in a7p and c2h; this run differs only in the script copy (confirm token and task name). A flake or an environment difference is the worker's guess, not shown. `find_menu_index` presses DOWN three times and keeps heap words that advance by 1 mod 5 each time; it returns `None` when no word does. No diagnostic was possible: the directive grants no retry after Popen.

## Restoration (MEASURED)

`stack-bottom.json` cleanup `hashes_restored: true`, `remaining_processes: []`.

## Facilitator check

Separate SSH calls after the run: no `Lemmings Revolution.exe` in `tasklist`, `schtasks /query | findstr /i LemWeb` empty (rc 1). Recorded hashes in `read_stack_bottom.py` (`AUTHORITY`, `SAVE_SHA`, `CONF_SHA`) equal the values the run's `cleanup` wrote for `Lemmings Revolution.exe` (`32675bdb...37a2f`), `SavedGame.dat` (`f7f86039...e6a`) and `dgVoodoo.conf` (`d639e220...72b`). `run.log` and `stack-bottom.json` re-read: the traceback and the empty `polls`/`judge` agree with the worker's note. Not independently re-hashed by me on mouv; I relied on the run's own cleanup hashes and the worker's SSH check. The directive start budget (one) was used exactly once; no debugger, no write to game memory.

## Resume

Q2 needs a **new** start grant. Recommendation: first an offline round on the menu entry (why `find_menu_index` found no candidate 15 s after launch: menu not yet interactive, a different screen, or focus), then one start with the unchanged reader. Do not spend a new grant on the unchanged entry.
