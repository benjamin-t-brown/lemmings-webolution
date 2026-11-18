# One Way Ticket backdrop

**Needs review. Supervisor smoke pending for the sky revision.** Node 22 (`MM_ALL_8_001`),
opening view. Playtest 13 C5 identified the black water and the displaced
scenery. [Playtest ledger](../playtest/2026-11-17-playtest-13.md)

## Cause and production path

`runApplicationLeg` builds `BackdropScene` from the compiled level and draws
its primary holder before the tube and its sorted secondary holder afterward.
The primary `null2` at `0x5D3C2` owns `transBraiseOuter8` at `0x5D443`.
`Rock03` on that node owns pixels on the displaced purple slab. The secondary
`null2` at `0x9CAB9` owns `transWater` and `transWater2` (`~Water01`, selector 1),
`transWater1_cutme` at `0xA3580` (`WaterRef01`, selector 2), and nine
`WaterRef02` rock-reflection nodes (selector 2). The material paths all start
with `Models\bg_mine01\`. The before raster probe attributed 124,220 very dark
pixels to `transWater1_cutme` alone in a backdrop-only diagnostic view.
[Before attribution](../../build/oneway-backdrop-001/before/attribution.txt)

- `BackdropScene::build` now applies each paired holder's existing serialized
  mirror flag to its mine backdrop root. The stored Z basis, matrix elements
  6–8, is negated once and feeds mesh projection, billboard placement, and
  secondary sorting. Static `0x437D83..0x437DBD` and `0x437E19..0x437E56`
  read the two flags and pass `(1,1,-1)` to `0x4381F0`; instructions at
  `0x438264..0x438291` confirm the three Z-basis stores. Translation is outside
  these stores. Both flags are 1 in the installed One Way Ticket holders.
  [Accepted holder evidence](scene-roots-001.md)
- `BackdropScene::renderStage` now passes the mine material's authored blend
  selector and texture alpha policy to `rasterizeSceneTriangle`. The old path
  only blended selector 1, so the nearly black `WaterRef01` texels replaced
  the destination. `0x469D92` and `0x469ECB` call `0x43BD80`, which binds the
  texture at `0x43BDCA` and passes `material+4` to `0x414D80` at `0x43BDDD`.
  The latter requests `ONE/ONE` for selector 2 at `0x414DC1..0x414DEA` and
  `SRCALPHA/INVSRCALPHA` for selector 1. `0x474430..0x474480` reads the leaf
  prefix; `0x47261F` selects texture alpha except for `~` and `+`, which
  modulate it. `_` applies the keyed alpha test and `^` uses its ALF plane.
  [Accepted material evidence](phase-c3q-scene-device-state.md)
- The ledger requests dark blue sky. The supplied screenshot samples black
  at exposed sky coordinates `(624,6)` and `(620,31)` on a normalized 640×480
  grid. Static instructions at `0x45D905..0x45D90C` pass zero to the setter.
  Its pseudocode stores the value at device offset `0x73C`. The frame calls
  the clear routine at `0x45DADA`, and instructions at `0x470ECC..0x470EF0`
  load and pass that value to the device. The requested clone sky is
  provisional RGB `(4,8,24)`. After the scene passes, `runApplicationLeg` fills
  only black pixels still at clear depth, scoped to the resolved One Way
  Ticket level with the backdrop enabled. Water and reflections blend
  against the black clear. Actors and HUD draw afterward.
  [Reference](../playtest/reference/playtest-13/one_way_ticket_orig.jpg)
  [Screenshot samples](../../build/oneway-backdrop-001/pixels.txt)
  [Static clear trace](../../build/oneway-backdrop-001/sky-revision-rea.json)

Static REA/Ghidra 12.1.4 used the registered x86 EXE, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`, profile
`3c06738e45e144d5f073edbd3244c6bff46222f389525fe659bc92dca2ccdece`.
The retained results include pseudocode and instruction cross-checks for
loading, the two frame passes (`0x441879`, `0x44199B`), material binding,
blend, and clear. Analysis closed without a snapshot. [Static evidence](../../build/oneway-backdrop-001/rea/)
[C5 introduction](../../build/oneway-backdrop-001/sky-revision-git-show.txt)

