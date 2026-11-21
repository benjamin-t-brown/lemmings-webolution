# water-motion-001

**Needs review.** Playtest 25 Q5 reported a static pool and pipe stream in **Just Climb** (`MM_CLIMB_002`, map node 0). The clone loaded the secondary scene tree and rendered its base UVs. It advanced sprite controllers and gravity effects but left the secondary mesh UV tracks unused. [Playtest ledger](../playtest/2026-11-21-playtest-25.md) · [Installed data census](../../build/water-motion-001/census.txt)

| Secondary mesh | Role | Preorder index / LVL offset | UV |
| --- | --- | --- | --- |
| `trans_moatwater1` | Pool at the tower base | 22 / `0x972D6` | V |
| `trans_hosewater` | Stream at the pipe mouth | 24 / `0x98334` | V |
| `trans_waterfalling` | Waterfall mesh | 23 / `0x98106` | U |
| `trans_river` | River mesh | 9 / `0x95CB3` | U |

All four meshes use material slot zero, `Models\bg_sand_medium\~Water1`, and kind-7 records. The installed secondary `myanim` clip has four 100-key UV tracks and a 1.333333373-second duration. The level has zero fluid receivers.

**Static evidence.** For reference EXE SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`, the secondary holder loads at `0x437DFD → 0x4749BF`. The clip reader `0x476277` calls the UV array reader at `0x4762C9`, and `0x47C93D/0x47CF5C` read the targets and keys. `0x47645E → 0x46BAFE` builds the preorder target table. The frame update at `0x442445 → 0x437BDE` wraps clip zero at its duration and calls `0x4764C4` at `0x437D3C`. The material stores at `0x476982..0x476993` and vertex additions at `0x46D596..0x46D5BC` establish UV translation for the pool and stream. Ghidra pseudocode and the instruction reads agree on the interval calculation at `0x476FB0/0x476A80`. The paused dispatch at `0x45DD76` supplies zero elapsed time. [Static trace and REA packets](../../build/water-motion-001/static/trace.md)

**Production path.** `BackdropScene::build → bindWaterUv` binds the installed secondary track targets. The existing app call to `advanceScenery → advanceWaterUv` samples the authored keys with the same interval evaluator as `animateGravity`. `renderStage → buildSceneMeshSource → adaptPackedSceneMeshVertex → packSceneVertex` carries the material offset into the vertex UV. The clock uses the existing 100 ms cap, freezes on pause and advances once per displayed frame during fast-forward, following the clone's scenery cadence. Static instructions show repeated world updates during original fast-forward. The playtest milestone leaves original framebuffer comparison parked.

The material filter admits **1,744 water, splash and reflection nodes across 81 levels**. Ten sand levels bind four nodes each and 71 mine levels bind 24 each. The production probe matched the installed census on all 102 levels. [Level and node counts](../../build/water-motion-001/level-census.tsv) · [Production probe](../../build/water-motion-001/frame-probe.log)

**Checks.**

- Fixed yaw 32 and identical scene geometry at 100 and 400 ms. The pool changed 12,946 pixels and the pipe-mouth stream changed 1,206. Depth and pixel ownership matched. A paused sample matched the 100 ms frame, and the probe checked the cap, clip wrap and fresh phase. The app also produced two CPU framebuffers through the normal level renderer. [Scene 100 ms](../../build/water-motion-001/just-climb-scene-100ms.png) · [Scene 400 ms](../../build/water-motion-001/just-climb-scene-400ms.png) · [App 104 ms](../../build/water-motion-001/just-climb-window-104ms.png) · [App 403 ms](../../build/water-motion-001/just-climb-window-403ms.png)
- All **42** settled and final sweep PNGs for the **21** unaffected levels were byte-identical to the retained sweep frames. [Sweep comparisons](../../build/water-motion-001/unaffected-sweep/comparison.tsv)
- All **24** promoted recordings on the affected levels reproduced their wins. Just Climb finished at step 1252, saving 10 with 1 required. The per-recording log gives the exact path, step and result for each route. [Replay results](../../build/water-motion-001/replays/results.tsv)
- UCRT64 built `smoke_tests`. The seven smoke-labelled scene tests passed. Clone runs used the task-local `LOCALAPPDATA`. [Final build log](../../build/water-motion-001/final-smoke-target-build.log) · [Final scene tests](../../build/water-motion-001/final-scene-smoke-checks.log)

**Handoff.** The supervisor's full smoke profile is pending, followed by independent review.  
Human acceptance is the next playtest of Q5.

**What to check** Open Just Climb, let the entry camera settle and watch the water at the tower base and the stream at the left pipe mouth. Expect both textures to move, freeze on pause and resume from the same phase.
