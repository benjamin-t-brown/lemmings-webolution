# builder-brick-anim-001: carried builder brick

Status: **needs_review**; supervisor smoke verification pending. Playable v1,
implementation-plan work-order step 3, [playtest 9 Y14](../playtest/2026-11-16-playtest-9.md).
Difficulty routine; recommended Sonnet; assigned Codex override used.
Before: the human saw the pull-from-pack animation without a brick until the
bridge appeared. The next dependency is the missing carried mesh on **Just
Build, MM_BUILD (node 4)**, also used wherever Builder is available.

Production path: `runApplicationLeg -> LemmingAnimTracker::advance ->
injectedWalkersFromWorld -> builderPresentationPhase -> renderFrame ->
TubeScene::syncTerrain -> drawInjectedWalkers -> prepareInjectedWalker ->
drawBuilderBrick -> drawActorSprite -> clipActorToScene -> composition`.
The adapter reads the existing builder frame, bricks remaining, rules and World
clock; it does not mutate simulation. It samples the build SPR and carried mesh
from one phase stretched between the actual hybrid placement ticks. Following
each placement, the body finishes its recovery frames before extracting the
next brick. This removes the old independent animation clock's accumulating
drift from the accepted 5 Hz placement schedule. Other clips keep their clocks.
Pause holds the phase; grouped fast-forward updates use their final confirmed
state. Builder reassignment resets the existing frame/counter, restarting it.

`World::stepBuilder`, terrain writes, actor movement, brick count, sounds and
all simulation clocks are unchanged. The mesh stops in the same update that
creates the solid cells; `syncTerrain` renders those cells on that frame.
The final mesh footprint is exactly the clone's forthcoming cells: right
`[x+1,x+3]`, left `[x-2,x]`, collision-grid row `[y,y+1]`, both shell layers.
The target uses simulation coordinates while extraction follows the displayed
actor, accommodating the existing motion interpolation without moving terrain.

The separate mesh uses the target block's installed OUT tile and cell UV
footprint. It is rasterised into actor scratch before the body, leaving hands
in front. Their union passes through the **same ActorOcclusionPlane**, completed
scene depth and retained pipe depth. Neither body nor brick writes scene depth.
Highlight includes the visible attachment; picking keeps its prior body box.
The pack anchor comes from the selected directional SPR's red-pack top edge,
with the body's horizontal/vertical mirrors. A body-relative fallback handles
art without red. This correction fades out on delivery to the map.

