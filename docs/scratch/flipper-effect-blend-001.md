# flipper-effect-blend-001

Playtest 21 L3 reported opaque gravity flippers on **2 sides to every story**
(node 30, `COMPILED\M08_KF_1.LVL`). The clone put the tagged type-3 model in
`LegacyUnattributed` before the tube and enabled mesh blending only for selector
1 on that path. The selector-3 coils replaced the wall pixels. [Playtest ledger](../playtest/2026-11-19-playtest-21.md)

## Static evidence

- `M08_KF_1` has placed roots `0xC1B13` and `0xC851A`. On each, preorder nodes
  1, 2, 4 and 5 use material slot 0, `Models\anti_grav\~shaft_antigrav`, with
  serialized blend selector **3**. On the first root the material records start
  at `0xC1F64`, `0xC313B`, `0xC44DB` and `0xC56B8`. The beam corners carry
  diffuse alpha 0–255 and the reflection corners 0–128. The panel uses selector
  0. The base archive's 32×32 shaft BMP has 512 black texels and no matching
  `.ALF`. `0x474430..0x474485` maps the `~` basename to flag **4**. The `.ALF`
  gate at `0x472328` tests `0x18`, and the colour-key paths in `0x472F99` test
  flags 1 and 2. [Material fields](../../build/flipper-effect-blend-001/materials.tsv) · [Texture audit](../../build/flipper-effect-blend-001/textures.tsv)
- The original upload routine at `0x472F99` checks for a nonzero alpha mask at
  `0x4736B6` and a 16-bit surface at `0x4737B3`. In that case the flag-4 loop
  at `0x473A8D..0x473AE2` ORs the alpha mask into nonzero texel words and leaves
  zero words untouched. The surface format selected for this shaft texture and
  its resulting uploaded alpha are unverified. [Upload instructions](../../build/flipper-effect-blend-001/rea/472f99-asm.json)
- `0x41D2B0` installs vtable `0x4B0D80`. Reader `0x41D730` reaches `0x432283`,
  which loads the model at `this+0x44` and registers the gadget with `0x43B517`.
  Tube walker `0x43A4EC` dispatches slot `+0x10` at `0x43A9BB` or `0x43AA47`
  to `0x4323EB`. Its model slot `+4` reaches `0x451865`, holder `0x47496D`,
  node draw `0x46C55D` and mesh dispatch `0x469D71` or `0x469EAA`. Binder
  `0x43BD80` passes the texture to `0x47261F` and the material selector to
  `0x414D80`. For flag 4 the binder disables alpha test (`0x4728DC`) and
  modulates texture and diffuse alpha (`0x4728E5` branch). Selector 3 enables
  blending with **SRCALPHA / ONE** (`0x414DA5`, `0x414DEF`, `0x414E05..0x414E1E`).
  Depth writes are inherited along this path. Effective wrapper depth state is
  unverified. [Ghidra instructions, pseudocode and xrefs](../../build/flipper-effect-blend-001/rea/) · [Retained depth scan](phase-a1-depth-clipping.md)

The registered x86 reference is `Lemmings Revolution/Lemmings Revolution.exe`,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Ghidra 12.1.4 examined it statically and closed without a snapshot.

## Clone change and checks

`runApplicationLeg` → `scheduleSceneFrame` → `BackdropScene::renderStage` now
draws the tagged gravity coils after the opaque tube through the existing mesh
rasterizer and `blendOptions`. The stage uses modulated alpha with alpha test
off. The clone provisionally uses the decoder's alpha of 255 for every shaft
BMP texel. With texture alpha fixed at 255, diffuse alpha controls source
alpha, and black samples contribute zero RGB under additive blending. Post-tube
compositing and the existing `Less` test with depth writes on are the clone's
provisional equivalents for native tube ordering and inherited depth.
Opaque panels stay in the legacy pass. `advanceScenery` and
`animateGravity` still supply the same UV animation. The edit is confined to
`src/app/BackdropScene.cpp`, `src/app/BackdropScene.h` and `src/app/main.cpp`.

The installed scenery census found 50 flippers across 18 levels. Mine scenery,
sand water, tree mist and pipe materials also use flag 4 with selector 1.
The teleport FX `~shaft` and speed-pad `~speedup` share flag 4 and selector 3.
Their models have separate presentation paths. The post-tube change is scoped
to the gravity roots covered by this playtest. [Census](../../build/flipper-effect-blend-001/census-summary.txt)

The source-built application rendered both M08_KF_1 placements at fixed camera
yaws 151 and 172 and 100 ms frame steps. Matched before/after frames differ only
within observed coil coverage, by 2,494 and 2,510 pixels respectively. After
frames 0 and 2 show moving bands on both coils. The after-pass write audit shows
the additive channel increases and unmodified wall pixels through dark texels.
[Yaw 151 before](../../build/flipper-effect-blend-001/before/pad-0-frame-0.png) · [Yaw 151 after](../../build/flipper-effect-blend-001/after/pad-0-frame-0.png) · [Yaw 151 after frame 2](../../build/flipper-effect-blend-001/after/pad-0-frame-2.png) · [Yaw 172 before](../../build/flipper-effect-blend-001/before/pad-1-frame-0.png) · [Yaw 172 after](../../build/flipper-effect-blend-001/after/pad-1-frame-0.png) · [Pixel summary](../../build/flipper-effect-blend-001/frame-summary.txt) · [Write audit](../../build/flipper-effect-blend-001/after-blend-summary.tsv)

The [native build](../../build/flipper-effect-blend-001/build-after.log) succeeded.
All 25 promoted recordings passed with `won=1 completed=1`. The affected route,
`M06_KF_1-1794888660414351700-3.replay`, finished at step 1544 with 91/50 saved.
The frame probe used no save store and replays ran with a private LocalAppData.
[Per-recording results and logs](../../build/flipper-effect-blend-001/replay-results.tsv)

The prior supervisor run passed 95 smoke tests and 84 recovery tests. The
supervisor smoke gate for this report revision is pending. [Prior supervisor log](../../build/agent-runs/20261120T002547-50436-76f10c68/verification.log)

Handoff — `needs_review`.
Next action — Supervisor smoke gate, then independent review.

What to check — On **2 sides to every story** (node 30), rotate to both flippers and watch a lemming cross. The blue bands should scroll with the wall visible through them, and the lemming should still flip gravity.
