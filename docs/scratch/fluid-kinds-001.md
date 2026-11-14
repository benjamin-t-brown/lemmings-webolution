# fluid-kinds-001: remaining fluid surfaces and water-to-land footing

Status: **needs_review**. Playable-v1 step: playtest 2 Q2/Q3 and sweep S3.
Difficulty: difficult; assigned recommendation Sonnet-class; this run uses the
authorized Codex override. Full registered main/recovery verification and
independent acceptance remain the supervisor's next actions.

Only source-built clone rendering and static REA ran. No original execution,
debugger, hooks, original capture, install tooling, subagents, queue/control edit,
commit or push. Prior edits, including walker-lane-001, were preserved.

## Level identification and an important correction to Q2

The best match for “third row, second down” is **node 8, MM_W_012,
“Introducing - Water Lemmings”**: the decoded indexed map has columns at
X=209,242,275; node 8 is second down the third column, centre **(275,203)**.
Node 7 above it is Just Dig; nodes 9/10 below are Block And Bomb / Bash and Mine.
The complete decoded map positions, titles, receiver kinds and hatch species
are in `build/fluid-kinds-001/{before,after}/census.txt`.

**This level has only kind-0 receivers, already admitted before this task.**
It is distinct from the sweep's 19 kind-1/kind-2 omissions. Its lower tubs really
are empty in the retained initial clone images, but their data says they start
drained: 16 receivers have request=1, low=0, current height=0 after the first
update. Only stream indices 8..10 (ids 12/11/10, X=120..144, bottom Y=59)
start raised to 22.5. This agrees with accepted fluid research sections 7.2–7.4
and the accepted implementation/assessment; filling the lower tubs at startup
would contradict that data.

Retained initial image:
`build/fluid-kinds-001/before/MM_W_012/initial-yaw-160.png`.
The same camera after actual contact with lever (183,60), ids 5/6 raised to 24.5:
`build/fluid-kinds-001/{before,after}/MM_W_012/bottom-tubs-after-lever-fixture.png`.
That is explicitly a **placed-contact fixture**, not a natural lever route;
the earlier accepted fluid-lever-assessment-001 supplies the natural route.
Both task-entry and modified renderers draw those raised tubs.

Thus Q2's empty lower tubs are reproduced and explained, **not claimed to be a
newly repaired kind-1/kind-2 omission on MM_W_012**. A human recheck should confirm
this level identification and whether the reported absence was before or after
the filling lever. If it was another map position or a filled tub, these data
do not close that separate observation. No new fluid has been invented there.

## 1. Remaining receiver kinds: static contract and implementation

Reference: repository-local `Lemmings Revolution/Lemmings Revolution.exe`,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Ghidra 12.1.4, project static profile; sessions closed without snapshots.
Full returned evidence is retained in `build/fluid-kinds-001/static/`.

Initializer **0x428848** writes table rows at **0x4BF6E8 + kind*0x48**;
**0x42B24D** consumes period at +0, amplitude +8 and lane count +0x38.
Both pseudocode and full instructions were read and retained. These are static
findings, not observed original frames.

| Kind | Period / address | Amplitude / address | Lanes / address | Alpha |
| --- | --- | --- | --- | --- |
| 0, water | 2.0 / 0x4BF6E8 | 0.5 / 0x4BF6F0 | 4 / 0x4BF720 | stored, unchanged |
| 1, lava | 8.0 / 0x4BF730 | 1.0 / 0x4BF738 | 16 / 0x4BF768 | stored, unchanged |
| 2, acid | 0.8 / 0x4BF778 | 0.25 / 0x4BF780 | 2 / 0x4BF7B0 | 191 + 64*cos(2*pi*t/4), clamped at 0 |

Kind-1 amplitude copies literal 1.0 from 0x4B1500; kind-2 amplitude divides
that literal by 4.0 at 0x4B02C8. Raw bytes for both literals are retained.
Kind-2 alpha base/amplitude/period are **0x4BF798=191**, **0x4BF7A0=64**,
**0x4BF7A8=4**; 0x42BF8D..0x42C1DC writes the six top corners and both
flat side triangles. No required render constant remains provisional.

Evidence IDs: initializer pseudocode
`ev_ffa03bffabc961a655c19fb5b48baa9016086f5851c8f6e3f8db7d84a2fa34e6`;
update pseudocode
`ev_df8513e73d30e17c2e488c49d852ccde68a3dd079d10a742bd014f7195981600`.
The matching `*-instructions.json` records cross-check table stores, stride,
phase wrapping, corner setters and alpha arithmetic.

