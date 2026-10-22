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

## Round 2 (repair 1): a structured click-pacing model gets close, but no integer value hits 208 s exactly

[Repair 1 brief](../../build/tmp/d3e-sim-solution-run-repair1-brief.md). Offline only;
zero stock starts; no live game process at any point.

A parametrized copy of round 1's probe (`SimSolutionRunDelay.cpp`) tested two
assignment-delay models, swept over many values, entirely offline:

- **`uniform D`**: each lemming's own assignment is delayed `D` ticks after that
  lemming's own spawn (independent per-lemming delay — round 1's own hypothesis in
  its simplest form).
- **`serial CI`**: a single globally-paced serialized clicking process, one click
  every `CI` ticks, processed in spawn order (modelling one person serializing ten
  clicks, rather than ten independent per-lemming delays).

**Correction to round 1's own cited anchor, found by re-reading the oracle's raw log
this round**: D3d's round-note's "~0,3,6,9,12,15s" click-landing estimate is **not**
literally computable from `build/tmp/d3d-solution-discovery/run1.log` — that file logs
each assignment attempt but prints no `t=` timestamp until the watch phase, which only
begins after all ten clicks have already landed. What *is* directly computable: from
`clock_at_entry=298` and the first watch sample's `clock=187`, the entire ten-click
assignment loop took **111 real/game seconds end-to-end** — a different, larger
number than the brief's cited estimate, reported honestly rather than silently
reconciled.

**Both models are step functions, not linear** (confirmed by direct tick-trace diff): a
1-tick change in the delay parameter can shift the finish time by 112 ticks — the cost
of one lemming narrowly missing the wall-climb window and having to walk a full extra
lap. **Neither model's integer sweep lands on exactly 1040 ticks (208 s)**:
`uniform` jumps 994→1106 between `D=513` and `D=514` (nearest: 198.8 s or 221.2 s);
`serial` jumps 1011→1106 between `CI=71` and `CI=72`, with a closer intermediate value
at `CI=77–78` giving 1074 (214.8 s, +6.8 s over).

**`uniform`'s closest value is implausible**: it would require even the *first*
assigned lemming to wait over 100 seconds, directly contradicted by `run1.log`'s own
record of the first click landing within two attempts of the first spawn. **`serial`'s
closest value (14.2–15.6 s/click) is far more plausible**: structurally consistent with
one person serializing ten clicks, and its implied total loop duration (≈128–140 s) is
the same order of magnitude as the independently-computed 111 s figure above (not an
exact match, but a real, loose agreement between two independently-arrived-at numbers).
D1's own already-landed per-event residuals remain explicitly ruled out — the needed
delays here are two to three orders of magnitude larger.

### Facilitator check (round 2)

Independently recompiled `SimSolutionRunDelay.cpp` from source, relinked against the
current `webolution_objects` object files; the `uniform 0` sanity case reproduces round
1's exact result (`finishedAtTick 514`, `wonAtTick 384`). Independently reran the four
cited boundary points: `uniform 513` → `994`, `uniform 514` → `1106`, `serial 71` →
`1011`, `serial 72` → `1106`, `serial 77` → `1074` — all byte-for-byte matching the
round's own sweep table. Independently confirmed the "111 s" clock arithmetic directly
from `build/tmp/d3d-solution-discovery/solution.json` (`clock_at_entry: 298`) and
`run1.log` (`t   0.00s saved=7 clock=187 ...`): `298 − 187 = 111`, exactly as claimed.
Independently grepped `run1.log` for `t=`/timestamp markers before the watch phase:
none exist — confirms the round's correction to the brief's own cited anchor is
accurate, not a misreading. `git status` shows only `docs/tasks.json`'s permitted
fields — no `src/**`, `CMakeLists.txt`, or other `docs/**` file touched; zero stock
starts used.

**Accepted as reported.** Neither delay model reproduces the original's exact 208 s;
the `serial` model's closest approach is within 5.8–6.8 s (under 4% of 208 s) and is
mechanistically and numerically plausible, while `uniform`'s is not. This is round 2 of
the question's 3-round cap. Whether to spend round 3 on a still-more-structured model,
accept this as a close-but-not-exact result and raise D3's own gate wording as a
pending question (the same shape of decision already deferred for D1), or something
else is recorded as a pending question for the user, not decided unilaterally — see
`standing_run.pending_questions`.