The mirror and per-material policy are scoped to the shared mine scenery.
Existing software `Less`/write-on, linear sampling and the `0xEF` key threshold
remain conditional clone choices. An additive write-off trial also restored
the water; the selected version uses the existing depth policy. Effective
original depth/device state, `cutme` frame-time visibility, secondary scenery
UV animation, and clipping are outside this result. [Depth-policy trial](../../build/oneway-backdrop-001/mirror-blend/MM_ALL_8_001-start.png)

## Clone observations

The before/after pairs use the ordinary level renderer at simulation tick 0,
640×480, collision height 96, and the serialized selected hatch's settled yaw.
They are headless scene shots. The user's reference also contains HUD and a
lemming. Shots used isolated `LOCALAPPDATA` and the clone replay path opens no
save. The original executable was read only.

| Level | Yaw | Matched opening frames | Observation |
| --- | ---: | --- | --- |
| `MM_ALL_8_001` | 34 | [Before](../../build/oneway-backdrop-001/before/MM_ALL_8_001-start.png) / [Black sky](../../build/oneway-backdrop-001/after/MM_ALL_8_001-start.png) / [Final](../../build/oneway-backdrop-001/sky-revision-final/openings/MM_ALL_8_001-start.png) | Green water and a cylinder reflection fill the foreground under navy sky. The mound, crystals and bridge sit to the right. |
| `MM_DIG_000` (Just Dig) | 84 | [Before](../../build/oneway-backdrop-001/before/MM_DIG_000-start.png) / [Corrected](../../build/oneway-backdrop-001/after/MM_DIG_000-start.png) / [Final](../../build/oneway-backdrop-001/sky-revision-final/openings/MM_DIG_000-start.png) | Water replaces the dark foreground. The zigzag route, hatch and balloon remain legible. The corrected and final frames are pixel-identical. |
| `MM_BOMB` (Bomber) | 162 | [Before](../../build/oneway-backdrop-001/before/MM_BOMB-start.png) / [Corrected](../../build/oneway-backdrop-001/after/MM_BOMB-start.png) / [Final](../../build/oneway-backdrop-001/sky-revision-final/openings/MM_BOMB-start.png) | Water, crystals and torches are visible. Differences between the corrected and final frames are confined to the animated torches. |

The two comparisons showed no new visible regression at these poses. The
One Way Ticket render had zero missing named textures before and after. Water
at `(628,308)` went from RGB `(1,1,1)` to `(15,67,39)` with the backdrop
fix, close to the normalized reference `(18,64,36)`. The sky revision changed
5,220 black pixels to navy. The other 43 changed pixels fall within the
clock-seeded torch flame. The exposed sky sample with `--scene-off` is black.
[Pixel comparison](../../build/oneway-backdrop-001/sky-revision-final/pixel-check.txt)
[Opening invocation](../../build/oneway-backdrop-001/sky-revision-final/openings/MM_ALL_8_001-start.log)

The effective archive census lists all **70 other levels** sharing the affected
roots and named materials.
Each has the same 24 primary and 66 secondary mine material references, one
`WaterRef01`, nine `WaterRef02`, and both mirror flags set. The mirror and
material correction applies to all 71. The navy applies to One Way Ticket only.
[Effective archive census](../../build/oneway-backdrop-001/shared-backdrops.csv)

## Checks and handoff

The registered UCRT64 `webolution` target built successfully. All 20 promoted
recordings reproduced their wins with exit 0, `PASS`, `won=1`, and
`completed=1`. One Way Ticket saved 46/40 at step 13812.
[Build run](../../build/oneway-backdrop-001/sky-revision-final-build.log)
[Each replay result and invocation log](../../build/oneway-backdrop-001/sky-revision-final/replays.tsv)

The prior supervisor gate passed 95 main smoke tests and 84 recovery tests.
The revised build awaits its supervisor smoke gate.
[Prior supervisor run](../../build/agent-runs/loop-8a26fce99099478a9dd5a0d1ebfff7de/0001-verify-922ab8/verification.log)

The sky revision is in `src/app/main.cpp` and the retained backdrop correction
is in `src/app/BackdropScene.cpp`.

One Way Ticket's opening shows the corrected mine scenery, green water, and provisional navy sky.\
Handoff to supervisor smoke and independent review, then human playtest confirmation of C5.

**What to check**

Open node 22, **One Way Ticket**, and let the opening camera settle. Expect
continuous green water and a cylinder reflection, with the sandy mound and
pink crystals beside the bridge on the right and dark navy sky above them.
Rotate the cylinder and check that the water stays beneath the route.
