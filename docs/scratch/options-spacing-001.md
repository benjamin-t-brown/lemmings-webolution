# Options row spacing

Playtest 30 W2 reported excessive space above **Window size** and **Accept Changes And Exit**. The clone applied a 58 px pitch to every row. The fix covers Options from the main menu and from the level menu. The in-level frame was taken on Just Climb (`MM_CLIMB_002`, node 0). [Playtest 30 W2](../playtest/2026-11-23-playtest-30.md)

`FrontendWindow` and `runApplicationLeg` share `FrontendSession` for Options. Its `soundBaseY` keeps the Music anchors at 204 and 100. `SoundMenu::render` and `SoundMenu::hit` use the same row layout for plates, arrows, meters, clicks and hover. Keyboard focus selects the same row indices. Music and Sfx each reserve a meter line. The provisional layout for the clone rows uses the in-level menu’s 29 px pitch after Unlock all levels and Window size. Labels, actions and values use the existing settings and sidecar paths. [SoundMenu.cpp](../../src/app/SoundMenu.cpp) · [FrontendSession.cpp](../../src/app/FrontendSession.cpp)

Ghidra 12.1.4 pseudocode for builder `0x48E5B4` shows three labels and two meters. Instructions at `0x48E62F`, `0x48E678` and `0x48E6B4` push label row indices 1, 3 and 5. Instructions at `0x48E767` and `0x48E7E4` push meter row indices 2 and 4, with 8 px added to the calculated Y. The incoming reference is at `0x4B24AC`. These are static reads of the registered x86 EXE, SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`. REA closed without a snapshot. [Instructions](../../build/options-spacing-001/rea-instructions.json) · [Pseudocode](../../build/options-spacing-001/rea-decompile.json) · [Xrefs](../../build/options-spacing-001/rea-xrefs.json)

| Clone frame | Before, text tops in px | After, text tops in px |
| --- | --- | --- |
| Main menu | [204 / 262 / 320 / 378 / 436](../../build/options-spacing-001/main-before.png) | [204 / 262 / 320 / 349 / 378](../../build/options-spacing-001/main-after.png) |
| Just Climb | [100 / 158 / 216 / 274 / 332](../../build/options-spacing-001/level-before.png) | [100 / 158 / 216 / 245 / 274](../../build/options-spacing-001/level-after.png) |

The Windows clone probe checked both meters, 2 px between compact plates, and bounds within 640×480. It checked keyboard focus, arrow and label hover and clicks, Accept and private sidecar persistence, and a paused simulation tick throughout in-level Options. Existing test expectations cover Options entry. The probe supplies the row-spacing check. Browser behavior remains unverified. [Before probe](../../build/options-spacing-001/probe-before.log) · [After probe](../../build/options-spacing-001/probe-after.log)

The `smoke_tests` target built and all eight selected smoke tests passed. All 32 active recordings reproduced. This includes 31 completed wins and the Playtest 29 Lone Ranger attempt that ends at the map at step 12893 with 0/90 saved. The table lists each recording and its exact log. [Smoke build](../../build/options-spacing-001/smoke-build.log) · [Focused smoke](../../build/options-spacing-001/focused-smoke.log) · [Replay results](../../build/options-spacing-001/replay-results.tsv)

- **Handoff** `needs_review`. The supervisor’s full `verify.py --profile smoke` gate is pending.
- **Next action** Review the diff and gate result, then confirm spacing in human playtest.

**What to check** Open Options from the main menu, then pause Just Climb and open Options again. Expect Window size and Accept to sit one row pitch below their preceding rows, with both volume meters visible. Hover Window size, change it with an arrow and click Accept. Expect the matching highlight, the chosen setting to persist and the calling menu to reopen.
