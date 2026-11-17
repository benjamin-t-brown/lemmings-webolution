# digger-facing-001

**needs_review; supervisor smoke pending.** Playable v1, implementation-plan
playtest-ledger work order. Before: [Playtest 12 B3](../playtest/2026-11-17-playtest-12.md)
reports side-facing diggers. Installed route: **Just Dig, MM_DIG_000**. Selecting
the readable front image is the next presentation dependency. Difficulty routine;
recommended Sonnet; assigned Codex override used.

Changed files: `src/app/TubeScene.cpp`, `src/__test__/test_actoranimstate.cpp`
and this report. Incoming acid-drain/actor-zoom changes, including earlier edits
to TubeScene, were preserved; backups and the incoming diff are under
`build/digger-facing-001/backup/` and `incoming.diff`. Parent supervisor PID 46052
owns RepositoryLease; no protected controls were edited. CIM process metadata
was denied; Get-Process showed no clone/build/test runner at entry.

Production path: `runApplicationLeg -> LemmingAnimTracker::advance/poseOf ->
injectedWalkersFromWorld -> TubeScene::drawInjectedWalkers ->
prepareInjectedWalker -> selectSpriteView -> actorSpritePlacement ->
drawActorSprite`. Before, the correct `dig` strip used the walking tangent and
camera-dependent chooser. Now its five-direction entries select **index 4,
unmirrored**, before explicit diagnostic overrides. Placement, lighting, gravity,
zoom and occlusion still use the shared actor path. Screen boxes and hover use
the same preparation, so picking follows the displayed rectangle. Once the clip
changes, ordinary directional selection resumes; no simulation facing, skill,
position, timing or save rule changes.

## Installed data and static authority

Retained starting evidence: [A3](phase-a3-sprite-view.md),
[actor animation](actor-anim-001.md), and
[digger entry/clock research](phase-d2i-skill-assign-digger.md).
Fresh read-only asset inspection used the production archive overlay/parser:
[inspector](../../build/digger-facing-001/inspect-assets.cpp),
[inventory of every temporal entry in all six sheets](../../build/digger-facing-001/assets.tsv),
[rectangles](../../build/digger-facing-001/asset-rectangles.txt) and
[reviewed directional sheet](../../build/digger-facing-001/directions.png).
The sheet's rows are dig/block/shrug/explode, columns are zero-based directions.
Dig has 20 HI / 10 LO temporal entries, each with five directions across ordinary,
water and acid resources. Its camera-facing image is direction 4. HI first-entry
rectangle: `Sheets\$LemSheetHi20`, `(232,155)..(255,186)`.

