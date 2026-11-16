# release-rate-control-001: player release-rate control

Status: **needs_review**. Playable v1, [playtest 6 V8](../playtest/2026-11-15-playtest-6.md).
Difficulty: difficult; recommended model Opus; assigned Codex override used.
**Mandatory smoke verification and independent review are pending supervisor execution.**
The user's next playtest determines acceptance of the visible behavior.

## Result and production call path

The demonstrated failure was inert left/right input on the release control.
The existing loader already knew each hatch's authored initial/minimum/maximum
values and its `RELEASERATE.PC` interval. Connecting that data to player input
was the next dependency; changing the simulation clock was unnecessary.

A visible **RATE** box below Nuke displays the current value. Left click lowers
the rate and slows releases; right click raises it and speeds releases. Each
press changes one table row; holding repeats after 0.4 seconds, then every
0.1 seconds. Values stop at the installed level's limits. The control works
while paused, while simulation remains paused. Release, pointer exit from the
box, focus loss and menu input reset cancel a hold. Right input is consumed
before camera grab, and left input before actor selection. The 1x/2x windows
use the existing game-coordinate conversion. CLI help describes the gestures.

Production load: `runApplication` -> `runApplicationLeg` ->
`RuntimeReleaseRateControl::create` -> repository-local `AssetLibrary` ->
`readHatchRecords(COMPILED/<resolved-id>.LVL)` and `loadReleaseRateTable`.
The first structured hatch supplies initial/minimum/maximum; creating the
control does not edit World. Each new level/retry creates a fresh control.

Production input: `SdlWindow::gameCoordinates` -> mouse-down branch in
`runApplicationLeg` -> `RuntimeReleaseRateControl::press/change` ->
`World::setReleaseInterval`. The setter changes only the existing recurring
seconds/ticks fields using `ReleaseRates::secondsPerLemming/intervalTicks`.
`RuntimeSimState::advanceElapsed` -> `World::advanceElapsedMsWithReleaseGate`
-> `updateSegment` -> `stepRelease` subsequently reads that same interval.
The running countdown finishes before the changed interval is added at the
next birth. The first-release seed, arm delay, countdown, remainder, mapping,
shared clock and spawn order are unchanged. `World.cpp`, `RetailLevel.cpp`
and `GameData.cpp` are unchanged from HEAD.

Production presentation: the existing panel/HUD composition calls
`RuntimeReleaseRateControl::draw` -> `drawReleaseRateDisplay` in RuntimeHud.cpp
using the cached retail font. The number is `current_`, the same value used
for the selected table row. Headless panel/HUD composition also draws the
initial value, keeping its established frontend-entry comparison meaningful.
No existing simulation/input object gains a data field; the control owns its
own state and the small World setter is inline.

## Retained static authority and installed values

Starting evidence: [D1 release-trigger research](phase-d1-release-trigger.md),
[accepted structured hatch loader](native-009-structured-hatch-records.md),
[hatch directions](hatch-release-direction-001.md) and
[hover feedback](hover-feedback-001.md). The resume used the
[reverse-engineer-anything skill](C:/Users/bbrown/.agents/skills/reverse-engineer-anything/SKILL.md)
to cross-check retained instruction packets. No new analysis session was
needed. No original executable, copied original instructions/oracle, debugger,
hook, capture, installer or launcher ran.

Reference: repository-local `Lemmings Revolution/Lemmings Revolution.exe`,
SHA-256 **32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f**,
rechecked as bytes during the resume. Earlier static packets identify Ghidra
12.1.4 and profile digest
`3c06738e45e144d5f073edbd3244c6bff46222f389525fe659bc92dca2ccdece`.
They remain under [static/](../../build/release-rate-control-001/static/);
the resume's selected instruction excerpts are in
[static-crosscheck.txt](../../build/release-rate-control-001/resume-20261116T0413/static-crosscheck.txt).

- `425F38/425F49/425F5A` read successive u32 values; stores at
  `425F40/425F51/425F62` place them at hatch `+A0/+A4/+A8`.
  Evidence `ev_d8424a50adcd2b28c217cb51210918c5aff48a86ac983d8712c4864d7bfdd2d9`.
- `425DF0..425E1B` clamps the requested rate against `+A4/+A8`.
  `425E24..425E44` copies the double at `4C56E8 + rate*8` to `+B8/+BC`
  and stores the selected rate at `+A0`. It never stores countdown `+C0`.
  Evidence `ev_30cd7fbc21061d5f28018e1b33e7255a901268dec00ca75d74251ac2b928b71f`.
