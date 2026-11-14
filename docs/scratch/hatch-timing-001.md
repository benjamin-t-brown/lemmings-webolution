# hatch-timing-001: entry text, hatch doors and first release

Status: **needs_review**. Playable v1, implementation-plan step 7, human
[playtest 3 R1/R2](../playtest/2026-11-13-playtest-3.md). Difficulty: difficult;
assigned recommendation Sonnet-class; executed with the explicit Codex override.
Full registered main/recovery verification is **pending supervisor execution**,
then independent acceptance review. A scripted win is regression evidence, not
human playability or original fidelity.

The task-entry production path released lemmings while the entry text covered
the scene and compressed a 2.8-second model clip into the derived 0.723-second
arm interval. The next dependency is correcting this start sequence; release
rate, skills, movement, general clocks and every historical fidelity gate remain
outside this correction.

## Production change

`runApplication` -> `runApplicationLeg` -> `loadRetailLevel` -> `World` loads
the ordinary installed level. Connected frontend legs now replace only the
initial derived `releaseArmSeconds` with zero: the full door presentation below
supplies that gate. The existing 1.3-second first countdown, recurring archive
interval, rollover arithmetic, populations and spawn coordinates are retained.

After input, `EntryStartCue::advanceBeforeFrame` keeps its existing frame clock
and strict expiration below -1. A per-leg `LevelStartSequence` advances the door
sample with the same unpaused, focused frame seconds. Its duration comes from
the longest installed hatch clip bound by
`BackdropScene::entranceExitPresentation`. All installed clips have the same
duration. The frame loop passes its actual normalized sample to
`setEntranceExitPresentation` -> `applyHatchAnimation` -> existing scene matrices,
scene stages and rasterization. The door curve and interpolation are unchanged;
the retained keys include overswing/bounce before their final hanging pose.

The frame loop calls the shared `RuntimeSimState::advanceElapsed` with an
explicit release gate. Every normal or fast-forward pass goes through
`World::advanceElapsedMsWithReleaseGate`; `updateSegment` skips **only**
`stepRelease` while either the cue is still active or the full clip is unfinished.
The hatch clock therefore cannot run ahead while the text is visible, including
pause/menu/focus freeze and the initially hidden part of the cue. Once both
conditions are satisfied, the existing arm/seed/countdown path runs normally.
Both conditions are monotonic within a leg. Retry creates fresh cue/door state.

The old public World and RuntimeSim entry points still delegate with release
enabled, preserving their contracts and class layouts for frozen demonstrations.
Direct-level/headless diagnostics retain the old release clock and arm-normalized
door sampling; they do not own the frontend entry cue. Normal human entry and
Retry use the corrected sequence. No diagnostic flag was added to the user flow.

**Level clock rule unchanged:** World time, HUD countdown, second events, actor
updates, gadget updates and completion slices still advance from level entry.
The added wait consumes level time. Camera intro and the cue's clocks, geometry
and blending were not changed. Stall cap, integer-ms carry, update splitting,
fast-forward pass count and all subsequent release arithmetic are unchanged.

## Static and retained evidence

Reference, statically opened with the project REA Ghidra profile:
`C:/progs/lemmings-webolution/Lemmings Revolution/Lemmings Revolution.exe`,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Provider: Ghidra 12.1.4; analysis profile digest
`3c06738e45e144d5f073edbd3244c6bff46222f389525fe659bc92dca2ccdece`.
The session was closed without snapshot arguments. Pseudocode is provider output,
not original source; instruction reads cross-check the claims below. No original
game, original instructions/oracle, debugger, hook or capture was executed.

New raw evidence is retained under [static/](../../build/hatch-timing-001/static/):

- `pseudo-1.json` / `instructions-0x425A95.json`: hatch update calls the model
  update, tests its completion at vtable +0x44, then arms the 1.3 countdown.
  Instructions `0x425B04..0x425B56` establish the gate and seed;
  `0x425B84..0x425BAA` subtract dt and compare to zero;
  `0x425D3C..0x425D4B` add the recurring interval. Instruction evidence ID
  `ev_bc03b8770e90cb03915c79e9d4f3273b7a949798e2b57d94d1e37c4b7b7a8ada`.
