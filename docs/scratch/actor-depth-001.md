# actor-depth-001: scene depth for lemmings (playtest 3 R3)

Status: **needs_review**. Playable v1, implementation-plan step 5 / P8 and
[human playtest 3 R3](../playtest/2026-11-13-playtest-3.md). Difficulty: difficult;
assigned recommendation Sonnet-class, executed with the authorized Codex override.
Full registered main/recovery verification is **pending supervisor execution**,
followed by independent acceptance review. These are clone observations and source
checks, not an original comparison or a playable-v1 completion claim.

The human reported both wall leaks and premature disappearance, especially while
climbing. The accepted actor-occlusion-001 used an unedited cylinder envelope for
far-facing actors and overlaid every near-facing actor. Actual side walls,
open silhouettes and skill edits cannot be represented by that rule. Replacing
that mask is the next presentation dependency; camera, geometry, simulation,
release, skill rules, shared timing and historical fidelity gates are unchanged.

Production path: `runApplicationLeg::renderFrame` -> `TubeScene::syncTerrain` ->
`scheduleSceneFrame` (primary scenery, legacy, tube geometry, fluids, secondary
scenery) -> `TubeScene::drawInjectedWalkers` -> `prepareInjectedWalker` -> native
submission gate, `ActorPose`, directional/temporal selection, placement, tint ->
`drawActorSprite` -> per-pixel scene-depth mask -> existing opaque composition.
Standalone `TubeScene::render` submits actors after its complete tube geometry.
The existing before-route diagnostic is preserved. Actors retain their accepted
reverse submission order and do not write or alter scene depth.

Picking, hover and click assignment now pass the **displayed raster explicitly**
to `injectedWalkerScreenBoxes`. Its preparation/mask is shared with drawing.
A fully hidden sprite has no box; a partly visible sprite keeps its original
rectangle and remains pickable. Countdown anchoring uses those same visible
actors, then each glyph pixel is tested against the displayed scene depth.
The four-argument legacy countdown API remains available for existing isolated
fixtures. The three-argument scene-box diagnostic constructs fresh tube-only
depth; production uses the complete frame, without a cached or stale depth map.

The depth derivation is explicit. `Raster::triangle` stores affine physical view
Z; smaller is nearer, cleared depth is infinity. `SceneDepth` converts scenery
and fluid submissions to those same units. The actor origin uses the same camera
world-to-view Z row through `ActorSprite::actorProjectionInput`, normalized by
`0.3 / width`. Existing placement already subtracts `0.0003`. Thus:

```
actor comparison Z = placement.depth * width / 0.3 - 4.0
                   = worldToViewZ(actor position) - 0.001 * width - 4.0
```

Each pixel of the flat camera-facing sprite/label plane uses that Z and passes
only when it is strictly less than the scene depth at that framebuffer pixel.
Actor texel alpha, sampling, screen placement, tint and the terrain raster's
sampling convention are preserved. The additional **4.0 world units** are a
**provisional contact allowance**, separate from the traced normalized bias.
At width 640 the combined allowance is 4.64 units. Keeping only the traced 0.64
units visibly clipped the contact climber and tunnel pose during development;
the final checks explicitly require intact exposed contact poses. This tolerance
can admit surfaces within the allowance, so it is not an original depth-state or
subpixel silhouette fidelity claim. More distant foreground edges still mask
individual sprite pixels.

