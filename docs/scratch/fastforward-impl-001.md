# fastforward-impl-001: fast-forward plate -> 7-pass simulation budget

Status: needs_review. Resumed after a keyboard interrupt; the existing partial
implementation was continued, not restarted.

## Milestone step and route

Milestone step: play an existing installed level in the connected application
with working HUD commands. Route: `webolution --level MM_CLIMB_002 --sim` (or
frontend selection) -> `runApplicationLeg` SDL loop (`src/app/main.cpp`) -> left
button on skill-panel plate 8 (physical index 8, fast-forward) ->
`RuntimeSimState::advance` -> `World::tick`.

Source-traced failure (not an observed retail failure): `consumePanelClick`
swallowed indices 8 and 9 and did nothing, `main.cpp` had no plate-8 action, the
`SDL_MOUSEMOTION` branch never checked a held button over plate 8, and
`RuntimeSimState::advance` always ran exactly one pass. Plate 8 was inert.

## Changed files

Production:
- `src/app/RuntimeSkillPanel.h/.cpp`: `enum class RuntimePanelCommand { None, FastForward, Armageddon }`
  and `runtimeSkillPanelCommand(x, y)`, built on the unchanged `runtimeSkillPanelIndex`.
  `consumePanelClick` still returns true for indices 8/9, reads no stock and does not change selection.
- `src/app/RuntimeHud.h/.cpp`: `RuntimeSimState::fastForward()`, `latchFastForward()`,
  `kFastForwardPasses = 7`; `advance()` runs up to 7 passes when latched. Each pass calls
  `clock_.advance(elapsedSeconds)` with the same delta and then the existing `takeStep` loop.
  The latch has no public clear. Pause returns false before any pass and leaves the latch alone.
- `src/app/main.cpp`: in the `args.sim && world` left `SDL_MOUSEBUTTONDOWN` branch and the
  `SDL_MOUSEMOTION` branch (`event.motion.state & SDL_BUTTON_LMASK`), plate 8 calls
  `runtimeSim.latchFastForward()`. Both are inside the existing guards: focused only, not
  `rightDown`; the open level menu and `menuDismissedThisFrame` batches `continue` or branch earlier.

Tests/registration: `src/__test__/test_fastforward_units.cpp`, `src/__test__/test_fastforward_app.cpp`,
`cmake/fastforwardimpl_identity.cmake`, `CMakeLists.txt` block "fastforward-impl-001".
Untouched: SimClock, World, LevelEndTransition, camera, scenery, rendering, HUD drawing, save, audio,
frontend, results, Retry, level menu, skill assignment. `AppEntry.h` was not edited for this task.

## Address -> source mapping (static, from the accepted research; reference SHA-256 32675bdb...7a2f)

| Address | Retained finding | Source |
|---|---|---|
| 0x455AFF-0x455B60 | hover index 8 with mouse button 0 down writes input+0x20 = 1; no stock read/decrement; never the selected skill | `runtimeSkillPanelCommand` + `latchFastForward()` in the button-down and motion (button held) branches; `consumePanelClick` unchanged for stock/selection |
| 0x452BC2, 0x455774 | constructor/reset clear +0x20; the live path only sets it | latch has no clear; a new per-leg `RuntimeSimState` is the only reset |
| 0x45DA6A-0x45DB0E | input dispatch outside the pass loop; positive-dt passes need pause byte 0 | input/camera/entry cues keep the real `elapsed`; paused `advance` returns false |
| 0x45DB0E-0x45DD5C | +0x20 selects 7 passes (dword 0x4B15F8) vs 1; each pass reuses the full delta | `kFastForwardPasses`; pass loop in `advance()`; stops once phase is Results |
| 0x45D927-0x45DA3C | 100 ms delta clamp | NOT reproduced (limit) |

## Per-leg construction

`app::RuntimeSimState runtimeSim;` is a local at `src/app/main.cpp:816` inside `runApplicationLeg`, so each
leg (direct `--level`, frontend selection, Retry, next map selection, return to map) gets a fresh, unlatched state.
It is destroyed when the leg returns.

## Executed before/after (source-built; injected SDL events are source-test setup, not natural play)

`runApplication --level MM_CLIMB_002 --sim`, SDL dummy drivers, `ApplicationRunOptions::frameSeconds = 0.4`,
the level-frame observer reads `World::tickCount()`, exit via observer-queued `SDL_QUIT`. At 0.4 s one pass is
exactly 2 ticks (0.2 s steps), so an unlatched frame adds 2 ticks and a latched frame adds 7 x 2 = 14 (hand-computed).
An event queued in frame f is processed at the start of frame f+1.

