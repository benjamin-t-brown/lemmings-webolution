# Playtest 27 S1 · Lemmings at the lower striped gate

**Needs review.** In **How do we get up there!** (`H02_JC_1`, node 35), lemming 10
disappears while walking past the fully retracted lower gate at replay step
**20571**, 164897 ms, in `H02_JC_1-1795313460665625000-2.replay`. Its simulation
feet are (25,67), Y down. The rendered sample is (23.98,28,0.75), Y up. The
striped gate is id 1, mode 0, at X [17,27), Y [28,63) in file coordinates. The
second reproduction is lemming 41 at step 11095 in
`H02_JC_1-1795314019550068000-9.replay`. The recordings carry no camera, so
the inspector used selected clone views. [Playtest 27 S1](../playtest/2026-11-21-playtest-27.md#defects) · [Recording copies](../../build/metal-wall-occlusion-001/inputs/) · [Inspector](../../build/metal-wall-occlusion-001/inspect.cpp)

| Recorded view | Visible body pixels | Clone framebuffer crops |
| --- | --- | --- |
| Gate open, step 20571, actor 10, yaw 46, zoom 0 | 0 → 119 of 119 | [Before](../../build/metal-wall-occlusion-001/detail-before/20571-10-46-0/20571-10-y46-z0-actor.png) · [After](../../build/metal-wall-occlusion-001/detail-after/20571-10-46-0/20571-10-y46-z0-actor.png) |
| Gate open, step 20533, actor 10, yaw 34, zoom 1 | 0 → 682 of 690 | [Before](../../build/metal-wall-occlusion-001/detail-before/20533-10-34-1/20533-10-y34-z1-actor.png) · [After](../../build/metal-wall-occlusion-001/detail-after/20533-10-34-1/20533-10-y34-z1-actor.png) |
| Gate closed, step 14300, actor 0, yaw 46, zoom 0 | 0 → 0 of 130 | [Before](../../build/metal-wall-occlusion-001/detail-before/14300-0-46-0/14300-0-y46-z0-actor.png) · [After](../../build/metal-wall-occlusion-001/detail-after/14300-0-46-0/14300-0-y46-z0-actor.png) · [Clipping bypassed](../../build/metal-wall-occlusion-001/detail-after/14300-0-46-0/14300-0-y46-z0-unclipped.png) |

At open-gate pixel (190,304), the actor plane is at view Z 337.6308752 and
completed scenery is behind it at 348.6755195. The body footprint erased it
before the scene-depth test because `TubeScene::build` stamps `0x403` across
the gate's closed rectangle to keep the retractable mesh topology.
`runApplicationLeg::prepareScene → syncCounterGates` now supplies the same
eased radial fraction to the footprint that it uses to move the gate vertices.
`drawInjectedWalkers → prepareInjectedWalker → clipBodyFootprint` tests the
volume from the inner shell to that position. Full retraction has zero volume.
`clipActorToScene` still tests completed scene and pipe depth, including a gate's
remaining visible faces. The closed-gate control is a faller in
`H02_JC_1-1795314898851260600-2.replay`, hidden by tube depth. Hover redraw
and effects that share the footprint use the same gate fraction.
[Matched-pixel log](../../build/metal-wall-occlusion-001/detail-before/20571-10-46-0/pixels.tsv) · [Crop comparison](../../build/metal-wall-occlusion-001/detail-compare.tsv) · [Gate sync](../../src/app/TubeScene.cpp#L1939) · [Footprint](../../src/app/ActorOcclusion.h#L54)

The footprint is a **provisional clone presentation rule**. Static REA/Ghidra
12.1.4 on the registered x86 EXE (SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`) matches
pseudocode to the easing branches at `0x427016–0x427053` and the vertex scale/add
calls at `0x427097` and `0x4270B5` in `0x426FEC`. Actor submit sets an
origin-depth bias at `0x43CCB5–0x43CCCA` and calls the sprite renderer at
`0x43CD4E`. The effective original device depth state remains unresolved.
REA closed without a snapshot. [Static packets](../../build/metal-wall-occlusion-001/rea.json) · [Retained device-state evidence](phase-c3q-scene-device-state.md)

## Coverage and checks

- The installed census lists **231 gates across 52 levels**, including 209
  switch-only and 22 quota gates. Both H02 gates use mode 0. The 19 recording
  scans sampled five yaws per pose near the gates and the steel ledge. All
  93 completely hidden open-gate samples regained visible pixels. Visibility
  counts matched for 5,100 closed-gate views and 975 steel-ledge views. There
  were no nearby recorded actors at the upper gate or at a moving gate.
  An isolated pose in each H02 gate confirmed the footprint admits layer
  0.5 at thicknesses 0 and 0.25 and masks it at 0.75 and 1.
  [Installed level list](../../build/metal-wall-occlusion-001/gate-census.tsv) · [Scan comparison](../../build/metal-wall-occlusion-001/scan-compare.tsv) · [Moving-gate pose check](../../build/metal-wall-occlusion-001/footprint.tsv)
- All 100 settled and final PNG pairs for the 50 gate-free levels matched
  byte for byte using the level sweep driver with scenery seed `0x610ecafe`.
  Both active promoted wins on gated levels passed, **The Iron Curtain**
  (step 18207, 21/20) and **Wheelbarrows of Doom** (step 24891, 99/98).
  All 19 H02 recordings also passed the replay checker and ended without wins.
  Clone runs used task-local LocalAppData.
  [Sweep comparison](../../build/metal-wall-occlusion-001/sweep-compare.tsv) · [Individual CLI replay results](../../build/metal-wall-occlusion-001/replays.tsv)
- The final `smoke_tests` build passed, as did eight focused smoke-labelled
  tests. The legacy gate test compiled with a regression case for the eased
  footprint and awaits milestone sign-off. The initial build caught two
  additional footprint callers, now on the same path.
  [Smoke build](../../build/metal-wall-occlusion-001/smoke-target-build.log) · [Focused smoke results](../../build/metal-wall-occlusion-001/focused-smoke.log) · [Legacy test build](../../build/metal-wall-occlusion-001/legacy-target-build.log)

Handoff **needs_review**. The supervisor smoke and recovery profile is pending.  
Next action is independent review, then the human playtest.

**What to check.** In How do we get up there!, open the lower striped gate
and watch walkers cross at its foot. Rotate to either side of the opening.
Walkers should stay visible across the retracted gate. The closed gate
should still hide lemmings on the far side.
