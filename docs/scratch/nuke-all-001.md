# nuke-all-001: working Nuke control

Status: **needs_review**. Playable v1, Playtest 6 V3, on installed
`MM_CLIMB_002` through New Game -> map node 0 -> level -> results.
Difficulty: difficult; recommended model Opus for input/state/accounting
integration; this run uses the authorized Codex override.

The control now cancels releases, counts down five seconds, then sends the
remaining actors through the existing Bomber explosion and crater path in
insertion order, one per existing 5 Hz tick. The level reaches ordinary results
and preserves the actual saved count. These are scripted clone observations,
not human playability acceptance or original timing/fidelity evidence.
Full registered verification for the retry is **pending supervisor execution**.
The first supervisor main run failed in historical mixed-source builds; the
retry correction and its 19 passing focused checks are recorded below.

## Failure, dependency and production path

[Playtest 6](../playtest/2026-11-15-playtest-6.md) records that Nuke does nothing.
At task entry, `RuntimeSkillSelection::consumePanelClick` consumed plate 9 but
`main.cpp` only dispatched the fast-forward command. The disconnected
`World::nuke()` lit every active fuse together and did not stop hatches.
Connecting only that helper would still release new lemmings and leave
`RuntimeSimState` waiting for `totalLemmings - spawned()` after cancellation.
Input, sequential replacement and completion accounting are therefore one
bounded dependency for this defect.

The connected path is now:

1. `SdlWindow::gameCoordinates` maps the pointer into the existing 640x480
   layout. `runApplicationLeg` passes complete left-button gestures to
   `RuntimeNukeInput`, using the same `runtimeSkillPanelIndex` as panel picking.
   Two releases on plate 9 within strictly less than 500 ms call `World::nuke`.
   Panel presses remain consumed before actor picking. Focus loss, right-button
   capture and menu transitions clear confirmation. No existing Nuke keyboard
   shortcut was found; the clone's skill keys remain 1..8.
2. `loadRetailLevelReport` reads serialized `gadgets.values_27d0[9]` into
   `LevelSetup::nukeStock`. The request consumes one command stock, is
   idempotent thereafter, and makes `remainingToRelease()` zero.
   `stepRelease` rejects all subsequent releases, including timeout-enabled
   updates and the start presentation's release gate. Spawned/saved/dead
   counters are not falsified to stand in for cancellation.
3. `updateSegment` spends its existing shared millisecond delta on the global
   countdown. After expiry, `stepNuke` replaces one eligible actor per existing
   tick boundary, oldest insertion first. Stable vector indices skip actors
   already saved/dead or already counted as splats; an existing Exploder is
   allowed to finish without restarting its Bomber animation.
4. Replacement calls `leaveBlocker`, clears teleport contact latches, and
   enters `State::Exploder`. The Nuke-owned state uses the unchanged
   `stepExploder` -> `explode` -> `Terrain::clearExplosion` and cue path.
   Its entry-only tick matches the ordinary fuse's entry-only tick. Nuke-owned
   animation cannot be replaced by fluid/exit contact before its crater/removal.
   Ordinary Bomber assignment, fuse, movement, animation and crater rules were
   not edited.
5. `RuntimeSimState` passes the cancellation-aware unreleased count to
   `LevelEndTransition`. For Nuke only, it waits for all actor types and holds
   the result-delay timer while the wave remains alive, preventing timeout or
   a long rat-only wave from truncating it. The existing pass predicate,
   current saved count, frontend results and progress write path then run.
   Hatch hover feedback also reads zero remaining after cancellation.

## Static evidence and provisional choices

Reference: repository-local `Lemmings Revolution/Lemmings Revolution.exe`,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Its identity was rechecked by reading bytes only. No original executable,
copied instructions, debugger, hooks or original capture ran.

This implementation starts from the accepted revision-5
[command research](gameplay-commands-research-001.md), whose acceptance is in
`agent/tasks.json` under `gameplay-commands-research-001.review`.
Retained Ghidra instruction envelopes are in
`build/gameplay-commands-research-001/direct-recovery/evidence.json`.
The used envelopes and readable assembly were copied to
[retained-static](../../build/nuke-all-001/retained-static/evidence.json).
These are retained static reads, not a new REA session or original observation.

