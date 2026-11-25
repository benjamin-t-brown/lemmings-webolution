# gl-effects-001

Phase 6 of the November 24 OpenGL request covers the Just Bomb and Just Dig routes
from playtest 7 and Kriss Kross. The native comparison previously skipped blast
marks, drowning and particles. In `runApplicationLeg`, the World observers and
level clock supply `TubeScene::prepareExplosionDecals`, `prepareActors` and
`prepareDeathParticles`. `OpaqueSceneComparison::draw` now submits those same
records to `NativeOpaqueGpu`. Both consumers use the prepared tint, pose and
materials. Software supplies the displayed frame in comparison mode. [Request](../playtest/2026-11-24-opengl-request.md) · [Plan](../OPENGL_PLAN.md)

- **Marks** use linear clamp and byte-domain ONE/ONE over completed scenery. They
  retain the provisional −0.0012 view-Z offset and test strict Less against a copy
  of scene Z. Marks leave scene Z intact before actors draw.
- **Drowning** keeps the body and bubbles in their actor's sorted slot. The depth
  test uses pre-fluid Z wherever later scenery left post-fluid Z unchanged. The
  body tests the pipe and terrain footprint too. A GPU visibility query applies
  the software body's scenery gate to the subsequent bubble draws.
- **Particles** share actor coverage with bodies and bricks. Meshes use affine
  physical Z and the RGB tile sampler. SPR pieces use origin depth, axes use
  the body plane and terrain footprint, and forced flights also test the pipe
  plane. All GL submissions run on the context thread. The simulation's level
  clock controls emission, spin and lifetime before either backend draws.

## Comparison evidence

| Installed level | Native OpenGL 3.3 versus software |
| --- | --- |
| Just Bomb (`MM_BOMB`), 14 checkpoints | Mark and particle colour changes matched exactly. At 15,122 ms, opaque actors replaced the same 74 marked pixels. The peak contribution at the centred camera fell from 31 to 25 bytes between 13,693 and 17,650 ms. [Frames](../../build/gl-effects-001/final-frames/MM_BOMB-001/) · [Route log](../../build/gl-effects-001/MM_BOMB-final-comparison.log) |
| Just Dig (`MM_DIG_000`), 14 checkpoints | Particle coverage and owners matched throughout. At 6,890 ms both drew the piece at (323, 121) and clipped it at (303, 128). Three one-byte delta differences came from the underlying scenery, which the debris replaced identically. At (535, 96) and (436, 108), a red tile filtered just below 153 on CPU and the 0.5 face shade produced red 76 versus GL's 77. [Frames](../../build/gl-effects-001/final-frames/MM_DIG_000-001/) · [Depth and rounding witnesses](../../build/gl-effects-001/MM_DIG_000-witnesses.log) · [Delta trace](../../build/gl-effects-001/effect-delta-trace.tsv) |
| Kriss Kross (`E19_KF_2`), six checkpoints | The promoted win yielded zero drowning states, so the images use natural idle ticks 187–188. Actor ownership, depth and RGB matched. At tick 188 and yaw 80, both showed one body pixel through water and clipped 11 body pixels by intervening scenery. A synthetic prepared frame independently exercised pipe clipping, bubble gating, the axe and forced-flight depth modes, and exact owner ties. [Frames](../../build/gl-effects-001/final-idle/) · [Drowning points](../../build/gl-effects-001/drowning-witnesses.tsv) · [Synthetic checks](../../build/gl-effects-001/effects-contract.log) |

Full-image RGB errors above one byte carried through unchanged from earlier
scenery passes. For example, at Just Bomb frame 9 the nearly coplanar secondary
`~Water01` and `WaterRef01` draws can quantize to the same hardware scene Z.
In Just Dig frame 12 the competing legacy draws use `Models\exit\basket`.
The per-pass candidate and material logs retain both depths and winners.
[Scenery trace](../../build/gl-effects-001/scenery-difference-trace.tsv) · [Pass summary](../../build/gl-effects-001/effects-summary.tsv)

Static REA/Ghidra used the registered x86 reference, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Instruction reads corroborated the mark dispatch at `0x4082B6` and blend call at
`0x4402FB`, the drowning selector and neutral tint at `0x404403` and `0x404434`,
and the axe animation update at `0x419AA5`. The original effective depth state
and the clone's provisional effect placement and timing remain fidelity questions.
The measured GPU precision was binary64 on an RX 9070 XT. [Static packets](../../build/gl-effects-001/rea-static.json)

## Checks and files

Registered UCRT64 GCC built `smoke_tests`. The 17 selected dummy-driver smoke
checks passed, including the three relevant sweep chunks. All 32 promoted
recordings reproduced their outcomes, including the archived return to the
map. Just Bomb won 17/10 at step 954, Just Dig 19/10 at step 1014, and Kriss Kross
24/20 at step 6034. The Emscripten target linked using the existing SDK cache.
[Build](../../build/gl-effects-001/smoke-target-complete.log) · [CTest](../../build/gl-effects-001/focused-smoke.log) · [Individual replays](../../build/gl-effects-001/replays.tsv) · [Emscripten](../../build/gl-effects-001/emscripten-build.log)

Changed `src/app/ActorDepth.h`, `ActorGlShaders.h`, `NativeOpaqueComparison.cpp`,
`OpaqueMeshShaders.h`, `OpaqueSceneComparison.cpp/.h`, `PreparedLevelFrame.h`,
`TubeScene.cpp/.h`, `main.cpp` and this report. Comparison sources, logs and
frames are under `build/gl-effects-001/`.

**Handoff** — `gl-effects-001` is `needs_review`.  
**Next action** — The supervisor runs the full smoke and recovery gate.

**What to check** — Start with `--compare-opaque DIR` and press F10 at each
effect. In Just Bomb, assign Bomber and watch later walkers cover the fading
mark. In Just Dig, assign Digger and rotate past the cut while chunks fly.
In Kriss Kross, let an ordinary lemming enter a tank and rotate past the tank
wall. Expect matching depth and coverage in the two images at each checkpoint.
