# targeted-play-001: targeted Blocker play and earned lever-level observation

Status: revised after review; executed in registered run `20261103-021131` (current); left for independent review. Everything is a **scripted clone observation** (public SDL events queued by
`src/__test__/test_targetedplay.cpp` through the production `runApplication`). It is not human play, not original timing and not original-fidelity
evidence. No production file changed; no original EXE/oracle/debugger/hook/capture ran.
Artifacts (ignored by git): `build/targeted-play-001/runs/20261103-021131/` - `run-id.txt` (save provenance), `identity-prepare.txt`, `step-1-node3-blocker/`, `step-2-node8-lever/`
(`play-result.txt`, `action-trace.csv`, `samples.csv`, `result.md`, `terrain-hazard.txt`, `earned-map.txt`, `identity.txt`, PNGs, `restart-result.md`).
Earlier pre-fix runs `20261103-014256` and `20261103-014825` are retained unedited in the same `runs/` directory; `current-run.txt` is overwritten by later runs, the run directories are not.
`build/targeted-play-001/git-show-main-cpp.txt` records the `git show HEAD:src/app/main.cpp` command and where its (large) output was persisted.

## Revision history of this report
* Review of runs `-014256` / `-014825` found: the driver's 60 s frontend stall guard fired on return from a ~92 s level leg, so the node-3 win was classified INCOMPLETE and `restart_1` printed UNVERIFIED; and this report's Checks text was stale.
* Driver-only fix in `test_targetedplay.cpp` (the `level` lambda): the stall-guard progress clock is refreshed on every level-observer frame, because a level leg has its own watchdog. Nothing else in the driver changed.
* Re-run `20261103-021131` (below) classifies the node-3 win as WIN, observes the Results screen and verifies persistence in a fresh process. Numbers below are from that run; the pre-fix runs are superseded, not edited.

## Static trace: real picker and input ordering (`src/app/main.cpp`, `RuntimeHud.cpp`, `World.cpp`)
* Event loop (`main.cpp` ~900-1019): all pending SDL events are drained first; then `runtimeSim.advance()` (~1048); then the frame is composed and the
  `LevelFrameObserver` runs (~1094). An event pushed from the observer in frame N is consumed at the top of iteration N+1 against the `World` as the observer saw it.
* `SDL_MOUSEMOTION` (~968) sets `pointerX/Y` (no fast-forward latch without a held button). Each frame `hoveredActor(...)` for the pointer is exposed as
  `LevelFrameView::hoveredActor` with `cursorX/Y`. A left click (~942) calls the **same** `hoveredActor()` and then `world->assign()` (`World.cpp:941`), which sets the
  state and spends stock in one call. Paused clicks are processed identically and a successful assign resumes (~962); this driver never needed pause.
* Protocol used: motion to pixel P -> read `hoveredActor` next frame -> click P only if hover == intended -> read stock and which actor entered Blocker.
  By loop order there is no `advance` between the hover observation and the pick. This is **source-trace reasoning**; the empirical support is the agreements below.
* Limits: the 32x32 cursor box + `pickHoveredLemming` rules are SOURCE-policy adapters, not the original picker; `hoveredActor` is the clone's snapshot.
* Normal speed is 5 ticks/s; the fast-forward latch runs 7 passes/frame and is never cleared within a level leg.

## Lever source inspection (before observation)
MM_W_012 (node 8): 20 lemmings, need 10, 300 s, Blocker x10, 4 levers (79,60) (72,34) (183,60) (189,35), 2x6, initial state 1. Clone rule (`World.cpp:161-180,846-851`):
a **Walker** in the lever AABB with facing -1 when `state==1` (else +1) sets `lever.state = facing`. `World.h:202-209` says the original also fans messages to **linked
objects**; the clone has **no consumer** of `Lever::state` (only the load in `RetailLevel.cpp` and the toggle in `World.cpp`). So the only observable lever effect here
is the toggle itself. A linked gameplay effect is not implemented, so its absence is not an original-behaviour claim.

