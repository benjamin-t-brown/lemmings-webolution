# panel-icon-anim-001: hovered and selected skill icons

Playable v1, panel feedback, playtest 3 R7. **Implementation complete;
needs_review.** Difficulty: routine to difficult; recommended model Sonnet-class.
This worker uses the explicit Codex override. Mandatory full registered
main/recovery verification and independent acceptance remain with the supervisor.

**Revision 2 (2026-11-14):** the first supervisor run failed three pause-frame
expectations. The correction and its successful focused run are appended below;
that section supersedes the earlier verification/handoff wording, not its retained
evidence. Full registered verification on the corrected content remains pending.

The demonstrated failure is [playtest 3 R7](../playtest/2026-11-13-playtest-3.md):
job icons remain static under hover and selection. This is the next assigned
player-feedback dependency after frontend-window-quit-001. No simulation timing,
skill, release, plate geometry, count, selection or command rule was changed.
No original execution, debugger, hooks, original capture, subagents, installation
writes or commits occurred. Source-built CPU framebuffer PNGs are clone evidence.

## Retained and static evidence

Start points: [HUD/panel lessons](../lessons/hud-and-panel.md),
[runtime integration](runtime-skill-panel-integration.md),
[C1g panel trace](phase-c1g-base-icon-draw-read.md), and
[C1l measured inputs](phase-c1l-icon-live-read.md). C1l is a retained selected,
paused-start observation, not proof of the all-idle pose. Historical C1 scores
and every fidelity gate retain their existing scope.

