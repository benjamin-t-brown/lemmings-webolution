# Fast-forward hold

**needs_review** for playable v1. On the installed route through the map to node
22, One Way Ticket (`MM_ALL_8_001`), the Playtest 13 C3 report asks for speed-up
only while the button is held. The previous SDL input path toggled on press or
drag onto plate 8 and left that speed selected after release.
[Playtest 13](../playtest/2026-11-17-playtest-13.md)

## Production path

`runApplicationLeg` processes SDL button and motion events in logical game
coordinates. An accepted left press arms the hold. The press and subsequent
motion use `runtimeSkillPanelCommand` to set `RuntimeSimState::setFastForward`
when the held pointer is over plate 8. Leaving the plate or game window clears
speed. Dragging back onto the plate during the same hold restores speed.
Left release, focus loss, the level menu, and a right-button camera grab cancel
the gesture. Focus or menu return requires a fresh press. The existing plate
outline follows `fastForward()` through `drawRuntimeSkillPanel`. The existing
click cue fires on entry into the active state.
[SDL input and rendering](../../src/app/main.cpp)

`RuntimeSimState::advanceElapsed` still uses one simulation pass at ordinary
speed and seven with fast-forward active. P still pauses all simulation passes.
Releasing the button during pause returns the first resumed update to ordinary
speed. Completion clears the hold on exit to Results, after recording the last
simulation budget. Every input-driven speed edge goes through the existing
`ReplayRecorder::commands` hook, including the final clear at level exit.
The same version-1 `f` rows carry value `1` for hold and `0` for release.
[Simulation gate](../../src/app/RuntimeHud.cpp), [replay code](../../src/app/Replay.hpp)

**Provisional clone input policy.** Accepted static evidence for the registered
`Lemmings Revolution/Lemmings Revolution.exe` (SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`) shows
`0x455AFF` testing held button 0 over plate 8 and storing 1 to input+0x20 at
`0x455B30`. That branch has no release clear. The reset at `0x455774` clears the
field at `0x455AF4`. The cancel gesture in the original remains unresolved.
The new release and pointer-exit behavior follows the user's C3 request.
[Accepted input evidence](fastforward-toggle-001.md#static-evidence-and-remaining-original-question)

## Checks

The registered UCRT64 GCC 16.1 build used `-O2 -g` with assertions. The focused
`fastforward_units` smoke test passed in 0.04 s. Its former second- and
third-toggle actions now use release and a new hold, with the same assertions
for inactive/active state and cumulative ticks (56, 58, 72). The existing
hatch matched routes and dry-receiver gadget replay also passed with their
assertions intact. These checks used private build output save paths and a
private LocalAppData path.
[Build](../../build/fastforward-hold-001/run-20261118-000615-243/build.log),
[unit](../../build/fastforward-hold-001/run-20261118-000615-243/fastforward-units.log),
[hatch routes](../../build/fastforward-hold-001/run-20261118-000615-243/hatch-routes.log),
[gadget replay](../../build/fastforward-hold-001/run-20261118-000615-243/gadget-replay.log)

- Removed legacy `fastforwardapp_after` because it expected drag motion alone to activate speed without a gameplay press.
- Removed legacy `fastforward_toggle_production` because it expected release to keep speed on and a second click to turn it off.
- Removed legacy `hatch_timing_fast` because its two down/up clicks depended on a speed latch between them.

All 20 promoted recordings ran through the rebuilt
`build/agent-native/main/webolution.exe --game-dir "Lemmings Revolution" --replay <file>`.
Each passed with exit 0, `won=1` and `completed=1`. One Way Ticket contains both
on and off transitions. No recording needs retirement.
[Exact filenames and results](../../build/fastforward-hold-001/run-20261118-000615-243/promoted-replays.log)

| Level | Result | Final step | Saved/needed |
| --- | --- | ---: | ---: |
| MM_BASH_002 | PASS | 1844 | 10/10 |
| MM_BASH_MINE_WLR1 | PASS | 2272 | 20/10 |
| MM_BLOCK_001_A | PASS | 1619 | 24/15 |
| MM_BLOCK_BOMB_A | PASS | 1546 | 12/10 |
| MM_BOMB | PASS | 954 | 17/10 |
| MM_BUILD | PASS | 1685 | 20/10 |
| MM_CLIMB_002 | PASS | 1252 | 10/1 |
| MM_DIG_000 | PASS | 1014 | 19/10 |
| MM_FLOAT | PASS | 978 | 10/1 |
| MM_MINE | PASS | 1137 | 20/10 |
| MM_W_012 | PASS | 1742 | 16/10 |
| MM_E1_3_SKILLS | PASS | 2095 | 10/5 |
| M06_KF_1 | PASS | 1544 | 91/50 |
| MM_E1_4SKILLS | PASS | 2506 | 20/10 |
| MM_E2_4SKILLS | PASS | 2142 | 14/10 |
| MM_BASH_DIG_001_A | PASS | 3241 | 20/10 |
| MM_BUILD_BLOCK_A | PASS | 2482 | 15/10 |
| E19_KF_2 | PASS | 6034 | 24/20 |
| E20_MG_1 | PASS | 12810 | 7/5 |
| MM_ALL_8_001 | PASS | 13812 | 46/40 |

Desktop interaction and a fresh recording of the hold still need the human
playtest. The promoted replay results cover decoding existing state edges.
Mandatory smoke verification is pending supervisor execution.

Handoff: fast-forward hold implemented, focused unit and two legacy checks passed, and all 20 promoted wins reproduced.  
Next: supervisor smoke, independent review, then human confirmation on One Way Ticket.

**What to check:** On One Way Ticket, hold plate 8 and watch the clock and plate
outline. Release, move off the plate while held, switch focus, or open the
level menu with Esc while running. Expect normal speed and the speed outline
to disappear.
A tap should leave ordinary speed, and dragging back onto the plate with the
same gameplay press should restore speed until release. While holding, press
P to freeze the clock. Release during pause and press P again to resume at
normal speed. Hold through Results, then start the level again and expect
normal speed until a new press.
