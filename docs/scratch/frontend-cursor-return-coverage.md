# Frontend cursor and return-route coverage — 2026-10-28

This records the offline package. A **subsequent separately authorized**
[native attempt](frontend-native-return-2026-10-28.md) confirmed released/held
cursor states but stopped on the return counter:9, not the predicted8 below.
Its failure and the later root/page explanation are retained separately.

**Later observations:** the [complete abandonment/footer route](frontend-native-return-complete-2026-10-28.md)
and [natural win / second unlock state](frontend-native-second-unlock-2026-10-28.md)
now close those bounded comparisons. The missing-case table and predictions below
record the original predeclared scope; use STATUS for remaining work.

## Scope and predeclared checks

Continue the connected-frontend package, offline. No native launch, attachment,
desktop input, registry access or installed-save/config writes are authorized by
this note. The retained [entry packet](frontend-native-comparison-2026-10-28.md)
is the image oracle, not permission to reuse its expired capture window.

The concrete missing behavior is frontend cursor presentation: the retained
map frame contains a centered cursor while the production frontend omits it.
Recover the frontend's resource/state producer before implementing it; do not
reuse gameplay hover/Grab selection merely because those textures already load.

Predeclared checks, before changing or scoring the renderer:

- Trace the native frontend cursor state/resource and positioning to executable
  byte anchors. Distinguish the software renderer from native OS-cursor mode.
- Compose that cursor last on the actual frontend window path. Motion must not
  activate map nodes or the footer, change progress, or construct a screen.
- Use the same absolute SDL coordinates for clicks and the drawn pointer;
  exercise motion with zero relative delta, clipping, focus loss/regain and
  screen transitions. SDL behavior is a source adapter, not physical-input proof.
- Check independently decoded cursor texels and unchanged background pixels;
  verify omission/position/input mutations fail the relevant regression.
- Re-score menu, confirmation and fresh-map images against the **same retained
  native frames**, full 640×480 RGB, no alignment, masks or exclusions. Keep the
  prior scores and images. A regression above 0.02 or wrong/missing cursor pixels
  fails this bounded comparison; a passing average alone cannot excuse absence.
- Build and run the complete configured Windows CTest suite, including retail
  tests and SDL-dummy application routes. This cannot close native B2/B3/C2.

## Missing native comparisons

These cases are selected before new observations. Their existing source tests
are useful regressions, not substitutes for the missing native sequence/frame.

| Case | Existing source coverage | Required native observation / failure condition |
| --- | --- | --- |
| New Game → No → Menu | `frontendsession`, `frontendwindow` | Screen-list/counter and focus before/after No; disagreement fails the route. |
| Map footer → Menu → New Game | `frontendsession`, `frontendwindow` | Click and Return activation, counter/list sequence, highlight and retained progress. Static footer routing alone does not prove physical binding. |
| In-level menu Resume/cancel/O-close | `levelmenuwindow` | UI2 dismissal to UI0 versus O-close UI1, counter/list and pause state. |
| In-level menu Quit → Map | `levelmenuwindow` | Map23 with no result or completion write, followed by a valid node re-entry. |
| Win → Continue → Map | `results`, `levelreturn` | Win selector21, displayed counts, completion write, Continue focus/action and map return. |
| Loss → Continue / Retry / Quit | `results`, `levelreturn`, `frontendwindow` | Loss selector20; map versus reload, intro preference and prior completion preserved. Test each action, not just their shared destination. |
| Second map unlock state | `mapscene`, `levelreturn` | After first-level completion: all 102 flags/states plus unscaled frame. Predicted node0=2 and nodes3/4=3; compare full frame ≤0.02 without exclusions. No synthetic flag fixture earns B3. |
| Frontend pointer / hover | No frontend pointer regression at start of this work | Background, unlocked/locked node and footer positions; resource/state, highlight, pressed/released and focus behavior. Do not infer hover semantics from the retained single center position. |
| Gameplay hover/no-hover/exit | `levelcursor`, `runtimecursor` (bounded subset) | Matched native state and sprite on each required C2 path; frontend cursor work does not close this. |

The smallest next live observation is an exclusively owned, bounded
entry → in-level-menu Quit → map → footer return, with pointer positions and
screen/state brackets. The second-unlock-state comparison separately needs a
real completed-level return (or an explicitly labeled state fixture, which
would not prove the natural progression route). Arrange fresh desktop access
before either. Parked simulation timing is not reopened by these cases.

