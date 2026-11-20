# laser-switch-blend-001

Playtest 22 M1 reported incorrect laser switch transparency on **Designed with
love** (node 33, `COMPILED\M11_KF_1.LVL`). `BackdropScene::build` admitted the
pads to `LegacyUnattributed`, before the tube. The generic mesh options in
`renderStage` enabled blending only for selector 1, so the beam and glow
surfaces replaced the scene colour wherever they passed depth. [Playtest 22](../playtest/2026-11-19-playtest-22.md)

## Static evidence

The level stores three type-0 `pressure_pad.a3d` gadgets. Each owns one
`null2` model tree with a `cube4` base, four `face2*cut` beam nodes and eight
`face4*cut` glow nodes. The material slots are zero. There are no type-1 lever
records on this level. [Installed record census](../../build/laser-switch-blend-001/census.tsv)

| Pad ID | Placed X, Y | Model root offset | First beam / glow material record |
| --- | --- | --- | --- |
| 0 | 163, 63 | `0xD7BB1` | `0xD7CBA` / `0xD9147` |
| 1 | 245, 72 | `0xE3B43` | `0xE3C4C` / `0xE50D9` |
| 2 | 83, 10 | `0xEFAB5` | `0xEFBBE` / `0xF104B` |

Under `Models\pressure_pad\`, `laser` and `laser_light` serialize selector **2**
and corner alpha **255**. `metal_tripswitch` serializes selector **0**. All three
basenames map to texture flags **0** at `0x4743F8..0x47448C`. The base archive
supplies the 64×64 `laser` and 8×8 `laser_light` BMPs with no sibling `.ALF`.
The clone decoder yields alpha 255 for both and counts 1,903 black beam texels.
With flags 0, `0x4728CA..0x4728DC` disables alpha testing and
`0x47293F..0x47296F` selects texture alpha. The material selector reaches
`0x414D80` via `0x43BDDD`. `0x414DA5` enables blending and the selector-2
branch at `0x414DBB..0x414DED` sets **ONE / ONE**. Those factors add the
modulated source RGB regardless of alpha. The flipper's flag **4**, selector
**3** instead modulates texture and diffuse alpha and uses **SRCALPHA / ONE**.
[Texture and blend instructions](../../build/laser-switch-blend-001/rea/) · [Flipper evidence](flipper-effect-blend-001.md)

Tagged dispatch `0x433DD9` constructs the pad at `0x433ED2` via `0x431837`.
Its vtable `0x4B1148` has `0x431B30` at `+0x18` and `0x4323EB` at `+0x10`.
The reader reaches `0x432283` through `0x433CEE → 0x43130E`, loading the model
and registering the gadget. The tube walker calls the draw slot at
`0x43A9BB` or `0x43AA47`. `0x432409` calls model slot `+4` (`0x451865`),
which enters the shared holder, node, and mesh draw path that reaches
`0x43BD80`. The draw and material path inherit depth state. The effective
wrapper Z-write and depth comparison state remain unverified. The retained
depth scan found no literal Z-write or comparison call in the EXE.
[Dispatch instructions](../../build/laser-switch-blend-001/rea/) · [Depth scan](phase-a1-depth-clipping.md)

Ghidra 12.1.4 examined the registered x86 `Lemmings Revolution/Lemmings Revolution.exe`
with SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
The static session closed without a snapshot.

## Clone change and checks

`runApplicationLeg → scheduleSceneFrame → BackdropScene::renderStage` now draws
the flag-0, selector-2 pad meshes through the existing post-tube effect stage
and `blendOptions`. The pass uses the original blend factors and texture alpha
operation. Post-tube compositing and the shared `Less` test with depth writes on
are provisional equivalents for native ordering and inherited device state.
The opaque base uses its existing legacy path. The `advanceScenery → prepareScene →
setPressurePadStates` path still supplies the authored clip, restarts it on
activation and hides the `cut` subtrees after the existing feedback interval.
[Pad animation evidence](pad-pipe-presentation-001.md)

The census found the same pad material pair on **45 pads across 23 levels**.
Other flag-0, selector-2 scenery includes the mine `WaterRef01/02` meshes
and the tree `light02` billboards, which already blend in the secondary pass.
`teleport_swirl_3` shares the material flags but is part of the teleporter's
ordered base and FX draw at `0x434AAF`. The pad pass is scoped to the traced
pad and flipper owners. [Census summary](../../build/laser-switch-blend-001/census-summary.txt)

The production framebuffer at fixed yaws 163, 245 and 83 shows the effect over
the scene. Frame 0 changed 63, 64 and 123 pixels respectively, all within the
sampled post-tube pad coverage. Later frames show the moving glow. A separate
renderer probe at yaw 83 counted 53 beam fragments preserving the nonblack
destination and seven brightening it at the initial sample. The fired sample
kept the clip visible, and the settled sample drew the base with zero beam or
glow fragments. [163 before](../../build/laser-switch-blend-001/before/yaw-163-frame-0.png) · [163 after](../../build/laser-switch-blend-001/after/yaw-163-frame-0.png) · [245 before](../../build/laser-switch-blend-001/before/yaw-245-frame-0.png) · [245 after](../../build/laser-switch-blend-001/after/yaw-245-frame-0.png) · [83 before](../../build/laser-switch-blend-001/before/yaw-83-frame-0.png) · [83 after](../../build/laser-switch-blend-001/after/yaw-83-frame-0.png) · [83 after frame 2](../../build/laser-switch-blend-001/after/yaw-83-frame-2.png) · [Pixel and state audit](../../build/laser-switch-blend-001/frame-summary.txt)

The registered UCRT64 native build succeeded. All **28** promoted recordings
returned `PASS`, `won=1` and `completed=1` with exit 0. The current promoted set
has no recordings on the 23 affected levels. The frame probe used no save
store and the probes and replays used a private LocalAppData.
[Native build](../../build/laser-switch-blend-001/native-after-build.log) · [Before frames log](../../build/laser-switch-blend-001/frames-before.log) · [After frames log](../../build/laser-switch-blend-001/frames-after.log) · [Per-recording results](../../build/laser-switch-blend-001/replay-results.tsv)

Changed `src/app/BackdropScene.cpp`, `src/app/BackdropScene.h`, `src/app/main.cpp`
and this report. The supervisor smoke gate is pending.

Handoff — `needs_review`.
Next action — Supervisor smoke gate, then independent review.

What to check — On **Designed with love** (node 33), rotate to the three laser switches and send a lemming through one. The red beam and glow should animate with the scene visible through them. After activation feedback, the light should go out and the metal base should stay visible.