## Route / outcome matrix (run `20261103-021131`)
Save provenance: `run-id.txt` - copied from playable-expansion-001 run `20261103-003501` step 1 (public node-0 win), 1504 bytes, FNV `c05382077a330a32` matched the value
in that accepted report; source file hash unchanged after copy. Flags at the first map: only node 0. No prepared flags. Normal user save absent before and after in both steps (`normal_save_unchanged=1`); installed save not touched.

| Step | Level (node) | Entry | Attempts | Outcome | Persistence |
| --- | --- | --- | --- | --- | --- |
| prepare | node-0 save | copy + hash check | - | verified=1 copied=1 | - |
| 1 | MM_BLOCK_001_A (3) | startup -> Play Current Game -> map click (240,179) | 2 of 3 | A1 **LOSS** 0/30 saved, 29 dead; A2 **WIN 28/30 saved, 0 dead** (need 15), results screen observed (`saveCount=1`, `flag_node=1`), `anomaly=-` | `restart_1` **VERIFIED**: fresh process, flag[3]=1, flags/states equal the play invocation's map, save FNV `980473be299a98f3` unchanged by the restart (was `c05382077a330a32` before the win) |
| 2 | MM_W_012 (8) | same path; node 8 selectable only because of the step-1 completion (map states `2,3,3,2,3,1,1,3,3`, selectable `0 1 2 3(done) 4 7 8`), map click (273,195) | 1 of 2 | **WIN** 20/20, 562 ticks, results screen observed, `lever_toggles=1` | `restart_2` **VERIFIED** (fresh process, flags/states equal, save FNV unchanged `163ecc74695712a2`) |

### Node 3 attempts (terrain-informed plans; `terrain-hazard.txt` in step-1)
Plan A1: ONE Blocker at a hazard-below drop edge. Plan A2 (chosen from A1 evidence: deaths clustered at x=74-78, y=45-77, i.e. bands 2/3/4): cap 2, prefer those bands, one Blocker per band.
* A1: Blocker on lemming 0 at 176/24 (top right ledge, band 1) at tick 65. It stood the whole level (1442 ticks), turned 6 walkers; lemmings 17-29 then fell along x=75-76 and 13 deaths were logged (29 dead total, 0 saved). Interpretation (inference): one right-side Blocker pushed the flow left toward the hazard column.
* A2: Blocker 1 on lemming 0 at 176/24 (decision tick 62, resolved 65) and Blocker 2 on lemming 1 at 80/45 (click tick 178, resolved 180, left ledge, band 2). Result 28 saved, 0 dead, 2 Blockers standing at the end, rest `exited@130/91`.
* The A2 score is an observed clone result of this plan; it is not proof of an optimal policy or of original fidelity. Two attempts of the three allowed were needed; the policy revision after A1 is in `action-trace.csv` (`revision`).

### Targeting (all assignments)
| Attempt | Intended | Hover at obs | Recipient | Stock | Probes | Decision tick | Click tick | Resolve tick | Class |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | 0 (176/24) | 0 | 0 | 10->9 | 1 | 62 | 63 | 65 | CONFIRMED |
| 2 | 0 (176/24) | 0 | 0 | 10->9 | 1 | 62 | 63 | 65 | CONFIRMED |
| 2 | 1 (80/45) | 1 | 1 | 9->8 | 1 | 176 | 178 | 180 | CONFIRMED |
Pixels were the box centre but each was **verified** by the production hover before clicking (cursor == pushed pixel in every probe). No MISDIRECTED, AMBIGUOUS, REJECTED or UNRESOLVED outcome occurred, so the multi-pixel search path is untested. Sample size is 3 sparse, uncrowded targets; this does not show targeting is reliable in a crowd. Decision-to-resolve (2-3 ticks) is observation latency; the click was pushed in the same observed tick as the hover agreement.

### Sampling gaps (not per-tick)
Normal speed did **not** give per-tick observation: A1 skipped 61 ticks and A2 124 ticks before fast-forward (single-tick ranges every 2-4 ticks, i.e. frames are slower than the 5 tps tick cadence in this process); after the fast-forward click gaps were up to 15-18 ticks (A1 1216, A2 1125 skipped). Full ranges are in `result.md` / `play-result.txt`.

