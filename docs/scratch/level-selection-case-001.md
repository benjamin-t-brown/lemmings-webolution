# level-selection-case-001: installed map level selection across name capitalization

Status: implemented; task left `needs_review`. All frontend results are **scripted clone observations** (SDL events queued
by a source-built driver, `src/__test__/test_levelselection.cpp`). They are not human play, not original timing and not
original-fidelity evidence. No original EXE/oracle/debugger/hook/capture ran. Artifacts: `build/level-selection-case-001/`
(ignored by git).

## Finding (confirmed through the frontend before any production edit)
The map names 20 of its 102 levels in mixed case (e.g. node 9 `MM_Block_Bomb_A`, node 11 `m06_kf_1`, node 21 `E20_mg_1`);
the archive stores `COMPILED\<UPPER>.LVL`. Both `src/app/main.cpp` (its own substring loop over `levelFileNames`) and
`sim::loadRetailLevel` (`findLevelFile`) matched the id with a case-sensitive `std::string::find`, so those 20 ids found
nothing although `AssetLibrary::read` is case-insensitive and the levels exist.

## Evidence
| Step | Build | Route | Outcome | Artifact |
| --- | --- | --- | --- | --- |
| Before, control | frozen task-entry sources | bare startup -> Play Current Game -> click node 0 | `MM_CLIMB_002` entered, 3 level frames composed (640x480, 306,913 lit px), status 0 | `evidence/before-node0/result.txt` |
| **Before, defect** | frozen task-entry sources | same route, node 9 (fixture below) | **FAIL**: log `frontend: entering MM_Block_Bomb_A from map` then `level: no .LVL matching MM_Block_Bomb_A`; `runApplication` status 1; 0 level frames | `evidence/before-node9/{result.txt,stderr.log}` |
| After, control | current | node 0 | `MM_CLIMB_002` entered, 3 frames, status 0 | `evidence/after-node0/` |
| After | current | node 9 | selected `MM_Block_Bomb_A` -> canonical `MM_BLOCK_BOMB_A` entered, 20 lemmings / need 10, 3 frames (306,301 lit px), status 0 | `evidence/after-node9/` |
| After | current | node 11 | `m06_kf_1` -> `M06_KF_1`, 100 lemmings / need 50, 3 frames | `evidence/after-node11/` |
| After | current | node 21 | `E20_mg_1` -> `E20_MG_1`, 10 lemmings / need 5, 3 frames | `evidence/after-node21/` |

"Entered" means the `LevelFrameObserver` fired: `runApplicationLeg` called `sim::loadRetailLevel`, built the `World`, composed the
tube/backdrop scene and HUD into the 640x480 image for three consecutive frames (`levelName` equals the canonical archive id).
Nothing beyond three frames was played; no gameplay, win or timing is claimed. The one remaining line named `first_error` in the
after-results is the informational `sim: tick=0 ...` message, not an error (driver reporting quirk; the entered flags are authoritative).

### Prepared-save fixture (diagnostic only)
Nodes 9, 11 and 21 are not selectable on a fresh map. For each, the driver finds the single earlier node whose completion
makes it selectable (`MapScene` link rule) and writes an isolated save under `evidence/<label>/save/` with only that node's
completion record set: **node 4** (group 0, slot 4) for node 9, **node 6** (group 0, slot 6) for node 11, **node 14**
(group 0, slot 13) for node 21. The frontend loaded exactly these flags (`flags_as_prepared=1`, `save_load_error=-`).
This is a loading-test fixture, **not** an earned unlock, win, persistence or natural play. Production unlock rules are
untouched. The normal clone save was absent and unchanged in every run (`normal_save_unchanged=1`); the installed game
directory is refused as a save target. Node 0 used a fresh zero-progress isolated save.

## Source and build identity
* Frozen task-entry copies: `build/level-selection-case-001/frozen/src/app/main.cpp` and `.../src/sim/RetailLevel.cpp`,
  written from the working tree before any edit and never rewritten. CTest `levelselection_identity` hashes them with
  `file(SHA256)` against `main.cpp ec816679d261f464f19b947ce0b37f948346899de0f668ca79bcf59b977bddd7` and `RetailLevel.cpp
  bd9dd9e724f6fdca564c4dcc9ff88d9749ab35c7e824a29efc1572b3a7982eab` (passed). **Caveat:** this worker could not locate the
  supervisor's entry manifest for this run; those hashes are the production hashes in the last retained supervisor snapshot
  (`build/agent-runs/loop-4070ec7c9130425f9242b8cbc09c0fab/0004-review-af050c/before-source-hashes.json`, the accepted
  playable-baseline-001 state, which changed no production file). The reviewer should compare against the run's own manifest.
  The copies were typed from file reads, not byte-copied; the passing hash match is the evidence they are faithful.
