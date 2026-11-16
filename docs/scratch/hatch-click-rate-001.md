# hatch-click-rate-001

Status: **needs_review**. Playable v1, [playtest 7 W3](../playtest/2026-11-15-playtest-7.md),
correcting release-rate-control-001. Difficulty: difficult; recommendation Opus;
assigned Codex override used. **Supervisor smoke verification is pending.**

The human reported that release-rate input belongs on the world hatch. The
previous RATE panel box was the demonstrated defect; reconnecting its existing
control to hatch picking is the next dependency. On **MM_CLIMB_002**, right-click
the hatch to raise the rate, left-click to lower it, within **1..50**, initially
**20**. Hover shows **RATE 20** alongside the accepted remaining-count feedback.
The separate RATE box and its headless compositor are removed. The user's next
playtest confirms the visible result; no new scripted play route was added.

## Production path and limits

`runApplication` -> `runApplicationLeg` -> `RuntimeReleaseRateControl::create`
loads every structured hatch's stored minimum/maximum through `readHatchRecords`
and the existing `ReleaseRates` table. Its initial shared rate still comes from
the first hatch; loading leaves World's accepted release setup unchanged.

`SdlWindow::gameCoordinates` -> mouse-down -> `hatchAtPointer` -> the displayed
scene/depth -> `BackdropScene::hatchHoverTargets` -> `hoveredHatch` ->
`RuntimeReleaseRateControl::press/change` -> `World::setReleaseInterval` -> the
existing `stepRelease` recurring interval. Hatch input is consumed before right
camera grab or left skill assignment. The same actor picker keeps precedence
over hatch hover/input; panel plates, menus, focus loss and camera grab exclude
hatch input. Scale conversion remains the existing 1x/2x adapter.

Holding repeats after 0.4 seconds, then every 0.1 seconds. Release, leaving the
target, moving to another hatch, focus loss or opening the menu cancels a hold.
Pause permits rate edits without advancing simulation. The in-flight countdown,
first release, spawn order, clocks and interval-table mapping are unchanged.

**Provisional multi-hatch limit:** M06_KF_1 has two hatches, both with stored
**1..99** limits and initial shared rate **30**. Clicking either changes the
**shared rate/cadence**, clamped to the clicked hatch's own limits. Every hatch
therefore displays the same effective rate; before a click it can be outside a
different hatch's limits. Independent hatch rates/cadences are not implemented.
multi-hatch-release-001 stays parked, including its M20_MT_1 regression.

`drawHatchHover` retains the brackets/count and adds the effective rate using
the retail font. The common entry-frame fixture now expects this hover label
and omits the defective panel box; its equality assertions/timeouts are unchanged.
Provisional choices remain the accepted visible mesh/depth picking rectangle,
actor precedence, label layout, physical SDL binding and repeat/pause ergonomics.
Static evidence does not settle their exact original appearance or repeat feel.

## Static authority

Registered reference: `C:/progs/lemmings-webolution/Lemmings Revolution/Lemmings Revolution.exe`,
SHA-256 **32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f**.
REA Ghidra **12.1.4**, profile digest
`3c06738e45e144d5f073edbd3244c6bff46222f389525fe659bc92dca2ccdece`.
Used the reverse-engineer-anything skill; opened only this binary for static
analysis and closed it without snapshot arguments. No original execution occurred.

- Picker `454D6C`: `454E52` projects the hatch model origin; `454E67..454E89`
  tests +/-20 pixels, `454E9A..454EA3` requires positive hatch word `+98`,
  `454EAE` retains the hatch and `45525C` publishes it to screen `+1894`.
  New [instruction packet](../../build/hatch-click-rate-001/static/picker-instructions.json),
  evidence `ev_0f6bb91c4c68b72973269fd50c9328c0c499c5fa474e4c2238284935fa2d5d88`,
  cross-checks the retained [picker pseudocode](../../build/hover-feedback-001/static/pseudo-0x454D6C.json).
- The new [caller packet](../../build/hatch-click-rate-001/static/picker-callers.json)
  identifies `4532E2`; retained instructions show its picker call at `4537EC`,
  then `453A31..453AAF` reading that selected hatch. `455DD0` leads to rate+1,
  `455D90` to rate-1, both calling `425DE3`. Retained wrappers and instructions
  resolve these to held mouse fields `+4` (button 1) and `+0` (button 0).
  [Earlier instruction cross-check](../../build/release-rate-control-001/resume-20261116T0413/static-crosscheck.txt).
