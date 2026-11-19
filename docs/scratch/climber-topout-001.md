# Make Mine a Large One — climber at the upper grid boundary

Playtest 16 F1, installed level `E18_KF_2`, map node 26. The climber now clears the first earth ledge, hits the ceiling at the taller steel wall and lands alive on its approach side. [Playtest entry](../playtest/2026-11-18-playtest-16.md)

## What the human recordings show

Coordinates below are clone columns and feet rows, increasing downward. Steps are replay input steps. Each linked trace includes the simulation tick, facing, gravity, state, fall distance and material at the head, wall lip and support. Probe coordinates in the trace use the **post-update** pose.

- In [attempt 1](../../build/playtest/replays/E18_KF_2-1795044400292762200-1.replay), step 1539 assigns **Blocker** (skill 3) to lemming 0 at (90,48). It stays there through step 12737. The recording ends in retry with zero deaths and contains no Climber assignment. The final build reproduces the result at step 12738. [Tick trace](../../build/climber-topout-001/observed-1-changes.csv) · [Final replay](../../build/climber-topout-001/final-1.log)
- In [attempt 2](../../build/playtest/replays/E18_KF_2-1795044489816902700-2.replay), step 1879 also assigns **Blocker**, to lemming 1 at (83,48). The climber is **lemming 0**. It builds at steps 1471 and 3068 and receives Climber (skill 0) mid-build at step 4097, (112,29). The original recording reproduced to its map return at step 6774 on the uncorrected clone. [Tick trace](../../build/climber-topout-001/observed-2-changes.csv) · [Replay run](../../build/climber-topout-001/observed-2.log)

| Attempt 2 step | Observed path of lemming 0 |
| --- | --- |
| 4805 → 5040 → 5162 | Climbs right at (114,28), hoists at (114,16), walks onto (114,14). Both builds clear this ledge. |
| 6133–6369 | Climbs the second wall at x=146, feet 14 through 2, one row per simulation tick. The approach column 145 is empty above the ledge. Steel (`0x0003`) occupies columns 146–151 through row 0. |
| 6374, before | At feet row 1, the clone reads empty at both (145,−2) and (146,−1). It hoists. At 6392 it walks at row −1, then falls at x=152 on step 6499. A 51-cell fall ends in Splatter at (152,50) on step 6568 and Dead on 6569. |
| 6374, fixed | At the same feet row it backs up to (145,1), turns left and falls. It lands at (145,14) on step 6389. At 6568 it is walking left at (126,14). The replay check then reports the intended difference, 11 spawned, 0 saved, **0 dead** versus 1 recorded. This is an unpromoted attempt. |

[Authored cell words from the installed level](../../build/climber-topout-001/grid.txt) · [Corrected tick trace](../../build/climber-topout-001/corrected-2-changes.csv) · [Final replay divergence](../../build/climber-topout-001/final-2.log)

## Cause and production change

The registered x86 EXE has SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`. In `0x004137CA`, the Y write at `0x0041388B` precedes the call at `0x004138E8` to `0x004139BF`. The latter clamps the head lookup at `0x00413A2A`, reads the cell at `0x00413A53`, then compares the **unclamped** head row to the grid height at `0x00413A67`. Even over air, a row strictly above height enters the step-back, reverse and fall branch. Only a clear head permits the wall probe at `0x00413B50` and eventual hoist lift at `0x00417D6F`. Ghidra pseudocode, xrefs and instructions agree on the branch order. The original was inspected statically. [Ghidra output](../../build/climber-topout-001/ghidra.json) · [Climb instructions](../../decomp-lemmings/docs/evidence/actor_climbing.asm) · [Hoist instructions](../../decomp-lemmings/docs/evidence/actor_topping.asm)

`World::updateSegment → stepElapsed → step → stepClimber` used only the terrain lookup, which returns empty above the grid. For height 96, clone row −1 maps to native 96 and samples the top cell. Clone row −2 maps to native 97 and forces a head hit before top-out. The fix restores that distinction in the climber and reuses the existing turn and fall transition. The upper limit explains the failure at column 146 without a change to wall geometry, climb speed or material rules. [Production change](../../src/sim/World.cpp)

## Checks and handoff

The final UCRT64 Release build passed the focused legacy test for equality, an occupied edge cell and the first row above the limit. All **23 promoted human wins** returned exit 0, `PASS`, `won=1` and `completed=1`. The table gives each recording's saved count and individual log path. Replay runs used task-private LocalAppData. Changed files are `src/sim/World.cpp`, `src/__test__/test_climber_ceiling.cpp`, `CMakeLists.txt` and this report. [Build](../../build/climber-topout-001/build-final.log) · [Focused test](../../build/climber-topout-001/unit.log) · [All recording paths and results](../../build/climber-topout-001/promoted.tsv)

- Handoff — **needs_review**. Supervisor smoke and recovery are pending.
- Next action — independent review after the gate, then human playtest. A full win on this level remains unverified.

**What to check** — On Make Mine a Large One, build toward the first ledge and give the builder Climber. Watch it clear the earth ledge, climb the tall steel wall, turn left at the ceiling and land back on the ledge.
