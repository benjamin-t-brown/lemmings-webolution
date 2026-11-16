# exit-jump-anim-001 — balloon entry jump

Status: **needs_review**. Playable v1, playtest 7 W5. The installed confirmation
route is **MM_CLIMB_002 / Just Climb**: assign Climbers, watch their balloon
arrivals, then Results. The [human ledger](../playtest/2026-11-15-playtest-7.md)
records the before failure: arrivals disappeared without a jump. Restoring the
missing saved-actor presentation is the next dependency. Difficulty: routine;
recommended model: Sonnet; worker uses the explicit Codex override.
**Supervisor smoke verification is pending after worker exit.**

## Evidence and chosen behavior

Read first: [exit/Yippee research](exit-yippee-sound-001.md),
[exit presentation](exit-balloon-path-001.md),
[hatch/exit integration](hatch-exit-presentation-001.md),
[window lifecycle](frontend-window-quit-001.md), and retained
[exit-state research](../../old/docs/scratch/phase3-round11-exit.md).
The inspected original contact sheets [balloon-a](../playtest/reference/original-2026-11-14/balloon-a.jpg)
and [s2-balloon](../playtest/reference/original-2026-11-14/s2-balloon.jpg) provide
basket/arrival context, but their small actors and half-second spacing do not
establish the exact jump path or sprite order. No new original run occurred.

Static target: `C:/progs/lemmings-webolution/Lemmings Revolution/Lemmings Revolution.exe`,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Project static REA used Ghidra 12.1.4; analysis was closed without snapshots.
[Returned records](../../build/exit-jump-anim-001/static/) retain pseudocode,
instruction reads, exact bytes and provider limitations. Pseudocode is
decompiler output, not recovered source; decisive statements were checked
against instructions:

| Static location | Contract |
| --- | --- |
| `0x426C29..0x426C46` | Exit installs state `0x417F00`, passing itself at state +0x30 and selector 1 at +0x34. |
| `0x41800A..0x418027`, `0x4683E9` | Enter selects string `0x4BF424` (`fall`, verified bytes) and sets controller rotation phase **0.25**, not playback speed. |
| `0x41803B..0x4180FA` | Target delta is exit logical position minus actor position **plus (0,2,0)**. The 2 is Y, not radial Z. |
| `0x4180FF..0x418154`, `0x418350` | Store vector length as jump impulse, entry Y, and the unnormalized delta. Dot-product instructions and square-root continuation bytes (`D9 FA` at `0x49C9EC`) corroborate length. Ghidra's math-helper body ends at a call; the retained continuation byte read supplies that missing opcode. |
| `0x4181AE..0x418252` | Advance timer; set Y from entry Y plus `verticalDir * (length(delta)*t - 12.5*t*t)`, then add `delta*dt`. Y resets before this addition, so its last term is **deltaY*dt**, not deltaY*t; X/Z accumulate. Constant `0x4B03A8` is 0.5, applied to -25. |
| `0x418257..0x418301` | Continue below **1 second**; at >=1 count the save, play Yippee and return completed status 2. `0x4B02D0` bytes confirm 1.0. |

Core instruction evidence: enter
`ev_96128c64559d08a720a0de27aac7bff8d4299c308090271ca4d510c2b74d4dc6`
(`static/0x417FE4-asm.json`), step
`ev_df52150906529fc765932243d16d12f33a8fa44f6db2da6087c779f1fc1f1ad1`
(`0x41818A-asm.json`). These establish a fall-strip
jump, rather than a separate jump/exit strip or a balloon departure change.

Installed artwork, decoded through production `loadSpriteSheet` in the legacy
check: ordinary, water and acid HI sheets each contain **15 fall temporal
entries, 5 directional images per entry, rate 1**. They use their own
`$LemSheetHi*`, `$LemSheetWaterHi*`, and `$LemSheetAcidHi*` textures. The
existing clock advances at 0.06 seconds/entry; its existing fall loop repeats
after 0.9 seconds until the jump disappears at 1 second. Full frame rectangles,
texture names and clip inventory are in
[unit-detail-final.log](../../build/exit-jump-anim-001/unit-detail-final.log).
The installed lemming SPR inventory has ordinary/water/acid HI/LO sheets;
no separate inverted lemming sheet was found. Inversion retains the existing
gravity placement and vertical sprite reflection.

**Save/cue choice:** preserve the clone's accepted immediate recognition,
saved increment and Yippee event. Static evidence ties the original's count/cue
to jump completion, but moving the clone's established event would alter result
timing and replay inputs. This task adds presentation only and does not claim
original save timing. Balloon departure and the level-end transition are unchanged.

## Production path and limits

`runApplicationLeg` already observes `LemmingMotionTracker` before simulation
and after **each** elapsed update, including fast-forward passes. On the first
observed `Exited` state the tracker now retains entry time and radial lane.
`positionOf` samples the traced jump; `exitSeconds` admits it only while age <1 s.
The existing level clock supplies pause and fast-forward; no clock feeds back
into simulation. Each actor owns its own timer, including coincident arrivals.
`LemmingAnimTracker::advance` now advances saved actors' fall strips too.

