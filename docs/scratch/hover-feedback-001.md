# hover-feedback-001: lemming marking and hatch remaining count

Status: **needs_review**. Playable v1, implementation-plan small presentation
fixes, [playtest 3 R5/R6](../playtest/2026-11-13-playtest-3.md). Difficulty:
difficult; assigned recommendation Sonnet-class, executed with the explicit
Codex override. Full registered main/recovery verification is **pending
supervisor execution**, followed by independent acceptance review.

The human reported missing lemming and hatch hover feedback. At task entry,
`hoveredActor` already drove the software cursor and click selection, but no
lemming-body feedback was drawn and no hatch display was composed. This change
supplies that missing presentation dependency. Simulation, picking priorities,
skill rules, release scheduling, cursor shapes, historical gates and full-project
completion criteria are unchanged. Scripted clone observations are not a human
playability pass or original-fidelity evidence.

## Production path

`runApplication` -> `runApplicationLeg` -> installed level loading -> World,
animation/motion tracking -> `renderFrame` -> completed scenery/tube/fluid depth
-> actor draw -> level image composition. The existing shared `hoveredActor`
uses `RuntimePick`, displayed actor boxes, displayed camera/reference, skill
stock and eligibility. Both pointer hover and the next SDL left-button event
continue using that function without changing its rules.

Before HUD/cursor composition, the returned World index is mapped to its alive
walker index. `TubeScene::drawHoveredWalker` calls the same
`prepareInjectedWalker` used by drawing and picking: pose, clip, temporal/view
selection, mirror, tint, placement and actual scene-depth mask. It adds a
half-strength copy of that one sprite to the image. It neither changes scene
depth nor writes World state. No actor hit means no marking call. Exactly one
optional index is composed and exposed as `LevelFrameView::markedActor`.

`BackdropScene::hatchHoverTargets` traverses the existing animated
`entities_425e50` roots in stream order. It projects their actual mesh triangles,
applies the existing facing predicate, and finds bounds of pixels matching the
completed scene depth within a provisional 0.5 world units. Fully hidden hatches
have no target. This reads the current door pose without changing geometry or
depth. Pointer containment selects one visible rectangle. Actors keep precedence;
panel input, right drag, focus loss and the level menu suppress hover feedback.
The hatch route never participates in skill assignment or cursor-state selection.

`hatchRemaining` counts the future releases belonging to that hatch under the
clone's existing `World::spawn` round-robin policy. `RetailLevel.cpp` constructs
the entrance vector in the same hatch stream order. For a single hatch this is
`totalLemmings - spawned`; for several hatches, quotient/remainder allocation
counts each hatch's remaining turns. It does not use saved, dead or live-actor
counts, and does not introduce the blocked per-hatch scheduling change.

`drawHatchHover` draws four gold corner brackets and the number through the
existing recovered `^Font` / `shell::fontCells` / `shell::drawHud` path. Labels
are centered above the displayed hatch and clamped into the viewport. Zero is
displayable after its last release. The usual panel, HUD, cue, menu, cursor and
pause-caption layers retain their order above the feedback. Headless diagnostic
shots retain their existing composition.

## Original static evidence and provisional choices

Registered reference:
`C:/progs/lemmings-webolution/Lemmings Revolution/Lemmings Revolution.exe`,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
REA Ghidra 12.1.4, analysis profile digest
`3c06738e45e144d5f073edbd3244c6bff46222f389525fe659bc92dca2ccdece`.
Only static tools were used; analysis was closed without snapshot arguments.
No original game, original-byte oracle, debugger, hook or capture ran.

Retained starting evidence: [HUD/panel lessons](../lessons/hud-and-panel.md),
[actor lessons](../lessons/actor-rendering.md),
[C2o cursor decision](phase-c2o-cursor-policy.md),
[accepted runtime hover adapter](runtime-cursor-hover.md),
[A4 actor material/blend research](phase-a4-actor-render-state.md), and the
accepted [actor-depth](actor-depth-001.md) and [hatch-timing](hatch-timing-001.md)
reports. New raw REA packets are under
[static/](../../build/hover-feedback-001/static/).

Instruction reads cross-check the pseudocode:

