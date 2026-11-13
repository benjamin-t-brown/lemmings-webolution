# actor-anim-001: per-state lemming animation and bomber countdown

Playable v1, roadmap step 4, playtest P4-P6. **Implemented; needs independent review.**
The worker's registered main-profile check passed **182/182**; final main/recovery
verification belongs to the supervisor. No original execution, copied-code oracle, hooks, debugger,
capture, subagents or background jobs were used. This is scripted clone evidence, not a
human playability confirmation or an original-fidelity pass.

## Resume and evidence history

The human failure is recorded in [playtest 1](../playtest/2026-11-12-playtest-1.md): every
state appeared as one walking frame and fused lemmings had no countdown. Terrain rendering
is already accepted; visible state/animation is the next dependency for the player to read
what skills are doing. P7 movement interpolation, P8 occlusion and hatch/exit effects remain
separate. Difficulty: difficult; assigned recommendation Sonnet5.5/high, with this run using
the explicit Codex provider override. No model pins changed.

This resumes the substantial earlier implementation. The original report and all incoming
task edits were backed up under `build/actor-anim-001/resume-backup/`. Authentic HEAD copies
of main.cpp and TubeScene.cpp/.h are in its `HEAD/` subtree. They are pre-task source, not
modified files relabelled as a baseline. The earlier report's compilation-only result stays
historical. The supervisor actually found a link failure in
`build/agent-runs/20261113T152739-41368-85eb59ad/verification.log`: TubeScene referenced
`LemmingAnimTracker::poseOf`, but the two manually listed scene test targets omitted its
translation unit. The incoming CMake edits already added ActorAnim/LemmingAnim to
`test_actorcompose` and `test_clicktoassign`; this worker confirmed both link successfully.
The cancelled provider-switch attempt is not a task failure.

New test development history is retained in `build/actor-anim-001/development-1.log`,
`development-2.log`, `development-3.log`, `development-4-float.log` and `development-5.log`.
These are not all passing runs. The first run exposed an invalid growing-vector sentinel
in the new walker fixture, a premature three-frame bomber stop against a four-frame
assertion, the real font's 17-pixel digit `1` against an arbitrary 20-pixel threshold, and
camera/picker assumptions in the new public-input driver. Corrections use an optional
walker index, wait through the actual blast, retain the measured glyph support (17 pixels
for `1`, at least 20 for the others), pause/aim/click through public input, and compare
simulation state/position at equal elapsed time after assignment. No pre-existing test
assertion, timeout or simulation rule was changed. The all-eight development run passed;
its final small evidence changes park the cursor away from actors and make PNG write errors
fail the test. The main-profile run checks those final changes.

## Production path: before and after

Before, the frame loop in `src/app/main.cpp` advanced RuntimeSimState and, on an update,
called `injectedWalkersFromWorld(world, terrainHeight)`. That adapter ignored every
`sim::Lemming::state` and left `walkTemporalIndex` at its default zero. Both drawing and
screen-box calculation used `prepareInjectedWalker` in TubeScene.cpp, which unconditionally
found `walk` and selected `animations[0].frames[direction]`. Thus even a climbing, floating,
building or exploding lemming drew walk temporal entry zero. ActorAnim existed but was not
driven by the production application.

After, each level leg owns a `LemmingAnimTracker`. Following the existing simulation update,
main passes its accepted `lastPassMs()/1000` budget (times the existing fast-forward pass
count) to `LemmingAnimTracker::advance`; it passes zero when no simulation update runs,
including pause. The tracker reads the final simulation state, selects its strip, resets
on a state change, and calls the unchanged recovered `advanceActorAnimClock`. It neither
writes to the World nor changes actor positions, release/skill rules, simulation clocks
or the fast-forward latch. Reusing lastPassMs preserves the accepted stall cap and the
owned-clock exception; it corrects the draft's additional unconditional 100 ms animation cap.

`injectedWalkersFromWorld(..., &tracker)` copies each pose's clip and temporal index into
the scene input. `prepareInjectedWalker` now finds that clip, selects its temporal entry,
and applies the existing ActorPose, directional selection, mirror, texture, light and
placement path. The same preparation supplies drawing and screen boxes. Pick boxes now
follow the selected frame's actual rectangle; the picker and assignment rules are unchanged.
Missing named clips fall back to walk; a missing whole sheet retains the prior no-sheet
behavior. The default null tracker preserves static/frozen walker fixtures.

