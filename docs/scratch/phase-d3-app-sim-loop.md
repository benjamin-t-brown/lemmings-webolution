# D3b: `sim::World` ticking inside `webolution`, its lemmings drawn on the tube

Queue id `d3b-app-sim-loop`. Round 1 of 3. Offline: builds, CTest, headless `--shot`
renders. No stock start, no live game. Full worker evidence:
`build/tmp/d3b-app-sim-loop/{prediction.md,round-note.md,before-A6.png,after-A6.png,
before-A7.png,after-A7.png,sim-tick-{0,15,75}.{png,stderr.txt},release-trace.csv}`.

## Question

Can `webolution --level MM_CLIMB_002 --sim` advance a `World` from the D3a loader at 5 Hz
and draw every live lemming through the same actor path the injected walkers use, while
every existing command line renders byte-identically?

## Answer: yes

- **`src/app/SimClock.h`/`.cpp`** (new): a 200 ms/5 Hz accumulator, same shape as
  `FixedCameraClock`. Never discards elapsed time; `takeStep()` caps at 10 ticks (2.0 real
  seconds) per `advance()` call, carrying any backlog beyond the cap to later frames rather
  than losing it.
- **`src/app/TubeScene.h`/`.cpp`**: `injectedWalkersFromWorld()` builds one
  `InjectedNativeWalker` per live (`alive()`) `sim::Lemming` through the existing,
  already-landed `terrainSampleFromLemming` adapter — `drawInjectedWalkers` stays the only
  actor draw path, no second one grown.
- **`src/app/main.cpp`**: new `--sim`/`--sim-ticks <N>` flags, off by default. The `World`
  construction and tick loop live entirely inside `if (args.sim)`, after every pre-existing
  assignment to `options.injectedWalkers` — a command line without `--sim` executes none of
  the new code and renders exactly as before.

## Byte-identical regression (acceptance criterion 1)

A6 (`MM_CLIMB_002`, canonical camera, default injected walkers):
`c4e6cf477e7f803d92329e5b223fe296b014801abb0552132d65133fb1a09ebf` before and after.
A7 (`MM_DIG_000`, second level, no injected walkers):
`78305c5c4911cb55071cd21fa982d99764aafd951b183e6315b5bef691f77a72` before and after. Both
exact. (The A6 hash also matches the value already on record in `docs/LESSONS.md`'s A6d9
entry for the same matched-state frame — independent cross-confirmation beyond this
round's own claim.)

## `--sim --sim-ticks N` matches `SimReleaseTrace` exactly (acceptance criterion 2)

At N = 0, 15, 75: every live lemming's cell, facing and state printed by `webolution --sim`
matches the corresponding row of a `SimReleaseTrace` run against the same
`loadRetailLevel` derivation, exactly — both tools build their `LevelSetup` from the same
D3a loader and call `World::tick()` the same number of times with no other divergence.

## `SimClock` unit test (acceptance criterion 3)

New CTest target `simclock`. Stated cap: 10 ticks (2.0 real seconds) per `advance()` call;
a backlog beyond the cap is carried, not discarded, and fully recovered over later frames.

## Named limitation, reported as predicted (not fixed, not hidden)

`drawInjectedWalkers` only has a `"walk"` sprite action loaded — no `"fall"`/`"climb"`/etc.
So only `sim::State::Walker` has real artwork; every other alive state (10 of 181
live-lemming-ticks in the sampled 75-tick window were `Faller`, drawn for exactly 2 ticks
right after each release) reuses the same walk strip at a fixed phase. `Dead`/`Exited`
lemmings are filtered out (they stay in `World::lemmings()` forever). No sprite was
invented and no animation-phase clock (A6b's `ActorAnim`) was wired into `World::tick()`,
per the brief's instruction and PLAN.md A6d's warning against feeding `Lemming::frame`
into that clock.

## Note on the concurrent D1n round

This round ran concurrently with `d1n-world-continuous-clock` (file-disjoint: `src/app/**`
vs `src/sim/World.*`). Every build/test in this round ran against whatever state D1n's
work was in at the time, which is fine: D1n's own change is additive/opt-in, so it does not
alter `World::tick()`'s external contract either way, and this round's checks are relative
(before/after the same build), not against a frozen `World.cpp`.

## Facilitator check (2026-10-21)

Independently hashed the four saved PNGs directly (not re-running the build, since D1n was
still actively editing `src/sim/World.*` in the same tree at the time) — reproduced all
four sha256 values exactly, including the cross-check against the A6d9 hash already on
record. Independently cross-checked the saved `--sim-ticks 75` stderr against the saved
`release-trace.csv` by direct text comparison — every cell/facing/state/spawned/saved/dead
field matches exactly. Reviewed the `main.cpp` diff directly: the new code path is
entirely inside `if (args.sim)`, placed after every pre-existing `injectedWalkers`
assignment, confirming the "every pre-existing command line unaffected" claim by
inspection, not just by the hash match.

Independent isolated mutate-one-alone, run outside the shared build tree (so as not to
race D1n's concurrent edits): compiled `test_simclock.cpp` + `SimClock.cpp` standalone with
`g++` (no CMake, no shared build directory), confirmed it passes; mutated the cap
comparison in `SimClock::takeStep()` from `>=` to `>` (one line), recompiled standalone,
confirmed the suite fails exactly as expected (`FAIL: one catastrophic frame takes at most
the stated cap of ticks`); restored to the worker-delivered SHA-256, recompiled standalone,
confirmed it passes again.

After D1n landed and the shared build tree was stable, did a full `cmake --build` +
`ctest --preset dev-debug`: **27/27**, confirming D3b's files integrate cleanly alongside
D1n's. No Lemmings Revolution process was started by this round or by this check.

**Landed.** D3c (click-to-assign in the app) depends on this round and D2a (both landed)
and may now be queued.
