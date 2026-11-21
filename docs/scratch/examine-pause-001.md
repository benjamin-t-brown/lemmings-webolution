# Examine pause

[Playtest 25 Q12](../playtest/2026-11-21-playtest-25.md#requests) requested a clone
addition for looking around a frozen level. Standard pause stopped the World,
but the left button still reached skill assignment, hatch rate controls and the
skill panel. In **Just Climb** (`MM_CLIMB_002`, node 0), a valid Climber click
could spend a skill and resume play. **Designed with love** (`M11_KF_1`, node 33)
provides the laser switch and gate used for the hover check.

## Production path

`runApplicationLeg` routes Return, keypad Enter and Pause/Break from the level
event loop to `RuntimeSimState::toggleExaminePause`. Both pause modes expose the
same `paused()` value to the World, level clock, countdown, scenery and replay
recorder. Enter or Pause/Break switches standard pause to examine without
advancing the World. The recorder still uses its existing `p` command for
transitions of `paused()`. P resumes from examine and still toggles standard pause
from play. Escape resumes from either pause, O opens the level menu, and O-close
returns to the prior pause. Focus loss cancels the grab and holds.

Examine consumes gameplay button events before assignment, hatch rate, panel and
Nuke handling, and keeps skill selection fixed. Either mouse button feeds the
same relative input to the 144 Hz camera update and restores the logical pointer
on release. Scroll up and down select zoom in and out during examine through the
existing camera zoom path. `materialize` still calls `BackdropScene::drawSwitchHover`
and its occupancy-sensor callback whenever the pointer can hover. Both drag
buttons suspend that feedback while held. The new HUD caption reads “Paused” and
“Drag to look around” in the centre of the logical framebuffer. The standard
bottom caption and the clone's successful-assignment resume path remain intact.
[Input](../../src/app/main.cpp) · [Pause state](../../src/app/RuntimeHud.cpp) ·
[Caption](../../src/shell/PauseCaption.cpp)

## Static reference

Ghidra 12.1.4 read the registered x86 EXE, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Instructions at `0x45D264` and `0x45D2A8..0x45D35A` support the localized
Paused caption and the retained `[277,422,363,454)` panel. The pseudocode and
instructions for `0x455657` agree that `0x455756..0x455769` clears UI state 1.
Branches at `0x455681` and `0x4556A2` also reach that check, so the accepted
clone's success-only resume is a separate behavior. REA closed without a
snapshot. [Static excerpts](../../build/examine-pause-001/rea-static-excerpts.json)

## Evidence and checks

- Dummy SDL toggled examine with each key, checked menu and focus routing, and
  turned **Just Climb** from yaw 32 to 42 with 24 logical pixels of left drag at
  1× and 2×. The World stayed at tick 20, one elapsed update, and 4,020 ms. Actor
  0 stayed at `(39,63)`. An assignable click spent no Climber during examine. The
  standard-pause click later spent one and resumed play. Wheel, Space, Backspace
  and right-drag also passed. [Probe build](../../build/examine-pause-001/sdl-probe-build.log) ·
  [Probe run](../../build/examine-pause-001/sdl-probe-run.log) ·
  [Centred caption](../../build/examine-pause-001/frames/just-climb-examine-scale1.png)
- In **Designed with love** at yaw 245, the framebuffer audit matched 192 visible
  switch pixels and verified all 24 visible gate ID 5 pixels brighten. The gate
  returned to its base colour while dragging, and both highlights returned at
  the restored pointer. [Hover](../../build/examine-pause-001/frames/switch-examine-hover.png) ·
  [Held](../../build/examine-pause-001/frames/switch-examine-held.png) ·
  [Restored](../../build/examine-pause-001/frames/switch-examine-restored.png) ·
  [Pixel audit](../../build/examine-pause-001/frame-audit.log)
- `smoke_tests` built with the registered GCC and all 95 main smoke-labelled
  tests passed, including the 102-level sweep. The new windowless legacy test
  passed in 0.03 s. [Native build](../../build/examine-pause-001/final-native-build.log) ·
  [Main smoke](../../build/examine-pause-001/main-smoke.log) ·
  [Legacy check](../../build/examine-pause-001/examine-pause-legacy.log)
- All 29 promoted recordings returned `PASS won=1 completed=1` with matching
  native and Wasm final steps and save counts. **Designed with love** finished
  at step 40836 with 97/85 saved. The tables name each recording and its CLI log.
  Native runs used task-private LocalAppData. [Native](../../build/examine-pause-001/native-replays.tsv) ·
  [Wasm](../../build/examine-pause-001/wasm-replays.tsv)
- Emscripten linked the game using the cached SDL2 port. The built page cancels
  defaults for all three DOM codes and leaves propagation available to SDL's
  window listener. The cached SDL2 source maps them to the three scancodes.
  Chrome's GPU subprocess exited with access denied before the headless browser
  probe ran. Physical browser keys and pointer lock still need a playtest.
  [Wasm build](../../build/examine-pause-001/wasm-build.log) ·
  [Page check](../../build/examine-pause-001/browser-key-handler.log) ·
  [SDL source](../../build/examine-pause-001/browser-sdl-excerpts.txt) ·
  [Browser run](../../build/examine-pause-001/browser-key-run.log)

Changed `src/app/{main.cpp,CameraInput.h,RuntimeHud.h,RuntimeHud.cpp,SdlWindow.hpp}`,
`src/shell/PauseCaption.{h,cpp}`, `src/web/webolution.html`, `web/shell.html`,
`cmake/webolution_tests.cmake`, `src/__test__/test_examine_pause.cpp`, the root and
release READMEs, both packaged `README.txt` files and this report. [Entry sources](../../build/examine-pause-001/entry/)

**Handoff.** `examine-pause-001` is **needs_review**.<br>
**Next.** Supervisor smoke is pending, followed by independent review and the human playtest.

**What to check.** In **Designed with love**, press Enter or Pause/Break, then
left-drag to bring the laser switch and gate 5 into view. Expect the centred
caption, a frozen level and the switch and gate to brighten when the pointer
rests on the switch. Dragging hides the highlight until release. P resumes
play. Repeat the same controls in the browser.
