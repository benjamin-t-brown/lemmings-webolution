# digger-outward-001

**needs_review; supervisor smoke pending.** Playable v1, playtest-ledger work order.
Before: [Playtest 12 B3 and the user's clarification](../playtest/2026-11-17-playtest-12.md),
**Just Dig / MM_DIG_000**. The accepted fixed-front implementation misread the
request: rotating the cylinder must reveal the outward-facing digger's sides.
This presentation correction is the next dependency. Difficulty routine;
recommended Sonnet; assigned Codex override used.

Changed: `src/app/TubeScene.cpp`, `src/__test__/test_actoranimstate.cpp`, this report.
Backups: [backup/](../../build/digger-outward-001/backup/). Incoming modifications
to the installation hook log, queue and generated RUN_STATUS were preserved.
Supervisor PID **15108** owns RepositoryLease; no protected controls were edited.
CIM process metadata was denied; Get-Process showed no clone/build/test runner
at entry. No original execution, hooks, debugger, capture, installation, subagents,
background work, commits or pushes occurred.

Production path: `runApplicationLeg -> LemmingAnimTracker::advance/poseOf ->
injectedWalkersFromWorld -> TubeScene::drawInjectedWalkers -> prepareInjectedWalker
-> spriteViewScores -> selectSpriteView -> actorSpritePlacement -> drawActorSprite`.
The adapter now supplies digger phase **0.75**. Removed the forced `{4,false}`
selection: the common camera-dependent chooser selects the installed views and
mirrors them. Screen-box preparation uses the same selection as drawing.

The digger's rendering reference is independent of walking facing: its normalized
cylinder tangent is `(-position.z, 0, position.x) / hypot(position.x,position.z)`.
**Provisional clone alignment:** add **3/64 turn (16.875 degrees)** when scoring
that reference. With the shared chooser, the front-view sector spans 33.75 degrees
and unadjusted phase 0.75 places the radial normal at its edge. Half a sector
centres the front image. This also avoids changes from the faceted walking tangent
and opposite walking directions. The phase and chooser are traced; this smooth
reference and sector alignment are player-facing choices, not native fidelity.
[Numerical inspection](../../build/digger-outward-001/orientation.csv) records why
applying 0.75 directly to the walking tangent is insufficient;
[aligned scores](../../build/digger-outward-001/outward-orientation.csv) retain both
walking facings over camera offsets -64..64 in steps of 8.

Simulation facing, position, skills, timing and saves are unchanged. Fresh samples
restore phase zero and the walking tangent after digging. Blocker's **0.25**
phase and eight-view chooser path are unchanged. Sprite views remain discrete
installed images; no interpolation or claim of continuous image morphing is made.

Static authority: registered `Lemmings Revolution/Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Retained accepted [digger-facing evidence](digger-facing-001.md) establishes
`4159CA..4159DA -> 4683E9` storing binary64 **0.75** at controller **+1D8/+1DC**;
leave `415995..4159A9` resets zero. Actor render `42EF38/42EF9F/42EFD8` constructs
the facing tangent and calls the view chooser. The [asset census correction](asset-census-001.md)
identifies the twelve Hi side-view rows as native-used.

Used [reverse-engineer-anything](C:/Users/bbrown/.agents/skills/reverse-engineer-anything/SKILL.md)
with static REA/Ghidra **12.1.4**, profile
`3c06738e45e144d5f073edbd3244c6bff46222f389525fe659bc92dca2ccdece`.
Fresh [complete returned evidence](../../build/digger-outward-001/static-evidence.json):
chooser pseudocode `ev_2e5b64e52c596fc325c0768fe6be63e3e28dda358eb713123846aad1ed54b72c`,
instructions `ev_a78b0efa80a5078338eda08e4203e6d7203aa8eb9a3c8fb5612dbb345f33e0d6`.
Cross-check: `468456..468462` adds the first offset to +1D8;
`46848E..4684D9` rotates +1E0/+1F0 while preserving +1E8;
`4684E4..4684ED` adds the second offset; `46857F/46858B/4685D4` normalizes
the camera vector and obtains both scores; `46861E/46862A` selects the five-view
branch and clears mirroring; `4686DF` writes front index 4. The chooser contains
no compensation for opposite walking facings. Analysis closed without snapshots.
Static output is not an original-game observation.

Checks: registered UCRT64 GCC/CMake/Ninja, eight jobs, existing `-O2 -g` with
assertions; [build.log](../../build/digger-outward-001/build.log).
Removed obsolete legacy `frontViewUnits` and its call because it required the
demonstrated fixed-front defect; other actoranimstate assertions, registrations,
smoke tests and timeouts were untouched. No new permanent test or scripted route.
[Focused log](../../build/digger-outward-001/focused.log) / [JUnit](../../build/digger-outward-001/focused.xml):
**spriteview and actoranimstate_units passed, 2/2, 0.07 s**.

The temporary [draw inspector](../../build/digger-outward-001/inspect-draw.cpp)
uses installed Just Dig geometry and production drawing/picking, without a window
or SaveStore. [Output](../../build/digger-outward-001/draw.log): **60/60 cases**
match visible pixels and pick rectangles for front **4**, three-quarter **3**,
side **2**, and the opposite-side mirrors. Covers three tribes, both walking
facings, both gravities and yaw offsets **-48,-24,0,24,48**. It also checks walking
phase/facing restoration and the retained blocker phase. This is a synthetic
presentation inspection, not natural play or original comparison.

All **19 promoted recordings** ran sequentially through rebuilt
`build/agent-native/main/webolution.exe --game-dir "Lemmings Revolution" --replay <file>`.
Every result is **PASS, exit 0, won=1, completed=1**; exact filenames and output:
[replays.log](../../build/digger-outward-001/replays.log).

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

No divergence or retirement. Build-identity stderr messages are informational.
These headless replay/unit/inspection paths open no SaveStore, so no installed
or normal user save was used. No verifier profile was run; mandatory smoke is
**pending supervisor execution after worker exit**. Task-file LF and diff checks
pass; pre-existing queue CRLF was left untouched.

Unknowns: human confirmation and exact native all-angle presentation remain open.
No timing, full-frame, depth or full-project fidelity gate is resolved here.

Handoff: outward digger presentation implemented; 60 draw/pick cases, two focused checks and all 19 promoted wins pass.
Next: supervisor smoke and independent review; leave needs_review, then human playtest confirmation.

**What to check:** Just Dig (**MM_DIG_000**): assign Digger, centre the camera,
then rotate slowly to place it at either side. Expect the front image at centre,
three-quarter and side images toward the edges, with no facing reversal when the
prior walking direction differs. When digging ends, walking uses its preserved
direction. On **MM_BUILD_BLOCK_A**, rotate around a Blocker: its eight-view turning
should remain unchanged.
