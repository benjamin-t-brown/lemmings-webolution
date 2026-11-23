# Touch Menu and Examine

The November 23 mobile request V4 identified a keyboard dependency. In **Just Climb**
(`MM_CLIMB_002`, node 0), the connected level only reached `openMenu` through Escape during
play or O during pause. Finger taps used the scene and HUD paths, and the level menu had
no Examine action. This failure was source-traced from the request ledger. [Mobile V4](../playtest/2026-11-23-mobile-request.md)

Review found a second failure after entering **Just Climb** with mouse and keyboard.
The first finger-down at logical `(320,20)` set `receivedTouch()` before the hit test.
A tap could open an unseen menu, and a swipe from that point could be consumed by it.
[Source at revision entry](../../build/touch-buttons-001/revision-entry-src-app-main.cpp#L1707)

`SdlWindow::touchCoordinates` records valid SDL finger input for the shared window across
frontend screens, levels and retries. `runApplicationLeg` draws the provisional **92×27**
logical-pixel Menu plate at `[274,366) × [9,36)` during the level, including either pause.
`LevelMenu` uses the existing red plate, black `TEXTURES\^FONT.BMP` glyphs and font cells.
The plate stays clear of the skill, fast-forward and Nuke column at x18–89, the top-right
counters and the bottom-right timer. The 1×, 2×, 3×, Auto and browser Fill layouts scale the
same 640×480 composition. [Plate](../../src/app/LevelMenu.cpp) · [HUD placement](../../src/app/main.cpp#L1145) · [Counters and timer](../../src/shell/HudText.h) · [Skill controls](../../src/app/RuntimeSkillPanel.cpp#L123)

`runApplicationLeg` records whether the plate was drawn and sets the hit-test state after
`SDL_RenderPresent`. The first touch can reveal the plate and go through ordinary gameplay
routing. A contact that starts on the presented plate stays with `menuTouch` through lift.
The existing tap limits are 300 ms and 8 logical pixels. Accepted taps call the same
`openMenu` as the keys, with the UI click, hold and drag cancellation and clock reset.
The level menu owns mouse, keyboard and touch activation of **Examine**, a clone label after
the four UK labels. Activation closes into `toggleExaminePause` semantics and preserves
examine if it was already active. Resume returns to play. The recorder still emits its
existing `p` command for changes in `paused()`. P and Backspace retain their keyboard
bindings, and the mobile warning is unchanged. [Menu entry and activation](../../src/app/main.cpp#L1387) · [Finger routing](../../src/app/main.cpp#L1706) · [Present](../../src/app/main.cpp#L2124) · [Pause semantics](examine-pause-001.md) · [Recorder](../../src/app/Replay.hpp#L107)

Ghidra 12.1.4 opened the registered x86 EXE with SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
The xref from `0x48F532` points to the table at `0x4C4228`. Raw bytes give
`{44,48,43,3,0}`. Instructions at `0x48F530–0x48F541` and `0x48F632–0x48F649`
corroborate the decompilation's row count and empty widget dispatch to selector `0x19`.
That widget stays unsupported. The revision confirmed the xref and dispatch and closed
REA without saving a snapshot. [Retained static evidence](../../build/touch-buttons-001/rea.json) · [Revision static evidence](../../build/touch-buttons-001/revision-rea.json) · [Raw anchors](../../build/touch-buttons-001/native-anchors.log)

## Checks

- In the source-built **Just Climb** probe, mouse and keyboard entry left the plate hidden.
  At 1× the first SDL tap at `(320,20)` revealed the plate while the clock advanced from
  1,500 to 2,000 ms. The second tap opened the menu, requested one UI cue and held the
  World at 2,000 ms. In a fresh 2× session the first swipe from `(320,20)` turned yaw from
  32 to 48.67 as the plate appeared, then reached 65.33 after lift. [Probe source](../../build/touch-buttons-001/sdl_probe.cpp) · [Probe build](../../build/touch-buttons-001/revision-sdl-probe-build.log) · [Probe log](../../build/touch-buttons-001/revision-sdl-probe.log) · [First tap](../../build/touch-buttons-001/frames/desktop-first-tap-reveals-menu-scale1.png) · [Second tap](../../build/touch-buttons-001/frames/desktop-second-tap-opens-menu-scale1.png) · [Before swipe](../../build/touch-buttons-001/frames/desktop-before-first-swipe-scale2.png) · [After swipe](../../build/touch-buttons-001/frames/desktop-after-first-swipe-scale2.png)
- Touch runs at 1×, 2× and 3× showed the plate in play and both pauses. The plate was
  hidden on both menu pages and on Results, and returned on Retry. A tap on the shown
  plate kept yaw, zoom, pointer, actors, inventory and release rate fixed. The plate owned
  swipes and long holds that began on it. A Menu tap cancelled a scene drag and a
  fast-forward hold. [Playing](../../build/touch-buttons-001/frames/touch-playing-scale2.png) · [Menu](../../build/touch-buttons-001/frames/touch-level-menu-scale2.png) · [Results](../../build/touch-buttons-001/frames/touch-results-scale3.png)
- Mouse hover focused Examine, keyboard Up wrapped from Resume to Examine and touch
  selected it from the menu. Examine from play and either pause froze the World. In the
  2× touch run, opening the menu and entering Examine kept tick 52, update 62 and 10,500 ms
  fixed. A scene swipe in Examine turned yaw from 32 to 48.67 while those values held.
  Both touch Resume steps restarted the clock at the ordinary rate. [Examine](../../build/touch-buttons-001/frames/touch-examine-scale2.png)
- Registered GCC built `smoke_tests` for the revision. Six smoke tests covering menu,
  level input, cursor, entry, return and window close passed. The previous supervisor run
  passed 95 main smoke tests, including the 102-level sweep, and 84 recovery tests. The
  supervisor's full smoke gate for this revision is pending. [Revision build](../../build/touch-buttons-001/revision-native-build.log) · [Focused smoke](../../build/touch-buttons-001/revision-focused-smoke.log) · [Previous supervisor run](../../build/agent-runs/20261123T091920-38732-5cbf8721/verification.log)
- All 30 promoted recordings reproduced with the clone CLI. This includes 29 completed
  wins and The Lone Ranger's recorded map exit at step 12893. The checks used private
  LocalAppData and scratch paths. Per-recording commands and results are retained, and
  the scratch probe executable and object were removed. Phone and browser input delivery
  and sizing still await a playtest. [Replay log](../../build/touch-buttons-001/revision-promoted-replays.log) · [Per-recording results](../../build/touch-buttons-001/revision-promoted-replays.tsv)

The revision changed `src/app/main.cpp`, the retained SDL probe and its runner, and this
report. [Revision diff](../../build/touch-buttons-001/revision-main.diff) · [Earlier entry sources](../../build/touch-buttons-001/entry/)

**Handoff** `touch-buttons-001` is **needs_review**.  
**Next** Supervisor smoke, independent review and the phone playtest.

**What to check** On a phone in landscape, open **Just Climb** and tap the red top-centre
Menu plate. Choose Examine and drag the view. Expect the timer and lemmings to stay frozen.
Tap Menu again, then Resume, and expect the timer and lemmings to advance. On a fresh
mouse-and-keyboard session with touch available, tap top centre once. Expect the plate to
appear while play continues. Tap it again to open the menu. In another fresh session,
start with a swipe from that point and expect the camera to turn as the plate appears.