`injectedWalkersFromWorld` appends these jumps **after the existing live prefix**,
carrying fall frame, type, gravity and the traced quarter-turn phase.
`TubeScene::drawInjectedWalkers` -> `prepareInjectedWalker` -> cylinder pose ->
directional SPR selection -> ordinary sprite raster/depth path draws them.
Picking, hover, countdown and observer mappings still consume only live actors
from that prefix. World actors remain `Exited`: no skill assignment, blocker
field, movement/contact updates or extra save/cue can be produced by a tail.

Provisional placement: invert the accepted exit-trigger box to recover the
rounded static root (centre X, native root Y = box maximum Y +6, radial 1.5),
then add the traced Y+2. This omits the original root's sub-cell placement and
idle bob. X takes the nearest wrapped displacement. Starting lane comes from
the existing clone motion adapter, and the update-sized Y term uses the clone's
existing update slices. Temporal phase retains the accepted frame-end animation
adapter. No exact original within-frame ordering, depth fidelity, basket
occlusion or inverted natural-arrival comparison is claimed. The last actor's
jump may overlap the already accepted early departure because completion still
starts immediately; playtest 6 V5 remains deferred. Human visual acceptance is
pending; no new image/replay fixture, frozen build or scripted gameplay route
was added. Historical fidelity gates and parks remain unchanged.

## Checks and handoff

Registered UCRT64 GCC 16.1/CMake build, `-O2 -g` with assertions, jobs 8:
[build-final.log](../../build/exit-jump-anim-001/build-final.log), success.
New **legacy** `exit_jump` passes in **0.03 s**:
[unit-check-final.log](../../build/exit-jump-anim-001/unit-check-final.log).
It protects independent lifetimes, exact expiry, advancing/frozen frames,
live-prefix indices, type/gravity propagation, inactive skill exclusion and
unchanged save/cue multiplicity. These synthetic contacts are logic checks,
not evidence of natural gameplay. An initial new-test fixture incorrectly
assumed a contact between bookkeeping ticks had already exited; the retained
`unit-check-1.log` records that failure. The fixture now actually steps the late
contact before checking its lifetime. No existing assertion or timeout changed.

All twelve playtest-7 recordings were run with the final built `webolution
--game-dir "Lemmings Revolution" --replay <file>`; **every result is PASS,
exit 0, won=1, completed=1**, with no divergence. Individual logs and exact
recording paths: [replay-results.tsv](../../build/exit-jump-anim-001/replay-results.tsv).

| Playtest 7 level | Saved / required | Result |
| --- | --- | --- |
| MM_CLIMB_002 | 10/1 | PASS |
| MM_FLOAT | 10/1 | PASS |
| MM_BOMB | 17/10 | PASS |
| MM_BLOCK_001_A | 24/15 | PASS |
| MM_BUILD | 20/10 | PASS |
| MM_MINE | 20/10 | PASS |
| MM_BASH_002 | 10/10 | PASS |
| MM_DIG_000 | 19/10 | PASS |
| MM_W_012 | 16/10 | PASS |
| MM_BLOCK_BOMB_A | 12/10 | PASS |
| MM_BASH_MINE_WLR1 | 20/10 | PASS |
| M06_KF_1 | 93/50 | PASS |

The three promoted playtest-8 wins also PASS, exit 0, won=1, completed=1:
MM_BASH_DIG_001_A **20/10**, MM_BUILD_BLOCK_A **15/10**, MM_FLOAT_CLIMB2 **10/6**.
Replays verify simulation/results and do not exercise the render path.

Changed: `src/app/{LemmingMotion,LemmingAnim,TubeScene}.{h,cpp}`,
`src/__test__/test_exit_jump.cpp`, five CMake registration lines, this report.
Incoming edits were preserved; source backups and the task-only patch are in
`build/exit-jump-anim-001/`. Supervisor PID **26408** holds RepositoryLease.
CIM process metadata was denied; Get-Process supplied the process check, with
no clone/build/test runner before edits. No protected control was edited.
Runs use private LocalAppData `build/exit-jump-anim-001/appdata`; unit/replay
runs open no saves. Normal and installed saves were untouched. No original,
oracle, hooks, debugger, capture, install tools, subagent, background worker,
commit or push ran. The supervisor owns the smoke gate; it was not duplicated.

Handoff: exit jumps are implemented; focused legacy check and all 15 promoted human-win replays pass.
Next: supervisor smoke and independent review, then human playtest confirmation of W5.

**What to check:** **MM_CLIMB_002 / Just Climb**, assign Climbers and watch the
balloon: each arrival visibly jumps into the basket for about one level second,
including close arrivals, then disappears. Clicking a jumper must not assign a
skill or prevent another arrival; pause freezes it, fast-forward speeds it up,
and Results still reports ten saved. On **MM_W_012** check the water artwork;
on **M06_KF_1** check that an inverted arrival retains its orientation and jumps
along its gravity axis. Balloon departure behavior should remain as before.
