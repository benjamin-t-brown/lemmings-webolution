# Natural loss / Retry / loss / Quit — predeclared comparison

## Prediction before launch

Use one ordinary New Game process, the first map node and **no skill assignments,
pause, fast-forward, timer/actor/progress writes or forced outcomes**. Let
`MM_CLIMB_002` reach a natural loss, Retry it, then lose naturally again and Quit.
This is a result/return-route comparison, not reopening D1/D3/digger timing.

| Transition | Counter | UI | Page / level |
| --- | ---: | ---: | --- |
| Startup | 3 | 3 | Menu `4B2528`, structural row0 |
| New Game / Yes | 4,5 | 3 | Confirm `4B3F38`, Map `4B3EB8` |
| First-node click206,204 | 5 | 0 | `MM_CLIMB_002`, empty frontend list |
| Natural loss | 7 | 5 | Loss results `4B1CB0`, saved0 / needed1 |
| Retry click320,360 | 7 | 0 | Same level, empty frontend list, intro preference0 |
| Second natural loss | 9 | 5 | Loss results, saved0 / needed1 |
| Quit click320,420 | 11 | 3 | Map, intro preference1 |
| Footer click320,455 | 12 | 3 | Menu, structural row0 |

Yes resets all102 completion DWORDs to0. They must remain zero throughout the
two losses and Retry/Quit, with returned map states `[3,3,3]+[1]*99`. This does
not test loss preserving an already-completed node. Source startup save-loading
remains separate; comparison starts with the common post-Yes fresh state.

Saved/needed display sources are **DWORDs** at `5D5994/5D5990`, not the signed
controller words they came from. Static reads: `4454A2:0fbf889c500000` sign extends
saved; `4454A9:890d94595d00` stores the DWORD; `4454BE:0fbf88a8500000` and
`4454C5:890d90595d00` do the same for needed. Loss builder reads DWORDs at
`4863AC:8b0d90595d00` and `486443:a194595d00`. Intro preference `4E3584` is a byte.
Retry request `4CBFAC` and reload-mode `5D5A0A` are distinct bytes; transient
request consumption is not an assumed stable post-load value.

## Waiting and failure conditions

Expect roughly two five-minute levels, but **do not score a completion time**.
Allow at most360seconds per result wait and a13-minute exclusive desktop window
including startup/cleanup. Native pending-end dispatch and `failed.avi` can precede
the result page; wait for UI5 plus the expected stable page/counter, not a fixed
assumed delay. No movie-skip input or new launch if this fails.

Record bounded UI/page transitions during passive waits, not actor trajectories
or per-tick clocks. Pointer centering at320,240 is permitted after level entry/
Retry so neither result is approached through its button rows. It is not a skill
assignment or camera command. Use the retained hidden-worker input/ownership
guards for input; require unobstructed640×480 geometry for all screenshots.

Unexpected win, changed path/flags, wrong settled page/counter/UI, unusable result
counts, unsafe ownership/foreground, missed input consumption or a wait deadline
falsifies/refuses this bounded route. Stop and preserve all artifacts; no blind
follow-up input or forced result. Verify owned shutdown and exact installed
save/config/exe/shim restoration independently before releasing the desktop.

## Predeclared image and application comparison

Compare the two loss-result frames, returned map and final menu with actual-app
SDL exports on the corresponding fresh-loss route, pointer320,240/no buttons
held. Full640×480 normalized absolute RGB difference ≤0.02, no masks, registration
or resizing. Intro/gameplay frames and timing are not scored. Result hover/focus,
movies and physical-device equivalence are not inferred from the average score.
Preserve the existing natural-win/completed-map regression; do not replace it or
substitute fixture results/progress for this new runtime route.

No fresh authorization or native result yet. Prepare offline, then request the
exclusive window. Earlier launch grants are spent.

## First attempt: stopped before any loss

One authorized process PID9860 ran in the window20:54:03Z–21:06:55Z. Packet:
`build/tmp/enacter-native-loss-retry-20261028/`. Menu3, Confirm4, fresh Map5 and
Level5/UI0 entry were recorded. The newly added **gameplay** `aim_exact320,240`
then failed: `RuntimeError('Cursor not exactly at (320, 240); refusing input')`.
No loss, Retry or Quit was observed; no result frame exists. Do not score the
source loss exports against fixtures or call this a native route pass.

