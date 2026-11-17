# frame-rate-001: level-play CPU rendering

Status: **needs_review**. Playable v1, [playtest 11 A5](../playtest/2026-11-16-playtest-11.md).
Difficulty difficult; recommended Opus; assigned Codex override. **Supervisor smoke/recovery
verification is pending after worker exit.** All twelve final means meet 16.6 ms:
**5.885-15.951 ms**, versus **35.275-41.979 ms** before. This is average
throughput, not locked 60 fps: Kriss Kross rotation p95/max is **18.315/19.625 ms**.

## Method and evidence

Evidence root: [build/frame-rate-001/](../../build/frame-rate-001/). Host rechecked through
hardware registry/OS API: Ryzen 9 9950X3D, 16 cores/32 logical processors, Windows build 26200.
Registered GCC 16.1.0; **Release -O3 -DNDEBUG C++23**, `build/cmake/gcc-release`, jobs 8.
Normal **1280x960 shown window**, 640x480 CPU raster, Windows video, reported 144 Hz display;
existing SDL **direct3d11 accelerated presenter, flags 0xa, vsync off**. No OpenGL scene renderer.

`measure.cpp`/`build_measure.py`/`measure.rsp` link current production objects and call
`runApplication` with `--direct-level --native --sim-ticks 180 --frames 240 --window-scale 2
--no-audio`. Before/after use identical owned 1/60-second simulation budgets. Existing
observer input supplies only 8 physical camera-drag pixels/frame or one fast-forward
plate click; no skill, World mutation, new gameplay/replay fixture or winning route.
Rest means stationary camera with gameplay running. Each mean uses **frames 30-229,
200 frames**; startup/loading/warmup excluded. Private LOCALAPPDATA is `build/frame-rate-001/
private-save`; direct entry and replay create no SaveStore. Normal/installed saves unused.

Kriss Kross has 30 authored lemmings; these samples average **18.3 submitted figures** at
rest/rotation and 15.6 in fast-forward, not 30 simultaneously visible actors. Existing
CSV `alive` means submitted figures, including exit tails. Audio is disabled in both runs.
Exclusive steady-clock scopes measure wall time, including raster worker dispatch/wait;
worker CPU times are not summed. CSV writing is outside the frame. Build includes clears,
mesh work, caches, copies/conversions/stats and texture decode; Actors includes separately
recorded footprint/brick scopes. Individual raster/SDL/footprint/brick columns remain in CSV.

`before-*.csv/log` and `before-summary.json` were produced **before optimization**, with
only added footprint/brick/particle timers and corrected actor-raster attribution.
`final-reviewed-*.csv/log` and `final-reviewed-summary.json` cover the first reviewed
implementation, before the surface-texture lifetime correction below. Earlier
`after-*`, repeat, `final` and `final-borrowed` runs are retained intermediate measurements.
`entry/`, `entry-status.txt`, `entry-diff.patch`, `head-main.cpp` preserve incoming edits.
No frozen-source build, new before/after fixture or identity check was added.

## Exclusive frame costs (ms)

Before:

