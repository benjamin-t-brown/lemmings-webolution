# Native OpenGL actors and displayed-frame queries

**Status** — needs_review. Phase 5 of the November 24 OpenGL request covers the
Loggerheads wall turn and lower round log, and the Just Build bridge feet from
playtest 9 Y10/Y11 and playtest 10 Z1. The comparison reference is the clone's
accepted upright-plane and collision-footprint behavior.
[OpenGL request](../playtest/2026-11-24-opengl-request.md) · [Playtest 9](../playtest/2026-11-16-playtest-9.md) · [Playtest 10](../playtest/2026-11-16-playtest-10.md)

`runApplicationLeg` prepares the actor order, SPR frames, tint, attachments,
placement boxes and projected hatch triangles once. `--compare-opaque`
passes that frame to `OpaqueSceneComparison` and `NativeOpaqueGpu`. The body
shader samples integer screen coordinates, clamps and mirrors within the SPR
frame, accepts alpha above `0xEF`, and multiplies tint in integer bytes. It
tests the upright plane at pixel centres against scene and pipe depth and
checks the cylinder rings and edited integer collision grid. Carried cuboids
have a private triangle-depth pass. The hands mask their own brick, then the
surviving fragment submits its depth and stable owner alongside the other
actors. Lower owners win exact ties. Decals have their own software depth
policy in the preceding slot. [Application](../../src/app/main.cpp) · [Native pass](../../src/app/NativeOpaqueComparison.cpp) · [Shaders](../../src/app/ActorGlShaders.h)

GL tests hatch triangles against scene depth at a 0.5 world-unit tolerance
and reduces each visible mask to a rectangle. Input uses the shared CPU
placement-box picker and checks the GL hatch rectangles in comparison mode.
The point read selects actor depth first and falls through to scene depth.
The displayed image comes from the complete software pipeline. [Query comparison](../../src/app/OpaqueSceneComparison.cpp)

## Loggerheads round-log review

- The promoted node 17 recording reaches the lower log at column 201, file
  row 17, at 42,005 ms. Actor 0 reverses from right to left as it changes from
  climbing to falling. The current motion tracker gives radial layer 0.5 at
  the turn. The earlier contact report recorded 0.75. The first probe assumed
  a walk clip and stopped at the climbing pose. The revised probe matches the
  recorded position and facing transition and uses the current clone's pose
  for both renders. It captured 16 evenly spaced yaws at the turn and three
  each at 41,965 and 42,044 ms. Each checkpoint includes CPU and GL images,
  heatmaps, and depth/owner diagnostics for all eight scenery passes and the
  actor pass. [Contact location](actor-occlusion-002.md) · [Capture log](../../build/gl-actors-001/round-log-revision/loggerheads-round-log.log) · [Paired frames](../../build/gl-actors-001/round-log-revision/frames/MM_E1_3_SKILLS-001/)
- All 22 checkpoints matched actor colour, coverage, depth, and owners across
  4,356 body pixels. At the turn, actor 0 has zero visible pixels at yaw 201
  and 2, 13, 12 and 6 at yaws 137, 153, 169 and 185 respectively on each
  backend. Scenery coverage matched. All 104 final scenery winner changes came
  from earlier triangles retained on normalized binary32 depth ties. Five
  final pixels differed by more than one colour byte, all at exit-basket edges.
  The final maximum was 4 at yaw 153, pixel (222, 374), where CPU owner 1080 at
  333.28650432784593 and GL owner 863 at 333.28652482263567 shared code
  `3e1011ec`. The intermediate maximum was 6 in `legacy-unattributed` at yaw
  25, pixel (395, 263), where CPU owner 1478 and GL owner 1263 shared `3e1e21b2`.
  [Actor totals by yaw](../../build/gl-actors-001/round-log-revision/views.tsv) · [Per-pass counts](../../build/gl-actors-001/round-log-revision/passes.tsv) · [Scenery winner trace](../../build/gl-actors-001/round-log-revision/scenery-winners.tsv)
- Actor boxes and hatch bounds agreed at every yaw. At yaw 137, the hatch
  rectangle (112, 186)–(131, 224) picked hatch 0 under (121, 205) on both
  backends. All 56 standard samples and 17 contact-specific samples agreed on
  owner and finite depth, with at most two binary64 ULPs of depth difference.
  Contact zoom poses agreed within 2 × 10⁻¹⁴ world units. The first pointer
  read after actor submission took 0.902 ms median and 43.151 ms maximum.
  Contact pointer rereads took 0.352 ms median and 0.437 ms maximum. The
  first hatch read took 24.182 ms, and later hatch reads took 0.232 ms median
  and 0.324 ms maximum. [Hatch example](../../build/gl-actors-001/round-log-revision/frames/MM_E1_3_SKILLS-001/frame-000015-queries.tsv) · [Point queries](../../build/gl-actors-001/round-log-revision/point-queries.tsv) · [Contact and zoom queries](../../build/gl-actors-001/round-log-revision/contact-queries.tsv) · [Timing summary](../../build/gl-actors-001/round-log-revision/analysis.log)

## Evidence

- The RX 9070 XT at 640×480 matched body and brick colour, coverage and owners
  across 30 paused replay views and five application input frames. These
  include 23,931 body pixels and 1,103 carried-brick pixels. At the Loggerheads
  turn, actor 1 had 38 visible pixels at yaw 87 and zero at yaw 151 in both
  renders. In Just Build the bridge walker was visible at yaw 195.046 and
  occluded at 99.046. At 26,361 ms and yaw 192, the builder contributed 110
  body and 272 carried-brick pixels. Ten repeat views and five input frames on
  the final build matched the same actor and hatch checks. [Loggerheads images](../../build/gl-actors-001/frames/MM_E1_3_SKILLS-001/) · [Just Build images](../../build/gl-actors-001/frames/MM_BUILD-001/) · [Actor totals](../../build/gl-actors-001/actor-summary.tsv) · [Final spots](../../build/gl-actors-001/final-spot-analysis.log)