Production path:
`loadRetailLevel -> World::updateSegment -> FluidGate::advanceWave` supplies
the kind's wrapped wave phase; `runApplicationLeg::renderFrame` supplies the
receiver snapshot; `BackdropScene::build` reads the stored kind and admits
the existing structurally checked base Fluid mesh; `renderStage(FluidGate) ->
applyFluidState -> renderFluid -> SceneRaster` draws it after the tube.
Kind 2's alpha phase is sampled from the **existing level clock**, modulo four
seconds: all installed receivers start it at zero and receive the same dt.
No additional simulation clock or FluidGate instance field was added.

The unchanged provisional **Less/write-on** depth policy and traced model
software bias apply to all three kinds. Wave/colour formulas, zero-height
normal inversion, geometry binding, submission order and kind-0 arithmetic
stay unchanged. Optional overlay/pipe/emitter omissions stay unchanged.

The authentic task-entry renderer diagnoses **19** affected levels, matching
the sweep exactly. The modified builder admits **every installed receiver
model in all 102 levels**, with **zero** remaining “receiver fluid not admitted”
diagnostics. This is an asset/admission census, not an all-level gameplay sweep.

| Installed image case | Kind | Before fluid pixels | After fluid pixels | Image under `build/fluid-kinds-001/after/` |
| --- | --- | ---: | ---: | --- |
| MM_ALL_8_001, One Way Ticket, node 22 | 2 | 0 | 5,060 | `MM_ALL_8_001/initial-yaw-128.png` |
| H02_JC_1, How do we get up there!, node 35 | 2 | 0 | 19,827 | `H02_JC_1/initial-yaw-160.png` |
| M16_MG_1, Lock In, node 43 | 1 | 0 | 16,437 | `M16_MG_1/initial-yaw-192.png` |
| M17_MT_1, Under and Up., node 44 | 1 | 0 | 21,961 | `M17_MT_1/initial-yaw-128.png` |

These counts are framebuffer pixels changed by the fluid stage with the full
production scenery/tube depth buffer present. Corresponding task-entry images
use identical filenames under `before/`. I inspected the green acid and yellow
lava in those frames, including foreground scenery over the fluid. Eight yaw
images per case are retained. All eight **MM_W_012 initial fluid/scenery frames
are byte-identical** before/after at matched phase (actors omitted to isolate
the fluid comparison).

## 2. What the data says about these lemmings and their fluid

MM_W_012 has one hatch at clone cell (132,7), serialized hatch **type 1**,
mapped by `RetailLevel.cpp` to `Entrance::sheetType=1` and inherited by released
lemmings. Its receivers are **kind 0, water**, with colour (0,0,192).
The loaded terrain has **528 water-zone cells (0x200), zero acid-zone cells
(0x4000), and zero water-hazard cells (0x40)**. The accepted reader clears the
drained lower receiver rectangles and retains the raised upper pool.

Retained evidence in `CollisionGrid.h`, `Terrain.h` and `World.cpp` distinguishes
0x40 drowning from 0x200/0x4000 zones; the accepted round-29 evidence maps a
water-zone/sheet-1 match to fall-landing exemption. Static references there
include 0x428A47 zone writers and 0x416433 landing. This supports water lemmings
walking on their matching upper water surface and then stepping onto land;
it does **not** justify making every visible liquid harmless or adding a new
wading offset. The natural clone route in this task stays a living Walker
through the crossing. Hazard/drowning/species/solidity rules were not changed.

Limits: the existing clone's separate zone/material representation and omitted
general fluid-object mismatch handler remain accepted limitations. This task
does not claim original drowning fidelity or validate other species in acid/lava.
The original initializer also exposes kind-2 height rate 1.0; this task preserves
the accepted height machine (whose rateFor(2) remains 0), rather than silently
expanding lever/zone simulation. The tested new surfaces start raised in data.

## 3. Standing-height cause, fix and natural crossing

The failure is presentation, **not a stale wet-state flag**. At X=143 the walker
has clone Y=14 (native feet Y=81); stepping right to X=144 sets clone Y=13
(native Y=82). Before the fix the actor's model Y stays **80** in both cases.

`TubeScene::build` incorrectly assigned the collision mask's **96** rows to
the pose coordinate-mesh height. `ActorPose.cpp` then clamped its upper row to
`96/16-1=5`, flattening the entire Y=80..96 band onto Y=80. The installed
coordinate mesh actually stores **112** and **7 rows**, so the upper row for
that band is row 6 at Y=96. This is measured for all 102 installed levels.

The new production helper uses the original's constant coordinate mesh,
not the terrain surface-index pool: **0x43D56C** reads world+0x18, clamps
using world+0x28, and interpolates radial depth, sector X and row Y.
**0x43E580** has row stride 0x600; **0x43EA20** selects layers at stride
0x300. Crucially **0x43D09A** reads alternating inner/outer FILE triples into
those separate runtime layers. `CylinderMesh` retains file interleaving, so
its existing `vertex()` accessor is correct here. Full instruction evidence
for each routine and the reader is retained, not just pseudocode.