- `454A85`, `454ACA..454B5C`: the selected actor's sprite at actor `+128`
  is copied into screen `+16E0` by `455BA0` (0x6C dwords), selector 2 is
  stored through `406240`, equal RGB operands come from runtime `4C6988`,
  then depth setup and `414920` submit the copy. `406260` stores those RGB
  operands in all four vertex colour blocks. A4 independently established
  selector 2 as ONE/ONE additive blending. This is evidence for a brightening
  sprite redraw, not an outline. Instruction evidence IDs:
  `ev_f60776027926e7f1ff7e9cfaa53564fda4b49def7e4610f1d077ada98ee1227e`,
  `ev_038ec755eb90d6063fd0cf80217354daa91112f692f8167ce7da845f59028ea6`,
  `ev_969ab508533210b8c509434f22627bf34daf54ecc0cd39b0f929ff79f134d3c3`,
  `ev_34ee3eab60cadd63a598d7ecdafb14174d3f658a359b9cbdba0f32267a715e81`.
- `454D6C` tests hatch model-origin rectangles of +/-20 pixels and a positive
  hatch word `+98`; it publishes the selected hatch ID and projected origin.
  Evidence ID `ev_2e7d2688bafe65d7afeb376bc482588a84ba4faee190e3cf1f7348ece307e857`.
- `454B8F..454BA5` pushes **dword hatch +A0** and the `%d` string at
  `4C041C`. The decompiler omits the variadic value argument, so the instruction
  read is necessary. `454BB0..454BEA` chooses a font accessor in mode 0 and
  `[4CBF78]` (the recovered normal `^Font`) otherwise. `454C34..454C64`
  centers the run using its width and subtracts `44 << mode` from projected Y.
  This establishes integer text and placement, not its visible final pixels.
  `%d` bytes and four anchors are registered as read-only CTest measurements in
  `hover_feedback_reference`.

**Quantity mismatch retained explicitly:** prior accepted
[D1 release research](phase-d1-release-trigger.md) identifies hatch `+A0` as
release rate, while `+98` is the remaining word decremented at
`425BBD..425BCB` (retained hatch-timing instructions, evidence
`ev_bc03b8770e90cb03915c79e9d4f3273b7a949798e2b57d94d1e37c4b7b7a8ada`).
The inspected original hover-text path therefore does **not** establish the
remaining-count behavior requested by R6. The clone deliberately displays the
requested unreleased population. It is not described as a recovered original
remaining-count contract.

Provisional presentation choices are also named in code: steady half-strength
additive lemming brightness, gold hatch brackets, rectangle hit bounds, depth
tolerance, normal white `^Font`, mesh-center anchor, viewport clamping and the
remaining-count semantics. Original runtime brightness/pulsing, effective depth
state, mode-0 font, hatch highlight appearance and exact label position remain
unresolved. An authorized original observation of a hovered walker and hatch at
640x480, across several releases and pointer moves, would settle the appearance
and whether the visible number is rate, remaining count, or another display.
No such observation was performed. Multi-hatch population fidelity remains
limited by the existing release scheduler; this task does not reopen it.

## Clone evidence and checks

Authentic task-entry source, including pre-existing accepted working edits, was
copied before changes to
[before-source/](../../build/hover-feedback-001/before-source/).
[Manifest](../../build/hover-feedback-001/before-manifest.json), incoming Git
status/diff and baseline main SHA-256
`de30cd251215178eaf62ee591e36cc0fa16e29468306a5b45926d7ca3d5a6ff7` are retained.
`hover_feedback_identity` pins that main. The baseline executable uses its
unchanged application compositor with shared rendering support; the support's
ordinary draw path remains unchanged, and its new feedback methods are not
called by baseline main. The complete source backup is never regenerated.

Both source-built sides replay the same public SDL event tape. Each shot retains
PNG, raw RGBA, simulation/stock state and pointer coordinates. Installed and
normal saves are snapshotted read-only; routes seed and inject their own private
`SavedGame.dat` under the evidence directory. All three saves remain byte- and
existence-identical. Nothing is imported from or written to real progress.