| Item | Instruction evidence | Clone policy / settling observation |
| --- | --- | --- |
| Gesture | `0x483CCB..0x483D00`: release edge; signed difference compared with `0x1F4`, pulse set below 500. `0x4839A0` initializes prior timestamp to zero. `0x455E10`, `0x454725..0x45476A` dispatch hover 9 through selected-stock consumption and request `0x444A98`. | **Provisional adapter:** require two complete clicks on this plate, use unsigned SDL timestamp subtraction, omit the ambiguous first-release-at-startup shortcut, and keep actor-skill selection independent. An original startup/activation observation with input timestamps would settle the initial-release edge and exact confirmation interaction. |
| Countdown | `0x4426BF..0x4426C9` pushes 5/5 to `0x413D40`; `0x413F15` subtracts dt from the five-second overlay. | Five seconds of existing shared level time; first attachment is the next existing tick boundary at/after expiry. Pause freezes it. No new shared clock or timing retuning. |
| Order and stagger | `0x442736..0x44274D`, two-sentinel list readers/insertions, and `0x4429C0..0x4429D5` establish oldest-first traversal. `0x4428C2..0x442A2C` attaches at most one per update, adds the retained 0.05-second credit and subtracts dt even during countdown. | **Provisional:** one replacement per existing tick, normally 200 ms; no attempt to imitate accumulated credit or event slices. A timestamped original activation recording showing successive state attachments/blasts would settle visible cadence. |
| State and removal | `0x416910`, `0x41695C`, `0x42E696`: primary-state replacement and detach. `0x41699E` -> `0x408227`: explosion animation then crater/removal. | Reuse the existing clone Bomber path, with Nuke ownership until removal. Skip stable indices already lost/saved instead of reproducing the unresolved original cursor lifetime hazard. |
| Release/end | `0x442753..0x442772` conditionally clears hatches and unreleased count; `0x445498` evaluates saved/needed and the ordinary result delay. | **Provisional task policy:** stop releases immediately even before the first hatch, and wait for the whole wave, including type 3 and timeout. Original before-first-release/late-timeout observations would settle those differences. |

The gold control outline, `CLICK AGAIN`, and `NUKE 5..1` text are explicitly
provisional feedback. Original confirmation/countdown presentation would settle
its appearance. The legacy skill-trailer fallback has no established command
stock field and disables Nuke; supported ordered retail reads retain the field
(portable cap 200, consistent with bounded stock handling). The retained
102-level census found stock 9 equal to one everywhere; the registered new unit
test independently measures the installed route's value as one.

## Before/after clone evidence

The immutable evidence for this worker's final focused run is under
[worker-evidence/final](../../build/nuke-all-001/worker-evidence/final/observations.json).
`trace.csv` records world/end/terrain state; `actors.csv` records, without a
header, frame, ms, actor index, state, Nuke ownership, state frame, x, y, drawn
clip, temporal entry, and screen left/top/right/bottom. PNGs are literal clone
framebuffers, inspected during this worker session.

Before edits, the task-entry files were copied under ignored
`build/nuke-all-001/before-source`; hashes are in
[before-sha256.json](../../build/nuke-all-001/before-sha256.json).
`nuke_baseline_identity` guards task-entry `main.cpp` SHA-256
`3a6f420e0c89fa54502b5b20135eb30b877c677ab85f7df6db6736d71cc149ca`.
The before driver compiles that exact input/application source against current
shared simulation/render code; this is an isolated input-path baseline, not a
claim that all shared sources were frozen. The before path never calls Nuke.
The entry snapshot includes prior uncommitted work, which was preserved.

| Route | Actual observation |
| --- | --- |
| Same pointer pair, before | Five released actors when clicked; no request, countdown, deaths or crater. By 21.6 s, six had released and the level was still running. [Still inert](../../build/nuke-all-001/worker-evidence/final/before/live/still-inert.png). |
| Same pointer pair, after | First release shows [confirmation](../../build/nuke-all-001/worker-evidence/final/after/live/confirmation.png); second shows [NUKE 5](../../build/nuke-all-001/worker-evidence/final/after/live/activated.png). Releases stop at five. Actors 0..4 enter `explode` at 22.8, 23.0, 23.2, 23.4, 23.6 s. Losses/craters follow at 24.0..24.8 s in 200 ms steps. Render synchronization records 71 changed cells. [Early wave](../../build/nuke-all-001/worker-evidence/final/after/live/exploding-mid-0.png), [later wave](../../build/nuke-all-001/worker-evidence/final/after/live/exploding-mid-3.png), [cleared](../../build/nuke-all-001/worker-evidence/final/after/live/cleared.png). |
| Normal loss results | Five released actors exploded, five were cancelled before release; total lost is therefore ten, saved zero. [Results](../../build/nuke-all-001/worker-evidence/final/after/live/results.png). No completion unlock or extra progress write. |
| One real rescue first | A normal projected pointer click assigns one Climber. After its natural rescue, Nuke explodes the other nine, actors 1..9, at distinct ticks. Results correctly show [needed 1 / saved 1 / success](../../build/nuke-all-001/worker-evidence/final/after/partial/results.png); normal completion flag and private progress write occur. 125 drawn terrain cells changed. |
| Before first release | Activates with zero spawned, prevents every release and reaches [0/10 loss results](../../build/nuke-all-001/worker-evidence/final/after/empty/results.png), with no invented actor deaths. |

