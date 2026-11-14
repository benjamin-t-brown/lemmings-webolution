# map-level-names-001 — map hover titles (P14)

2026-11-13. Playable v1 step 8, human playtest P14 from
[playtest 2](../playtest/2026-11-13-playtest-2.md). **Implementation ready for
needs_review; full registered main/recovery verification is pending the supervisor.**
Difficulty: routine. Recommendation: Sonnet-class, as assigned; this run uses the
explicit Codex provider override. No model or controller settings changed.

The player could not name the water level in their report because map hovering
showed nothing. The concrete fresh-game routes here are map nodes 0/1/2,
`MM_CLIMB_002` / `MM_FLOAT` / `MM_BOMB`, whose installed display titles
are **Just Climb / Just Float / Just Bomb**. The fix supplies those names before
level selection. The broader 102-node check uses a private completion-save fixture;
it is not a claim of human playability or natural victories on those levels.

## Production path and change

`runApplication` in `src/app/main.cpp` creates the ordinary `FrontendSession`,
then calls `runFrontendWindow`. Its focused `SDL_MOUSEMOTION` handler calls
`FrontendSession::setPointer`. Each presented frame calls `composeFrame`, uploads
that RGBA buffer with `SDL_UpdateTexture`, and presents it. The frontend screenshot
paths also use `composeFrame`.

Before: `composeFrame` copied `image()` and drew only the cursor. Pointer motion
could not add a title. `click` / `clickMap` already used `MapScene::pickNode`, the
installed indexed mask, native graph state and the accepted group/slot mapping.

After: on a focused map, `composeFrame` uses that same unchanged `pickNode` and
draws the node's display title through the existing `shell::drawHud` alpha blitter,
then the existing frontend cursor. The cached `image()` stays the base map with
footer, so each frame starts clean and off-node motion removes the title without
stale render state. No hover result is published as a selection or saved progress.

`MapScene::build` already loads `LEVELS\\LEVELS.MAP` and maps node to level id.
It now also reads `COMPILED\\LEVELMAP.DAT` through the existing `loadLevelMap`,
joins the id case-insensitively, and retains `LevelInfo::name`. All 102 installed
nodes have nonempty display names; no internal-id fallback is needed. A missing
title is a visible construction error rather than an invented name. `titleForNode`
is read-only. Picking, graph/eligibility, launch ids, completion flags and save
operations are unchanged.

Changed files: `CMakeLists.txt`, `src/app/MapScene.{h,cpp}`,
`src/app/FrontendSession.{h,cpp}`, `src/__test__/test_maphover.cpp`, this report.
The header changes were made together before the first build. Task-entry copies
are under `build/map-level-names-001/before-source/`. The supervisor already owns
the repository lease (PID17836); this worker made no protected control edits.
Pre-existing changes to the installed hooks log, tasks and generated RUN_STATUS
were preserved. No original game, hooks, debugger, original capture, copied
machine-code oracle, subagent, install, commit or push was run.

## Retained and new static evidence

Starting evidence: [frontend lessons](../lessons/frontend.md), especially B3k's
mask/state predicate and B1k/B1v's recovered default font;
[B3 map preparation](phase-b3i-map-preparation.md);
[accepted two-state B3 comparison](frontend-native-second-unlock-2026-10-28.md);
[frontend window/Quit integration](frontend-window-quit-001.md). B3 explicitly did
not establish hover behavior. Its historical gate and all timing/fidelity parks
remain unchanged.

