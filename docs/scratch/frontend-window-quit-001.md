# frontend-window-quit-001: one connected window and working main-menu Quit

Status: **needs_review**. Playable v1, human playtest P12/P13 in
[the ledger](../playtest/2026-11-12-playtest-1.md). Difficulty: routine;
recommended model: Claude Sonnet5.5/high; this invocation uses the explicit Codex
provider override. Full registered main/recovery verification is **pending the
supervisor**, followed by independent review. No queue or shared controls were edited.

## Failure and next dependency

The installed route is ordinary menu -> New Game confirmation -> map node0 ->
`MM_CLIMB_002` -> natural timeout/loss results -> results Quit -> map -> footer ->
main menu -> application Quit. Making this route retain its window and actually
exit addresses the two demonstrated human failures; it does not establish a win
or human-confirmed playability.

Before, `runApplication` retained `FrontendSession`, but not presentation resources:

1. `runApplicationLeg` called `runFrontendWindow`. Its local `SdlResources`
   created a 640x480 SDL window, software renderer and RGBA texture. Returning the
   selected level destroyed the texture, renderer and window and stopped video.
2. The same leg loaded the retail level and, near old `main.cpp:810`, created
   another window, software renderer and RGB texture. Near old line1183 it
   destroyed all three and stopped video when the level finished or returned.
3. `runApplication` called `FrontendSession::completeLevel`, then started another
   leg. `runFrontendWindow` created the third window for results/map/menu.

Quit had two independent missing paths: `FrontendSession::menuBoxes_` contained
only rows0/1, so clicking the visible fifth row did nothing; `shell::Frontend::press`
handled Return only at rows0/1, so keyboard focus4 + Return returned Unobserved.
Neither requested application shutdown.

## Measured before and after

The new source-built CTest driver uses public `SDL_PushEvent` input, the existing
read-only frontend/level observers, and an owned 10-second frame-clock input.
It never mutates World or fabricates an outcome. The installed tutorial times out
after 30 presented level frames (saved0/needed1). GNU link wrappers count actual
SDL_CreateWindow/SDL_DestroyWindow calls **inside this clone test executable**;
they are not installed hooks or instrumentation of the original.

| Measurement for the complete round trip | Before | After |
| --- | ---: | ---: |
| Successful SDL window creations | 3 | 1 |
| Destructions between frontend/level/results legs | 2 | 0 |
| Total destructions, including final exit | 3 | 1 |
| Window and image dimensions at each sampled page | 640x480 | 640x480 |
| Still running after mouse main-menu Quit | yes | no |
| Still running after keyboard main-menu Quit | yes | no |
| Application return status, including mouse/keyboard routes | 0 after watchdog close | 0 without watchdog |

The baseline observer sees another frame after Quit and sends SDL_QUIT to stop
it. After the fix that observer is never called again. The regression asserts
these differences, final resource cleanup and video shutdown. A separate fresh
application invocation covers Down four times + Return on the main menu.

Evidence, relative to the repository root:

- Initial authentic baseline, built and executed before production edits:
  `build/frontend-window-quit-001/before.exe`, `before.log`, and
  `before/run-1794589193080560800/{windows.csv,summary.txt,*.png}`.
- Registered baseline rerun with the same final driver:
  `build/agent-native/main/frontend-window-quit-before-test/run-1794589556901866800/`.
- Registered production run:
  `build/agent-native/main/frontend-window-quit-test/run-1794589554426782600/`.
  Its `windows.csv` records creates1/destroys0 for menu, initial map, level,
  results, returned map and returned menu; destruction happens only at exit.
- `build/frontend-window-quit-001/before-after-ctest.log`: both registered cases
  pass. The optional baseline target is available only while its ignored frozen
  sources exist; a clean checkout always builds the production regression.
- Task-entry copies of every changed production source/header were made before
  editing under `build/frontend-window-quit-001/before-source/`.
  `before-hashes.json` and corrected `baseline-source-check.json` establish all
  ten initial copies still match their entry hashes, including pre-existing edits.
  The remainder of src was copied later without replacing those files, for the
  optional baseline build; no other production files changed in between. Its
  `manifest.json` records the retained files. Test harness additions are not
  claimed to be pre-task production code. Both targets use unchanged shared
  asset/simulation objects. `pre-existing.patch` preserves the entry diff.

The seven corresponding PNGs (`menu`, `map-before`, `level`, `level-settled`,
`results`, `map-after`, `menu-return`) are **byte-identical** between the registered
before/after runs: `build/frontend-window-quit-001/image-comparison.json`.
I inspected both settled level images and the after results/map images. They show
the cylinder/HUD, loss page and returned map at the same presentation size. The
first `level.png` is the opening camera frame; `level-settled.png` shows the tube.
These are CPU clone frame exports, not original captures or evidence of desktop
window-manager behavior. The SDL lifetime counts establish the window fix.

Image links:
[before level](../../build/agent-native/main/frontend-window-quit-before-test/run-1794589556901866800/level-settled.png),
[after level](../../build/agent-native/main/frontend-window-quit-test/run-1794589554426782600/level-settled.png),
[after results](../../build/agent-native/main/frontend-window-quit-test/run-1794589554426782600/results.png),
[returned map](../../build/agent-native/main/frontend-window-quit-test/run-1794589554426782600/map-after.png).

## Production change and original evidence