| Installed route | Before | After / result |
| --- | --- | --- |
| MM_CLIMB_002, direct level, 100-tick pre-roll, pause, pointer over two eligible overlapping walkers | [no body marking](../../build/hover-feedback-001/before/actor/overlap.png) | [actor 5 brightened](../../build/hover-feedback-001/after/actor/overlap.png); 115 changed pixels, all inside its 14x20 placement. Independent nearest-walker oracle agrees; the same-pointer click assigns only actor 5 and spends Climber stock 10 -> 9. |
| Same actor route, pointer moved off | [off](../../build/hover-feedback-001/before/actor/off.png) | [off](../../build/hover-feedback-001/after/actor/off.png); zero changed pixels. Re-entry restores the same one-actor marking. |
| Ordinary frontend -> map node 0 -> MM_CLIMB_002, natural release, hatch pointer | [no hatch feedback](../../build/hover-feedback-001/before/hatch/hatch-7.png) | [7 remaining](../../build/hover-feedback-001/after/hatch/hatch-7.png), [6](../../build/hover-feedback-001/after/hatch/hatch-6.png), [5](../../build/hover-feedback-001/after/hatch/hatch-5.png), at frames 121/144/173; total 10, spawned 3/4/5. |
| Same hatch route, pointer moved off | [off](../../build/hover-feedback-001/before/hatch/off.png) | [off](../../build/hover-feedback-001/after/hatch/off.png); zero changed pixels. |

All paired images above were visually inspected. The body brightens without
marking its overlapping neighbour; hatch brackets enclose the blue hatch and
the white numerals are readable above it. The actor route is a prepared clone
simulation, not an ordinary frontend playthrough. The hatch route is ordinary
frontend entry and natural release, not a level win.

[Comparison](../../build/hover-feedback-001/comparison.txt) asserts exact paired
World/stock state, actor-only pixel changes, and byte-identical framebuffers on
all four sampled no-hover frames. It independently composes the expected count
glyphs over baseline images and checks 74/98/93 visible glyph pixels for 7/6/5,
excluding the later cursor's rectangle. Brackets without numerals cannot pass.
The unit test independently enumerates future release turns for every release
prefix, totals 0..24 and 1..8 hatches, including uneven totals and depletion.

Development verification:

- [Final focused CTest log](../../build/hover-feedback-001/development-ctest-3.log),
  [JUnit](../../build/hover-feedback-001/development-ctest-3.xml): **14/14 pass**,
  35.40 seconds. Eight new task checks plus existing `runtimewindow`,
  `frontendentry`, `leveloverlap`, `levelvisibility`, `levelcursor` and
  `cameraentry_after`. Registered UCRT64 tools, -O2/assertion build, 12 build jobs,
  SDL dummy drivers; source-built application only. Build logs are retained in
  the same task directory. `git diff --check` passes for task source files.
- [Earlier failed check](../../build/hover-feedback-001/development-ctest-final.log)
  is retained: the exact window comparison expected the centered pointer to
  draw no hatch feedback. Its old fixture and actual frame are preserved in
  [fixture-before/](../../build/hover-feedback-001/fixture-before/). The independent
  entry compositor now explicitly adds hover feedback for window expectations;
  its closed headless control remains unchanged. Camera/frontend expected frames
  opt into the same correction. **No existing assertion, timeout, comparison
  mask or tolerance was weakened.** The other failure was the new static check
  passing a hexadecimal OFFSET to CMake file READ; converting that offset with
  `math(EXPR)` fixed the test reader, retaining all expected bytes.
- Mandatory full registered main/recovery verification was not duplicated and
  remains pending the supervisor. No claim that criterion is already satisfied.

Changed production files: `main.cpp`, `TubeScene.{h,cpp}`, `BackdropScene.{h,cpp}`,
`RuntimeHud.cpp`, `AppEntry.h`, new `HoverFeedback.h`. Test/support changes:
`test_hover_feedback.cpp`, `HatchEntryFixture.h`, `test_hatch_entry_fixture.cpp`,
`test_cameraentry.cpp`, `test_frontendentry.cpp`, CMake registration and two new
`cmake/hover_feedback_*.cmake` identity/reference checks. Pre-existing edits were
preserved. No queue, supervisor, verifier, registry, permissions or shared
instructions were edited; no commits, original launches or background work.

Handoff: R5/R6 feedback implemented; paired images reviewed; focused checks 14/14; status **needs_review**.
Next: supervisor full main/recovery verification and independent acceptance review, then human playtest confirmation.

## Verification-failure follow-up (resumed worker)

