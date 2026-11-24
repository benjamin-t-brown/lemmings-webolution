# Level menu row order

Playtest 30 W7 requested a longer Examine label and Quit at the bottom. In **Just Climb**
(`MM_CLIMB_002`, node 0), the pre-change clone drew Resume, Try Again, Quit, Options, Examine.
It now draws **Resume, Try Again, Options, Examine Level, Quit**. The task calls the third
action Sound Options, and the before probe read its installed UK string 3 as **Options**.
The implementation preserves that displayed native label. Resume is focused when the menu opens.
[Playtest ledger](../playtest/2026-11-23-playtest-30.md#defects) · [Before frame](../../build/level-menu-order-001/before/just-climb.png) · [After frame](../../build/level-menu-order-001/after/just-climb.png) · [Installed strings](../../build/level-menu-order-001/before/probe.log)

`LevelMenu` now pairs each action with its label source in one ordered table. `load` resolves
the labels for drawing and strict content hits. `layoutTitle` uses the same row count for its
eight-line budget. In `runApplicationLeg`, mouse clicks and translated touch taps reach
`actionAt`, arrow keys update focus, and Return reads `focusedAction`. Those paths still call
the existing action dispatcher. The production diff is confined to `LevelMenu`.
[Row table](../../src/app/LevelMenu.h#L47) · [Layout and selection](../../src/app/LevelMenu.cpp#L57) · [Action dispatcher](../../src/app/main.cpp#L1407)

Static REA opened the registered x86 executable with SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`. Ghidra 12.1.4
pseudocode at `0x0048F51B` shows a five-entry call. Instructions at `0x0048F530–0x0048F541`
push the count and table pointer, and the xref at `0x0048F532` points to `0x004C4228`.
The table bytes decode to `44, 48, 43, 3, 0`, corroborating the retained native-order evidence.
The reordered list and Examine Level wording follow W7. REA closed without a snapshot.
[Static tool results](../../build/level-menu-order-001/rea.json) · [Retained menu evidence](in-level-menu.md)

- The source-built SDL probe passed all 15 combinations of action and input method in Just
  Climb. Injected touch also opened the menu through the Menu plate in `E17_KF_1`, whose full
  installed title fits above the rows. The probe used private saves. Physical touch and human
  confirmation remain pending. [Input log](../../build/level-menu-order-001/after/probe.log) · [Longest title before](../../build/level-menu-order-001/before/longest-title.png) · [Longest title after](../../build/level-menu-order-001/after/longest-title.png)
- The `levelmenu` expectations now follow the new hit rectangles, the Examine Level text and
  focus wrapping to Quit. The level-menu window and camera-entry tests aim their Quit clicks
  at the bottom row. The window test's non-activating Options click now targets the strict
  content edge. The UCRT64 `smoke_tests` build and all 95 main smoke tests passed, including
  `levelmenu` and the 102-level sweep. The window and camera-entry routes were outside this run.
  [Build](../../build/level-menu-order-001/build-smoke-tests.log) · [Main smoke](../../build/level-menu-order-001/main-smoke.log)
- `--replay` dispatches before the frontend. All 32 promoted recordings reproduced, including
  31 wins and the recorded map exit for The Lone Ranger at step 12893. The per-recording ledger
  gives the outcome for every file. The probe executable and object were removed.
  [Replay ledger](../../build/level-menu-order-001/promoted-replays.tsv) · [Replay CLI log](../../build/level-menu-order-001/promoted-replays.log)

**Handoff** `needs_review`. The supervisor's smoke and recovery gate is pending.  
**Next action** Independent review after the gate, then the human playtest.

**What to check** Open the menu in Just Climb with Escape or the touch Menu plate. Expect
Just Climb above the rows and Resume highlighted. Press Up to highlight the bottom Quit
row and Return to go back to the map. Reopen the level and tap Examine Level to see the
paused “Drag to look around” caption.
