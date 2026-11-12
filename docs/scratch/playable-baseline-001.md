# playable-baseline-001: clone end-to-end playable baseline

Status: assessment executed (revision 2 after review); task left `needs_review`. All results below are **scripted
clone observations** (SDL events queued by a source-built driver, `src/__test__/test_playablebaseline.cpp`). They are
not human natural play, not original-timing evidence and not original-fidelity evidence. No production file changed;
no original EXE/oracle/debugger/hook/capture was used. Full artifacts: `build/playable-baseline-001/`
(ignored by git; mirrored summary in `build/playable-baseline-001/report.md`).

Revision 2 change: the driver now has Bomber logic, so route 3 (MM_BOMB) is genuinely played with skills instead of
being a no-input timeout. Numbers below are from the verifier run of this revision (the level time is the production
wall clock plus the public Fast-forward latch, so counts vary run to run; revision 1 got e.g. MM_CLIMB_002 10/10 in 793
ticks, this run 10/10 in 1132 ticks).

## Route / outcome matrix
| # | Level (node) | Entry | Selection reason | Outcome | Saved / need | Sim time (wall) | Persistence (fresh process) |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | MM_CLIMB_002 (0) | bare startup -> Play Current Game -> map click | task-fixed; Climber x10 | **WIN** | 10/10, need 1 | 226.4 s sim (34.9 s wall, post-FF ~7.2x) | **VERIFIED**: flag[0]=1, 1504-byte file, identical map |
| 2 | MM_FLOAT (1) | same frontend path | adds Floater + water/acid cells (608) | **WIN** (poor) | 2/10, need 1; 8 dead | 61.8 s sim (11.4 s wall) | **VERIFIED**: flag[1]=1 |
| 3 | MM_BOMB (2) | same frontend path | adds Bomber x20 (need 10/20) | **WIN** with real Bomber play (8 Bombers primed, 8 clicks / 8 assign_ok) | 12/20, need 10; 8 dead | 98.8 s sim (16.6 s wall) | **VERIFIED**: flag[2]=1, map equals the play invocation's, save file hash unchanged by loading |

"Win" means the clone's own results screen reported `success` with saved >= required; the driver never edited
World/actors/terrain/stock/progress and injected no unlocked map or shortened clock. The 8 deaths on MM_BOMB equal the
8 primed Bombers (each Bomber lemming is consumed by its own explosion); the other 12 lemmings exited.

Route 3 detail (`build/playable-baseline-001/route-3/`): first Fast-forward click at tick 15, key `3` selects Bomber, then
left clicks on lemmings at pos 146/25, 174/23 (top ledge), 116/45, 144/45, 82/63, 92/87, 99/68, 84/87 (lower ledges, i.e.
lemmings reached lower ledges after the first holes). Every click was `assign-ok` (stock 20 -> 12). The results screen showed
WIN 12/20 and the map flag was set; `actors-at-end` in the trace lists the 12 survivors `exited@165/87`.
`on_target` is 4/8: the one-frame-later check for "fuse armed on the clicked lemming" missed in 4 cases while the stock was
spent in all 8, so click targeting remains approximate (see finding 3).

## What was run
* Build identity: `build/playable-baseline-001/identity.txt` (compiler 16.1.0 UCRT64, SDL 2.32.10, FNV-1a of the
  production sources read and of the driver, `lemmings.box` 322,893,635 B / `patch.box` 8,533,284 B). Built by the registered
  verifier (`build/agent-native/main`), CTest names `playablebaseline_{survey,play_N,restart_N}`.
* Invocation (every route, no diagnostic options): `runApplication(["playable-baseline","--game-dir",<Lemmings Revolution>])`
  = ordinary frontend default; `ApplicationRunOptions.saveStore` = `FileSaveSink` at
  `build/playable-baseline-001/route-N/save/LemmingsWebolution/SavedGame.dat` (guarded against the installed dir);
  `frameSeconds = 0` (production wall clock), no `--frames`. Audio off, SDL dummy video/audio.
* Frontend inputs, from the read-only `FrontendWindowObserver`: left click (320,363) Play Current Game; left click at
  the map pixel that `MapScene::pickNode` maps to the node; on the results screen left click (320,360) Continue.
* Level inputs, from the read-only `LevelFrameObserver`: one left click on plate 8 (public **Fast-forward**, recorded in
  every trace; sim ran about 7.2-7.8x wall after it), skill selection by number keys, skill assignment by left click on
  the projected actor box centre. Policy: strategy 1 "reactive" (Climber when the terrain shows a >=3-high wall ahead, Floater
  when a drop is ahead, else Basher/Miner at a wall, and **new: Bomber** on a Walker with no wall within 8 cells and no
  earlier-primed lemming within 40 cells on the same ledge band (|dy| <= 10), at most 8 Bombers; strategy 2 "blanket"
  spacing 70, at most 5, only used for the one retry after a loss, which route 3 did not need). The policy still has no
  Blocker/Builder/Digger logic. Policy decisions read World/terrain state only; nothing is written.
* Watchdog: `limit_s/3 + 90` s wall (190 s for the 300 s levels); never reached. Termination would be reported INCOMPLETE.
* Persistence: separate OS process (`restart N`) opens the same save file with a fresh `runApplication`, clicks Play
  Current Game and compares completion flags/map states and file hash. Loading did not write (saveCount 0, hash unchanged).
* Normal user save `%LOCALAPPDATA%/LemmingsWebolution/SavedGame.dat` was recorded before/after every play run
  (`normal_save_unchanged=1` in each `play-result.txt`); the installed game directory is never a save target.

