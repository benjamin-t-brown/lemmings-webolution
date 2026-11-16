# pad-pipe-presentation-001

Status: **needs_review**. Playable v1, [playtest 9 Y1/Y2](../playtest/2026-11-16-playtest-9.md),
Seafood Sarnie (node 15, `MM_BLOCK_BUILD_BASH`). The human won but saw a static
beam switch and lemmings drawn over its tank pipe. These are the next presentation
dependencies after hatch-click-rate-001. No simulation code changed. Difficulty:
routine; recommendation Sonnet; assigned Codex override used.

## Evidence and implementation

Retained starting points: [pressure-pad-drain-001](pressure-pad-drain-001.md),
[pipe-render-001](pipe-render-001.md), and their accepted reviews in `agent/tasks.json`.
Their smoke logs are `build/agent-runs/20261116T071902-26408-272b94c1/verification.log`
and `build/agent-runs/20261116T080016-26408-73ac4f0b/verification.log`; neither
verifies this change.

Static reference: `Lemmings Revolution/Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Used the reverse-engineer-anything skill and static REA Ghidra **12.1.4**,
profile digest `3c06738e45e144d5f073edbd3244c6bff46222f389525fe659bc92dca2ccdece`.
New returned packets are under `build/pad-pipe-presentation-001/static/`.
These are static instructions/pseudocode, not observed original execution:

- `model-vtable.json`: model vtable `4B1688` maps +34 to `451479` (select
  clip), +38 to `45142E` (seek), +3C to `451649` (loop flag), +44 to `45161B`
  (finished). `pad-static-0.json` and `seek-instructions.json` agree that
  `45142E` writes the supplied time to +120/+124, clears finished and samples
  `4764C4`. Seek instruction evidence:
  `ev_e201c3a302405d1529a6cc58da2f228d64dd88b183feaadfaac2b7534d668479`.
- `pad-static-2.json`, `431936/431949`: initialization selects clip zero
  and disables automatic looping. `pad-pseudo-2.json`, `4319F2`: on finished,
  the update seeks zero and reselects zero. This supports repeating idle animation.
- `pad-static-1.json`, `431969/43197E`: activation selects clip zero and
  seeks zero; `4319E5` immediately hides collected `cut` nodes. Pseudocode and
  instructions agree. Activation instruction evidence:
  `ev_912069bbb6a99db8e48050425471ffb1919d0cfd633defdf939ac150497cfb9a`.
- Installed data (`census.txt`, `trip_switch_two_way.bin`, `read_clip.py`):
  clip `trip_switch_two_way`, duration **1.6666666269302368 seconds**, eight
  transform tracks, 50 keys each, targeting nodes 10/13/9/12/6/11/8/7; two
  50-key UV tracks target nodes 3/5 and 4/1, material zero; no event tracks.
  Existing transform instructions are retained with
  [hatch presentation](hatch-exit-presentation-001.md); UV grammar/interpolation
  comes from [gravity presentation](gravity-gadgets-001.md), `47C93D/47CF5C`,
  `476FB0/476A80`. The parsed node/material targets match the placed pad tree.

Production path: `runApplicationLeg -> BackdropScene::build ->
pressurePadPresentation` binds the stored clip and original corner UVs once.
`advanceScenery -> prepareScene -> setPressurePadStates` samples transforms
through `applyHatchAnimation` and material offsets through the retained UV rule.
The existing scenery clock freezes on pause. Each sample starts from the pristine
tree, preventing accumulated UV drift or changed node indices after hiding beams.

**Provisional Y1 feedback:** idle repeats the authored clip. First observed
activation restarts it, keeps its beams visible for one authored cycle, then hides
the `cut` subtrees, retaining the four-mesh base. Native static code hides them
immediately; delaying that hide is an explicit playable-v1 presentation choice.
The simulation latch, fan-out and water lowering still start immediately. A pad
loaded already fired stays settled. Initial phase and frame-end latch observation
are clone adapters, not recovered original timing.

Y2's source-traced failure is the actor's provisional **four-world-unit terrain
contact allowance** (`ActorSprite.h -> actorPhysicalDepth -> clipActorToScene`).
Pipes already wrote depth; that allowance could pull actors through nearby pipes.
`renderFrame -> Stage::FluidPipe -> SceneRasterObserver` now retains only accepted
pipe depth fragments before water can overwrite them. `SceneOptions::pipeSceneDepth
-> prepareInjectedWalker -> clipActorToScene` additionally tests those fragments
without the terrain allowance. Drawing and hover/picking use that same preparation.
Foreground sprites still pass depth; terrain contact and drowning behavior remain.
Stored pipe placement, native software projection biases, draw order and fluid
policy are preserved. Exact original device depth state remains unresolved; this
separate pipe comparison is a **provisional clone policy**, not a fidelity pass.

## Checks and limits

- Registered UCRT64 GCC/CMake/Ninja production build: **PASS**,
  `build/pad-pipe-presentation-001/build.log`, eight build jobs, existing -O2/-g
  configuration. Final build has no compiler warnings.
- Bounded data/render inspection: **PASS, failures=0**,
  `build/pad-pipe-presentation-001/presentation.txt` (driver/build log alongside).
  All **45 pads / 23 installed levels** bind eight transform and two UV tracks.
  Seafood Sarnie samples change matrices/UVs and raster pixels, hold on pause and
  repeated draw, restart on activation, animate before hiding and restore beams
  on reset. Inspected `pad-ready.png`, `pad-idle.png` and the pipe-only images.
- Both pipe stages still rasterize: Seafood Sarnie **2,899** pipe-depth pixels
  at yaw 116, MM_W_012 **5,052** at default yaw. Synthetic constant-depth planes
  through production sprite drawing/picking show behind-plane actors completely
  hidden, foreground actors visible, and the terrain allowance preserved. These
  checks are supplemental, not a natural actor/pipe overlap or a human playtest.
- All **17** promoted recordings ran through the built
  `webolution.exe --game-dir "Lemmings Revolution" --replay <file>` and returned
  **exit 0, PASS, won=1, completed=1**, unchanged. Exact filenames and individual
  logs: `build/pad-pipe-presentation-001/replay-results.tsv` and adjacent `.log`s.

| Level | Saved/required | Result |
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
| MM_BLOCK_BUILD_BASH | 21/20 | PASS |
| MM_E4_4_SKILLS | 28/20 | PASS |

Removed legacy `pressurepad` registration/target/source: its immediate beam-hide
expectation encodes the Y1 presentation being replaced. It was removed under
Verification tiers, not repaired; no smoke assertion, registration or timeout
changed. No new test, replay fixture, frozen build or scripted gameplay route
was added. Mandatory **smoke verification is pending supervisor execution** after
worker exit; no verification profile was duplicated here.

Changed: `src/app/{BackdropScene.cpp,BackdropScene.h,TubeScene.cpp,TubeScene.h,main.cpp}`,
`CMakeLists.txt`, removed `src/__test__/test_pressurepad.cpp`, and this report.
Pre-edit copies and incoming patch are under `build/pad-pipe-presentation-001/`.
The parent supervisor holds RepositoryLease (PID **11176**); no protected controls
were edited. CIM process metadata access was denied; Get-Process showed no live
clone/compiler/test runner before edits. Incoming original-log, queue and generated
status edits remain. Scoped diff checks pass; edited text is LF. All clone commands
used private `build/pad-pipe-presentation-001/appdata`; replay opens no SaveStore.
Installed/normal saves were not used. Static analysis was closed without snapshot
arguments. No original execution, oracle, debugger, hook, capture, install,
subagent, concurrent writer, commit or push occurred.

Handoff: authored pad transform/UV animation and strict pipe-to-actor depth are implemented; all 17 promoted replay results pass.
Next: supervisor smoke and independent review; the user's next playtest confirms visible Y1/Y2 acceptance.

What to check: **Seafood Sarnie, node 15** — watch the red switch before crossing:
its beams move and scroll; crossing restarts one brief animation, then its beams
vanish while the water drains immediately. Rotate around the tank: lemmings behind
the pipe disappear behind its opaque parts; those in front remain visible. Pause
freezes the switch. **MM_W_012** — activate the lower lever; its supply pipes
remain drawn while the tank fills, with foreground lemmings still visible.