- `453A31..453AAF` uses selected hatch `screen+1894`: query `455DD0`,
  add one at `453A62`, call `425DE3`; query `455D90`, subtract one at
  `453A9F`, call the same setter. There is no repeat-delay gate here.
  Evidence `ev_fc89df8ce30d2242ff4be14f9e3b258396a62fcc0bfe360dd358127382886efa`.
- Retained wrapper pseudocode links `455D90` to `455DB0` and `455DD0`
  to `455DF0`. Instructions at `455DB0` test mouse `+0 == 1`; `455DF0`
  tests `+4 == 1`. `483C3E..483C70` stores the held high bits of sampled
  buttons 0 and 1 into those fields. This establishes held-button direction;
  conventional physical left/right mapping remains an SDL adapter choice.

`LevelObjects.cpp::readHatchRecords` projects the recovered compiled reader's
`gadgets.entities_425e50[*].values_a0_a4_a8[0..2]` into
`HatchRecord::rate/rateMin/rateMax`. `RetailLevel.cpp` already clamps that
triple and maps the primary hatch through the same interval table. This is a
varying stored range, not a global 1..99 control.

| Installed level | Primary initial | Minimum | Maximum | Initial interval, seconds |
| --- | ---: | ---: | ---: | ---: |
| MM_CLIMB_002 | 20 | 1 | 50 | 2.8864866275534937 |
| MM_DIG_000 | 25 | 1 | 50 | 2.6246160353341117 |
| E07_JC_1 | 10 | 10 | 99 | 3.5453803358448166 |
| E10_KF_1 | 1 | 1 | 1 | 5.0 |

The retained [installed-rates.csv](../../build/release-rate-control-001/resume-20261116T0413/retained-observations/after/units/installed-rates.csv)
contains 102 levels / 144 hatches / 35 primary-hatch triples. The current short
unit check again verifies all 102 initial values and limits. E10_KF_1's other
hatches have their own fixed values; independent scheduling remains out of scope.

## Before/after clone observations and resume correction

Before: the [human ledger](../playtest/2026-11-15-playtest-6.md) records inert
input. Earlier worker passes also retained source-built clone observations on
installed `MM_CLIMB_002`, using production SDL input, a deterministic 10 ms
frame delta and a private FileSaveSink. Those are historical development
observations, not new runs in this resume. Their images/traces were copied
unchanged to [retained-observations/](../../build/release-rate-control-001/resume-20261116T0413/retained-observations/)
so subsequent verification cannot overwrite this report's evidence.

Inspected clone images: [before, no rate box](../../build/release-rate-control-001/resume-20261116T0413/retained-observations/before/pointer/initial.png),
[initial 20](../../build/release-rate-control-001/resume-20261116T0413/retained-observations/after/pointer/initial.png),
[minimum 1](../../build/release-rate-control-001/resume-20261116T0413/retained-observations/after/pointer/minimum.png)
and [maximum 50](../../build/release-rate-control-001/resume-20261116T0413/retained-observations/after/pointer/maximum.png).
The current value is legible below Nuke. The retained traces show the same
first three births at 2030, 4920, 7810 ms. A right click after birth 3 changes
20 -> 21; the birth-4-to-5 gap changes from 2890 ms to 2840 ms. A left click
after birth 5 restores 20; the birth-6-to-7 gap is 2880 ms. The current
countdown is preserved in both cases. Held input reaches 50 and 1 without
crossing either bound. The earlier 2x input route also completed with zero
failures. With no input, both earlier sides recorded exactly
**2030, 4920, 7810, 10690, 13580, 16470, 19350 ms**.

The original detailed report and its earlier development claims remain
[archived here](../../build/release-rate-control-001/resume-20261116T0413/entry/docs/scratch/release-rate-control-001.md).
Correction: those earlier focused passes did **not** establish a passing
supervisor gate. Attempt 1's [verification.log](../../build/agent-runs/20261116T011339-39368-7a50ec64/verification.log)
failed 15 of 339 checks. Attempt 2 was interrupted by the reported host
low-memory condition; it did not finish verification. Its partial expected-frame
repairs were inspected and backed up before resuming.

The protected `frontendentry` smoke check expected a panel without RATE. The
shared `HatchEntryFixture` now explicitly composes the installed initial rate,
matching headless and live presentation. Assertions/timeouts are unchanged.
This common compositor also resolves `runtimewindow`, `levelmenuwindow` and
`cameraentry_after`; all four passed the current focused run.

