# First connected native frontend comparison — 2026-10-28

**Observed:** one owned stock process traversed menu → new-game confirmation →
fresh map → `Compiled\MM_CLIMB_002.lvl`. Four unscaled640×480 client captures
are retained in `build/tmp/enacter-native-20261028/`. This closes the missing
native capture for this route, **not** all B2/B3 or first-level fidelity gates.

## Capture and restoration

The user granted a10–15minute exclusive desktop window at approximately04:38:45Z.
The worker's fixed expiry was04:51:45Z; its supervisor allowed100seconds, with
no restart. Read-only preflight at04:45:06Z found no stock process and authenticated
the authority SHA256. The dated worker claimed its attempt exclusively before
config/launch effects. It did not invoke any historical launcher's main or attach
to a process selected by name. Old parked allowances were not reused.

`capture_once.py` and `owned_process.py` are frozen in that packet with a source
manifest. The supervisor backed up `SavedGame.dat` and `dgVoodoo.conf`; the worker
changed only `FullScreenMode=true` to false temporarily. It created the stock
child suspended, assigned it to a kill-on-close job, then resumed it. The hidden
worker itself was similarly owned by a supervisor job, covering descendant
termination even before the inner job assignment. Harmless Python-child checks
covered ordinary termination, natural exit, injected start failures and worker
death with a nested child. These checks are not native behavior evidence or a
general crash-recovery certification; simultaneous supervisor death/power loss
would still require restoration from the retained backups.

Owned game PID31676; hidden worker PID49484. Foreground acquisition used the
reviewed helper and its balanced ALT fallback, not input-queue attachment. Each
capture checked foreground, exact client bounds `(10,0)-(650,480)`, absence of
intersecting visible higher-z-order windows, nonblank image and stable screen
list/counter before/after. Main-agent image inspection confirms game pixels,
English labels and no visible occlusion. OS rounded client corners remain in
the full images; **no masks, alignment shifts, resizing or exclusions** were used.

Menu highlight was located by three DOWN differential samples, requiring exactly
one candidate, then driven/read to0 before Return. Inputs use the installed shim,
not physical-button validation. Map entry required an exact `(206,204)` cursor
read before button0 and the expected loaded path afterwards.

The worker restored its input snapshot and verified the stock process handle
signaled on termination. The supervisor verified no stock process remained,
restored save/config and checked all four hashes (save, config, exe, shim).
Independent main-agent rereading at04:46:49Z confirmed all hashes and no stock
process. The user was told the desktop was free. **One actual stock start; no
relaunch; no further desktop authority inferred.** See `restoration.json` and
`worker-result.json`. Save backup is retained and must not be deleted casually.

## Observations

| Capture | Counter | Screen list | Other evidence |
|---|---:|---|---|
| `menu.png` | 3 | `0x4B2528` | highlight0, UI3 |
| `confirm.png` | 4 | `0x4B3F38` | Continue? / Yes / No, UI3 |
| `map.png` | 5 | `0x4B3EB8` | UI3;102 raw flags all0; states0–2=3, remaining99=1 |
| `level-entry.png` | 5 | empty | UI0; loaded `Compiled\MM_CLIMB_002.lvl` |

Map records and flags were read before the image and checked unchanged afterwards;
binary snapshots and JSON values accompany the screenshot. This supplies **one
fresh unlock-state reference**, not the required two-state B3 gate.

The entry image was taken about2seconds after the map click. No simulation tick
or camera snapshot was acquired, so it is **not** a tick0/pose-matched rendering
oracle. It proves the selected-level handoff and provides qualitative discrepancy
evidence. Neither a level-return route nor confirmation-No activation was exercised.

## Offline comparison

`FrontendRenderDump <game-dir> <menu|confirm|map> <out.png>` renders the exact
production `FrontendSession`, without SDL or native interaction. Fresh source
frames and full-frame differences are in the same packet. Comparison uses
`tools/harness/compare_camera_frame.py --roi 0 0 640 480`; normalized absolute RGB
error, threshold0.02, with no exclusions:

| Screen | Initial error | Scope |
|---|---:|---|
| Menu | 0.005244204452614379 | this highlight0 frame passes |
| Confirmation | 0.002384191176470588 | this initial-Yes frame passes |
| Fresh map | 0.01654637288943355 | this one unlock state passes |

The map difference visibly contains the missing **Back to Main Menu** footer
and cursor. Passing the area-averaged score does not excuse missing controls.
The confirmation difference includes cursor, glyph-edge/blending and small
backdrop residuals; the recovered heading/art/layout is supported by the frame.

The actual application's retained first-tick entry image and native entry image
show substantially different starting views. Current `CameraParams` defaults are
a canonical diagnostic view, not yet established as retail startup state. This
is a lead to trace initialization, **not permission to tune yaw to a screenshot**.
Time/pose are unmatched, so no numerical level score is claimed here.

**Resolved lead:** the [subsequent static trace and implementation](level-entry-camera-intro.md)
finds the missing three-second opening camera move. Base yaw32 is supported;
do not change it to fit this unmatched screenshot. The first-level frontend
path now includes the opening move, without altering settled fixture defaults.

An initial render-dump shell command lost quoting around the game directory and
printed usage after a successful build; no images resulted. A small retained
`render.sh` then invoked the tool correctly. This did not involve another native run.

## First correction: the map footer

Static follow-up recovered the footer rather than fitting it to the image:
`49AC6B` sets shared layout base340; `49AC90` calls existing row builder488A92
with one UK127 label from4C44E0. The shared100-pixel offset gives text Y440.
The existing renderer therefore produces border `[205,441,436,468)`; all512
border pixels agree exactly with the retained native screenshot. `49B1E4`
routes event0 from `[map+40]` to a new selector7/main menu. Raw checks/REA
records and the complete recipe are in `build/tmp/enacter-map-footer/`.

Production `FrontendSession` now composites that row above the unchanged
`MapScene` base, and its SDL adapter activates it by click or Return. Tests
check all border pixels, selected fill/text, outside-hit rejection, screen
counters, renewed New Game, and the unchanged first-node entry. The semantic
route is statically read; physical footer activation was not observed during
the native run. Cursor/hover presentation is still missing.

Full-frame error against **the same native image**, without changing masks or
registration, improves **0.01654637288943355 → 0.008217405364923748**. Before/after
source frames and differences are separate, preserving the original baseline.
Removing the row fails the new pixel tests; routing it back to Map instead of
Menu fails both session and SDL tests. Production files were restored byte-
exactly after those isolated mutations (`pre-mutation.sha256`).

Restored build and full CTest: **55/55 PASS**,95.39seconds
(`build/tmp/enacter-map-footer/final-tests.log`). The retained capture packet's
source/image hashes, state sequence, flags and restoration receipts also pass
`python build/tmp/enacter-native-20261028/verify_packet.py`.

The first SDL regression run correctly exposed an outdated test expectation
that an extra map Return did nothing. It now tests suppression of a repeated
Return and separately exercises the footer's new click/Return routes. A shell
regex quoting error before that run was rerun with separate test commands;
neither issue caused native effects.