| Level / mode | Sim | Build/copies | Raster | Actors | Particles | HUD | SDL | Other | Total |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Just Climb / rest | 0.015 | 5.937 | 27.315 | 2.553 | 0.000 | 0.789 | 0.535 | 0.053 | 37.197 |
| Just Climb / rotate | 0.017 | 6.097 | 29.374 | 2.005 | 0.000 | 0.748 | 0.543 | 0.059 | 38.843 |
| Just Climb / fast | 0.027 | 5.881 | 27.237 | 2.438 | 0.000 | 0.741 | 0.526 | 0.066 | 36.917 |
| Sergeant Bash (20) / rest | 0.019 | 5.902 | 28.483 | 1.959 | 0.000 | 0.738 | 0.523 | 0.056 | 37.679 |
| Sergeant Bash (20) / rotate | 0.019 | 5.928 | 29.432 | 2.412 | 0.000 | 0.746 | 0.531 | 0.060 | 39.129 |
| Sergeant Bash (20) / fast | 0.025 | 5.989 | 28.533 | 2.315 | 0.018 | 0.763 | 0.525 | 0.053 | 38.220 |
| Kriss Kross (19) / rest | 0.027 | 8.008 | 29.354 | 3.028 | 0.000 | 0.790 | 0.541 | 0.058 | 41.806 |
| Kriss Kross (19) / rotate | 0.028 | 8.039 | 28.815 | 3.719 | 0.000 | 0.794 | 0.525 | 0.059 | 41.979 |
| Kriss Kross (19) / fast | 0.049 | 8.003 | 29.084 | 2.648 | 0.000 | 0.789 | 0.538 | 0.052 | 41.163 |
| The Crowded House (21) / rest | 0.024 | 7.383 | 24.579 | 1.982 | 0.000 | 0.725 | 0.525 | 0.059 | 35.275 |
| The Crowded House (21) / rotate | 0.019 | 7.443 | 25.015 | 2.052 | 0.000 | 0.724 | 0.539 | 0.053 | 35.845 |
| The Crowded House (21) / fast | 0.041 | 7.524 | 24.642 | 2.086 | 0.000 | 0.736 | 0.543 | 0.058 | 35.630 |

After:

| Level / mode | Sim | Build/copies | Raster | Actors | Particles | HUD | SDL | Other | Total |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Just Climb / rest | 0.015 | 1.798 | 1.049 | 2.163 | 0.000 | 0.632 | 0.404 | 0.058 | 6.120 |
| Just Climb / rotate | 0.014 | 2.837 | 5.337 | 1.757 | 0.000 | 0.628 | 0.428 | 0.049 | 11.050 |
| Just Climb / fast | 0.015 | 1.799 | 1.060 | 2.169 | 0.000 | 0.626 | 0.388 | 0.052 | 6.109 |
| Sergeant Bash (20) / rest | 0.011 | 1.871 | 1.139 | 1.804 | 0.000 | 0.630 | 0.383 | 0.047 | 5.885 |
| Sergeant Bash (20) / rotate | 0.017 | 2.871 | 5.506 | 2.183 | 0.000 | 0.639 | 0.441 | 0.050 | 11.708 |
| Sergeant Bash (20) / fast | 0.020 | 1.835 | 1.133 | 2.041 | 0.014 | 0.624 | 0.387 | 0.047 | 6.101 |
| Kriss Kross (19) / rest | 0.024 | 3.397 | 2.717 | 2.685 | 0.000 | 0.681 | 0.427 | 0.058 | 9.989 |
| Kriss Kross (19) / rotate | 0.026 | 4.587 | 6.619 | 3.487 | 0.000 | 0.698 | 0.479 | 0.054 | 15.951 |
| Kriss Kross (19) / fast | 0.046 | 3.367 | 2.723 | 2.434 | 0.000 | 0.694 | 0.434 | 0.058 | 9.757 |
| The Crowded House (21) / rest | 0.017 | 2.965 | 1.084 | 1.875 | 0.000 | 0.632 | 0.478 | 0.051 | 7.103 |
| The Crowded House (21) / rotate | 0.019 | 3.902 | 4.907 | 1.831 | 0.000 | 0.633 | 0.445 | 0.054 | 11.791 |
| The Crowded House (21) / fast | 0.037 | 2.820 | 1.028 | 1.851 | 0.000 | 0.632 | 0.395 | 0.048 | 6.811 |

## Production path and preservation

`runApplication -> runApplicationLeg -> prepareScene/renderFrame -> scheduleSceneFrame`
submits Primary, LegacyUnattributed, tube, pipes/fluid, Secondary, actors/particles, HUD,
RGB upload/copy/present. Scene meshes/billboards use `rasterizeSceneTriangle`; terrain,
bricks and debris use `Raster::triangle`. The measured raster bound was the next dependency.

