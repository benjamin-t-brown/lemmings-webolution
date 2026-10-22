# D2e: 20-position rescore attempt — clean negative (stock exe crashed before the scan)

Queue id `d2e-rescore`, round 1 of 3. [Brief](../../build/tmp/d2e-rescore-brief.md).

## What this round built

`tools/harness/measure_d2_rescore.py` (new, owned file). Imports `measure_d2_pick.py`
and `measure_d2_tie.py` unmodified for the launch shape, lemming/cursor/widget/panel
readers, the 20-position accept-class quota, the live tie-key reader
(`lemming_tie_fields`), the file-table cross-check, and the close-pair aim search. Feeds
every eligible candidate's live priority (`0x004C05D0 + 4*[state+0x14]`, `state =
[lemming+0x340]`) and live squared distance (`lemming+8` minus `screen+0x58/+0x60/+0x68`)
into the landed `pickHoveredLemming` (`src/app/Pick.cpp`) through a small stdin/stdout
driver (`build/tmp/d2e-rescore/pick_driver.cpp`), built from an unedited HEAD snapshot of
`Pick.cpp`/`Pick.h`. Neither tie key is ever forced to 0; a sample with an unreadable key
on an eligible candidate is discarded, not padded.

## Result: the one authorized live attempt crashed before reaching the level

Pid 23616 launched, reached the main menu (`ui_state` 3, `screen` 3), then died with an
access violation (exit code `3221225477` = `0xC0000005`) while the harness was
acquiring foreground to send the first of three menu `Down` presses:

- Before Down 1/2: `acquire_game_foreground.acquire(pid)` raised `TimeoutError('Game did
  not poll neutral input after foreground acquisition')`.
- Before Down 3: `acquire(pid)` raised `OSError('Cannot verify executable identity')` —
  consistent with the process already being gone or corrupted by that point.
- No `Down`, no `Return` were ever sent (`return_sent: false`, `menu-index candidates: 0`).
  `picks.jsonl` was never written.
- `subprocess.terminate()` on Windows produces exit code 1; this process's own exit code
  (`0xC0000005`) means it crashed on its own, not that the harness killed it.

This is a clean negative, not a workaround: per the brief's "no second Popen," there was
no retry. The 20-position gate is untested; D2 part (a) remains exactly where
[D2d](phase-d2-tie-live.md) left it (one measured overlap, both priority 0, real distance
picks the lemming the original stored).

## Restoration

`pid_gone: true`; a post-run `find_pids` for `Lemmings`/`dgVoodoo` found nothing. Install
hashes before/after match:

| File | SHA-256 |
| --- | --- |
| `Lemmings Revolution.exe` | `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f` |
| `dgVoodoo.conf` | `d639e220814d80b7956b96bad0ca708b768458bd348bfd607afbf1913598e72b` |
| `SavedGame.dat` | `f7f8603928890fb13d022d828c8a602cf7a7619ab24de82c7de4ae86ec337e6a` |

## Facilitator check (2026-10-22)

Independently re-ran, outside the worker's own invocation: `python
tools/harness/measure_d2_rescore.py --self-test` (driver self-test + protocol self-test,
including the priority-beats-closer-distance case) and `--check-file` — both reproduce
the worker's output exactly, including the `0x004C05D0` table head
`[2,1,1,4,2,3,3,3,3,1,0,2,2,2,2,2,2,0,0,2]` and file dword index `0x12 = 0` (the same
values D2c/D2d already independently verified). Independently re-hashed the installed
`Lemmings Revolution.exe`, `dgVoodoo.conf` and `SavedGame.dat` directly from disk with
PowerShell `Get-FileHash` (not trusting the worker's own log): all three match the
before/after values above exactly. Confirmed `build/tmp/d2e-rescore/snap/app/Pick.{h,cpp}`
is byte-identical to `src/app/Pick.{h,cpp}` (`diff`, no output), and `git diff --stat` on
`src/app/Pick.cpp`, `src/app/Pick.h`, `tools/harness/measure_d2_pick.py`,
`tools/harness/measure_d2_tie.py` shows zero changes — the round touched no production or
existing-harness file. Reviewed `pick_driver.cpp`'s own self-test: five distinct
assertions (box-overlap inclusive/exclusive left edge, ineligible-candidate exclusion,
earliest-index tie-break, empty-candidate none), not a single trivial check. Read
`measure_d2_rescore.py` directly: the driver call path never substitutes 0 for a missing
live key on an eligible candidate (samples with a missing key are discarded at
`snapshot()`'s `missing` check before reaching the driver). No Lemmings/dgVoodoo process
or `LemWeb-*` scheduled task after this check. Fresh `cmake --build --preset dev-debug` +
`ctest --preset dev-debug`: **28/28**. `stock_starts_consumed` corrected 0 to 1 (pid 23616
did launch). D2 part (a) does not pass; no new data was added toward it.
