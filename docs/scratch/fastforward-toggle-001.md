# fastforward-toggle-001: return to normal speed

Status: **needs_review**. Playable v1, human playtest P17 in
[the ledger](../playtest/2026-11-12-playtest-1.md). Difficulty: routine
implementation, difficult evidence; recommended model: Sonnet-class. This worker
uses the explicit Codex provider override. Full registered main/recovery
verification is **pending supervisor execution**, then independent review.

## Failure, route and production change

The player reported that fast-forward could be enabled but never disabled.
The accepted [implementation](fastforward-impl-001.md) deliberately latched it
for a whole level. Returning to normal speed is the next dependency for a usable
P17 control; no broader timing or skill change is required.

Installed route: `runApplication --level MM_CLIMB_002 --sim` ->
`runApplicationLeg` SDL input (`src/app/main.cpp`) ->
`runtimeSkillPanelCommand(x,y)` identifies physical plate8 ->
`RuntimeSimState::toggleFastForward()` -> existing `advanceElapsed` ->
`World::advanceElapsedMs`. Normal frames retain one pass, fast frames retain
seven passes with the identical integer-ms input. The shared stall clamp,
event slicing, pause, completion stop, release rate and simulation rules are
unchanged. The legacy `advance` wrapper and explicit `latchFastForward` enable
helper are preserved for existing fixtures.

**Provisional clone policy:** a second left-button gesture on plate8 disables
fast-forward; a third enables it again. The original's direct player cancel
gesture is not settled by the retained static evidence. A press, or dragging a
held button onto the plate, toggles once. Subsequent motion during the same
press cannot reverse it. Left release rearms the gesture without changing speed.
The existing focused/not-right-drag/menu guards still apply. Gesture bookkeeping
is reset at focus loss and menu input reset. Pause preserves speed but allows
the player to change it while paused; a new level leg starts at normal speed.

Drawing uses the same `runtimeSim.fastForward()` value ->
`drawRuntimeSkillPanel` -> `RetailSkillPanel::draw`. While fast, plate8 gets
the existing selected-plate artwork, independently of the selected actor skill.
The selected-plate appearance is also **provisional clone feedback**, not a
claim about the original artwork policy. The three-argument fixture draw remains
available and retains its output.

## Static evidence and remaining original question

