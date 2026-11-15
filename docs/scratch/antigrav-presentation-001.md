# antigrav-presentation-001: inverted lemming presentation

Playable v1, playtest 3 R8. **Implementation complete; needs_review.** Retry 002
fixes two frozen-baseline layout crashes; **12/12 focused checks pass**. The prior
supervisor verification failed, and full registered main/recovery verification on
this final content is **pending supervisor execution**. These are source-built
clone observations, source tests, retained historical research and static EXE reads;
there was no original-game execution, debugger, hook, capture or copied-byte oracle.
No timing, release, skill rule, save codec, queue, verifier, controller or shared
instruction was changed. Earlier uncommitted work was preserved.

## Reproduced level and route

The earliest installed anti-gravity candidate is map **node 11**, `m06_kf_1`
(resolved `COMPILED\M06_KF_1.LVL`), **Sometimes up..sometimes down.** This is a
reproduction of the unnamed human defect, not identification of the user's level.
The next dependency was carrying the simulation's existing gravity into the shared
actor draw/pick preparation: the simulation already reaches a ceiling while the
sprite remains upright.

The [sweep census](level-sweep-001.md) identifies 18 candidate nodes:
11, 30, 33, 34, 46, 49, 55, 56, 59, 61, 63, 65, 67, 73, 80, 83, 94, 96.
The task driver reloaded their real terrain, map IDs, titles and current progress.
Every candidate had map state 1, hence none was reachable from the current clone
save at this run. Exact rows are in
[`before/candidates.txt`](../../build/antigrav-presentation-001/before/candidates.txt).
The read used the accepted save codec and did not change progress. The reproduction
therefore uses the explicitly labelled **direct installed-level route**, not a
frontend unlock claim. No progress was fabricated or imported.

On node 11 the six flipper cells in Y-down terrain are `(24,6)`, `(25,6)`,
`(24,7)`, `(25,7)`, `(68,88)`, `(69,88)`; hatches are `(43,81)` and `(70,7)`.
Lemming 0 releases naturally from the first hatch, walks right on row 87, flips at
the `(68,88)` support cell at 7400 ms, rises and first becomes a ceiling Walker
at `(68,77)` at 7900 ms. At 9000 ms it is `(73,77)`, gravity -1, with solid
support at `(73,76)` and **no flipper cell at its current support**. Gravity is
persistent after a pad, not membership in a spatial zone to resample while drawing.

The route runs production `runApplication --level m06_kf_1 --sim --native
--frames 220 --no-audio`, with the existing owned test clock at 0.1 seconds/frame.
Public SDL key 4 selects the real Blocker stock (30), P pauses at 9000 ms, the
pointer moves over the displayed actor, and P resumes after four paused frames.
No skill is assigned. The 220 frames cover 21.6 level seconds. Each direct run
has a private `FileSaveSink` under its output directory; no direct route writes it.
Installed and normal clone saves are byte/presence checked before and after.

## Before/after images and measured placement

All images below are clone framebuffer PNGs from the production application.
`*-detail.png` is a 6x nearest-neighbour enlargement of an actual framebuffer
crop, with the unchanged full frame retained beside it.

| Evidence at the same 9000 ms state, frame 89 | Before | After |
| --- | --- | --- |
| Full frame | [ceiling-steady.png](../../build/antigrav-presentation-001/before/m06_kf_1/ceiling-steady.png) | [ceiling-steady.png](../../build/antigrav-presentation-001/after/m06_kf_1/ceiling-steady.png) |
| Contact detail | [upright clipped fragment](../../build/antigrav-presentation-001/before/m06_kf_1/ceiling-steady-detail.png) | [inverted ceiling contact](../../build/antigrav-presentation-001/after/m06_kf_1/ceiling-steady-detail.png) |
| Drawn terrain sample X, native Y, lane | 72.5, 18, 0.75 | 72.5, 18, 0.75 |
| Projected feet X,Y | 512.3883305, 342.3363265 | identical |
| Sprite top,bottom | 323.3363342, 342.3363342 | 342.3363037, 361.3363037 |
| Clip / temporal entry | walk / 8 | walk / 8 |

Before: the upright sprite points into the ceiling and scene depth leaves a
thin, apparently unsupported fragment. After: feet are at the underside of that
ceiling and the blue body/green hair extend downward. I reviewed the paired
full frames/details, the ceiling animation sequence, and the second-flipper
sequence. The natural sequences are `before/m06_kf_1/ceiling-0.png` through
`ceiling-7.png` and `after/m06_kf_1/ceiling-0.png` through `ceiling-11.png`.
Different visible-frame counts reflect different scene-depth coverage; the
underlying per-frame simulation is byte-identical.