The application demonstrations use `runApplication --frontend --native
--no-audio --frames 560`, a declared 0.2-second owned clock, dummy SDL, ordinary
New Game/map input, SDL pointer gestures, and read-only frame observers. They
never mutate the application World or fabricate completion. Each run uses a
fresh unique private save under its test output and checks the normal and
installed save's bytes/existence before/after. No save was imported or reset.

During the countdown, ordinary gameplay still runs. Actors naturally saved or
already lost before their turn are skipped. The installed live demonstration
has no such intervening removal: all five activation survivors enter Exploder
and die once. Supplemental source tests cover already-primed Bomber coexistence,
Blocker detach, a natural splat during the wave without double loss accounting,
and forty type-3 actors with a time limit that expires during Nuke.

## Checks, changes and handoff

First worker invocation's final focused check (historical):
[focused-final-ctest.log](../../build/nuke-all-001/worker-evidence/final/focused-final-ctest.log),
**12/12 passed in 54.39 s**, after
[development-build-6.log](../../build/nuke-all-001/worker-evidence/final/development-build-6.log).
Registered CTest cases cover before/after production pointer routes, empty and
partial-rescue routes, baseline identity, and units/installed stock. Existing
`world`, `runtimeskillpanel`, `fastforward_units`, `fastforwardapp_after`,
`fastforward_toggle_production`, and `actoranimstate_bomb` also passed unchanged.
No existing assertion or timeout was edited. The Bomber demonstration still
uses its ordinary pointer assignment/countdown/animation path.

The development history remains under `build/nuke-all-001`: the initial test
driver omitted SDL key symbols and initially evaluated relative-frame actions
before a click existed; its first partial-rescue expectation also incorrectly
assumed a loss despite this level's target being one. Those new-driver defects
were corrected. The first link exposed a HUD text dependency in the small panel
test, resolved by defining the feedback drawing in `RuntimeHud.cpp`. No failed
development run is represented as a passing final run. The PowerShell wrapper
was unavailable under the current execution policy; builds used the registered
absolute MSYS2 UCRT64 CMake/Ninja/GCC paths without changing that policy, with
`-O2 -g` and assertions.

Task changes: `src/sim/World.h`, `World.cpp`, `RetailLevel.cpp`;
`src/app/main.cpp`, `RuntimeSkillPanel.h/.cpp`, `RuntimeHud.cpp`;
`src/__test__/test_nuke.cpp`; `cmake/nuke.cmake`, `nuke_identity.cmake`;
the additive include in `CMakeLists.txt`; this report.
[Task-only diff](../../build/nuke-all-001/worker-evidence/final/task-only.patch)
and [source hashes](../../build/nuke-all-001/worker-evidence/final/source-sha256.json)
separate this work from pre-existing edits. Queue, verifier, controller,
registry, shared instructions, model pins and completion criteria were not edited.

Unproven: original gesture startup edge, exact cadence and overlay pixels;
original pause/timeout/type-3 equivalence; every installed level and every
gadget/state interleaving; audible Nuke playback (app evidence disables audio).
Original Armageddon timing and all historical fidelity gates remain parked/open.

Handoff: Nuke is implemented and focused checks pass; finish **needs_review**.
Next: supervisor runs full registered `all` verification, then independent acceptance review.


## Retry 2: preserve historical methods with compatible class layouts