- Cache immutable paired-primary RGB/Z by **exact CameraBasis**. Cache opaque nearest tube
  RGB/Z by exact camera and terrain rebuild revision; invalidate on edits, rotation,
  zoom/elevation/intro or viewport size. Merge strictly nearer contiguous spans at the
  original tube slot against live scenery; equal Z keeps scenery. Flat/no-depth,
  actors-before-route and C3 diagnostics keep direct rendering. About **6.45 MiB** of
  cached pixel/depth storage at 640x480. Caches retire with the level leg.
- Keep doors, balloon bob/departure, hazards, pads, levers, gravity, fluids and secondary
  animation live. Eight raster row ranges (seven persistent workers plus caller) visit
  commands in original order, with a barrier before each next stage. Each pixel has one
  writer. Assets, scene state, RNG, observers and simulation stay on the caller; threads
  join at teardown. No quantization, animation suppression or timing/rule change.
- Write ordinary scene fragments directly into RGB: supported blend factors use source
  alpha, never destination alpha. Observers retain synchronous complete RGBA events.
  Preserve barycentric/UV/filter/rounding, edge, blend and Z operations exactly. Pipe
  accepted-depth output replaces its full-mask diagnostic event and permits batching.
- Skip absent stages/irrelevant root and billboard work; retain primary all-root record
  counts without discarded mesh copies. Move texture storage into stage maps. Body/pick
  preparation borrows const decoded sheets within the owning application leg; standalone
  consumers still own images. Footprint/occlusion geometry and all simulation stay intact.

Exact synthetic serial/batched color and depth checks pass, including direct RGB, accepted
pipe depth, filter/address/blend/shade/depth variants and opaque merge ties/occluders.
Cached-geometry counters now count merged nearest stores, not hidden re-executed overdraw;
this is a telemetry difference. No original execution/new REA session was needed: accepted
[profiling](frame-rate-profile-001.md), [body occlusion](basher-occlusion-001.md),
[actor coverage](actor-occlusion-002.md) and [debris](debris-3d-001.md) evidence remains the
static/provisional authority. Original depth/timing/fidelity gates remain unresolved.

## Pacing and dense drawing limits

`wall-*.csv/log`/`wall-summary.json` repeat all twelve modes with production wall time,
no `--frames`/replacement dt, closing after 240 draws: means **5.721-15.886 ms**; Kriss
Kross rotation p95/max **18.385/19.382 ms**. Present costs about 0.2-0.3 ms. The unchanged
144 Hz fixed camera loop's 1 ms sleep fallback is not taken during simulated play (Idle
zero); vsync/sleep granularity is not the measured bottleneck. Heavy-view cost and worker/
host scheduling variation remain; no claim to isolate every OS stall or eliminate all stutter.

`supplemental/` adapts the retained debris benchmark, current Release objects, four named
levels, eight artificial cutters actually removing cells in five World ticks, then eight
synthetic carried-brick render poses. Each zoom has 24 warmups and 60 measured 3D frames,
interleaved with the retained flat comparator; ages sweep 290 ms. Full geometry is rendered,
without application caches, HUD or SDL. This is dense drawing evidence, **not natural play,
application FPS, original comparison or a new replay fixture**. Source/response/workload,
CSV/summary and clone PNGs are retained there.

| Level / zoom | Pieces | Body ms | Footprint ms | Bricks ms | 3D debris ms | CPU scene ms |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Just Climb / 0 | 124 | 3.799 | 0.074 | 4.473 | 0.395 | 23.152 |
| Just Climb / 1 | 124 | 4.039 | 0.051 | 4.693 | 0.571 | 29.755 |
| Sergeant Bash (20) / 0 | 88 | 4.037 | 0.084 | 4.651 | 0.354 | 24.884 |
| Sergeant Bash (20) / 1 | 88 | 4.179 | 0.054 | 4.760 | 0.586 | 30.552 |
| Kriss Kross (19) / 0 | 92 | 4.091 | 0.087 | 4.658 | 0.441 | 26.233 |
| Kriss Kross (19) / 1 | 92 | 4.216 | 0.068 | 4.928 | 1.103 | 30.397 |
| The Crowded House (21) / 0 | 88 | 4.296 | 0.094 | 4.715 | 0.395 | 26.132 |
| The Crowded House (21) / 1 | 88 | 4.070 | 0.033 | 4.664 | 0.516 | 28.860 |