Retained static evidence, reread rather than rerunning research:
[A6d16](phase-a6-actor-tube-pixel-z.md),
[A4 actor material state](phase-a4-actor-render-state.md), and
[accepted actor-occlusion report](actor-occlusion-001.md). The original submit
chain is `0x43CC68 -> 0x47A75D -> 0x47AB96 -> 0x414920`; absence of a software
fragment comparison does not establish effective hardware depth state.
The retained Ghidra instructions at `0x43CC94/43CC9A`, `0x43CCB5`, `0x43CCCA`
and `0x43CD4E` were cross-checked against retained pseudocode: origin sign gate,
subtraction of `[0x4B1498]`, depth-slot setup, sprite submission.
The current registered reference SHA-256 is
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
A static byte read confirms `0x4B1498`, file offset `0xB1498`, contains
`613255302aa9333f` (binary64 0.0003). Retained REA evidence ID:
`ev_9a8209aba409c9745a27529fbccfc174c42f9a83172b48c6c3544c73701ba1e3`.
[Retained evidence and check](../../build/actor-depth-001/static/retained-evidence-check.json)
include the original Ghidra packets. No new REA session was needed or opened.
CTest `actor_depth_reference` registers the hash/constant measurement without
executing the reference. Original observation of paused contact/tunnel actors
through a camera rotation, together with effective device Z state, would settle
the original plane/bias policy. That observation was not run or authorized here.

The paired source-built routes use real installed level data, natural hatch
release, production `World::assign` for skills, the animation tracker, and the
same actor/world/render adapters as the application. No actor state, position,
terrain or completion was injected into the simulation. Only actor 0 is selected
for these isolated render measurements. These are snapshots of actual simulated
routes, not full frontend playthroughs or human playability evidence.

| Route | Snapshot | Clock | Terrain edits |
| --- | --- | --- | --- |
| MM_CLIMB_002 wall climber | (64,59), Climber, climb entry 15 | 9.8 s | 0 |
| MM_CLIMB_002 ledge walker | (49,63), Walker, walk entry 10 | 6.0 s | 0 |
| MM_BASH_002 pillar walker | (155,55), Walker, walk entry 2, facing left | 25.9 s | 0 |
| MM_BASH_002 dug tunnel | (150,55), Basher, bash entry 2, facing left | 31.8 s | 12 cells |

The Basher is assigned three cells before the tall non-steel pillar, then observed
five cells into its excavation; `syncTerrain` supplies the resulting geometry.
Paired `state.txt` files assert identical simulation snapshots. Each route has
**48 evenly spaced views** (7.5-degree increments). All camera parameters are
canonical `CameraParams` defaults except `yawUnits = actor.plus8X + view*256/48`.
The CSV records exact yaw units; image names use view numbers. The 12-view review
sheets select every fourth view, at 30-degree intervals across a full rotation.

Examples from the retained matching full frames (degrees are the yaw parameter
in degrees, before the fixed camera yaw offset):

| Route / view | Yaw units / degrees | Before actor pixels | After |
| --- | --- | --- | --- |
| Climber / 8: drawn through wall | 106.66666666666666 / 150 | 116 | 10, clipped at edge |
| Climber / 36: wrongly hidden | 256 / 360 | 0 | 121, intact |
| Climber / 0: exposed contact | 64 / 90 | 112 | 112, intact |
| Ledge / 36: wrongly hidden | 241 / 338.90625 | 0 | 129, intact |
| Pillar / 40: foreground pillar edge | 368.33333333333337 / 517.96875 | 123 | 9 |
| Tunnel / 12: wrongly hidden | 214 / 300.9375 | 0 | 95, foreground ceiling clips head |
| Tunnel / 0: exposed contact | 150 / 210.9375 | 134 | 134, intact |

Reviewed all paired 12-view sheets:
[climber before](../../build/actor-depth-001/evidence/actor-depth-before/climber/review-12-views.png) /
[after](../../build/actor-depth-001/evidence/actor-depth-after/climber/review-12-views.png),
[ledge before](../../build/actor-depth-001/evidence/actor-depth-before/ledge/review-12-views.png) /
[after](../../build/actor-depth-001/evidence/actor-depth-after/ledge/review-12-views.png),
[pillar before](../../build/actor-depth-001/evidence/actor-depth-before/pillar/review-12-views.png) /
[after](../../build/actor-depth-001/evidence/actor-depth-after/pillar/review-12-views.png),
[tunnel before](../../build/actor-depth-001/evidence/actor-depth-before/tunnel/review-12-views.png) /
[after](../../build/actor-depth-001/evidence/actor-depth-after/tunnel/review-12-views.png).
Full 640x480 frames, expected masks, sixfold nearest-neighbor crops, countdowns,
exact state and rotation CSVs are adjacent to each sheet. Foreground tunnel lips
can legitimately hide part of a head at oblique views; the contact allowance does
not make all near-facing/tunnel actors unconditional overlays again.

