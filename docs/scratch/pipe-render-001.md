# pipe-render-001 — drain and supply pipe presentation

Status: **needs_review**. Playable v1, playtest 8 X2: the human saw no drain
on Seafood Sarnie (`MM_BLOCK_BUILD_BASH`, node 15). Adding its existing model
is the next presentation dependency after the accepted pressure-pad and crab
work. No simulation code changed. Difficulty: routine; recommended model:
Sonnet (existing parsed meshes and scene depth path).

## Evidence and production path

- **Human before:** [playtest 8, X2](../playtest/2026-11-16-playtest-8.md).
  Retained starting points: [fluid research, 6.2/6.4](fluid-surface-research-001.md),
  [pressure pad](pressure-pad-drain-001.md), [crab](crab-hazard-001.md).
- **Static EXE:** SHA-256
  `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`,
  repo-local registered reference, Ghidra 12.1.4. Returned evidence is retained
  under `build/pipe-render-001/static/`. `decompile.json` (Evidence
  `ev_76734f65106aed265778e28abcbe9fd36a648e773bef5f063ca4a86c23b2a292`)
  shows `0x42ACAC` reads the two optional models, while `0x42B24D` updates
  the base fluid, overlay and spray without modifying the pipe models.
  `pipe-draw.json` and `pipe-draw-asm.json` independently agree:
  `0x42C560/0x42C57D` test/draw `+0x160`; `0x42C583/0x42C5A0`
  test/draw `+0x164`. There is no height/request/flow predicate in this draw.
  `frame-order-asm.json`: tube call `0x441884`, pipe call `0x4418C2`,
  then base-fluid virtual call `0x44190D`. These are static instructions and
  pseudocode, not observed execution. Analysis was closed without a snapshot.
- **Installed data:** `build/pipe-render-001/census.txt` finds **37 drainage
  models in +0x160 and 34 supply models in +0x164, on 35 levels**. Every
  inline tree and named texture resolves; no pipe has an auxiliary animation
  track. Unresolved pipe levels: **none**. Seafood Sarnie gate index 4 / id 4
  stores `pipe_drainage.a3d`, translation `(17.3334,14.5,-57.1857)`.
  MM_W_012 gate index 11 / id 6 stores `pipe_supply.a3d`, translation
  `(31.7076,9.5,-38.6273)`; it has four supplies overall. Full orientation
  matrices are in the census and are retained exactly, including reflection.
  Third model doubles are 0.0003 for drains and zero for supplies. Their
  model+0xF8 projection-bias role is retained static evidence at `0x451865`
  ([fluid research, 6.2.7](fluid-surface-research-001.md)).

`runApplication -> BackdropScene::build -> read_compiled_level ->
entities_42acac[i].model_160/model_164 -> readSceneNode/buildSceneMeshSource`
now admits both pipe models as `Stage::FluidPipe` roots. It preserves the
authored root/child transforms, geometry, materials and model projection bias.
No placement is synthesized from the collision box or current water height.

`main.cpp::renderScene -> scheduleSceneFrame` draws the pipe stage after
`TubeScene::render`, before the fluid surface and secondary scenery.
The existing scene triangle raster uses Less/write-on and the same physical
depth units as actors. The drowning scenery-depth snapshot is taken **after
pipes**, so drowning actors retain opaque pipe occlusion while their existing
water exception remains. Ordinary `TubeScene::drawInjectedWalkers ->
clipActorToScene -> actorPhysicalDepth` tests actors against the completed
scene buffer; a foreground actor passes the usual depth comparison. Pipes
are never painted over the completed actor layer. `BackdropScene::render`
also includes the new stage; billboard collection selects the requested stage
so existing legacy billboards are not drawn again in the pipe pass.

## Checks and limitations

- UCRT64 production build passed: `build/pipe-render-001/build.log`.
  Existing `backdropscene` test passed, **1/1, 0.16 s**:
  `build/pipe-render-001/unit-ctest.log` (build: `unit-build.log`).