After the runtime HUD, `drawBomberCountdowns` reads each live actor's existing fuseMs and
screen box, draws ceil(fuseMs/1000) above its top using the existing ^FONT glyph table, and
removes the digit when the fuse reaches zero. The countdown does not drive the fuse or
explosion. `--no-actor-anim` is a diagnostic: old walk[0] drawing and no countdown. It is a
source-backed reconstruction of the old view, **not an executable built from frozen HEAD**.

## Mapping, rates and mirroring

Reference identity for the retained static research:
`Lemmings Revolution/Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
No new REA session was needed; accepted retained research was sufficient.

- [A3 selection/SPR axes](phase-a3-sprite-view.md), [SPR read](phase-c1p-spr-sheet-read.md)
  and [animation clock](phase-a6-actor-anim-clock.md): temporal entries and directional
  images are distinct. `0x468833` reads strip +0x10 at `0x46887F/0x468885`, accumulates
  dt*rate, and advances while the accumulator is strictly greater than 0.06 (`0x4B1940`).
  It carries the remainder. `0x468AB8` resets on binding a new strip. This uses the existing
  recovered clock rather than reproducing its arithmetic in the new tracker.
- The actual installed HI asset table is measured by the unit test in `clips.txt`. All
  ordinary rates are 1 (nominal 16.67 entries/s); shrug is 0.5 (8.33/s), cartwheel 2 (33.33/s).
  `Sprite.cpp` projects serialized speed_bits_10 into Sprite::rate. The draft's uncertainty
  about shrug's rate is settled by this read-only asset measurement.
- Mirroring remains `actorPoseFromNativeSample(facing)` -> spriteViewScores ->
  `selectSpriteView` -> actorSpritePlacement/drawActorSprite. A3's 608 branch cases and
  42 actor-bound index/mirror cases are retained evidence, not new original observations.
  Five-direction strips choose a direction/mirror from the scores; eight-direction strips
  preserve the prior mirror argument (false in this adapter); the single-direction shrug
  uses direction zero. No chooser or pose arithmetic changed.

| Simulation state | HI strip / temporal count | Evidence or individual provisional choice |
| --- | --- | --- |
| Walker | walk / 12 | retained actor_walking.md, entry 0x41BDBA selects walk |
| Climber | climb / 18 | actor_climbing.md, entry 0x4137A3 selects climb |
| Hoister | flip / 14 | actor_topping.md, entry 0x417CCC selects flip |
| Faller | fall / 15 | actor_falling.md: entry selects fall |
| Floater | umbopen / 8, then float / 15 | actor_falling.md: opening then completion selects float; presentation handover is provisional below |
| Blocker | block / 16 | **PROVISIONAL:** the blocker-named strip, not a recovered Blocker entry call |
| Builder | build / 16 | retained data-and-simulation/World skill research, strip 2 |
| Shrugger | shrug / 8, rate 0.5 | same retained research, strip 10; actual asset rate measured |
| Basher | bash / 24 | same retained research, strip 0 |
| Miner | mine / 25 | same retained research, strip 9 |
| Digger | dig / 20 | same retained research, strip 4 |
| Exploder (pre-blast) | explode / 17 | actor_flapper.md, entry 0x41695C selects explode for non-rat actors |
| Drown | drown / 17 | actor_drowning.md, entry 0x40874C selects drown (string 0x4BF130) |
| Bounce | fall / 15 | actor_bouncing.md, entry 0x40ECCA selects fall |
| Teleporting | cartwheel / 16, rate 2 | retained World.h pad state/type 0x28 evidence; pairing rules unchanged |
| Splatter | fall / 15, hold at end | **PROVISIONAL:** nearest evidenced falling strip; no splat strip in this sheet |
| Exited / Dead | not submitted | existing simulation removes these immediately; the measured sheet has no exit strip |

The actor_* references above live under `decomp-lemmings/docs/evidence/`; the general skill
research is in [data-and-simulation](../lessons/data-and-simulation.md) and World.h/Rules.
These are retained static/source findings, not new execution of those recovered routines.

Individually provisional presentation policies: cyclic states loop; flip, shrug, explode,
drown and umbopen hold at the end (Splatter additionally holds fall). A frame-end state
change receives that frame's whole animation budget; a completed umbopen switches to float
at frame end and drops the excess opening budget. These replace original within-frame model
callbacks only in presentation. The countdown's yellow colour, HUD font, placement above the
sprite, ceil rounding and overlay order are provisional. It can draw over scenery because
actor occlusion is a later task. A Blocker entry/model trace, original state-transition/loop
observations, a splat sequence and a fused-lemming observation would settle these choices;
none is authorized or required in this worker.

## Clone evidence and checks

The CTest driver calls `runApplication --level ... --sim --native` on installed level data,
with a declared 200 ms clock. It queues skill keys, P to pause, mouse motion, right-button
camera grabs and a left click confirmed by the production hover. A successful assignment
resumes through production input. It runs both animation and diagnostic legacy modes;
state and position are checked at matched elapsed time since the assignment. Standalone
level runs install no save store (`useDefaultSaveStore` stays false), so no progress is
imported, reset or saved. The world is never edited by the observer.

Eight registered tests: real-asset unit selection/frame advance, public-input pause/resume,
and six public skill routes. They check stock consumption, actual state/strip, at least two
temporal entries, matching simulation positions, and visible pixel differences. Floater
also measures the Faller strip; Bomber reads/counts all countdown pixels, requires decreasing
digits, samples explode, then continues until the actual blast and actor removal.

Final evidence snapshot: [`build/actor-anim-001/worker-20261113T154733/`](../../build/actor-anim-001/worker-20261113T154733/).
All image and CSV names below refer to this snapshot, not the root output directory which a
later supervisor run will regenerate. `manifest.json` pins its files; `worker-tested-sources.json`
pins all ten build/test source files and authentic baseline HEAD
`d55cf9c167ddde36006c3785181157b04a16d68b`. No production/test source changed after that fingerprint.

Image/measurement index:

| Route | Files and interpretation |
| --- | --- |
| MM_CLIMB_002 | climb-before-after.png and walker-before-after.png; 12 sampled frames per row |
| MM_FLOAT | float-before-after.png (umbrella opens, then float); faller-before-after.png (first fall and subsequent landing); float-frames.csv records all fall samples |
| MM_BOMB | bomb-countdown.png (5,4,3,2,1); bomb-before-after.png (pre-blast explode); bomb-blast-frame.png (after actual blast) |
| MM_BLOCK_001_A | block-before-after.png |
| MM_BUILD | build-before-after.png |
| MM_BASH_002 | bash-before-after.png |
| Pause on MM_CLIMB_002 | pause-running.png, pause-held.png, pause-held-later.png; measured temporal indices held across 20 paused frames and moved after resume |

Contact sheets have animated crops on the upper row and diagnostic walk[0] crops below,
scaled 2x. Columns are consecutive application samples beginning with the named state;
a short fall or explosion can end before the 12th column. Corresponding *-frames.csv names
each sample's state, position, strip, temporal index and countdown. *-animated-frame.png and
*-legacy-frame.png retain full application frames. These are source-built framebuffer
outputs, not original capture. Cursor position is moved away after the click for readability.

Worker ran `python tools/agent_control/verify.py --profile main` exactly once in this
invocation after settling the build: **182/182 passed, zero failures/skips**, CTest wall
time 529.30 s. Exact retained log:
[worker-main-verification.log](../../build/actor-anim-001/worker-20261113T154733/worker-main-verification.log).
The snapshot also retains `main-ctest.log`, `main-agent-results.xml` and `worker-build.log`.
The reference SHA was verified by byte reading only. The pre-existing CRLF warning for
controller-owned agent/tasks.json remains; this worker did not edit that file.
Existing frozen rendering, actor composition/view/mirror, picking, terrain rendering,
shared timing, fast-forward, frontend and gameplay source regressions all passed; no
pre-existing assertion or expected render identity was revised.

Final per-target measurements from the snapshot's CSVs and CTest log:

| Route/state | Animated observations | Diagnostic legacy view |
| --- | --- | --- |
| MM_CLIMB_002 Walker | 32 samples, all 12 walk entries | walk[0] throughout |
| MM_CLIMB_002 Climber | 12 samples, 12 distinct climb entries | walk[0] throughout |
| MM_FLOAT Faller | 8 target samples across falls, entries 3/6/10/13 | walk[0] throughout |
| MM_FLOAT Floater | 12 samples: 2 umbopen entries, 9 distinct float entries | walk[0] throughout |
| MM_BOMB Exploder | 6 samples, explode entries 3/6/10/13/16, then actual blast/removal | walk[0] until removal |
| MM_BLOCK_001_A Blocker | 12 samples, 10 distinct block entries | walk[0] throughout |
| MM_BUILD Builder | 12 samples, 10 distinct build entries | walk[0] throughout |
| MM_BASH_002 Basher | 12 samples, 10 distinct bash entries | walk[0] throughout |

Every compared wanted-state sample differed from the diagnostic walking picture by at
least 30 pixels (12/12 on the five long states, 6/6 on explode). Both runs' target simulation
states and positions matched at equal elapsed time after assignment. Non-walk actor count
in all legacy routes was zero. The unit read of the real asset covers the remaining mappings
and visits all 12 walk entries over two seconds. Pause held the complete observed walker
index map for 20/20 paused frames; 12 walk entries were seen before and after pause.

Worker image inspection: reviewed all eight before/after sheets (six skill routes plus
walker and faller), the countdown row, post-blast frame and three pause frames. Walking
legs and climbing poses vary above the static walking row. The umbrella visibly opens
and then hangs overhead; the first two fall-sheet columns show the falling pose, followed
by landing/walking (the CSV records the other fall segments). The Blocker's body sway is
subtle; Builder's red cap/tools and Basher's arm cycle clearly replace the walking pose.
The explode sheet shows the pre-blast collapse for six columns; the remaining columns
are blank because the actor is removed. The full post-blast image shows the crater.
The countdown row clearly reads **5,4,3,2,1**: 31/29/30/31/17 pixels of the digit colour,
respectively. The two held pause PNGs are byte-identical. These inspections establish
bounded visible clone behavior; independent acceptance and human playtest are still pending.

`saves-proof.json` retains before/after hashes: installed SavedGame.dat and normal
LocalAppData/LemmingsWebolution/SavedGame.dat both remained unchanged. No private save
was necessary for these standalone, no-store routes. No original asset or binary was
copied to version control.
The supervisor must still run its full registered main/recovery verification after worker
exit and make a separate acceptance decision. This report does not mark queue acceptance.

## Changed files and limits

Continued/new task files: `src/app/LemmingAnim.h`, `LemmingAnim.cpp`, `LemmingCountdown.cpp`,
`src/__test__/test_actoranimstate.cpp`, and this report. Integration: `src/app/main.cpp`,
`TubeScene.cpp/.h`, `AppEntry.h`, `CMakeLists.txt`, `cmake/webolution_sources.cmake`.
The report lists the incoming edits as part of the continued task; it does not claim this
worker originated all of them. Queue, supervisor, verifier, tool registry, permissions and
shared instructions were not edited. Pre-existing edits, including the installation log,
were preserved. Baseline source and incoming task edits remain under ignored build/.

Limits: positions retain the existing stepped presentation; actor occlusion is unchanged.
No new exit/death lifecycle or hatch effects were invented; Exited/Dead are still omitted.
Only normal LEMMINGSHI is used by the existing scene path: water/acid skins, inverted actor
orientation and speed-pad-specific animation scaling are not established by these routes.
Miner, Digger, Hoister, Shrugger, Drown and Teleporting have real-asset mapping coverage here,
not new public-input route/image claims. Human playtest P4-P6 is still needed. Original
loop/transition timing and full-frame fidelity remain unproved; historical gates/parks
and completion.json are unchanged.

## Handoff

1. Supervisor: run full main/recovery verification on the final content, then independent acceptance review against this report and the retained worker snapshot; leave needs_review until accepted.
2. Human: playtest P4-P6 on an optimized build, especially umbrella opening, blockers, countdown visibility and the provisional dying presentation; P7/P8 remain later tasks.
