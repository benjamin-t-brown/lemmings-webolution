# balloon-gyrate-001

[Playtest 19 J4](../playtest/2026-11-19-playtest-19.md) reported that the exit basket in **Now you're stalking** (`E22_KF_1`, node 27) followed the accepted departure path without the small circle. The source traced the missing tilt to `BackdropScene::setEntranceExitPresentation`, which translated the exit root and adjusted its heading.

## Static evidence

REA/Ghidra 12.1.4 inspected `C:/progs/lemmings-webolution/Lemmings Revolution/Lemmings Revolution.exe`, SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`. The session closed without snapshot arguments. [Instruction reads, Ghidra pseudocode, xrefs and bytes](../../build/balloon-gyrate-001/rea-static.json)

- `0x4268CA..0x426901` advances the shared idle/departure phase by `dt * 0.5` cycles. `0x426993..0x426A03` sends `0.02 * sin(phase)` to model slot `+0x14` and `0.02 * cos(phase)` to slot `+0x10`, with phase converted to radians by `0x426D90` and `0x426DB0`. `0x49CC52` decodes as `FCOS`, and `0x4B0FB0` stores double `0.02`.
- Vtable `0x4B1688` maps those slots to `0x451821` and `0x451843`. Model positioning at `0x451668` builds **X tilt from cosine, then Z tilt from sine, then Y heading** through `0x481E26`, `0x481F3C`, `0x481D10` and the row-vector multiply at `0x412880`/`0x4128E0`. It writes the model root rotation and separately copies its mapped translation. The basket, ropes and envelope therefore tilt together about the root. The amplitude is **0.02 turns (7.2°) per axis**, with one cycle per **approximately 2 level seconds**.

## Production

Review traced the late-rescue case to `RuntimeSimState::advanceElapsed`. It continues simulating after timeout enters Pending, including after the first second of departure. The earlier sway ramp used the Pending timer alone and could offset the basket while a newly rescued lemming was landing.

`runApplicationLeg` samples each rescue's existing level-clock age from `LemmingMotionTracker::exitBalloonSwayScale` during Pending. `BackdropScene::setEntranceExitPresentation` limits its tilt using that sample and composes the tilt into the shared basket, rope and envelope root before the existing heading. Root translation, idle bob, jump trajectory, camera and Results timing keep their existing inputs.

**Provisional landing blend.** The ordinary departure still starts swaying after one Pending second. A late rescue fades the tilt to center over the first 0.25 seconds of lift-off, holds the centered pose through the one-second jump, and restores sway over the next 0.25 seconds. The smallest scale across overlapping jumps applies to the departing models. The clone also keeps its zero phase seed, radial heading and continuous bob. The original applies full tilt in the departure branch and seeds the phase at load. [Accepted path changes](../../build/balloon-gyrate-001/accepted-path.git-show.txt) · [Accepted bob changes](../../build/balloon-gyrate-001/accepted-bob.git-show.txt)

## Checks

- The registered MinGW-w64 GCC release build linked `webolution`. A focused sample combined the production timeout clock and jump tracker with the installed J4 model. Two synthetic contacts at Pending 1.8 and 2.4 seconds confirmed the centered landing pose through overlap and pause, full sway after the last jump's recovery blend, identical root translation across sway weights, and the timeout Results boundary and saved count. The earlier pose samples for `E22_KF_1` and `MM_CLIMB_002` measured the unrestrained circle. The J4 basket radius was about **1.06 world units**. [Revision build](../../build/balloon-gyrate-001/review-revision/build.log) · [Late-landing sample](../../build/balloon-gyrate-001/review-revision/late-landing.log) · [Earlier pose samples](../../build/balloon-gyrate-001/pose-sample.csv)
- The revised release CLI reproduced **23/23** promoted wins with exit 0, `PASS`, `won=1`, and `completed=1`. `E22_KF_1-1795077450383129100-7.replay` finished at step **24402** with **49/49** saved. Runs used task-private `LOCALAPPDATA`. The replays check simulation and Results. The landing blend still needs a visual playtest. [All recording results and logs](../../build/balloon-gyrate-001/review-revision/replays.tsv) · [J4 route log](../../build/balloon-gyrate-001/review-revision/replays/E22_KF_1-1795077450383129100-7.replay.log)
- The previous iteration passed 95 main smoke and 84 recovery tests. The supervisor smoke gate for this revision is pending. [Previous supervisor run](../../build/agent-runs/20261119T183443-27936-1ffd5e44/verification.log)

Changed `src/app/BackdropScene.{cpp,h}`, `src/app/ExitBalloon.h`, `src/app/LemmingMotion.{cpp,h}`, `src/app/main.cpp`, and this report.

Handoff **needs_review**. The late-landing check and all promoted wins passed.  
Next action is supervisor smoke, independent review and the player's visual check.

**What to check.** In **Now you're stalking** (`E22_KF_1`), save all 49 and watch the last jump and the departure through Results. The basket should trace a gentle circle about every two level seconds once the jump finishes, with the ropes and envelope tilting together. On a run where a lemming reaches the exit more than one second after timeout starts, watch the basket return to center before the landing and resume circling afterward while departure continues.