| Scenario | Frozen (before) ticks | Current (after) ticks |
|---|---|---|
| click plate 8 at f0 | 2,4,6,8,10 | 2,16,30,44,58 |
| left held, moved onto plate 8 at f1 | 2,4,6,8,10 | 2,4,18,32,46 |
| click plate 9 / non-plate / hover plate 8 no button / key 8 / right press / right held then left click | 2,4,6,8,10 | 2,4,6,8,10 |
| pause (P) f0, click plate 8 f1, P f3 | 2,2,2,2,4 | 2,2,2,2,16 |
| click plate 8 at f0, then click a Walker to assign climber | ticks 2,4,...,16 (unlatched); assigned, climber stock -1, selection unchanged | ticks 2,16,30,44; assigned, climber stock -1, selection unchanged |

Stock array (8 skill counts) and selected skill index were checked on every frame of every scenario: unchanged by plate 8
(the assignment scenario expects exactly one climber spent). Full lines: `build/fastforward-impl-001/app_results_{before,after}.txt`.

Unit checks (`fastforward_units`, installed MM_CLIMB_002 World, hand-computed): unlatched 3 x 0.4 s = 6 ticks; latched = 14/frame;
0.5 s latched = 17 then 35 cumulative (unlatched 2 then 5); 3.0 s latched = 70 (7 x the 10-tick cap), unlatched 10; paused latched ticks nothing and
resume keeps the latch; fresh state unlatched; Results stop; plate 8/9 consumed without stock/selection change; assignment after latching works.

## Superseded assertions

None. No existing test asserted plate 8/9 do nothing beyond `test_runtimeskillpanel` "fast-forward plate consumed, selection unchanged" and
"Armageddon plate consumed, selection unchanged", which still hold. Plate 9 remains inert.

## Verification

`python tools/agent_control/verify.py --profile all` (registered MinGW-w64 GCC, run via the worker), exit 0:
- main: 107 of 107 registered tests passed (the previous 103 plus 4 new: `fastforward_units`, `fastforwardimpl_identity`,
  `fastforwardapp_before`, `fastforwardapp_after`). Total CTest time 662.79 s.
- recovery: 84 of 84 passed; `native_int_array` remains the only native_-prefixed source test; `LEMMINGS_REFERENCE_EXE` empty.
- `git diff --check` clean. Reference SHA-256 32675bdb...7a2f verified by reading bytes only.
- New tests: fastforward_units 0.06 s, fastforwardimpl_identity 0.03 s, fastforwardapp_before 20.53 s, fastforwardapp_after 19.68 s,
  against the 60 s / 60 s / 120 s / 120 s timeouts registered for them. No existing timeout was changed or raised.
  `levelreturn` took 232 s of the verifier's 360 s ceiling; it is an existing test whose assertions are unchanged.
- Incident: a second `verify.py --profile all` was started while the first was still running (mistake by the worker, bash timeout
  handling). Its output is not used here; the first run's result above is the evidence. It writes only under `build/`.
- The frozen-source identity test checks all five entry hashes (main.cpp, RuntimeHud.{h,cpp}, RuntimeSkillPanel.{h,cpp}) against
  `build/fastforward-impl-001/entry-manifest.json` and fails on mismatch. It passed in this run. The entry manifest and frozen files
  were not re-frozen or replaced.

## Limits and unknowns

- 7 is a pass budget (dword 0x4B15F8), not a measured 7x wall-clock speedup and not a 200 ms tick.
- The original 100 ms delta clamp and event-boundary slicing are not reproduced.
- The existing SimClock quantization and 10-ticks-per-advance cap apply to every pass.
- Event-5 recording emitter, aliased +0x20 writers, the 0x1C9B3 force-100ms flag and audio scheduling are unverified.
  Repeated simulation may repeat sound cues (`afterTick` drains each tick); no audio redesign.
- Source tests do not establish original timing fidelity. Natural play-through is unverified. C3 stays FAIL.
- No keyboard fast-forward, toggle-off, hover-only activation or plate highlight (not evidenced / not authorised).

## Milestone step support (source-built only)

startup (ordinary frontend start): source-tested earlier. Select an installed level: source-tested earlier. Play with working plate 8: this task.
Finish: source-tested earlier (results/level-end). Restart-and-retain progress: source-tested earlier (save codec work).
None of these is a natural-play or original-timing verification.

## Next dependency

Timed Armageddon (plate 9) stays parked pending the user's explicit decision between keeping it parked and authorising a coarse
existing-clock approximation (research Decision A/B). No Armageddon work, `World::nuke` use or SimClock change was made.
