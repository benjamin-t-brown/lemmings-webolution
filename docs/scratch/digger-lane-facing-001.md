# digger-lane-facing-001

**Status** `needs_review`. Supervisor smoke pending. [Playtest 19 J6](../playtest/2026-11-19-playtest-19.md)
refines the shared digger pose from [digger-outward-001](digger-outward-001.md).
The ledger covers Now you're stalking (`E22_KF_1`, node 27) and Escape to
victory (`E11_KF_1`, node 28) and gives no J6 location. The source applies
one outward rotation to every digger. The installed Just Dig route
(`MM_DIG_000`) supplies the focused clone check.

## Original static evidence

REA/Ghidra 12.1.4 read the registered x86 PE
`Lemmings Revolution/Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Profile `3c06738e45e144d5f073edbd3244c6bff46222f389525fe659bc92dca2ccdece`.
Decompilation was cross-checked against instructions, bytes and xrefs.
Analysis closed without snapshot arguments. [Static evidence](../../build/digger-lane-facing-001/static-evidence.json) ·
[Prior change from git show](../../build/digger-lane-facing-001/digger-outward-001.git-show.txt)

- Digger entry `0x4159B4` selects `dig` at `0x4159BD..0x4159C5`.
  `0x4159CA..0x4159DA` passes binary64 `0.75` unconditionally to
  `0x4683E9`, whose stores at `0x4683F6/0x4683FF` write controller
  `+0x1D8/+0x1DC`. Leave `0x415995` clears the phase via `0x4159A9`.
  The phase is independent of prior facing and lane.
- The render at `0x42EEF2` copies the actor position, including radial
  `+0x18`. `0x42EF38..0x42EF44` offsets the sample X by facing
  `+0x40` times `0.1`. It maps the offset sample into model space,
  normalizes the difference from actor position and writes the basis at
  `0x42EF9F`. At `0x42EFD8` it passes the actor-minus-camera vector to
  `0x46840B`. The chooser reads phase at `0x46845C`, rotates the basis,
  and scores the normalized camera vector at `0x46857F/0x46858B/0x4685D4`.
  The native view uses facing and position through that basis and vector.
  The digger's pre-render slot `0x4B0A00` points to the retained no-op
  `0x4011D0`. [Retained basis trace](phase-a3-sprite-view.md)

## Production change

`runApplicationLeg → LemmingAnimTracker::advance → injectedWalkersFromWorld
→ TubeScene::drawInjectedWalkers → prepareInjectedWalker → spriteViewScores
→ selectSpriteView`. The digger draw and hover redraw now apply a half-turn
to the smooth radial reference for the left walking target (inner `0.25`).
The right target (outer `0.75`) keeps its outward pose. Selection still
runs from the camera on every draw. The clone rule is provisional relative
to native fidelity. It uses the walking direction that defines the motion
tracker's `walkingLane` target even during a lane transition, and
applies the half-turn to the clone's smooth basis with its existing
`3/64` sector alignment. The original uses the uniform `0.75`
phase with a facing-dependent sampled basis.

The half-turn applies only to `dig` sprite draws. Rectangle-only requests
from `injectedWalkerScreenBoxes` continue to use the outward frame for
click/hover targeting. Simulation facing, sampled position, timing and
other clips retain their existing inputs and behavior.
[Scene change](../../src/app/TubeScene.cpp#L556) · [Lane rule](../../src/app/LemmingMotion.cpp#L33)

## Checks

- Registered MinGW-w64 GCC/CMake/Ninja built `build/agent-native/main/webolution.exe`.
  [Native build](../../build/digger-lane-facing-001/native-build.log)
- The Just Dig presentation probe assigned Digger to walkers on both
  tracked lanes. Across 32 yaws, three sprite types and two gravities,
  all 384 draws matched the expected installed view and mirror, and all
  384 screen boxes matched the existing outward-frame geometry.
  The probe bypasses terrain visibility to sample both hemispheres.
  [Probe source](../../build/digger-lane-facing-001/inspect-draw.cpp) ·
  [Probe build](../../build/digger-lane-facing-001/inspect-build.log) ·
  [Draw and pick results](../../build/digger-lane-facing-001/inspect-draw.csv)
- All 23 promoted recordings ran via `webolution --game-dir "Lemmings Revolution"
  --replay <file>` and returned exit 0, `PASS`, `won=1`, `completed=1`.
  Just Dig passed at step 1014 (19/10 saved) and Now you're stalking at
  step 24402 (49/49 saved). The index gives every recording, final step and log.
  The headless checks used a private LocalAppData path and opened no save store.
  [Per-recording results](../../build/digger-lane-facing-001/replays.tsv)

Human confirmation and exact original all-angle appearance remain open.

**Handoff** `src/app/TubeScene.cpp` and this report are ready for independent review.\
**Next** Supervisor smoke, then human playtest acceptance.

**What to check**

In Just Dig (`MM_DIG_000`), assign Digger in separate attempts to a
right-moving and a left-moving walker. With the digger on the near side
of the cylinder, expect the outer-lane digger to show its face and the
inner-lane digger to show its back. Rotate past each while it digs and
look for the side views to change with the camera.
