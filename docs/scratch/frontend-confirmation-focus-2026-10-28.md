# Confirmation focus and keyboard No — predeclared investigation

## Question and prediction (before a live run)

The application currently sends every confirmation Return to Yes; its arrows
only move main-menu focus. Native mouse No is already observed, but that does
not establish keyboard traversal or Return at focused No.

After static validation of the confirmation's two-row focus structure, test one
ordinary process using guarded shim keys, with pointer320,240 outside the choice
rows. Predicted route:

| Action | Counter | Page / focused choice |
| --- | ---: | --- |
| Startup / Return | 3,4 | Menu row0, then Confirm Yes(row0) |
| DOWN | 4 | Same Confirm page, No(row1) |
| UP | 4 | Same Confirm page, Yes(row0) |
| DOWN, then RETURN | 4,5 | No(row1), then rebuilt Menu row0 |
| RETURN, then RETURN | 6,7 | New Confirm Yes(row0), then fresh Map |
| Footer click320,455 | 8 | Rebuilt Menu row0 |

All pages remain UI3. No must preserve the102 startup completion DWORDs; Yes
then resets them and yields the retained fresh map state `[3,3,3]+[1]*99`.
The startup flags may be nonzero: do not inject a source fixture to imply native
save loading exists in the clone. No level/gameplay entry is needed.

Structural row pointers/owners/labels and stable brackets—not just screenshots—
must establish focused No before Return. A different focus transition, page,
counter, UI or progress result falsifies the prediction. Lost foreground,
invalid geometry, malformed structure or unconsumed input stops the run. Preserve
failures; no blind follow-up keys or retries under a one-launch grant.

Compare Confirm Yes and Confirm No at counter4, post-No Menu5, post-Yes Map7 and
final Menu8 against actual-app SDL route exports. Full640×480 normalized absolute
RGB difference ≤0.02, no exclusions/registration/resizing. This is additional
bounded input/frame evidence, not all menu hover, physical input or B2 acceptance.

## Pre-run status (retained)

Offline preparation only. Static field/key-chain validation is complete below.
Any capture needs a fresh exclusive desktop window of at most five minutes,
the retained owned-process envelope and exact save/config restoration. Earlier
grants are spent. No native result or implementation change claimed yet.

Offline capture review found that `LRInput.wait_frames()` counts mouse polls,
not keyboard polls. This packet now brackets each held/released key phase with
both counters, requiring at least two keyboard polls as well as20/2 mouse polls,
under a two-second deadline. `dinput_shim.c:149–161` increments `key_frame` at
keyboard `GetDeviceState` entry; that is polling evidence, not proof of semantic
acceptance. Stable page/focus observations must still show the declared effect.
Earlier packets' “consumed frames” labels denote mouse polling, not independent
keyboard counts; their observed routes are not retracted by this distinction.

The five scored images remain `confirm-yes`, `confirm-no`, `no-menu`, `map`, and
`returned-menu`. Initial-menu, restored-Yes, pre-Return-No and fresh-confirmation
captures are additional structural/visual evidence, not extra comparison gates.

## Static read completed; no live result yet

`build/tmp/enacter-confirm-focus-20261028/` retains findings, raw disassembly and
the stock-hash/raw-backed checker. Main reran87 byte pins and8 negative controls,
and the copied reader's6 synthetic test methods. These are static/helper checks,
not observed key traversal.

Confirm vtable4B3F38 links its raw/semantic handlers49BC21/49BC59. Manager48F835
gets manager+10 and invokes the focused row's input slot48FCDD;490380 tests
internal key13 and row48FD72 dispatches event0 to its owner.49BCAB compares the
event row with page+44 (No),49BCE5 with page+40 (Yes). Thus activation is not
hardwired to Yes. Shared builder488A92 creates two rows from4C4514 labels21/22,
sets Yes initially, and wires both previous/next links to the opposite row.
Direction events2/3 delegate through48896A to the same manager focus setter.
Physical scan-code translation and runtime input priority remain unmeasured.

The capture uses the separate read-only `read_confirm` predicate: page+40/+44,
manager+8 owner/+10 focus pointer, correct labels/eligibility/ring, configured
No7/Yes23 routes, non-choice heading at+48, singleton UI3 and duplicate stable
reads. Its expected root comes from the validated startup main menu, not a guessed
Confirm root vtable. Every call records its raw reads. No pointers survive a
page change except the explicitly guarded persistent root identity.