The supervisor's
[20261114T034228-38924-3eed7e7f verification](../../build/agent-runs/20261114T034228-38924-3eed7e7f/verification.log)
passed **240/242 main checks**, including all eight hover-feedback checks, but
failed `levelmenuwindow` and `actor_occlusion_app_after`. This was failed
verification; it did not establish a main/recovery pass. The original log,
incoming files/diff and failed occlusion images are preserved under
[retry-3/](../../build/hover-feedback-001/retry-3/). Production hover code and
the authentic task-entry source remain unchanged in this follow-up.

Both failures were expectations of the missing feedback reported in R5/R6:

- `levelmenuwindow` copied Retry's entire first image, then drew the menu over
  it. The initial pointer actually hovers the hatch; opening the menu suppresses
  that feedback. The corrected expectation independently composes the same
  pre-release scene, door sample, HUD and entry cue with `HatchEntryFixture`.
  Adding hatch feedback and the cursor must exactly reproduce Retry's first
  image. Drawing the menu over the unhovered version must exactly reproduce
  its next image. Both comparisons cover every RGBA byte. Existing page/root
  counters, clock freezes, Retry/map routes and callback counts are preserved;
  the menu frame additionally asserts no marked actor or hovered hatch.
- `actor_occlusion_app_after` compared **21 no-lemming frames** and two
  near-side frames against an authentic application without hover feedback.
  The first frames hover the hatch; both near-side frames hover actor 0.
  The baseline still runs its frozen, hash-checked main and scene. The driver
  also retains their raw RGBA bytes. Expected images start with those bytes
  and add only hatch feedback or actor 0's brightening, followed by the cursor.
  The selected sprite uses its displayed sample/clip/frame and an independently
  rebuilt installed scene for depth. Actual and expected encoded full images
  must match exactly. Empty-feedback frames retain the original PNG comparison.
  Simulation/clock/camera equality, placement rectangles, far-side invisibility,
  no skill spent on a hidden actor, and the returned-near public Climber
  assignment remain asserted. `OCCLUSION_HOVER_EXPECTATION` compiles this expected
  overlay only into the current test; the baseline's frozen renderer is intact.

These explicitly corrected expectations encoded the demonstrated presentation
defect. No comparison mask, tolerance, timeout, simulation rule, verifier or
historical fidelity criterion changed. The new expected overlays supplement
the existing independent task regression, which checks actual selected-actor
bounds, same-pointer click assignment, no-hover identity and recovered count
glyph pixels; they are not independent proof of original appearance.

Focused development records:

- `retry-3/build-1.log`: compilation failed because the test helper attempted
  to use a caption declaration without its header; corrected before execution.
- `retry-3/development-1.log` / `development-1.xml`: **4/5 passed**. Menu and
  all 21 hatch comparisons passed; the two near-side comparisons exposed the
  expected helper drawing the already present translucent pause-caption
  background twice. The failed helper images are retained in
  `development-1-artifacts/`. Leaving that existing caption intact corrected
  the helper; its pixels lie outside this route's feedback. Production code
  was unchanged.
- [build-3.log](../../build/hover-feedback-001/retry-3/build-3.log) and
  [development-2.log](../../build/hover-feedback-001/retry-3/development-2.log)
  / [JUnit](../../build/hover-feedback-001/retry-3/development-2.xml): registered
  UCRT64 incremental build and **13/13 focused checks passed in 32.74 seconds**.
  This includes all eight hover checks, `levelmenuwindow`, both public-input
  occlusion application routes and both occlusion baseline identities. The
  final application images and expected overlays are archived under
  `retry-3/passed-actor-occlusion-app-{before,after}/`.

The task's paired before/after routes were replayed by those registered checks.
The resumed worker visually reviewed the retained overlap before/after, actor
off, hatch before, hatch 7/6/5 and the ordinary near-side highlighted image.
The comparison remains **115 pixels changed within actor 5's placement**, zero
changes on all four sampled no-hover frames, and visible independently checked
7/6/5 glyphs. The overlap has two eligible candidates; only actor 5 is marked
and the next click spends one Climber on that actor. All retained route
summaries report unchanged installed, normal and private saves.

