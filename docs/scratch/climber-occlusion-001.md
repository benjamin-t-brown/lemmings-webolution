# climber-occlusion-001: cover climbers behind their climbing wall

Status: **needs_review**. Playable v1, implementation-plan step 5 / playtest 5
U1 and playtest 3 R3. Difficulty: difficult; assigned Sonnet-class recommendation,
authorized Codex implementation override. **19 focused registered tests pass**;
full registered main/recovery verification and separate acceptance review are
**pending supervisor execution**. No original execution or fidelity pass is claimed.

## Failure, dependency and production change

The human still saw climbers through walls after the accepted actor-depth-001
([report](actor-depth-001.md), accepted verification
`build/agent-runs/20261114T014716-38924-40c75173/verification.log`). The next
presentation dependency is correcting its unconditional contact allowance.

Production path: `runApplicationLeg::renderFrame -> TubeScene::syncTerrain ->
scheduleSceneFrame` (primary scenery, legacy, tube geometry, fluids, secondary
scenery) `-> drawInjectedWalkers -> prepareInjectedWalker -> ActorPose / sprite
selection / actorSpritePlacement / gravity placement / drawActorSprite ->
clipActorToScene -> compositeActorOverlay`. The completed frame's physical scene
Z is passed to drawing, `injectedWalkerScreenBoxes` and `drawHoveredWalker`.
The last two use the same preparation, so a fully covered climber has neither a
pick box nor highlighted fragments. Partly exposed sprites retain their existing
rectangle and remain eligible for picking, subject to ordinary skill exclusions.

`pose.basis` points in the facing direction, into the climbing wall. A positive
`dot(pose.basis, camera.eyeWorld - pose.modelPosition)` puts the camera across that
face. For the `climb` strip only, that side now receives **zero extra contact
allowance** instead of 4 physical world units. It still receives the traced
normalized 0.0003 bias. The exposed side and every other strip keep the accepted
allowance. Scene depth, sprite placement, lane, alpha, geometry, simulation,
skills, release, animation and timing are unchanged. No widely included header
was edited.

This is explicitly **provisional clone presentation**, not recovered original
device behavior. In 640-pixel views, smaller Z wins and the comparison is:

```
before / exposed contact: placement.depth * 640 / 0.3 - 4
                         = world view Z - 0.64 - 4
behind climbing face:    placement.depth * 640 / 0.3
                         = world view Z - 0.64
```

The prior allowance moved a billboard in front of a nearby wall even when the
wall was visibly between actor and camera. Removing it on that side restores
per-pixel coverage; it does not unconditionally hide the whole actor. Silhouette
views 1/2 retain the genuinely exposed left-hand fragments. More distant walls
and foreground scenery still reject pixels through the same scene depth test.

## Installed routes and retained images

Both routes load real installed data, release actor 0 naturally, spend real
Climber stock through `World::assign`, and use the production animation/pose
adapters. No simulation state, position or terrain is injected. These are direct
installed-level routes, not frontend unlocks or wins.

| Route | Frozen natural snapshot | Terrain sample / strip |
| --- | --- | --- |
| `MM_CLIMB_002`, Just Climb | 10,000 ms, (64,58), facing right, Climber | (64,37,0.75), climb entry 0 |
| `MM_FLOAT_CLIMB2` | 5,000 ms, (243,9), facing right, Climber | (243,86,0.75), climb entry 0 |

Each snapshot has **48 exact evenly spaced views**, `yawUnits = sample.x +
view * 256/48` (7.5 degrees apart), both tube-only and with the complete scenery
stage ordering. Other camera operands are canonical `CameraParams` defaults.
Angles below are the yaw parameter before its fixed camera yaw offset.

| Complete-scene example | Yaw units / degrees (unwrapped) | Before actor pixels | After |
| --- | --- | --- | --- |
| Just Climb, view 4, wall covers actor | 85.33333333333333 / 120 | 102, pick box | 0, no pick box |
| Just Climb, view 8, wall covers actor | 106.66666666666666 / 150 | 13, pick box | 0, no pick box |
| Just Climb, view 40, exposed face | 277.33333333333337 / 390 | 108 | 108, entire raw sprite |
| Float/Climb, view 4, wall covers actor | 264.33333333333331 / 371.71875 | 101, pick box | 0, no pick box |
| Float/Climb, view 8, wall covers actor | 285.66666666666669 / 401.71875 | 7, pick box | 0, no pick box |
| Float/Climb, view 40, exposed face | 456.33333333333337 / 641.71875 | 99 | 99, entire raw sprite |

