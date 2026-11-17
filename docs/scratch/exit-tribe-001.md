# exit-tribe-001

Status: **needs_review**. Playable v1, [playtest 11 A7](../playtest/2026-11-16-playtest-11.md),
Kriss Kross (**node 19, E19_KF_2**): water lemmings used the normal balloon and
normal lemmings used the water exit. Retaining exit tribes is the next simulation
dependency. Difficulty: difficult; recommendation Opus, assigned Codex override used.
**Mandatory smoke verification remains pending supervisor execution.**

## Production path and static authority

`runApplicationLeg -> loadRetailLevelReport -> read_compiled_level ->
gadgets.entities_426c6b[i].value_98 -> Exit.sheetType -> World::step -> atExit ->
Exit.accepts -> Exited / saved_ / Exit cue`.
Both legacy `tick` and connected `advanceElapsedMsWithReleaseGate / updateSegment`
reach that predicate. An unmatched exit is skipped; the existing actor state
continues, without a save or Yippee. Matching exits retain the existing closed
2x3 trigger, save transition, hazard ordering and Blocker exclusion.

The loader correlates serialized exit records with the existing pose list in
stream order, rejects count disagreement or decoder failure with a load error,
and retains the serialized type rather than deriving behavior from artwork.
All 111 installed records correlate with their pose-derived AABBs, independently
checked against the six stored spatial doubles by the new short legacy unit.
The hatch path was already correct: `findLevelObjects -> readHatchRecords ->
Entrance.sheetType -> spawnFromEntrance -> Lemming.sheetType`.

Registered reference: `C:/progs/lemmings-webolution/Lemmings Revolution/Lemmings Revolution.exe`,
SHA-256 **32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f**.
Used the reverse-engineer-anything skill, Ghidra **12.1.4**, profile digest
`3c06738e45e144d5f073edbd3244c6bff46222f389525fe659bc92dca2ccdece`.
Only static analysis ran; the session was closed without snapshot arguments.

- Retained starting evidence: [structured hatches](native-009-structured-hatch-records.md),
  [hatch/exit presentation](hatch-exit-presentation-001.md), [pipes](pipe-render-001.md),
  [simultaneous release](multi-hatch-simultaneous-001.md), and
  [data/simulation lessons](../lessons/data-and-simulation.md).
- Exit reader **426C6B** calls the spatial reader, then `426D0C` reads a u32
  and `426D14` stores it at exit **+98**. The recovered reader preserves it as
  `Entity426c6b.value_98`. New pseudocode/instructions:
  [query-3](../../build/exit-tribe-001/static/query-3.json),
  [query-4](../../build/exit-tribe-001/static/query-4.json), evidence
  `ev_5ce778c2f7222796bb4c3c2608a57d1aba4badeaa6cd960f1f0db737de232ba7` /
  `ev_e17d44bab991a1cb2577437bf47ace638aa4e94141f278c22f84722e687238e7`.
- Exit update **42688D** calls inclusive point-overlap **430BA1** at **426B5B**.
  **426B79..426B91** compares lemming **+4C** against exit **+98**, allowing
  equality or exit type **4**; mismatch branches directly to the next actor
  at **426C4B**. **426BB7..426BBB** excludes actor type **3**, even at an any
  exit. Only then does **426BE7 / 426C46** construct/attach the save state.
  New [pseudocode](../../build/exit-tribe-001/static/query-0.json) and
  [instructions](../../build/exit-tribe-001/static/query-2.json), evidence
  `ev_de7d4d624e8e17de40c796b9794a3fff2ee5d689d009ff32662ab1a926d6bf86` /
  `ev_8ce7d8372b3f35fdc6f0ec18f32df71b4a576cd4df574fe502642a24fc304891`.
  Pseudocode was checked against these instruction branches. Query-1 used
  interior address 426A00 and resolved to the same function, not a second exit rule.
- Hatch **+B0** is the serialized type (accepted reader **425E50**). Retained
  [release instructions](../../build/multi-hatch-release-001/static/0x00425A95-instructions.json)
  load it at **425BFA**, push it at **425C00**, and call constructor **42DE51**
  at **425C30**. Retained [constructor instructions](../../build/hatch-exit-presentation-001/static/0x0042DE51-instructions.json)
  store that argument to lemming **+4C** at **42E05F**. Evidence IDs
  `ev_529e52e7994f42f2209413a10a0a48aac5cbb0bec1015ee42e9da22e3e9464c8` /
  `ev_0127c213588ee61c76fed2b78a45986552693a321f6650bdba2004233e8b74e8`.

