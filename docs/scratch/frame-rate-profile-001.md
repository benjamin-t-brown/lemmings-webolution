# frame-rate-profile-001: measured software-rendering waste

Status: **needs_review**. Playable v1, implementation-plan step 8/P18 and the
[playtest 5 frame-rate addendum](../playtest/2026-11-14-playtest-5.md).
Difficulty: difficult; recommendation Opus; this worker used the authorized Codex
override. Full registered main/recovery verification is **pending supervisor
execution**, followed by independent acceptance review. No existing test assertion
or timeout was changed.

The demonstrated failure is low frame rate in ordinary clone play. This task
measures and improves the existing CPU renderer; it establishes neither human
playability nor original fidelity. It changes no simulation rule, elapsed-time
budget, release rate, skill, camera motion or random-seeding policy.

## Method and authentic baseline

Machine: Windows 11 build 26200, AMD Ryzen 9 9950X3D, 16 cores/32 logical processors.
Registered UCRT64 GCC 16.1.0, CMake 4.3.3, Ninja and SDL2 2.32.10. The benchmark
build is **Release, `-O3 -DNDEBUG`, C++23**. The separate development pixel checks
use the registered main build's `-O2 -g`, with assertions enabled. Builds used at
most eight jobs. Machine/entry identity: `build/frame-rate-profile-001/machine.json`.

All evidence below is relative to the repository root, under
`build/frame-rate-profile-001/` unless another root is shown. Entry HEAD was
`c96bb6998bb056cdd2fc7368b7826c4048f1d604`, with accepted uncommitted changes.
`entry-source/`, `entry-sha256.json`, `entry-status.txt` and `pre-existing.patch`
were retained before edits. The baseline was then instrumented, built and run
**before any optimization**. Its exact sources are in
`instrumented-before-source/` with their separate manifest; `before.exe` is that
authentic Release binary, not rebuilt from resumed modified sources. The complete
entry manifest was rechecked unchanged. `after.exe`, `final-source/`,
`final-source-sha256.json`, `worker-tested-sources.json` and
`executables-sha256.json` preserve the final demonstration provenance.

Configure with the registered CMake/G++/Ninja paths, `-DCMAKE_BUILD_TYPE=Release`
and `-DWEBOLUTION_GAME_DIR="C:/progs/lemmings-webolution/Lemmings Revolution"`;
then `cmake --build build/frame-rate-profile-001/release --target webolution --parallel 8`.
`configure.log`, `reconfigure-tests.log` and the `build-*.log` files retain actual
commands/results. One early hatch-target build lacked the game-directory cache
argument and did not build that target; it was corrected before measurement.

Set `WEBOLUTION_FRAME_PROFILE` to a private CSV path and run the **production
executable**, for each installed level:

```
webolution.exe --direct-level --level MM_CLIMB_002 --native --sim-ticks 50 --frames 150 --no-audio --window-shot <private-output.png>
webolution.exe --direct-level --level MM_W_012 --native --sim-ticks 50 --frames 150 --no-audio --window-shot <private-output.png>
```

`--direct-level` is explicit diagnostic entry into the production level loop.
The first level has 776 outer cell quads; the larger, fluid-bearing second has
988. Both runs have **3?4 live released lemmings**, recorded in every CSV row.
The water receivers start empty in this sample; its small fluid cost is not a
measurement of a full screen of moving water. The slow route below also exercises
live fluid submissions. These are controlled clone routes, not human playtests.

The established `--frames` clock supplies **1/144 second of simulation per frame**,
keeping the before/after state identical despite different rendering speed.
FPS here is measured wall-clock draw throughput, not that simulation frequency.
Main production benchmarks used the default desktop video backend, dummy audio
and `--no-audio`; child `LOCALAPPDATA` pointed to private scratch storage. Direct
entry does not construct a persistent save store. No installed/normal save was
used for these benchmarks.

`FrameProfile.h` uses exclusive `steady_clock` scopes. `main.cpp` measures the
level-loop iteration, simulation/update work, scene preparation, HUD/input
feedback and individual SDL operations. Nested triangle scopes in `SceneRaster`
and `Raster` separate pixel rasterization from preparation; fluid selects its
own category. Texture read/decompress/BMP/alpha work is separately timed in
`loadTexture`. Actor time covers pose/clipping/composition except nested texture
decoding. Preparation includes clears, mesh/billboard generation, conversions,
statistics and depth copies. HUD includes its image copies and picking feedback.
The scripted observer interval is otherwise Other; texture work it triggers
still lands in Texture decode. CSV writing is outside the measured interval.
Startup loading and the initial pre-loop frame are excluded.