Eight carried bricks still cost 4.5-4.9 ms, with full-frame scratch RGB/depth allocation
and attachment-depth copies; bodies also retain full-frame scratch images. Debris costs
0.35-1.10 ms here. Dense skills/4096 particles are not proven at 60 fps. Regional/reusable
actor scratch crosses body/attachment/picking lifetimes and needs bounded output validation;
that path is preserved after removing the larger repeated scenery costs. This task's
60-fps average result is limited to the twelve ordinary measured modes.

## Checks and handoff

`final-build.log`: production Release build succeeds. `focused-build.log` and
`test_raster_rows-final.log`: new **legacy-only**, assertions-enabled check passes;
`test_sceneraster-final.log`: existing explicit-state checks pass. The playtest cache has
BUILD_TESTING off, so an initial focused CTest call found no tests; these executables
were then run directly, not counted as a CTest pass. One `after-eight` launch under UCRT
Python failed without samples; registered Python 3.13/absolute paths fixed orchestration.
`ordinary-size.log/png`: 1280x720 initial diagnostic to 640x480 window transition exits 0;
inspected clone image shows terrain/figures/HUD, checking cache resize, not fidelity.

All **17 active promoted recordings pass**, won=1/completed=1, using final
`webolution --game-dir "Lemmings Revolution" --replay FILE`. Exact paths, steps and results:
`replay-results.json`, `replays/*.log`. No recording/check was changed or retired.

| Recording level | Saved/needed | Result |
| --- | ---: | --- |
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
| MM_E1_3_SKILLS | 10/5 | PASS |
| M06_KF_1 | 91/50 | PASS |
| MM_E1_4SKILLS | 20/10 | PASS |
| MM_E2_4SKILLS | 14/10 | PASS |
| MM_BASH_DIG_001_A | 20/10 | PASS |
| MM_BUILD_BLOCK_A | 15/10 | PASS |

Kriss Kross/Crowded House have no active promoted win here. Replays establish simulation
regression, not every rendered animation phase; no whole-frame fidelity claim. User
playtest remains visible acceptance. **Smoke/recovery remains pending the supervisor.**

Changed: CMakeLists.txt; src/app/{main.cpp,BackdropScene.cpp,BackdropScene.h,Raster.cpp,
Raster.h,RasterRows.h,SceneRaster.cpp,SceneRaster.h,TubeScene.cpp,FrameProfile.h};
src/__test__/{test_raster_rows.cpp,FrameProfileDisabled.h}; this report. Incoming edits
preserved; scoped diff/LF checks pass. Supervisor PID 15712 owns RepositoryLease; entry
Get-Process found no clone/build runner (optional CIM metadata denied, not retried).
No protected controls/instructions, original execution/capture/hooks/debugger, subagents,
installation, commits/pushes/reset/clean/stash were used or changed.

## Review revision: deferred surface textures

The review of run `20261117T063248-15712-db585079` requested correction of a
source-traced lifetime defect in **`--geom wall+surface`**. Its supervisor
[`verification.log`](../../build/agent-runs/20261117T063248-15712-db585079/verification.log)
passed **95/95 main smoke and 84/84 recovery** before this correction; those
results do not verify the revised content. **Revision smoke remains pending
supervisor execution after worker exit.** This is the next dependency for
acceptance of playtest 11 A5's rendering optimization, not another optimization
or reopening of simulation/timing work.

Production path: `runApplicationLeg -> renderFrame -> scheduleSceneFrame` creates
`RasterCommandBatch` around `TubeScene::render` when rebuilding the tube cache,
then flushes after `render` returns. `drawSurface -> Raster::triangle` previously
passed `tile ? tile->texture : Texture{}`. The conditional creates a temporary
even when the tile exists, copying its pixel vector; the temporary dies at the
end of that call, but `triangle` queued a closure capturing it by reference.
This was undefined behavior; the review did not reproduce a runtime failure.

