# actor-motion-001: smooth presented lemming positions

Status: **needs_review**. Playable v1, roadmap step 4, human playtest P7 in
[playtest 1](../playtest/2026-11-12-playtest-1.md). Difficulty: difficult;
recommendation: Sonnet-class, with this worker using the explicit Codex provider
override. Full registered main/recovery verification is **pending supervisor
execution**, followed by independent acceptance review. No original execution,
oracle, debugger, hooks, original capture, subagents or background jobs.

## Failure, dependency and production path

The player reported intermittent jumps on every level. Accepted actor animation
already makes the sprite strip advance, but its origin still follows integer
simulation cells. Smooth placement is the next dependency for reading movement.
This task changes presentation, without changing World positions, rules, release,
tick cadence, shared timing, skills or any fidelity gate/park.

Actual installed route: direct `runApplication --level MM_CLIMB_002 --sim
--native`, natural hatch release, walking along the tutorial's first ledge,
pause, public hover and click to assign Climber, resume and reach the wall.
No injected actor, terrain or result is used in these application measurements.
This is scripted clone observation, not human play or an original comparison.

Before: `RuntimeSimState::advanceElapsed` -> `World::advanceElapsedMs` -> integer
`Lemming::x/y` -> `injectedWalkersFromWorld` -> `terrainSampleFromLemming`
(file Y = height - 1 - feet Y, unchanged Z = 0.75) ->
`TubeScene::prepareInjectedWalker` -> ActorPose/projection -> drawing and screen
boxes. Animation already advances independently, but the position adapter only
copies the latest integer cell. The actual projected **origin**, rather than a
sprite's changing artwork/bounds, is the position measurement.

After: each level leg owns `LemmingMotionTracker`. The frame loop observes the
World before advancing (including input-driven state changes), then after **each
existing World update** through the existing callback, alongside the unchanged
cue drain. It uses `World::levelClockMs()` directly, including the accepted stall
clamp, partial completion, integer-ms carry and fast-forward passes. It supplies
no elapsed time or data to simulation. Animation clocks remain unchanged.

`injectedWalkersFromWorld(..., anim, motion)` substitutes fractional terrain X/Y
before the same ActorPose/projection. Both `injectedWalkerScreenBoxes` and the
draw call use that sample. `hoveredActor` already uses these boxes and the same
sample for distance/ranking, so click input selects the displayed actor. The
countdown's existing boxes also follow the interpolated placement. The original
three-argument adapter remains available, including its link symbol, for frozen
source demonstrations and fixtures. `LevelActorView` appends a read-only sample
for evidence; the normal draw path does not depend on the observer.

## Explicit provisional policy

The tracker buffers confirmed cell endpoints by one bookkeeping interval:
`max(1, 1000 / ticksPerSecond)` ms, currently **200 ms of level time**. On each
frame it linearly samples the surrounding endpoints, with alpha clamped to
[0,1]. Sparse skill movements occupy at most their preceding interval; earlier
stationary time remains a hold. There is no velocity prediction or extrapolation.
X is unwrapped locally at the cylinder seam and wrapped after interpolation.
History older than the sample bracket is discarded.

New actors snap to their first known position. Facing and gravity changes,
teleport state/arrival, transitions to/from stationary states and implausibly
large jumps clear the buffered path and snap immediately. Compatible moving
states retain the path, including fall-to-walk landings and hoisting into walking.
Limits admit the existing seven-cell Bounce step. Sampling every update preserves
turns encountered between fast-forward passes. Pause freezes the World clock and
therefore the fractional position exactly.

This deliberately adds presentation latency and a short initial hold at spawn.
Animation selection still uses the current simulation state while the compatible
position path is buffered; reconstructing original within-frame pose/state
handover is not part of this fix. These are provisional player-facing choices,
labelled in the source, not recovered original interpolation or timing fidelity.

Retained evidence, not newly executed original code:
[data-and-simulation](../lessons/data-and-simulation.md),
[D1m](phase-d1-stepper-clock.md) and
[D1n](phase-d1-continuous-stepper-clock.md).
Registered reference: repository-local `Lemmings Revolution/Lemmings Revolution.exe`,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
D1m cross-checked instructions at `0x0041BDD4`: mutable remaining-time loop,
boundary slices and X += 5 * slice * facing. It withdrew the old inference of
inherently discrete original movement. Its retained 97% changing consecutive
20-ms actor samples support continuous motion, not this buffering policy.
No new static REA session was necessary. An authorized original per-frame trace
of actor coordinates and rendered origin through walking, landing, turning,
teleport, pause and fast-forward would settle the presentation comparison.
D1/D3, C2/C3 and all other full-project fidelity requirements remain unresolved.

