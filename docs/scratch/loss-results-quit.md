# Loss-results Quit and retry intro preference (2026-10-28)

Offline static READ/DERIVED follow-up to [results/map](level-results-map-return.md).
No original process, desktop, registry or installed save was used or changed.

## Evidence and scope

Packet: `build/tmp/enacter-results-quit/findings.md`, raw assembly/reference scans
and `verify.py`. Main independently reran the passing checker and inspected the
loss handler, main dispatch, loader and map handler. Portable retained checker
`python -B tools/harness/read_level_results.py` now checks35 raw anchors and6
doubles, without scratch imports. All are static assertions against the stock
SHA256, not native observations or a pixel/input acceptance gate.

Loss486510 widget+4C writes4CBFAA=1 and4E3584=1. Main45D72F consumes the request,
sets UI3, repeats cleanup, then constructs **map selector23** at45D7D8. It does
not select menu7 or set application exit4E3581. Continue instead synchronously
constructs map23 through4865AC without these writes; its native enclosing UI may
remain5. The clone retains distinct semantic actions but abstracts both as Map.

4E3584 is **next-load intro preference**, not a routing flag. Initialization and
successful map activation49B1CC set it; Retry48656F clears it; Quit48658A sets it.
Loader45CAC4 copies it to camera-intro-active. The same loader seeds title clock
3.5 and, when the preference is zero, subtracts2.0 (yielding1.5) and disables
camera intro. This change gated the existing bounded first-level camera move;
the subsequent [entry-cue implementation](level-entry-start-cue.md) also uses
the preference for its independent3.5/1.5 clock. Other-level camera intro support,
physical input, shared fades and native transition frames remain open.

Native Quit is consumed on the next active main-loop dispatch; Continue and the
map footer build their replacement synchronously. The SOURCE click adapter
implements the destination immediately; no claim of matching displayed latency
or an invented fade/countdown is made.

Map footer49B242 selects **menu7**. Map cancel49B1F3 separately sets the real
application-exit latch. Neither generic Escape nor results Quit is treated as
that cancel operation. New Game No preserves flags; Yes calls the native reset,
but the clone only resets its own memory and never invokes stock serialization.

## Implementation

- `ResultAction::Quit` and loss-only `Frontend::activateResultsQuit()` replace
  the former unsupported click. Win results have no Quit button/action.
- Session Quit retires selected level/node and old result ownership, retaining
  the current completion flags and prepared map. It neither reloads nor exits.
- Native5D59F0 can remain populated without a launch: independent5D5A0A controls
  reload. Clearing the clone's selected level is a launch-request invariant,
  not an asserted native pointer write.
- `levelIntroEnabled()` is false on Retry and true on Quit/map activation.
  Continue leaves it alone. Main uses it when constructing `CameraIntro`; a
  fresh Retry still resets World, skills, clock and pending-end transition.
- Default direct-level/headless behavior is unchanged.

## Regression coverage

Session/result tests cover loss-only shell guards and hit boxes, completed-node
loss/Retry/Quit, stale completion rejection, fresh different-node ownership,
Continue preserving the disabled preference and map selection re-enabling it,
footer/Menu and New Game No/Yes progress behavior.

SDL dummy frontend uses a known-value win/loss fixture to test the actual click
dispatcher through Quit→Map→footer→Menu→New Game No→Yes→fresh selection. The
actual application test rescues ten, continues to map, replays to timeout,
Retries with a fresh simulation and no camera intro, times out again, Quits to
the unchanged completed map, and reselects to verify the intro returns. The
read-only observer queues SDL events; it does not mutate World or fake outcomes.

Tests demonstrate SOURCE implementation behavior, not native visual/timing
fidelity. Full build, mutation sensitivity and restoration results follow below.

Targeted baselines pass: `results`0.75s, `frontendwindow`0.74s and
`levelreturn`106.15s. Initial review corrected test-only shell setup: results
cannot be shown directly from Menu, and rejected semantic actions return Ignored,
not Unobserved. Production guards were not weakened to satisfy the tests.

Four isolated regressions were rejected: consume Quit without leaving Results;
enable intro on Retry; omit re-enabling on map activation; bypass the preference
in the actual application's camera construction. The final mutation fails the
actual application test at Retry, not merely a model assertion. Production
restoration is checked against `pre-mutation.sha256` in the packet.

Restored full build/CTest: **66/66 PASS**,219.65seconds; `results`0.74s,
`frontendwindow`0.71s, `levelreturn`115.80s. Log:
`build/tmp/enacter-results-quit/full-suite.log`. No native timing/image acceptance
is implied by these SOURCE passes.