Each measured sample uses frames **20?148** (129 frames), excluding warmup and
the final image export. The primary comparison alternates before/after order
across three trials per level, running only one clone at a time. Raw CSV/log/PNG
sets are `before-{small,fluid}-trial{0,1,2}.*` and the corresponding `after-*`.
`paired-benchmarks.json` and `cost-table.json` retain means and individual trials.

## Frame costs

Milliseconds per frame, mean of the three trial means:

| Exclusive cost | Tutorial before | Tutorial after | MM_W_012 before | MM_W_012 after |
| --- | ---: | ---: | ---: | ---: |
| Simulation/update | 0.010 | 0.010 | 0.012 | 0.011 |
| Scene preparation | 5.411 | 4.392 | 6.435 | 5.323 |
| Texture decode | 8.963 | 0.009 | 5.067 | 0.000 |
| Backdrop rasterization | 23.957 | 18.572 | 24.536 | 19.175 |
| Tube rasterization | 8.101 | 7.053 | 8.216 | 7.249 |
| Actors | 0.998 | 1.034 | 0.489 | 0.470 |
| Fluid rasterization | 0.000 | 0.000 | 0.004 | 0.004 |
| HUD/panel/cursor | 1.270 | 1.278 | 0.738 | 0.680 |
| SDL_UpdateTexture | 0.094 | 0.089 | 0.091 | 0.083 |
| SDL clear/RenderCopy | 0.003 | 0.002 | 0.002 | 0.002 |
| SDL_RenderPresent | 0.240 | 0.225 | 0.232 | 0.225 |
| Other | 0.053 | 0.043 | 0.025 | 0.008 |
| Sleep | 0 | 0 | 0 | 0 |
| **Total** | **49.100** | **32.707** | **45.847** | **33.229** |
| **FPS** | **20.37** | **30.57** | **21.81** | **30.09** |

Tutorial improves 1.50x and the larger level 1.38x. Trial totals were
48.81?49.53 / 32.55?32.81 ms for the tutorial and 45.58?46.05 /
32.97?33.42 ms for water. The initial, unpaired measurements made before edits
were slower: **60.390 ms/16.56 FPS** and **59.214 ms/16.89 FPS**, retained as
`before-small.csv` and `before-fluid.csv`. Their complete costs follow; they
are not silently substituted for the later alternating measurements. Host
variation was not separately diagnosed.

| Initial pre-optimization cost (ms) | Tutorial | MM_W_012 |
| --- | ---: | ---: |
| Simulation | 0.011 | 0.013 |
| Preparation | 6.939 | 8.371 |
| Texture decode | 10.702 | 6.330 |
| Backdrop raster | 30.438 | 32.994 |
| Tube raster | 9.134 | 9.600 |
| Actors | 1.213 | 0.615 |
| Fluid raster | 0 | 0.004 |
| HUD | 1.516 | 0.898 |
| Upload / copy / present | 0.117 / 0.003 / 0.272 | 0.103 / 0.002 / 0.252 |
| Other / sleep | 0.045 / 0 | 0.032 / 0 |

SDL reports renderer **software**, flags **0xD**, including its reported vsync
bit. We did not request `SDL_RENDERER_PRESENTVSYNC`; the existing software
presenter reports it anyway. Observed Present costs about 0.225 ms, with no
refresh-period wait dominating the samples. Simulation frames execute no
`SDL_Delay`; only the unchanged no-update branch sleeps 1 ms. Replacing SDL's
presenter would address about 0.3 ms, so this task leaves its policy unchanged.

## Production changes and dependency trace

The production path is `runApplication` -> `runApplicationLeg` -> retail loader,
`TubeScene::build`/`BackdropScene::build` -> level loop -> `renderFrame` ->
`scheduleSceneFrame` (primary, legacy, tube, fluid, secondary) -> actor pass ->
HUD/panel -> RGB upload -> SDL copy/present. Scene meshes/billboards call
`rasterizeSceneTriangle`; tube triangles call `Raster::triangle`. This is the
same path timed and exported by the executable demonstrations.

1. **Raster address waste:** backdrop sampling performed floating `fmod` and
   repeated addresses for bilinear taps. It now computes the four integer tap
   addresses once, uses integer modulo, and masks power-of-two dimensions.
   Bounded UV (?1e6) times positive int dimensions gives integer floors below
   2.15e15: exactly representable in double and safely in int64. Negative wrap
   remains mathematical modulo. Tube fractional UV already bounds adjacent
   indices to [-1,size], so one conditional wrap replaces six modulo operations.
   Interpolation, tap accumulation order, depth, blending and rounding remain
   unchanged. Measured backdrop savings are about 5.4 ms/frame, tube about 1 ms.