## Measurements and retained images

Ordinary clock runs omit both `--frames` and `frameSeconds`, use normal speed
(one pass), and stop after six seconds of level time. SDL dummy video/audio
still executes the production frame composition and `SDL_RenderPresent` calls.
Rates are measured with `steady_clock` between observed/presented frames, not
inferred from a requested FPS. CPU rendering/presentation performance is outside
this task.

| Same installed walking route | Before | After |
| --- | ---: | ---: |
| Walking frame intervals | 63 | 63 |
| Walking wall seconds | 3.487 | 3.405 |
| Presented frames/s | 18.07 | 18.50 |
| Simulation cell changes/s | 4.88 | 4.99 |
| Actual projected-origin changes/s | 4.88 | 18.50 |
| Origin changes on a frame with no cell change | 0 | 46 |

Exact summaries/CSV and twelve consecutive full clone PNGs on each side:
`build/actor-motion-001/evidence/wall-{baseline,production}/`.
I reviewed the before image and after frames 0, 5 and 11; the actor advances
along the ledge between cell updates. The deterministic sequence independently
separates position changes from sprite animation.

The replay uses a declared 20-ms frame clock at normal pass count. Its identical
input script pauses at a fractional draw position, holds eight frames, aims at
the actual displayed sprite rectangle, asserts hover index 0, then clicks.
The click assigns Climber, spends one stock and resumes through normal input.
The final replay additionally asserts that the first natural fall-to-walk
landing advances by at most one cell per 20-ms frame, avoiding a transition pop.
Across 76 walking intervals (1.52 declared seconds), both sides have seven cell
changes. The baseline has seven actual drawn-origin changes; production has
**76/76**, including **69 between cell changes**. The first retained twelve-frame
sequence is in `evidence/{baseline,production}/walk-0.png` through `walk-11.png`.
I reviewed the production sequence at entries 0, 5 and 11, pause/end and landing.

The two complete **904-frame simulation traces are byte-identical**:
`evidence/{baseline,production}/simulation.txt`, SHA-256
`8e9bdb811d2ca6cebea7441b14c633de03745d4088f9044b6af5255bdc641af9`.
They compare every frame's positions, facing, state, frame counters, fall/brick/
bash/fuse/boost fields, skill abilities, gravity, sheet, teleport exclusions,
floating step budget, stock, terrain version, ticks, explicit update count,
spawned/saved/dead and finished/won outcome. Final state: 17,920 level ms,
89 ticks, 913 explicit updates, six released, zero saved/dead, still running.
Actor 0 has Climber and is climbing at cell (80,56). This bounded route is not a win.

Frames 202-209 freeze the level clock at 4040 ms and displayed feet at
**(38.5,63)** while the simulation feet are **(39,63)**. Hover index 0 and the
subsequent successful stock-spending public click prove selection follows the
displayed actor. Two natural fall-to-walk landings advance the displayed Y by
**0.5 cell**, including frame 125->126, rather than the earlier candidate's
4.5-cell reset. `evidence-index-final.json` records the exact samples, trace hash
and archive paths. Each side's summary reports zero failures and unchanged saves.

Images: [ordinary before](../../build/actor-motion-001/evidence/wall-baseline/walk-0.png),
[ordinary after 0](../../build/actor-motion-001/evidence/wall-production/walk-0.png),
[after 5](../../build/actor-motion-001/evidence/wall-production/walk-5.png),
[after 11](../../build/actor-motion-001/evidence/wall-production/walk-11.png).
Additional reviewed images:
[fractional paused pick](../../build/actor-motion-001/evidence/production/pause-end.png),
[landing](../../build/actor-motion-001/evidence/production/landing.png),
[climbing](../../build/actor-motion-001/evidence/production/climb.png).

## Tests, provenance and preservation

The new CTest cases register the source-built before/after demonstrations,
ordinary-clock measurements, baseline identity and units. Units exercise Walker,
Faller, Floater, Climber, Builder, Miner, Digger and Bounce, confirm movement
between cell updates, assert monotonic walk/climb/fall/float without overshoot,
freeze under RuntimeSimState pause, check wall turns, the 255->0 seam and nearby
and distant teleport arrivals. Every unit update compares all public lemming
fields/counters against a second unobserved World. Synthetic fixtures supplement
the installed route; they do not establish natural gameplay or original fidelity.

