# hatch-rate-hold-001

**needs_review**. The supervisor smoke gate and the next human playtest are pending.

## Playtest and production path

Playtest 25 Q1 on **Designed with love** (`M11_KF_1`, node 33) reported that clicks and holds
needed to be repeated as lemmings fell past the hatch. The source trace reaches
`main.cpp::hatchAtPointer`, where the actor pick suppressed the hatch index.
`RuntimeReleaseRateControl::advance` treated that missing index as a pointer exit.
[Playtest ledger](../playtest/2026-11-21-playtest-25.md)

- In `runApplicationLeg`, mouse motion and the per-frame update now send displayed
  `BackdropScene::hatchHoverTargets` to `RuntimeReleaseRateControl::advance`. It uses
  `heldTarget` and `hoveredHatch` to look up the specific held hatch, even where another
  hatch's rectangle overlaps it. Button release, departure from the hatch or window,
  focus loss, the menu, and camera grab end the hold.
- **Provisional clone precedence.** Right press on a hatch goes to the rate control before
  camera grab. Left press goes to the normal actor picker and skill assignment if
  `World::canAssign` accepts the selected skill, and otherwise acts on a hatch under
  the pointer. Idle hover shows the left-click target. During a hold, the entrance
  cursor and RATE readout follow the held hatch. Successful rate changes still go
  through `RuntimeReleaseRateControl::change` and `World::setReleaseInterval`, and
  recording still emits the resolved release-rate command.
  [Input path](../../src/app/main.cpp) · [Release control](../../src/app/RuntimeSkillPanel.cpp)

## Static evidence

The registered `Lemmings Revolution/Lemmings Revolution.exe` matched SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Ghidra 12.1.4 instructions at `0x453A42..0x453AAF` show a selected-hatch increment
and decrement through `0x425DE3`. The wrapper pseudocode connects those queries
to `0x455DF0` and `0x455DB0`, whose instructions compare mouse state at offsets
`+4` and `+0` with 1. Retained reads at `0x483C3E..0x483C70` identify those
fields as sampled held state. The static evidence establishes those queries.
Overlap precedence and repeat behavior remain provisional clone choices.
[REA instructions, pseudocode and xrefs](../../build/hatch-rate-hold-001/static-evidence.json) ·
[Retained rate research](release-rate-control-001.md)

## Checks and limits

- Registered UCRT64 GCC/CMake built `smoke_tests` and `test_hatch_rate` in
  `build/agent-native/main`. The windowless legacy unit passed using synthetic
  displayed rectangles and an assignable lemming. It covered right press during
  overlap, left-press precedence, continuing an existing hold during overlap,
  release, and exit into another hatch's geometry.
  [Build log](../../build/hatch-rate-hold-001/build.log) ·
  [Unit log](../../build/hatch-rate-hold-001/hatch-rate-unit.log)
- All nine focused smoke-labelled checks passed, covering the adjacent skill,
  cursor, HUD, and input paths. Full smoke and recovery verification are pending
  with the supervisor. The checked clone processes used a private local app data
  path. [CTest log](../../build/hatch-rate-hold-001/focused-smoke.log)
- All 29 promoted recordings won through `webolution --game-dir "Lemmings Revolution" --replay`.
  Designed with love reproduced at step 40836 with 97/85 saved. These recordings
  exercise resolved commands. The live cursor and readout during a passing lemming
  still need the next human playtest. [Per-recording results](../../build/hatch-rate-hold-001/promoted-replays.tsv) ·
  [M11_KF_1 log](../../build/hatch-rate-hold-001/replays/M11_KF_1-1795249788316136900-14.log)

## Two-line handoff

- Review the hatch press, held-geometry lookup, and cursor/readout path.
- Run the supervisor smoke gate, then carry the following check into the next playtest.

### What to check

On **Designed with love** (`M11_KF_1`, node 33), hold right click on a hatch before
a lemming drops past the pointer. Expect the entrance cursor and RATE to stay
visible and the rate to keep increasing within its limit. Release or drag off the
hatch to stop repeating. Right-click again during an overlap and expect the rate
to increase. Left-click during an overlap with an assignable skill and expect the
skill to be assigned. With a skill the lemming cannot receive, expect the hatch to
lower the rate.