These are static instructions/pseudocode, not observations of original play.
Installed type/model correspondence is **0 normal, 1 water, 2 acid, 4 any**.
No provisional type rule was needed. Existing clone timing and immediate save
accounting remain unchanged and are not original timing/fidelity findings.

## Installed data and exit availability

[Census and units](../../build/exit-tribe-001/census-and-units.log) inspect all
**102 levels / 111 exits**, comparing actual production setup types and boxes
against serialized records. [coverage.csv](../../build/exit-tribe-001/coverage.csv)
joins each level to the retained [102-level sweep matrix](../../build/level-sweep-001/matrix.csv):
every hatch count, exit count and summed hatch population agrees. Every nonempty
hatch has a compatible exit; **zero tribes lose their only usable exit**.
This establishes compatible data, not terrain reachability or 102 winning routes.

All **21** levels mixing tribes or carrying a water exit are listed below.
There are **13 mixed-tribe levels**, **14 water-exit levels**, with six in both sets.
Each exit entry gives tribe and minimum terrain X,Y; all boxes are closed 2x3.
Raw hatch words (Y-up), model names and every exit are retained in coverage.csv.

| Node / level | Released tribes | Exits (terrain min X,Y) |
| --- | --- | --- |
| 8 / MM_W_012 | 20 water | water (132,77) |
| 10 / MM_BASH_MINE_WLR1 | 20 water | water (213,87) |
| 11 / M06_KF_1 | 50 normal, 50 water | water (115,85), normal (4,20) |
| 16 / MM_E4_4_SKILLS | 30 water | water (15,24) |
| 19 / E19_KF_2 | 15 water, 15 normal | water (50,89), normal (76,89) |
| 23 / MM_E1_5_SKILLS | 10 water, 10 normal | normal (157,90), water (97,90) |
| 25 / MM_ALL_8_003 | 30 water | water (12,87) |
| 30 / M08_KF_1 | 25 normal, 25 water | normal (118,91), water (118,16) |
| 31 / E16_KF_1 | 25 normal, 25 water | any (210,80) |
| 43 / M16_MG_1 | 4 water, 10 normal | any (62,21) |
| 46 / SHOW_KF3 | 10 water, 20 normal | water (59,88), normal (205,70) |
| 50 / M20_MT_1 | 3 acid, 50 normal | any (9,46) |
| 57 / E14_KF_1 | 40 water | water (10,76) |
| 67 / H10_MT_1 | 50 water | water (67,90) |
| 68 / VH22_JC | 50 water | water (245,34) |
| 71 / H16_MT_1 | 100 water | water (122,31) |
| 76 / H23_MG_1 | 10 acid, 10 water | any (80,33) |
| 77 / H24_KF_1 | 35 water, 35 acid | any (219,70) |
| 85 / VH09_MT | 75 water, 1 normal | any (202,25) |
| 87 / VH11_MT | 50 normal, 50 water | normal (30,91), water (168,75) |
| 88 / VH12_KF1 | 5 acid, 5 normal, 5 water | any (129,52) |

Kriss Kross's hatch records explicitly store **type 1/count 15/X214/Y88** with
`Entrance_water.a3d`, then **type 0/count 15/X46/Y88** with `Entrance.a3d`.
Its exits store **type 1 / Exit_water.a3d**, then **type 0 / Exit.a3d**.
No filename convention is used to set those types. The nine any exits stay usable;
the two not in the table are H17_MG_1 (node 56, 11 acid) and H06_MT_1 (node 59,
66 normal). E23_JC_1 (node 70) releases 50 acid into its type-2 acid exit (82,22).

## Checks, recordings and limits

Registered UCRT64 GCC **16.1.0** / CMake / Ninja build passed:
[build.log](../../build/exit-tribe-001/build.log), targets `webolution test_exit_tribe`.
The short legacy test checks the full type matrix through World (saved count,
state, continued walking and cue), overlapping exits, and installed type/AABB
propagation/availability. **exit_tribe PASS, 1.09 s**:
[focused.log](../../build/exit-tribe-001/focused.log), [JUnit](../../build/exit-tribe-001/focused.xml).
The census-to-sweep join is retained in [summarize.py](../../build/exit-tribe-001/summarize.py).
Its first reads exposed PowerShell's UTF-16 log encoding / decorated JSON strings;
the script was corrected without changing gameplay/check results.

