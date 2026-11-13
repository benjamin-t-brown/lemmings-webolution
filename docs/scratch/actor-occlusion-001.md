# actor-occlusion-001: cylinder occlusion (P8)

Status: **needs_review**. Playable v1, roadmap step 5, human playtest
[P8](../playtest/2026-11-12-playtest-1.md). Difficulty: difficult; assigned
recommendation Sonnet-class, this worker using the explicit Codex provider
override. Full registered main/recovery verification is **pending supervisor
execution**, then independent acceptance review. No original game, original-byte
oracle, debugger, hooks, original capture, subagents or detached work ran.

## Failure, dependency and production path

The human saw lemmings through the sides of the cylinder during rotation. This
task follows accepted actor animation and motion: those paths now provide a
readable moving sprite, but the actor overlay still ignores the cylinder body.
It changes only P8 presentation and the existing visibility list consumed by
picking/countdowns. Camera, tube geometry, simulation, skills, release/timing,
visible-actor pick rectangles and all fidelity gates/parks are unchanged.

Production path: `runApplication` -> `injectedWalkersFromWorld` (including the
accepted motion/animation trackers) -> `TubeScene::render` -> the four tube
route passes -> `drawInjectedWalkers` -> `prepareInjectedWalker` -> native
submission gate, ActorPose, view/temporal selection, sprite placement and tint
-> `drawActorSprite` -> overlay composition. Previously only the angular
submission gate could reject an actor; it admits far-facing actors as well.

`injectedWalkerScreenBoxes` calls the same preparation helper.
`main.cpp::hoveredActor` constructs candidates only from its non-null entries;
the fuse-label stage likewise builds `LevelActorView` only from those entries
before calling `drawBomberCountdowns`. A wholly masked actor now has a null
entry for all three consumers. A partially visible actor retains its entire
original placement, preserving existing visible-actor selection behavior and
label anchoring.

## Static evidence and provisional policy