`drawSurface` now skips missing tiles and passes `tile->texture` directly. The
tile belongs to `TubeScene::outer_`/`inner_`, whose storage remains alive and
unchanged through the caller's flush. Missing textures still draw nothing,
matching `triangle`'s empty-texture return. Geometry, triangle ordering, UV,
sampling, modulation and depth rules are unchanged. The default `route` path and
simulation are untouched. No visual exception is intended; prior undefined
surface results are not an output baseline.

`Raster.h` documents the borrowed lifetime and deletes the temporary-texture
overload. Existing legacy `test_raster_rows` adds compile-time checks accepting
const/non-const lvalues and rejecting const/non-const rvalues, preventing this
conditional-temporary mistake from silently returning. Its existing RGB/depth
and deterministic batch assertions are unchanged; no test registration, tier,
timeout, replay or fixture was changed.

Audited the other batched callers: wall, cell-mesh, route and polylist triangles
also use scene-owned tiles. Actor/debris draws run outside the scenery batch;
actor-owner triangles execute synchronously. `BackdropScene`'s separate batches
flush before their local white texture and stage-owned texture maps expire;
sprite pixels are moved into those maps before submission. No additional
deferred texture lifetime defect was found in these source paths.

Revision evidence: [revision-002/](../../build/frame-rate-001/revision-002/).
`entry/` preserves the four incoming files; `entry-status.txt`, `entry-diff.patch`
and `parent-lease.json` retain the entry context. `revision-only.patch` isolates
this correction; `diff-check.log` records scoped diff-check exit 0; all four
revision files use LF. Supervisor PID **15712** owns
the lease. Get-Process found no clone/build runner before edits; optional CIM
metadata was denied and was not needed. Only `src/app/{TubeScene.cpp,Raster.h}`,
`src/__test__/test_raster_rows.cpp` and this report were changed in this revision.
No protected controls or shared instructions were edited.

The registered **gcc-release -O3 -DNDEBUG**, jobs-8 build completed
(`release-build.log`). The assertions-enabled `test_raster_rows` and existing
`test_sceneraster` each exit **0** (`focused-results.json`, individual logs).
The production executable's `--direct-level --native --geom wall+surface
--frames 8 --no-audio` diagnostic exits **0** for MM_CLIMB_002, MM_E1_4SKILLS,
E19_KF_2 and E20_mg_1 (`surface-*.log`). These SDL dummy-driver checks exercise
the previously unsafe cached submission/flush path; they do not establish
visual fidelity or a frame-time measurement.

All **17 active promoted recordings were rerun on the revised Release executable**
and pass, exit **0**, **won=1/completed=1**. Each recording's exact command,
step, saved count and log is retained in `revision-002/replay-results.json` and
`revision-002/replays/*.log`; the per-level results in the table above still
hold. No divergence, retirement or recording/check change. Runs use private
`revision-002/private-save` LocalAppData; no installed/normal save is used.

The twelve performance tables remain the retained, reviewed measurements;
they were not rerun or relabelled as new measurements. They used default
`route`, which this correction does not alter. The rotation/stress limitations
above remain. No new original analysis/execution, runtime capture, debugger,
hooks, subagents, background jobs, installation or commit was used.

Handoff: deferred surface textures now remain scene-owned through flush; focused checks and all 17 promoted replays pass; needs_review.
Next: supervisor smoke on this revision, independent review, then user playtest; retained performance/tail limits remain.

What to check: Just Climb, Sergeant Bash (20), Kriss Kross (19), The Crowded House (21):
play, rotate/zoom, toggle fast-forward and build/bash/mine/dig. Expect smooth ordinary
play with unchanged scenery, actor/brick/debris occlusion, terrain edits and hatch/panel
input; watch Kriss Kross rotation and many builders for the documented frame-time spikes.
For the corrected diagnostic, open each with `--geom wall+surface`, rotate/zoom:
expect stable textured surfaces without corruption or crashes.