2. **Repeated decoding:** every frame repeatedly read, decompressed, decoded
   and alpha-processed identical textures, including actor sheets during
   drawing and picking. `TextureCacheScope` binds successful decoded images to
   the one live library in `runApplicationLeg`, returning independent copies.
   It retires at leg exit/retry; other libraries cannot hit it, and errors are
   not cached. There is no process-global persistent asset cache or class-layout
   change. Tutorial retains 116 images/9,928,912 RGBA bytes; water 117/9,468,112,
   with 5,941/5,408 hits over each demonstration. Decode drops from 8.96/5.07 ms
   to effectively zero. Copies remain deliberately caller-owned.
3. **Discarded scene meshes:** legacy and secondary passes rebuilt every root,
   including large primary meshes they discarded. They now build only roots of
   their selected stage. Primary retains the all-root record census. Selected
   draw order, per-root ancestor predicates, animations and sprite RNG remain
   unchanged. This removes about 1.0?1.1 ms of preparation in the primary samples.

The intermediate cache/integer-address build is retained in `after-small.csv`
and `after-fluid.csv` (35.301/36.162 ms, respectively). Final alternating results
include the further tap wrapping and discarded-root removal. These changes are
algebraic/cache/scheduling optimizations of the accepted clone behavior; no new
provisional player behavior or original device-state policy was introduced.

## Slow play test and output checks

The unchanged `test_hatch_timing_after routes <private-output>` driver was built
and run before and after. It exercises winning/losing MM_BLOCK_001_A placements
and the earned MM_W_012 route through the connected frontend, with lemmings out,
picking, fast-forward, fluid and natural results. Both runs exit 0, all checks
pass, and each records **1,614 level-loop iterations**. `hatch-before.exe`,
`hatch-before.log`, `hatch-before.csv`, `hatch-before-wall.json` and their after
counterparts retain exact runs. `hatch-costs.json` contains all sums and means.

| Whole route / mean exclusive frame cost | Before | After |
| --- | ---: | ---: |
| Process wall time | 126.427 s | 72.522 s |
| Timed rendering/presentation work | 121.687 s | 66.983 s |
| Rendering share of process wall | 96.25% | 92.36% |
| Whole-route iteration throughput | 12.77 FPS | 22.26 FPS |
| Simulation (ms/frame) | 0.029 | 0.030 |
| Preparation | 7.158 | 6.554 |
| Texture decode | 27.748 | 0.005 |
| Backdrop raster | 25.937 | 20.210 |
| Tube raster | 9.044 | 8.415 |
| Actors | 2.689 | 3.255 |
| Fluid raster | 0.538 | 0.436 |
| HUD | 2.102 | 2.439 |
| Upload / copy / present | 0.094 / 0.003 / 0.082 | 0.101 / 0.003 / 0.082 |
| Other / sleep | 2.305 / 0 | 2.797 / 0 |
| Timed frame total | 77.729 | 44.329 |

Rendering share is a conservative measured share: initial loading/render and
frontend frames are uninstrumented. Actor/HUD time includes cached image copies
that previously lived in the separately charged decoder. All three gameplay,
release, action, frame-state and outcome files are **byte-identical**, including
23/30 Blocker win, 0/20 gadget loss with the accepted two dry-pit splats, and the
0-save losing-placement route. These are regressions, not human-playable wins.

`hatch-comparison.json` retains all 49 file comparisons. Twenty Blocker PNGs
have 445?609 changed pixels at flame locations. The existing application seeds
flame first-draw frames from wall time, and the before/after logs retain different
RNG states. I inspected the first-release pair: the flames differ while the tube,
actors and UI agree. These uncontrolled images are **not pixel-equality evidence**;
`hatch-image-deltas.json` records their differences. Water-route PNGs and results
match. No seeding policy was changed to manufacture a match.

The two controlled production level images have identical complete PNG bytes in
all before/after trials. I inspected the tutorial/water baseline and final tutorial
images. The sentinel hashes are:

- MM_CLIMB_002: `61933b158580b1c869dd545500c7592b02c26cc36bd67ae2d93ec79cec4413ba`.
- MM_W_012: `3633704696154420f7a8edf02eb20fe109c6e32669867e20a0f8a1feca8c4b40`.

[Before tutorial](../../build/frame-rate-profile-001/before-small-trial0.png),
[after tutorial](../../build/frame-rate-profile-001/after-small-trial0.png),
[before water](../../build/frame-rate-profile-001/before-fluid-trial0.png),
[after water](../../build/frame-rate-profile-001/after-fluid-trial0.png).

