# C1k: make the menu entry observable (directive part 1b, offline)

Queue `c1k-observable-entry`. Worker packet `build/tmp/c1k-observable-entry/round-note.md`, `prediction.md`, `dry_check.py`,
`dry-check.out`, `goto_level.c1k.diff`, `probe.py`. Dispatched 01:51:34 MDT, worker packet complete 01:55, landed 02:35 MDT. No game
started, no debugger, no `src/**` change. The cause of the c3j / c1i `no menu highlight` failures is **still not named**; this
round only makes the next failure say what happened.

## What changed (`tools/harness/goto_level.py`)

- `find_menu_index(h, inp, verbose=True, attempts=3, log=None)`: backward compatible. The c3m retry (3 attempts of 3 DOWN presses)
  stays. Each press now writes `menu scan A.P: key polls N, foreground before->after, mod-5 n, kept m` and the `menu-index
  candidates` line to `log` (callable or file-like) as well as stdout.
- `foreground_pid()` hoisted to module level; returns -1 when it cannot ask (no `user32`, call fails).
- `snapshot_idle(h, inp, seconds=15, interval=1, log, ...)`: no input, no write; one line per second of screen id, `ui_state`,
  shim `key_frame` and foreground pid. It separates "menu not interactive / game not polling" from "keys sent and lost".
- Probe copy `build/tmp/c1k-observable-entry/probe.py` (output dir `build/tmp/c1l-icon-live-read/`): read list identical to c1i's;
  adds the idle snapshot after `open_game()`, the foreground pid before the scan, `log=say` into `run.log`, and a
  `scan failed:` line (ui, screen, foreground, key_frame) before the raise. `prediction.md` must exist in the c1l directory before launch.

Decision on the dirty `goto_level.py` on mouv (c3m patch, sha256 `c28335b5...31c8`): **landed**, with this change (now
`4c4bc28b...8f3f`). Defaults for other callers are unchanged except the retry, and reverting would drop the only tee. mouv's tree is
clean after this commit except a pre-existing untracked `NUL`.

## Facilitator check

Re-ran `dry_check.py` and `py_compile` on mouv (`dry check OK: 15 per-press lines, 15 with foreground, 16 idle samples`; sha256
as above). **Mutated one thing alone:** made `emit` return before writing to `log`; the dry check failed (`AssertionError: (0, 0)`
press lines); file restored, hash equal to the pre-mutation hash. Read the diff of `goto_level.c1k.diff`; read the probe-vs-c1i diff:
only `OUT`, the idle snapshot, `foreground_pid`, `log=say` and the failure line differ, so the read list is unchanged.

**Narrowed / not proven:** the dry check uses stubs. It does not show that DOWN reaches the game, that the idle snapshot sees an
interactive menu, that `foreground_pid` works in session 1 (it returned 0 in mouv's session 0), or why the entry failed.
The entry is not claimed fixed.