`runApplication` now owns one lazy `SdlWindow` for its connected loop. It passes
that presentation through `runApplicationLeg` to `runFrontendWindow` and the level
loop. Both borrow the same software renderer, retaining their own RAII texture
(RGBA frontend, RGB level). The textures are retired between legs; the window,
renderer and video subsystem remain alive across completion, results, retry and
abandon-to-map paths. At final return the owner destroys the renderer/window and
shuts video down, preserving the separately owned audio adapter. Local owners
keep standalone window diagnostics and standalone frontend tests working.

`FrontendSession` derives a fifth-row hitbox from the existing rendered layout.
Rows2/3 remain unsupported and retain their old mouse policy. Click and Return
both reach `shell::Frontend::press`, which sets `quitRequested_` for row4.
`runFrontendWindow` notices that request immediately and returns no selection;
the existing Exit disposition unwinds `runApplication` with status0, which CLI
main returns. No results/menu routing, focus rules, simulation clock, save codec
or save operations were retuned.

Retained context: [frontend lessons](../lessons/frontend.md), especially the
nonzero-Return correction and the separation of loss-results Quit from process
exit; [loss Quit](loss-results-quit.md); gate B2 remains OPEN in docs/STATUS.md.

Targeted static REA/Ghidra12.1.4, registered reference only:
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Complete returned evidence is retained under
`build/frontend-window-quit-001/static/`:

- `menuConstructor{,Instructions}.json`: `0x48ECDF`, instructions
  `0x48ECF4..0x48ED05`, passes count5, label table0x4C4214 and menu+0x40 to
  shared row builder0x488A92. Retained MenuRender label indices are
  `{125,126,120,3,43}`. The fifth pointer is menu+0x50.
- `quitPseudo.json` and `quitInstructions.json`: main-menu event handler
  `0x48ED89`; `0x48EDCB` requires event0, `0x48EE41..0x48EE47` compares its
  row argument to `[menu+0x50]`, and `0x48EE4C` stores byte1 to0x4E3581.
  Pseudocode agrees with these instructions. No confirmation page is created
  in this branch. `quitXrefs.json` retains the starting references.
- `reaOpen.json` records exact target identity. Analysis was closed without
  snapshot arguments. Ghidra/Java analysis ran; the original game did not.

The exit request is statically supported. Exit status0 is the requested clone
policy; the original's operating-system exit status was not observed. Shared SDL
ownership is explicitly **provisional clone policy**, labelled in SdlWindow.hpp;
no original HWND/device continuity was established. A permitted original handle
trace across menu/level/results would settle that comparison. No such run is
needed for this scoped player-facing fix, and none was launched.

## Checks, preservation and remaining review

Registered UCRT64 GCC16.1 / CMake / Ninja / CTest paths from tools.windows.json
were used, with -O2/-g and assertions. Development builds targeted relevant
executables; the worker did not duplicate the supervisor's full suite.

- `build/frontend-window-quit-001/targeted-first.log`: frontend_sequence,
  results, frontendwindow, frontendentry, new frontendwindowquit and levelreturn
  pass (levelreturn46.40s). frontendsession initially fails21 assertions encoding
  Quit's old unsupported behavior; this failure is retained.
- `targeted-final.log`: frontendsession and frontendwindowquit pass after replacing
  exactly those expectations. The old row4 keyboard-Unobserved check becomes an
  exit-request assertion. The old `(320,450)` no-op checks become positive Quit
  checks from each prior focus, preserving page counter/progress. Unsupported
  rows2/3, inter-row gaps, exterior pixels and every other expectation remain.
- `before-after-ctest.log`: production regression and authentic frozen baseline
  pass with the final driver, including its second-frame image and video cleanup
  checks (4.96s total).
- `lifetime-ctest.log`: existing savepersist, audioapp and levelmenuwindow pass
  unchanged (25.20s total), checking persistence, connected audio lifetime and
  retry/abandon/menu input alongside the window ownership change.
- `final-content-build.log` / `final-content-ctest.log`: production regression
  rebuilt and passes on the final code, including the explicit memory include
  and revised shutdown comments (2.49s). `worker-tested-sources.json` records
  the twelve task code/build-file hashes; no code changed afterward.
- `git diff --check` passes for task code. Pre-existing worker/lead changes remain.

Each application run injects FileSaveSink at its unique output directory's
SavedGame.dat. Each test captures existence and complete bytes of the installed
and normal LocalAppData saves before and after, and asserts both unchanged;
both summaries record1 for each preservation check. New Game Yes is the one
expected private-save write; neither Quit route adds a write. The installed save
and normal progress were never used as stores. No original execution, debugger,
original hooks/capture, install, subagents, concurrent writers, commits or pushes.

Changed task files: CMakeLists.txt; src/app/{main.cpp,FrontendWindow.cpp,
FrontendWindow.h,FrontendSession.cpp,FrontendSession.h,SdlWindow.hpp,
SdlShutdown.hpp}; src/shell/{Frontend.cpp,Frontend.h};
src/__test__/{test_frontendwindowquit.cpp,test_frontendsession.cpp}; this report.
AppEntry.h, world/render changes and source-list additions already present at
entry were preserved, not implemented by this task.

Limitations: dummy video cannot confirm desktop focus/position or perceived
window flicker. Existing level-return tests cover win/Continue, loss/retry and
menu returns; the new lifetime measurement uses one natural-loss route.
The hybrid timing/fidelity parks and full-project completion criteria remain
unchanged. A short optimized human playtest remains necessary for P12/P13.

Handoff: implementation and scoped regressions are ready for review; full registered main/recovery verification remains pending supervisor execution.
Next action: supervisor verification, independent acceptance review, then the requested human frontend/level/Quit playtest; stop after review.