Both sides retain `flip-inverted.png`, `inverted-fall.png`, `ceiling-first.png`,
`hover.png`, `final.png`, `positions.csv`, `simulation.txt` and `pixels.txt` in
their level directory. The after route observes 128 visible inverted frames,
72 supported ceiling-Walker frames, 13 distinct animation entries, and three
paused frames with actor 0 both hovered and highlighted. Pause freezes the
motion sample, radial lane, animation entry and placement. Its first 73 frame
hashes match the baseline, before the first inversion changes the image.

**Return upright, supplemental real-data test:**
[`after/placed-return-flipper/`](../../build/antigrav-presentation-001/after/placed-return-flipper/transition.csv)
places an inverted Walker in actual empty space at `(24,8)`, adjacent to the
installed second flipper at `(24,7)`. Terrain is unmodified. Frame 0 is inverted;
the ordinary World update at 20 ms changes gravity to +1/Faller, and frames 1-7
draw upright. Eight retained `frame-N.png` images and `transition.csv` show the
same support origin changing sprite side. The motion tracker snaps its buffered
position on the gravity change. This is **placed-actor regression evidence**,
not a natural route to a second flip or a full-level win. The natural first
actor does not return upright in the measured route.

## Static original evidence and provisional choice

Authority throughout: registered repository reference
`Lemmings Revolution/Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`, x86,
image base `0x400000`. REA used Ghidra 12.1.4, profile digest
`3c06738e45e144d5f073edbd3244c6bff46222f389525fe659bc92dca2ccdece`.
Analysis was closed without snapshot arguments. Full returned records, including
limitations, are retained in
[`static-evidence.json`](../../build/antigrav-presentation-001/static-evidence.json)
and [`static-uv.json`](../../build/antigrav-presentation-001/static-uv.json).

Retained accepted recovery evidence:
[actor geometry](../../decomp-lemmings/docs/evidence/actor_geometry.md),
[actor finalization](../../decomp-lemmings/docs/evidence/actor_finalization.md),
and [historical inverted steppers](../../old/docs/scratch/phase3-round30-inverted.md).
The historical observations are not new execution or permission to rerun them.

Targeted static findings, cross-checked against instructions:

* `42F5E3` negates actor `+44`. Inverted gravity sets `+368=3.0` and
  `+358=0.0`; upright restores the reverse. Pseudocode evidence
  `ev_e755b7fadea8f79d2c717b572fdccc32763b56d77e026cb07fc27c6b48b21adb`;
  instructions `ev_78b5cc00224c85438bb00a735544b3f2fbafce204be931d1e62b38ff4b276fc0`
  include the exact stores at `42F653/42F65D`.
* `42F22E` adds the `+368` bias to raw actor Y for the support sample
  (`ev_3d563197fb51679f2538ad2c3ad0911714a2ab9bf003b1c489715088ea40c8d9`).
  The clone instead names `Lemming::y` as the support/feet coordinate; copying
  that three-unit bias into its position would conflate two conventions.
* Finalization `42E58C..42E5D4` writes actor `+2D8=0` for gravity +1 and
  `=1` for gravity -1, unless behavior `0x2B` is attached. Instruction evidence
  `ev_b23059172f4f4ed65ee8e6e57652b0f3676a696003f01e5adc66682b52f1cc65`
  agrees with the retained recovered implementation.
* `43CC68` calls sprite submission `47A75D` with its inline controller at
  model `+B0`; model is actor `+78`, so controller `+1B0` aliases actor `+2D8`.
  `47A7F1` and `47A999` branch on that orientation. The argument loads in
  `47A830..47A85F` / `47A9D8..47AA07` exchange frame `+10` and `+18`
  (V-bottom/V-top), preserving U; horizontal mirror remains independent.
  `440C50` stores those UV arguments in the quad corners. This establishes a
  **vertical sprite flip**, not a 180-degree horizontal-facing reversal.
  Pseudocode `ev_c5e85bc2a2bd795b822a12aa184a101250ac30d4f050ba4daa1a6f11da28fd52`,
  instructions `ev_53e9d740e04fdc4e207b40d840d1d7bafb8bf973014570a5b8ae8053b4f24523`.

**Provisional anchoring:** reflect the ordinary sprite destination vertically
around the clone's projected feet, and reverse V. Original `47A75D` reverses V
without reflecting its destination box; its raw-origin and collision-bias
convention differs from the clone's. The exact native-to-clone anchor conversion,
including perspective, is not established. The support-point reflection is the
simplest player-correct adapter and is explicitly labelled provisional in code.
An original inverted Walker/Faller contact sequence with raw position, `+368`,
frame offsets and destination corners at the same camera would settle it. That
comparison was not performed. No fidelity gate is changed. Behavior `0x2B`'s
orientation exception and untested skill-specific inverted poses remain outside
this normal Walker/Faller evidence.

## Production change and call path

