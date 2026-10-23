# B1t: live `^Font` read, no hit at `0x47E4AD`; source not observed, Q1 parked

2026-10-23. Directive Q1 round 3 (the explicit cap exception). Packet: `build/tmp/b1t-font-live-read/`
(`round-note.md`, `prediction.md`, `run/trace-result.json`, `run/gdb-mi.log`, `script.diff`,
`launch-command-sequence.txt`, hashes). Script: `tools/harness/trace_font_source.py --launch`. One stock start
(13:53:07 MDT, one-shot task in session 1), spent; no retry.

## Result (OBSERVED, one run)

- `starti` works on the stock exe under the mingw32 GDB. The entry stop is in SysWOW64 `ntdll` (`0x774c91a9`),
  not at the exe's first instruction. The bytes at `0x47E4AD` read `8b95d8feffff8b45e8` (site guard passed).
- The hardware breakpoint, its condition and its actions were all accepted (`type="hw breakpoint"`).
- The game then ran the full 90 s boot timeout (about 55 threads) with **zero breakpoint stops**.
  `FONT_PROBE_STOP` never printed and `hits` is empty.
- The breakpoint was deleted before detach, GDB exited 0, and the debugger was gone afterwards.
  The report was written on the timeout path with `wait_error`.
- **The source is not observed.** Per the prewritten falsifier, no hit says nothing about the source.
  Not separable from this run: (1) the loose route, which the archive-hit arm never sees (the fallback pair
  `{0x47E4AD, 0x47E463}` was not armed; the brief allowed one breakpoint); (2) a load before the breakpoint
  was effective; (3) a debug register that did not reach the loading thread; (4) a condition that did not
  match the real entry name (the hit count was never read before cleanup deleted the breakpoint).
  The worker did not rank them, and neither do I.

## Script changes (offline, before the start)

`--stop-after` defaults to 2 with `--launch` (4 with `--pid`); a timeout copies the hits already fired into the
report and records `wait_error`; the launch flow runs end to end against a recording stub, which found and fixed
two bugs (a `*stopped` record consumed while waiting for `^done`; a relative dump path under the install
directory). The `dump binary memory` path never ran live.

## Facilitator check

Read the trace report and `gdb-mi.log` myself: no `FONT_HIT`, no `breakpoint-hit`, two `*stopped` records (entry
stop and the timeout interrupt), one `bkpt` insert. Re-read `0x47E4A0..` on the exe with `read_steppers.py`
(`je 0x47e4fc` at `0x47E4AB`, the arm at `0x47E4AD`). Install hashes before and after equal each other and the
startup record. The script hash on mouv equals the Mac copy. Selftest passes on mouv. Mutation alone: the launch
`--stop-after` default 2 to 4 makes the selftest fail on exactly "launch default --stop-after is 2" (run on a
scratch copy, deleted afterwards; the real file hash is unchanged). Full ctest 31/31. After the run the game was
still running (pid 42624, session 1, started by the task, debugger detached); I ended it once I had checked it,
and no Lemmings or gdb process or `LemWeb-*` task remains. Not verified by me: the live key/launch events
themselves (taken from the log), and anything about why there was no hit.

## Consequence

Q1 is parked: no observed source, so Q2 (B1 render) does not run and nothing may be chosen by score. A fourth
round would need a different method (both arms armed, or a breakpoint not dependent on the entry stop thread),
which is outside this directive's one start for Q1.