- `pseudo-0.json` / `instructions-0x451121.json`: model time +0x120 adds its dt
  at `0x451175..0x451181`, compares to the selected clip duration +0xB0 and
  samples `0x4764C4`. There is no multiplier in that addition. Instruction ID
  `ev_9f31dee8a3202e1a3344e56b3bd79039c86259dff11ded509d322c88e0994d1a`.
- `instructions-0x432383.json` forwards the hatch's supplied dt to its model
  at `0x432393..0x4323A9`; ID
  `ev_837c9830879aabd1de30a45c3dee5f49414b9384df8da0bc5291de0b01f82978`.
  `pseudo-3.json` corroborates the loader selecting animation 0. The accepted
  [hatch presentation research](hatch-exit-presentation-001.md) retains selection
  and track-reader instruction checks, including `0x451479` and `0x477130`.
- `pseudo-2.json` / `instructions-0x45CC09.json` corroborate cue subtraction
  before drawing and strict retirement below -1. Instruction ID
  `ev_510cca9b2b3951bcabd9c204f41dafa6ff352160704bf2155527115289799d25`.
  `frame-loop.json` / `frame-loop-instructions.json` retain `0x45CFF9`: frame
  dt is converted from milliseconds to seconds, the cue/camera routine precedes
  World updates, and the level clock advances during playing. Instruction ID
  `ev_aa69eab7e6f64da0b3eb6ee618ffba3f4d3cc879bac373766558d9019ddb4790`.
  Cue seeds and literal display limits remain documented in
  [level-entry-start-cue.md](level-entry-start-cue.md).

The new registered read-only asset measurement covers **102 installed levels,
144 hatches, two 70-key tracks per hatch**. Clips are `entrance` or
`entrance_acid`; every full duration is `2.7999999523162842` seconds. Keys encode
nominal 40 ms samples, or 25 keys per animation second, with cumulative float
rounding and the final key snapped to clip duration. These are transform keys,
not a claim that the original renders exactly 70 video frames. The check asserts
the exact float-accumulation pattern, rather than a false uniform-double grid.
[Census](../../build/hatch-timing-001/assets/door-keys.csv) and
[summary](../../build/hatch-timing-001/assets/summary.txt).

**Original order/duration limit:** static evidence establishes model completion
before arming and the 1.3 seed after that. Unit-speed model-dt playback supports
using the complete 2.8-second clip. It does not settle the original startup phase
relative to the text, or reconcile that clip with the older
[derived ~0.723-second arming estimate](phase-d1-door-open-duration.md) and its
retained ~2.020-second first release. Those old findings are preserved, not
reclassified. The exact original elapsed-time relationship remains unresolved.

**Provisional player policy**, labelled in `LevelStartSequence.h`: begin the full
door clip at connected level entry, use one animation second per presentation
second, and wait for **both** the full clip and the entire cue lifetime before
arming the unchanged countdown. This is the simplest correction to the human
defect without reopening shared timing. A permitted original observation from
level load through text retirement, full door motion and first birth, including
Retry and matched clock anchors, would settle the remaining phase/rate mapping.
No such run was made or requested here. Existing quaternion interpolation and
broader timing/fidelity limitations remain as documented in the accepted report.

## Before/after clone measurements and image review

The authentic entry `main.cpp` was copied **before its production changes** into
[before-source](../../build/hatch-timing-001/before-source/), with
[hashes](../../build/hatch-timing-001/before-sha256.json), incoming Git diff/status
and a separate HEAD-main copy. `hatch_timing_identity` checks its exact SHA-256.
Paired executables compile frozen versus current main against common support.
The shared World/RuntimeSim additions delegate their old entry points with the
gate enabled; the frozen main never opts into holding release. No modified source
was substituted for an authentic baseline. The initial before measurement ran
before the production main change: [before.log](../../build/hatch-timing-001/before.log).