Used reverse-engineer-anything and static REA/Ghidra **12.1.4** on only the registered
`Lemmings Revolution/Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Complete returned packets, including pseudocode, instructions and limitations:
[static-evidence.json](../../build/digger-facing-001/static-evidence.json).
Profile: `3c06738e45e144d5f073edbd3244c6bff46222f389525fe659bc92dca2ccdece`.
Closed analysis without snapshot arguments; no original execution occurred.

- Digger entry `4159B4`: `4159BD..4159C5` selects `dig` at `4BF350`;
  `4159CA..4159DA` passes binary64 **0.75** to `4683E9`. Leave `415995`
  passes zero at `41599C..4159A9`. Setter instructions `4683F6/4683FF`
  write controller `+1D8/+1DC`. Pseudocode checked against instructions.
  Instruction evidence IDs: `ev_863e84a0507ce14e8f2ef07338a17b1c22a0a47d50797e3b18bfcef95184c5e2`,
  `ev_bab1ec895d0a3c7a6154a80b746bc0bb9fabe8f23c02aef73fd89043eb3b3285`,
  `ev_ae84aef10a9d329670ac880930ee097b8a502b9645e28ffd42cd6b183ca89b61`.
- Actor render `42EEF2`: facing at `42EF38`, basis setter at `42EF9F`,
  chooser at `42EFD8`, state pre-render dispatch at `42EFFE`. Digger vtable
  `4B09F0` gives the no-op `4011D0` for that slot. These reads establish a
  phase-based native choice, **not a fixed camera-facing original at every yaw**.
- **Blocker:** eight directions, 16 HI / 8 LO entries. Entry `40E20C` selects
  `block`, then `40E237..40E247` sets **0.25**; leave `40E120..40E12D`
  resets zero. This missing orientation is now copied to the production actor's
  `rotationPhase` while blocking; a fresh sample restores zero afterwards.
  It retains native camera-dependent eight-view selection, rather than forcing
  its front index. Instruction evidence:
  `ev_84bf649e11d2ec77193b5751cb16cb4f2c86785979c721f5df58b480ddff3e2b`.
- **Shrugger:** one direction per entry (8 HI / 4 LO), already front-facing;
  no correction needed. The generic half-turn entry `41A3DE` found through
  phase-setter xrefs cannot alter a singleton's selected image.
- **Explosion/countdown:** `explode` has five directions (17 HI / 8 LO).
  Flapper entry `41695C/416993` selects that strip without setting phase;
  vtable `4B0A58` also uses no-op pre-render. No forced-front explosion was
  established. The lit fuse retains the current job's strip; countdown text
  is a separate overlay. Neither is changed. Exit jump already supplies its
  evidenced phase 0.25 (`418027`), also unchanged.

**Provisional/unknown:** always choosing dig view 4 is the requested readable
clone policy, not a reproduction of native phase 0.75. Native all-angle appearance,
device depth and full-frame fidelity remain unresolved. No original observation
or new human gameplay confirmation is claimed. Rendering units isolate submission
and scene occlusion; they do not establish natural gameplay or occlusion fidelity.

## Checks

Registered UCRT64 GCC/CMake/Ninja build, eight jobs, existing `-O2 -g` with
assertions: [build.log](../../build/digger-facing-001/build.log).
Expanded the existing **legacy** `actoranimstate_units` with a seconds-long
selection regression; no new route, recording, frozen build or tier edit.
[Final focused run](../../build/digger-facing-001/focused.log) and
[JUnit](../../build/digger-facing-001/focused.xml): **1/1 passed, 0.96 s**.
It checks 96 draw/pick cases (eight yaw angles, three tribes, both walking
facings and gravities), temporal entries 9/19, visible pixels, and adjacent
Digger/Walker/Blocker adapter samples. Initial new-test failures remain in
`focused-initial.log/xml`: the chosen Y was offscreen, and Just Dig has no Builder
stock. Corrected the new geometry sample to Y=32 and used independent state
samples, rather than inventing an assignment or gameplay route. Existing assertions
and timeouts were preserved. LF and task-scoped `git diff --check` pass.

All **19 promoted recordings** ran sequentially using rebuilt
`build/agent-native/main/webolution.exe --game-dir "Lemmings Revolution" --replay <file>`.
Exact filenames/output: [replays.log](../../build/digger-facing-001/replays.log).
Each exited 0, **PASS, won=1, completed=1**:

| Level | Final step | Saved/needed |
| --- | ---: | ---: |
| MM_BASH_002 | 1844 | 10/10 |
| MM_BASH_MINE_WLR1 | 2272 | 20/10 |
| MM_BLOCK_001_A | 1619 | 24/15 |
| MM_BLOCK_BOMB_A | 1546 | 12/10 |
| MM_BOMB | 954 | 17/10 |
| MM_BUILD | 1685 | 20/10 |
| MM_CLIMB_002 | 1252 | 10/1 |
| MM_DIG_000 | 1014 | 19/10 |
| MM_FLOAT | 978 | 10/1 |
| MM_MINE | 1137 | 20/10 |
| MM_W_012 | 1742 | 16/10 |
| MM_E1_3_SKILLS | 2095 | 10/5 |
| M06_KF_1 | 1544 | 91/50 |
| MM_E1_4SKILLS | 2506 | 20/10 |
| MM_E2_4SKILLS | 2142 | 14/10 |
| MM_BASH_DIG_001_A | 3241 | 20/10 |
| MM_BUILD_BLOCK_A | 2482 | 15/10 |
| E19_KF_2 | 6034 | 24/20 |
| E20_MG_1 | 12810 | 7/5 |

No divergence or retirement. Replay build-identity diagnostics on stderr are
informational. These asset/unit and headless replay paths construct no SaveStore;
no installed or normal user save was read or written. Mandatory smoke remains
**pending supervisor execution after worker exit**; no verifier profile was run.

Handoff: dig draws unmirrored front view 4; blocker gains its evidenced phase; focused units and all 19 promoted wins pass.
Next: supervisor smoke and independent review, then human playtest acceptance; status needs_review.

**What to check:** Just Dig (MM_DIG_000): assign Digger and rotate the camera
while it digs; the face should stay toward the camera, including after zooming.
When it stops digging and resumes walking, its walking direction should be
restored. On MM_BUILD_BLOCK_A, assign Blocker and rotate:
its pose should turn across the eight installed views with the quarter-turn offset.