## Level selection
`build/playable-baseline-001/level-survey.txt` decodes every map node's level (read-only). On the fresh zero-progress map only
nodes 0, 1 and 2 are selectable by `pickNode` (MM_CLIMB_002, MM_FLOAT, MM_BOMB), so routes 2 and 3 were picked by a
deterministic feature rule among the frontend-reachable levels (no unlocked state was injected). That limits "distinct
interactions" to Floater+water and Bomber (terrain destruction, multi-ledge descent). Gadget levels (teleport MM_TELE_003
node 38, lever MM_W_012 node 8, speed pad M21_KF_1 node 51) are **still not attempted**: reaching them through the frontend
needs genuine wins on the intervening nodes, several of which need Blocker (node 3), Builder (4), Miner (5), Basher (6),
Digger (7) play that this driver has no policy for, and the budget of this revision went to the Bomber fix. This is a scope
gap, not a demonstrated blocker. Manual procedure and next step are below.

## Ranked player-visible findings
1. **Gadget levels untested.** Teleport, lever and speed-pad levels have no baseline evidence; unlocking them needs real wins
   on nodes 3-7 (and probably more). Impact: the largest unobserved part of the playable game.
2. **MM_FLOAT: 8 of 10 lemmings died after two Floater assignments** (win by threshold only; 9/10 died last revision). The
   clicks landed off-target (`on_target=0`, Floaters still spent). Whether deaths are splat/drown and whether better timing
   saves more is unverified (no actor-state log beyond counts).
3. **Click targeting is approximate**: Climber clicks hit 10/10 this run (9/10 in revision 1), Floater 0/2, Bomber 4/8 by the
   one-frame-later check; the stock is spent either way. Likely driver/pick ambiguity (box-centre pick), not proven a clone defect.
4. **20 of 102 map node ids fail `loadRetailLevel` when passed in the map's mixed case** ("not in the archive", e.g. node 9
   `MM_Block_Bomb_A`, 11 `m06_kf_1`, 21 `E20_mg_1`). Verified this revision: the survey now retries the upper-cased id and
   **all 20 load** (e.g. `MM_BLOCK_BOMB_A`, `M06_KF_1`), so the cause is the case-sensitive substring match in
   `findLevelFile` (`src/sim/RetailLevel.cpp`), not missing levels (82/102 decode with the exact id, 102/102 with the
   upper-cased fallback). That is consistent with STATUS saying all 102 levels render. `main.cpp:461`/`:596` use the same
   case-sensitive match on the frontend-selected id, so selecting such a node from the frontend would probably report "no .LVL
   matching", but this was **not exercised** (those nodes are not reachable on a fresh map). A limitation of my survey
   plus an unverified frontend risk, not an observed player failure.
5. No crash, hang, save error or anomaly occurred in any route.

## Limitations / unverified
* Scripted input, wall-clock level time with the public fast-forward latch; fast-forward timing is a source policy, not
  original timing (timing/release/digger parks untouched). Screens are clone image buffers only. Outcomes vary between runs
  because the clock is the wall clock.
* Only 3 levels, all from the fresh-map starting set; no gadget, teleport, lever, speed-pad, anti-grav, multi-hatch level was played.
* One Bomber strategy worked on the first attempt; the retry path (strategy 2, Retry button) was exercised in revision 1 only
  (after a no-input loss), not in this run.
* Armageddon not exercised (parked). "Genuine" win is by the clone's own rules; it does not validate level difficulty or
  original behaviour. The bomb sites were chosen by a driver heuristic, not shown optimal.

## Reproduce
`python tools/agent_control/verify.py --profile all` runs the seven play/restart/survey tests; or after a main build run
`test_playablebaseline play N build/playable-baseline-001` with `SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy WEBOLUTION_GAME_DIR=<Lemmings Revolution>`.
Manual procedure (outside automation): start `webolution` (no args), click Play Current Game, click the node,
press the number key for a skill and click the lemming (for MM_BOMB: key 3, then click walkers on the top ledge and then on
lower ledges, at most ~8, spaced apart), click the fast-forward plate if desired, Continue on results, quit,
restart and check the map.

## Checks
`python tools/agent_control/verify.py --profile all` (single foreground run that moved to the background at 600 s; not restarted;
exit 0): main **114/114** registered tests passed (7 are `playablebaseline_*`: survey 5.3 s, play_1 36.6 s, play_2 13.4 s,
play_3 18.6 s, restarts 0.26-0.28 s each; total CTest time 745.9 s), recovery **84/84** passed, `git diff --check` clean.
No compiler warning appeared in the build output this run (revision 1's unused-parameter `restartMode` warning is gone: the
parameter is now unnamed). These are source-test results; the new tests assert driver integrity only (status 0, level entered
matches target, results observed, outputs written), never gameplay success. Changed files this revision:
`src/__test__/test_playablebaseline.cpp` (Bomber policy, actor dumps, upper-case survey fallback, unnamed unused parameter) and
the two reports; `CMakeLists.txt` registration is unchanged from revision 1.

## Smallest recommended next step
Authorize a follow-up that extends the assessment policy (not production) to Blocker/Builder/Miner/Basher/Digger so that
nodes 3-7 can be won through the frontend, then attempt the first gadget level that unlocks (teleport, lever, speed pad).
Separately, the 20 mixed-case ids (finding 4) deserve one frontend selection attempt once such a node is reachable. Only a
demonstrated failure of a correctly played level justifies a production fix.