Both installed routes use actual SDL Play Current Game and map-node clicks, a
private explicitly seeded original-format save for map access, and an owned
100 ms frame clock. They pause for four frames. Observers read the real World
and cue after composition; they cannot change simulation state. The baseline
door sample is reconstructed from the frozen production expression and the
observed World clock; current main publishes the exact sample passed to its
scene. Full frame/state CSVs and PNG sequences are retained by each route.

Times below are unpaused presentation seconds from level entry, resolution
100 ms. Door phase starts at zero; the first sampled positive progress is 0.1 s
in all four runs. End means the **full clip** reaches its final pose, not the
first instant a swinging door passes through a right angle. Cue end is the first
frame with the cue retired; its existing floating arithmetic retires at 4.5 s
on this partition, while the exact -1 boundary remains active in the cue tests.

| Installed route | Version | Full door end | Cue end | First release | Final released count |
| --- | --- | --- | --- | --- | --- |
| node 0, MM_CLIMB_002 | before | 0.8 s | 4.5 s | 2.2 s | 10 |
| node 0, MM_CLIMB_002 | after | 2.8 s | 4.5 s | 5.9 s | 10 |
| node 2, MM_BOMB | before | 0.8 s | 4.5 s | 2.2 s | 20 |
| node 2, MM_BOMB | after | 2.8 s | 4.5 s | 5.9 s | 20 |

Reviewed first-release images show the old cue covering the scene at birth, and
clear text-free fallers under fully open hatches after the correction:

| Route | Before | After |
| --- | --- | --- |
| MM_CLIMB_002 | [first birth](../../build/hatch-timing-001/before/MM_CLIMB_002/first-release.png) | [clear first birth](../../build/hatch-timing-001/after/MM_CLIMB_002/first-release.png) |
| MM_BOMB | [first birth](../../build/hatch-timing-001/before/MM_BOMB/first-release.png) | [clear first birth](../../build/hatch-timing-001/after/MM_BOMB/first-release.png) |

The intro camera and expanding text obscure portions of the ordinary opening.
Supplemental model-only renders expose the unchanged actual door geometry at
[0](../../build/hatch-timing-001/assets/model-seconds-0.000000.png),
[0.8](../../build/hatch-timing-001/assets/model-seconds-0.800000.png),
[1.4](../../build/hatch-timing-001/assets/model-seconds-1.400000.png), and
[2.8 seconds](../../build/hatch-timing-001/assets/model-seconds-2.800000.png).
Visual review confirms the closed pose, swing/overshoot and final hanging pose.
These isolated renders supplement the production routes; they are not claimed
as full application frames or original comparisons.

The same public entry routes with fast-forward enabled near the beginning and
then cancelled retain zero releases while the cue/doors gate is closed. The test
asserts that the public gesture actually accelerated World time. Four paused
frames freeze the submitted door sample. A public Escape -> Try Again route
tests the shorter Retry cue: **cue end 2.5 s, full door end 2.8 s, first birth
4.2 s**. [Retry frames/summary](../../build/hatch-timing-001/retry/) prove release
also waits for the doors when the text finishes earlier.

## Cadence, routes and corrected test expectations

Every one of the 30 measured births on the two 100 ms routes shifts by exactly
**+3700 ms**. **Every subsequent gap is identical**, as asserted by the paired
regression check. [Exact release lists/comparison](../../build/hatch-timing-001/release-comparison.json).
The World release seed, rate-table values and rollover logic are unchanged.
A supplemental real-level source test holds only release for six seconds and
then checks every release count against the ordinary World stream at the matched
offset; the held World's level clock still reaches 6000 ms.

The ordinary public-input Climber route wins **10/10 before and after** and
reaches Results with persistence in its private save:
[before](../../build/hatch-timing-001/win-before/summary.txt),
[after](../../build/hatch-timing-001/win-after/summary.txt).
Its 200 ms partition has a **+3800 ms** start offset. Results show 191 versus
187 seconds remaining because the unchanged level clock counts the wait.
The existing `levelreturn` win/loss/Retry/map/persistence route also passed with
the changed production code. Broader scripted route classifications remain part
of pending supervisor verification; no all-level gameplay claim is made.

