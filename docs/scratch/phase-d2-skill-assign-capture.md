# D2f: one live climber assignment, 50-tick actor-list trace (D2 part (b), climber)

Queue id `d2f-skill-assign-capture`, round 1 of 3. [Brief](../../build/tmp/d2f-skill-assign-capture-brief.md).

## What this round did

Reused already-proven tooling end to end, nothing new live: `tools/harness/assign_skill.py`
(pre-reset, 38/38 historical click-to-assign hit rate) for the panel click, and
`tools/harness/measure_d1_release_trace.py`'s own `capture()` for the post-click sampling.
The new file, `tools/harness/measure_d2b_skill_trace.py`, composes those two primitives
and nothing else — no new memory write, no new click mechanism.

One stock start (pid 5708) on `MM_CLIMB_002`: entered with no input, waited for the
hatch, selected panel index 0 (climber), and clicked the first hoverable lemming found
(second attempt — the first found no hoverable lemming yet, matching the pre-reset
round's own "Observation 2" about lemmings still in the hatch-release grace). The
assignment landed on the first click actually sent: `[lemming+0x48]` (the climber
attribute) flipped `0 -> 1` and the stock count at `0x004C91D8` index 0 went `10 -> 9` in
the same instant, independently re-confirmed a third time after a full 14-second
(70-tick) post-click sample held both values unchanged.

The assigned lemming's own 50-tick trace, read directly off `live_trace.csv`:

```
tick  x   y  facing  state
   1  36  63    1    walker
  ...                         (walks +1 column/tick, 27 more ticks)
  28  63  63    1    walker
  29  64  63    1    climber   <- hits the column-64 wall, starts climbing
  ...                         (rises exactly 1 cell/tick, 14 ticks)
  42  64  50    1    climber
  43  64  49    1    hoister   <- climb -> hoist (topout) transition
  44-46   64  49    1    hoister
  47  64  47    1    walker    <- resumes walking, 2 cells above the wall top
  48  65  47    1    walker
  49  66  47    1    walker
  50  67  47    1    walker
```

This independently reconfirms (not re-derives — the values are already `MEASURED` in
`World.h`'s `Rules`) `climbSpeed = 1` cell/tick (14 consecutive 1-cell climb steps, no
exceptions) and `climberTopOut = 2` cells (the walker resumes exactly 2 cells above the
wall-entry row).

## The offline comparison

A fresh `webo::sim::World` was seeded with one `Lemming` already a `Walker` at the
live-measured cell (`x=36, y=63, facing=Right`) via the already-existing
`World::place()` — whose own doc comment says this is exactly its purpose ("a live
comparison starts from a measured cell instead of that drop") — then
`World::assign(0, Skill::Climber)` was called once and the sim ticked forward 49 times.
This sidesteps D1's own still-unaccepted release-schedule question entirely (it is not
replayed from level start) rather than solving it.

The probe (`SimAssignProbe.cpp`, round-local, **not** a CMake target, **not** referenced
by `CMakeLists.txt`) was compiled and linked directly against the already-built
`webolution_objects` object files, using the exact object list and flags already recorded
in `build/cmake/dev-debug/build.ninja` for `SimReleaseTrace.exe`'s own link line. No file
under `src/`, `CMakeLists.txt`, or any must-not-touch harness file was modified.

Result (`diff_assign_trace.py`, sim tick T vs live tick T+1 by construction — the sim's
own tick-0 marker IS live's already-aligned tick-1 row, so no inferred phase-matching was
needed):

```
checked 50 aligned tick-pairs (shift=1), 45 exact matches
FIRST DIVERGENCE: sim tick 42 vs live tick 43
  sim:  (64, 49, 1, 'climber')
  live: (64, 49, 1, 'hoister')
```

**45 of 50 compared tick-pairs match exactly** — the full 28-tick walk approach and the
full 14-tick climb run, X/Y/facing/state all exact every tick. The one divergence is a
**single, bounded, one-tick-early state label** at the exact same cell: both reach
`(64, 49)` at the corresponding tick, but live is already `hoister` there while our sim
is still `climber` for one more tick. The lag then propagates forward as a constant
one-tick shift for the remaining 7 captured rows (hoist duration and post-hoist walk
speed are both unaffected and still exactly right) — not a growing or compounding error.

Stated as a hypothesis, not yet confirmed against the original's bytes: our `World`'s
climb stepper (`stepClimber`) evaluates the topout-transition check using the
*pre-this-tick* Y position before moving, so it can only detect "we have reached the
topout row" on the tick *after* Y has already arrived there — one tick later than the
original, which (on this one sample) hands off to hoist the same tick Y reaches the
topout row.

## Facilitator check (2026-10-22)

**Confirmed `World::place()` already existed before this round** (git diff on
`src/sim/World.h`/`World.cpp` is empty; the method and its exact doc comment, cited
verbatim in the round's own note, are present in the file at the lines claimed) — this
round did not add new API surface to a must-not-touch file.

**Independently recompiled the probe from source**, not reusing the worker's `.exe`:
rebuilt `SimAssignProbe.cpp` to a fresh object file with the same flags, relinked against
the current `webolution_objects` object files (the same ones a fresh, no-op
`cmake --build --preset dev-debug` confirmed were already up to date), and reran with the
worker's exact seed arguments (`36 63 1 49`). Output is **byte-identical** to the
worker's own saved `sim_trace.csv` (`diff`, no output). Independently reran
`diff_assign_trace.py` against this freshly-produced trace and the worker's unmodified
`live_trace.csv`: reproduces the exact same result (45/50, first divergence at sim
tick 42 / live tick 43, identical values).

**Independently extracted the raw data myself**, not trusting the round's printed table:
read `live_trace.csv` directly with fresh Python, pulled every row for `lemming_index 0`
(50 rows, ticks 1-50, no gaps) — matches the round's printed table exactly, cell for
cell. Read `live_trace.meta.json` directly: `assigned_lemming_index_in_order` is `0`
(confirming which lemming is the assigned one), `counts_before_click`/`counts_after_quick`/
`counts_after_capture` are `[10,...]`/`[9,...]`/`[9,...]` (stock held at 9 through the
full window), `climber_byte_before`/`_after_quick`/`_after_capture` are `0`/`1`/`1` (held
through the full window) — both claimed independent re-confirmations are real, not
asserted.

Independently re-verified process/install state: no `Lemmings`/`dgVoodoo` process after
the round's work; installed exe and `dgVoodoo.conf` hashes (`32675bdb...`,
`d639e220...`) match the authority and the round's own before/after values. `git status`
shows only `docs/tasks.json`'s permitted fields and one new untracked file
(`tools/harness/measure_d2b_skill_trace.py`) — no file under `src/**`, `CMakeLists.txt`,
or any other harness/doc file was touched. Spot-read `measure_d2b_skill_trace.py`'s
`click_and_capture`: it sends the click via `inp.button(0, True)`/`wait_frames(4)`/
`button(0, False)` (a real input-shim click) and calls `assign_skill.select_skill`/
`nearest_hoverable` and `measure_d1_release_trace.capture` by reference — no
reimplemented click mechanism, no new memory write, consistent with the brief's "reuse,
do not reinvent" instruction.

`ctest` was correctly not re-run by the worker (nothing under `src/`/`CMakeLists.txt`
changed); no rebuild was needed for this check either, confirmed by `ninja: no work to
do` immediately before compiling the verification probe.

**This round is accepted as reported: a genuine, reproducible 45/50-tick match with one
specific, bounded, well-characterized residual.** D2 part (b) is not yet settled (only
one of four skills attempted, and even this one skill has an open residual) — the
climb-to-hoist one-tick lag is queued as repair 1 (round 2 of 3) rather than chased
further here, per the brief's own scope.
