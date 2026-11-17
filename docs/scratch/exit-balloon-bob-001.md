# exit-balloon-bob-001 — slow exit balloon bob

Status: **needs_review**. Playable v1, [Playtest 9 Y8](../playtest/2026-11-16-playtest-9.md):
exit balloons were static. Confirmation route: **Just Climb / MM_CLIMB_002**,
assign Climbers and watch arrivals at the balloon. The missing idle motion is
the next presentation dependency after the accepted exit jump. Difficulty:
routine; recommended model: Sonnet; worker uses the explicit Codex override.
**Smoke verification is pending supervisor execution after worker exit.**

## Static evidence

Read first: accepted [exit departure](exit-balloon-path-001.md),
[exit jump](exit-jump-anim-001.md), [hatch/exit presentation](hatch-exit-presentation-001.md),
and [scenery clock](scene-branch-impl-001.md) research. Earlier omissions and
historical fidelity gates remain unchanged; this task supplies the omitted idle bob.

Only static REA/Ghidra analysis ran, on the registered
`C:/progs/lemmings-webolution/Lemmings Revolution/Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Ghidra 12.1.4 returned the [retained records](../../build/exit-balloon-bob-001/static/),
including limitations; analysis closed without snapshot arguments. Pseudocode
is decompiler output, checked against instructions for the following findings.

| Static location | Finding |
| --- | --- |
| `0x4268CA..0x426901` | Exit phase at +0xA0 advances by `dt * 0.5`, then calls `0x426D90`. |
| `0x426D90..0x426DAD`, `0x49CBA2` | Double phase, multiply by stored pi, take sine. The latter instruction is **FSIN**, bytes `D9 FE`; Ghidra's short CRT entry body alone omits this continuation. |
| `0x4B0F88`, `0x4B03A8` | Stored doubles are `3.1415927410125732` and `0.5`. Thus amplitude **0.5 logical height unit**, period approximately **2 level seconds**. |
| `0x426AC0..0x426B09`, `0x423275` | While waiting, set logical Y to stored base at +0xA8 plus `0.5*sin(phase*2*pi)`; forward the logical triple to the entire model via slot +8. Basket, ropes and envelope move together. |
| `0x426D31..0x426D51`, `0x4040A0` | Level load seeds phase through the existing random helper and stores the unbobbed logical Y. |
| `0x45DAEF..0x45DAF9`, `0x45DD63..0x45DD76`, `0x4B17B8` | Paused/non-play update takes the branch that supplies double **zero** to `0x4423D2`. Exit phase therefore does not advance while paused. |
| Retained jump instructions `0x41803B..0x4180FA` | The original jump captures the exit logical position plus `(0,2,0)` on entry, so its target includes that entry's bob. See the accepted exit-jump report and its instruction records. |

Key new evidence: exit update instructions
`ev_b0c198e311c05b6747f9452479251b4b8a2092a3d1448c2ab615ebd794046a01`,
sine wrapper `ev_5a6af6cb8147246c8eadaaefcdd31beb459f0fb11ec4d0d6aa50f7056e0cb69a`,
FSIN `ev_8311672b10dac076fd5e7dd5ba0aeab3edf5f9737bed1703a6089aea270ba2db`,
load instructions `ev_7c7fe72228254a31eb08376e5aa13734cead025ba1e18adf6285c0a375d71a66`,
pause branch `ev_55506fb963c53542be49bb01f0e1532dafe85d1a619dcc20de69597ba30f4c97`.

## Production path and limitations

`runApplicationLeg` samples `World::levelClockMs()` after the existing simulation
updates and supplies it to `BackdropScene::setEntranceExitPresentation`.
The shared `ExitBalloon.h` sine sample offsets the serialized logical exit Y;
the existing installed coordinate lattice maps it into a translation from the
saved root matrix. Normal scene traversal draws the whole model. Absolute
sampling avoids cumulative drift and resets naturally on reload. Pause freezes
both balloon and actors; fast-forward uses the same existing level-time budget.

`LemmingMotionTracker::positionOf` uses that same bob for saved actors' existing
one-second jump; its correction blends from zero at contact to the basket's
offset at the jump's end, with Y-down conversion independent of actor gravity.
`injectedWalkersFromWorld` -> `TubeScene::drawInjectedWalkers` retains the fall
strip, radial trajectory, live-prefix indexing, nonassignable tail and expiry.
`World::step`/`atExit`, trigger boxes, save increment, cue emission, end timer and
result accounting are unchanged. Saves and Yippee still occur at the accepted
recognition tick, before presentation completion.

**Provisional choices:** every exit starts phase zero instead of reproducing
the original randomized load phase; the clone's existing level clock supplies
time. The jump follows the current bob by a linear blend instead of altering
its accepted impulse from the original's entry snapshot. Idle sine continues
during the existing departure path so Pending cannot snap the model to its
unbobbed pose; the original instead rises from its current height and applies
small departure rotations. Existing departure rates/trigger/result timing are
preserved. Exact sub-cell jump placement, basket occlusion, last-arrival overlap
with early departure, native clock fidelity and departure rotations remain the
previously documented limitations. No new natural gameplay observation or
original comparison is claimed; human visual acceptance is pending.

## Checks and preservation

- Registered UCRT64 GCC 16.1/CMake build, `-O2 -g` with assertions, jobs 8:
  [build-final-2.log](../../build/exit-balloon-bob-001/build-final-2.log), success;
  [build-final.log](../../build/exit-balloon-bob-001/build-final.log) includes the
  production executable link. Initial `build.log` retained a new-test name
  collision, corrected without changing production behavior.
- Extended existing **legacy** `exit_jump` passes in **0.06 s**:
  [focused-final.log](../../build/exit-balloon-bob-001/focused-final.log).
  New logic checks use installed MM_CLIMB_002, MM_W_012 and M06_KF_1 exit roots,
  protect amplitude/period, shared basket/jump offsets for both gravities,
  pause, fast-forward expiry, unchanged immediate save/cue multiplicity, fixed
  trigger and continuous departure onset. Synthetic contacts are supplemental,
  not gameplay routes. Initial `focused.log` retains a fixture's incorrect
  zero-delta contact assumption; it now uses the established bookkeeping tick.
  Existing assertions/timeouts and the smoke tier were not changed.
- Ran the final built `webolution --game-dir "Lemmings Revolution" --replay <file>`
  for **all 15 promoted .replay files**. Every result is **exit 0, PASS, won=1,
  completed=1**, with no divergence. Exact commands, paths and individual logs:
  [replay-results.json](../../build/exit-balloon-bob-001/replay-results.json).

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

Replays verify simulation/results, not rendered bob appearance. Two retired
playtest-9 recordings stay excluded/untouched per their
[README](../playtest/replays/README.md). No replay fixture, frozen-source build,
identity check or scripted play route was added. No full/gate suite was duplicated.

Changed: `src/app/ExitBalloon.h`, `BackdropScene.{cpp,h}`, `LemmingMotion.cpp`,
`main.cpp`, `src/__test__/test_exit_jump.cpp`, `src/__test__/HatchEntryFixture.h`,
this report. Incoming edits are
preserved in [entry/](../../build/exit-balloon-bob-001/entry/); a task-only patch
separates this work from them. Parent supervisor PID **46016** retains
RepositoryLease. CIM metadata was denied; Get-Process supplied the process
check, with no clone/build/test runner before edits. No protected controls,
queue, shared instructions, completion criteria or existing test assertions
were edited. Source text is LF. Runs set private LocalAppData to
`build/exit-balloon-bob-001/appdata`; replays/units do not open SaveStore.
Normal and installed saves were not opened or changed. No original execution,
oracle, debugger, hooks, capture, install tools, subagents, background jobs,
commits or pushes ran.

## Verification-failure retry (20261117T003037-46016-c45dfbea)

The preceding supervisor [smoke log](../../build/agent-runs/20261117T001949-46016-ff78a88a/verification.log)
failed **frontendentry**, 94/95 main tests passing; recovery was not reached.
This corrects any implication that the first worker's checks established a
passing gate. Both failures were exact image comparisons: the independent
entry compositor still sampled a stationary balloon, while production now
bobs at the frame's 208 admitted milliseconds. The retry changes only
`HatchEntryFixture.h` and this report; prior production work is retained.

The existing compositor now samples bob at its explicit frame time, floored
to integer milliseconds with the existing clock's epsilon. Door animation
keeps its independent sub-ms sample. Zero-time controls and **every original
full-frame assertion, timeout and test registration remain unchanged**. This
corrects the expectation encoding Y8, without masks or pixel tolerances.

Registered GCC/CMake rebuild succeeded:
[retry build](../../build/exit-balloon-bob-001/retry-20261117T003037/build.log).
Targeted checks pass: **frontendentry 30.36 s**, **exit_jump 0.07 s**:
[focused log](../../build/exit-balloon-bob-001/retry-20261117T003037/focused.log).
All **15** promoted human recordings were rerun sequentially using
`build/agent-native/main/webolution.exe`; each returned **0, PASS, won=1,
completed=1**, with the same per-level saved/required results in the table above.
[Individual commands and results](../../build/exit-balloon-bob-001/retry-20261117T003037/replay-results.json)
retain each output log. The initial summary checked stdout alone; replay
diagnostics are on stderr. The corrected summary reads the retained combined
logs, with no replay reruns needed. No replay file changed.

The same live supervisor PID **46016** holds RepositoryLease; no build/game
runner was live before edits. Retry backups and incoming diff are under
[retry entry](../../build/exit-balloon-bob-001/retry-20261117T003037/entry/).
Retained static instructions were rechecked; no new REA session or original
execution occurred. Private LocalAppData remained under the task build tree.
**The final smoke profile remains pending supervisor execution.**

Handoff: static-backed idle bob and synchronized exit-tail correction implemented; frontendentry, focused legacy check and all 15 promoted replays pass; needs_review.
Next: supervisor smoke and independent review, then human playtest confirmation; phase and jump/departure adapters remain provisional.

What to check: **Just Climb (MM_CLIMB_002)**, watch the waiting exit balloon
for several seconds, then assign Climbers. Basket, ropes and envelope should
bob together gently up/down about every two level seconds, with arrivals still
jumping into the basket and the same save count. Pause should hold both model
and jump; resume should continue smoothly, fast-forward should speed them up,
and final departure should start without a height snap. Check the water exit
on **MM_W_012** and inverted arrivals on **M06_KF_1** when next playing them.
