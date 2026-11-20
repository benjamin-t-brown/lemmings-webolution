# Inverted Builder on Designed with love

Playtest 22 M2 concerns **Designed with love** (`M11_KF_1`, node 33). The retained pre-fix run of `build/playtest/replays/M11_KF_1-1795151456076104200-17.replay` reproduced the return to the map at step 27079 with 0/85 saved. Coordinates are clone terrain cells with Y increasing downward. [Playtest ledger](../playtest/2026-11-19-playtest-22.md) · [Pre-fix CLI result](../../build/inverted-builder-001/recorded-attempt.log)

| Assignment | What the recorded clone did |
| --- | --- |
| 9964, actor 1 at `(194,65)`, upright, left | The first brick at 10122 filled `(193,65)` and `(192,65)`, supporting the step to `(193,64)`. The twelfth brick led to Shrugger at `(182,53)` at 12465. At 12722 the actor returned to Walker, then turned right and walked back down the stairs. |
| 23392, actor 5 at `(34,45)`, inverted, left | At 23534 the builder filled `(33,45)` and `(32,45)`. The upright rise moved Y to 44 and the forward foot check at `(33,44)` hit the ceiling. The actor turned right into Walker at `(34,44)` with 12 bricks left. |
| 25873, actor 5 at `(73,53)`, inverted, right | At 26028 the builder filled `(74,53)` and `(75,53)`. The upright rise moved Y to 52 and the forward foot check at `(74,52)` hit the ceiling. The actor turned left into Walker at `(73,52)` with 12 bricks left. |

Both inverted assignments succeeded. The recorded state changes and the old `stepBuilder` branch place both failures at the first forward foot check, before a horizontal step, brick decrement or shrug. The brick paint in each case was already on the correct feet row. [Pre-fix World trace](../../build/inverted-builder-001/recorded-attempt-world.log)

## Static evidence and production path

Ghidra statically analyzed the registered x86 `Lemmings Revolution.exe`, SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`. Xrefs put the paint and advance callbacks in the Builder constructor at `0x40F9B2` and `0x40FA10`. The paint instructions at `0x410141..0x4101FE` take the biased feet position and stamp the two columns selected by horizontal facing. The advance at `0x40FBF6..0x40FC29` adds actor `[+0x44]` to the original's Y-up position before the forward foot check at `0x40FE77..0x40FE7F`. On a terrain hit, `0x40FEE6..0x40FEFB` turns and returns to Walker before decrement.

`0x42F5ED..0x42F65D` negates `[+0x44]` and swaps the foot and head Y biases between 0 and 3. Their accessors read them at `0x42F275..0x42F27E` and `0x42F323..0x42F329`. The head is thus three cells against gravity from the feet. With bricks remaining, `0x410032..0x4100A6` samples one column ahead of that head and turns on a hit. The final-brick transition to Shrug at `0x40FF96..0x40FFFF` precedes the head check. Shrug completion calls the Walker transition at `0x41AC06`. The Ghidra session closed without a snapshot. [Ghidra instructions, pseudocode and xrefs](../../build/inverted-builder-001/rea-review-evidence.json)

The frontend and the replay each enter Builder through `World::assign`. The update runs through `RuntimeSimState::advanceElapsed` → `World::advanceElapsedMsWithReleaseGate` → `World::updateSegment` → `World::stepElapsed` → `World::step` → `World::stepBuilder`. After painting at the current feet, Builder now moves one cell against gravity and uses that same direction for the head-height offset. The evidence covers cell geometry and state transitions. Retail floating-point clamping, object-overlap handling, event timing and rendered motion are unverified. [World change](../../src/sim/World.cpp#L1001)

## Checks

- The registered native `webolution` build passed. All 28 promoted `.replay` files reproduced their wins through the headless CLI with private LocalAppData. The log gives the outcome and final step for each file. [Build](../../build/inverted-builder-001/review-webolution-build.log) · [Promoted replays](../../build/inverted-builder-001/review-promoted-replays.log)
- All 22 traced upright events matched the pre-fix run. The inverted builder advanced from `(34,45)` to `(33,46)` at step 23534 and took 11 supported steps through `(23,56)` at 25871. The unpromoted attempt diverges as expected at **25873** because its second Builder click expects actor 5 walking at `(73,53)` and the actor is still building at `(23,56)`. [Upright comparison](../../build/inverted-builder-001/review-upright-comparison.log) · [Fixed CLI result](../../build/inverted-builder-001/review-fixed-attempt-cli.log)
- In a diagnostic copy of the divergence World, a placed inverted follower walked all 12 steps on Built support. The builder reached Shrugger at `(22,57)`, resumed walking and went onto the overhang at `(21,57)`. Both actors fell upward from `(20,57)` when support ended. Two separate copies after step 23534 exercised turns with a solid cell inserted in each. At `(32,47)` the forward foot hit turned the builder at X 33 with 11 bricks left. At `(31,50)` the head hit turned it after advancing to `(32,47)`, with 10 left. [World trace](../../build/inverted-builder-001/review-world-trace.log) · [Trace build](../../build/inverted-builder-001/review-trace-build.log)
- The earlier supervisor run failed only in `actorcompose` because five camera fixtures were missing. [Earlier smoke log](../../build/agent-runs/20261120T062837-16728-3afe8bb1/verification.log)

**Handoff.** `inverted-builder-001` is `needs_review`. Changed `src/sim/World.cpp` and this report.  
**Gate.** Supervisor smoke and human confirmation on Designed with love are pending.

**What to check.** On Designed with love, assign Builder after a lemming reaches the underside from the gravity flipper. Each brick should support a step away from the ceiling, and a following inverted walker should climb onto it. Let one open staircase finish through the shrug, then try another against an obstruction and check that the builder turns.
