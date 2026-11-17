# acid-lever-drain-001

Status: **needs_review**. Playable v1, [playtest 12 B2](../playtest/2026-11-17-playtest-12.md),
One Way Ticket (map node 22, `MM_ALL_8_001`). The human operated the switch but
the acid stayed full. Making that existing drain connection work is the next
playability dependency. Difficulty difficult; recommendation Opus; assigned
Codex override used. Mandatory smoke verification is **pending supervisor execution**.

## Cause and retained evidence

The link was already correct. The missing **kind-2 height rate** made acid
immobile after the receiver accepted the drain request. `FluidGate::rateFor`
handled water and lava, then returned zero for acid. The production fix adds
acid's traced **1.0 height unit per shared elapsed second**; it changes no
links, contact rules, clocks, zone painting or drain direction.

**Installed data through the production loader:** lever id 0 at clone (75,36)
starts facing -1. Its minus list is empty; its plus list contains
`(target=1,type=5,p1=0,p2=0)`. Receiver id 1 is kind 2, initially request 0,
height/high 25.75, low 0, original-coordinate bounds
`[128,136) x [1,26.75]`. Rightward contact sends type 5, requesting drain.

**Retained static evidence, instruction checked:** registered reference
`Lemmings Revolution/Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
No new original execution or REA session was needed.

- [Accepted lever research](lever-links-research-001.md): fan-out
  `0x433AC0 -> 0x433B2E -> 0x444BA1`; type 5 reaches `0x42C8E0`, writing
  request 1. The level's target resolves to the fluid receiver, not its pipe.
- [Accepted fluid research/integration](fluid-kinds-001.md), retained packets
  `build/fluid-kinds-001/static/0x428848-{pseudo,instructions}.json`:
  `0x4289B1` writes low dword 0 to `0x4BF788`, and `0x4289BB` writes high
  dword `0x3FF00000` to `0x4BF78C`: IEEE double **1.0**. This is rate-table
  base `0x4BF6F8 + 2*0x48`.
- `build/fluid-kinds-001/static/0x42B24D-{pseudo,instructions}.json`:
  `0x42B42F..0x42B44E` indexes that table by kind, multiplies by supplied dt,
  subtracts from live height and stores it. `0x42B3E3..0x42B423` only clears
  water/lava bits, skipping zone writes for kind 2. Acid's existing no-paint
  behavior is therefore preserved; writing the clone's acid plane here would
  invent a different contract. Rendering and fluid contact already read height.

These are static instructions and serialized data, not an original comparison.
Earlier reports intentionally deferred acid ramping; this authorized defect fix
implements that retained rate without changing their historical results.

## Production path and clone checks

`loadRetailLevelReport -> linkLevelGates -> World` retains receiver kind and
height plus the lever's stored face and ID-resolved message. During play,
`RuntimeSimState::advanceElapsed -> World::advanceElapsedMs -> updateSegment ->
stepGadgetContacts -> activateLeverFace -> dispatchLeverMessages` sets request 1.
The same segment then calls `FluidGate::step -> stepHeight -> rateFor(2)`.
Compare-before-subtract, overshoot and next-update settling stay unchanged.

`main.cpp`'s scene preparation copies `World::setup().gates[].height` into
`BackdropScene::FluidGateState`; `renderFluid -> applyFluidState` moves the
existing acid geometry. `World::groundAt` and `attachFluidHazard` consume the
same live height and skip a zero-height receiver. Thus an empty tank ceases to
support or drown actors; no separate visual-only or hazard-only patch was added.

The user's unchanged recording
`build/playtest/replays/MM_ALL_8_001-1794930260246912900-8.replay` was stepped
headlessly. `build/acid-lever-drain-001/observed-attempt-before.log` records
the switch at **step 3374, clock 43206 ms**, request 1, delivered 1, height
still 25.75 through the end. After, `observed-attempt-after.log` records the
same operation with height **25.735**, then 24.939 at 44002 ms, 0.933 at
68008 ms, and **0 / done 1** at 69004 ms. The first drain update receives
15 ms. Both ordinary `webolution --replay` runs finish at step 7564,
saved 0/40, won=0, completed=0: reproducing this abandoned attempt is not a win
or proof the human crossed the drained tank.

The ignored `build/acid-lever-drain-001/inspect.cpp` uses a local playback-header
copy with state logging added; every canonical replay check remains. It also
loads installed controls, places direct-contact fixture actors, and steps their
changed receivers to their serialized endpoints. These are contact/receiver
fixtures, not new gameplay routes. Full results: `census-after.txt`.

## Installed acid census

All 102 installed levels were loaded; nine have acid. Six also have a lever
or pad. Nodes below use `MapScene::nodeMapping`, rather than LEVELMAP row indices.
All three linked levels had the same zero-rate fault and now lower correctly.

| Node / level | Acid control and result |
| --- | --- |
| 22 / MM_ALL_8_001, One Way Ticket | Lever 0 plus list -> receiver 1, type 5; contact delivered 1; 25.75 -> **0**, nominal 25.75 seconds. |
| 32 / M08_mg_1, Wheelbarrows of Doom | Lever 7 plus list -> receivers 1/3/2, type 5; contact delivered 3; all 4.75 -> **0**, nominal 4.75 seconds. Lever 4 targets `entities_424187`. |
| 35 / H02_JC_1, How do we get up there! | Two levers target `entities_424187` ids 1/10, not the five acid receivers. **No acid drain link**; remains full. |
| 50 / M20_MT_1, Bridges | Lever 7 minus list -> receivers 13/12/11/10, type 5; contact delivered 4; all 15.75 -> **1**, nominal 14.75 seconds. Low 1 is serialized and preserved. Lever 8 targets `entities_424187`. |
| 77 / H24_KF_1, Bash Street Kids | Two pads target `entities_424187` ids 5/4. **No acid drain link**; remains full. |
| 88 / VH12_KF1, Green with Envy | Three pads target `entities_424187` ids 1/10/6/0. **No acid drain link**; remains full. |

The other acid levels have no lever/pad: node 23 `MM_E1_5_Skills` (Swings
Both Ways), node 38 `MM_TELE_003` (The Lone Ranger), node 76 `H23_mg_1`
(When Two Tribes go to War). No extra drain links were invented. General
`entities_424187` effects and their indirect gameplay remain outside this fix.

Water checks in the same census: all four **MM_W_012 levers raise** their
16 linked receivers from 0 to 24.5 (correction to the task's informal "drain"
description); Seafood Sarnie's pad delivers five drain messages and lowers all
five receivers from **10.5 to 0**. Those contact fixtures pass. Seafood's
retired builder-dependent recording was left untouched.

## Verification and limits

Changed: `src/sim/World.{cpp,h}`, `CMakeLists.txt`, new
`src/__test__/test_fluidgate.cpp`, and this report.
Final registered GCC/UCRT64 build: `build/acid-lever-drain-001/final-build.log`.
New **legacy** unit `fluidgate`: `build/acid-lever-drain-001/final-unit.log`,
**PASS, 0.03 seconds**. It checks all three rates, lowering, nonzero-low
settling, raising again, unknown kinds, acid's unchanged zone-plane behavior,
wet contact versus drained contact, and an ordinary walker crossing a drained
fixture footprint. This guards silent reintroduction of the missing rate and
disconnection from the height-based hazard consumer; it is not a level route.

All **19 promoted human-win recordings** ran unchanged with
`webolution --game-dir "Lemmings Revolution" --replay <file>` and returned
exit 0, PASS, won=1, completed=1. Exact filenames/results are in
`build/acid-lever-drain-001/replays.json`, with one log per basename alongside it.

| Level | Saved / required | Result |
| --- | ---: | --- |
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
| MM_E1_3_SKILLS | 10/5 | PASS |
| M06_KF_1 | 91/50 | PASS |
| MM_E1_4SKILLS | 20/10 | PASS |
| MM_E2_4SKILLS | 14/10 | PASS |
| MM_BASH_DIG_001_A | 20/10 | PASS |
| MM_BUILD_BLOCK_A | 15/10 | PASS |
| E19_KF_2 | 24/20 | PASS |
| E20_MG_1 | 7/5 | PASS |

**No promoted divergence; no recording retirement required.** No replay, replay
check, existing assertion or timeout was modified. The initial diagnostic census
incorrectly treated the map table's image-row field as a slot offset and stopped
with bad_alloc; its partial output is retained as `census-probe-incomplete.txt`.
The corrected source uses the same group/slot join as MapScene; `census-after.txt`
is the complete successful run. This diagnostic error did not affect production.

`git diff --check` passes for the changed tracked files; all five task files use LF.
The supervisor owns mandatory smoke verification after worker exit; it was not
duplicated. No save sink is constructed by replay/census/units, so no installed
or normal user progress was opened or changed. Source backups are under
`build/acid-lever-drain-001/pre-edit/`. The supervising lease was already owned
by PID 46052; no protected controls were edited. CIM metadata was denied;
Get-Process showed the supervising Python processes and no live clone/build at
task entry. Existing original-log, queue and generated-status edits remain.

**Limits:** mean-height contact, integer actor coordinates, hybrid/shared-dt
timing and provisional fluid depth rendering remain unchanged. No fresh pixel
comparison or natural crossing/win is claimed. The original's timing/depth,
historical fidelity gates, parked work and full completion criteria remain open.

Handoff: acid's missing rate is connected; the three installed linked acid levels reach their stored lows, and all 19 promoted wins pass.
Next: supervisor smoke and independent review, then the user's One Way Ticket playtest; acceptance remains pending.

**What to check:** One Way Ticket (node 22), operate the switch at clone (75,36)
from the rightward-pulling side, then allow about 26 shared simulation seconds.
The green acid tank at X=128..136 should visibly lower to empty; lead an ordinary
lemming across its former acid footprint and confirm it survives. Retry restores
the full tank. Also check Wheelbarrows of Doom's lower switch (215,13) drains its
three tanks, and Bridges' switch (212,42) lowers its four tanks to the stored
one-unit residual; MM_W_012 still fills and Seafood Sarnie's pad still drains.
