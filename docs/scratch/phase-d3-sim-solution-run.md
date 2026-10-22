# D3e: D3d's scripted solution run through our own `sim::World`

Queue id `d3e-sim-solution-run`, round 1 of 3. [Brief](../../build/tmp/d3e-sim-solution-run-brief.md).
Entirely offline: zero stock starts, no live game process at any point.

## What this round did

A round-local probe (`SimSolutionRun.cpp`, linked against the already-built
`webolution_objects` object files — the same technique D2f/D2g/D2h/D2i's own
`SimAssignProbe.cpp` used) loads `MM_CLIMB_002` via `loadRetailLevelReport`,
unmodified — the same production `LevelSetup` the app's own `--sim` path uses,
including the real-seconds release model and the default `continuousStepperClock =
false`. Starting from a fresh `World`, it ticks with no human input except: as soon
as a lemming is `active()` and not yet assigned, call `assign(index, Skill::Climber)`
once. This matches `World::assign`'s own `Skill::Climber` branch exactly (confirmed by
direct read: it requires only `l.active() && !l.canClimb`, no `State::Walker`
requirement), so assignment can and does land on a lemming's very first active tick,
before it has ever reached a wall.

## Result: saved count matches exactly; end time does not

```
FINAL: saved 10 dead 0 spawned 10 assignedCount 10 climberStockLeft 0
finished() at tick 514 (102.8 s); won() at tick 384 (76.8 s)
```

| | Original (D3d) | Our `World` |
| --- | --- | --- |
| Saved | 10 of 10 | **10 of 10 — matches exactly** |
| End time | **208 whole seconds** | **102 s truncated / 103 s rounded (514 ticks)** |

**Saved count matches exactly. End time does not — our run finishes in about half the
original's time**, a gap of ~105-106 seconds, not a few-tick residual. `won()`
(`saved_ >= saveTarget`, target 1) fires at tick 384, 130 ticks (26 s) before
`finished()` on the very first save — the same won-vs-all-resolved asymmetry D3d's own
live capture already showed (7 lemmings already saved before its watch even began).

## A concrete, well-isolated candidate cause (named, not proven)

Every one of our ten lemmings takes **exactly 373 ticks** from its own spawn to its
own save, with zero variance (`spawn 11→save 384`, `spawn 25→save 398`, ... `spawn
141→save 514`, all diff 373). This is expected: assignment lands on the very first
active tick, before any lemming has ever touched a wall, so every lemming's path is
the shortest possible one — walk straight to the first wall, climb immediately, no
back-and-forth — identically for all ten (no lemming-vs-lemming collision exists in
this engine, already an established fact from round 8's blocker investigation).

D3d's own round-note is explicit that the original's own assignment process was **not**
instantaneous: "all 10 assignments landed on the first or a quick retry," and "the
assignment loop itself took" about 95 seconds of real/game time — comparable in
magnitude to this round's own 102.8 s total, and on the same order as the ~105 s gap
measured here. The candidate story: in the live capture, lemmings spent real, ticking
game time walking (possibly reaching a wall and turning back) before their own climber
click actually landed — this probe's "instant at spawn" rule cannot reproduce that by
construction.

**Explicitly ruled out as the (sole) explanation**: D1's own already-landed per-event
residuals (at most a handful of ticks per spawn/landing event) are far too small, even
summed across ten lemmings, to account for a ~525-tick gap. The assignment-timing-model
difference is the candidate that matches the gap's own magnitude; D1's known drift does
not.

## Facilitator check (2026-10-22)

**Note on this round's own delivery**: the worker that ran this round was interrupted
by a session rate limit immediately after writing the complete `round-note.md` and all
artifacts, before it could update `docs/tasks.json`'s own `status`/`worker_output`
fields (which were found still reading `"running"`). No continuation was needed — the
deliverable itself was complete; only the queue bookkeeping was finished here.

Independently recompiled `SimSolutionRun.cpp` from source, relinked against the current
`webolution_objects` object files, reran with the same arguments (`MM_CLIMB_002`,
tick cap 2000): **byte-for-byte identical stderr output** — `FINAL: ... saved 10 dead 0
spawned 10 ... finished() 1 won() 1`, `finishedAtTick 514`, `102.8 s`/`wonAtTick 384`,
reproduced exactly. Directly read `src/sim/World.cpp`'s `assign()` entry guard
(`index`/`l.active()`/stock check) and the `Skill::Climber` branch
(`if (l.canClimb) return false; l.canClimb = true;`) — confirms the round's claim that
no `State::Walker` requirement exists. Directly read the wall-hit climb transition
(`if (rise > rules_.maxStepUp) { ...; if (l.canClimb) { l.state = State::Climber; ...
} }`) — confirms the "instant climb-enable means identical shortest path for every
lemming" explanation. Independently recomputed the spawn-to-save tick differences from
the round's own `run_stdout.csv`: all ten pairs give exactly 373, no variance. `git
status` shows only `docs/tasks.json`'s permitted fields and this round's own
gitignored `build/tmp/**` files — no `src/**`, `CMakeLists.txt`, or other `docs/**`
file touched, and no live game was ever started this round (zero stock starts, as
authorized).

**Accepted as reported. D3's gate does not pass**: the saved-count half is met exactly;
the end-time half is not, by a large, precisely-measured, well-explained-but-not-yet-
confirmed margin. The assignment-timing-model hypothesis is falsifiable and testable
offline (vary the probe's own assignment delay, not `src/**`, and see what delay
reproduces ~208 s) before considering any live re-capture or `src/**` change.