This invocation changes only `test_levelmenuwindow.cpp`,
`test_actor_occlusion.cpp`, its CMake compile definition and this report. Earlier
task changes and unrelated incoming edits were preserved. The existing
supervisor RepositoryLease belongs to live PID 38924; no clone/compiler/CTest
was live at entry. CIM process enumeration was denied, so ordinary
`Get-Process` supplied the process check. No original, static-analysis session,
subagent, detached work, protected-control edit or commit was started. Text
changes use LF; final source hashes and the incremental patch are retained
under `retry-3/`.

Unknowns are unchanged: brightness/pulsing and exact original hatch appearance,
font/position/depth behavior remain provisional. Static evidence reads hatch
`+A0` for its integer label, previously identified as release rate; it does not
establish R6's requested remaining-count semantics. An authorized original
hover observation across releases would settle that mismatch. None ran here.
Full registered main/recovery verification of the final content remains
**pending supervisor execution**, followed by independent acceptance review.

Handoff: R5/R6 implementation retained; corrected full-frame expectations and all 13 focused checks pass; status **needs_review**.
Next: supervisor full main/recovery verification and independent acceptance review, then human playtest confirmation.

## Resume 20261114T044918: interrupted verification audit

The preceding follow-up describes the retained attempt-2 edits and its focused
development run. Its later supervisor verification,
[20261114T041703-38924-b0c5b662](../../build/agent-runs/20261114T041703-38924-b0c5b662/verification.log),
was interrupted by the host low-memory condition recorded in the project lead's
resume note. The log ends after 18 of 242 main checks, with no completed main or
recovery result. It is **incomplete verification**, not an additional test
failure or a full-suite pass. The earlier 240/242 failure above remains retained.

This resumed worker inspected the existing production path, corrected test
expectations, authentic baseline, retained static instruction packets and paired
images. No further production, header, test or CMake edits were needed. The
retained instruction reads confirm selector 2, the sprite copy and equal colour
operands, and the hatch label's `+A0` source; the provisional choices and quantity
mismatch documented above still apply. No new REA session or original execution
was started.

The registered UCRT64 tools rebuilt the five relevant test executables in the
existing optimized/assertion-enabled build. The same **13 focused CTests passed
in 34.26 seconds**, exit zero:
[log](../../build/hover-feedback-001/resume-20261114T044918/focused.log),
[JUnit](../../build/hover-feedback-001/resume-20261114T044918/focused.xml),
[build log](../../build/hover-feedback-001/resume-20261114T044918/build.log).
These are scoped development checks, not the registered full verifier profile.
Fresh paired comparison again reports 115 changed pixels inside actor 5's
placement, identical simulation/stock and no-hover pixels, and independently
verified count glyphs 7/6/5. All four route summaries retain zero failures and
unchanged installed, normal and private saves. The worker visually reviewed the
overlap before/after, hatch before and 7/6/5 after, and actor-off before/after.

Incoming task sources, hashes, patch and paired evidence were preserved under
[resume-20261114T044918/](../../build/hover-feedback-001/resume-20261114T044918/)
before rebuilding. The authentic task-entry baseline was not regenerated.
The existing supervisor RepositoryLease is owned by live PID 33768. Ordinary
`Get-Process` supplied the process check after CIM enumeration was denied; no
clone, build or CTest process was live at entry. This invocation changes only
this report, using LF; prior task edits and unrelated incoming files remain
preserved. No protected control, shared instruction, original asset or save was
edited, and no subagent, detached work or commit was started. Final source and
protected-file preservation results are retained alongside the focused log.

Handoff: R5/R6 implementation retained; fresh focused checks 13/13 pass; report status **needs_review**.
Next: supervisor full registered main/recovery verification, independent acceptance review, then human playtest confirmation.

## Timeout audit after 20261114T044918 verification

The supervisor's completed
[20261114T044918-33768-7fbc0c48 verification](../../build/agent-runs/20261114T044918-33768-7fbc0c48/verification.log)
passed **240/242 main tests** but timed out both `hatch_timing_routes_before`
and `hatch_timing_routes_after` at their existing 180-second limits. All eight
hover-feedback tests and the corrected menu/occlusion expectations passed.
Recovery did not run. This is a failed full verification, superseding the
preceding pending-verification note for that run; it is not an acceptance pass.
The timeout logs, JUnit, partial route files, incoming sources and patch are
preserved under [timeout audit](../../build/hover-feedback-001/resume-20261114T0520/).

