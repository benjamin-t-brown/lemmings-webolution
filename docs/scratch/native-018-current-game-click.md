# native-018: Play Current Game mouse connection

2026-10-30 implementation handoff: intended **needs_review** after successful
supervisor verification, then independent acceptance. Full verification is
currently blocked by repeated `levelreturn` timeouts (details below); the queue
is left to the supervisor. This task connects a SOURCE portable
mouse adapter to an existing semantic action; it adds no recovered native mouse
predicate or save implementation.

## Behavior and production path

Before, ordinary startup rendered Play Current Game but clicking its panel did
nothing. Now that click selects model highlight1 and calls the existing
`FrontendSession::pressReturn`; the existing `shell::Frontend::press(Return)`
replaces Menu with Map. Progress and the already prepared map remain owned by
the same session. New Game still selects highlight0 and opens confirmation;
only the existing Yes action clears session progress.

The traced path is `main` -> `runApplication` (one persistent session) ->
`runApplicationLeg` -> `runFrontendWindow` -> focused left-button-down ->
`FrontendSession::click` -> `pressReturn` -> `shell::Frontend::press` ->
`replacePage(Map)`. A subsequent map-node click uses `clickMap`, sets the selected
level and returns it through `runFrontendWindow`. `runApplicationLeg` assigns
that selected name, enables simulation, sets diagnostic ticks to zero, and uses
the existing `sim::loadRetailLevel` and fresh `World` construction. No alternate
event route, session lifetime or loader is introduced.

Production changes are confined to `src/app/FrontendSession.cpp` and its private
declarations/comments in `FrontendSession.h`. `create` retains exactly two
rectangles, using the loaded UK labels, font cells and the same `MenuParams`
passed to the menu renderer. Each uses labelLeft-pad through
labelLeft+textWidth+pad, and baseY+row*rowStep+panelTop through top+panelHeight.
All bounds are half-open. The resulting UK targets are:

| Row | Loaded label / width | Horizontal | Vertical |
| --- | --- | --- | --- |
| 0 | Start New Game / 166 | [218,422) | [321,348) |
| 1 | Play Current Game / 191 | [206,435) | [350,377) |

These numbers are test expectations and documentation, not a production table.
Row0 preserves its previous footprint exactly. A hit selects the existing
model highlight and invokes Return; a miss does not mutate focus or page.
Confirmation, map, results, retry, level menus and mouse motion retain their
previous implementations. The menu image cache remains a cache of model focus.

## Evidence classification

The retained [menu navigation note](frontend-menu-focus-navigation-2026-10-28.md)
identifies UK126 and observes row1 Return producing Menu3/UI3 -> Map4/UI3. Its
mouse-selection exclusion remains valid. It is semantic evidence, separate from
the new mouse policy. `src/shell/MenuRender.h/.cpp` establish current panel
geometry, including the retained measured vertical values and their limitations.
No binary analysis was needed for this connection. No new native arithmetic,
calling convention or layout is asserted; the new adapter calculations are
ordinary integer panel coordinates.