- Brick depth differed at 147 same-owner pixels by at most two binary64 ULPs
  in affine triangle interpolation. All 102 sampled point depths agreed
  within one ULP. The 206 inherited scenery winner differences were normalized
  binary32 depth ties. Of these, 108 exceeded one colour byte. The largest
  difference was 36 at Just Build frame 11, pixel (210, 236). Exit-basket
  triangles 1300 and 1087 had CPU depths 327.5519765581863 and
  327.55198146917576 and shared code `3e0d314f`, so GL kept the earlier
  triangle. A synthetic actor overlap matched in binary64 and demonstrated the
  binary32 fallback limit. CPU owner 19 at 9.99999995 and GL owner 4 at 10
  differ when the depths round to the same float. Equal-depth draws chose
  owner 4 in either submission order. [Point depths](../../build/gl-actors-001/point-queries.tsv) · [Scenery trace](../../build/gl-actors-001/scenery-winner-trace.tsv) · [Binary64 probe](../../build/gl-actors-001/actor-contract-final.log) · [Binary32 probe](../../build/gl-actors-001/actor-contract-binary32-final.log)
- All sampled actor boxes and hatch rectangles matched. Just Build hover at
  (506, 157) picked hatch 0, and the click at (178, 318) assigned Builder to
  actor 0. At that pointer both backends read 344.29983071498668 and produced
  the same zoom pose. The two live zoom readbacks took 0.180 and 0.737 ms.
  Subsequent hatch readbacks had a 0.226 ms median and a 0.332 ms maximum.
  First hatch reads took 24.1–25.0 ms. The first diagnostic point read after
  queued actor work reached 43.2 ms. All five input-frame presentation reads
  matched the complete software image byte for byte. [Input log](../../build/gl-actors-001/input-final.log) · [Query timing](../../build/gl-actors-001/analysis.log)
- Retained instructions and pseudocode identify the registered EXE as
  `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
  The origin check at `0x43CC94/0x43CC9A`, bias at `0x43CCB5` and call at
  `0x43CD4E` support the prepared placement. Tint follows `0x403F40`,
  and `0x410C0E/0x410E78` establish separate brick geometry. The upright
  plane, footprint, carried cuboid and zoom sampling are clone policies.
  [Retained Ghidra packets](../../build/gl-actors-001/retained-static/) · [Tint evidence](actor-light-lookup-001.md) · [Brick evidence](builder-brick-anim-001.md)

## Checks and limits

- The registered `smoke_tests` target was up to date, and four focused smoke
  tests passed with dummy drivers. All 32 promoted recordings passed
  again, including Loggerheads 10/5 at step 2095 and Just Build 20/10 at step
  1685. The previous supervisor run passed 95 smoke and 84 recovery tests. The
  supervisor gate for this revision is pending.
  [Revision build](../../build/gl-actors-001/round-log-revision/smoke-target-build.log) · [Focused CTest](../../build/gl-actors-001/round-log-revision/focused-dummy-smoke.log) · [Replay results](../../build/gl-actors-001/round-log-revision/replays.tsv) · [Previous supervisor run](../../build/agent-runs/20261124T211505-9332-cb63854e/verification.log)
- Registered MinGW built `smoke_tests`. All 14 focused smoke-labelled tests
  passed with dummy drivers. All 32 promoted replays passed. Just Build was
  20/10 at step 1685, and Loggerheads was 10/5 at step 2095. The registered
  Emscripten 5.0.3 build linked `webolution.js`. PowerShell execution policy
  blocked its wrapper, and the build completed through registered UCRT64 Bash.
  [Smoke build](../../build/gl-actors-001/smoke-target-final.log) · [Focused CTest](../../build/gl-actors-001/focused-smoke-final.log) · [Replay results](../../build/gl-actors-001/replays.tsv) · [Emscripten build](../../build/gl-actors-001/emscripten-build.log)
- Drowning, decals and particles await phase 6. Throughput qualification and
  input checks on binary32-only hardware remain open. The timings are
  synchronous readbacks from this adapter, including any queued work.

The review revision adds comparison evidence and updates this report. Phase 5
changed `ActorDepth.h`, `ActorGlShaders.h`, `ActorOcclusion.h`,
`ActorSprite.h`, `BackdropScene.cpp/.h`, `NativeOpaqueComparison.cpp`,
`OpaqueMeshShaders.h`, `OpaqueSceneComparison.cpp/.h`, `PreparedLevelFrame.h`,
`main.cpp` and this report. Probe sources, logs and frames are under
`build/gl-actors-001/`.

**Handoff** — `gl-actors-001` is `needs_review`.\
**Next action** — The supervisor runs the pending smoke and recovery gate.

**What to check** — Open Lems at Loggerheads with `--compare-opaque DIR`.
Right-drag at the lower round log at column 201, file row 17, and press F10.
Expect both actor images to hide and reveal the reversing lemming at the same
angles. In Just Build, rotate across an active
builder and the feet of a bridge walker. Hover the hatch, assign Builder and
zoom at the selected walker. Expect the same hatch rectangle, actor pick and
zoom anchor in the comparison logs.