This was a capture-script error, not a demonstrated game bug. `drive.cursor`
reads frontend integer globals `4C36A0/A4`; retained `assign_skill.py` documents
the gameplay cursor doubles at `[4CBFB0]+16A0/16A8` and their writers. The helper
sent relative mouse movements while reading the wrong feedback path, then its
guard stopped further input. No skill click, fast-forward, actor/grid/timer or
completion write was sent. The failed attempt is not a no-pointer-input gameplay
measurement and says nothing about native loss timing.

Owned shutdown/input restoration succeeded; independent verification at
**20:54:56.396503Z** found no stock process and exact save/config/exe/shim hashes.
The desktop was released. Its one-launch grant is spent; all artifacts remain.

## Amended follow-up prediction, still before a fresh launch

The page/counter/count/progress predictions and four image comparisons above
are unchanged. **Remove gameplay pointer centering altogether.** The first
pointer movement after either loss wait is permitted only after UI5 and the
expected result page are established. Add a frontend-flag guard before every
frontend aiming call. No new gameplay pointer controller is needed for this
result-route task. Centered result screenshots do not imply identical hover/focus
history; retain that limitation even if the frame average passes.

Use a new packet and one newly authorized process. No input during either natural
loss wait; same360second per-loss bound and13-minute exclusive window. This is
the smallest remedy for the failed observation, not a capture-framework redesign.

## Successful follow-up observation

A fresh one-launch grant covered21:32:14Z–21:45:05Z. Packet:
`build/tmp/enacter-native-loss-retry-20261028-b/`; owned stock PID15264. The
mode-guarded frontend helper passed its offline refusal test. No pointer, key,
skill or speed input was sent during either gameplay wait.

**The full predicted page/counter route was observed:** Menu3→Confirm4→Map5→
Level5→loss Results7/UI5→Retry Level7/UI0→loss Results9/UI5→Quit Map11/UI3→Menu12.
Both results display saved0/needed1; their DWORD bytes and mirrors agree in
stable brackets. Intro preference is1 at entry,0 after Retry and1 after Quit.
Retry preserves the loaded `Compiled\\MM_CLIMB_002.lvl` path and has no frontend
page. All102 post-Yes completion flags remain0; returned map states equal
`[3,3,3]+[1]*99`. Nonzero completion retention on loss was not exercised.

Seven native images were inspected. Both loss images are byte-identical, as are
the initial/returned map and initial/final menu. Six balanced input records and
six raw structural main-menu replays corroborate the route. `verify_packet.py`
checks those records, count bytes, progress/state arrays, image/source hashes,
intro preference and both attempts' restoration. Native end-trigger cause and
clock were **not sampled**: these are natural no-assignment losses, not proof of
the precise timeout/pending event model. No movie playback/latency claim.

Owned process/input cleanup succeeded. Independent verification at
**21:43:33.252922Z** found no stock process and exact save/config/exe/shim hashes.
The desktop was released before offline comparison work. Both grants are spent;
the failed first attempt remains preserved separately, not relabeled a pass.

## Connected application and image comparison

`test_levelreturn` retains its earlier natural win/completed-map/retry coverage
and adds a separate fresh `app::runApplication` route. It assigns no skills and
injects no result/progress fixture. The existing2-second frame seam accelerates
only the source test. Assertions cover Menu3/Confirm4/Map5, Level5 and Retry
Level7, both pending timeout outcomes, Results7/9, Map11/Menu12, independent
native fresh-map state values, zero flags, fresh Retry stock and disabled intro.
Composed exports require SDL-delivered pointer320,240 and both buttons released.

Four predeclared full640×480 comparisons, no masks/registration/resizing:

| Native / actual-app frame | Normalized RGB error |
| --- | ---: |
| First loss Results7 | 0.0033732681440631807 |
| Retry loss Results9 | 0.0033732681440631807 |
| Quit Map11 | 0.007829214324618736 |
| Footer Menu12 | 0.0049483506944444445 |

All pass0.02. Native and source frames were inspected; exact font/cursor pixels
and broader hover/input remain outside this bounded average-score result.
`compare_source.py` retains candidates, diffs and hash-bearing scores. Final
rebuilt candidates are byte-identical to those scored; the earlier natural-win
and completed-map exports remain byte-identical to their retained evidence.

Final UCRT64 build and retail CTest: **70/70 PASS,317.81s**. The added source route
fits the existing240-second `levelreturn` timeout; no timeout was relaxed. No
production behavior fix was needed in this package. Offline packet, documentation
and LF checks pass. No commit/push. Loss Continue/UI5 retention, paused/keyboard
Resume, wider input/hover, startup persistence, movies and C2/C3/D timing remain
open. The B2 blanket RETURN clause remains contradicted, not silently rewritten.
