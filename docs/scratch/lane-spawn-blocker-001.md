# lane-spawn-blocker-001

**Status** needs_review. [Playtest 19 J3 and J7](../playtest/2026-11-19-playtest-19.md)
followed the installed routes through Now you're stalking (`E22_KF_1`, node 27)
and Escape to victory (`E11_KF_1`, node 28). The ledger gives no level-specific
location for either lane defect. The shared source path explains both.

## Production change

Before J3 and J7, hatchlings started at 0.75 and blockers held their last
walking depth. The previous revision fixed placed-walker depth but left a
first-frame gap. The retained `MM_CLIMB_002` trace shows actor 0 as a faller
at 2040 ms and a walker at 2540 ms. Fast-forward can advance seven 100 ms
passes before assembling the scene, enough for a hatchling released early to
land and spend a later pass walking before the first sample.
[Retained clone trace](../../build/walker-lane-001/evidence/production/lane.csv)

`World::spawnFromEntrance` now records hatch origin for presentation. This
distinguishes a hatchling that lands in its release update from a walker added
with `World::place`. `runApplicationLeg` calls `LemmingMotionTracker::observe`
before and after World updates. For a new hatchling, `positionOf` returns
0.5 through all sampling for the first presented frame. `frameDrawn` runs
after `SDL_RenderPresent` and starts the next level-time interval from that
centre sample. A skipped frame leaves the first sample pending. The hatchling
holds its depth through the drop and approaches 0.25 left or 0.75 right once
walking. A placed walker starts in its facing's lane. A newly assigned blocker
approaches 0.5 from its current depth, holds depth through a fall after support
loss, and eases back to its side once walking. Both transitions use one slab
unit per level second and the previous confirmed state's target. A
midpoint-to-side trip takes 250 level ms.

`positionOf` supplies `plus8Z` through `injectedWalkersFromWorld` to the shared
pose, pick and occlusion path. The frame handling only changes the supplied
depth. The exit-jump calculation still uses the tracked contact depth and
targets 1.5. A hatchling that exits before its first live draw uses the tracked
depth at contact, or its birth depth if it exits before its first observation.
[Tracker](../../src/app/LemmingMotion.cpp#L44) · [Scene adapter](../../src/app/TubeScene.cpp#L1906) ·
[Window presentation](../../src/app/main.cpp#L1824)

## Static evidence

Reference `Lemmings Revolution/Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
The retained Ghidra 12.1.4 archive includes pseudocode, instructions, typed
references and constant bytes. This revision rechecked the constructor, walking
and blocker helpers and closed REA without a snapshot.
[Retained static evidence](../../build/lane-spawn-blocker-001/static-evidence.json) ·
[This revision's REA results](../../build/lane-spawn-blocker-001/first-render/rea-static.json)

- **Hatch** `0x425C30` calls `0x42DE51`. At `0x42DE6F–0x42DE89` the
  constructor passes binary64 0.5 as the third coordinate. `0x4230F9` copies
  six DWORDs to actor+8, placing depth at actor+0x18.
  [Retained constructor evidence](../../decomp-lemmings/docs/evidence/motion_trace.md)
- **Walk** `0x42F93C` sets the 0.25/0.75 targets from 0.5 ± 0.25.
  At `0x42F985` the update checks actor+0x5E and at `0x42F9A5` it checks
  facing −1. `0x42F977` approaches the selected target in clamped 0.02
  steps. The walk caller is `0x41BF53`.
- **Block** `0x40DFB0` installs the type-3 vtable. Its slot at `0x4B07BC`
  points to `0x40E003`, which passes the update budget to `0x42EDCD` at
  `0x40E053`. The helper moves actor+0x18 toward 0.5 by 0.1 times that
  argument and clamps at the midpoint, independently of facing. The support
  test at `0x40E06B` can lead to the Faller attachment.

The requested spawn and blocker targets agree with those static results. The
clone's first presented sample, shared level-time approach and fixed falling
depth are provisional. The original uses separate walking and blocker step
formulas, and its falling update at `0x41622E–0x416236` can reproject depth from
the mesh. The original's first rendered depth under fast-forward, exact
transition duration and the clone's desktop visual result remain unverified.

## Checks

- Registered MinGW-w64 built the clone and both legacy targets
  `test_exit_jump` and `test_walker_lane` in `build/agent-native/main`. The
  first attempt used the release preset, where `test_walker_lane` is unavailable.
  The lane fixture covers seven fast-forward passes between observation and
  first draw for both facings. It checks for 0.5 on that draw and a 0.02 step
  on the next 20 ms update. Source tracing the exit fixture gives
  `0.75 + (1.5 - 0.75) * 0.25 = 0.9375` at 250 ms, satisfying its strict
  `z > 0.75` assertion. Legacy execution remains scheduled for milestone sign-off.
  [Build log](../../build/lane-spawn-blocker-001/first-render/build-clone-and-legacy-targets.log) ·
  [Release attempt](../../build/lane-spawn-blocker-001/first-render/gcc-release-target-unavailable.log) ·
  [Exit fixture](../../src/__test__/test_exit_jump.cpp#L99)
- All 23 promoted human recordings returned `PASS won=1 completed=1` with
  the rebuilt clone. These include Now you're stalking (49/49, step 24402),
  Just Block (24/15, step 1619) and Block and Bomb (12/10, step 1546).
  The replay path returns before save setup and the run used private
  `LOCALAPPDATA`. Replays verify gameplay outcomes.
  [Every recording and result](../../build/lane-spawn-blocker-001/first-render/replays/results.tsv)

This revision changed `src/app/LemmingMotion.cpp`, `src/app/LemmingMotion.h`,
`src/app/main.cpp`, `src/__test__/test_walker_lane.cpp` and this report.
[Diff from revision entry](../../build/lane-spawn-blocker-001/first-render/review-source.diff)

- **Handoff** needs_review. Supervisor smoke is pending.
- **Next action** Review the first-frame depth and smoke result, then confirm J3 and J7 in play.

**What to check** In **Now you're stalking** (`E22_KF_1`, node 27), hold
Fast Forward across the first release. The first hatchling should appear in
the middle of the walkway and slide toward its facing's side as it walks.
In **Just Block** (`MM_BLOCK_001_A`, node 3), assign Blocker to walkers
facing each way. Both should slide to the middle and stay there. In
**Block and Bomb** (`MM_BLOCK_BOMB_A`, node 9), remove a blocker's support
with a nearby bomb. A surviving blocker should drop from its current depth
and slide back to its walking side after landing.