Across 192 views the authentic before renderer leaks pixels in **25 views** and
omits expected visible pixels in **17 views**. After: zero mismatches against the
independent unmasked-sprite + actual-scene-depth comparison, with **67 fully
visible, 32 partially visible and 93 fully hidden** views. Picking agrees in all
192. Countdown checks exercise **3,277 accepted and 1,235 rejected** glyph pixels.
A labelled supplemental later-scenery mask on the real Climber level also checks
the final actor-pass API and explicit-raster picking: partial actor stays pickable,
fully hidden actor does not, scene depth is unchanged. It is a synthetic contract
check, separate from the actual wall/pillar/tunnel routes.

The retained public SDL application route runs 75 frames, pauses for 14, rotates
away, attempts a hidden click, returns, and assigns Climber normally. Its
simulation, clock, camera and exposed pick rectangles match before/after.
**21 full production frames with no living lemmings are PNG-byte-identical**;
all **192 paired actor-free tube frames** are also byte-identical.
[Application traces/images](../../build/actor-depth-001/evidence/actor-occlusion-app-after/)
use their own private `SavedGame.dat` through `FileSaveSink`; default saving is
disabled. Snapshot render tests never open a save store. All check installed and
normal clone-save existence/full bytes unchanged.

Checks use registered UCRT64 GCC 16.1/CMake/Ninja/CTest, `-O2 -g`, assertions on,
12 build jobs and four test jobs. The earlier
[26-case scoped run](../../build/actor-depth-001/tests-scoped.log) passed in 85.25 s:
actor placement/composition, visibility/picking, Basher routes, Climber/Bomber/
Basher animation, movement, walker lanes and paired occlusion routes. After the
final countdown loop optimization and evidence additions,
[9 task/occlusion cases](../../build/actor-depth-001/tests-handoff.log) passed in
25.39 s. The added full-production actor-free-frame assertions then passed in
[2 application cases](../../build/actor-depth-001/tests-no-actors.log), 8.41 s.
Build logs are adjacent. No full verification profile was run or restarted.
**Full registered main/recovery verification remains pending the supervisor.**

Development failures remain in the archive. `tests-depth-1.log` exposed a frozen
renderer class-layout mismatch from an attempted depth cache; that cache and all
class data-layout changes were removed. The final API receives the raster
explicitly. `tests-legacy-1.log` exposed old expectations that every near-facing
actor must remain visible/pixel-identical. Those two expectations encoded R3;
they now compare against actual scene depth, preserving the before expectations,
other assertions, existing counts and timeouts. `tests-evidence.log` and
`tests-scene-contract.log` exposed Windows text-mode CRLF translation in the new
snapshot equality check; state records now write binary LF, without relaxing
that equality assertion. `tests-handoff.log` resolves those failures. An initial
scoped build named a nonexistent lane target; its log is retained, and the
registered `test_walker_lane` target was built and tested successfully.

Authentic entry source is retained under `build/actor-depth-001/before-source/`,
with `before-hashes.json`, entry status/diff and `HEAD-main.cpp`. Frozen TubeScene
SHA-256 is `1ebb174a55f8a18ef3157d11b2e622dda829890af135c05b4e8a09ea1a066b77`.
CTest pins it and the entry main. The older actor-occlusion comparison retains
its original frozen TubeScene, using this task's frozen entry main to avoid
running the newly changed application compositor as its before side. Existing
historical evidence/source backups are not refreshed. A checkout without ignored
baselines still runs the production checks and explicitly omits paired tests.

