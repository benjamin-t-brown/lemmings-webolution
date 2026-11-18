# In-level menu pressed cursor

Playtest 15 E2 reported that the in-level Escape menu kept the full-size idle cursor while a mouse button was down. Both the level menu and its sound options page now show the same pressed artwork as the main menu until the last held button is released. The report identifies the screen across levels. Wandering Free (`MM_ALL_8_002`, node 24) is an installed route for checking it. [Human playtest](../playtest/2026-11-18-playtest-15.md)

## Original software cursor

Ghidra 12.1.4 inspected the registered x86 `Lemmings Revolution/Lemmings Revolution.exe`, SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`. At `0x45D40A..0x45D41E`, Escape sets UI2, enables frontend flag `0x4C369D` through `0x455EF0(1)`, and selects page `0x1F`. Its builder loads the label table at `0x4C4228`. The xref to `0x4C4214` belongs to the main menu. Options pushes return page `0x1F` and sound page 9 at `0x48F615..0x48F619` while the frontend flag stays set. [Ghidra instructions and xrefs](../../build/level-menu-cursor-001/static-evidence.json)

The decompilation of `0x4532E2` agrees with the instructions. The flag selects the frontend path at `0x453BF6..0x453C02`. The two held-field comparisons at `0x455DBC/0x455DFC` feed a short-circuit OR. Either held writes state 10 at `0x453D68`, both up writes state 1 at `0x453D77`, and `0x453D94` skips gameplay picking. The software binder at `0x45620B/0x4562E7` selects the two sprite slots. Retained resource reads identify state 10 as `_curMseDwn` and state 1 as `_curNoLem`. This establishes artwork selection statically. Original display and physical input were unobserved. Ghidra closed without a snapshot. [Instruction and pseudocode excerpts](../../build/level-menu-cursor-001/static-evidence.json) · [Retained texture mapping](frontend-cursor-return-coverage.md)

## Clone path and checks

`runApplicationLeg` previously hard-coded the focused menu cursor to `NoLemming`. It now shares `updateFrontendCursorInput` with the frontend window. Button and focus changes reach the session before page actions can consume an event. Both pages draw through `FrontendSession::cursorState → frontendCursorState → RuntimeCursor::draw`. Losing focus clears both holds. Closing the menu switches drawing back to the existing `runtimeCursorState` call with its gameplay grab and hit-test inputs. [Level input and draw](../../src/app/main.cpp) · [Shared SDL handler](../../src/app/FrontendWindow.cpp) · [Declaration](../../src/app/FrontendWindow.h)

The UCRT64 Release application built. The existing dummy-SDL frontend window test passed its checks for individual and overlapping holds, page changes and focus loss. The in-level draw path was source-traced. Its visual acceptance awaits human play. All runs used task-private LocalAppData. [Release build](../../build/level-menu-cursor-001/build-release.log) · [Focused build](../../build/level-menu-cursor-001/build-frontendwindow.log) · [Focused test](../../build/level-menu-cursor-001/frontendwindow.log)

All 23 promoted recordings returned exit 0, `PASS`, `won=1` and `completed=1`. Each value below is saved/required. The log index gives each exact recording path, final step and invocation log. [Individual replay results](../../build/level-menu-cursor-001/replays.tsv)

| Recording | Saved | Recording | Saved | Recording | Saved |
| --- | ---: | --- | ---: | --- | ---: |
| MM_BASH_002 | 10/10 | MM_FLOAT | 10/1 | MM_BUILD_BLOCK_A | 15/10 |
| MM_BASH_MINE_WLR1 | 20/10 | MM_MINE | 20/10 | E19_KF_2 | 24/20 |
| MM_BLOCK_001_A | 24/15 | MM_W_012 | 16/10 | E20_MG_1 | 7/5 |
| MM_BLOCK_BOMB_A | 12/10 | MM_E1_3_SKILLS | 10/5 | MM_ALL_8_001 | 46/40 |
| MM_BOMB | 17/10 | M06_KF_1 | 91/50 | MM_E1_5_SKILLS | 16/16 |
| MM_BUILD | 20/10 | MM_E1_4SKILLS | 20/10 | MM_ALL_8_002 | 19/10 |
| MM_CLIMB_002 | 10/1 | MM_E2_4SKILLS | 14/10 | MM_ALL_8_003 | 21/20 |
| MM_DIG_000 | 19/10 | MM_BASH_DIG_001_A | 20/10 | | |

**Handoff** — `needs_review`. Supervisor smoke and recovery are pending.  
**Next action** — Independent review after the gate, then human playtest.

**What to check** — On Wandering Free, press Escape and hold each button over blank space. Hold both and release them one at a time. The smaller cursor should persist until both are up. Repeat on Options, return to the level menu, then Resume. Hover a lemming or hatch and right-drag over open terrain to check the gameplay cursors.