The first supervisor attempt is retained at
[verification.log](../../build/agent-runs/20261116T000052-39368-4f53a67f/verification.log).
It passed 321/330 main tests, including all six Nuke checks, but failed nine:
`frontendwindowquit_before`, `fastforward_toggle_baseline`,
`fluid_hazard_before`, `fluid_hazard_gadget_before`, `empty_receiver_before`,
`empty_receiver_hatch_before`, `empty_receiver_gadget_before`,
`gravity_gadgets_before`, and the dependent `gravity_gadgets_after` comparison.
The earlier 12/12 focused result above was not a full-verification pass.

Inspection found eight historical-method binaries combining frozen `World.h`
with current simulation/app objects. Nuke's added `Lemming`, `LevelSetup` and
`World` members made these layouts incompatible. Seven crashed or reported
heap/stack corruption; `gravity_gadgets_before` also crashed, leaving its
`after` comparison without the matching baseline frames. The new simulation
members were already present at retry entry; their source files are unchanged
by this retry.

`cmake/nuke.cmake` now supplies these eight binaries with separate build
copies of their retained method sources and headers. Every compiled `.cpp`
copy is byte-identical to its original snapshot. Only `src/sim/World.h` in the
build overlay comes from the current declaration, so both sides of calls to
shared objects agree on layout. Historical method bodies and the other retained
headers remain frozen. This is explicitly a historical-method comparison with
a current layout, not a wholly frozen application. Original snapshots and
identity manifests were never modified. No test assertion, timeout, registration,
expected result, supervisor/verifier source or shared control was changed.

The dependency repair is bounded to the binaries broken by this task's layout
addition. It does not change the production Nuke/Bomber implementation or any
other gameplay behavior. It also avoids copying the old `World.h` over the
new one during each configure, preventing needless repeated recompilation.
[Provenance](../../build/nuke-all-001/retry-2/provenance.tsv) records each
original file, actual compiled copy and SHA-256, including the current header.
[Validation](../../build/nuke-all-001/retry-2/validation.json) checks those hashes
and confirms the inherited production files remain identical to retry entry.

Actual retry checks, using the registered UCRT64 tool paths and the existing
`-O2 -g` assertions-enabled development build:

- [Build](../../build/nuke-all-001/retry-2/build-abi.log): all eleven requested
  executables built successfully. The final
  [incremental build](../../build/nuke-all-001/retry-2/final-build.log) after
  the configure/provenance cleanup compiled or linked no additional objects.
- [Focused CTest log](../../build/nuke-all-001/retry-2/focused-abi.log) and
  [JUnit](../../build/nuke-all-001/retry-2/focused-abi.xml): **19/19 passed,
  293.92 seconds**. This includes all nine previously failing comparisons,
  their identity/seed prerequisites, and all six Nuke tests. Original snapshot
  identity checks passed unchanged.
- Fresh [before pointer summary](../../build/nuke-all-001/retry-2/observations/before/live/summary.txt)
  still records no Nuke, explosion, crater or result. Fresh after runs reproduce
  [0 saved / 10 lost, 5 released](../../build/nuke-all-001/retry-2/observations/after/live/result.txt),
  [1 saved / 9 lost after a real rescue](../../build/nuke-all-001/retry-2/observations/after/partial/result.txt),
  and [0 saved / 10 lost before any release](../../build/nuke-all-001/retry-2/observations/after/empty/result.txt).
  Their PNGs and traces are retained alongside these summaries. Both the
  original retained frames and these fresh frames show the same control and
  sequential explosion behavior; earlier artifacts remain intact.
- Normal and installed save preservation assertions passed in each Nuke app
  run. Source reference hash was rechecked by byte reads only and still matches
  the SHA-256 above. `git diff --check` passes for the task paths.

Retry edits are only `cmake/nuke.cmake` and this report. All incoming production,
queue/status, window, pick/assignment and installed hook-log changes were
preserved. The supervisor lease remained owned by PID 39368; no protected
control was edited. CIM process detail enumeration was denied, so the supported
`Get-Process` check was used. No original execution, new REA session, subagent,
background helper, tool installation, commit or push occurred.

Unknowns remain those already documented: exact original activation/startup
and pause semantics, cadence, feedback appearance, and general gameplay
interleavings. The provisional gesture/stagger/feedback choices and settling
observations are unchanged. Mandatory full `all` verification and independent
review remain pending; this focused retry does not establish full-project or
original-fidelity completion.

Handoff: nuke-all-001 is **needs_review**; production Nuke evidence is retained and all 19 retry-focused checks pass, including the prior nine failures.
Next action: supervisor runs mandatory full registered `all` verification, followed by independent acceptance review.