Full retained view-4 frames:
[Just Climb before](../../build/climber-occlusion-001/evidence/before/climber-full-scene/view-4.png) /
[after](../../build/climber-occlusion-001/evidence/after/climber-full-scene/view-4.png);
[Float/Climb before](../../build/climber-occlusion-001/evidence/before/climber2-full-scene/view-4.png) /
[after](../../build/climber-occlusion-001/evidence/after/climber2-full-scene/view-4.png).
Exposed view-40 frames:
[Just Climb](../../build/climber-occlusion-001/evidence/after/climber-full-scene/view-40.png),
[Float/Climb](../../build/climber-occlusion-001/evidence/after/climber2-full-scene/view-40.png).

I reviewed **all 48 before/after complete-scene views on both levels** using
`review-page-0.png` through `review-page-3.png` beside those frames. Each page
shows twelve consecutive views with numbered, nearest-neighbour enlarged actor
crops. `review-12-views.png` also retains the strided overview. In both levels
views 3-9 lose the through-wall sprite; views 1/2 keep only the portion outside
the wall silhouette. Exposed views retain the whole sprite. At other angles the
cylinder, other walls and hatches still cover it; e.g. the Float/Climb hatch
partly covers views 34/35. Tube-only images separate that scenery from terrain.

The per-pixel trace records unmodified world view Z, normalized bias, chosen
extra allowance, actual comparison Z, scene Z, framebuffer coordinates and
whether the actor changed the pixel. Representative wall-covered view-4 pixels:

| Level / pixel | World Z | Stored scene Z | Before comparison Z | After comparison Z |
| --- | --- | --- | --- | --- |
| Just Climb / (196,277) | 336.33320262815107 | 333.05802335766532 | 331.69320262815108, drawn | 335.69320262815108, rejected |
| Float/Climb / (191,62) | 326.94908140944398 | 323.64324959612213 | 322.30908140944399, drawn | 326.30908140944399, rejected |

Exact rows: [before pixels](../../build/climber-occlusion-001/evidence/before/pixels.csv),
[after pixels](../../build/climber-occlusion-001/evidence/after/pixels.csv).
The adjacent `rotation.csv` records all angles, counts, wall-side operands,
picking and per-pixel expected-image comparisons. The natural snapshots are
identical before/after (`state.txt`). Independent categorical real-wall
assertions require view 4 to have **zero actor pixels and no pick box**, and
view 40 to equal the entire unmasked sprite. They supplement the general
unmasked-sprite/scene-Z comparison instead of deriving those two requirements
from the new allowance decision.

## Public application and regressions

The paired application driver runs `runApplication --level ID --sim --native
--camera camera.json --frames 250 --no-audio` with a test-owned 0.1-second frame
clock and private `FileSaveSink`. Public SDL key 1, pointer motion and click
assign Climber; P pauses after five climbed cells. Right drag then requests 48
views around a full turn, ending by SDL quit. Integer mouse counts limit these
public angles to within **0.21 yaw units** of the requested grid; the snapshot
rotations above supply the exact evenly spaced grid. No controller was changed.

Just Climb runs 153 frames, Float/Climb 103. Their world/clock/camera/stock traces
match exactly before/after. Each captures 48 full production images with HUD,
animated panel, scenery and all living actors. The covered actor is absent from
the application's displayed/pick-box list; the exposed actor is present. With
Float/Climb's actual Floater stock selected through key 2, public hover selects
the exposed climber and cannot select the covered one. Just Climb has no other
skill stock: its existing Climber capability exclusion correctly prevents
selecting a climber for Climber again. Those rules are unchanged.

[Application before](../../build/climber-occlusion-001/evidence/app-before/) /
[after](../../build/climber-occlusion-001/evidence/app-after/) contain per-level
`simulation.txt`, `rotation.csv`, view images, hover images and summaries.
All **128 full application frames containing no climber** are PNG-byte-identical.
All **480 paired actor-free scene images** are byte-identical. Across the five
snapshot routes, both rendering contexts and 48 views there are **480 cases**:
after has zero leaked/missing pixels against the stated provisional policy;
36 climber views change. All **288 non-climber frames** (ledge walker, pillar
walker, dug tunnel) are PNG-byte-identical, preserving the earlier ledge/contact
and excavation coverage. The tunnel still has its 12 naturally removed cells.
No-climber simulation/rendering behavior is not redefined to obtain equality.

The installed save and normal LocalAppData clone save are presence/full-byte
checked unchanged. Snapshot tests do not open a save store. Application tests
use only `build/climber-occlusion-001/app-{before,after}/LEVEL/SavedGame.dat`;
those private saves also remain unchanged. No import, progress reset or normal
save write occurred.