Registered UCRT64 GCC 16.1 / CMake / Ninja / CTest, `-O2 -g`, assertions on,
12 build jobs. Final scoped check: `build/actor-motion-001/tests-landing.log`,
with its build in `build-landing.log`: **15/15 passed in 146.41 seconds**.
It includes the motion cases plus
sharedtiming, runtimepick, clicktoassign, animation units, pause, climb, float and
both fast-forward toggle demonstrations. This is the final production/source
content identified by `worker-tested-sources.json`; only this report was edited
afterward. `git diff --check` passes. The pre-existing protected queue file still
causes Git's CRLF-normalization warning; it was not edited or normalized here.
No full profile was run or restarted by this worker.

Development history is retained: `tests-1.log` passed the two application routes
but exposed an over-strict Bounce jump limit and a teleport fixture initialized
one countdown update too early. The guard now reads the actual Bounce speed;
the fixture now reaches the existing relocation update. `build-2.log` caught an
older frozen header requiring the old three-argument adapter symbol; the
forwarding overload preserves it. `build-3.log` and subsequent checks pass.
No pre-existing assertion, timeout or test count was changed.

The first candidate reset every state transition. Its passing scoped checks
(`tests-2.log`, `tests-final.log`, `tests-fractional-pick.log`,
`tests-complete.log`) and images/traces remain historical under `evidence-final/`
and `evidence-fractional-pick/`. Reviewing that trace exposed a **4.5-cell landing
pop** (frame 125 draw Y 58.5 -> frame 126 Y 63). That policy was refined: compatible
moving states now preserve their confirmed path, with an installed-route landing
assertion. The earlier source and report are in `state-reset-policy/`; the final
checks below supersede its movement conclusion. No historical failure is erased.

Authentic task-entry source was copied **before edits** to
`build/actor-motion-001/before-source/`. `before-hashes.json`,
`baseline-source-check.json` (269 files match the supervisor entry manifest),
`pre-existing.patch`, `entry-status.txt` and `HEAD-main.cpp` retain provenance.
The baseline executable compiles that actual frame loop over shared support;
its null-motion adapter keeps the old integer behavior. It is not a diagnostic
flag recreating a pre-change view. Its main SHA-256 is checked by CTest:
`fe1454539c44483b5c8b9f0fc8aa4423c6e1f47e63697a27b17af363f20bf30f`.
CSV origins on both sides are actual production projection results; the frozen
loop's omitted native sample fields are reconstructed only for the integer X/Y
columns. `task-only.patch` separates this task from accepted incoming edits.
`worker-tested-sources.json` identifies final source content.
`preservation-check.json` confirms all 263 non-task files in the source backup
remain byte-identical. Earlier source copies and evidence were never refreshed
from changed files or relabelled as authentic pre-task material.
On clean checkouts without the optional frozen source, CMake explicitly requests
the production-only motion/picking checks. When the frozen source is present, as
in this run, the paired simulation comparison is mandatory and its baseline runs
first. The summary records whether that comparison ran.

Every application invocation supplies a private `FileSaveSink` under its test
output directory and leaves default saves disabled. Installed and normal clone
save existence and full bytes are checked unchanged. No installation, queue,
controller, verifier, registry, permissions, shared instructions, model pins or
completion criteria were edited. The existing supervisor lease owns this worker;
no manual protected control edits were performed. Pre-existing changes were
preserved. No commit, push, reset, clean or stash. Task text files use LF.

Changed task files: `src/app/{LemmingMotion.h,LemmingMotion.cpp,TubeScene.h,
TubeScene.cpp,AppEntry.h,main.cpp}`; `src/__test__/test_actor_motion.cpp`;
`cmake/{webolution_sources.cmake,actor_motion_identity.cmake}`;
`CMakeLists.txt`; this report.

Limitations: bounded first-level route and source fixtures, not all-level human
play. The deterministic route remains in progress at its stop; it is not a win.
No claim of improved renderer speed, exact original movement, animation/physics
synchronization or resolved fidelity gates. A short optimized human P7 playtest
is still needed after supervisor verification/review.

Handoff: P7 presentation interpolation is implemented with before/after origins, clone images, identical simulation traces and shared picking; status needs_review.
Next action: supervisor full registered main/recovery verification, independent acceptance review, then optimized human movement/pause/turn/teleport playtesting.