- `425DF0..425E1B` clamps against hatch `+A4/+A8`; `425E24..425E44` copies
  the indexed interval to `+B8/+BC` and rate to `+A0`, leaving countdown `+C0`.
  Hover formatting pushes `+A0` at `454B95`, independently identifying it as
  the rate. [Accepted release report](release-rate-control-001.md) and
  [hover report](hover-feedback-001.md) retain the underlying packets.

These establish the world-hatch path, not a separate RATE panel control; no
static evidence for that added box was found in this bounded trace. Selected
instructions are retained in [crosscheck.txt](../../build/hatch-click-rate-001/static/crosscheck.txt).
Original timing/fidelity gates and full-project completion criteria remain open.

## Replays and checks

Replay remains `WEBOLUTION_REPLAY 1`. An effective `r` command records rate,
interval ticks, **clicked hatch index in the previously unused third value**,
and interval seconds. Old files have zero in that slot, retaining their primary
hatch interpretation. The reader checks the selected hatch's limits and table
values, then changes the shared interval directly. Raw clicks, pixels and hold
time are not serialized. A clamped click with no effective change needs no row.
Existing replay files were left untouched.

Registered UCRT64 GCC/CMake/Ninja build passed: [build.log](../../build/hatch-click-rate-001/build.log).
Focused checks: **frontendentry PASS (30.10 s), hatch_rate_units PASS (0.17 s)**;
[log](../../build/hatch-click-rate-001/focused.log), [JUnit](../../build/hatch-click-rate-001/focused.xml).
The new legacy unit guards per-hatch limits (including E10_KF_1's different
fixed values), countdown preservation, hold cancellation and replay round trips.
`release_rate_units` was removed because it encoded the superseded RATE panel
geometry; it was not repaired. Smoke registrations/assertions/timeouts are unchanged.

Ran the built `webolution.exe --game-dir "Lemmings Revolution" --replay <file>`
on every promoted recording. All twelve playtest-7 results are **PASS, exit 0,
won=1, completed=1**; exact file names, commands' results and individual logs:
[replays.json](../../build/hatch-click-rate-001/replays.json).

| Playtest-7 level | Saved/required |
| --- | ---: |
| MM_CLIMB_002 | 10/1 |
| MM_FLOAT | 10/1 |
| MM_BOMB | 17/10 |
| MM_BLOCK_001_A | 24/15 |
| MM_BUILD | 20/10 |
| MM_MINE | 20/10 |
| MM_BASH_002 | 10/10 |
| MM_DIG_000 | 19/10 |
| MM_W_012 | 16/10 |
| MM_BLOCK_BOMB_A | 12/10 |
| MM_BASH_MINE_WLR1 | 20/10 |
| M06_KF_1 | 93/50 |

The three playtest-8 recordings also PASS: MM_BASH_DIG_001_A 20/10,
MM_BUILD_BLOCK_A 15/10, MM_FLOAT_CLIMB2 10/6. Replays exercise simulation and
file compatibility, not world picking or visible playability. No mandatory
smoke/full profile was duplicated; supervisor smoke and review remain pending.

Changed: `src/app/{main.cpp,RuntimeSkillPanel.h,RuntimeSkillPanel.cpp,RuntimeHud.cpp,
HoverFeedback.h,Replay.hpp}`, removed `ReleaseRateDisplay.h`, the shared
`src/__test__/HatchEntryFixture.h`, replaced legacy `test_release_rate.cpp` with
`test_hatch_rate.cpp`, `cmake/release_rate.cmake`, and this report.
Incoming edits are preserved in [entry/](../../build/hatch-click-rate-001/entry/);
[task-only.patch](../../build/hatch-click-rate-001/task-only.patch) isolates this diff.
Supervisor PID 26408 holds RepositoryLease. Get-Process found no clone/build/test
runner at entry; CIM command-line access was denied. No protected controls were
edited. Runs used private LocalAppData `build/hatch-click-rate-001/appdata`;
frontendentry uses its injected save, and unit/replay paths open no SaveStore.
Normal/installed saves were not used or changed. No original/oracle, debugger,
hooks, capture, installs, subagents, background workers, commits or pushes ran.

Handoff: hatch input and rate hover are implemented; two focused checks and all fifteen promoted human-win replays pass.
Next: supervisor smoke and independent review, stop after review; human playtest confirms W3.

**What to check:** **MM_CLIMB_002 / Just Climb**, hover the hatch: read **RATE 20**,
right-click to raise it and left-click to lower it; holding stops at **50/1**.
No separate RATE panel box should appear. Release or move away to stop repeats;
pause should freeze lemmings while permitting rate edits. On **M06_KF_1**, click
each of the two hatches: either changes the same displayed/shared rate within
**1..99**, without right camera grab or skill assignment on the hatch.
