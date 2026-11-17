# basher-occlusion-001

Status: **needs_review**. Playable v1, presentation/occlusion step, human
[playtest 9 Y9–Y11](../playtest/2026-11-16-playtest-9.md), reopening W1.
Difficulty difficult; recommended Opus; assigned Codex override used.
**Post-worker supervisor verification and independent review are pending.**
Worker revision checks and the earlier failed smoke run are recorded below.
Human playtest acceptance is still required; these are clone observations.

The next dependency was the demonstrated wall/bridge leakage, after accepted
actor-occlusion-001, actor-depth-001, climber-occlusion-001 and
pad-pipe-presentation-001. Their reports and accepted queue reviews were read.
Their historical verification logs remain historical; none verifies this change.

Production path and cause:
`runApplicationLeg -> renderFrame -> TubeScene::syncTerrain -> scheduleSceneFrame`
(primary/legacy scenery, tube, pipes, fluid, secondary scenery)
`-> drawInjectedWalkers -> prepareInjectedWalker -> actorPoseFromCylinderSample
-> actorSpritePlacement -> drawActorSprite -> clipActorToScene -> composition`.
Drawing and highlight share preparation and the displayed scene depth. Picking
shares pose/frame/placement, but skips pixel occlusion (revision below).

The actor uses the **constant cylinder template**, coordinate height 112, not
the deformed terrain surface pool. Installed inner radius is 50, shell thickness
10; layer 0.25/0.75 therefore means nominal radius 52.5/57.5, with polygonal
interpolation between template vertices. `LemmingMotionTracker` moves walking
lanes between 0.25 and 0.75; excavation states retain the lane. This is why the
test must include the inner lane, rather than assuming every actor is at 0.75.

Terrain and built bricks reach the same `syncTerrain` collision-to-cell-mesh
rebuild. The cell routes project the actual stored/generated surface vertices;
`Raster::triangle` writes screen-affine physical view Z (minus the route context),
with smaller Z nearer. Side ribbons span the shell thickness. Scenery/pipes use
`SceneDepth`/`SceneRaster` in those comparison units. Pipe depths are retained
separately before fluid can replace them.

Previously the **entire sprite** compared at
`placement.depth * width / 0.3 - 4`, i.e. origin view Z minus **4.64 world units**
at 640 pixels. Climbing-wall and pipe exceptions removed only the extra four.
The same foreground pull leaked walkers at walls and feet through bridge faces.
Constant origin Z also gave head and feet the same depth under a pitched camera.

The provisional replacement is an **upright plane through the actual actor
position**, facing the camera horizontally. Each opaque texel's scene pixel-centre
ray intersects that plane. It gets no foreground pull and no state/facing-specific
allowance. Native screen projection, SPR sampling, lane and simulation stay as
they were. `actorPhysicalDepth` undoes the projection bias for origin-plane
effects/countdowns; the traced 0.0003 remains in sprite placement. Drowning figures
use the same upright plane against their existing opaque-scene snapshot and pipe
depth; their additive rendering/water exception is retained. Actors never write Z.