Targeted REA static Ghidra12.1.4 evidence is retained in
`build/map-level-names-001/static/`. Registered reference:
`Lemmings Revolution/Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
These are instruction reads and decompiler output, not original execution.
Ghidra analysis was closed without snapshot arguments.

| Question | Static evidence and instruction cross-check |
| --- | --- |
| Which node / when? | `49B121` calls `49B26A` to update the selected group/slot for a valid hover, then calls `49B2F6` when it does not launch a level. `49B2F6` reads cursor XY through `455EC0`/`455ED0`, calls indexed-mask consumer `49ABC8`, and permits only state2/3. `0x49B323..0x49B351` verifies those conditions. |
| Which text? | `49B2F6` uses `4E3654 + group*0x1B58 + slot*0x118`, passes that record through `406500`, `48E530` and `4605F0`, then fills both title widgets. Instructions `0x49B3B3..0x49B409` confirm the strides, title address, and widgets at map+54 and map+48. The launch path instead uses `4E35D4`. The clone uses the installed metadata's display-name field, not that launch id. The native record-population loader was not independently retraced in this task. |
| Where / colour? | `49AC47` creates black text widgets at `(30, row*20)` in map+44/+48/+4C, and red widgets at `(32, row*20+2)` in map+50/+54/+58. Thus the title pair is `(30,20)` and `(32,22)`. Instructions `0x49AF2D..0x49AF74` and `0x49AFB6..0x49B003` confirm arithmetic and RGB arguments `(220,50,50)`. No title panel is added. |
| Alignment / font? | Setup replaces default flags with0x100 (`0x49AEB8..0x49AEC3`); `48A720` leaves horizontal offset0 when neither0x40 nor0x80 is set. `4898EC` -> `48B650` -> `486A9C` gets `[4CBF78]`, retained B1 evidence's `^Font`. `489F9B` and `48B980` copy it into widget+50. Instructions for these helpers and `48A720` are retained alongside pseudocode. The clone reuses its existing installed `^FONT.BMP`/ALF loading and ASCII glyph table. |
| Off-node / locked? | `0x49B571..0x49B63D` replaces all six strings with empty text when the mask misses or state is neither2 nor3. Locked nodes therefore show no title. This is statically supported, not a guessed reveal policy. |

Principal Evidence IDs: hover pseudocode
`ev_86a710ce59793650501692c1114cfb96e823ecda067e0863218df3470457e4e4`,
hover instructions
`ev_4ecb0e4d2b84e5a75cf7eff1241a2cedafb5d393a2e2f2926f56d5750958a289`,
setup instructions
`ev_88a268f69ad07c27dd2c94b48edf27c3f4554503464df8b9dd70773393d6f76e`.
Complete records include provider identity, profile digest and limitations.

Explicit provisional clone choices, also labelled in `composeFrame`: draw black
then red, and suppress the hover title while unfocused. Exact native final widget
stacking, focus behavior, sampler pixels and framebuffer equivalence were not
observed. An authorized original observation at the same node and focus states
would settle those comparisons. No such observation was required or attempted
here. The native hover also fills lemming-count/time widgets; adding those is
outside P14. Installed titles used here are English, single-line ASCII and fit
the recovered font/window; no broader localization claim is made.

## Clone evidence and checks

`maphover` is registered in CTest and uses `runApplication`, the ordinary
New Game confirmation or Play Current Game menu, real SDL pointer events,
installed mask/title/font assets and unique private `FileSaveSink` paths. The
test independently decodes mask bytes to choose points and builds a title pixel
oracle from the installed glyph alpha, without calling `drawHud` for that oracle.
The same driver builds as optional `maphover_before` against the authentic
task-entry MapScene/FrontendSession sources; other production files were unchanged.
This baseline does not contain resumed modified sources.

- `build/map-level-names-001/targeted-build.log` and `driver-build.log`: registered
  UCRT64 GCC16.1/CMake/Ninja, -O2/-g, assertions retained, jobs12; targeted builds pass.
- `build/map-level-names-001/targeted-ctest.log`: **7/7 pass**, including unchanged
  mapscene, frontendsession, savepersist, results, frontendwindow, plus the new
  production/baseline hover tests. Existing assertions/timeouts were not changed.
- `build/map-level-names-001/final-driver-build.log` and `final-hover-ctest.log`:
  final driver (including valid CSV quoting for titles containing commas) rebuilt;
  **2/2 pass**. No production code changed afterward.
- Final after run:
  `build/agent-native/main/map-hover-test/run-1794611655427153900/`.
  Final baseline run:
  `build/agent-native/main/map-hover-before-test/run-1794611656743842900/`.
  Both `summary.txt` files record102 selectable nodes, one actual level frame,
  failures0, installed-save-unchanged1 and normal-save-unchanged1. The final click
  still launches the installed first level through the ordinary production loader.
- `build/map-level-names-001/check-evidence.py` and `evidence-comparison.json`:
  **114 no-title frames are identical before/after**, including off-node after
  every visit, locked-node, footer, edge and unfocused frames. All109 eligible
  hover visits change, including102 distinct nodes and repeat/refocus checks.
  Every base-map hash remains identical. The18 retained PNG pairs differ only
  within the title box; all no-title PNG pairs are byte-identical. The manifest
  records image hashes and tested/frozen source hashes. This offline checker
  uses Python's standard library and reads only the generated clone images.
- Task code `git diff --check` passes and tracked task files retain LF.

Retained fresh-game images, visually inspected (640x480 clone framebuffers):

| Node / pointer | Before | After |
| --- | --- | --- |
| 0, `(209,203)`, Just Climb | [before](../../build/agent-native/main/map-hover-before-test/run-1794611656743842900/fresh-node-0.png) | [after](../../build/agent-native/main/map-hover-test/run-1794611655427153900/fresh-node-0.png) |
| 1, `(209,236)`, Just Float | [before](../../build/agent-native/main/map-hover-before-test/run-1794611656743842900/fresh-node-1.png) | [after](../../build/agent-native/main/map-hover-test/run-1794611655427153900/fresh-node-1.png) |
| 2, `(209,269)`, Just Bomb | [before](../../build/agent-native/main/map-hover-before-test/run-1794611656743842900/fresh-node-2.png) | [after](../../build/agent-native/main/map-hover-test/run-1794611655427153900/fresh-node-2.png) |
| Off-node `(320,240)` | [before](../../build/agent-native/main/map-hover-before-test/run-1794611656743842900/fresh-off.png) | [after](../../build/agent-native/main/map-hover-test/run-1794611655427153900/fresh-off.png) |

The three hover differences have bounds `[30,26,137,48)`, `[30,26,130,48)` and
`[30,26,137,48)`. The cursor, map lamps and footer retain their before pixels.
These are source-built clone outputs, not original captures. Dummy SDL does not
establish desktop presentation or perceived responsiveness. Each route compares
the installed and normal clone save's existence and full bytes before/after;
only the unique test-directory save is used as a store. Fresh New Game makes its
one expected private write; hovering makes no writes, and the all-progress route
preserves fixture A/B/flag fields. The injected all-progress save is test coverage,
not a natural unlock/playthrough observation.

Handoff: implementation and scoped regressions are ready for needs_review; full registered main/recovery verification remains pending supervisor execution.
Next action: supervisor verification and independent acceptance review, then human confirmation of P14 on the ordinary optimized clone; no broader task was undertaken.