### Blocker-crossing candidate
`pass_per_tick=0`, `sign_change_gap_unresolved=0` for all three retained Blockers; facing turns near each Blocker (6 / 5 / 17) were seen only across gaps (`turns_per_tick=0`). The earlier "crossed" heuristic did not reproduce under the stricter criterion (consecutive-tick samples, Walker both samples, same ledge band, |dx|<=8). Because of the skipped ticks above this **neither establishes nor excludes** passing through a Blocker. Verdict: **unverified, not a defect claim**.

### Node 8 lever observation
Attempt 1 (natural flow, no skill, fast-forward latched at tick 16): `lever-toggle lever=2 state=1->-1 at=183/60`, tick 213 (previous sample 201, gap 12), one Walker in the AABB, face -1, pos 178/66. That matches the toggle rule above (state 1 needs face -1). `lever_toggles=1`. The linked effect is **not observable** in this clone (no consumer, see above); the level was then won 20/20 with no deaths, so no evidence links the toggle to the outcome. Attempt 2 was not needed. Only one lever toggled; levers 0, 1, 3 were not exercised. The toggle tick is bracketed (202-213), not exact.

## Driver limitations (separate from any production-defect hypothesis)
1. Fixed in this revision: the frontend stall guard no longer fires on return from a long level leg (see Revision history). It remains a 60 s guard for genuinely idle frontend screens.
2. Single-tick sampling not achieved (above). Whole-attempt wall time was dominated by real-time normal speed (A1 72 s, A2 92 s).
3. Hover agreement was only tested 3 times with a box-centre first pixel; the 8-pixel fallback is untested.
4. Node 8 latched fast-forward at tick 16, so the toggle tick is bracketed, not exact.
5. No Builder rules claim; Builder was not run. No claim about original fidelity or about solvability. The node-3 win is one scripted plan on the clone; it does not show the plan is robust.
6. The registered Blocker/lever tests assert driver integrity and persistence checks; the WIN outcome itself is an observation recorded in the run files, not an assertion.

## Checks
* Driver-only change after review: `src/__test__/test_targetedplay.cpp` (one line plus comment). `git --no-pager show --no-ext-diff --no-textconv HEAD:src/app/main.cpp` was run; see `build/targeted-play-001/git-show-main-cpp.txt`.
* **Worker verification run** `python tools/agent_control/verify.py --profile all` (foreground 600 s, then moved to background, **killed by the background time limit during test #124 `retaillevel` of 136**): build OK (one `-Wunused-variable` warning for `kSkillNames` in the new driver); tests #1-#123 **passed**, including `targetedplay_prepare`, `play_1` (167.3 s), `restart_1`, `play_2` (20.6 s), `restart_2`. Tests #124-#136 and the recovery portion were **not run** in this worker session and were not restarted (the run already used the longest allowed time). This change touches only `test_targetedplay.cpp`, so those untouched tests are not affected by it, but that is inference, not a verified result by this worker.
* Earlier supervisor run for the pre-fix driver (`build/agent-runs/20261103T011941-38484-108edbbd/verification.log`) reported main 136 and recovery 84 registered tests passed; it does not cover the revised driver. The supervisor's independent post-revision run is authoritative for the full profile.
* The worker could not run `git` diff beyond the one permitted command.

## Smallest justified next decision
Independent acceptance review of this driver-only revision and the supervisor's full registered verification. No production change is justified by this evidence: a correctly hover-confirmed, terrain-informed second Blocker plan won MM_BLOCK_001_A (28/30) through the frontend, persisted across a fresh process and unlocked MM_W_012, where the lever toggled per the clone's source rule. Whether any original lever-linked effect exists needs a separate, explicitly scoped static-research decision; this task does not claim it.

## Changed files
`src/__test__/test_targetedplay.cpp` (new in this task; revised one line after review), `CMakeLists.txt` (registration only), this report. No production file, queue, verifier, tool registry, permissions, models, completion criteria, existing assertion/timeout, or parked work changed. No git commit/push/reset/clean/stash.