Under AGENTS.md's verification-tier rule, these 11 failed **legacy**
registrations were removed, with original source/evidence retained:

- `hatch_direction_profile_before`: old full-frame hash omits RATE.
- `hatch_direction_profile_after`: old full-frame hash omits RATE.
- `hover_feedback_comparison`: pre-task full frames omit RATE.
- `exit_balloon_compare`: pre-task pending-result frame omits RATE.
- `actor_occlusion_app_after`: pre-task expected full frames omit RATE.
- `antigrav_after`: non-antigravity frame identity omits RATE.
- `climber_occlusion_app_after`: no-climber frame identity omits RATE.
- `lemming_type_sprites_after`: unchanged-route frame identity omits RATE.
- `gravity_gadgets_after`: unchanged-route frame identity omits RATE.
- `frame_profile_MM_CLIMB_002`: old full-frame hash omits RATE.
- `frame_profile_MM_W_012`: old full-frame hash omits RATE.

Seven interrupted, task-specific legacy compositor repairs were removed;
those test sources now match HEAD. This task's own before/after route driver,
frozen-main target, binary-anchor check and identity checks were removed from
source/registration. Their historical build artifacts are preserved. The only
new test is `release_rate_units`, in legacy: stored bounds, table row selection,
repeat/cancellation, first-countdown preservation and unchanged idle timing.
The configuration retains **95 smoke tests**, with 320 tests total.

## Current checks, changed files and limits

Registered UCRT64 GCC 16.1.0/CMake/Ninja, `-O2 -g` with assertions:
[focused-build.log](../../build/release-rate-control-001/resume-20261116T0413/focused-build.log).
Focused CTest: **5/5 passed, 71.26 seconds**:
`runtimewindow`, `frontendentry`, `levelmenuwindow`, `cameraentry_after`,
`release_rate_units`. The new unit took **0.39 seconds**.
Exact [log](../../build/release-rate-control-001/resume-20261116T0413/focused.log)
and [JUnit](../../build/release-rate-control-001/resume-20261116T0413/focused.xml).
The application tests use their existing memory-only/injected private-save
paths; direct CLI composition does not enter the persistent frontend. The new
unit does not use a save store. `git diff --check` passed. No mandatory smoke
profile or full suite was duplicated; supervisor smoke verification is pending.

Changed production files: `src/app/main.cpp`, `RuntimeSkillPanel.h/.cpp`,
`RuntimeHud.cpp`, new `ReleaseRateDisplay.h`, and `src/sim/World.h`.
Supporting files: `src/__test__/HatchEntryFixture.h`, `test_release_rate.cpp`,
`CMakeLists.txt`, `cmake/release_rate.cmake`, `antigrav_presentation.cmake`,
`climber_occlusion.cmake`, `frame_rate_profile.cmake`, `gravity_gadgets.cmake`,
`lemming_type_sprites.cmake`, and this report.

Incoming edits and backups are retained under
[resume-20261116T0413/entry/](../../build/release-rate-control-001/resume-20261116T0413/entry/).
Supported process inspection found no active clone/compiler/CTest at entry;
CIM enumeration was denied, while Get-Process was available. The supervisor
held RepositoryLease PID 49132. The worker edited no protected controls; queue
and generated run status were confirmed byte-unchanged from resume entry.
No subagent, background helper, commit, installation or save edit was made.

Provisional: RATE geometry/artwork, physical button mapping, 0.4/0.1-second
repeat and pause/cancellation ergonomics. Original held-state updates target
the hovered world hatch, not this new panel layout. An authorized original
observation of both buttons, a sustained hold and pause/focus transitions would
settle physical mapping and effective repeat cadence. No such run is authorized
for this worker. The clone retains its accepted **first-hatch/shared-clock**
policy. Multi-hatch control/scheduling, D1/D3, original timing, fidelity gates
and full-project completion criteria remain unchanged.

## Two-line handoff

Release control uses authored primary-hatch limits and the existing interval table; the first-attempt failures are addressed and 5 focused checks pass.
Supervisor: run the mandatory smoke profile, then independent review; leave visible acceptance to the user's next playtest.

What to check: **MM_CLIMB_002**, starting RATE **20**. Right click RATE to increase
and speed recurring releases; left click to decrease and slow them. Hold to
reach **50** or **1**, then keep holding: the value must stop at that bound.
The in-flight countdown finishes normally; a retry starts at **20** again.