Two old expectations encoded the demonstrated defects and were corrected with
the original failures retained:

1. `frontendentry` still checks exact full-frame equality and its zero-time
   direct-render control. Its independent hatch fixture now samples the full
   2.8-second clip at **30/144 presentation seconds**, instead of fitting to
   the old arm delay at 208 integer World milliseconds. The direct diagnostic
   fixture retains its prior mapping. No pixels are masked or tolerated.
2. `hatch_exit_end_after` formerly required the entire connected simulation to
   equal the old route at the same absolute time, which forbade R1's delay.
   Its retained old trace is now aligned at first birth. It compares **all
   gameplay fields, actor state/positions, stock, terrain, outcome and pending
   timer** at every aligned frame, and separately asserts the unchanged absolute
   level-clock/tick/update-count relation and the one constant time offset.
   The added pre-birth frames must match the old empty gameplay state; the whole
   old route must be consumed. The direct-entry comparison remains exact at
   equal time. The 200 ms route passed with +3800 ms, the same 10/10 win, 33
   Pending frames, three paused frames and unchanged balloon movement.

No existing timeout was changed. The shared-timing source-path assertion was
preserved by using an overload of `advanceElapsed`; its behavioral checks pass.
No original recovery/completion assertion, fidelity exclusion or park was changed.

## Checks, preservation and handoff

Builds used registered `C:/progs/msys2/ucrt64/bin` GCC/CMake/CTest, the existing
`-O2 -g` assertion-enabled main configuration and 12 build jobs. Focused CTests
ran serially; no full verifier profile was duplicated by this worker.

- [development-1.log](../../build/hatch-timing-001/development-1.log): 5/8 passed;
  retained failures are the old source-path spelling assertion, the compressed
  frontend image expectation, and equal-absolute-time hatch/exit simulation.
  The production entry/order, pause/fast-forward and levelreturn checks passed.
- [development-2.log](../../build/hatch-timing-001/development-2.log): 11/12 passed,
  266.80 s. Corrected frontend/shared timing and the full aligned hatch/exit route
  passed, as did paired entry and natural wins. The new census test incorrectly
  assumed a uniform double 40 ms grid to within 1 microsecond; installed keys
  instead retain cumulative float rounding, reaching 1.44005 microseconds error.
- [development-3.log](../../build/hatch-timing-001/development-3.log): **5/5 passed**,
  72.18 s, after checking the exact serialized float pattern, adding supplemental
  model renders and the public Retry route, and strengthening paired cadence to
  exact equality. These five checks cover production entry, hold behavior,
  early fast-forward, all installed door keys and Retry. The other successful
  checks above were not needlessly repeated. Full main/recovery remains pending.

Nine new CTests register authentic-source identity, paired installed entry,
paired natural win, hold/order units, early fast-forward, read-only asset/model
measurement and Retry. Normal and installed save existence/contents are checked
unchanged by the connected drivers. All persistence uses explicit private
`FileSaveSink` paths under `build/hatch-timing-001/`; no real save was imported,
reset or written. The installed EXE was read only.

The supervisor already owns RepositoryLease (PID 38924). Process checks found
no live clone/compiler/CTest before work and again after focused tests. Windows
CIM enumeration was denied; ordinary `Get-Process` supplied the process check.
No second runner/lease, subagent, detached work, installation change, commit or
push was started. Existing unrelated edits were preserved, including the incoming
actor-depth work and original hook log. Task source files use LF.

Changed production: `src/app/LevelStartSequence.h`, `main.cpp`, `AppEntry.h`,
`RuntimeHud.cpp/.h`, and `src/sim/World.cpp/.h`.
Changed supporting checks: `src/__test__/test_hatch_timing.cpp`,
`test_hatch_exit.cpp`, `test_frontendentry.cpp`, `test_cameraentry.cpp`,
`test_audioapp.cpp`, `HatchEntryFixture.h`,
`cmake/hatch_timing_identity.cmake`, and task registrations in `CMakeLists.txt`.
This report is the only task documentation change. No queue, controller, verifier,
tool registry, permissions, shared instructions or completion criteria were edited.