This follow-up compared all retained source/build input hashes against that
verifier's `after-source-hashes.json`. Only the controller-generated queue/status
files differed at entry; production, test, CMake and authentic baseline sources
were identical. The earlier full run
`20261114T034228-38924-3eed7e7f` had passed these same hatch routes in
142.29/147.62 seconds, before its separate menu/occlusion failures. The frozen
hatch route does not call the new feedback methods. These observations do not
establish a hover-code performance regression. Resource contention is a possible
explanation, not a measured cause; no timing, assertion, scheduling, verifier or
production changes were made to force a pass.

With the existing registered optimized/assertion-enabled executables and no
other clone/build/CTest process, focused **serial** CTest execution gave:

- Frozen `hatch_timing_routes_before`: **1/1 passed, 145.35 seconds**, under its
  unchanged 180-second timeout. [Log](../../build/hover-feedback-001/resume-20261114T0520/timing-before.log),
  [JUnit](../../build/hover-feedback-001/resume-20261114T0520/timing-before.xml).
- Current hatch route, its baseline identity, all eight hover checks, menu and
  occlusion checks: **15/15 passed in 176.66 seconds**, exit zero.
  [Log](../../build/hover-feedback-001/resume-20261114T0520/focused.log),
  [JUnit](../../build/hover-feedback-001/resume-20261114T0520/focused.xml).
  The current route preserves its matched 23/30 win, 0/30 loss and 2/20 gadget
  loss, complete release/state comparisons and real-save preservation.

The fresh paired comparison again reports exactly 115 changed actor pixels
inside actor 5's placement, matching same-pointer Climber assignment with two
overlapping eligible walkers, and zero changed pixels on all four sampled
no-hover frames. Recovered-font glyph checks verify remaining counts 7/6/5.
Every hover route reports zero failures and unchanged installed, normal and
private saves. The worker visually inspected the paired overlap, actor-off,
hatch-before and 7/6/5-after images; their presentation agrees with the existing
report. Fresh images, pointer/state/event traces, comparison text and hashes
are frozen in [paired evidence](../../build/hover-feedback-001/resume-20261114T0520/paired-evidence/);
both successful longer routes are archived beside them. No authentic baseline
was regenerated. This is scoped clone regression evidence, not a human playtest
or original-fidelity result.

Only this report changed during this invocation. Earlier task production/test
edits remain intact and are listed above. The existing supervisor RepositoryLease
belongs to live PID 33768. Ordinary `Get-Process` supplied the process check after
CIM access was denied. No protected control, shared instruction, installed asset,
real save or launch profile was edited; no original execution, new REA session,
subagent, detached work or commit ran. Retained static instruction reads were
rechecked; additive redraw evidence and the hatch `+A0` quantity mismatch remain
as documented above. Brightness/pulsing and exact original hatch look/font/position
remain provisional and need an authorized original hover observation to resolve.

Full registered main/recovery verification remains **pending supervisor execution**
on the final content, followed by independent review. The repository documents
`--test-jobs 1` as the supervisor's existing fallback; choosing it is the
supervisor/coordinator's responsibility, not a worker control edit. This focused
serial result does not claim that a future four-job run will meet its timeouts.

Handoff: R5/R6 implementation retained; both timeout cases pass at unchanged limits, focused checks 15/15; status **needs_review**.
Next: supervisor full main/recovery verification and independent acceptance review, then human playtest confirmation.

## Lead note, 2026-11-13 (not worker evidence)

Attempts 2 and 3 failed supervisor verification only on `hatch_timing_routes_before`
and `hatch_timing_routes_after` timing out at their unchanged 180 s limit. The lead
reproduced this on an idle machine (`build/lead/verify-2026-11-13-hover.log`) and
measured that the hover work is not the cause: `levelsweep_chunk_16` alone takes
128.6 s on this tree (131 s before the task), and the heavy play tests are about 30%
slower four at a time even with no hover test running. The two route tests need about
145 s alone, so the lead marked them `RUN_SERIAL` in `CMakeLists.txt`; timeouts and
assertions are unchanged. Full verification then passed: main 242/242, recovery 84/84
(`build/lead/verify-2026-11-13-hover2.log`). Why four-at-a-time became slower since
the host restarted at 21:33 is not established.