## Static evidence, verification and preservation

Registered x86 reference: `Lemmings Revolution/Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Retained research [A6d16](phase-a6-actor-tube-pixel-z.md),
[actor material](phase-a4-actor-render-state.md) and actor-depth-001 distinguish
software submission from effective hardware depth. I reread retained Ghidra
pseudocode and instructions: `0x43CC94/0x43CC9A` gate on origin Z,
`0x43CCB5` subtracts `[0x4B1498]`, `0x43CCCA` calls the depth-slot setup and
`0x43CD4E` submits through `0x47A75D`. The registered static CTest read confirms
`0x4B1498` / file offset `0xB1498` contains `613255302aa9333f`, binary64 0.0003.
Packets copied unchanged under `build/climber-occlusion-001/static/` retain
instruction evidence `ev_9a8209aba409c9745a27529fbccfc174c42f9a83172b48c6c3544c73701ba1e3`
and pseudocode `ev_0fc271b435b10a844d45e665b97bf248628b7f01ab9bd21a64416ae76d58f60f`.
No new REA session or original observation was run.

The existing `test_actor_depth.cpp` comparison formerly required the 4-unit
allowance even behind the climbing wall: that expectation encodes U1. Only its
current climber expectation now removes the extra allowance on the wall side;
the historical before branch, other states, assertions, counts and timeouts
remain. New tests retain that old behavior in the authentic task-entry renderer
and assert the corrected covered/exposed cases on both levels.

Registered UCRT64 GCC/CMake/Ninja build, **at most 8 parallel jobs**, `-O2 -g`,
assertions on. [Final focused CTest log](../../build/climber-occlusion-001/tests-handoff.log):
**19/19 pass in 150.32 s**, four test jobs. This includes all five new registered
cases, the earlier actor-depth/occlusion render and application routes,
Climber animation, walker lane regressions, both baseline identities and the
read-only reference measurement. Build log: `build-handoff.log`. No full main or
recovery verifier was run or restarted; **full verification remains pending**.

Development logs remain separate. `test-app.log` failed because the new driver
initially expected public Climber selection to hover an already-capable climber.
The retained `RuntimePick.cpp` exclusion explains it. The corrected driver
uses real Floater stock on the second level and asserts the exclusion on Just
Climb; `test-app-corrected.log` and `tests-handoff.log` pass. No production pick
rule or existing assertion was weakened for that driver correction.

Authentic entry renderer SHA-256:
`1e32aab61974b641e08664011c10fe966f1678afe62475e4040aa1f7e2a1dbf5`.
`before-source/` is frozen from this task's entry, with the entry source backup,
361-file hash manifest, status/diff and HEAD main under the task directory.
`climber_occlusion_identity` pins the frozen renderer, header, main and actor
header. Baselines are never refreshed from the implementation. A checkout
without ignored baselines still registers the production checks and omits the
paired before/identity checks explicitly. The final `evidence/` archive is
pinned by `evidence-sha256.json`; supervisor regeneration uses separate live
output directories. `task-only.patch` and `preservation-check.json` isolate the
three existing task edits from all incoming changes. The active parent lease
was retained; no protected controls were edited. CIM process enumeration was
denied, so the entry check used `Get-Process`; no clone/build/test was present.

Existing changed files: `src/app/TubeScene.cpp`,
`src/__test__/test_actor_depth.cpp`, `CMakeLists.txt` (one include).
New files: `src/__test__/test_climber_occlusion.cpp`,
`src/__test__/test_climber_occlusion_app.cpp`, `cmake/climber_occlusion.cmake`,
`cmake/climber_occlusion_identity.cmake`, and this report. Task text uses LF.

Remaining unknowns: original effective device depth state and exact contact/lane
policy; untested levels, zooms, elevations, left-facing and inverted climbing
contacts. The exposed-side allowance remains provisional, with the pre-existing
billboard/contact limitations. This bounded rotation evidence is not universal
geometry fidelity or human playability. An approved original observation of a
paused climber from both sides of its wall, retaining effective vertex Z/device
state across rotation, would settle the original policy. No park, historical
gate or full-project completion criterion is reopened or marked passed.
No original game/oracle, debugger, hook, capture, installation tool, subagent,
detached job, commit/push/reset/clean/stash or control edit was performed.

Handoff: U1 climbers lose the contact allowance behind their wall; retained two-level images/depth traces and 19 focused checks pass; needs_review.
Next action: supervisor full registered main/recovery verification and independent acceptance review, then a human Climber rotation playtest.