Retained static evidence, copied unchanged to
`build/basher-occlusion-001/static/`: Ghidra 12.1.4 pseudocode and instructions
for registered `Lemmings Revolution/Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Cross-check: `43CC94/43CC9A` origin sign gate; `43CCB5` subtracts `[4B1498]`
(0.0003); `43CCCA` depth-slot setup; `43CD4E` sprite submission. Instruction
evidence ID `ev_9a8209aba409c9745a27529fbccfc174c42f9a83172b48c6c3544c73701ba1e3`.
This does **not** establish original hardware depth policy. No new REA session
was necessary or opened; no original execution/comparison occurred.

Inspection uses passive render observations of existing human recordings, not
new scripted gameplay routes. All outputs are under `build/basher-occlusion-001/`.
`inspection-summary.json` indexes **480 views**, including labelled supplemental
poses; CSVs retain clock, position, layer, temporal frame, yaw and visible pixels.
Each sheet has six columns: angles 0, −45, +45, −67.5, +67.5, −78.75 degrees,
then +78.75, −90, +90, −101.25, +101.25, 180; each pair shows masked then raw.
Angles are relative to the actor column in canonical CameraParams, not an
original-game camera measurement. No frozen-source build or replay fixture added.

| Installed route / snapshot | Side-view result (visible/raw pixels at −90°, +90°) |
| --- | --- |
| Just Bash `MM_BASH_002`, 29,222 ms, inner-lane basher five columns into cut | 0/132, 100/154: wall covers actor; exposed tunnel fragments survive. At +78.75°, leakage decreases 130 to 99 pixels. `upright-bash/`; additional layer-0.75 pose is explicitly supplemental. |
| Loggerheads `MM_E1_3_SKILLS`, 19,642 ms wall turn | 122/122 exposed, 0/115 covered. At +45°, 112 visible pixels become zero. `upright-wall/`; source recording is the existing unpromoted `build/playtest/replays/MM_E1_3_SKILLS-1794852964640681600-1.replay` (10/5 win, unchanged). |
| Just Build `MM_BUILD`, builder 25,625 ms / bridge walker 25,663 ms | Builder 0/131, 156/164; walker 0/127, 80/114. Exposed bodies remain; brick faces cover the feet. At +78.75°, walker 129 becomes 117 pixels. `upright-bridge/`. |
| Just Climb, grounded walker 8,018 ms | 0/127 behind structure, 114/114 exposed; front view 107/107. `final-climb/ground-walk-sheet.png`. |
| Just Climb, climber 12,731 ms / faller 5,822 ms | Climber 107/107 exposed, 0/102 across wall; faller 0/127, 125/125. `final-climb/`. |
| Just Climb, exit jump 80,346 ms | 108/108, 0/108; basket/foreground structure occlude correctly. `final-climb/exiting-sheet.png`. |
| Just Float `MM_FLOAT`, floater 22,712 ms | 0/246, 9/255 exposed edge fragments; other sampled rotations preserve the exposed figure. `final-float/float-sheet.png`. |
| Just Block `MM_BLOCK_001_A`, blocker 12,337 ms | 50/161 partial cover, 141/141 exposed. `final-block/block-sheet.png`. |
| Just Mine `MM_MINE`, excavation 12,204 ms | 98/177 through opening, 0/157 behind wall. `final-mine/mine-deep-sheet.png`. |
| Just Dig `MM_DIG_000`, excavation 14,934 ms | 92/92 exposed, 25/98 edge fragments, then fully covered at +101.25°. `final-dig/dig-deep-sheet.png`. |

Fluid/pipe pass: `final-fluid-pipe/` uses the MM_W_012 winning recording.
At 49,088 ms the water lemming meets the water surface; the inspected −90°/+90°
views retain 113/133 and 104/121 pixels, clipping feet below the surface without
burying the body. The sampled natural actors had **no pipe-pixel overlap**.
Supplemental figures at (column 156, row 8, layers 0.25/0.75) exercise the actual
installed supply pipe: at front view 83 of 104 overlapping pixels are covered,
with the exposed top surviving; inner-lane +45° covers 87 of 116 pipe-overlap
pixels. These are synthetic poses, not natural actor/pipe gameplay evidence.
All 480 paused repeats are pixel-identical: no repeated-frame depth flicker.
The same cause is fixed across the listed states/surfaces; no further confirmed
shared depth leak was found in these snapshots.

Limits/remaining checks: Just Bash at (150,40), including front/side openings,
has a four-row-high cut and a taller current sprite; the ceiling still covers
the upper sprite. No sprite-scale or tunnel-size change was made to expose it
through solid terrain. Rapid animated rotation, every temporal frame, other
zooms/elevations and natural pipe overlaps remain human-playtest coverage gaps.
The upright plane, existing affine terrain Z and original effective device state
are not a full geometry/fidelity solution. No unrelated park is reopened.

Checks: registered UCRT64 GCC/CMake/Ninja, -O2/-g, eight build jobs;
`final-build.log` succeeds and retains **95 smoke registrations**. Six focused
checks pass in `focused-tests.log` (composition, interaction, overlap, visibility,
picking, numerical plane). The expanded final legacy check passes in **0.11 s**,
`plane-mask-ctest.log`: **3,456** independently projected points, foreground and
contact planes 0.01 units from an installed sprite, retained pipe coverage,
then-matching picking, and unchanged scene depth. These are first-attempt checks;
the picking contract and smoke result are corrected in the revision below.

All **15 active promoted recordings** ran through the changed
`webolution.exe --game-dir "Lemmings Revolution" --replay <file>`; every one
returned **exit 0, PASS, won=1, completed=1**. Exact commands, filenames and
individual logs: `replays/results.json` and adjacent logs.

| Level | Saved/required | Result |
| --- | ---: | --- |
| M06_KF_1 | 93/50 | PASS |
| MM_BASH_002 | 10/10 | PASS |
| MM_BASH_MINE_WLR1 | 20/10 | PASS |
| MM_BLOCK_001_A | 24/15 | PASS |
| MM_BLOCK_BOMB_A | 12/10 | PASS |
| MM_BOMB | 17/10 | PASS |
| MM_BUILD | 20/10 | PASS |
| MM_CLIMB_002 | 10/1 | PASS |
| MM_DIG_000 | 19/10 | PASS |
| MM_FLOAT | 10/1 | PASS |
| MM_MINE | 20/10 | PASS |
| MM_W_012 | 16/10 | PASS |
| MM_BASH_DIG_001_A | 20/10 | PASS |
| MM_BUILD_BLOCK_A | 15/10 | PASS |
| MM_FLOAT_CLIMB2 | 10/6 | PASS |

Legacy removals under Verification tiers (registrations, targets and sources):
`actor_occlusion_after` asserted a flat biased comparison;
`actor_depth_after` explicitly required the four-unit contact allowance;
`climber_occlusion_after` required the facing-dependent allowance. Those
expectations encode the demonstrated defect; none was repaired or weakened.
No smoke assertion, registration, timeout, verifier or controller changed.

Revision 002, review feedback (2026-11-16): the lead's
`build/lead/smoke-verify-5.log` passed 94/95 main smoke tests and failed
`clicktoassign`, "each alive lemming has a screen box". At tick 25 on
`MM_CLIMB_002`, the walker at (44,63) and faller at (32,57) are alive, but
`clipActorToScene` could mask every opaque texel and make
`prepareInjectedWalker` return nullopt. Both screen-box overloads used that
result, coupling drawing occlusion to hover/click eligibility. The stricter
upright plane exposed this existing coupling; the earlier focused checks did
not establish a smoke pass.

`injectedWalkerScreenBoxes -> prepareInjectedWalker(PlacementOnly)` now keeps
the submitted actor's unchanged projected rectangle without rasterizing or
depth-testing its body. Submission, valid frame/texture and projection checks
remain. `drawInjectedWalkers` and `drawHoveredWalker` still use `ClippedSprite`:
their plane, terrain/pipe/fluid comparisons, rejection and pixels are unchanged.
Thus a fully covered actor can be hovered/clicked through its rectangle, while
neither its body nor its additive highlight shows through the covering surface.
This is the review-requested clone picking policy, not original fidelity.
Countdowns still test their own glyph pixels against scene depth. No simulation
or skill rule changed.

The test added by this task, `test_actor_occlusion_plane`, now requires a box
on both sides of the supplemental contact/pipe planes and additionally requires
the hover highlight to remain hidden behind the foreground plane. Its body and
depth assertions are retained. This supersedes that task-added test's earlier
hidden-box expectation, which encoded the reviewed picking defect; the protected
`test_clicktoassign.cpp`, all smoke assertions, counts and timeouts are unchanged.
Both focused tests pass in **0.21 s**; exact output and checks are in
`revision-002/focused-results.xml` (paths here are relative to
`build/basher-occlusion-001/`). The first attempt's 480 visual observations and
replay logs above are retained, not rerun or relabelled as revision observations.

Revision verification: the requested single worker run of
`python tools/agent_control/verify.py --profile smoke` exits **0**:
**95/95 main smoke** (126.84 s), **84/84 recovery** (21.95 s), and
`git diff --check` pass. Exact retained log:
`revision-002/smoke-verification.log`; copied JUnit outputs:
`revision-002/main-smoke-results.xml` and `revision-002/recovery-results.xml`.
The unchanged click-to-assign test now reports both boxes, walker
`[385,292,397,310]` and faller `[313,270,325,288]`; the native angular rejection
and hidden-click test `levelvisibility` also passes unchanged. The verifier's
incoming `agent/tasks.json` CRLF warning was left alone; controls are protected.

All **15 active promoted recordings were rerun after the revision** and again
return exit **0, PASS, won=1, completed=1**, with the same saved/required results
in the table above. Per-file commands, exit codes and individual outputs are in
`revision-002/replays/results.json` and adjacent logs, generated sequentially by
`revision-002/run_replays.py`. The two Y6-retired recordings remain retired;
no replay input was edited. No profile was repeated or full legacy suite run.

Changed: new `src/app/ActorOcclusion.h`; `src/app/{ActorSprite.h,TubeScene.cpp}`;
comments only in `src/app/{TubeScene.h,main.cpp}`; new
`src/__test__/test_actor_occlusion_plane.cpp`; `CMakeLists.txt` and
`cmake/climber_occlusion.cmake`; the three retired legacy sources; this report.
Backups/incoming patch are in `entry/`; `task-only.patch` separates edits to
existing files from incoming work. Task text is LF; scoped diff checks pass.
The first attempt's parent supervisor lease was PID 1620. CIM enumeration was denied;
Get-Process showed no clone/build/test runner before edits. No protected manual
edits, queue edits, concurrent writer, subagent, background job, original process,
commit or push. Clone commands use private `build/basher-occlusion-001/appdata`;
replay/inspection never construct a SaveStore or touch installed/normal saves.
Revision backups are in `revision-002/entry/`; the active parent supervisor lease
is PID **46016**. Get-Process again showed no live clone/compiler/test runner at
entry. This revision changes only `TubeScene.cpp`, header comments, this task's
new plane test and this report; all incoming edits are preserved. Commands use
private `revision-002/appdata`; no protected controls or shared instructions changed.

Handoff: Y9–Y11's upright depth policy is retained; screen boxes survive body occlusion, drawing/highlight stay masked; 95 smoke + 84 recovery and 15 replays pass; needs_review.
Next: post-worker supervisor smoke and independent review, then human rotation/contact/picking playtest; original depth fidelity remains unresolved.

What to check: **Just Bash** — bash into the tall wall, rotate across both sides,
including an inner-lane basher: foreground wall/ceiling covers it and the cut
opening exposes it. **Lems at Loggerheads, node 17** — rotate while walkers turn
at the wall: none shows through it. **Just Build** — view a bridge side-on while
building and walking across it: brick faces cover feet below their top, with
exposed bodies visible. Recheck **Just Mine/Just Dig**, **Just Climb/Just Float**
and **MM_W_012** during slow rotation and fast-forward for new flicker or excessive
clipping; pipes and water should cover only the parts between actor and camera.
**Just Climb** — hover/click a live lemming while rotating it behind a wall:
its screen box remains available, but its hidden body and highlight stay covered.