`RetailLevel` imports `Terrain::fromCollisionGrid`, retaining collision bit
`0x8000` as the anti-gravity marker while converting to Y-down rows.
`World::stepWalker` probes `(x,y+verticalDir)`, flips `verticalDir`, clears fall
distance and enters Faller. `stepFaller` advances toward that gravity and tests
the corresponding support side; walking/climbing/digging already carry gravity.
These simulation functions were not changed.

The production frame loop observes World updates through `LemmingMotionTracker`,
advances `LemmingAnimTracker`, then calls `injectedWalkersFromWorld`. It now copies
`verticalDir` into the injected actor along with the existing fractional X/Y,
radial lane, facing, clip and temporal entry. No new time source or position
offset enters simulation or motion interpolation.

`prepareInjectedWalker` keeps cylinder pose mapping, view selection, tint,
horizontal mirror and projected depth, then calls `actorSpriteGravityPlacement`
on the sprite rectangle. For negative gravity only, top/bottom are reflected
around projected feet; bounds stay ordered. `drawActorSprite`'s added overload
reverses V independently of U. The old overload forwards with vertical flip off,
preserving upright callers and historical frozen-source linkage.

The production scene draws actors after completed terrain/scenery/fluid depth.
The same prepared reflected rectangle and rasterized flipped silhouette feed
`clipActorToScene`, actor compositing, `injectedWalkerScreenBoxes`,
`hoveredActor`/`runtimePickCandidate`, and hover highlighting. There is no separate
pick offset or stale upright silhouette. The existing provisional depth/contact
policy and native submission gate are unchanged. Header edits were made together
before the focused build.

## Simulation defect deliberately left

The natural route exposes an existing roof-escape defect: lemming 0 reaches
`(104,-1)`, still an inverted Faller, at 19000 ms, and `(104,-53)` at 21600 ms.
`Terrain::at(y<0)` returns Empty and `stepFaller` has no upper-world termination.
The renderer's cylinder sampler subsequently clamps the out-of-range pose at
the top ring. Both baseline and after traces have 27 above-world Faller frames
and exactly the same simulation fields. This can leave an actor apparently
suspended near the top of the screen. **Left unchanged**, with exact evidence
in both `m06_kf_1/simulation.txt` files and `positions.csv`. The separately queued
`gravity-gadgets-001` includes ceiling deaths; this presentation change does not
silently implement that dependency. No wrong simulated support placement was
found on the measured first supported ceiling section.

## Provenance and checks

All task artifacts are under ignored `build/antigrav-presentation-001/`.
`frozen/src/app/{TubeScene.cpp,TubeScene.h,ActorSprite.cpp,ActorSprite.h,main.cpp}`
contains authentic task-entry source, not reconstructed HEAD or modified files.
Every saved source hash matches supervisor entry
`build/agent-runs/20261114T222010-47060-48bc1f5c/before-source-hashes.json`;
see `before-sha256.json` and `entry-provenance.json`. `antigrav_identity` checks
the frozen files, including the task-entry CMake backup, before baseline replay.
The baseline executable overrides just those task-entry renderer/application
objects; shared support retains the same accepted surrounding code.

Registered CTest driver: `src/__test__/test_antigrav_presentation.cpp`, via
`cmake/antigrav_presentation.cmake`; no existing test assertions or timeouts changed.
The after test independently checks reversed raster texels/alpha with both
horizontal mirror choices and 0.5/1/2 sprite scales, reflected support anchoring,
the real second flipper, natural release/first flip/ceiling walking, animation,
pause, hover/highlight, simulation identity and saves.

Focused development run
[`development-ctest-3.log`](../../build/antigrav-presentation-001/development-ctest-3.log):
**8/8 passed, 66.77 seconds**: actorsprite, actorpose, actor_motion_units,
runtimepick, walker_lane_units, antigrav_identity, antigrav_before, antigrav_after.
Registered UCRT64 GCC 16.1.0 / CMake/CTest paths were used, existing `-O2 -g`
configuration with assertions, 12 build jobs, serial focused tests. This was
not a main/recovery profile run. Full verification remains the supervisor's work.

[`comparison.json`](../../build/antigrav-presentation-001/comparison.json):

| Scripted production route | Simulation before/after | Entire-frame pixel hashes |
| --- | --- | --- |
| node 11, m06_kf_1, 220 frames | byte-identical | first 73 equal; inversion changes subsequent images |
| node 0, MM_CLIMB_002, 160 frames | byte-identical | 160/160 equal |
| node 6, MM_BASH_002, 160 frames | byte-identical | 160/160 equal |

The second control was selected by installed structural data: zero anti-gravity
cells and zero randomized sprite-scenery nodes (`control-selection.txt`). No
image mask or tolerance is used. These two routes, not all non-anti-gravity
levels or every skill/camera, establish the bounded pixel regression claim.

