# explosion-pieces-3d-001

**Needs review. Supervisor smoke pending.** Playtest 20 K4 reported flat red
squares in the Bomber burst. I used the
promoted **Just Bomb** win (`MM_BOMB`, node 2) for the framebuffer evidence.
[Playtest 20](../playtest/2026-11-19-playtest-20.md) · [Human replay](../playtest/replays/2026-11-15-playtest-7/MM_BOMB-1794810807247401300-2.replay)

## Production

`World::stepExploder -> explode` clears the crater and counts the loss before
`runApplicationLeg` forwards the death observer to `TubeScene::emitDeathParticles`.
The old `drawDeathParticles` fallback projected the red burst and crater
fragments to screen-aligned squares. The draw pass now projects the eight
rotating vertices of each chunk in the cylinder's tangent/up/radial frame.
Crater chunks use UV patches from the cells the observer reports as cleared.
The red flecks use solid colors and the same triangle renderer. Its faces test
scene and actor depth after the ordinary actors and leave scene depth intact.
The level clock ages the presentation and the six body pieces still use the
installed sprite strips. [Task-only patch](../../build/explosion-pieces-3d-001/task-only.patch)

The mesh dimensions, face lighting, terrain-coordinate gravity and lifetimes
are provisional. The clone keeps four crater chunks per eligible site, with
its existing thin-ledge fallback. It gives the 100 red flecks independent
three-axis launch and per-channel color variation from a generator local to the
burst. The red flecks last 0.55–0.97 s and the crater chunks 1.2–1.5 s. The
ten pink fall-splat particles share the solid-fleck draw path and retain their
fan and sprite pieces. Rat victims use `emitForcedDeathFlight` and its rotating
figure. [Rat-victim trace](rat-victim-fling-001.md)

## Registered reference

Static Ghidra 12.1.4 evidence for `Lemmings Revolution.exe`, x86, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
The REA session closed without a snapshot. [Address cross-check and limits](../../build/explosion-pieces-3d-001/static/reference-notes.md)

- **Body pieces.** `408227 -> 401BB0/407560` creates six. `401C50/407600`
  bind Bits and select head, body and paired limbs. `43CAD3` projects their
  sprite frames. Mode 6 (`4932D3`) adds signed random velocity components
  scaled by 50. The life is 4 s and the shared update applies
  `-1400 * dt²` to Y velocity per update.
- **Crater pieces.** `43F510 -> 40C960` configures 12 slots per eligible aligned
  site. `495F97` makes eight volumes and four single faces using terrain
  texture. Mode 4 (`4933A3`) adds components scaled by 10 to the mapped
  Y+10/Z+2 direction. `495E2C/495EBA` advance two rotation angles.
  `40C9BF..40C9F9` sets life to `1 + 1.75*Y/height`.
  `40D5E0` applies gravity and final-life shrink.
- **Red points.** `43F510 -> 402870` configures 100. `492CE5` varies RGB
  independently up to (255,32,32). Mode 7 (`49326B`) uses signed components
  scaled by 100 plus the mapped Y+40/Z+10 direction. `4033C0/4034B0` give
  nominal life 1–3 s and the same per-update gravity form. `492F51..492F66`
  draws a point list. The clone's tumbling red flecks are a deliberate visual
  adaptation. `40B3EC` uses the same point renderer for ten fixed pink points
  with mode 5 and nominal life 2–6 s. The traced blast branches cover body
  sprites, terrain, points and the existing `~splat` surface mark. Native
  effective depth, point appearance and any indirect flame or smoke remain
  unverified. [Surface mark trace](explosion-decal-001.md)

## Checks

- The final GCC desktop and Emscripten release targets built. Two initial Wasm
  attempts resolved to the SDK's default cache, which lacked SDL2. Explicitly
  selecting the existing workspace cache resolved that failure.
  [Desktop build](../../build/explosion-pieces-3d-001/desktop-final-build.log) · [Wasm build](../../build/explosion-pieces-3d-001/wasm-final-build.log) · [SDK cache attempt](../../build/explosion-pieces-3d-001/wasm-build.log) · [Activation attempt](../../build/explosion-pieces-3d-001/wasm-build-local-cache.log)
- The framebuffer probe observed recorded step 391 at 13,621 ms, cell (132,26),
  with 15 cleared cells, one death and 110 particles. Desktop and Wasm each
  submitted 1,248 particle triangles at 255 ms. Nearer scene depth and nearer
  actor depth each hid the entire burst in the occlusion check. Pieces fell
  past the cut edge and the effect expired by 1,645 ms. The Emscripten probe
  ran under Node using the production renderer objects.
  [Desktop frames](../../build/explosion-pieces-3d-001/desktop-frames/) · [Wasm frames](../../build/explosion-pieces-3d-001/wasm-frames/) · [Desktop run](../../build/explosion-pieces-3d-001/desktop-frames.log) · [Wasm run](../../build/explosion-pieces-3d-001/wasm-frames.log) · [Probe](../../build/explosion-pieces-3d-001/frame-probe.cpp)
- In **Just Float** (`MM_FLOAT`), the existing no-input diagnostic showed the
  first fatal fall at tick 104, cell (139,88), with one loss and pink flecks
  among the body sprites. Tick 106 showed the actor dead and the loss at one.
  [Desktop close-up](../../build/explosion-pieces-3d-001/desktop-frames/splat-104-close.png) · [Wasm close-up](../../build/explosion-pieces-3d-001/wasm-frames/splat-104-close.png) · [Desktop ticks 104](../../build/explosion-pieces-3d-001/desktop-splat-104-close.log) [and 106](../../build/explosion-pieces-3d-001/desktop-splat-106-close.log) · [Wasm ticks 104](../../build/explosion-pieces-3d-001/wasm-splat-104-close.log) [and 106](../../build/explosion-pieces-3d-001/wasm-splat-106-close.log)
- All **24 promoted human replays passed on each target**, including Just
  Bomb (step 954, 17/10 saved) and Escape to Victory (step 28060, 43/40).
  Each returned `PASS won=1 completed=1`. Replay checks use the save-free
  path and the clone diagnostics used the private save context.
  [Every recording, result and command log](../../build/explosion-pieces-3d-001/replays.json)

Changed `src/app/DeathParticles.h`, `src/app/TubeScene.cpp`,
`src/app/TubeScene.h` and this report. The scoped whitespace check passed.
[Incoming source and Git HEAD reads](../../build/explosion-pieces-3d-001/entry/)

- **Handoff** `needs_review`. Supervisor smoke is pending.
- **Next action** Review the frame sequence and smoke result, then confirm K4 in play.

**What to check** In **Just Bomb**, assign Bomber to a lemming on a wooden ledge.
Watch at normal zoom, then rotate or zoom in. Expect a scatter of tumbling red
flecks and textured pieces from the crater, with foreground terrain and
nearer lemmings covering fragments behind them. The ledge should open and the
out count should drop once.
