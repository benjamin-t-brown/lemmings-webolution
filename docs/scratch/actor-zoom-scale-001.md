actor-zoom-scale-001 — **needs_review; supervisor smoke pending**

Playable v1, implementation-plan playtest-ledger work order. Before: human
[Playtest 12 B1](../playtest/2026-11-17-playtest-12.md) reports terrain growing
while lemmings stay the same screen size. Installed route: **Just Climb,
MM_CLIMB_002**. Correct shared sprite/hover sizing is the next presentation
dependency. Difficulty routine; recommended Sonnet; assigned Codex override used.

Production path: `CameraController::update` toggles `CameraParams::zoomShift`;
`runApplicationLeg::renderFrame -> buildCamera -> drawInjectedWalkers ->
prepareInjectedWalker -> actorSpritePlacement -> drawActorSprite`. Previously
the origin was perspective-projected, but the SPR rectangle always used native
scale delta -1 (half the stored pixels), independent of depth or zoom. There
was no actor-height projection or actor-size clamp in that path.

Changed `Camera.{h,cpp}` stores the view-Z difference to zoom 0 at the same
yaw, pitch, level height and intro phase, undoing zoom-only elevation too.
`ActorSprite.{h,cpp}` scales all four offsets about the existing projected
origin by **d0 / d**, where d0 and d are that point's physical view depths at
default/current zoom. This is the terrain's perspective magnification ratio,
not a blanket 2x factor. At zoom 0 it returns the original rectangle directly,
including float rounding, asymmetric SPR offsets and intro behavior. The
semantic projection overload retains the recovered native arithmetic.

`injectedWalkerScreenBoxes -> hoveredActor -> pickNearestScreenLemming` feeds
both ordinary hover and click assignment; `drawHoveredWalker` prepares the
same scaled rectangle. Gravity reflection follows scaling. The existing
upright-plane, body-footprint, scene/pipe depth and actor-coverage rules still
operate on the resulting pixels; placement depth and the traced normalized
0.0003 origin bias are unchanged. No simulation or save behavior was changed.

The builder's carried brick already projects eight world-space vertices
through the terrain camera; its pack anchor now comes from the scaled body
rectangle. Cut debris likewise already projects rotated world-space cuboids,
so neither mesh receives a second scale. Body-piece death sprites share
`actorSpritePlacement` and now scale automatically. `TubeScene.cpp` applies
the existing chip-radius clamp at reference zoom before magnifying it, keeping
the default size while removing the zoom-time size ceiling/floor. Drowning
bubble sprites also inherit the shared placement correction. Countdown text
keeps its existing readable font size and follows the scaled actor top.