Retained evidence consulted:
[actor rendering](../lessons/actor-rendering.md),
[camera/tube/sprites](../lessons/camera-tube-sprites.md),
[A6c composition](phase-a6-actor-tube-composition.md),
[A6d16 submit depth](phase-a6-actor-tube-pixel-z.md), and the
[historical actor-Z park](../plan-history/phase-e.md#a6-per-pixel-actor-z).
The latter remains a historical fidelity park; this authorized P8 fix does
not resolve it.

Reference analyzed statically, without executing it:
`C:/progs/lemmings-webolution/Lemmings Revolution/Lemmings Revolution.exe`,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
REA Ghidra 12.1.4 open/identity, pseudocode and complete instructions are
retained under [static evidence](../../build/actor-occlusion-001/static/).
Analysis was closed without snapshot arguments.

**Retained READ:** `0x43A4EC` submits actors after INN/OUT tube passes; its
`0x43A94D..0x43AA47` gate uses the inclusive angular bucket range for shell
Z < 1, bypassed for Z >= 1 (`ActorVisibility.cpp`). A6d16 traced the submit
chain `0x43CC68 -> 0x47A75D -> 0x47AB96 -> 0x414920`; it found no software
actor-fragment-versus-scene-depth comparison. That does not establish effective
hardware depth state.

**New READ, cross-checked against instructions:** `0x43CC94..0x43CCA7`
compares the projected origin component at object+0xC8 with zero and skips
the whole draw on the negative branch. `0x43CCB5` subtracts the retained
0.0003 constant; `0x43CCCA` calls `0x43CDA0`; `0x43CD4E` submits through
`0x47A75D`. These are origin/depth preparation, not a tube-fragment rejection.
Evidence IDs: pseudocode `ev_0fc271b435b10a844d45e665b97bf248628b7f01ab9bd21a64416ae76d58f60f`,
instructions `ev_9a8209aba409c9745a27529fbccfc174c42f9a83172b48c6c3544c73701ba1e3`.
Pseudocode is provider output, not recovered original source.

**Provisional playable-v1 choice:** an opaque, finite prism following the
installed template's 32-sided outer envelope masks **far-facing** actors.
Far-facing means radial outward direction dotted with camera-minus-actor is
negative, including the perspective tangent. Each opaque sprite texel uses
the accepted integer pixel center. Its inverse-view ray is intersected with
the hull half-spaces and vertical bounds, stopping at the billboard depth.
The conversion `placement.depth * camera.width / 0.3` keeps the accepted
0.0003 projection bias in its existing normalized units. Surviving texels
keep their original RGB/alpha; hidden texels never overwrite another actor.

Near-facing sprites take the old path without masking. Actor origins normally
sit inside the shell at slab 0.75, so testing every near-side texel against an
intact outer surface would bury actors in the shell and in dug tunnels. The
new policy preserves those actors while rejecting views through the body.
It changes neither the tube raster nor its depth buffer. The raw-world-pose
diagnostic (`composeSkipActorPose`) retains its existing overlay behavior.
The explicit visibility diagnostic (`composeSkipActorVisibility`) bypasses
both the native submission gate and this mask; normal drawing, boxes,
countdowns and picking leave it off. This diagnostic extension was added in
the retry described below and does not change the normal paired images.

This is **not** original depth-state recovery. The envelope intentionally does
not model far-side visibility through a hole traversing the complete body,
individual scenery/ledge occluders, or actor-to-actor depth sorting. Near-side
tunnel excavation remains visible through the existing terrain-sync path.
A future authorized original observation at a matched rotating pose, including
a dug opening and draw-time effective depth state, would settle the true
far-side/opening behavior. No original observation was requested or attempted
in this worker.

## Installed before/after measurements and reviewed images

All are **source-built clone observations**, not human play or original
comparisons. Two executables compile authentic task-entry versus production
`TubeScene.cpp`; they share unchanged application/support code. The scene
measurements load real installed LVL/archive data, naturally release lemmings
through `World::advanceElapsedMs`, then render the first live actor separately
through the production route. The paused snapshots are sampled over a complete
360-degree yaw turn in four-unit steps, with additional one-unit silhouette
samples: **88 distinct views per level**. No actors are injected into these
natural scene snapshots.

| Installed route/snapshot | Simulation evidence | Before -> after |
| --- | --- | --- |
| `MM_CLIMB_002`, hatch to first ledge | 6,000 ms; Walker at (49,63) | 40 submitted far-facing views paint actor pixels -> all 40 paint zero and have no box; 31 near views stay visible |
| `MM_BASH_002`, first tall earth pillar | 31,800 ms; active Basher at (150,55); 12 terrain cells removed through simulation, synchronized to the scene | Same 40 far rejections and 31 unchanged near views, including the actor in its dug tunnel |
| `MM_BOMB`, hatch route with `World::assign(Bomber)` | 6,000 ms; Walker at (169,23); live 5,000 ms fuse | Same rotation result; far actor's countdown disappears with it, near countdown frame stays identical |

Each level has **48 pixel-identical views**: 31 near-side views plus 17
non-submitted views. This includes every tested frame without a far-side
actor. CSVs retain camera yaw in degrees, facing classification, submission
gate, visible box, actor-painted pixel count and whole-frame hash.
For example, `MM_CLIMB_002` at yaw **153.28125 degrees** paints **120 -> 0**
pixels and at **164.53125 degrees**, **114 -> 0**. Its near yaw
**68.90625 degrees** paints 105 pixels on both sides with an identical frame.

Reviewed matching full frames:
[ledge before](../../build/actor-occlusion-001/evidence/before/MM_CLIMB_002/yaw-68.png),
[ledge after](../../build/actor-occlusion-001/evidence/after/MM_CLIMB_002/yaw-68.png),
[dug tunnel before](../../build/actor-occlusion-001/evidence/before/MM_BASH_002/yaw-0.png),
[dug tunnel after](../../build/actor-occlusion-001/evidence/after/MM_BASH_002/yaw-0.png),
[near fuse](../../build/actor-occlusion-001/evidence/after/MM_BOMB/countdown-0.png),
[hidden fuse](../../build/actor-occlusion-001/evidence/after/MM_BOMB/countdown-68.png).
The tunnel actor remains readable beside its visible notch.

The **supplemental** silhouette fixture reuses the installed Climber pose with
explicit explosion-strip selections. It is an injected visual fixture, not a
natural explosion route. Four selections retain some pixels while clipping
others. At temporal 9/yaw 153.28125 degrees, **91 -> 1** painted pixels:
[full before](../../build/actor-occlusion-001/evidence/before/MM_CLIMB_002/supplement-explode-9-60.png),
[full after](../../build/actor-occlusion-001/evidence/after/MM_CLIMB_002/supplement-explode-9-60.png),
[enlarged before](../../build/actor-occlusion-001/evidence/before/MM_CLIMB_002/supplement-explode-9-60-enlarged.png),
[enlarged after](../../build/actor-occlusion-001/evidence/after/MM_CLIMB_002/supplement-explode-9-60-enlarged.png).
The review aids magnify the clone pixels sixfold with nearest-neighbor copying;
the full frames remain the measurement artifacts. Inspection confirms the
remaining pixel lies outside the cylinder silhouette.

The public application route uses `runApplication --level MM_CLIMB_002 --sim
--native`, natural release, pause, hover, right-button camera drag, hidden
click, return drag, near hover and Climber click. It uses an owned 100 ms
frame clock solely for repeatability. Both runs have **75 frames**, **14 paused
frames**, identical simulation positions/clock/camera throughout, and identical
near/returned-near full PNGs and visible pick rectangles. At yaw units
**127.83333333333334** (179.765625 degrees), before actor 0 has box
`[126.00924,224.83849 .. 137.00925,244.83849]` and is hovered through the body;
after it has no box, cannot be hovered, and the same click spends no skill.
Returning to yaw units 32 restores selection and assigns Climber normally.

Reviewed [application before](../../build/actor-occlusion-001/evidence/app-before/far.png)
and [after](../../build/actor-occlusion-001/evidence/app-after/far.png), plus
[enlarged before](../../build/actor-occlusion-001/evidence/app-before/far-enlarged.png)
and [after](../../build/actor-occlusion-001/evidence/app-after/far-enlarged.png).
The other, near-side actor remains visible. Besides the removed far sprites,
the changed selection bracket/crosshair is expected existing cursor feedback
for the absent hover target. The screenshot differences are not camera or
simulation changes.

## Checks, preservation and limitations

**Retry after supervisor failure (2026-11-13):** the prior full main run
[verification.log](../../build/agent-runs/20261113T204941-36400-c32647a7/verification.log)
passed 200/202 tests and failed `levelvisibility` and `basherfix_away`; it was
not a verification pass. Recovery was not reached. The prior worker's
pending-verification handoff did not establish full acceptance.

`levelvisibility` deliberately bypasses the native visibility gate to obtain
a hidden actor's projection and prove that the ordinary app cannot draw,
hover or assign it. The new mask initially defeated that diagnostic too.
`prepareInjectedWalker` now honors that existing explicit diagnostic for the
mask as well. The unchanged `levelvisibility` assertions pass. The new P8
rotation test additionally verifies that diagnostic far actors have boxes and
pixels matching the authentic unmasked baseline at all 40 submitted far views
per installed level, while ordinary drawing/boxes remain occluded.

`basherfix_away` previously dragged toward `256 - actor column`, which puts
its gap-64 target behind the cylinder: the retained failed trace at tick 59
has actor 0 at (216,33), camera yaw 41.1667 units, and no visible actor-0 box.
Camera::buildCamera and ActorPose use the same angular direction. The driver
now drags toward the actor column instead. This corrects its public camera
input, without changing any Basher assertion, skill/simulation rule, distance,
test registration, count or timeout. The retry clicks at **the same tick 59
and cell (216,33)**, now at recorded camera yaw 218.833 units, then observes 15 Basher
ticks, stock 10 -> 9, x 215 -> 214, no terrain removed, and Walker at tick 75.
The pillar case retains tick 132/cell (154,55), 123 Basher ticks, 64 removed
cells and eight followers through the tunnel. Both gap-1/3 probe routes also
complete and record scripted 10/10 wins after the corrected camera follows
them to later walls; these are clone regressions, not human playability claims.
Earlier Basher reports/results remain historical and were not rewritten.

Final retry checks: **23/23 scoped CTest cases passed** in
[tests-1.log](../../build/actor-occlusion-001/retry-002/tests-1.log)
(10/10, 54.71 s: both previous failures, all four Basher routes and all five P8
cases) and
[tests-regression.log](../../build/actor-occlusion-001/retry-002/tests-regression.log)
(13/13, 51.30 s: composition, placement, pose, visibility, picking, click
assignment, state animation, movement and terrain rendering). Registered
UCRT64 builds are retained in the same directory. No full profile was run in
the retry; **full registered main/recovery verification is pending the
supervisor**, followed by independent acceptance review.

The regenerated 912 evidence files are all byte-identical to the original
retained evidence; both sets were checked against the original pinned SHA-256
manifest. See
[evidence-comparison.json](../../build/actor-occlusion-001/retry-002/evidence-comparison.json).
This includes all 264 installed snapshot views, public app traces, near frames,
countdowns and silhouette fixture. Regenerated before/after side and partial
silhouette images were visually reviewed again; the app far-side crop and
near tunnel frame were also reviewed. The retry retains an additional immutable
[evidence copy](../../build/actor-occlusion-001/retry-002/evidence/); failed
Basher traces are in the retry root, corrected traces under `evidence/basher-route/`.
The authentic before-source was never
refreshed from resumed production files.

Prior attempt's scoped check (superseded as final source verification):
[tests-complete.log](../../build/actor-occlusion-001/tests-complete.log),
**18/18 passed in 51.57 s**, build in
[build-complete.log](../../build/actor-occlusion-001/build-complete.log).
Registered UCRT64 GCC 16.1 / CMake / Ninja / CTest; `-O2 -g`, assertions on,
12 build jobs, four test jobs. Five new CTest cases register baseline identity,
before/after installed rotation measurements and public application routes.
The other checks cover actor composition/placement/pose/visibility, runtime
picking and click assignment, animation units/pause/climb/bomb/bash, accepted
movement, and terrain rendering. No existing assertion, timeout or test count
was changed. No full profile was run or restarted; the supervisor owns it.

Development logs remain historical. `tests-1.log` exposed a test-driver mistake:
the first any-solid approach included a thin step/steel seam, never excavated
a tunnel, and correctly failed its tunnel assertions. The driver was corrected
to choose the accepted route's tall non-steel pillar; `tests-2.log` and later
checks pass. This did not change production skill behavior or an existing test.
`tests-app-1.log` and `tests-final.log` are earlier passing scoped runs;
`tests-complete.log` identifies the prior attempt's tested source/test content.

All detailed artifacts are archived under
[build/actor-occlusion-001](../../build/actor-occlusion-001/).
`before-source/`, `before-hashes.json`, `pre-existing.patch`, `entry-status.txt`,
`HEAD-main.cpp` and `baseline-source-check.json` retain authentic pre-edit
provenance. Frozen TubeScene SHA-256:
`954eb8113f0349b7a22bb3ffe104ec64f4ba93597bc13f41919538656de21ee3`, pinned by
CTest. It was never refreshed from modified production files. A clean checkout
without that ignored baseline runs production checks and explicitly omits the
paired comparison; this workspace runs both and requires the comparison.

`evidence/` retains 912 image/trace/measurement files copied after the first
attempt's final scoped run; `evidence-sha256.json` pins them against future verifier rewrites.
`task-only.patch` separates P8 from incoming accepted edits;
`worker-tested-sources.json` identifies that attempt's tested content. Its `preservation-check.json`
confirms all 279 non-task source/build files in the 281-file backup are
byte-identical. The controller's ready-to-in-progress queue transition is
recorded separately, not attributed to this worker.
The retry's [preservation check](../../build/actor-occlusion-001/retry-002/preservation-check.json)
retains the additional header-comment and Basher driver edits: 277 of the
281 original source/build files remain byte-identical, with only the four
declared existing P8 files changed. Retry entry backups and hashes preserve
the interrupted implementation and failed traces; its final source manifest
and `retry-only.patch` identify the changes made in this invocation. Protected
controls, shared instructions, incoming unrelated edits and the active
supervisor lease are unchanged. Process enumeration through `Get-Process`
showed no clone/build/test process at entry; CIM command-line enumeration was
unavailable in the sandbox, so no attempt was made to bypass it.

Both application runs use a private temporary `FileSaveSink` under their own
test output directory and leave default saving disabled. Scene measurements
never open a save store. Every new test asserts installed and normal clone save
existence/full bytes unchanged. No installation, queue, verifier, controller,
registry, permissions, shared instructions, model pins or completion criteria
were edited. The existing supervisor lease owns this worker; no manual protected
control edits were performed. No commit/push/reset/clean/stash. Task text is LF;
`git diff --check` on task files passes.

Changed task files: `src/app/TubeScene.cpp`,
`src/app/TubeScene.h` (comments only), `src/__test__/test_actor_occlusion.cpp`,
`src/__test__/test_basherfix.cpp` (camera driver only), `CMakeLists.txt`,
`cmake/actor_occlusion_identity.cmake`, and this report. Limits: three installed
snapshot routes and the documented public-input routes,
not all-level human play; no playable-v1 completion or original-fidelity claim. An optimized human
rotation/tunnel/ledge/countdown playtest is still needed after review.

Handoff: P8 cylinder masking and its two verification regressions are addressed; retained paired images are unchanged, 23 scoped tests pass, status needs_review.
Next action: supervisor full registered main/recovery verification, independent acceptance review, then optimized human rotation/tunnel/ledge/countdown playtest.