For that smallest follow-up, the predeclared expected construction sequence is
Menu3 → Confirm4 → Map5 → Level5 → in-level menu7 → Map8 → Menu9. The7 is
supported by the earlier observed ESC route;8/9 are predictions from the static
Quit/footer constructions, not new observations. Read UI/list/counter brackets,
loaded path and all map flags/states; retain unscaled map-return/footer frames.
At a safe map-background position, sample released and held-button cursor state
and artwork. Any route/count/flag mismatch is a failed comparison, not permission
to retry or silently revise the expected sequence. Reuse the prior owned-process
capture envelope with a fresh dated packet and explicit expiry only after fresh
authorization; its original script/attempt/backups remain immutable evidence.

Historical source-inventory caveat for that comparison (subsequently corrected
and mutation-tested in the [connected lifecycle implementation](frontend-native-return-2026-10-28.md#connected-implementation-and-offline-regression)):
`shell::Frontend::press(Esc)` has
the retained dialog construction-counter increment, but the production in-level
menu uses its own `main.cpp` menu state rather than that method. Do not use the
standalone shell replay as proof that the connected app reports the same native
construction sequence through the menu. This diagnostic/state integration gap
must be checked alongside the actual visible return route, not papered over by
adding an expected counter only to a test.

## Static recovery and retained-image finding

**READ, not a fresh runtime observation:** `453BF6` checks the separate frontend
flag `4C369D`, not UI3 or the gameplay cursor policy. The true branch integrates
frontend XY, then `453D3E/453D54` tests two **held** button predicates. `453D68`
writes cursor state10 (`_curMseDwn`) if either is held; `453D77` writes state1
(`_curNoLem`) otherwise. `453D94` skips the gameplay hover policy. Right-hold is
not Grab and does not freeze frontend motion. Menu highlighting and map-node
hover do not enter this cursor-shape decision.

`45620B/4562E7` bind the normal/held sprites at screen+70/+1468. Resource
registrations `45E7F3/45E8CB` join these to the named BMPs; `455774` supplies
32×32 source rectangles. `4577CD/4579D0/457A30` use integer centers and half-size
offsets. `45E2C7` draws the frontend before `45E311` draws the cursor. The static
initial XY is 320,240; `453C3A..453D23` clamps to inclusive [0,width]/[0,height].
Native movement adds relative device deltas without the gameplay 2.0/2.4 scales.
Absolute SDL input remains an explicit adapter, not a newly proven native sampler.

The software path requires backend byte `58CDAD==0`. The alternate path uses
monochrome PE cursor groups106/116, not the colored artwork. The retained capture
did not sample that byte; changing dgVoodoo's window option does not establish it.
The colored source-image correspondence supports the software path for these
retained images only. No alternate-backend implementation is claimed.

Reproduce the static anchors with
`python tools/harness/read_frontend_cursor.py` (**34 anchors**). The main agent
also independently disassembled the full flag/motion/held branch and reran the
researcher's 35-anchor/resource checker. Detailed evidence/REA IDs:
`build/tmp/enacter-frontend-cursor/findings.md`; independent checks and comparison
artifacts: `build/tmp/enacter-frontend-cursor-comparison/`.

**Measured file-image agreement, unresolved mechanism:** at the unshifted center
320,240, only 52/240 opaque `_curNoLem` source texels have exact raw RGB matches.
Converting each channel with `(v & 248) | (v >> 5)` gives exact matches for all
236 surviving nonblack texels on menu, confirmation and map. Four other texels
become black; confirmation's black background also agrees there, whereas menu/map
show background colors. This is consistent with five-bit texture conversion
before black keying, but the native conversion stage/format is **not proved**.
The first exploratory checker incorrectly required all240 converted values to
match on every background and failed on menu/map; that negative is retained in
the research record, not a native renderer failure. Its exploratory stdout was
overwritten by the refined checker; `initial-texel-check-failure.json` explicitly
reconstructs the counts/assertion from the tool response, not a byte-preserved log.

Do **not** introduce a frontend-only quantization exception to fit these images.
The integration retains the existing unquantized cursor compositor. Normal
cursor color fidelity therefore remains open even if the full-frame averages
pass; held-state artwork has no native screenshot comparison yet. Resolve the
shared texture conversion mechanism before changing the renderer's colors.

Baseline build succeeded; full configured Windows CTest **70/70 PASS**,236.25s.
The first wrapper invocation expired at120s during `levelreturn`; rerunning with
a600s command timeout completed. Both logs are preserved in the comparison packet.

## Production integration

`RuntimeCursor` now supports the held artwork and a separate frontend state
selector. `FrontendSession::image()` remains the cursor-free scene; its
`composeFrame()` adds the cursor last. The actual SDL loop, `--frontend --shot`
and `FrontendRenderDump` all use that composition. Menu, confirmation, map and
results share the session's logical pointer; no gameplay hover policy is reused.

The SDL adapter takes absolute motion/button XY for both the cursor and click
hit test. Left/right held bits are independent; releasing one does not clear the
other. Focus loss hides the software cursor, clears holds and ignores unfocused
pointer/key input. A new frontend window retains logical XY but resets button
ownership and focus so an earlier map click cannot stick through a level return.
These lifecycle choices are source adaptations, not native focus or handoff
measurements. The level window still has its own pointer adapter; exact native
pointer continuity through that boundary remains open.

SDL's ordinary OS cursor is hidden while this software path is active, with
scope-based restoration on every return after acquisition. This is not the
native monochrome OS-cursor backend. No cursor warping or registry/device reads
were added. Existing button-panel highlighting/keyboard traversal and the
in-level-menu held-cursor lifecycle remain separate unsupported work.

## Retained-frame comparison

`FrontendRenderDump` uses the production composition. The independent
`check_composed_frames.py` in the comparison packet checks each entire output
against the old base frame plus independently decoded black-key BMP texels:
all three agree exactly, including unchanged background/transparent pixels.
The native comparison uses the existing `compare_camera_frame.py` over
`--roi 0 0 640 480`, with no registration changes or exclusions:

| Screen | Before cursor | After cursor |
| --- | ---: | ---: |
| Menu | 0.005244204452614379 | 0.004949231515522876 |
| Confirmation | 0.002384191176470588 | 0.00223380480664488 |
| Fresh map | 0.008217405364923748 | 0.007829422828159042 |

All remain below0.02 and the missing cursor is now present at the correct
unshifted center. Before/after PNGs, differences and score JSONs are separate.
This is a re-score against old observations, **not** a new native input run,
second unlock state, held-cursor comparison, or closure of B2/B3/C2. The explicit
normal-cursor RGB residual above remains despite the passing full-frame means.

## Regression coverage and sensitivity

- `runtimecursor`: independent whole-frame composition for all four supported
  resources, including MouseDown, at centered/clipped/offscreen positions;
  frontend normal/held/hidden policy checked separately from gameplay.
- `frontendsession`: unchanged cursor-free base, normal and pressed artwork
  texels and keyed background, inclusive edge clipping, integer-extreme clamps,
  both-button release, and hidden composition identical to the base.
- `frontendwindow` under SDL dummy: absolute motion with zero relative delta,
  movement while right-held, independent button holds/releases, held cursor
  across confirmation transitions, focus loss/regain and ignored unfocused
  keys/buttons/motion, same-session window recreation, no progress/construction
  change on node/footer motion, and OS cursor hidden while active. Its captured
  moved-cursor final PNG equals the observer's composed frame. The test now
  refuses non-dummy drivers and gets its output directory from CTest.
- `frontend_menu`: actual app headless/window-loop/dump composed frames agree;
  the separate base renderer remains checked by `frontendsession`.

Each isolated production mutation failed its intended regression: omit frontend
cursor drawing; select gameplay Grab instead of MouseDown; substitute relative
deltas for absolute SDL pointer motion; retain stale button holds on window
recreation. Failure logs and the relative-motion failure image are retained in
the comparison packet. Source files were restored against `pre-mutation.sha256`
before the final build. No native image, gate threshold or golden pixels were
changed to make these regressions pass.

Final restored UCRT64 build and full CTest: **70/70 PASS**,225.58s
(`final-build-tests.log`). Retail assets were configured and also exported for
the optional menu-render asset checks. Only the opt-in historical C1bc scratch
stream comparison remained skipped; this is not an additional native gate pass.
The rebuilt menu/confirm/map dumps are byte-identical to the scored frames.
The original capture packet's hash/state/restoration verifier still passes.
Documentation validation, edited-source LF checks and `git diff --check` pass.
During this offline package no original process was started, no desktop was
acquired, and no installed save/config/registry was modified. The subsequent
authorized native attempt and its restoration are recorded in the linked note.