Static evidence started with accepted A4/A5 actor placement, hatch input,
builder-brick, death/debris and subsequent actor-occlusion reports. Used the
reverse-engineer-anything skill and static REA/Ghidra 12.1.4, only on registered
`Lemmings Revolution/Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Full returned packets are in [static/evidence.json](../../build/actor-zoom-scale-001/static/evidence.json),
profile `3c06738e45e144d5f073edbd3244c6bff46222f389525fe659bc92dca2ccdece`.
`43CC68` pseudocode was checked against instructions: transform call `43CC89`,
reciprocal depth `43CCFE`, projected X multiplication `43CD13`, sprite call
`43CD4E`. `47A75D` instructions confirm global scale subtraction at
`47A7CD..47A7D8`, half-size divisions `47A8AE..47A8E1` and doubled dimensions
`47A91F..47A93B`, with the retained mirror/offset asymmetry. Evidence IDs:
`ev_0fc271b435b10a844d45e665b97bf248628b7f01ab9bd21a64416ae76d58f60f`,
`ev_9a8209aba409c9745a27529fbccfc174c42f9a83172b48c6c3544c73701ba1e3`,
`ev_975579ddb61a15d5a2a8d9ff64a1b9c439dd7697c786d6f3f8da437ee76db9eb`.
Closed REA without snapshot arguments. No original execution occurred.

**Provisional/unknown:** d0/d scaling is the requested clone policy, rather
than the original's discrete global scale mechanism. Original filtering,
device depth and full-frame fidelity remain unresolved. Geometry/picking
units and headless replay do not establish visible appearance or human
playability; the next human zoom/occlusion playtest remains acceptance.

Registered UCRT64 GCC/CMake/Ninja built with eight jobs and the existing
`-O2 -g` assertions configuration: [build.log](../../build/actor-zoom-scale-001/build.log).
Five focused checks pass in 0.51 s: **unmodified clicktoassign**, actorsprite,
builder_brick, debris_3d and new legacy actor_zoom; see
[focused.log](../../build/actor-zoom-scale-001/focused.log) and
[JUnit](../../build/actor-zoom-scale-001/focused.xml).
The new 0.025 s unit guards reference-depth arithmetic over 864
depth/yaw/elevation/height/intro/mirror cases, both gravities, default-size
preservation, shared pick rectangles, raster bounds and physical origin depth.
Its first raster sample used an offscreen zoomed anchor; corrected only that
new synthetic sample to the viewport centre. Initial failure is retained in
`focused-initial.log/xml`; existing assertions/timeouts remain untouched.
No replay fixture, frozen build or per-task gameplay route was added.

All **19 active promoted human-win recordings** ran sequentially with the
rebuilt `build/agent-native/main/webolution.exe --game-dir "Lemmings Revolution"
--replay <file>`. Each exited 0 with PASS, won=1, completed=1. Exact recording
paths, steps and individual logs: [replay-results.json](../../build/actor-zoom-scale-001/replay-results.json).

| Level | Final step | Saved/needed | Result |
| --- | ---: | ---: | --- |
| MM_BASH_002 | 1844 | 10/10 | PASS |
| MM_BASH_MINE_WLR1 | 2272 | 20/10 | PASS |
| MM_BLOCK_001_A | 1619 | 24/15 | PASS |
| MM_BLOCK_BOMB_A | 1546 | 12/10 | PASS |
| MM_BOMB | 954 | 17/10 | PASS |
| MM_BUILD | 1685 | 20/10 | PASS |
| MM_CLIMB_002 | 1252 | 10/1 | PASS |
| MM_DIG_000 | 1014 | 19/10 | PASS |
| MM_FLOAT | 978 | 10/1 | PASS |
| MM_MINE | 1137 | 20/10 | PASS |
| MM_W_012 | 1742 | 16/10 | PASS |
| MM_E1_3_SKILLS | 2095 | 10/5 | PASS |
| M06_KF_1 | 1544 | 91/50 | PASS |
| MM_E1_4SKILLS | 2506 | 20/10 | PASS |
| MM_E2_4SKILLS | 2142 | 14/10 | PASS |
| MM_BASH_DIG_001_A | 3241 | 20/10 | PASS |
| MM_BUILD_BLOCK_A | 2482 | 15/10 | PASS |
| E19_KF_2 | 6034 | 24/20 | PASS |
| E20_MG_1 | 12810 | 7/5 | PASS |

No divergence or recording retirement. Mandatory smoke verification is pending
supervisor execution after this worker exits; the full gate was not duplicated.
Build/test/replay processes used a private `build/actor-zoom-scale-001/appdata`
LOCALAPPDATA; these checks open no save sink. Installed and normal user saves
were not imported, reset or written. Parent supervisor PID 46052 owns the
RepositoryLease. CIM metadata was denied; Get-Process found no clone/build/test
runner at entry. Backups are under `build/actor-zoom-scale-001/pre-edit/`.
Incoming acid-drain CMake/World edits, report/test, original log and protected
queue/status edits were preserved. No protected controls were edited.

Handoff: shared actor/effect sizing follows terrain zoom; default sizing, click picking and all 19 promoted wins pass focused checks.
Next: supervisor smoke and independent review, then the user's zoom playtest; status remains needs_review.

**What to check:** Just Climb (MM_CLIMB_002), let walkers emerge, toggle zoom in
and out and rotate/raise/lower the camera. Walkers should grow/shrink with the
nearby terrain and return to their previous default-zoom size; hover glow and
click-to-assign should follow the displayed body, with terrain/pipe occlusion
preserved. On Just Build, zoom during building: the carried brick should stay
with the hands and meet the stair. On Just Bash/Just Mine/Just Dig and Just
Bomb, zoom while cutting/exploding or falling fatally: chunks and body pieces
should scale with the terrain and stay behind foreground geometry.