Ran `build/agent-native/main/webolution.exe --game-dir "Lemmings Revolution"
--replay <file>` sequentially on **all 17 active promoted recordings**, including
every affected recorded level. All returned **0, PASS, won=1, completed=1**.
Earlier commentary incorrectly counted 18; the directory has 17 active files.
Exact filenames, logs and outputs: [replays.json](../../build/exit-tribe-001/replays.json).

| Recording level | Step | Saved/required |
| --- | ---: | ---: |
| MM_CLIMB_002 | 1252 | 10/1 |
| MM_FLOAT | 978 | 10/1 |
| MM_BOMB | 954 | 17/10 |
| MM_BLOCK_001_A | 1619 | 24/15 |
| MM_BUILD | 1685 | 20/10 |
| MM_MINE | 1137 | 20/10 |
| MM_BASH_002 | 1844 | 10/10 |
| MM_DIG_000 | 1014 | 19/10 |
| MM_W_012 | 1742 | 16/10 |
| MM_BLOCK_BOMB_A | 1546 | 12/10 |
| MM_BASH_MINE_WLR1 | 2272 | 20/10 |
| MM_BASH_DIG_001_A | 3241 | 20/10 |
| MM_BUILD_BLOCK_A | 2482 | 15/10 |
| MM_E1_3_SKILLS | 2095 | 10/5 |
| M06_KF_1 (playtest 11) | 1544 | 91/50 |
| MM_E1_4SKILLS | 2506 | 20/10 |
| MM_E2_4SKILLS (playtest 11) | 2142 | 14/10 |

**Introducing - Water Lemmings (MM_W_012) remains winnable** on its existing
recorded human route: all 20 released actors and its only exit are type 1;
the promoted recording still completes 16/10. No promoted recording diverged
or needs retirement for this task. Retired recordings were not run or modified.

Also ran the assigned **unpromoted** Kriss Kross attempt
`build/playtest/replays/E19_KF_2-1794889005988486200-6.replay` unchanged:
[log](../../build/exit-tribe-001/kriss-kross-attempt.log). It returns **1, DIVERGED
at step 1194 / clock 45412 ms**: expected spawned/saved/dead **26/1/0**, actual
**26/0/0**, both phase 0. The only simulation change is the exit predicate:
the former first save is now rejected for tribe mismatch. This is the intended
correction, not a reproduced win or a claim that the rest of this recording runs.

Changed: `src/sim/World.h`, `World.cpp`, `RetailLevel.cpp`,
`src/__test__/test_exit_tribe.cpp`, `CMakeLists.txt` (one legacy registration),
and this report. Incoming source/CMake edits are preserved under
[entry/](../../build/exit-tribe-001/entry/); [task-only.patch](../../build/exit-tribe-001/task-only.patch)
isolates this task. Supervisor PID **15712** holds RepositoryLease; no protected
controls were edited. CIM metadata access was denied; Get-Process provided the
available process check, with no clone/build runner at entry. Source LF and
scoped `git diff --check` passed. A misplaced JUnit output directory was moved
under this task's ignored build directory; no generated file remains at root.
No original/oracle, debugger, hooks, capture, installs, subagents, concurrent
writers, commits or pushes ran. Replays open no save; commands used private
`build/exit-tribe-001/appdata`. Normal/installed saves were not used.

No smoke/full profile was duplicated. The supervisor must run the smoke gate,
then independent review; visible acceptance remains the user's next playtest.
The installed census does not prove geometric reachability, every level's
winnability, or original save timing. Full-project criteria and parked fidelity
gates remain unchanged.

Handoff: serialized exit tribes now gate saving; all 102 levels retain compatible exits, and all 17 promoted wins pass.
Next: supervisor smoke and independent review, then human confirmation of playtest 11 A7.

What to check: **Kriss Kross, node 19 / E19_KF_2** — send water lemmings through
the normal balloon and normal lemmings through the water exit: each continues
walking without a save or Yippee. Each tribe should save at its own exit.
**Introducing - Water Lemmings, node 8 / MM_W_012** — play the usual route;
water lemmings still save at the sole water exit and can win.
