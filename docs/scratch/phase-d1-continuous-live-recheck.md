# D1o: fresh live D1 recheck, scored against the continuous stepper clock

Queue id `d1o-continuous-live-recheck`. Brief: `build/tmp/d1o-continuous-live-recheck-brief.md`.
Worker output: `build/tmp/d1o-continuous-live-recheck/round-note.md`.

User decision (2026-10-22, CEO review): option 4 on D1's pending gate-amendment
question — leave D1 parked and proceed with the rest of Phase D and beyond — but take one
more live round first, since D1n's continuous-stepper-clock improvement
([D1n](phase-d1-continuous-stepper-clock.md)) had only been checked against three *saved*
captures. This round measures; it does not tune. D1 is parked after this round regardless
of the result, unless the result passes the gate literally.

## Result

One fresh live, no-input capture of `MM_CLIMB_002` (150 ticks / 30 s, the one authorized
stock start), scored with both `--continuous-stepper-clock` off and on against the D1 gate's
literal text (`MM_CLIMB_002`, no input, from level start: for 150 ticks every lemming's cell
X/Y, facing and state match the original each tick, and the released count and order match).

**Fails literally under both settings.**

| | flag off | flag on |
| --- | --- | --- |
| total divergences | 657 | 305 |
| x | 592 | 252 |
| y | 28 | 28 |
| facing | 15 | 8 |
| state | 13 | 8 |
| released_count | 9 | 9 |
| spawn-tick drift | 4/10 nonzero, max 1 | 4/10 nonzero, max 1 (identical) |
| landing-tick drift | 10/10 nonzero, max 2 | 8/10 nonzero, max 1 |
| released order | matches | matches |

The flag-on improvement generalizes to fresh data (657→305, a 54% cut), confirming D1n's
saved-capture result was not an artifact of those three captures. Two things the round's own
prediction did not anticipate, both traced to source rather than left as bare numbers:

- `y` does not collapse at all here (28→28), unlike any of D1n's three saved captures,
  because this capture's landing-tick-drift improvement (8/10 lemmings still nonzero) is
  itself weaker than on any of those three (5/10, 3/10, 5/10) — the comparator is a strict
  per-tick integer-cell equality test, so a smaller drift-count improvement shows up as no
  `y` collapse at all rather than a partial one.
- The `x`-propagation improvement is binary per lemming, not proportional: it drops to
  exactly 0 only for the lemmings whose on-flag landing drift happens to equal their own
  spawn drift (a spare tick of budget to absorb via the same-tick landing→walker handoff),
  and is completely unchanged for the rest. Re-derived twice directly from the raw CSVs.

Spawn-tick drift is unchanged between flag settings (identical set, identical magnitudes),
confirming D1n's structural claim — `World::spawn()` fixes the tick before any continuous
budget exists — on new data. The Phase E facing-label family is present under both settings
(15 off, 8 on, same two wall columns), not reopened. First divergence (tick 11,
`released_count`) matches D1l's own tick-11 alignment artifact exactly, as expected since the
flag touches only fall/walk, not release.

## Facilitator check

Independently reproduced every headline number, not by re-running the worker's own script
output but by regenerating it from scratch:

- Rebuilt `SimReleaseTrace` (`cmake --build --preset dev-debug --target SimReleaseTrace`;
  Ninja reported no work — the binary already matched the committed source).
- Regenerated both sim traces myself from the installed binary with the round's exact
  arguments (`MM_CLIMB_002 150 5 10 --real-seconds 0.723061695938398 1.3
  2.8864866275534937`, with and without `--continuous-stepper-clock`), via PowerShell (a
  Bash-tool attempt against the quoted `C:\progs\Lemmings Revolution` path silently produced
  an empty file — a path-quoting artifact of the shell, not a real discrepancy, resolved by
  rerunning through PowerShell). Diffed against the worker's saved `sim_trace_off.csv` /
  `sim_trace_on.csv`: identical except for a UTF-8 BOM PowerShell's own redirection added to
  the header line; byte-identical from line 2 onward in both files.
- Independently reran `diff_trace_all.py` (imported unedited from D1a's own already-verified
  `diff_trace.py`) against the worker's saved `live_trace.csv` and both sim traces: my own
  fresh run's per-field counts match the round's table exactly — off: x 592, y 28, facing 15,
  state 13, released_count 9, 4 coverage gaps (total 657 after excluding the gap-notice
  lines); on: x 252, y 28, facing 8, state 8, released_count 9, 4 coverage gaps (total 305).
  My raw line counts (661, 309) are exactly 4 higher than the reported totals in both cases —
  confirmed this is the 4 coverage-gap notice lines, which the round's own table caption
  already states are excluded from the total, not a discrepancy.
- Confirmed no `Lemmings Revolution` / `dgVoodoo` process running, and the installed exe's
  SHA-256 (`32675bdb...3737a2f`) matches both the round's claim and
  `measure_d1_release_trace.py`'s own `STOCK_SHA256` constant.
- Fresh `cmake --build --preset dev-debug` (no work) + `ctest --preset dev-debug`: 28/28.
- `git status --short` showed only `docs/tasks.json`'s two permitted fields
  (`status`→`ready-for-verify`, `worker_output`) touched by the worker — no `src/**`,
  `tools/harness/**`, `PLAN.md`, or other `docs/**` file. `stock_starts_consumed` was
  correctly left at 0 by the worker (outside its own edit permission) and is corrected to 1
  here.

Round accepted as reported; nothing narrowed. D1's gate does not pass under either setting.
Per the user's 2026-10-22 decision, **D1 is now parked** (Phase E) at this result: the
continuous-stepper-clock fix is confirmed to generalize and meaningfully helps, but does not
close the gate, and no further D1 round is authorized by this decision. Work proceeds to D3
(`d3f-timed-solution`), then Phase B/C.
