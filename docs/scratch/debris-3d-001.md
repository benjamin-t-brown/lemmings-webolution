# debris-3d-001

**needs_review; performance revision complete, supervisor smoke pending for this
handoff.** Prior implementation smoke passed in the exact run cited below.
Playable v1, implementation-plan
playtest-ledger work order, [playtest 11 A6](../playtest/2026-11-16-playtest-11.md).
Before: the user saw flat rectangles while bashing, mining and digging. Visible
solid, rotating pieces are the next presentation dependency, including **Sergeant
Bash (node 20, MM_E1_4SKILLS)** and the installed **Just Bash / Just Mine / Just
Dig** routes. Difficulty routine; recommended Sonnet; assigned Codex override used.

## Static evidence and correction

Started from accepted [bash particles](bash-particles-001.md),
[death particles](death-particles-001.md), [two-hit bash](basher-two-hit-001.md),
[hatch input](hatch-click-rate-001.md), their queue acceptance and verification
records, and retained recovery terrain-blast evidence. Used the
reverse-engineer-anything skill and static REA/Ghidra **12.1.4**, only on
`Lemmings Revolution/Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Complete returned evidence, IDs, pseudocode and instruction packets are in
[static/evidence.json](../../build/debris-3d-001/static/evidence.json), profile
`3c06738e45e144d5f073edbd3244c6bff46222f389525fe659bc92dca2ccdece`.
REA was closed without snapshot arguments. No original execution occurred.

| Static observation | Instruction cross-check |
| --- | --- |
| Shared constructor `40C960` configures **12 slots**, not four. `40DBF0(...,4)` selects a velocity callback. | `40C9B5 PUSH C`, call `40DAF0` at `40C9BA`; `40DBFD` indexes callback table `4C428C`, `40DC04` stores `+744`; `40D572` calls it and copies its vector to velocity. |
| `495F97` builds slots 0..7 with **8 vertices / 6 quad faces**, slots 8..11 with **4 vertices / 1 face**. | Counts `495FBD..495FC7`; branch `495FFD..49601A`; allocation `496040/496058`; face-index writes `496212..496269`. |
| Pieces use the cut cell's terrain material and one-cell UV rectangle; vertex positions have random perturbation. | UV calls `4960AA..4960E0`; material lookup `4960FB`, binding `496171..4961A2`; vertex jitter calls `496622/49665B/496694`. |
| `495A40` supplies a one-grid-unit X/Y footprint with depth layers stepping by 1/14 slab, including random skipped layers; `495F97` recentres each piece. | Cylinder interpolation calls `495C1A/495CBC/495D52/495DD9`; centroid divide `4966DE` and vertex subtraction `49673F`. This is not a fixed world-unit cube size. |
| Two angles advance by dt times individual rates, then compose a mesh transform. | Angle stores `495E80/495EAA`; random rate stores `4967C6/4967DB`; rotation/transform calls `495EFC/495F2C/495F4D`, copy at `495F75`. |
| Base life is computed as `1 + 1.75 * inputY / 40BAA0(world)`, with zero life variation here. Remaining life decreases with dt; the final 30% shrinks. | Constructor `40C9BF..40C9F9`, life copy `40D501..40D558`, decrement `40D637`, shrink call `40D712`. No native time/unit mapping is transplanted. |

**Correction:** the four-fragment interpretation of `40DBF0` in the accepted
bash/death reports and retained terrain-blast callback naming was unsupported.
This report supersedes that interpretation only; historical reports/recovery
remain intact. Four fragments **per removed clone cell** remain an explicitly
provisional density, not original count fidelity. Slot scheduling, exact native
random motion, shading/device depth state and original visible lifetime remain
unverified; these are static observations, not an original comparison.

## Production path and limits

Unchanged `World::step -> stepBasher/stepMiner/stepDigger -> clearSkillBox ->
TerrainCutObserver`, installed by `runApplicationLeg`, calls
`TubeScene::emitTerrainDebris`. The emitter now stores the cell material and a
quarter-cell UV patch alongside each existing particle, plus deterministic
initial angles/rates. It retains four pieces/cell, the 4096 bound, original
clone arcs, 1.6..1.9 s lives, gravity direction and actual-removal filtering.
No simulation, RNG, timing, observer or save behavior changes.

The existing gameplay-clock callback advances presentation; pause freezes both
translation and rotation, and fast-forward advances both. `renderFrame` finishes
scenery/terrain/fluid and actors, then calls `drawDeathParticles`. Skill chunks
use `DeathParticle::chunkOffsets`: rigid cuboids rotating about two local axes,
transformed into model-space tangent/up/radial coordinates at the cylinder pose.
Eight vertices and twelve triangles replace each rectangle. Installed OUT tile
patches wrap through the existing bilinear sampler; missing artwork retains the
earth-colour fallback. Face normals drive provisional ambient/directional shade.

`Raster::triangle` accepts an optional actor owner: chunk faces pass the existing
strict scenery-depth comparison, then share `ActorDepth` with bodies, carried
bricks and other particles. Faces retain separate physical depths, so nearer
faces/pieces/actors hide farther ones. The actor path **does not write scenery
depth**; ordinary triangle calls retain their prior depth-write behavior.
No extra depth bias or contact allowance is introduced. The existing body/death
sprite policies remain intact; original device depth fidelity stays unresolved.
No whole-scene scan or per-chunk framebuffer allocation is added; empty particle
frames retain their early return.

**Provisional:** cuboid dimensions (half extents size, 0.65*size, 0.55*size;
size 0.65/0.85 model units), rigid two-axis rotation, angular speeds 2.5..5.0 rad/s,
fixed lighting and UV quadrant choice. The native irregular layered meshes,
mixed solid/flat slots, shrink envelope and native emission density are not
reproduced. Visible shape/scale/occlusion still need the user's next playtest;
component tests and headless replays do not establish that appearance.

## Rendered performance revision (2026-11-17)

The first review accepted implementation/evidence/replays/smoke but requested
the missing rendered comparison with simultaneous cutters. **This revision
changes no production source or tests.** A foreground benchmark links the
current production objects; its flat comparator copies the emitted particles
and clears only `terrainChunk`, selecting the retained pre-task rectangle
branch of `drawDeathParticles`. The other mode uses the production 3D branch.
This compares before/after drawing work in one binary, not a historical
pre-task executable; there is no frozen-source build or new gameplay route.

Eight artificially placed cutters advance together for five ordinary World
ticks on each installed level's unpainted terrain. All eight actually cut via
`World -> clearSkillBox -> TerrainCutObserver -> emitTerrainDebris`; the final
snapshot has 96/188/96/88 live pieces respectively. Exact worker positions,
events and removed cells are in [workload.txt](../../build/debris-3d-001/performance/workload.txt).
These are synthetic performance workloads, not human gameplay evidence.

Settings: Ryzen 9 9950X3D, Windows build 26200, registered UCRT64 GCC 16.1.0,
C++23 `-O2 -g` with assertions; 640x480, high detail terrain textures, bilinear
wrap, production route geometry and scenery stages, actor and scenery depth
enabled, eight submitted actors, both zooms. Camera yaw faces the first worker;
other CameraParams defaults. Each case has 24 warm-up pairs then **180 measured
pairs**, alternating flat/3D order, with matching particle ages sweeping 290 ms.
Frame time includes clear, complete CPU scenery/tube/actor rendering and the
particle pass. It excludes simulation, terrain rebuild, HUD, SDL presentation,
loading and measurement-output I/O; this is not end-to-end application FPS.

| Installed level | Zoom | Pieces | Mean frame ms, flat -> 3D | Increase | Mean debris ms, flat -> 3D |
| --- | ---: | ---: | ---: | ---: | ---: |
| Just Bash, MM_BASH_002 | 0 | 96 | 62.003 -> 62.519 | 0.83% | 0.199 -> 0.496 |
| Just Bash, MM_BASH_002 | 1 | 96 | 64.902 -> 65.385 | 0.74% | 0.201 -> 0.842 |
| Just Mine, MM_MINE | 0 | 188 | 65.355 -> 66.017 | 1.01% | 0.206 -> 0.630 |
| Just Mine, MM_MINE | 1 | 188 | 54.090 -> 54.638 | 1.01% | 0.192 -> 0.734 |
| Just Dig, MM_DIG_000 | 0 | 96 | 68.807 -> 69.013 | 0.30% | 0.188 -> 0.499 |
| Just Dig, MM_DIG_000 | 1 | 96 | 67.138 -> 67.750 | 0.91% | 0.200 -> 0.841 |
| Sergeant Bash, MM_E1_4SKILLS | 0 | 88 | 61.776 -> 62.107 | 0.54% | 0.183 -> 0.390 |
| Sergeant Bash, MM_E1_4SKILLS | 1 | 88 | 62.461 -> 62.892 | 0.69% | 0.187 -> 0.619 |

Mean scene-frame increase is **0.206..0.662 ms (0.30..1.02%)**; p95 changes
range from -0.506 to +2.231 ms. The added mean particle cost is 0.207..0.641 ms.
No material regression is demonstrated in these workloads, so no optimization
or timing change is needed for this revision. Existing 54..69 ms scene costs
remain a separate performance issue; these results do not establish 60 fps or
the unmeasured 4096-particle cap. Every measured 3D frame submits 1056..2256
chunk triangles and changes 459..7628 pixels against the completed scene,
confirming actual raster work. A source-built Just Bash framebuffer was also
inspected; user appearance confirmation remains pending.

Reproduction, compiler response, benchmark source and measurement limitations:
[performance/README.md](../../build/debris-3d-001/performance/README.md).
Raw **2880 measured frames**, means/medians/p95/paired deltas and clone PNGs:
[samples.csv](../../build/debris-3d-001/performance/samples.csv),
[summary.json](../../build/debris-3d-001/performance/summary.json),
[run.log](../../build/debris-3d-001/performance/run.log).
Build and benchmark exit 0; no concurrent runner, original execution or save
access. Private LocalAppData is `build/debris-3d-001/performance/appdata`.
Only this report is revised outside ignored build/; its incoming version is
backed up as [report-before.md](../../build/debris-3d-001/performance/report-before.md).

## Checks and preservation

Registered UCRT64 GCC 16.1.0/CMake/Ninja build, eight jobs, passed:
[build.log](../../build/debris-3d-001/build.log),
[unit rebuild](../../build/debris-3d-001/build-unit.log).
Focused **3/3 PASS, 0.07 s**:
[log](../../build/debris-3d-001/focused.log),
[JUnit](../../build/debris-3d-001/focused.xml). Existing terrain/death units pass;
the new fast **legacy** unit guards rigid distances through rotation, stable
paused pose, texture variation, near/far face and actor ordering, unchanged
scenery depth, and ordinary triangle depth writes. No existing assertion,
timeout, replay or smoke membership was edited; no test was removed.

Ran `webolution --game-dir "Lemmings Revolution" --replay <file>` on **all 17
promoted recordings**: every one returned exit 0, **PASS, won=1, completed=1**.
Exact filenames, commands, results and individual logs:
[replays.json](../../build/debris-3d-001/replays.json).

| Level | Saved/required |
| --- | --- |
| MM_BASH_002 | 10/10 |
| MM_BASH_MINE_WLR1 | 20/10 |
| MM_BLOCK_001_A | 24/15 |
| MM_BLOCK_BOMB_A | 12/10 |
| MM_BOMB | 17/10 |
| MM_BUILD | 20/10 |
| MM_CLIMB_002 | 10/1 |
| MM_DIG_000 | 19/10 |
| MM_FLOAT | 10/1 |
| MM_MINE | 20/10 |
| MM_W_012 | 16/10 |
| MM_E1_3_SKILLS | 10/5 |
| M06_KF_1 | 91/50 |
| MM_E1_4SKILLS | 20/10 |
| MM_E2_4SKILLS | 14/10 |
| MM_BASH_DIG_001_A | 20/10 |
| MM_BUILD_BLOCK_A | 15/10 |

No recording diverged or needs retirement. Those prior **17/17** results remain
the replay evidence for this source-unchanged performance revision; they were
not rerun. Replays check simulation/file
compatibility; their headless World does not install the presentation observer.
No new scripted route, replay fixture, frozen build or original capture was
added. The prior supervisor run
[20261117T060233-15712-483aefaf/verification.log](../../build/agent-runs/20261117T060233-15712-483aefaf/verification.log)
passed **95/95 main smoke and 84/84 recovery**, with `git diff --check`.
Mandatory verification for this report revision belongs to the supervisor after
worker exit and was not duplicated. Human confirmation and full original
fidelity remain pending.

Changed: `src/app/{TubeScene.cpp,TubeScene.h,DeathParticles.h,Raster.cpp,Raster.h}`,
`src/__test__/test_debris_3d.cpp`, five CMake lines and this report. Incoming edits
are preserved under [entry/](../../build/debris-3d-001/entry/); the
[task-only patch](../../build/debris-3d-001/task-only.patch) separates this work.
Task text is LF and scoped diff checks pass. Supervisor PID **15712** holds the
active RepositoryLease; process checks found no clone/compiler/test runner
before edits. No queue/controller/verifier/permission/shared-instruction edits,
subagents, concurrent writers, background work, commits or pushes. Commands used
private LocalAppData `build/debris-3d-001/appdata`; these checks do not open a
SaveStore. Installed and normal user saves were not accessed.

Handoff: 3D skill chips retained; eight-cutter rendered comparison adds 0.30..1.02% mean frame time; prior 17 promoted wins and smoke pass; needs_review.
Next: supervisor smoke and independent review of this performance revision, then human playtest; exact original appearance remains provisional.

What to check: **Sergeant Bash (MM_E1_4SKILLS)** or **Just Bash (MM_BASH_002)**,
bash a wall: both punches throw small solid pieces with terrain texture, changing
silhouette and face shade as they tumble/fall. **Just Mine (MM_MINE)** and **Just
Dig (MM_DIG_000)** should show the same effect at successful cuts. Rotate the
cylinder: foreground terrain and nearer lemmings cover pieces; pause freezes
their pose, fast-forward advances them, and they disappear after about two seconds.