## Verification-failure follow-up (second worker invocation)

The supervisor's
[20261114T020612-38924-d9cb0f70 verification](../../build/agent-runs/20261114T020612-38924-d9cb0f70/verification.log)
passed **230/232 main tests**, including all nine new hatch-timing checks, but
failed `audioapp` and `cameraentry_after`. This is failed verification, not a
full main/recovery pass. Its log and failed camera artifacts are preserved under
[retry-3/](../../build/hatch-timing-001/retry-3/), alongside incoming source
backups. Production timing code was retained unchanged in this follow-up.

The two remaining old expectations also encoded the corrected start behavior:

- `cameraentry_after` compared the new connected hatch against the direct
  diagnostic's old compressed clip. At frames 3/6 it differed by 83/400 pixels;
  all camera operands, ticks, Retry and re-entry checks already passed. The
  independent `HatchEntryFixture` now accepts the installed level name and
  pre-release sample time. Each explicit-camera production image must first
  match the independent compositor exactly with its old door mapping. Then the
  same compositor supplies the full 2.8-second connected door sample at
  `(frame+1)*0.25` seconds. The actual connected frame must still match **every
  pixel**; no masks/tolerances were added. All three diagnostic controls and
  all three connected comparisons now have zero changed pixels. The frozen
  camera test's expectations remain unchanged.
- `audioapp` stopped at absolute tick 108, before the delayed MM_FLOAT actor
  could naturally splat (`drained=0`, no cue). It now replays an ordinary
  unheld source World using each observed production World delta. The difference
  between the observed first-birth timestamps is subtracted solely from that
  test cutoff, retaining tick 108 in the original release timeline. Before
  first birth the original cutoff remains in effect. Neither application timing
  nor the 120-second CTest timeout changed. The literal single `FallSplat`,
  real audio-device, dead-actor, drained-cue and observer-queued-exit assertions
  remain. The successful wall-clock run measured reference birth **2109 ms**,
  connected birth **5853 ms**, offset **3744 ms**, then observed `FallSplat` at
  tick **123**. No store is injected into this test; its default options perform
  no save I/O. The camera route continues using its private temporary save.

Registered UCRT64 targeted rebuild:
[build-1.log](../../build/hatch-timing-001/retry-3/build-1.log).
Focused CTest run:
[development-1.log](../../build/hatch-timing-001/retry-3/development-1.log),
**4/4 passed in 39.24 seconds** (`frontendentry`, `audioapp`,
`cameraentry_before`, `cameraentry_after`). The complete output and corrected
camera artifacts are archived in the same retry directory. No full verifier
profile was duplicated. Full registered main/recovery verification on the final
content remains **pending supervisor execution**.

The resumed worker independently reviewed both installed routes' retained
before/after first-birth images, four full-clip model poses and the short-cue
Retry images: corrected births are text-free fallers beneath hanging open doors.
The 2.8-second playback policy and unresolved original startup phase remain
explicitly provisional; no new original comparison or analysis session occurred.
The supervisor lease is still owned by PID 38924. No clone/build/CTest process
was live before this follow-up; unrelated incoming edits and protected controls
were preserved. Final retry source hashes and patch are retained under retry-3/.

Handoff: R1/R2 implemented; four focused follow-up checks pass; status **needs_review**. Original startup phase remains provisional.
Next: supervisor full registered main/recovery verification and independent acceptance review, then human playtest confirmation.

## Acceptance-review follow-up: matched route outcomes

The separate review of
[20261114T024345-38924-0bb0704d](../../build/agent-runs/20261114T024345-38924-0bb0704d/verification.log)
correctly withheld acceptance despite **232 main / 84 recovery tests passing**:
the expansion observation driver had changed from WIN to LOSS on
`MM_BLOCK_001_A`, blocking its gadget prerequisite. That verification is historical;
it does not cover the two new regression registrations below. Full verification
of this follow-up remains **pending supervisor execution**.

### Diagnosis and correction to the initial hypothesis