Changed existing files: `CMakeLists.txt`, `src/app/{ActorSprite.h,LemmingAnim.h,
LemmingCountdown.cpp,Raster.h,TubeScene.cpp,TubeScene.h,main.cpp}`, and
`src/__test__/test_actor_occlusion.cpp`. New files:
`src/__test__/test_actor_depth.cpp`, `cmake/actor_depth_identity.cmake`,
`cmake/actor_depth_reference.cmake`, and this report. Preservation manifests and
`task-only.patch` are under `build/actor-depth-001/`; final images are copied
under `evidence/` and pinned by `evidence-sha256.json` for review after verifier
regeneration. The [preservation check](../../build/actor-depth-001/preservation-check.json)
confirms 287 of 296 existing source/build files are byte-identical, with only the
nine declared existing task files changed. All 1,468 retained evidence files are
pinned. Incoming log/queue/status diff entries and the active supervisor lease
are preserved. No protected control or shared instruction was edited.

Remaining limits: original effective depth state/bias is unknown; four snapshots
and a bounded public route do not establish all-level/camera/zoom correctness or
human playability. Partial actors retain rectangular picking. The provisional
contact tolerance needs an optimized human Climber/tunnel rotation recheck.
No original game, copied instructions/oracle, debugger, hook, original capture,
installation tooling, subagent, detached work, commit/push/reset/clean/stash ran.

## Retry 002: static measurement registration

The first supervisor verification is a **failure**, not a completed main/recovery
test run: [verification stderr](../../build/agent-runs/20261114T011640-38924-b0e5f70c/verification.stderr.log)
reports `Unexpected original-code test: actor_depth_reference`. Its build finished,
then the inventory guard stopped before CTest because the static measurement's
command arguments contained the registered reference EXE path. None of that
invocation's main/recovery tests ran. This corrects the earlier pending-verification
handoff without replacing the failed run or its history.

This retry changes only `CMakeLists.txt`, `cmake/actor_depth_reference.cmake`
and this report. CTest now supplies the asset directory to the CMake script;
the script constructs its read-only input path internally. The command runs
CMake, and its only reference operations remain the same `file(SHA256)` and
`file(READ)` assertions. No process-launch operation was added. The verifier,
test assertions, test counts, timeouts and production implementation are unchanged.
The unchanged `verify.py::check_test_inventory` accepts all **223 registered
main tests**, including this static check:
[inventory check](../../build/actor-depth-001/retry-002/inventory-check.json).

The registered UCRT64 build/configuration succeeded, and the **9 task/occlusion
tests passed in 25.58 s**:
[scoped CTest log](../../build/actor-depth-001/retry-002/tests-scoped.log).
They include both static identities, the reference-byte measurement, both
48-view snapshot renderers, the older occlusion routes and both public application
routes. The regenerated after summary still has zero leaked/missing views,
32 partial / 93 hidden / 67 intact views and zero failures. Save-preservation,
actor-free frame equality, countdown and picking assertions passed. This retry
did not run the full suite; **full registered main/recovery verification remains
pending supervisor execution on this final content**.

I reread the production depth/actor/countdown call paths and reviewed all eight
retained before/after 12-view sheets. The climber wall edge at view 8 clips
instead of showing the whole actor; the open climber and ledge at view 36
reappear. Pillar view 40 retains only the exposed fragments. The tunnel's
foreground lip at views 8/12 clips the head, while its exposed view 0 remains
intact. These images are clone observations under the stated provisional plane
and bias, not an independent original-depth comparison. The original effective
device state and contact-bias fidelity remain unknown.

Retry-entry task sources and the incoming patch are preserved under
`build/actor-depth-001/retry-002/entry-source/` and `entry.patch`; the authentic
pre-task `before-source/` and archived paired evidence are unchanged. The retry
preservation check and final hashes are adjacent. Process checks used
`Get-Process` after CIM enumeration was denied; no clone/build/test process was
present at entry. The parent supervisor lease remains owned by PID 38924; no
protected control edits or independent lease acquisition were needed. Task text
is LF and scoped `git diff --check` passes.

Handoff: R3 sprites, countdowns and picking use per-pixel scene depth; retry fixes the static-test registration, 9 scoped tests pass, status needs_review.
Next action: supervisor full registered main/recovery verification and independent acceptance review, then a human Climber/pillar/tunnel rotation playtest.