* The frozen driver (`test_levelselection_before`) links the frozen files in place of the production ones (object library
  `lsc_before_base` + frozen `main.cpp`); the current driver (`test_levelselection_after`) links production. Same driver
  source; `LSC_FROZEN` only changes the expected outcome. No production file was swapped.
* Compiler: registered MSYS2 UCRT64 g++ via `python tools/agent_control/verify.py`.

## Fix
`src/sim/RetailLevel.{h,cpp}`: new public `sim::resolveRetailLevel` (the single resolution used by both layers) with ordered tiers:
1. **ExactId**: whole id equals the archive file's id, ignoring case (so `M06_KF_1` can never be captured by a longer id).
2. **Substring**: the previous case-sensitive first-hit substring match, unchanged (partial ids such as `CLIMB_002`, `\X.LVL`).
3. **CaseInsensitive**: only if nothing above matched, a case-insensitive substring naming exactly one distinct level
   (distinct by `normaliseAssetName`, so spellings of one archive path count once). More than one level is an
   `ambiguous level id` error, never a guess; none is `not in the archive`.
`loadRetailLevelReport` now calls it. `src/app/main.cpp` replaced its private loop with the same call and, when the id matched
other than by the legacy substring tier, continues with the archive's own id so the skill panel, outcome and observers see the
canonical id. Ids that matched the legacy way keep the caller's text (e.g. `--level CLIMB_002` behaves as before). Archive
precedence is unchanged: reads normalise case and `patch.box` wins. The CLI error text for a missing id changed from
`level: no .LVL matching X` to `level: X: not in the archive` (no test asserted the old text).

Precedence note: tier 1 runs before tier 2, so an id equal to a level's whole id now wins over an earlier-sorting level whose id merely
contains it. In the installed data only 2 ids are contained in other ids (checked) and both already resolved to themselves.

## Census and focused checks (`levelselection_census`, `evidence/census.txt`)
102 map spellings -> 102 distinct canonical levels; 20 are mixed-case (82 matched the legacy case-sensitive search; the 20 did not);
all 102 resolve to an archive path whose canonical name is independently `COMPILED\<UPPER(spelling)>.LVL` and all 102
`loadRetailLevel` successfully; failures 0. Also asserted: `CLIMB_002` -> `MM_CLIMB_002` (Substring), `\MM_CLIMB_002.LVL`,
`mm_climb_002` (ExactId), `climb_002` (CaseInsensitive, unique), `NOT_A_REAL_LEVEL_ID`/lower-case/empty fail, `mm_` is rejected as
ambiguous (message lists the candidates), exact-id precedence for every id contained in another, lower/upper-case archive reads
identical bytes for all 102 paths.

## Limitations / unverified
* The installed archive has 102 level paths and **no** path with more than one case spelling, so duplicate-spelling
  deduplication and `patch.box` override precedence for level paths are covered by code and the read-equality check only, not by
  an installed example.
* Only nodes 0, 9, 11, 21 were driven through the frontend; the other 17 mixed-case nodes are covered by the resolver/loader census,
  not by menu clicks. Three rendered frames per route; no gameplay, no teleport/lever/pad behaviour and no timing was exercised.
* Only the shared lookup was changed; the 82 matching ids' behaviour is unchanged by construction, and `MM_CLIMB_002` is
  re-verified through the frontend (before and after).
* The hash-identity reference caveat above. Parked work (D1/D3, release/digger timing, clipping, controller, capture, Armageddon)
  untouched.

## Reproduce
`python tools/agent_control/verify.py --profile all`; or after a main build with `SDL_VIDEODRIVER=dummy WEBOLUTION_GAME_DIR=<game>`:
`test_levelselection_before frontend 9 build/level-selection-case-001/evidence` (expect exit 0 = failure reproduced),
`test_levelselection_after frontend 9 ...` (exit 0 = entered), `test_levelselection_after census ...`.

## Checks
`python tools/agent_control/verify.py --profile all` (one foreground invocation that moved to the background at 600 s; not
restarted; exit 0): main **122/122** registered tests passed (CTest 777.8 s; 8 are new: `levelselection_identity`,
`levelselection_before_node{0,9}`, `levelselection_after_node{0,9,11,21}`, `levelselection_census`), recovery **84/84** passed,
`git diff --check` clean. The earlier 114 main tests, including `playablebaseline_*` and `retaillevel`, still pass with no
assertion or timeout edited. Build output showed no warning from the new files or the production edits (the warnings seen
belong to existing frozen/test sources). Changed files: `src/sim/RetailLevel.{h,cpp}`, `src/app/main.cpp`,
`src/__test__/test_levelselection.cpp`, `cmake/levelselectioncase_identity.cmake`, `CMakeLists.txt` (registration only), this report.
The verifier ran the sources once; no separate pre-fix build of the driver was run by hand, the "before" results come from the
frozen-source driver in that same run.