The relevant retained expansion runs are
[pre-task 20261114-015157](../../build/playable-expansion-001/runs/20261114-015157/step-2-core-blocker/action-trace.csv),
[post-task 20261114-023201](../../build/playable-expansion-001/runs/20261114-023201/step-2-core-blocker/action-trace.csv)
and [post-task 20261114-024914](../../build/playable-expansion-001/runs/20261114-024914/step-2-core-blocker/action-trace.csv).
The first pre-task attempt already lost with zero saved; its Retry won 23/30.
The post-task Retry used different blocker positions, rather than replaying the
same input sequence after the start offset.

An initial hypothesis that some assignments had not left blockers was wrong.
`trackFrame` in `test_playableexpansion.cpp` resets its previous-state arrays
whenever the actor vector grows. A birth can therefore hide a real transition
from `state-blocker`/`blockers_seen`, even while `assign-ok` records spent stock.
The authoritative `actors-at-end` lines show **four lasting blockers in each
attempt**. The observer has not been changed as part of this production task.

| Retained Retry | Actor 0 | Actor 1 | Actor 2 | Actor 3 | Saved |
| --- | --- | --- | --- | --- | --- |
| Pre-task WIN | (177,24) | (82,45) | (176,65) | (155,65) | 23/30 |
| Post-task 024914 LOSS | (175,24) | (82,45) | (175,65) | (153,65) | 0/30 |

The driver observes wall-clock frames, enables seven-pass fast-forward, chooses
an edge within a range, and clicks the previous projected actor rectangle.
The added entry wait changes the frame/actor alignment; it does not promise the
same cell or picked actor at the next adaptive click. Positions matter here:
`World::assign` -> `stampBlockerField` stamps the actor's **one X column**, and
`stepWalker` tests that column during its next-cell probe. The matched replays
below reproduce both outcomes on both implementations. This diagnoses the
observed difference as placement-sensitive scripting, without retuning release,
skills, input, clocks or blocker behavior. It does not identify one individual
cell shift as the sole cause, or promise stable outcomes for the old wall-clock
policy under every machine load.

### Production replay and asserted outcomes

`test_hatch_timing.cpp` now also registers `hatch_timing_routes_before` and
`hatch_timing_routes_after`. They compile the same driver against the existing
hash-checked authentic frozen main and the current production main, using common
unchanged support. They enter via Play Current Game and real map clicks, use
public Escape -> Try Again for the historical short-cue Retry, select Blocker
with key 4, and click real actor rectangles at the retained cells. The owned
frame step is 200 ms on both sides. Fast-forward is activated only after the
placements and all births, keeping individual birth observations unambiguous.

This is a matched replay of the **retained resulting assignments**, not an exact
reconstruction of the historical wall-clock frames, which were not all logged.
Every click must leave the intended actor a Blocker on the next observed frame;
the final win/loss and exact saved/released counts are assertions, rather than
observation-driver exit-code interpretations.

| Matched route | Frozen before | Current after | First birth before / after |
| --- | --- | --- | --- |
| MM_BLOCK_001_A, winning Retry placements | WIN, 23/30 | WIN, 23/30 | 2200 / 4200 ms |
| MM_BLOCK_001_A, losing Retry placements | LOSS, 0/30 | LOSS, 0/30 | 2200 / 4200 ms |
| MM_W_012, historical gadget placements | LOSS, 2/20 | LOSS, 2/20 | 2200 / 6000 ms |

Only node 0 completion is seeded in the private fixture save. The first blocker
win actually earns node 3 completion, returns to the map and unlocks node 8.
A fresh application loads that **same private save** and selects MM_W_012; its
flag/selectability is never injected. The gadget replay places actors 2, 3 and
12 at (214,41), (163,41) and (185,26). The historical third click intended actor
11 but picked actor 12, so the matched driver explicitly clicks actor 12. This
preserves the historical two-save LOSS while restoring the previously blocked
coverage; the earlier no-input gadget diagnostic separately won 20/20.