## Native observation

Fresh five-minute grant at23:24:52Z, deadline23:29:40Z; one owned PID11560.
Packet: `build/tmp/enacter-native-confirm-focus-20261028/`. All predicted focus/
page/counter transitions occurred, including focusedNo→Return→Menu5 and fresh
Confirm6 initiallyYes→Map7→Menu8. All remained UI3, pointer320,240 in screenshots.
The nine captures include the five declared comparison frames; all extra frames
are corroborating evidence only. Yes/No highlights visibly exchange colours.

Current startup flags were **all102 zero**, and No left them unchanged. Yes also
produced allzero flags and map states `[3,3,3]+[1]*99`. The earlier mouse-No run's
nonzero startup flag must not be transferred here: this run does not establish
keyboard-No preserving nonzero progress or Yes clearing a nonzero flag. The
startup difference remains unexplained; save loading/persistence is still open.
Exact installed-save hash restoration does not by itself prove native loading.

An initial offline verifier incorrectly hardcoded the earlier run's `[1]+zeros`
and failed. `verification-initial-failed.log` is retained; the check now describes
this packet's actual zero baseline. The predeclared live prediction allowed
arbitrary startup flags and required No to preserve them, which was observed.

Owned process/input cleanup succeeded. Independent verification at
**23:25:27.992762Z** found no stock process and exact save/config/exe/shim hashes.
Desktop released; grant spent. `verify_packet.py` replays22 raw structural reads,
nine stable image brackets, seven held/released keyboard-poll records and the
footer click. Main inspected all five declared frames; source comparison pending.

## Implementation and completed comparison

`FrontendSession` now uses its existing model-owned highlight for the two
confirmation choices. Directional input toggles it; the image cache redraws the
choice panels and independent heading, and Return selects the focused choice.
`shell::Frontend::press` agrees with the session. No hover/repeat rule or unrelated
menu action was added. Returning No reconstructs Menu at row0; entering the next
Confirm starts at Yes and replaces any cached No image.

The production `runFrontendWindow` SDL-dummy regression executes the exact
Menu3→Confirm4 DOWN/UP/DOWN→Return Menu5→Confirm6→Yes Map7→footer Menu8 route.
No focus/progress/outcome fixture is injected. All five exports require
SDL-delivered pointer320,240 with both buttons released. It asserts page/counter,
focus, zero completion flags, no level selection and independent fresh map states.
This is the production frontend window path, not a new full-gameplay replay.
Separate unit tests check heading/backdrop preservation, cache reset, shell/session
agreement and nonzero progression retention/reset; their completed-level setup
is **SOURCE-only**, not the native packet's starting state.

Five predeclared full640×480 comparisons, no masks/registration/resizing:

| Native / production SDL frame | Normalized RGB error |
| --- | ---: |
| Confirm4 Yes | 0.002228592218137255 |
| Confirm4 No | 0.0022185585171568627 |
| No → Menu5 | 0.004944018927015251 |
| Yes → Map7 | 0.007833243974673203 |
| Footer → Menu8 | 0.004944018927015251 |

All pass0.02; native and source frames inspected. `compare_source.py` retains
candidates, diffs and hash-bearing scores. An initial comparison-script path
omitted the hyphen in CTest's `frontend-window-test` output directory and failed
before scoring; corrected to the actual CMake path, not a different candidate.
These averages do not establish exact font/cursor pixels or broader input fidelity.

Four independent old-behavior mutations were built and tested: session Return
alwaysYes, confirmation arrows disabled, stale confirmation image, and shell
Return alwaysYes. The first three fail both `frontendsession` and `frontendwindow`;
the fourth fails the shell/session agreement check. No assertions were weakened.
All six affected source/test files were restored byte-exactly before the final
build; logs and hash manifest are in `build/tmp/enacter-confirm-source-20261028/`.

Final UCRT64 build/retail CTest **70/70 PASS,306.13s**. The five exports were
regenerated and rescored unchanged. Earlier win/completed-map and all four fresh
loss-route exports remain byte-identical to their retained comparison evidence.
Packet replay, offline input/reader checks, documentation and diff checks pass.

Remaining: hover/physical-device equivalence/repeats, other menu actions, paused/
keyboard Resume, loss Continue/UI5, startup persistence, movies and parked timing.
This closes the bounded confirmation focus gap, not full B2 or game acceptance.
