# pressure-pad-drain-001 — Seafood Sarnie drain switch

Status: **needs_review**. Playable v1, playtest 8 X1; supervisor smoke verification
pending. Before: [human ledger](../playtest/2026-11-16-playtest-8.md), node 15,
`MM_BLOCK_BUILD_BASH` (Seafood Sarnie), abandoned with 0/20 saved because the
switch/draining mechanic was absent. Connecting that demonstrated blocker is the
next dependency. No new human win or original-fidelity pass is claimed.

## Static contract and installed data

Reference: `Lemmings Revolution/Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Ghidra 12.1.4 static analysis only; session closed without snapshot arguments.
Full returned evidence, including limitations, is retained in
`build/pressure-pad-drain-001/static/`. No original execution, debugger, hooks,
capture, copied machine-code execution or installation change occurred.

- **STATIC:** tagged dispatch `0x433DD9`, case 0, constructs `0x431837`, vtable
  `0x4B1148`; reader `0x431B30` reads stored latch `+0xB8` and one message list
  `+0xBC`. These records' model is `pressure_pad.a3d`, establishing the switch's
  identity from structure and data, beyond its visual resemblance.
- **STATIC, instruction checked:** `0x431A59` scans actor list `0x4C88D0`, gets
  each actor point through `0x42F22E`, and calls closed three-axis AABB predicate
  `0x430BA1`. It has no facing, species, gravity or walking-state test. The
  previous-actor exclusion is `+0xA4`. `0x431950`, specifically
  `0x431984..0x4319A7`, sends the list only when latch != 1, setting it to 1
  first. Further contacts/cooldown expiry never reset that latch. Shared step
  `0x4339FC` and the pad's empty vt+0x24 explain why the serialized cooldown is
  immaterial to this one-shot fan-out.
- **STATIC, instruction checked:** reader `0x431B93..0x431BF9` collects model
  names containing `cut` (`0x4BF82C` bytes); activation writes visibility 0 at
  `0x4319E5`. Clone presentation suppresses those meshes/subtrees, leaving the
  base visible. It does not invent a colored substitute switch.
- **ACCEPTED STATIC:** [lever links](lever-links-research-001.md) establishes
  `0x433AC0 -> 0x433B2E -> 0x444BA1`, signed base-id lookup in construction
  order, and type 5 -> `0x42C8E0` setting receiver request 1. Existing
  `0x42B24D` lowering machinery drains kind-0 fluid at 2 height units per
  shared elapsed second, clearing the current mask before painting the shorter
  volume. No new fluid clock, rate or hazard semantics were introduced.
- **SERIALIZED:** `build/pressure-pad-drain-001/census.txt` is a read-only
  structured census of installed archive records, distinct from runtime evidence.
  Node 15 has pad id 0, latch 0, volume **[45,47] x [20,26] x [0,1]** in
  original cylinder coordinates, cooldown 100, and five `(id,type,p1,p2)`
  messages: `(8,5,0,0)`, `(7,5,0,0)`, `(6,5,0,0)`, `(5,5,0,0)`, `(4,5,0,0)`.
  All five resolve to kind-0 water gates, initially request 0 / height 10.5,
  low 0. Nominal lowering time is 5.25 shared elapsed seconds, with the existing
  next-update clamp. The pad stays fired and there is no other switch in this
  level to refill the water; reload restores the stored initial state.

## Production change and checks

`loadRetailLevelReport -> linkLevelGates` now retains type-0 records as
`LevelSetup::pressurePads`, using their stored boxes, latch and messages.
`RuntimeSimState::advanceElapsed -> World::advanceElapsedMs -> updateSegment ->
stepGadgetContacts -> dispatchLeverMessages -> FluidGate::step` connects contact
to the existing receiver height/mask path in the same update. Tagged pad/lever
stream order is retained; the previous unpaired-lever fallback is preserved.
`groundAt` and `attachFluidHazard` already read live receiver height, so a zero
height removes both fluid support and drowning contact without a separate hazard
patch. Accepted lever initialization, facing, cooldown and receiver logic remain.

`BackdropScene::build` admits each type-0 placed model through the existing
depth-tested scenery path. `main.cpp` binds pad ids and supplies live latch
states in scene preparation; `setPressurePadStates` hides `cut` geometry on the
first fired state. Pristine model storage permits deterministic state restoration.
Seafood Sarnie's ready model has 16 mesh blocks; firing leaves its four base
blocks. The single initial production screenshot
`build/pressure-pad-drain-001/Seafood-Sarnie-ready.png` was inspected: the red
beam frame is visibly present on the lower walkway. This is a clone initial
view, not a fired/drained pixel comparison or a natural gameplay route.

Changed: `src/sim/{RetailLevel.cpp,World.cpp,World.h}`,
`src/app/{BackdropScene.cpp,BackdropScene.h,main.cpp}`,
`src/__test__/test_pressurepad.cpp`, `CMakeLists.txt`, and this report.
Pre-edit copies are under `build/pressure-pad-drain-001/pre-edit/`.
The supervisor already owned RepositoryLease (PID 26408); no protected control
or instruction edits were made. Pre-existing queue/status/original-log edits
were preserved. CIM process metadata was denied; `Get-Process` succeeded and
showed the supervising Python processes, with no live clone/build process.

Registered MinGW UCRT64 build passed:
`build/pressure-pad-drain-001/review-build.log`. New **legacy** `pressurepad`
passed in **0.82 s**, exact run
`build/pressure-pad-drain-001/review-unit-ctest.log`. It checks one-shot and
closed-corner contact (including inverted/falling and non-walker actors), removed
actors/outside points, multiple contacts, same-update and ordered message
dispatch, drained hazard removal versus wet contact, load reset, installed
bindings and visible geometry changes. Synthetic logic fixtures are not level
playability evidence. No per-task scripted route, replay fixture or frozen build
was added. An initial test execution used the just-built binary with a mistaken
new census expectation of 40; the retained census showed 45, and that new
expectation was corrected before the successful runs. Existing tests were not
edited or removed. `git diff --check` passed; changed tracked text remains LF.

The required **smoke profile remains pending supervisor execution after worker
exit**. The worker did not run that gate or the full suite. Ready-view rendering
used private `build/pressure-pad-drain-001/appdata`; replays and logic checks do
not open saves. No installed or normal user save was read, imported or written.

## All 23 installed levels

Production-loader census: `review-unit-ctest.log` above. **45 pads; zero missing
target IDs.** “Other” means the ID resolves but its receiver class/message effect
is not implemented by the accepted gate dispatcher. Such messages are counted
as unsupported and never redirected to a fluid gate. This task connects all pads
and their fluid targets; it does not make those other gadget types functional.
Counts below are message records across a level's pads, not unique receivers.

| Map node | Level | Pads | Fluid messages | Other messages / receiver collections |
| ---: | --- | ---: | ---: | --- |
| 15 | MM_BLOCK_BUILD_BASH | 1 | 5 | 0 — all effects supported |
| 33 | M11_KF_1 | 3 | 3 | 3 / 424187 |
| 43 | M16_mg_1 | 1 | 0 | 1 / 424187 |
| 44 | M17_MT_1 | 1 | 20 | 0 — all effects supported |
| 48 | M18_MT_1 | 1 | 0 | 1 / 424187 |
| 51 | M21_KF_1 | 3 | 0 | 3 / tagged, 424187 |
| 53 | H01_KF_1 | 1 | 5 | 0 — all effects supported |
| 56 | H17_mg_1 | 1 | 0 | 2 / 424187 |
| 57 | e14_kf_1 | 1 | 0 | 1 / 424187 |
| 59 | H06_MT_1 | 2 | 0 | 2 / 424187 |
| 61 | H08_KF_1 | 2 | 0 | 3 / 424187 |
| 64 | M15_mg_1 | 5 | 0 | 5 / 424187 |
| 65 | H12_mg_1 | 4 | 0 | 8 / 424187 |
| 69 | H15_MT_1 | 2 | 0 | 12 / 424187 |
| 73 | H20_KF_1 | 2 | 0 | 3 / 424187 |
| 77 | H24_KF_1 | 2 | 0 | 2 / 424187 |
| 80 | H08_MT_1 | 4 | 0 | 5 / 424187 |
| 82 | VH06_MT | 1 | 0 | 3 / tagged |
| 85 | VH09_MT | 1 | 8 | 1 / 424187 |
| 88 | VH12_KF1 | 3 | 0 | 4 / 424187 |
| 92 | VH15_KF_1 | 1 | 0 | 1 / 424187 |
| 93 | VH16_MT | 2 | 0 | 6 / 424187, 4342bf, named |
| 98 | VH22_KF1 | 1 | 2 | 2 / 4342bf |

## Recorded human-win regressions

Ran **all 15** promoted files, unchanged, using
`webolution --game-dir "Lemmings Revolution" --replay <file>`.
Every run exited 0 with PASS, won=1, completed=1; no divergence.
Exact filenames and results: `build/pressure-pad-drain-001/replays.txt`, with
one `<recording-basename>.log` per file alongside it.

| Level | Saved / required | Result |
| --- | ---: | --- |
| MM_CLIMB_002 | 10/1 | PASS |
| MM_BOMB | 17/10 | PASS |
| MM_FLOAT | 10/1 | PASS |
| MM_BLOCK_001_A | 24/15 | PASS |
| MM_BUILD | 20/10 | PASS |
| MM_MINE | 20/10 | PASS |
| MM_BASH_002 | 10/10 | PASS |
| MM_DIG_000 | 19/10 | PASS |
| MM_W_012 | 16/10 | PASS |
| MM_BLOCK_BOMB_A | 12/10 | PASS |
| MM_BASH_MINE_WLR1 | 20/10 | PASS |
| M06_KF_1 | 93/50 | PASS |
| MM_BASH_DIG_001_A | 20/10 | PASS |
| MM_FLOAT_CLIMB2 | 10/6 | PASS |
| MM_BUILD_BLOCK_A | 15/10 | PASS |

## Limits and handoff

**Provisional:** contact uses clone integer feet and radial coordinate 0 because
the simulation has no continuous three-dimensional actor point; every installed
pad's Z range includes 0. The existing hybrid/shared elapsed timing, fluid depth
policy and mean-height drowning/support remain provisional. The native pad's
transient model animation is not replayed; the traced fired endpoint is applied
immediately. Legacy `World::tick()` still does not step tagged receivers/triggers;
ordinary application/replay play uses `advanceElapsedMs`. Non-fluid message
effects listed above remain unsupported. Crab and pipes are separate queued
work. No Seafood Sarnie human win, full-fidelity result or historical gate change
is claimed.

Handoff: pressure-pad activation, fluid dispatch and fired-model visibility are implemented; all 15 human-win replays pass.
Next: supervisor smoke and independent review, then the user's Seafood Sarnie playtest; acceptance remains pending.

**What to check:** Seafood Sarnie (map node 15), lead a lemming through the red
beam switch on the lower walkway (column 46, original Y 20..26). The beams
disappear while the base stays visible; the linked pool lowers over about 5.25
shared simulation seconds. Once empty, ordinary lemmings can cross its former
water volume without drowning. Further crossings do not refill it; retry restores
the full pool and visible beams. A human win is the remaining playability check.