Target: repository-local `Lemmings Revolution/Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Ghidra 12.1.4 static REA records, including complete returned pseudocode and
instruction lists, are retained at
`build/panel-icon-anim-001/static-evidence.json`. Analysis was closed without a
snapshot. Ghidra/Java analysis did not execute the target.

Instruction-checked findings:

- `45B639`, `45B74D..45B784`: ten fixed-position icon draws through `47A75D`.
  That routine resolves temporal entry `+1CC`, then directional image `+1B8`;
  drawing itself does not advance the clock.
- `45AFCD..45AFE0`: panel update uses selected index `panel+28`, stride `208`,
  base `1150`, and calls `468833` once for that selected icon with its double
  delta argument. This establishes selected advancement, not its full outer
  clock/pause contract.
- `46889E..4688E6`: accumulator adds `dt * clip.rate`; a strict `>0.06`
  threshold advances the temporal entry. `468916..468959` branches on loop flag
  `+201`. The panel's initialization of that flag was not traced.
- `45AC64..45ACD7` and `45AD66..45ADD9`: switching selection restores old/new
  icons using the table numerator times temporal count divided by denominator.
  Climber's table reset is 3, whereas the retained C1l selected snapshot is 4.
- Hover path `45325F..45327B` calls `45AE5E`, which refreshes label text/timer;
  no hover-icon advancement was found in that path. This is bounded static
  coverage, not proof that no other original hover animation path exists.

## Installed frame inventory

Registered test measurement: `build/panel-icon-anim-001/after/assets.txt`.
`Sprite::animations` indexes time; `SpriteAnimation::frames` indexes direction.
All measured rates are 1, giving approximately 16.67 temporal entries/second
under the traced `0.06` threshold. Texture names for every used temporal pose
are in that measurement file; several clips cross multiple texture sheets.

| Plate / clip | Installed SPR | Temporal entries | Directions per entry | Retained facing | Retained idle |
| --- | --- | ---: | ---: | ---: | ---: |
| 0 Climber / climb | LEMMINGSHI | 18 | 5 | 2 | 4 |
| 1 Floater / float | LEMMINGSHI | 15 | 5 | 4 | 0 |
| 2 Bomber / bomb | GREYICONS | 1 | 1 | 0 | 0 |
| 3 Blocker / block | LEMMINGSHI | 16 | 8 | 4 | 0 |
| 4 Builder / build | LEMMINGSHI | 16 | 5 | 2 | 15 |
| 5 Basher / bash | LEMMINGSHI | 24 | 5 | 2 | 0 |
| 6 Miner / mine | LEMMINGSHI | 25 | 5 | 2 | 21 |
| 7 Digger / dig | LEMMINGSHI | 20 | 5 | 4 | 1 |
| 8 command / questionmark | BLACKICONS | 16 | 1 | 0 | 0 |
| 9 command / armageddon | ADDITIVEICONS | 13 | 1 | 0 | 6 |

All SPRs are under `SPRITES\\` in the installed archive. The Bomber panel icon
is one static bomb; cycling its available frames cannot produce motion. No
substitute lemming artwork was invented. Command icons remain on their existing
poses, outside the eight job-icon change.

## Production path and policy

`main.cpp::runApplicationLeg` constructs the fixed `RetailSkillPanel` and a
separate `SkillPanelIconAnimation` cache from installed assets. After public SDL
input and frame-time sampling, it advances the cache once using UI seconds,
selected index and `runtimeSkillPanelIndex(pointerX,pointerY)`. It passes that
cache through the new `drawRuntimeSkillPanel` overload into `RetailSkillPanel`.
The panel draws the unchanged plates/counts, animated skill poses at their C1
anchors/facing/scale, and the unchanged command icons. RuntimeHud then draws
ordinary counters. Rendering does not mutate the cache or World.

Each temporal pose retains its own texture reference and frame geometry;
textures are loaded once and shared within the animation cache. The old fixed
renderer's fields, layout and overloads are preserved for existing headless
fixtures and authentic historical source-built baselines.

Provisional choices are marked in code: animate an unselected hovered job as
well as the selected job; loop; reset fractional phase on activation; restore
the retained fixed pose when neither hovered nor selected; continue while the
simulation is paused, freeze during menu/focus suspension. Fast-forward never
multiplies this UI budget. An icon both hovered and selected advances once.
The retained idle artwork, particularly Climber 4 versus static reset 3, is a
continuity policy rather than a claim of exact original idle behavior.

## Before/after clone evidence and checks

Task-entry copies of both panel files, both runtime-adapter files and main.cpp
were retained **before edits** under `build/panel-icon-anim-001/frozen/src/app/`.
`panel_icon_identity` checks their exact SHA-256 values. Initial dirty changes,
including accepted lever rendering, are preserved; main.cpp's frozen copy
contains those changes. `pre-task.diff`, `CMakeLists.before.txt`, initial hashes
and `pre-test-updates/` retain the entry state.

Registered `panel_icon_before` / `panel_icon_after` exercise installed
`MM_CLIMB_002` through `runApplication`, public SDL P/1 keys and pointer motion,
with a private `FileSaveSink` below the task evidence directory. At frame 0 the
test pauses and hovers Floater while Climber remains selected. Frames 1..14
keep world time at 30 ms while the two icons animate after the change. The
observer reads frames and queues input; it never mutates World.

- Before sequences: `before/public-{1,4,8,12}.png`, static hovered Floater and
  selected Climber. After sequences: corresponding `after/public-*.png`.
  I inspected the before/after full-frame sequence: selected Climber limbs and
  hovered Floater poses change, with the same stock and stable other plates.
- All eight jobs also have isolated hover/selection sequences at UI time
  0, .19, .38, .57 and .76 seconds:
  `{before,after}/{hover,selected}-{0..7}-{0..4}.png`. These are supplemental
  production-renderer fixtures, not natural skill-assignment routes. Seven
  multi-frame jobs change; Bomber remains its one installed frame. Builder and
  Miner sequence images were also visually inspected.
- `before/idle.png` and `after/idle.png`, nothing hovered/selected, are
  pixel-identical: PNG SHA-256
  `e5f281dd22220a1891f70cb3a0010f51c35743f38c6929c96d708746a2eea558`.
  Tests compare RGBA too, including after seven seconds of idle UI time.
- Regression checks cover both triggers for every skill, inactive icons,
  single-frame Bomber, repeated-draw stability, idle restoration, equal time
  split over 1/30/144 frames, repeated wrapping, and no double speed when
  hovered and selected. The Climber change is confined to its independently
  measured 189-pixel temporal sprite union.
- Both before/after `checks.txt` record installed and normal user save existence
  and bytes unchanged. No installed or normal save is used for persistence.

Existing fixture corrections are explicit. `old-fixture-failure.log` retains
the old runtimewindow failure at tick 1: it expected a static icon after 30
frames. HatchEntryFixture now accepts explicit panel time; runtimewindow,
frontendentry, cameraentry and levelmenuwindow retain exact whole-frame equality
and their zero-time controls. `development-3.log` retains the old hover
comparison failures caused by selected Climber animation. Historical hover and
actor-occlusion clone comparisons normalize only the 189 actual Climber sprite
pixels, assert selection 0/pointer outside the panel, and retain exact comparison
elsewhere. The new task test separately checks animation and that the change
stays inside that footprint. No historical fidelity-gate exclusion changed.
Existing assertions/timeouts/counts were not relaxed to hide gameplay failures;
these are corrections of the static-icon expectation that encoded R7.

Development runs: `development-1.log` 4/4 passed; `development-2.log` 8/8 passed;
`development-3.log` 12/13 (the retained stale hover expectation failure);
`development-4.log` 16/16 passed. The final focused run is
`build/panel-icon-anim-001/final-focused.log`: **22/22 passed, 55.78 seconds**.
`build-final.log` records the final focused-target build. Whitespace checks,
new-file LF endings and report links also pass. Full main/recovery verification is
**pending supervisor execution**, not claimed passed here. Registered tools:
MinGW-w64 GCC/UCRT64, `-O2 -g` with assertions, existing main build directory,
12 build jobs, sequential focused CTest. No full suite was duplicated.

## Changed files, limits and handoff

Production: `src/app/SkillPanel.{h,cpp}`, `RuntimeSkillPanel.{h,cpp}`, `main.cpp`.
Registration: CMakeLists.txt (one include), `cmake/panel_icon_anim.cmake`,
`cmake/panel_icon_identity.cmake`. Tests: `test_panel_icon_anim.cpp`,
`PanelIconComparison.h`, `HatchEntryFixture.h`, `test_hatch_entry_fixture.cpp`,
`test_frontendentry.cpp`, `test_cameraentry.cpp`, `test_levelmenuwindow.cpp`,
`test_hover_feedback.cpp`, `test_actor_occlusion.cpp`. This report is the only
documentation change. No queue, controls, registry, permissions, shared
instructions, model pins or completion criteria were edited by this worker.

Unknowns: original hover-only advancement outside the traced path, loop flag,
pause/focus/menu contract and exact all-idle pose. A future authorized interactive
original observation with a different hovered and selected job, then pause,
pointer departure and a complete loop would settle these choices. None is
required to claim provisional playable-v1 feedback; no original-fidelity or
human playability acceptance is claimed.

Handoff: panel-icon-anim-001 is needs_review; retained clone sequences and focused checks support R7, with single-frame Bomber and provisional UI policy documented.
Next action: supervisor runs full registered main/recovery verification, then independent acceptance review; worker does not advance the queue.

## Revision 2: correct paused-frame expectations after supervisor failure

The supervisor's exact run
`build/agent-runs/20261114T210303-47060-910328db/verification.log` completed
**254/257 main tests**, failing `entrycuewindow`, `hatch_exit_app_after` and
`hatch_exit_end_after`. The earlier 22/22 focused result did not cover these
three cases and was insufficient to establish full verification. No passing
full main/recovery result is claimed.

The failure is a direct consequence of the authorized R7 behavior: the selected
Climber uses frame time while the simulation is paused. These tests compared
whole paused frames, thereby also requiring the formerly static icon to freeze.
The observed differences are confined to the same **189 actual Climber sprite
pixels** already measured by `PanelIconComparison.h`. Entry-cue frame 6, hatch
frame 15 on both installed `MM_CLIMB_002` and `MM_FLOAT`, and departure frames
534/535 demonstrate `raw_frozen=0` with `frozen_outside_icon=1`.

Only `src/__test__/test_entrycuewindow.cpp`, `src/__test__/test_hatch_exit.cpp`
and this report changed during this retry. The tests now assert selection 0
and a pointer outside the panel, normalize only that sprite footprint, and
retain exact RGBA equality everywhere else. Frozen hatch/exit controls still
require **unmodified whole-frame equality**. The cue timing/opacity/retirement,
hatch/actor state, departure timer, balloon-region motion, natural win,
simulation-trace identity and save checks remain intact. There is no new
historical fidelity-gate mask. `panel_icon_after` independently checks icon
advancement and confines the selected Climber change to that footprint.
Production files, headers, provisional policy and authentic frozen sources
were preserved byte-for-byte from retry entry; no new static REA call was needed.

Retry-entry copies/hashes/diff are under
`build/panel-icon-anim-001/retry-2/`. An intermediate fixture incorrectly assumed
opaque plate backgrounds; its own assertion rejected that assumption, and the
fixture was removed. Its source and failed log remain in `opaque-fixture-attempt/`.
A subsequent missing-include build failure and stale-binary CTest invocation
are retained as `build-corrected.log` and `corrected-focused.log`; neither is
evidence for the final code. The required includes were then added and rebuilt.

Final registered focused checks:

- `build/panel-icon-anim-001/retry-2/build-final.log`: successful build of the
  entry-cue test and both hatch/exit executables, registered GCC/UCRT64,
  `-O2 -g`, assertions retained, 12 build jobs.
- `build/panel-icon-anim-001/retry-2/final-focused.log`: **9/9 passed, 125.15 s**,
  sequential CTest: entry cue, both hatch and departure controls/production
  routes, hatch frozen identity, both panel demonstrations and panel identity.
- `build/panel-icon-anim-001/retry-2/evidence/manifest.json`: SHA-256 index of
  **85 retained clone evidence files**, copied after that successful run.
  `entrycuewindow/`, `hatch-{before,after}-{climb,float}/` and `end-{before,after}/`
  contain raw `actual` PNGs, explicitly normalized `compared` PNGs, pause logs
  and route summaries. I reviewed the raw cue, hatch and departure pause pairs:
  the Climber pose changes while the rest of each paused frame stays fixed.
  `panel-{before,after}/` retain the public hovered/selected image sequences,
  asset inventory, input/clock CSV, save checks and identical idle images.

The hovered Floater and selected Climber public sequences were reviewed again;
both move while world time stays at 30 ms. Idle PNG SHA-256 remains
`e5f281dd22220a1891f70cb3a0010f51c35743f38c6929c96d708746a2eea558`
on both sides. Every hatch/exit and panel save summary reports installed and
normal saves unchanged. Entry-cue testing uses the application's null test save
store and performs no persistence. No original process, hooks, debugger, capture,
subagents, protected-control edits or full-suite duplication occurred.

Original hover-only advancement, looping, pause/focus/menu behavior and exact
idle poses retain the provisional limits and proposed observation above. The
single-frame Bomber remains static. No human-playability or fidelity pass follows.

Handoff: panel-icon-anim-001 is needs_review; R7 implementation retained, all three supervisor failures corrected, final focused checks 9/9.
Next action: supervisor runs mandatory full registered main/recovery verification on the corrected content, then independent acceptance review; worker leaves the queue unchanged.
