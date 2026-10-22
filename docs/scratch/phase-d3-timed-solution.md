# D3f: a timed scripted solution, recorded live and replayed in our World

Queue id `d3f-timed-solution`. Brief: `build/tmp/d3f-timed-solution-brief.md`.
Worker output: `build/tmp/d3f-timed-solution/round-note.md`.

User decision (2026-10-22, CEO review): do not amend D3's gate, and do not spend the
pending question's third pre-authorized round on a still-more-structured click-pacing
model. Instead, record exactly when each assignment landed in a live run (relative to that
lemming's own release) and replay our `World` with precisely those recorded times, rather
than any fitted delay model.

## What this round did

Added a read-only, additive 50 Hz side-channel timing watcher to
`tools/harness/measure_d3_solution.py` (`watch_assignment_timing` / `build_assignment_timing`)
that runs alongside D3d's already-proven click loop without touching `assign_skill.py`. It
polls the actor list and each lemming's climber byte and records, per pointer: first
actor-list appearance, the climber-flag landing instant, and last-seen (an exit/save proxy).
All of `measure_d3_solution.py`'s existing output keys and signatures are unchanged; the only
new key is `solution.json`'s `assignment_timing`.

A new round-local probe, `SimSolutionRunTimed.cpp` (extends, does not edit, D3e's own
`SimSolutionRun.cpp` pattern; no `src/sim/**`, `RetailLevel.*` or `CMakeLists.txt` change),
assigns climber to lemming `i` at `spawnTick[i] + offsetTicks[i]`, where the ten offsets are
read directly from the live run's own recorded `assignment_timing` table — not fitted or
swept, as D3e repair 1's two delay models were.

## An unanticipated, directly-measured finding: click-landing timing varies enormously between live runs of the identical script

D3d's live run ended at 208 s. This round's own live run of the exact same scripted
solution — same level, same ten climber assignments, same click mechanism, same engine
build — ended at **133 s**: a 75-second difference between two live runs of the identical
script, measured directly rather than inferred from a model sweep. This independently
confirms, by direct measurement, D3e repair 1's "click-pacing variance, not a `World`
defect, explains most of the gap" story, and at a larger magnitude than either of its two
fitted models predicted.

## Result: the self-consistent timed replay

| | this round's live run | `World`, fed this run's own recorded times |
| --- | --- | --- |
| saved | 10 of 10 | 10 of 10 — exact match |
| `finished()` / last save, exact | 125.726 s | 125.200 s (tick 626) |
| `finished()` / last save, truncated whole s | 125 s | 125 s — exact match |
| `end_time_seconds` (gate's own field, `ui_state` 0→4) | 133 s | 125 s (truncated) |

Every one of the ten scheduled assignments landed on exactly its own scheduled tick
(`activeAssignFailures 0`, `neverActiveAtAssignTime 0`, `clampedCount 0`).

On the quantity `World::finished()` actually measures, the match is exact to the whole
second (125 vs 125) and within half a second at full precision — inside D1's already-parked
few-tick per-event budget, not a new residual family.

Against the gate's own literal `end_time_seconds` field, the residual is **-8 s**. This round
independently re-measured, a second time on a different absolute timescale, the
last-save-to-`ui_state`-transition delay D3d's own round already named but did not quantify
twice: 6.345 s here vs D3d's 6.4 s — two live captures 75 s apart in absolute end time agree
on this specific delay to within 0.1 s, itself evidence the delay is a stable feature of the
original (not capture noise) that `World` does not model. That accounts for -6.3 s of the
-8 s; the remaining -1.7 s (whole-second scale) / -0.53 s (continuous-clock scale) is left
unattributed beyond D1's already-parked few-tick budget — not folded into either bucket by
assumption, and no source address was read for it this round.

Both `--continuous-stepper-clock` settings give the identical `finishedAtTick` (626); the
flag changes five lemmings' own exit tick by exactly one tick each, consistent with
D1n/D1o's own "reduces, does not close" characterization, not a new finding.

## Facilitator check

- Reviewed `measure_d3_solution.diff` directly: purely additive (new optional `t0` parameter
  on `watch_to_level_end`, defaulting to `None` — unchanged behavior for every existing
  caller; one new output key `assignment_timing`; no existing key, signature or CLI default
  removed or altered).
- Independently recompiled `SimSolutionRunTimed.cpp` from source
  (`g++ -g -std=c++23 -Wall -Wextra -Wpedantic -IC:/progs/lemmings-webolution/src -c ...`,
  clean compile, no warnings) and relinked against the current `webolution_objects` object
  files with the round's own saved `link_cmd.txt` command (path-substituted only).
- Independently reran the sanity-zero check (all-ten-offsets-0): `FINAL: lastTick 514
  finishedAtTick 514 wonAtTick 384` — exactly reproduces D3e's own result, confirmed by my
  own binary, not the worker's.
- Independently reran the full timed replay with the ten offsets read directly from
  `solution.json`'s own `assignment_timing` table (`6,3,6,9,14,8,19,12,5,37`), both clock
  settings: both give `finishedAtTick 626` (125.2 s), matching the round's headline exactly.
  The full output CSVs are byte-identical to the worker's saved `replay_off_stdout.csv` /
  `replay_on_stdout.csv` (differing only by a UTF-8 BOM PowerShell's own redirection adds to
  the header line — confirmed identical from line 2 onward in both).
- Independently recomputed the 6.345 s last-save-to-`ui_state`-transition delay directly from
  `solution.json` / `watch_samples.json` raw data (not the round's own printed number): max
  `exit_t` across all ten lemmings is 125.72596..., the final watch sample's `t` is
  132.07096..., difference 6.345004... — matches exactly. Confirmed `end_saved_count: 10`,
  `end_time_seconds: 133`, `won: true`, `reached_level_end_state: true` directly from the
  JSON.
- Confirmed no `Lemmings Revolution` / `dgVoodoo` process running, and the installed exe's
  SHA-256 (`32675bdb...3737a2f`) matches the authority constant.
- `git status --short` showed only `docs/tasks.json`'s two permitted fields (`status`,
  `worker_output`) and the additive `tools/harness/measure_d3_solution.py` diff — no
  `src/**`, `CMakeLists.txt`, `PLAN.md`, or other `docs/**` file. `stock_starts_consumed`
  was correctly left at 0 by the worker (outside its own edit permission) and is corrected
  to 1 here.

Round accepted as reported; nothing narrowed. **D3 does not pass the gate literally** —
and, more importantly, this round demonstrates the gate's own premise (a scripted solution
has *a* reproducible end time to match) does not hold: two live runs of the identical script
differ by 75 real seconds, before any sim comparison even enters the picture. Per the user's
2026-10-22 decision (do not amend, do not run a further click-model round), and consistent
with the round cap already having been spent on this question across D3d/D3e/D3f, **D3 is
parked in Phase E** at this result: saved count exact, the quantity `World::finished()`
computes matches its own run's ground truth almost exactly, and the gap against the gate's
literal field is understood and bounded (-8 s, of which -6.3 s is a named, twice-measured,
unmodeled delay, and the remainder is small enough to fit inside D1's own already-parked
budget). Work proceeds to Phase B (B1).