The current driver compares every post-birth sample before absolute timeout:
all serialized actor fields, skill stock, spawned/saved/dead counts, lever
state/cooldown and gate height/request/done/armed match the frozen route exactly.
Every one of the **80 births** has the identical first-birth-relative timestamp;
all subsequent observed intervals and released counts are equal. Matched click
ages and screen coordinates also agree. The level clock still advances during
the added wait: Retry shifts by **2000 ms**, ordinary gadget entry by **3800 ms**
on this partition. Comparisons stop at the current absolute timeout rather than
inventing extra playable time. Both final outcomes and saved counts must still
match. Fluid wave phase, absolute clock/update counters and whole-frame pixels
are not claimed equal at the shifted times.

The paired traces contain **538 identical state rows for each blocker route**
and **445 for the gadget route**. Their clock differences remain exactly the
stated constant offsets at every compared row. Each route's Results PNG is
byte-identical between before and after.

Retained final sequences and summaries:
[routes-before](../../build/hatch-timing-001/routes-before/) and
[routes-after](../../build/hatch-timing-001/routes-after/), with per-route
`actions.csv`, `frames.csv`, `gameplay.txt`, `births.txt`, `outcome.txt`,
`summary.txt`, pre-click/confirmed-blocker PNGs, Pending and Results images.
The winning route also retains `earned-map.png`. The condensed comparison is
[matched-comparison.json](../../build/hatch-timing-001/review-followup/matched-comparison.json).
Reviewed result images show 23 saved against 15 required on both versions; the
earned map shows the newly selectable gadget branch. Both two-save gadget losses
and zero-save blocker losses are retained, not reclassified as gameplay wins.
The earlier MM_CLIMB_002/MM_BOMB entry images and short-cue Retry images were also
reviewed again: corrected births remain text-free beneath open doors.

### Checks and preservation for this follow-up

No production source changed in this invocation. Incremental changes are confined
to `src/__test__/test_hatch_timing.cpp`, `CMakeLists.txt` and this report. Existing
assertions/timeouts and the expansion observation policy were preserved. Earlier
task implementation and unrelated incoming changes remain intact.

Development evidence under
[review-followup](../../build/hatch-timing-001/review-followup/):

- `development-1.log` / `incomplete-replay-1/`: manually stopped the new, incomplete
  replay after discovering the mistaken two-blocker interpretation. Its failed
  route is preserved; it was not a full verifier run or a passing comparison.
- `development-2.log` / `replay-2/`: frozen replay passed; current replay failed
  three new outcome-text comparisons because a binary CRLF read was compared to
  an LF literal. Actor traces, actions, cadence and numeric outcomes matched.
  The comparison now uses a text stream. This diagnostic used no gadget skills;
  the final check above instead replays the historical gadget assignments.
- `build-3.log` and `development-3.log`: registered UCRT64 targeted rebuild and
  focused CTests for baseline identity, both matched route versions and existing
  release-hold/order units: **4/4 passed in 273.24 seconds**. The frozen and
  current route suites took 135.42 and 137.74 seconds respectively; both report
  zero failures and unchanged real saves.

The same registered optimized GCC build retains assertions. No full verifier
profile was run by this worker. Installed and normal save contents/existence are
checked unchanged by both replay executables; their persistence uses only their
private `routes-before/SavedGame.dat` / `routes-after/SavedGame.dat`. Source and
report files use LF; Windows text-mode runtime artifacts may use CRLF.

The existing supervisor RepositoryLease (PID 38924) covers this worker. No new
runner, concurrent writer, subagent, original process, debugger, hook, original capture,
installation change, static-analysis session, protected-control edit or commit
was started. Initial Windows CIM access was denied; ordinary `Get-Process`
provided the clone/build/test process check. Backups and incoming diff/status
are retained under `review-followup/incoming/` and its parent directory.

Unknowns remain unchanged: original cue/door startup phase and complete elapsed
time mapping are provisional; this is clone regression evidence, not original
fidelity or a human playability pass. The original observation needed to settle
that mapping remains described above; none was executed here.

Handoff: R1/R2 production fix retained; matched winning/losing/gadget outcomes preserved; status **needs_review**.
Next: supervisor full registered main/recovery verification and separate acceptance review, then human playtest confirmation.