No original execution, debugger, hooks, original capture or instruction oracle.
Target: repository-local `Lemmings Revolution/Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
REA/Ghidra12.1.4, analysis profile
`3c06738e45e144d5f073edbd3244c6bff46222f389525fe659bc92dca2ccdece`.
The analysis session was closed without snapshot arguments.

Re-read accepted [command research](gameplay-commands-research-001.md), its
`build/gameplay-commands-research-001/direct-recovery/{evidence.json,evidence.md}`
and `instruction_evidence.md`. New complete REA envelopes are retained under
`build/fastforward-toggle-001/static/`:

| Question | Static instruction evidence |
| --- | --- |
| Second press or hold | `0x455AFF`, `0x455B12..0x455B37`: hover8, call button predicate0x455D90; if true, load input pointer0x4CBFB0 and **store1** to input+0x20 at0x455B30. No read/inversion or clear. Pseudocode agrees with full instructions. |
| Release | False button predicate skips that store. No release-to-zero branch in this plate helper. The re-read full input dispatcher0x4532E2 has no direct input+0x20 store. This is not exhaustive alias analysis. |
| Another plate | `0x455B39..0x455B58` dispatches selection through0x45AC5C; this plate helper has no speed clear in that branch. |
| Initialization/reset | Constructor0x452BC2 stores0 at0x452D2F; reset0x455774 stores0 at0x455AF4. Full instructions cross-check both decompilations. |
| Pause/UI transitions | Retained frame-loop instructions distinguish pause byte game+0x55A0 from input+0x20. Simple unpause at0x45D502..0x45D525 writes pause/state flags without calling input reset there. Reset xrefs also include UI transitions, e.g.0x45D600 sets game+0x572C=3 and0x45D675 calls0x455774; they do clear speed through reset. This does not establish a plate-click or simple pause cancel gesture. |
| Budget | Accepted0x45DB0E..0x45DD5C reads input+0x20 and selects dword0x4B15F8=7 versus1. Preserve the accepted shared-dt integration. |

`static/evidence.json` contains new pseudocode/instructions for0x455AFF,
0x455774 and0x452BC2, global input xrefs and the full0x4532E2 pseudocode.
`dispatcherInstructions.json` contains its instruction cross-check;
`resetXrefs.json` records reset callers. Plate instruction Evidence ID:
`ev_d689fb865b8b5ed5e7b17fda9dd037c8e94881180972f3ab08a115d16a10110b`.
Reset instruction ID:
`ev_a65e51425bce0d3cc8953d052404767c0ee35ebee4f3fd3ed8c1ba5d20a2740d`.

Limit the earlier report's word **only**: the known reset paths do not prove
that every aliased writer, replay-event5 producer or UI transition was exhausted.
The static checks establish set and reset sites, not an observed original input
sequence for returning to normal speed. No new direct cancel was established;
the task explicitly authorizes the second-click fallback in that case. An
authorized original observation of click/hold/release, second click, pause/resume,
other-plate selection and level-menu return, with speed and plate-state tracing,
would settle that comparison. It was not performed here.

## Executed before/after regression

`test_fastforward_toggle.cpp` drives public SDL events into the production
application, loading the actual installed Climber tutorial without injected
actors, terrain, results or World mutation. Its owned clock supplies50ms/frame;
dummy video/audio avoid an interactive window. This is **source-built scripted
clone evidence**, not human play or original fidelity. It is registered in CTest
as `fastforward_toggle_production`, plus an optional authentic baseline.

Actions queued after frameN are processed before frameN+1:

| Frame / input already processed | Baseline passes | Production passes | Production plate |
| --- | ---: | ---: | --- |
| 0, fresh level | 1 | 1 | plain |
| 1, first plate press | 7 | 7 | outlined |
| 2, held motion across plate | 7 | 7 | outlined |
| 3, release | 7 | 7 | outlined |
| 4, second press plus held motion | 7 | 1 | plain |
| 5, release and hover | 7 | 1 | plain |
| 6, third click | 7 | 7 | outlined |
| 7, pause | 0 | 0 | outlined |
| 8, cancel while paused | 0 | 0 | plain |
| 9, resume | 7 | 1 | plain |
| 10, drag onto plate | 7 | 7 | outlined |
| 11..15, same held gesture, release, skill plate, right-drag and unfocused input | 7 | 7 | outlined |
| 16, another click | 7 | 1 | plain |
| separate fresh level leg | 1 | 1 | plain |

Passes are independently inferred from `levelClockMs` delta /50. The regression
also asserts actual `explicitUpdateCount`: passes plus one extra trailing
zero-delta update per due1000ms level event. Updates and passes are distinct,
as established by shared-timing-impl-001. No simulation clock was changed.
Every frame asserts unchanged skill stock and selection.

Actual plate pixels are classified against references drawn with the existing
three-argument panel renderer. Black/white backgrounds bound the possible RGB
range behind translucent artwork.299 plate pixels have disjoint normal/selected
ranges, allowing a scene-independent check. The trace's `drawn_fast` is measured
from those pixels, and every frame must classify unambiguously and match speed.
The baseline plate remains plain even while fast; that prior missing feedback
is retained explicitly, not described as an original rendering observation.

Final measured artifacts:

- Production: `build/agent-native/main/fastforward-toggle-production/run-1794599434397530600/`.
- Authentic baseline: `build/agent-native/main/fastforward-toggle-baseline/run-1794599435715230800/`.
- Both directories contain `trace.csv`, `summary.txt` and seven full clone PNGs.
  I inspected production off/on and baseline second-click images. In production
  the plate changes while the actor-skill highlight remains selected.

Images: [baseline second click](../../build/agent-native/main/fastforward-toggle-baseline/run-1794599435715230800/frame-4.png),
[production first click](../../build/agent-native/main/fastforward-toggle-production/run-1794599434397530600/frame-1.png),
[production second click](../../build/agent-native/main/fastforward-toggle-production/run-1794599434397530600/frame-4.png),
[production third click](../../build/agent-native/main/fastforward-toggle-production/run-1794599434397530600/frame-6.png).

## Checks and preservation

Registered UCRT64 GCC16.1/CMake/Ninja/CTest were used, `-O2 -g`, assertions on,
12 build jobs. `build/fastforward-toggle-001/regression-build.log` records the
targeted build; `regression-tests.log` records **12/12 passing in14.27s**:
sharedtiming_baseline, sharedtiming, levergates, runtimehud, runtimeskillpanel,
levelmenuwindow, fastforward_units, fastforwardimpl_identity,
fastforwardapp_before, fastforwardapp_after, fastforward_toggle_production,
fastforward_toggle_baseline. Full main/recovery and the existing scripted play
chains remain pending supervisor verification; they were built but not rerun by
this worker. No full suite was duplicated or restarted.

The initial `targeted-first.log` intentionally remains a failed development run:
the new test incorrectly assumed opaque plate pixels and one update per pass.
Corrected tests retain the same behavior requirements and account for the
existing alpha compositing and level-event trailing updates.
`targeted-corrected.log` records the subsequent two-case pass; the final measured
run above additionally classifies actual plate pixels independently of expected
mode. Earlier failures are not acceptance evidence.

Existing assertions/timeouts/counts were preserved. The fastforward unit helper's
second-enable assertions retain their values; their old **second press** wording
encoded P17 and is corrected to **explicit enable**. Added assertions require
toggle-off to advance56->58 ticks and toggle-on to advance58->72. Existing
application checks pass unchanged. The scripted playable-baseline, expansion,
targeted-play and fluid-lever drivers each enable speed once and do not click
the speed plate again; their actions/expectations need no change.

Every new application invocation receives an explicit private save store at its
unique output directory's `SavedGame.dat`. It never enables the default store.
Both test summaries assert installed-save and normal LocalAppData-save existence
and full bytes unchanged. Direct-level entry does not import or reset progress.

Authentic task-entry source copies were retained **before production edits** at
`build/fastforward-toggle-001/before-source/`, including the fullsrc tree and
CMakeLists.txt. `before-hashes.json`, `baseline-source-check.json`,
`pre-existing.patch` and retained `HEAD-main.cpp` establish provenance. The optional
baseline compiles the old main/RuntimeHud/RuntimeSkillPanel with old headers over
shared support; the old three-argument SkillPanel draw is preserved. Frozen files
were never swapped into src or refreshed from changed sources.
`worker-tested-sources.json` identifies final tested code. Task files use LF and
`git diff --check` passes. Pre-existing hook-log, queue and RUN_STATUS edits were
preserved. No protected control edits, subagents, concurrent source writers,
original launch, installation/save writes, commits or pushes were performed.

Changed task files: CMakeLists.txt; src/app/{main.cpp,RuntimeHud.h,
RuntimeSkillPanel.h,RuntimeSkillPanel.cpp,SkillPanel.h,SkillPanel.cpp};
src/__test__/{test_fastforward_units.cpp,test_fastforward_toggle.cpp}; this report.

Limitations: this bounded route checks input, budgets, artwork and reset, not a
natural win or perceived desktop speed. Human optimized-build P17 playtesting
remains necessary. The original cancel gesture/artwork, historical fidelity
gates, timing parks and full-project completion remain unresolved/unchanged.

Handoff: P17 toggle and plate feedback are implemented; 12 targeted tests pass, with before/after traces and clone images retained; status needs_review.
Next action: supervisor full registered main/recovery verification, independent acceptance review, then a short optimized human fast-forward on/off/on playtest.