Reference identity was checked by reading bytes only:
`Lemmings Revolution/Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.

## Authentic public-session regressions

Before production edits, authentic sources were frozen under ignored
`build/native-018/FrontendSession.before.cpp` and `.before.h`, also retained
under `frozen/app/` for isolated header lookup. Their SHA-256 values are:

- cpp: `2bd640752a354bbf9272979b51067c6505fbb031caaf03c23bd2caf2077a7f49`
- header: `3abea40f1b61b6715ddcdc5b7a7ed76d722e3e1027c5ea57eac26198cea5de67`

`session_probe.cpp` was separately compiled against frozen/current session
implementations and matching headers, linking the unchanged source-built
support objects/libraries. Production files were never swapped. The public API
probe independently requires Map, highlight1, one construction increment, no
selected level, preserved 102 flags/states and the prepared map image after
clicking visible row1 centre320,363 from prior keyboard focus4. The fresh case
starts on Menu3; the nonzero case uses an **injected LevelOutcome as source-test
setup**, then Results Continue and the map footer to Menu9. This is neither a
natural gameplay victory nor additional native evidence.

| Source / setup | Actual click and page | Counter | Normal exit / elapsed |
| --- | --- | --- | --- |
| Frozen / fresh | false, Menu, focus4 | 3 (expected4) | 1 / 0.185s |
| Current / fresh | true, Map, focus1 | 4 | 0 / 0.166s |
| Frozen / progress | false, Menu, focus4 | 9 (expected10) | 1 / 0.249s |
| Current / progress | true, Map, focus1 | 10 | 0 / 0.229s |

All four terminate normally within their eight-second failure guards; the
frozen exit1 is the independently expected assertion failure, not a crash or
timeout. Flags/states are unchanged in all four. These direct public-session
drivers have zero frontend/level observer callbacks, explicitly reported.
Commands, compiler/link inputs, identities and logs are in
`before-commands.json`, `after-commands.json`, `*-source-identities.json` and
`*-fresh.log` / `*-progress.log`. Current production identities are also in
`preservation.json`.

## Executed source checks and discrepancies

Existing tests were extended additively; every prior byte outside the added
blocks matches its task-start hash. Existing assertions and timeouts remain.

- `frontendsession`: **660 checks, zero failures**, CTest 2.00s. Both supported
  centres, inclusive left/top, last interior pixels/corners, exclusive
  right/bottom, adjacent exterior pixels, both inter-row gaps, unsupported
  rows2-4 and distant exterior are checked from all five prior focus values,
  with fresh and nonzero progress. Checks cover return value, page, highlight,
  counter, completion, map states, selected level and prepared map image.
  Additional New Game/No/Yes checks retain progress until Yes.
- `frontendwindow`: **PASS, 2.64s**. New dummy-SDL fresh/progress routes each
  produce six observer callbacks and one consumed observer-queued SDL_QUIT;
  counters finish4/10. Motion over row1 keeps focus4. Right-down/up, left-up,
  repeated Return and unfocused clicks/keys/motion do not activate it. Focused
  left-down reaches the prepared Map with flags/states unchanged; release
  clears held state. Existing keyboard, confirmation, focus, cursor, results
  and frame-cap scenarios remain intact.
- `frontendentry`: **PASS, 27.07s**, retaining its 120s timeout and previous
  render/reset checks. An additional ordinary no-option startup clicks row1,
  reaches Map4, selects another available node (`MM_FLOAT`), and observes its
  fresh retail simulation with matching stock/setup/terrain, zero saved/dead,
  default selected skill and counter4. It reports two frontend callbacks,
  one level callback and one SDL_QUIT, then returns0. The production clock stays
  real; this is not a new timing comparison or deterministic tick0 assertion.

The first focused CTest run failed only the new session exterior expectations
(30 failures); window and entry passed. I had used the shortened name New Game
to calculate row0 width107. A read-only UK dump established the actual label
Start New Game and width166. Production already derived the correct bounds.
The corrected independent fixture now also checks both complete label strings.
The failing log/JUnit remain at `ctest.log` / `focused-results.xml`; the passing
session rerun is `session-ctest.log` / `session-focused-results.xml`. Two earlier
probe linker setup failures (command parsing, then omitted support libraries)
are retained as `initial-link-failure*` and `second-link-failure*`. No production
behavior or prior assertion was changed to resolve these harness mistakes.

Actual focused build/CTest commands and external failure guards are recorded in
`focused-commands.json` and `session-focused-commands.json`. Tools come from
`agent/tools.windows.json` (UCRT64 MinGW-w64 GCC16.1.0); dummy video/audio are
set process-locally. There were no successful timeouts or forced terminations.
The initial handoff delegated `python tools/agent_control/verify.py --profile all`
to the existing supervisor. That executed check failed: **69/70 main tests
passed**, but unchanged `levelreturn` timed out at **240.03s**; total main CTest
time was399.53s. Recovery was not reached in that run. Its full log remains at
`build/agent-runs/20261030T202350-40868-9d0bf425/verification.log`. The three
focused tests passed there too (session1.98s, window2.68s, entry29.25s).
This worker's focused checks do not replace full main/recovery verification.

## Retry inspection and verification blocker

The assigned retry preserved all implementation/test source bytes and the
authentic frozen/current regression artifacts. Static review traced the real
session/SDL/model/map/retail-simulation path again and found no further bounded
production change needed for the assigned connection. No replacement source
snapshots or reconstructed "before" implementation were made.

One serial diagnostic rerun used the registered CTest with dummy SDL and the
unchanged test registration: `python build/native-018/retry_checks.py levelreturn`.
Its exact command, output and JUnit are in ignored `build/native-018/retry/`.
`levelreturn` again **timed out at240.02s** (JUnit240.017s); CTest exited8 after
240.056s. The outer260s failure guard did not fire. The command record's
`normal_exit: true` describes the CTest process only: the source-built test was
terminated by CTest's existing240s limit and did **not** exit normally or pass.
This failure is not an expected-success timeout. There is no fresh callback-count
success claim for that incomplete run.

For context, the preceding native-017 supervisor run had passed the same test
in231.68s, with70 main/84 recovery passes, in
`build/agent-runs/20261030T200540-40868-62bb3786/verification.log`. This is a
historical baseline, not a native-018 full pass. The failing test takes the
New Game keyboard route and does not activate the new row1 target. Its cause
remains unknown; proximity to the earlier limit does not establish a performance
explanation or prove this change unrelated. No assertions, frame sweeps, build
settings, renderers or timeouts were changed to obtain a pass. Local test reruns
stop after this repeated failure as required. No original execution was used.

The retry changes only this focused note and ignored diagnostic/report files.
`retry/preservation.json` records comparison against the retry-start snapshot,
the unchanged index, original implementation hashes and installation hashes.
That check passed for all1,624 captured tracked files, the five implementation/
test sources, frozen sources/driver and all63 installation files; `git diff
--check` also passed. The preservation helper initially raised `KeyError` because
it assumed the untracked queue was in the tracked-file snapshot. It was corrected
to record a finish-time queue digest and explicitly disclose the missing initial
digest, without inventing a comparison. No queue edit was performed.
The external supervisor is instructed to run the registered full verifier after
worker exit; that result must be assessed independently of these focused passes.

## Preservation, limits and next action

`build/native-018/preservation.json` compares all 1,624 task-start tracked files:
only the two production files and three existing test files changed, none are
missing, and the Git index is byte-identical. This note is the only new tracked
candidate. All 63 installation files retain their initial hashes. Queue entries,
native-006 deferral, STATUS, recovery PROGRESS/FUNCTIONS, the 372 / 6,186 baseline,
historical findings/failures and D1/D3/digger parks remain untouched. Read-only
CTest inventories retain70 main /84 recovery, 120s window/entry and240s
levelreturn limits, empty recovery `LEMMINGS_REFERENCE_EXE`, and
`native_int_array` as its sole native_-prefixed source test. Changed text is LF.

Native mouse predicates, hover selection, rows2-4 actions, persistent saves,
physical input, audio, full frontend fidelity, timing and full-frame acceptance
remain unverified or outside scope. No original executable or copied machine
instructions were executed. No oracle, hook, debugger, capture, original launcher,
installation modification, commit or push was used. Renderers, simulation, archives, audio,
saves, `FrontendWindow.cpp`, shell navigation, main and build registration are
unchanged.

Next: the supervisor runs registered full verification. If `levelreturn` still
times out, retain the blocker and decide a separate diagnosis within its own
scope; do not weaken this task's acceptance or test limits. Only after full
verification passes should a separate read-only acceptance review check every
native-018 criterion and the real production path. This worker leaves queue
mutation to the supervisor. Any later native mouse comparison requires
separately authorized original execution.
