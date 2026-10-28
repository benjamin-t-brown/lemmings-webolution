# Confirmation No and Resume — predeclared connected comparison

## Prediction, before any new launch

Use one ordinary startup process and the retained guarded shim input path. The
missing observation is whether **confirmation No returns a fresh row0 menu, and
an in-level Resume click destroys the frontend without constructing a new page
or changing the selected level/progress**. No previous authorization is reused.

Predicted route (counter / UI / page):

| Action | Counter | UI | Page |
| --- | ---: | ---: | --- |
| Startup | 3 | 3 | Menu, `4B2528`, structural row0 |
| RETURN on New Game | 4 | 3 | Confirm, `4B3F38` |
| Click No at320,340 | 5 | 3 | Menu, structural row0 |
| RETURN, then Yes RETURN | 6,7 | 3 | Confirm, then Map `4B3EB8` |
| Click first node206,204 | 7 | 0 | `MM_CLIMB_002`, frontend list empty |
| ESCAPE | 9 | 2 | In-level menu, `4B26B0` |
| Click Resume at320,112 | 9 | 0 | Same level, frontend list empty |
| ESCAPE | 11 | 2 | In-level menu |
| Click Quit at320,170 | 13 | 3 | Map |
| Click footer320,455 | 14 | 3 | Menu, structural row0 |

Record all102 completion DWORDs using the retained correct addresses before
confirmation and after No; equality is predicted. This does not prove preservation
of nonzero progress if the baseline is all zero. Yes establishes the already
observed fresh game: flags all0, map states `[3,3,3]+[1]*99`. Level entry, Resume
and Quit must preserve those flags; the returned map must preserve all102 states.

Any differing page/counter/UI, changed level path, unexpected flags, invalid
structural focus or failed input consumption falsifies/refuses the predicted
route. Stop without further input and preserve failures. No loss/outcome is
forced; no actor/grid/timer/progress memory writes, assignments or timing probes.

Matched frontend images: initial menu, confirmation, post-No menu, fresh map,
post-Quit map and final menu, all unscaled640×480 with pointer320,240 and no held
buttons. Compare actual-app route exports with full-frame normalized absolute
RGB difference, threshold0.02, no masks/registration/resizing. Additional menu
frames are context only: in-level scene/animation timing is not matched or scored.
Resume is a UI/page/level identity observation, not simulation-clock acceptance.

## Access and result

Prepare offline, then request one exclusive desktop window of at most five
minutes. Reuse the successful owned-job envelope and structural reader; preserve
save/config, verify sole stock process/foreground/client geometry, and close and
independently verify exact restoration before releasing the desktop. Loss/retry,
paused-origin Resume, keyboard Resume and broader physical-input equivalence
remain separate.

## Observed native result

The user granted the single window at20:15:32Z, hard deadline20:20:20Z. One owned
stock PID22172 completed **the entire predicted sequence**, through No Menu5,
Resume Level9, reopened menu11, Quit Map13 and footer Menu14. Packet:
`build/tmp/enacter-native-no-resume-20261028/`. The retained owned-process
envelope and structural reader were reused unchanged; no new launch mechanism.

All nine images were visually inspected. Initial/post-No/final menu images are
byte-identical, both confirmation images are byte-identical, and the initial/
returned map images are byte-identical. Each frontend image has stable page/UI/
cursor brackets; all main-menu images have structural focus0. Ten balanced input
records preserve before/after identities and shim consumption. The offline
`verify_packet.py` checks images, raw structural replay, input route, flags,
record states, executed-source hashes and restoration receipts.

**No preserved a nonzero baseline:** the102 initial completion DWORDs were
`[1]+[0]*101` and remained identical after No. This is stronger than the possible
all-zero equality case anticipated above. Yes then reset all102 to0; map states
were `[3,3,3]+[1]*99`. Entry, Resume and Quit retained those zeros, and the returned
map retained all102 fresh states. The installed save itself was not reset by the
agent; any native changes were restored exactly after shutdown.

The pre-Yes source startup currently has zero in-memory flags because retail save
loading is not implemented. Thus the forthcoming fresh-app comparison can match
screen/input/image behavior and post-Yes flags, **not the pre-Yes progress
baseline**. Do not inject a synthetic startup flag and call that normal runtime
parity. Existing nonzero No-retention source tests remain separate evidence.

Resume restored UI0, frontendflag0, empty page list, counter9 and the identical
`Compiled\\MM_CLIMB_002.lvl` path, without a reload request from the harness. No
actor/timer sampling or simulation-fidelity claim. In-level frames are context:
the intro/camera advanced between them and the second menu image highlights
Options after the shim pointer path. No menu-hover or initial-keyboard-focus
equivalence is inferred. Native cursor after Resume was9, a retained handoff
observation, not a source cursor acceptance test.

Owned process closure and input restoration succeeded. Independent verification
at **20:16:22.444489Z** found no stock process and exact save/config/exe/shim hashes.
The desktop was released immediately. No further launch authority or restoration
debt. Native loss/retry/Quit, paused-origin Resume, keyboard Resume, persistence
and broader C2 remain open.

## Connected source comparison and checks

`test_levelmenuwindow` now runs the added route through `app::runApplication`,
without a synthetic outcome or substituted frontend fixture. SDL input drives
No, Yes, level selection, mouse Resume, reopening, Quit and footer. Per-stage
assertions check the construction sequence and owned progress. Resume also
advances the source simulation after pause; that is not a native-clock comparison.
Exports require pointer320,240 and both buttons released. The earlier Menu3→
Map5→menu7→Map9→Menu10 trace and all three of its frame exports remain exercised.

Six predeclared full640×480 comparisons, unchanged0.02 metric, no exclusions:

| Native frame / actual-app counterpart | Error |
| --- | ---: |
| Menu3 | 0.004960873672385621 |
| Confirm4 | 0.002245446963507625 |
| No → Menu5 | 0.004960873672385621 |
| Yes → Map7 | 0.007837979983660131 |
| Quit → Map13 | 0.007837979983660131 |
| Footer → Menu14 | 0.004960873672385621 |

All pass. Source images were inspected; identical menu/map counterparts share
the same pixels. The final rebuilt exports are byte-identical to the scored
candidates. The three earlier abandonment-route exports also remain byte-identical
to their previously scored evidence. `compare_source.py`, score/diff files and
`final-source-manifest.json` retain the comparison and final source hashes.

Review corrected the first test draft's unbalanced No click and unestablished
pointer on post-No exports, and retained the earlier route rather than leaving
old files in the output directory. Those draft exports were not used for scores.
The worker's assertion-only mutations are not behavioral mutation evidence.
Separate parent mutations in production code—route No to Map, or suppress only
mouse Resume—each fail the new trace (`no-route-mutation.log`,
`resume-click-mutation.log`). Both implementation files were restored byte-exactly.

Final UCRT64 build and retail CTest: **70/70 PASS,239.59s**. Offline packet replay
and documentation/LF checks pass. No production behavior correction was required
for these branches; the change extends connected regression and native evidence.
No commit/push. Before-Yes progress baseline mismatch and all limitations above
remain explicit; this is not full B2 or C2 acceptance.
