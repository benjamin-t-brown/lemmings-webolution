# Phase D1a: first-divergence localization on MM_CLIMB_002 (no input)

Queue id `d1a-release-walkout-trace`. Round 1 of the D1 gate question ("PLAN.md
Phase D, D1 release and walk-out"). Landed 2026-10-21.

## What was built

- `src/tools/SimReleaseTrace.cpp` (new executable, `CMakeLists.txt` wired the
  same way as `lrtool`/`actorfixture`): builds the real `MM_CLIMB_002`
  `LevelSetup` the way `src/__test__/test_world.cpp::testRetailSpawn` already
  does for one tick (terrain from the stored collision grid, one `Entrance`
  from the hatch's own words via `webo::hatchSpawnCell`), then runs
  `World::tick()` for N ticks and prints one CSV row per lemming per tick.
  `World::spawn()` releases from the `Entrance` on its own; the tool never
  hand-places a `Lemming`.
- `tools/harness/measure_d1_release_trace.py` (new harness script): enters
  `MM_CLIMB_002` with the safe `select_level` + `restart_level.ensure_playing`
  pattern (never `goto_level.focus()`), confirms `RELEASED == 0` at entry,
  then samples the actor list (`0x004CBB18`) plus `RELEASED` (`0x004CBA78`)
  and `CLOCK` (`0x004CBAAC`) for 30 s / 150 ticks at a capped 50 Hz with **no
  further input at all**, and aligns the raw wall-clock samples onto the
  engine's own 200 ms tick grid (phase inferred from walker-X/climber-Y
  near-integer probe instants, generalizing `measure_steppers.tick_series` to
  several simultaneously-live lemmings).
- `build/tmp/d1a-release-walkout-trace/diff_trace.py`: compares the aligned
  live trace against the sim trace, tick by tick, and reports the first
  tick/lemming/field of disagreement.

## The measurement

One live capture (pid 25696, one authorized stock start). `RELEASED`
transitions, read directly off the raw dump
(`build/tmp/d1a-release-walkout-trace/live_raw.jsonl`), independent of the
alignment code:

| release | t (s) | gap from previous |
| --- | --- | --- |
| 1 | 2.0200 | — |
| 2 | 4.9200 | 2.90 |
| 3 | 7.8000 | 2.88 |
| 4 | 10.6800 | 2.88 |
| 5 | 13.5800 | 2.90 |
| 6 | 16.4600 | 2.88 |
| 7 | 19.3400 | 2.88 |
| 8 | 22.2200 | 2.88 |
| 9 | 25.1200 | 2.90 |
| 10 | 28.0000 | 2.88 |

Mean gap 2.8867 s (14.433 ticks at the measured 5 Hz / 200 ms tick — not a
whole number of ticks). This sits beside the existing `src/sim/World.h`
comment citing "the original released a lemming every 2.889 s on a level
whose table entry is 2.8865 s" — almost certainly the same level, now with a
fresh, independently-reproduced live series rather than a single cited
figure.

**The first release, at `t = 2.020 s` (~10.1 ticks), is neither tick 1 (what
our sim does) nor a full measured interval (2.887 s) after level start.** This
is a third possibility the round's own prediction did not list going in.

## The first divergence

```
FIRST DIVERGENCE at tick 1  lemming None  field 'released_count'
  sim:  1
  live: 0
```

Cause, read directly from `src/sim/World.cpp`: `World::World(LevelSetup,
Rules)` sets `sinceSpawn_ = setup_.releaseIntervalTicks` at construction
(line ~52), and `World::tick()` does `if (!timedOut() && ++sinceSpawn_ >=
setup_.releaseIntervalTicks) spawn();` (line ~755) *before* stepping any
lemming — so `++sinceSpawn_` already equals `releaseIntervalTicks + 1` on the
very first call, for **any** positive `releaseIntervalTicks`. Reproduced
directly (not just read): `SimReleaseTrace.exe ... 20 5` spawns on ticks 1, 6,
11, 16 for `releaseIntervalTicks=5`.

The released *order* is not wrong (both traces release the ten lemmings
0..9 one at a time, in the same order); only the *count*, from the very
first tick this capture can check, because the original visibly waits
~2 s before its first lemming while the sim does not wait at all. No
code change was made this round (out of the brief's scope); the left-turn
divergence at `0x0041C67E` (the previously known D1 blocker) was never
reached — this release-timing gap is earlier and separate.

## An alignment bug the capture itself caught, fixed offline

The first alignment pass pooled walker/climber probe-instant anchors
together with `RELEASED`-transition times as if they shared one tick phase.
The pooled residual spread came back `~0.2 s` — essentially the whole
period, the signature of anchors with no common phase. Splitting them apart
and re-measuring showed why: releases land on an almost-constant 2.887 s
real-time gap, not a whole number of ticks, so a release is a continuous
real-time countdown and not a valid tick-phase anchor. Fixed (and a second,
unrelated bug where a tick with zero matching lemming samples silently
dropped its `released` count instead of reporting `0`) **offline, from the
saved raw dump, with no second live start**.

## Open limitation, not chased this round

Restricted to walker/climber anchors only, the *pooled* residual spread is
still close to the full period, but *per lemming* the crossings are tight
(e.g. lemming 0 at residual ≈ −0.08, lemming 1 at ≈ +0.02 — each stable, but
at different phases). This is consistent with `docs/LESSONS.md`'s "X moves
once per rendered frame... in steps of about 0.03 world units": lemming
motion looks like a continuous per-object integration, with each lemming's
own phase set by when it entered its current state, rather than one shared,
globally-synchronized 200 ms tick firing identically for every lemming at
once. It does not affect the reported first divergence (off by whole
seconds, far past any ~100 ms phase ambiguity). Left as an open lead for
whichever future D1 round needs tick-exact alignment past the first
disagreement — not yet a measured fact with an address.

## Facilitator check (2026-10-21)

Independently re-verified, separately from the worker's own report:

- **Rebuilt** after touching the new source file (`cmake --build --preset
  dev-debug`): clean compile and link, confirming the new target is not
  stale. Fresh `ctest --preset dev-debug`: **24/24**.
- **`CMakeLists.txt` diff** is exactly the claimed two lines
  (`add_executable(SimReleaseTrace ...)` / `target_link_libraries(...
  PRIVATE webolution_objects)`), in the `lrtool` shape, nothing else
  touched (`git diff CMakeLists.txt`).
- **Reproduced the sim-side mechanism independently**: re-ran
  `SimReleaseTrace.exe "C:\progs\Lemmings Revolution" MM_CLIMB_002 20 5`
  myself; spawns land at ticks 1, 6, 11, 16 exactly as claimed, and the
  hatch read (`x 32 y 42 count 10 type 0 -> cell (32, 53)`) matches
  `testRetailSpawn`'s own MM_CLIMB_002 case. Re-read `World.cpp` lines
  46-53 and 751-764 directly: `sinceSpawn_ = releaseIntervalTicks` at
  construction, `++sinceSpawn_ >= releaseIntervalTicks` on the first
  `tick()` call is true for any positive interval — confirms the claimed
  cause from the actual source, not just the round note's paraphrase.
- **Reproduced the headline divergence directly from the raw capture**,
  without trusting the alignment code: `live_raw.jsonl`'s own `"event":
  "released"` rows show the first transition at `t = 2.020000100019388`,
  `before 0 after 1` — this alone, independent of any phase-alignment
  question, already contradicts the sim's immediate tick-1 release (nominal
  tick-1 time is `0.18-0.2` s under any reasonable phase choice, long before
  `2.02` s).
- **Reproduced `diff_trace.py`'s output exactly**
  (`FIRST DIVERGENCE at tick 1 ... sim: 1 live: 0`).
- **Mutate-one-alone, twice**: (1) built a synthetic "live" trace directly
  from the sim trace (a positive control) — `diff_trace.py` correctly reports
  "no divergence in 150 ticks checked"; perturbing one field at one
  tick/lemming (`x` at tick 50, lemming 0, `+1`) alone is caught exactly
  there and nowhere earlier. (2) patched *only* the `released` column for
  ticks 1-9 of the real live trace to match the sim's value — the reported
  divergence moves to tick 10 (a real, unpatched `released` mismatch further
  along), confirming the `released_count` field is specifically,
  independently responsible for the original reported result, not an
  artifact of the x/y/facing/state comparison path.
- **Live-game state, independently checked** (own `Get-Process`,
  `Get-FileHash`, `Get-ScheduledTask`, separate from the worker's own
  report): no `Lemmings Revolution` process, no `LemWeb*` scheduled task,
  installed exe hash `32675BDB...` matches the authority. `dgVoodoo.conf`
  differs from the windowed-swap file it was temporarily replaced with
  (i.e. was restored, not left swapped).
- **`git status --short`** shows only the allowlisted files changed
  (`CMakeLists.txt`, `docs/tasks.json`, plus the two new untracked files) —
  nothing landed-doc-adjacent was touched outside this facilitator pass.

Accepted as a landed, bounded negative result for round 1 of D1: D1's gate
does not pass (it was never expected to, on round 1), and the dominant,
*first* cause is now measured and attributed to a specific line of our own
sim code — but the fix is not yet known, because the original's actual
release-trigger mechanism (what determines the ~2.02 s first-release delay,
and whether the 2.887 s inter-release gap is level-specific or derives from
`RELEASERATE.PC` via a mapping this project has not yet found) has not been
read. See `PLAN.md` Phase D for the follow-up question.