Earlier development attempts are retained, not presented as passing:
`development-1/ctest.log` and `development-ctest-1.log` retain the zero-stock
hover-driver error and mismatched pause trigger; `development-2/ctest.log` and
`development-ctest-2.log` retain the MM_BOMB random-flame control mismatch.
MM_FLOAT also has random flames. The final test selects stocked Blocker, uses
the same 9000-ms pause on both sides, and chooses a deterministic full-frame
control from measured sprite counts. Production randomness was not changed.
The initial partial build also exposed a missing include search directory for
the frozen Camera header; CMake now supplies the normal app include directory.

Both final `before/summary.txt` and `after/summary.txt` report installed and normal
saves unchanged and zero failures. Private saves are also asserted unchanged.
Source LF and scoped diff checks pass; unrelated pre-existing line-ending warnings
were left untouched. `final-source-audit.json` records the four production files
and CMake change against entry. Its queue hash difference is the supervisor's
ordinary ready-to-in_progress transition at 22:20:10 UTC (`supervise.py:317`),
before worker execution; the worker did not write the queue. No commits, detached
jobs or agents were created. Only Ghidra/Java static analysis and source-built
clone/test executables ran.

Handoff: antigrav-presentation-001 is needs_review; R8 presentation fixed on node 11, static flip evidence and provisional feet anchor documented, focused checks 8/8.
Next action: supervisor full registered main/recovery verification, then independent acceptance review; roof-escape simulation remains for gravity-gadgets-001 and human playability remains unconfirmed.

## Retry 002: frozen renderer/header consistency

The first supervisor [verification log](../../build/agent-runs/20261114T222010-47060-48bc1f5c/verification.log)
is a **failed** run: **258/260 main tests passed**. `actor_occlusion_before`
terminated with `0xc0000409`; `actor_depth_before` segfaulted. That log does not
establish a completed recovery pass. The earlier scoped 8/8 result remains
historical evidence; it did not exercise those two snapshot executables.

Source inspection identified an incompatible vector element layout. Both old
`TubeScene.cpp` files include their own authentic local `TubeScene.h`, without
the new gravity field, while the snapshot drivers included today's header.
Returning, indexing and copying the resulting walker vectors crossed those two
definitions. This is a baseline-build defect introduced by the R8 header change,
not an observed gameplay crash. No debugger was used to diagnose it.

`CMakeLists.txt` now gives each affected **before** target a private include
directory containing a byte-for-byte copy of its own frozen `TubeScene.h`.
Only that header is selected; current support APIs stay available to the driver.
This matches the driver and its frozen renderer without rewriting either frozen
file or constraining the production actor structure to a historical layout.
The frozen main used by the old occlusion route uses that same layout. No
assertion, timeout, test count, verifier, controller or production behavior changed
in this retry. Both historical headers match their retained task-entry manifests;
their copied include headers match as well. Exact hashes and the unchanged
260-test inventory are recorded in
[`retry-002/preservation-check.json`](../../build/antigrav-presentation-001/retry-002/preservation-check.json).

Registered UCRT64 configuration and the six affected demonstration targets built
successfully: [configure](../../build/antigrav-presentation-001/retry-002/configure.log),
[build](../../build/antigrav-presentation-001/retry-002/build.log).
The [focused CTest run](../../build/antigrav-presentation-001/retry-002/ctest.log)
passed **12/12 in 125.58 seconds**: both historical identities, the reference-byte
check, both old/new snapshot pairs, both application routes, and anti-gravity
identity/before/after. This includes the previously crashing tests, unchanged
simulation and control-frame comparisons, and save-preservation assertions.
No full verifier profile was run in this retry; supervisor verification remains
pending on the final content.

The regenerated node 11 paired images and second-flipper frames were reviewed.
The before sprite is clipped into the ceiling; the after sprite extends down
from the same feet origin. The animation and return-upright sequences retain
the documented provisional anchor and unchanged simulation/roof-escape limits.
Final paired artifacts are archived under
[`retry-002/evidence/`](../../build/antigrav-presentation-001/retry-002/evidence/),
pinned by `evidence-sha256.json` so later verifier runs cannot replace this review
evidence. `comparison.json` there retains the three per-frame comparison results.
Retry-entry sources/images/diff are under `retry-002/entry-source/`,
`entry-evidence/` and `entry.patch`; the original task-entry frozen baseline was
not refreshed. This retry changed only CMake registration and this report.
The parent supervisor lease remained active; no protected manual edit was made.

Handoff: antigrav-presentation-001 is needs_review; R8 presentation and frozen-baseline compatibility are addressed, with 12/12 focused checks passing.
Next action: supervisor full registered main/recovery verification, then independent acceptance review; provisional anchoring and roof escape remain documented limits.
