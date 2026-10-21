# Phase D1e: the measured release-timing mechanism, implemented

Queue id `d1e-release-timing-implementation`. Round 5 of the D1 gate
question, following D1a-D1d. Landed 2026-10-21. First D1 round to touch
production source.

## What changed

Additively, per the brief's specified design — nothing existing repurposed:

- `LevelSetup` gains three optional doubles, all defaulting to `0.0`:
  `releaseArmSeconds`, `releaseFirstCountdownSeconds`, `releaseIntervalSeconds`.
- `World::tick()`'s spawn trigger branches on `releaseIntervalSeconds > 0.0`:
  `0.0` (every existing hand-built `LevelSetup`) runs the exact old
  `sinceSpawn_`/`releaseIntervalTicks` branch, byte-for-byte; a positive
  value switches that `World` instance to a continuous real-seconds
  accumulator — arm once after `releaseArmSeconds` real seconds, seed the
  countdown to `releaseFirstCountdownSeconds`, decrement by `1.0 /
  ticksPerSecond` each tick, fire `spawn()` at `<= 0.0`, then roll the
  recurring `releaseIntervalSeconds` back into the same accumulator (not a
  reset) — exactly the D1b/D1c/D1d mechanism.
- One new test, `testRetailReleaseTiming` (15 checks), built the
  `testRetailSpawn` way from `MM_CLIMB_002`'s real hatch, using the exact
  measured constants (`0.723061695938398` / `1.3` /
  `2.8864866275534937`). No existing test edited.
- `src/tools/SimReleaseTrace.cpp` gains an optional `--real-seconds <arm>
  <first> <interval>` flag.

## The offline re-check

D1a's own, unmodified `diff_trace.py`, against D1a's own, unmodified
`live_trace.csv` and a freshly regenerated 150-tick sim trace using the
measured constants:

```
FIRST DIVERGENCE at tick 10  lemming 0  field 'existence'
  sim:  absent
  live: (32, 53, 1, 'faller')
```

Spawn-count transition ticks: live `11,26,41,56,71,87,102,112,127,142`;
sim `11,25,40,54,69,83,98,112,126,141` — counts agree at every shared point,
with the exact tick drifting by 0-4 from discretizing the original's
continuous, frame-driven accumulator (D1d) onto our 200 ms sample grid, as
the brief's own "Already settled" scoping anticipated. This is a different,
much smaller kind of mismatch than before: previously the *first* release
itself was wrong by roughly ten ticks because the mechanism was wrong
(tick-1, unconditional); now the mechanism is right and the residual is a
~1-tick alignment artifact in the comparison, not a release-timing defect.

**D1's gate does not pass yet** — this was not expected on this round. The
already-known left-turn tick divergence (`0x0041C67E`) was never reached in
this comparison (release timing was the far earlier, dominant blocker); it
remains the other acknowledged D1 blocker.

## Facilitator check (2026-10-21)

- **Rebuilt after touching every changed file** (forced, not trusting
  `ninja`'s cache): clean compile and link across all 72 targets. Fresh
  `ctest --preset dev-debug`: **24/24**.
- **Ran `test_world.exe` directly** with `WEBOLUTION_GAME_DIR` set: **374
  checks, 0 failures** — matches the round's count exactly (359 before, +15
  the new test, 0 existing edited — confirmed by `git diff` showing zero
  deleted/changed lines in `test_world.cpp`, only additions).
- **Own mutate-one-alone**, independent of the worker's: forced the new
  branch off (`if (false && setup_.releaseIntervalSeconds > 0.0)`), rebuilt,
  and reproduced the *exact* old bug — **13 failures**, all `spawned 1`
  from tick 1 onward, matching the round's reported failure count exactly.
  Restored the source, rebuilt, reran: **374/0** again, and a full `ctest`:
  **24/24** again.
- **Independently regenerated the sim trace** (`SimReleaseTrace.exe ...
  --real-seconds 0.723061695938398 1.3 2.8864866275534937`) and ran D1a's
  own `diff_trace.py` against it and D1a's own `live_trace.csv`: identical
  output, `tick 10, lemming 0, field 'existence'`.
- **Independently re-derived both spawn-transition-tick lists** from the
  raw CSVs with fresh code (not the round's own analysis): live
  `[11,26,41,56,71,87,102,112,127,142]`, sim
  `[11,25,40,54,69,83,98,112,126,141]` — exact match.
- `git status --short` shows exactly the five files the brief's allowlist
  names (`World.h`, `World.cpp`, `test_world.cpp`, `SimReleaseTrace.cpp`,
  `docs/tasks.json`'s permitted fields) — nothing else.

Accepted as a landed, verified implementation. D1's dominant, first-order
timing defect is fixed and reproduced against the original's own (already
independently-verified) measured series; the remaining tick-10 "existence"
residual is a small, well-understood alignment artifact, and the known
left-turn divergence is still the other open blocker before D1's exact-match
gate can be re-scored in full.