Static evidence: registered repository-local `Lemmings Revolution/Lemmings
Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Read accepted [actor animation](phase-c1p-spr-sheet-read.md),
[builder reassignment](builder-shrug-assign-001.md),
[actor occlusion](basher-occlusion-001.md), [skill debris](bash-particles-001.md)
and [hatch input](hatch-click-rate-001.md) evidence and queue reviews first.
New Ghidra 12.1.4 pseudocode and complete instruction packets, including Evidence
IDs and profile digest, are retained in
[static/evidence.json](../../build/builder-brick-anim-001/static/evidence.json).
The reverse-engineer-anything skill was used only for targeted static lookups;
REA was closed without snapshot arguments. No original execution occurred.

| Static finding | Instruction cross-check |
| --- | --- |
| A separate model is submitted, not a brick image in the build SPR. | Builder draw slot `410486` calls the object at state `+218`; `410413` points it at state `+40`. Mesh setup `410C0E` allocates 12 positions, six normals and two groups, then constructs faces using the `4B084C` index table. |
| Visibility is build time 0.12 through 0.78 seconds. | `410328..410334` multiplies 2 by 0.06; `410385..410391` multiplies 13 by 0.06; comparisons `4103A8..4103D5` gate the model. Constants and file offsets are in [constants.json](../../build/builder-brick-anim-001/static/constants.json). |
| The smaller shape expands toward the placed terrain shape. | `410445..41045C` computes twice the elapsed fraction; `410703..410710` interpolates vertex arrays using half that parameter. `4115D9..4115E6` divides the centred vertices by 3; subsequent copies fold selected small-shape vertices. |
| The model moves forward/down, with facing and gravity-dependent rotations. | Offset helper `4104B6`, instructions `4104E8..41057A`: Y is +/-5 until travel 0.5, then approaches zero; facing changes X. `4105B0` blends radial position toward 0.5, composes two rotations and installs its transform. This is a procedural attachment, not serialized hand sockets per SPR entry. |
| Placement and advance are separate callbacks. | Constructor `40F9A8/40F9B2` stores percent 81 / `41011C`; `40FA06/40FA10` stores percent 90 / `40FBCC`. `41011C` writes two solid cells; `410E78` derives model vertices and UV/material data from the cylinder and terrain block. |

The build strip in **all three installed ordinary/water/acid Hi sheets** has
16 temporal entries and rate 1, checked through production asset readers.
Thus extraction starts at entry 2 and finishes at entry 13 (0.78 / 0.96), near
the native percent-81 placement. Exact native within-update ordering is not
transplanted into the clone. The accepted clone callback owns solidity.

**Provisional choices/limits:** uniformly expanding eight-vertex cuboid instead
of the original folded 12-vertex model; red-pack pixel anchoring and fallback;
screen correction at the clone's fixed sprite size; simplified forward/down
travel instead of the full rotation chain; single OUT material/UV mapping and
actor tint instead of both native groups. The ramp starts at 1/3 scale and ends
at full size, following the traced smaller/placed shapes. Stretching the body
strip and carried phase to the existing hybrid tick boundaries is presentation,
not original timing fidelity. Original device depth policy remains unresolved;
the accepted clone upright-plane policy is preserved. Synthetic poses show the
attachment, not natural gameplay or an original comparison. Human confirmation
of hands, delivery and size at rotated views/zooms remains the acceptance step.

Checks: registered UCRT64 GCC 16.1.0/CMake/Ninja, eight build jobs, existing
`-O2 -g` with assertions: [final-build.log](../../build/builder-brick-anim-001/final-build.log).
Two focused legacy checks passed, **0.41 seconds wall**:
[focused.log](../../build/builder-brick-anim-001/focused.log),
[JUnit](../../build/builder-brick-anim-001/focused.xml).
New `builder_brick` checks all twelve actual terrain handoffs in both seam
directions at 1 ms sampling, stable paused reads, grouped four-pass sampling,
installed strip data, visible mesh, unchanged picking/depth and shared
foreground/pipe occlusion. Existing `actor_occlusion_plane` also passes.
The new test stays legacy; no smoke assertion, timeout or registration changed.
No test was removed, and no replay fixture, frozen build or gameplay route added.
Supplemental rendered poses/crops under [poses/](../../build/builder-brick-anim-001/poses/)
were inspected: the brick grows at the pack and lowers to the delivery footprint
in both facings. These are injected poses using MM_BUILD assets, not playtests.

Ran the final built clone with `--game-dir "Lemmings Revolution" --replay <file>`
for **all 15 promoted recordings**; each returned **exit 0, PASS, won=1,
completed=1**. Exact paths, commands and individual logs:
[final-replays.json](../../build/builder-brick-anim-001/final-replays.json).

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

Replays cover simulation compatibility, not rendered attachment appearance.
The two retired playtest-9 recordings remain untouched/excluded per their README.
Mandatory smoke is pending supervisor execution; no full/gate suite was duplicated.

Changed: `src/app/TubeScene.{cpp,h}`, new `src/app/BuilderBrick.h`, new
`src/__test__/test_builder_brick.cpp`, six CMake registration lines, this report.
Incoming changes are preserved in [entry/](../../build/builder-brick-anim-001/entry/);
[task-only.patch](../../build/builder-brick-anim-001/task-only.patch) separates
the task changes. Task text is LF and scoped diff checks pass. Parent supervisor
PID 46016 retains RepositoryLease; no protected controls were edited. CIM process
metadata was denied; Get-Process found no clone/build/test runner before edits.
No subagents, concurrent writers, background jobs, commits or pushes. Runs use
private LocalAppData `build/builder-brick-anim-001/appdata`; replays/units do not
construct SaveStore. Installed and normal user saves were not opened or changed.

Handoff: carried builder mesh and synchronised hands are implemented; two focused legacy checks and all 15 promoted replays pass; needs_review.
Next: supervisor smoke and independent review, then human playtest; exact native folded geometry/rotations and appearance remain provisional.

What to check: **Just Build (MM_BUILD)**, assign Builder in each facing and watch
several bricks at normal speed and fast-forward. Each brick should emerge and
grow from the pack with the hands, lower into the new bridge, and give way to
solid terrain with no missing/doubled brick. Pause holds it; rotate the cylinder
to check that foreground walls/bridge faces cover brick and carrier together.
