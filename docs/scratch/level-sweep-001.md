# level-sweep-001: all-level clone smoke sweep

Status: assessment executed; task left `needs_review`. Everything below is a **scripted clone observation**:
direct level entry through the production `runApplication`, no player input, scripted source-test frame timing.
It is not frontend-reachability evidence, not winnability, not original timing and not original fidelity. No
original EXE, copied-byte oracle, debugger, hook or capture ran; the reference EXE
(SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`) was not executed.
Nothing was fixed. No production file, existing test, assertion, timeout or protected control changed.
Artifacts (ignored by git): `build/level-sweep-001/`.

## Headline result (observation)
* **102/102 levels completed the production load -> render -> idle-simulation path.** No load error, exception,
  assertion, abnormal exit or watchdog kill (every child exit code 0, 0 watchdog kills, 0 parent-written failure records).
* **0/102 blank or near-uniform settled or final frames** by the thresholds below, and no two levels share a settled
  frame hash. I also looked at all five contact sheets: every level shows a recognisable tube view with HUD and panel.
* 99/102 released every lemming before the time cap. 3 did not (nodes 11, 91, 97), see finding 2.
* The notable shared findings are scenery-fluid partial admission (19 levels), the single-cadence release model on
  multi-hatch levels (33 levels, 2 observed shortfalls), and a possible teleport ping-pong (3 levels, inference only).

## What ran
* Driver `src/__test__/test_levelsweep.cpp`; CTest `levelsweep_chunk_00..16` (17 chunks x 6 nodes, 102 nodes) and
  `levelsweep_report`. Registered in `CMakeLists.txt` after the `playablebaseline_*` block. Each chunk owns
  `build/level-sweep-001/chunk-NNN/`; the report depends on all chunks.
* **Isolation:** the chunk test is a parent that starts one child process per node (`test_levelsweep level N <chunkdir>`)
  with stdout+stderr to `node-NNN/child.log`, a 420 s wall watchdog (TerminateProcess) and a parent-written
  `CRASH`/`HANG` record when a child leaves no result. One level's crash or hang therefore cannot hide another. This
  sweep produced none; the failure path was not exercised (not claimed as tested).
* **Enumeration:** `app::MapScene::build(lib, {})` then `levelForNode(0..101)` (decoded frontend map nodes). The id "as
  stored" is that map spelling; "as resolved" is `sim::resolveRetailLevel`'s archive id and match tier.
* **Per-level invocation (direct entry, labelled):** `runApplication(["level-sweep","--game-dir",<dir>,"--level",<stored id>,
  "--sim","--frames",F,"--no-audio"], {}, LevelFrameObserver, {frameSeconds=1.0})`, SDL dummy video/audio, 640x480
  window raster (as the ordinary level route). `--frames`+`frameSeconds` is the deterministic owned clock: **scripted
  source-test timing, 1.0 s of level time per presented frame**; the production clock rules are unchanged. The
  explicit-update path is documented to split elapsed time into world updates of at most one tick boundary
  (`main.cpp:1058` comment, `World::updateSegment`), so lemming stepping should not be coarser than a normal run; that is a
  source-comment inference, not measured for 1 s versus 0.2 s frames, and a different frame step could change idle outcomes.
  The only injected event is `SDL_QUIT` when the idle window ends. No save store is passed to `runApplication`,
  so nothing reads or writes any save; each chunk also hashes the normal user save before/after (report only,
  `chunk.txt`).
* **Idle duration, from level data.** Release is `arm + first countdown` then one lemming per `releaseIntervalSeconds`
  (first hatch's RELEASERATE.PC row; `LevelSetup` carries one cadence). Expected release end
  `E = arm + first + (total-1) * interval`; the run stops 5 s of level time after the last lemming is spawned, or at
  `limit + 2 s` (untimed: `E + 60 s`), or on an end transition, with a frame cap of `ceil(min(cap, E + 15 s) / 1.0 s) + 60`. Observed stop
  reasons: 99 `idle-window-after-release`, 3 `sim-cap` (nodes 11, 91, 97).
* **Observation points:** the first frame with `cameraIntroActive == false`, no entry-cue frame and the level menu closed
  ("settled", `frame-settled.png`, `thumb.rgb`) and the last observed frame (`frame-final.png`). Every level
  reached a settled frame.
* **Frame statistics (`settled_*` / `final_*` in `matrix.csv`):** distinct colours (whole frame and the view rows 0-319
  above the panel), mean/sd luminance of the view, modal-colour fraction, black fractions, FNV-1a frame hash.
  Verdict `BLANK` if view distinct colours <= 2 or modal fraction >= 0.98; `NEAR-UNIFORM` if modal >= 0.90 or sd < 8;
  else `ok`. Observed ranges over the 102 settled frames: 17,325-68,443 distinct view colours, modal-colour fraction
  0.0022-0.0457.
* **Scenery admission:** a second, driver-side call `BackdropScene::build(lib, bytes, storedId)` on the same bytes and id
  the run uses (the run's own instance is not exposed). `empty()` plus `diagnostics()` give the branch.
* Gadget types: `findLevelObjects` placed `.a3d` basenames, plus the loaded `LevelSetup` teleport/speed-pad/lever/fluid-gate
  counts and terrain hazard/trampoline/anti-gravity cell counts.

## Build and source identity
Compiler MinGW-w64 GCC 16.1.0 (`C:/progs/msys2/ucrt64/bin/g++.exe`), SDL 2.32.10, `-O2 -g` main suite build by the
registered verifier. Archive sizes `lemmings.box` 322,893,635 B, `patch.box` 8,533,284 B. FNV-1a-64 of sources at the
time of the measured run (`build/level-sweep-001/identity.txt`): `main.cpp 8a0154bf1d7f5011`,
`BackdropScene.cpp 56cb70d12240f36d`, `AppEntry.h 6ace91f8168201c7`, `World.cpp 87a817adbc968bb9`,
`RetailLevel.cpp 5a3ed48d7d7a91c8`, driver `f5555ee942d49ec7`. The tree already held uncommitted basher-fix-001 changes
(`World.cpp/.h`, `test_world.cpp`, CMake); they are not mine and were preserved. **After the measured run I changed one
line of the driver** (guard `#ifndef NOMINMAX` to silence a redefinition warning); the matrix below was produced by the
pre-guard build, and the supervisor's verification rebuilds with the guard.

## Per-level matrix (102 rows)
Columns: node; id as stored on the map (`->` resolved archive id when the spelling differs by case; every one of the
20 resolved as `ExactId`, i.e. case-insensitive whole-id match); lemmings / need / time limit; hatches H and exits X;
released/expected; end-of-idle state counts (W walker, F faller, D dead, E exited, T teleporting, S splatter; E = saved);
simulated seconds and wall seconds of the run; scenery (`A` = admitted with no diagnostic, `A*` = admitted with a
"receiver fluid not admitted" diagnostic, see finding 1). Every row: load ok, run status 0, outcome `completed`,
settled frame verdict `ok`, final frame verdict `ok`. "End states" are an **idle, no-input** outcome and say nothing
about winnability.

| node | stored id | lem/need/limit s | H/X | rel/exp | end states | sim s | wall s | scen |
|---:|---|---|---|---|---|---:|---:|---|
| 0 | MM_CLIMB_002 | 10/1/300 | 1/1 | 10/10 | W10 | 34.0 | 2.4 | A |
| 1 | MM_FLOAT | 10/1/300 | 1/1 | 10/10 | D5 F1 W4 | 34.0 | 2.6 | A |
| 2 | MM_BOMB | 20/10/300 | 1/1 | 20/20 | W20 | 57.0 | 4.2 | A |
| 3 | MM_BLOCK_001_A | 30/15/300 | 1/1 | 30/30 | D26 F2 W2 | 84.0 | 5.5 | A |
| 4 | MM_BUILD | 20/10/300 | 1/1 | 20/20 | F1 W19 | 57.0 | 4.1 | A |
| 5 | MM_MINE | 20/10/300 | 1/1 | 20/20 | W20 | 57.0 | 4.3 | A |
| 6 | MM_BASH_002 | 10/10/300 | 1/1 | 10/10 | F1 W9 | 34.0 | 2.2 | A |
| 7 | MM_DIG_000 | 20/10/300 | 1/1 | 20/20 | W20 | 57.0 | 4.8 | A |
| 8 | MM_W_012 | 20/10/300 | 1/1 | 20/20 | E4 F1 W15 | 62.0 | 4.0 | A |
| 9 | MM_Block_Bomb_A -> MM_BLOCK_BOMB_A | 20/10/600 | 1/1 | 20/20 | D15 F1 W4 | 75.0 | 4.5 | A |
| 10 | MM_BASH_MINE_WLR1 | 20/10/600 | 1/1 | 20/20 | W20 | 75.0 | 6.1 | A |
| 11 | m06_kf_1 -> M06_KF_1 | 100/50/180 | 2/2 | **75/100** | D35 F37 W3 | 182.0 | 16.2 | A |
| 12 | MM_BASH_DIG_001_A | 20/10/300 | 1/1 | 20/20 | W20 | 57.0 | 4.0 | A |
| 13 | MM_FLOAT_CLIMB2 | 10/6/600 | 2/1 | 10/10 | D6 F1 W3 | 36.0 | 2.5 | A |
| 14 | MM_BUILD_BLOCK_A | 20/10/300 | 1/1 | 20/20 | W20 | 75.0 | 5.6 | A |
| 15 | MM_BLOCK_BUILD_BASH | 25/20/600 | 1/1 | 25/25 | W25 | 46.0 | 3.8 | A |
| 16 | MM_E4_4_SKILLS | 30/20/600 | 1/1 | 30/30 | W30 | 110.0 | 8.3 | A |
| 17 | MM_E1_3_SKILLS | 10/5/600 | 1/1 | 10/10 | W10 | 44.0 | 3.2 | A |
| 18 | MM_E2_4SKILLS | 15/10/600 | 1/1 | 15/15 | D13 W2 | 64.0 | 4.6 | A |
| 19 | E19_KF_2 | 30/20/300 | 2/2 | 30/30 | D26 W4 | 100.0 | 7.1 | A |
| 20 | MM_E1_4SKILLS | 20/10/600 | 1/1 | 20/20 | W20 | 68.0 | 4.6 | A |
| 21 | E20_mg_1 -> E20_MG_1 | 10/5/600 | 1/1 | 10/10 | W10 | 26.0 | 1.6 | A |
| 22 | MM_ALL_8_001 | 50/40/480 | 1/1 | 50/50 | W50 | 86.0 | 10.6 | A* |
| 23 | MM_E1_5_Skills -> MM_E1_5_SKILLS | 20/16/600 | 2/2 | 20/20 | D13 F3 W4 | 75.0 | 5.4 | A* |
| 24 | MM_ALL_8_002 | 20/10/600 | 1/1 | 20/20 | W20 | 38.0 | 3.6 | A |
| 25 | MM_ALL_8_003 | 30/20/600 | 1/1 | 30/30 | D25 F1 W4 | 110.0 | 8.1 | A |
| 26 | E18_KF_2 | 30/25/300 | 1/1 | 30/30 | D29 F1 | 153.0 | 10.4 | A |
| 27 | E22_KF_1 | 50/49/360 | 1/1 | 50/50 | F1 W49 | 205.0 | 22.2 | A |
| 28 | E11_KF_1 | 45/40/480 | 1/1 | 45/45 | W45 | 185.0 | 18.1 | A |
| 29 | e06_kf_1 -> E06_KF_1 | 10/5/300 | 1/1 | 10/10 | W10 | 44.0 | 3.2 | A |
| 30 | M08_KF_1 | 50/40/600 | 2/2 | 50/50 | D47 S1 W2 | 205.0 | 11.9 | A |
| 31 | E16_KF_1 | 50/45/300 | 2/1 | 50/50 | D23 E23 F1 W3 | 155.0 | 10.6 | A |
| 32 | M08_mg_1 -> M08_MG_1 | 100/98/240 | 1/1 | 100/100 | D90 F3 W7 | 107.0 | 9.9 | A* |
| 33 | M11_KF_1 | 99/85/600 | 1/1 | 99/99 | F1 W98 | 302.0 | 51.3 | A |
| 34 | M04_KF_1 | 32/32/360 | 2/1 | 32/32 | W32 | 82.0 | 5.1 | A |
| 35 | H02_JC_1 | 50/50/240 | 1/1 | 50/50 | W50 | 86.0 | 8.0 | A* |
| 36 | E07_JC_1 | 25/10/300 | 1/1 | 25/25 | W25 | 93.0 | 7.3 | A |
| 37 | M02_MG_1 | 20/20/300 | 1/1 | 20/20 | D11 F1 W8 | 103.0 | 7.4 | A |
| 38 | MM_TELE_003 | 100/90/600 | 1/1 | 100/100 | **T94** W6 | 167.0 | 23.4 | A* |
| 39 | M03_mg_1 -> M03_MG_1 | 100/99/600 | 1/1 | 100/100 | D97 F1 W2 | 503.0 | 32.6 | A |
| 40 | E17_KF_1 | 50/45/600 | 1/1 | 50/50 | D48 F1 W1 | 155.0 | 10.5 | A |
| 41 | M01_KF_1 | 25/25/600 | 1/1 | 25/25 | W25 | 24.0 | 1.9 | A |
| 42 | M10_JC_1 | 50/46/300 | 2/1 | 50/50 | W50 | 106.0 | 11.3 | A |
| 43 | M16_mg_1 -> M16_MG_1 | 14/11/600 | 2/1 | 14/14 | **T6** W8 | 28.0 | 2.1 | A* |
| 44 | M17_MT_1 | 60/48/300 | 1/1 | 60/60 | **T44** W16 | 124.0 | 13.8 | A* |
| 45 | VH04_JC1 | 50/42/240 | 2/1 | 50/50 | W50 | 32.0 | 3.2 | A |
| 46 | show_kf3 -> SHOW_KF3 | 30/25/600 | 2/2 | 30/30 | W30 | 54.0 | 4.5 | A |
| 47 | M03_MT_1 | 101/96/240 | 2/1 | 101/101 | F3 W98 | 168.0 | 19.4 | A |
| 48 | M18_MT_1 | 50/45/360 | 1/1 | 50/50 | D41 F2 W7 | 86.0 | 7.1 | A* |
| 49 | Show_MT3 -> SHOW_MT3 | 25/12/480 | 1/1 | 25/25 | W25 | 104.0 | 7.4 | A |
| 50 | M20_MT_1 | 53/48/210 | 2/1 | 53/53 | D26 W27 | 43.0 | 3.2 | A* |
| 51 | M21_KF_1 | 100/75/900 | 2/1 | 100/100 | W100 | 167.0 | 21.8 | A* |
| 52 | M19_MT_1 | 50/48/300 | 1/1 | 50/50 | W50 | 86.0 | 10.8 | A* |
| 53 | H01_KF_1 | 50/49/360 | 1/1 | 50/50 | D47 F1 W2 | 181.0 | 12.8 | A |
| 54 | H03_KF_1 | 4/4/300 | 4/1 | 4/4 | W4 | 11.0 | 1.0 | A |
| 55 | M12_MT_1 | 50/46/480 | 1/1 | 50/50 | W50 | 86.0 | 10.8 | A* |
| 56 | H17_mg_1 -> H17_MG_1 | 11/10/600 | 2/1 | 11/11 | W11 | 10.0 | 0.9 | A* |
| 57 | e14_kf_1 -> E14_KF_1 | 40/20/300 | 1/1 | 40/40 | D37 F1 W2 | 165.0 | 11.2 | A* |
| 58 | H05_KF_1 | 50/43/600 | 1/1 | 50/50 | F4 W46 | 106.0 | 11.6 | A |
| 59 | H06_MT_1 | 66/50/360 | 3/1 | 66/66 | W66 | 215.0 | 26.1 | A |
| 60 | VH02_JC1 | 50/45/420 | 2/1 | 50/50 | E21 W29 | 57.0 | 5.1 | A |
| 61 | H08_KF_1 | 10/10/600 | 1/1 | 10/10 | W10 | 22.0 | 1.9 | A |
| 62 | H09_KF_1 | 50/50/300 | 1/1 | 50/50 | D50 (world finished) | 86.0 | 5.5 | A |
| 63 | M07_mg_1 -> M07_MG_1 | 100/98/600 | 2/1 | 100/100 | E34 W66 | 107.0 | 9.1 | A |
| 64 | M15_mg_1 -> M15_MG_1 | 15/15/600 | 1/1 | 15/15 | W15 | 30.0 | 2.2 | A |
| 65 | H12_mg_1 -> H12_MG_1 | 10/4/1200 | 1/1 | 10/10 | W10 | 31.0 | 2.7 | A |
| 66 | H13_MT_1 | 100/99/225 | 1/1 | 100/100 | D98 F1 W1 | 203.0 | 15.3 | A |
| 67 | H10_MT_1 | 50/46/300 | 1/1 | 50/50 | W50 | 86.0 | 8.8 | A |
| 68 | vh22_jc -> VH22_JC | 50/44/195 | 1/1 | 50/50 | D36 F3 W11 | 57.0 | 5.2 | A |
| 69 | H15_MT_1 | 50/30/330 | 1/1 | 50/50 | W50 | 17.0 | 2.5 | A |
| 70 | E23_JC_1 | 50/48/200 | 1/1 | 50/50 | W50 | 41.0 | 3.6 | A* |
| 71 | H16_MT_1 | 100/99/360 | 1/1 | 100/100 | W100 | 167.0 | 24.8 | A |
| 72 | H18_mg_1 -> H18_MG_1 | 20/16/600 | 1/1 | 20/20 | W20 | 11.0 | 1.3 | A |
| 73 | H20_KF_1 | 10/10/600 | 1/1 | 10/10 | W10 | 22.0 | 1.7 | A |
| 74 | E25_JC_1 | 20/19/95 | 1/1 | 20/20 | W20 | 27.0 | 2.1 | A* |
| 75 | H22_MT_1 | 50/48/240 | 1/1 | 50/50 | W50 | 181.0 | 19.1 | A |
| 76 | H23_mg_1 -> H23_MG_1 | 20/18/600 | 2/1 | 20/20 | D10 W10 | 75.0 | 5.2 | A* |
| 77 | H24_KF_1 | 70/64/600 | 2/1 | 70/70 | E30 W40 | 118.0 | 12.1 | A* |
| 78 | e10_kf_2 -> E10_KF_2 | 15/12/300 | 1/1 | 15/15 | D13 W2 | 64.0 | 3.8 | A |
| 79 | VH01_JC1 | 50/49/300 | 2/1 | 50/50 | D25 F5 W20 | 106.0 | 9.0 | A |
| 80 | H08_MT_1 | 2/2/600 | 1/1 | 2/2 | W2 | 8.0 | 0.8 | A |
| 81 | VH05_JC1 | 50/40/300 | 1/1 | 50/50 | F1 W49 | 106.0 | 12.7 | A |
| 82 | VH06_MT | 53/52/240 | 3/1 | 53/53 | D13 F5 W35 | 18.0 | 2.0 | A |
| 83 | VH07_MT | 50/48/225 | 1/1 | 50/50 | D49 F1 | 86.0 | 6.3 | A |
| 84 | VH08_MT | 60/51/300 | 2/1 | 60/60 | D29 F1 W30 | 149.0 | 15.3 | A |
| 85 | VH09_MT | 76/76/240 | 2/1 | 76/76 | E32 W44 | 23.0 | 2.9 | A |
| 86 | VH10_MT | 50/50/375 | 1/1 | 50/50 | D48 F1 W1 | 86.0 | 6.1 | A |
| 87 | VH11_MT | 100/98/300 | 2/2 | 100/100 | E49 W51 | 27.0 | 3.0 | A |
| 88 | VH12_KF1 | 15/15/600 | 3/1 | 15/15 | W15 | 78.0 | 6.2 | A* |
| 89 | VH13_KF_1 | 50/40/300 | 1/1 | 50/50 | F1 W49 | 125.0 | 13.7 | A |
| 90 | VH13_MT | 51/51/300 | 2/1 | 51/51 | F2 W49 | 18.0 | 2.7 | A |
| 91 | e10_kf_1 -> E10_KF_1 | 50/50/180 | 4/1 | **36/50** | D36 | 182.0 | 11.5 | A |
| 92 | VH15_KF_1 | 50/40/600 | 1/1 | 50/50 | W50 | 125.0 | 15.2 | A |
| 93 | VH16_MT | 30/29/300 | 1/1 | 30/30 | E8 W22 | 54.0 | 4.9 | A |
| 94 | M02_JC | 20/5/300 | 1/1 | 20/20 | D19 F1 | 75.0 | 5.3 | A |
| 95 | VH18_JC | 100/98/250 | 1/1 | 100/100 | F2 W98 | 167.0 | 27.3 | A |
| 96 | VH19_JC | 100/94/225 | 2/1 | 100/100 | W100 | 167.0 | 23.4 | A |
| 97 | VH21_JC | 50/49/210 | 2/1 | **42/50** | D36 W6 | 212.0 | 14.7 | A |
| 98 | VH22_KF1 | 50/49/180 | 1/1 | 50/50 | D49 F1 | 181.0 | 12.1 | A |
| 99 | MM_H_001 | 20/20/120 | 4/4 | 20/20 | W20 | 21.0 | 1.9 | A |
| 100 | MM_H_002 | 45/45/180 | 2/1 | 45/45 | D23 F2 W20 | 17.0 | 2.0 | A |
| 101 | MM_H_003 | 100/99/600 | 1/1 | 100/100 | W100 | 407.0 | 70.9 | A |

Full data (all fields, the log tail and diagnostics) is `build/level-sweep-001/matrix.csv`; per-level records and
frames are `build/level-sweep-001/chunk-NNN/node-NNN/{result.txt,parent.txt,child.log,frame-settled.png,frame-final.png}`.
Contact sheets (5 x 5 thumbnails, 160x120, yellow node number): `contact-sheet-0.png` (nodes 0-24) to
`contact-sheet-4.png` (100-101). Auto groups: `groups.txt`. Identity: `identity.txt`.

## Failures and findings, grouped and ranked by levels affected
Observation = measured in this sweep. Inference = from reading source, not measured here.

| rank | group | levels | kind |
|---:|---|---:|---|
| - | crash, hang, exception, load/resolve/render failure, blank frame | 0 | observed |
| 1 | Scenery fluid receivers not admitted | 19 | observed diagnostic |
| 2 | Multi-hatch levels run on one shared release cadence | 33 (2 observed shortfalls, a 3rd is level data) | observation + inference |
| 3 | Lemmings accumulate in `teleporting` | 3 | observation, cause inference |
| 4 | Unexplained 37 simultaneous fallers | 1 | observation, unexplained |

**1. Scenery fluid receivers not admitted (19 levels).** Nodes 22, 23, 32, 35, 38, 43, 44, 48, 50, 51, 52, 55, 56, 57,
70, 74, 76, 77, 88. `BackdropScene::build` succeeds (scene not empty) but returns diagnostics of the form
`<level> FluidGate entities_42acac[i]: receiver kind 1|2 wave constants are not measured; receiver fluid not
admitted`. Kind 1 on nodes 43, 44, 48, 50, 51, 52, 55, 56, 57, 70, 74 (11 levels); kind 2 on nodes 22, 23, 32, 35, 38,
76, 77, 88 (8 levels); no level showed both in the first diagnostics read (full text per level is `scenery_detail` in
`matrix.csv`/`result.txt`). Observed: all 19 render a settled frame. Inference from the diagnostic and the
fluid-surface work: those levels' linked fluid surfaces are not drawn, so a lever-driven fluid change would not be
visible there. Whether that blocks winning a given level was not tested. The other 83 levels have no scenery
diagnostic. No level hit the earlier "empty partial scene" branch: all 102 builders returned a non-empty scene.
Whole-level admission is therefore not the blocker the scene-admission-001 census (10 of 102) showed.

**2. Single release cadence on multi-hatch levels (33 levels have more than one hatch).** Source fact
(`src/sim/RetailLevel.h` header): `LevelSetup` carries one release cadence, from the first hatch; `World::spawn()`
round-robins the hatches against it. Observed: three levels were still releasing when the time cap hit.
* Node 11 `M06_KF_1`: 2 hatches `n50@rate30 (2.39 s)` x2. Observed 75/100 released at 182 s. Inference: one shared 2.39 s
  accumulator needs 100 x 2.39 = 238 s > the 180 s limit; per-hatch cadences (50 x 2.39 = 119.5 s) would finish in time.
* Node 91 `E10_KF_1`: 4 hatches `n7@5.00 s, n13@3.00 s, n20@2.01 s, n10@4.03 s`. Observed 36/50 released. Inference: the
  first hatch's 5.00 s x 50 = 247 s > 180 s; per-hatch times are about 35-40 s.
* Node 97 `VH21_JC`: 2 hatches `n1@5.00 s, n49@5.00 s`. Observed 42/50 released at 212 s. Inference: even independent cadences
  need 49 x 5.00 = 245 s > the 210 s limit, so this one may be level data (or depend on a release-rate control the clone
  does not offer), not the shared cadence.
Nothing was changed; release/timing stays parked, and this is only a report of a demonstrated count. A fix would sit in the
release-timing area; see recommendations.

**3. Lemmings accumulate in `teleporting` (nodes 38, 43, 44).** At the end of the idle window MM_TELE_003 had 94 of 100
lemmings in state `teleporting`, M16_MG_1 6 of 14 and M17_MT_1 44 of 60, with 0 saved and 0 dead. The other 15 teleport
levels did not show it (idle lemmings did not reach a pad there, or were not in the state at the sample), so the
effect is level-dependent. Inference (source reading, **not verified**): `World::stepTeleporting` sets the lemming to
`Walker` at the partner pad's cell (`World.cpp:304-318`), and the pad arm at `World.cpp:960-966` re-enters `Teleporting` for
any Walker inside a pad box, so a lemming landing inside the partner box may bounce back repeatedly. A sampled
position trace of these three levels would confirm or refute it. This is a possible soft-lock for the teleport levels,
nothing more is claimed.

**4. Node 11 ends with 37 lemmings in `faller`.** The final frame shows lemmings on the spiral ledges to the right of the
tube, so it may be ordinary stair-to-stair descent; not diagnosed. Listed so a later playtest of this level looks at it.

**Other observations, not failures.** All 20 mixed-case map ids loaded (level-selection-case-001 fix holds under direct
`--level`; frontend selection of those nodes not exercised here). 8 levels had lemmings exit during the
no-input idle run (nodes 8, 31, 60, 63, 77, 85, 87, 93), which says only that an unaided path to an exit exists there.
Node 62 had all 50 lemmings dead and the world finished before the idle window; nodes 11, 91, 97 and 98 reached
the time limit (`timed_out`). Many levels end with all or most lemmings dead; with no input this is expected.

## Skill and gadget census (for play coverage)
Per level: `build/level-sweep-001/census.md` (skill stocks for the eight skills, teleports, speed pads, levers,
fluid gates, hazard/trampoline/anti-gravity cells, decoded `.a3d` gadget models with counts) and `matrix.csv`.
Tallies I verified from that table:
* **Teleports** present in 18 levels: nodes 38, 43, 44, 46, 48, 49, 51, 52, 55, 59, 67, 80, 84, 86, 88, 90, 95, 97.
* **Speed pads** in 9 levels: 49, 51, 56, 67, 69, 80, 82, 93, 97.
* **Levers** in roughly 55 levels (hand tally of `census.md`, treat as approximate); fluid gates appear on many levels (see the `gate` column).
* **Trampolines:** 0 cells in all 102 levels. **Anti-gravity cells** in 18 levels (nodes 11, 30, 33, 34, 46, 49, 55, 56,
  59, 61, 63, 65, 67, 73, 80, 83, 94, 96).
* **Skill stock:** the early nodes teach one skill each (0 Climber, 1 Floater, 2 Bomber, 3 Blocker, 4 Builder, 5 Miner,
  6 Basher, 7 Digger); levels with all eight skills in stock include 22, 24, 26, 33, 46, 49, 51, 72, 83, 88, 93, 94 (not an
  exhaustive list); node 80 (2 lemmings) and node 54 (4 lemmings) are tiny; node 39 (503 s of level time) and node 101
  (407 s, also the slowest in wall time at 70.9 s) are the longest idle runs.
* Gadget models seen: lever, pipe_supply, pipe_drainage, pressure_pad, teleport, speedup_two_way, anti_grav, deathroller,
  buzzsaw, Crab, Scorp, robin, PlaceHolder, light_green.

## Limitations
* Direct entry (not the frontend), no input, 1.0 s scripted frames, idle outcome only. No claim about winnability, level
  difficulty, skill behaviour, animation, audio or original fidelity.
* The blank-frame test looks at pixel statistics and my reading of the thumbnails; it cannot prove a frame is correct.
  Contact sheets show the first settled frame (lemmings are mostly not out yet); frame-final.png shows the end.
* Admission is read through a second `BackdropScene::build` call, not the run's own instance.
* The parent's crash/hang path was written but never triggered in this sweep.
* The wall times are from a four-job parallel CTest run on a machine also running other suite tests; they are not performance
  measurements. Sum of chunk wall times ~957 s CPU, 23.6-125.6 s per chunk.
* The normal user save was hashed before/after per chunk (report only); it was never a target of any write.

## Smallest recommended next fixes (ordered by value for playable v1)
1. Probe then fix teleport re-entry (finding 3) if confirmed: three levels, plus likely every teleport level once a player
   uses them. Needs a position/state trace first; falls in a parked-adjacent area, so ledger authorization applies.
2. Per-hatch release cadence for multi-hatch levels (finding 2): 33 levels, 2 observed starvation cases. A release-timing
   change; it needs explicit lead approval.
3. Fluid receiver kinds 1 and 2 (finding 1): 19 levels, but only matters where fluid is lever-driven; needs the unmeasured
   wave constants (research first).
4. Add a position trace to a later play-coverage pass so finding 4 (node 11 fallers) can be settled.

## Checks
* `python tools/agent_control/verify.py --profile main` run once in the foreground (exit 0): **170/170** registered tests passed,
  total CTest time 522.28 s (foreground tool output; build tree `build/agent-native/main`); the build had one warning from this
  driver (the NOMINMAX redefinition, since guarded). The
  17 `levelsweep_chunk_*` tests passed in 23.6-125.6 s each and `levelsweep_report` passed (102/102 records, 5 contact
  sheets). `git diff --check` reported only the existing CRLF notice for `agent/tasks.json`.
* Recovery profile and the post-edit rebuild (the one-line NOMINMAX guard) are **pending supervisor verification**; I did not run
  them.
* The tests assert driver integrity only. The guard line was not rebuilt by me.

## Changed files
`src/__test__/test_levelsweep.cpp` (new), `CMakeLists.txt` (level-sweep block only), `docs/scratch/level-sweep-001.md` (this
report); ignored artifacts under `build/level-sweep-001/`. Pre-existing uncommitted edits (basher-fix-001) were left alone.