Pose pseudocode evidence:
`ev_46801e9b385932e916a942fa3ff117a369ea3159c63bd5880912539c9a09e922`;
pose instruction evidence:
`ev_7a9f159b89013b953a8ef4a65362aa6fd68028510e960fc3d055a405e9d580d3`.
The reader/instruction files resolve the file/runtime layout distinction.

Production path:
`World -> LemmingMotionTracker -> injectedWalkersFromWorld ->
TubeScene::prepareInjectedWalker -> actorPoseFromCylinderSample ->
actorSpritePlacement -> drawActorSprite`. Picking uses the same prepared pose.
The existing motion buffer, radial lanes, simulation Y, timings and skill state
are preserved. The older surface-pool helper remains available for its existing
fixtures; live drawing now uses the traced coordinate-mesh helper.

Natural hatch release, without actor injection or skills: **111 matched frames**
from level time 4.2 to 6.4 seconds, stable yaw 120, in
`build/fluid-kinds-001/{before,after}/walk.csv`. Simulation positions, states,
draw samples and water-zone flags match exactly. Before, projected foot error
reaches **8.7031 pixels**; after, it is **0** against independent interpolation
of the decoded coordinate mesh. On land the expected model Y is **82**.

Image sequence: `MM_W_012/crossing-{4200,4400,4600,4800,5000,5200,5400,5600,5800,6000,6200,6400}.png`
under both `before/` and `after/`. I inspected the paired 5200-ms frames: before
the walker is buried in the top-right path edge; after its feet stand on that
edge. The ordinary `runApplication --level MM_W_012 --sim --native` path also
asserts projected footing after the natural crossing; see `app/frames.csv` and
`app/MM_W_012-frame-{60,100}.png`. Other ordinary app images cover kind 1 and 2.

## Checks, provenance and remaining work

New CTests: `fluid_kinds_identity`, `fluid_kinds_before`, `fluid_kinds_after`,
`fluid_kinds_app`, registered by `cmake/fluid_kinds.cmake`.
The component driver creates no save sink; app runs use a private sink under
`build/fluid-kinds-001/app/SavedGame.dat`. Complete installed/normal save bytes
and existence are compared before/after; all three summaries say unchanged.

Authentic task-entry sources are under `build/fluid-kinds-001/before-source/`;
the four files compiled by the baseline are byte-identical copies under
`frozen-render/src/app/`. The identity test pins the task-entry manifest SHA-256
`eac202e7e68477ceb4f9f3dd370e2c974a6e16603535200767fb6675a578ab85`.
Baseline render files are frozen, while support and height/actor simulation are
shared; no altered source was substituted for a pre-task baseline.

Development correction retained in `attempt-1-failed-ctest.log`: the first new
pose/test implementation confused runtime grouped layers with serialized file
interleaving. It failed visibility checks. The exact reader 0x43D09A resolved
that mistake; no existing assertion or timeout was changed. The corrected
focused run `focused-ctest-2.log` passed both component demonstrations.

Focused regression run `build/fluid-kinds-001/focused-final-ctest.log` passed
**9/9**: actorpose, actorcompose, fluidsurface_impl, actor_motion_units,
walker_lane_units and the four new cases. Registered GCC 16.1.0/UCRT64 tools,
`-O2 -g`, assertions enabled; no full suite was duplicated. A final focused rerun
after the remaining comment/build-portability edits is recorded separately below.
Final-content rerun: `build/fluid-kinds-001/final-content-ctest.log`, **9/9
passed in 46.77 seconds**. All production/build changes were compiled and run;
only this result paragraph and the rateFor preservation comment followed it.
`git diff --check` passes (only the pre-existing tasks.json CRLF notice).
`final-source-hashes.json` records this task's final file identities; all task
files use LF. Source-manifest comparison lists only the eight intended
production files and CMakeLists.txt as changed among the task-entry files.

Changed for this task: `src/app/ActorPose.{h,cpp}`, `BackdropScene.{h,cpp}`,
`TubeScene.cpp` (two pose hunks only), `main.cpp` (fluid snapshot only),
`src/sim/World.{h,cpp}` (render-wave period only), new
`src/__test__/test_fluid_kinds.cpp`, `cmake/fluid_kinds{,_identity}.cmake`,
the one new CMake include, and this report. Existing dirty files are retained.

Unknowns: original inherited fluid depth state remains unresolved under the
accepted provisional policy; optional secondary fluid effects and general
fluid hazard matching remain omitted. All-level playability, timing gates and
full original fidelity are unchanged. Human confirmation of the Q2 level/tub
state is still required before calling that specific observation closed.

Handoff: kind-1/kind-2 rendering and the evidenced Q3 pose fix are ready for full registered verification and independent review; task remains needs_review.
Next: supervisor runs main/recovery on the final content, then the reviewer assesses the explicit Q2 data correction and the retained before/after images.