- Installed-data/render inspection `build/pipe-render-001/inspection.txt`
  admits **71/71** pipe roots on all **35/35** affected levels, preserves each
  stored root matrix/source extent, and reports zero rejected pipe records or
  missing textures. Rendering against a nearer filled depth buffer changes no
  color pixels. This is a bounded synthetic depth/data check, not natural play.
  Its temporary driver initially missed `Box.h`; that include was corrected
  before the successful build/run (`inspection-build.log`).
- Production clone images inspected: `build/pipe-render-001/Seafood-Sarnie.png`
  (existing crab camera, yaw 116) shows the round grilled drain at the right
  of the blue pool; `MM_W_012.png` (default camera) shows a grilled supply
  beside the bottom walkway. Exact invocations/results are in the corresponding
  `*-render.log` files. These are initial views, not a human interaction,
  foreground-lemming image comparison, or a claim of original fidelity.
- All **15** promoted human-win recordings reproduced their recorded results,
  **PASS, won=1, completed=1, exit 0**. MM_W_012 is the affected pipe level
  among them; all were run to check the no-simulation-change requirement.
  Exact filenames and outputs: `build/pipe-render-001/replay-results.tsv`
  and one log per recording in the same directory. No recording was edited.

| Level | Saved / required | Result |
| --- | ---: | --- |
| M06_KF_1 | 93/50 | PASS |
| MM_BASH_002 | 10/10 | PASS |
| MM_BASH_MINE_WLR1 | 20/10 | PASS |
| MM_BLOCK_001_A | 24/15 | PASS |
| MM_BLOCK_BOMB_A | 12/10 | PASS |
| MM_BOMB | 17/10 | PASS |
| MM_BUILD | 20/10 | PASS |
| MM_CLIMB_002 | 10/1 | PASS |
| MM_DIG_000 | 19/10 | PASS |
| MM_FLOAT | 10/1 | PASS |
| MM_MINE | 20/10 | PASS |
| MM_W_012 | 16/10 | PASS |
| MM_BASH_DIG_001_A | 20/10 | PASS |
| MM_BUILD_BLOCK_A | 15/10 | PASS |
| MM_FLOAT_CLIMB2 | 10/6 | PASS |

The pipe geometry remains static during fill/drain: the inspected draw has no
flow predicate, the inspected receiver update does not change these models,
and installed pipes have no animation tracks. Exact original device depth
state remains unresolved; Less/write-on is the existing **provisional clone
policy**, not a fidelity finding. Spray/stream effects and the additive fluid
overlay remain omitted. No flowing-water animation is invented. Natural
foreground-lemming occlusion and playability await the user's playtest.

Changed files: `src/app/BackdropScene.cpp`, `BackdropScene.h`, `main.cpp`,
and this report. Task-only diff and pre-edit copies (including incoming edits)
are under `build/pipe-render-001/`. Incoming pressure-pad/crab, CMake,
queue/status and original-log edits were preserved. Supervisor PID 26408
owns RepositoryLease; no protected controls were edited. CIM process metadata
was denied; `Get-Process` confirmed no clone/build process before edits.
Source text uses LF and scoped `git diff --check` passes. No original game,
debugger, hook, capture, subagent or background work was launched. App image
runs used private `build/pipe-render-001/appdata`; replays never open a save.
Normal and installed saves were untouched. No test/assertion/timeout was changed
or new test registered. Mandatory **smoke verification is pending supervisor
execution after worker exit**; no full suite was run here.

Handoff: both pipe kinds now render through scene depth with stored placement/bias; all 15 human-win replays pass.
Next: supervisor smoke and independent review, then the user's playtest for acceptance.

What to check: Seafood Sarnie — reach the pressure-pad switch and drain the pool;
the grilled drain remains visible at the pool edge as water lowers, with lemmings
in front remaining visible. MM_W_012 — pull the lower lever to fill its tank;
the supply pipes remain at their stored positions while the water rises.