`frame_profile_MM_CLIMB_002` and `frame_profile_MM_W_012` register these actual
production-executable demonstrations in CTest, including private storage, 150 CSV
frames and the unchanged pre-task image hashes. The assertions-enabled executable
passes both hashes too. No existing framebuffer test was weakened.

## Verification and disabled cost

- `targeted-pixels.log`: scene raster plus cache isolation and both production
  image demonstrations pass, **4/4**, 14.04 s on the assertion-enabled main build.
- `pixel-final.log`: unchanged sceneraster, terrainrender_scene/bomb/build/dig,
  backdropscene, fluidsurface_impl and scene_pack_adoption, plus the new
  differential test, pass **9/9**, 38.53 s. The differential compares **640 scene
  and 640 tube triangles**, exact colour and depth, with power-of-two and NPOT
  textures, negative/extreme UV, alpha, shade, depth and blend states, against
  authenticated pre-task clone sources compiled into a separate namespace.
- The new cache check exercises an installed caret texture and its alpha plane,
  caller mutations, library isolation, nested scopes, errors and lifetime.
- An initial differential build failed because GCC deduplicated identical
  pragma-once headers. A subsequent manifest-key formatting error also stopped
  configuration. Those logs and `pixel-regressions.log` (new executable Not Run)
  remain retained; `build-pixel-final.log`/`pixel-final.log` are the completed
  corrected checks. Existing passing checks were rerun after the corrected build.
- `hatch-before/summary.txt` and `hatch-after/summary.txt` both report installed=1,
  normal=1, failures=0. Their stores are private fixture files under those output
  directories; neither installed nor ordinary user progress was overwritten.

Profiling is off unless `WEBOLUTION_FRAME_PROFILE` names a CSV. Disabled scopes
read a null thread-local pointer and take no clock samples or file writes; no
per-pixel timers were added. The optional, excluded-from-default-build
`webolution_profile_compiled_out` target compiles the same optimized production
code with only timer scopes erased through a test-only header. Four alternating
pairs, 300 frames each, dummy video and no profile environment variable, gave
**35.677 ms/frame disabled vs 36.029 ms compiled out**, including identical startup
and PNG export costs. Standard deviations were 1.385/1.483 ms; there is **no
measurable disabled slowdown** within this run variation, not a claim of literally
zero instructions. All eight output hashes match. `disabled-overhead.json`,
`off-*.log/png` and `build-no-timers.log` retain this control.

The supervisor owns full registered verification; it was not duplicated here.
No full-suite pass is claimed by this worker. Task changes are CMakeLists.txt;
`src/app/{main.cpp,TubeScene.cpp,BackdropScene.cpp,Raster.cpp,SceneRaster.cpp,FrameProfile.h}`;
`src/assets/{Bitmap.cpp,TextureCache.h}`; the three new files
`src/__test__/{test_texture_cache.cpp,test_frame_raster_equivalence.cpp,FrameProfileDisabled.h}`;
`cmake/{frame_rate_profile.cmake,frame_rate_profile_run.cmake}`; and this report.
Pre-existing species/render edits remain intact; the task-entry snapshots separate
those changes from this task. Protected queue/controller/verifier/instruction files
were not edited. No agents, original game/oracle, debugger, hook, original capture,
installation, commit, push, reset, clean or stash was used.

## Limits and next dependency

Afterward, backdrop+tube software rasterization still costs **25.63/26.42 ms**,
about **78?80%** of frame time. Perspective sampling, interpolation, blend/depth
and CPU traversal are now the largest limit at this design. Clears, conversions,
statistics, caller-owned image copies and other preparation still cost several
milliseconds; these measurements do not prove every remaining instruction is
inherent. They do show that simulation and SDL presentation are small. A hardware
scene renderer would be separate work; none was added here.

The normal player wall clock was not retuned. Performance on the remaining 100
levels, different actor populations/cameras, actual monitor cadence and human
perception still need broader observation. Existing provisional fluid/actor depth,
timing parks, full-frame fidelity and project completion gates remain unresolved.
The registered reference EXE was only hash-read, matching
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
No original semantics needed new inference for these output-preserving changes;
retained comments/reports remain their original-evidence authority.

Handoff: implementation, Release measurements and unchanged-pixel regressions are ready for review; full registered main/recovery verification is pending the supervisor.
Next action: supervisor verification and independent acceptance review, then a human optimized-build frame-rate playtest; task remains needs_review.
