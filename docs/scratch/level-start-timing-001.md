# level-start-timing-001

**Needs review.** Playtest 30 W5 asks for the first hatchling to leave at
4.04–4.12 s while **Let's Go!** is still visible. The original recording shows
a map entry with 20 lemmings and a target of 10. The clone probe used **Just
Climb** (`MM_CLIMB_002`, node 0) and the public map and Try again menus with a
private save. [Playtest 30](../playtest/2026-11-23-playtest-30.md) ·
[Original recording analysis](../../build/lead/level-start-gif/analysis.md)

## Clone timing

Level time starts at 0 on each entry. At 20 ms per frame, the first rendered HUD
showed 05:00 at 0.02 s. The table compares the retained baseline to this revision.
Times are in seconds from level-clock start and exclude time spent loading.

| Route | First HUD decrement, before → revised | Title draw / removal, before → revised | Door end, before → revised | First Out = 1, before → revised |
| --- | ---: | ---: | ---: | ---: |
| Map | 0.20 → 1.00 | 2.00 / 4.50 → 2.00 / 4.50 | 2.80 → 2.80 | 5.80 → 4.10, title visible |
| Try again | 0.20 → 1.00 | 0.02 / 2.50 → 0.02 / 2.50 | 2.80 → 2.80 | 4.10 → 4.10 |
| Fresh direct play | 0.20 → 1.00 | No overlay | 0.74 → 2.80 | 2.04 → 4.10 |

[Baseline map and Retry](../../build/level-start-timing-001/before-front.csv) ·
[Baseline direct](../../build/level-start-timing-001/before-direct.csv) ·
[Revised map and Retry at 20 ms](../../build/level-start-timing-001/revision-frontend-20ms.csv) ·
[Revised direct at 20 ms](../../build/level-start-timing-001/revision-direct-20ms.csv)

At 30 ms the opening update spans 2.790–2.820 s. The prior implementation's
source trace first reached **Out = 1** at 4.140 s. The revised run shows **Out = 1**
at 4.110 s on all three routes, with the map title drawn from the 2.010 s frame
through the frame before 4.530 s. The variable-frame run sampled map title draw,
door end, first release and title removal at 2.001, 2.810, 4.100 and 4.504 s.
The HUD showed 04:56 at the map release. Retry sampled first release at 4.103 s,
close to the timing the user judged about right in playtest 28. At 60 Hz both
routes first released at 4.100 s. [30 ms map and Retry](../../build/level-start-timing-001/revision-frontend-30ms.csv) ·
[30 ms direct](../../build/level-start-timing-001/revision-direct-30ms.csv) ·
[First map release frame](../../build/level-start-timing-001/revision-frontend-30ms/map-first-release.png) ·
[Variable-frame run](../../build/level-start-timing-001/revision-frontend-0ms.csv) ·
[60 Hz run](../../build/level-start-timing-001/revision-frontend-16.666666666666668ms.csv)

## Static evidence and production path

REA Ghidra 12.1.4 inspected the registered x86 EXE with SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Loader `0x45C9AB` calls level start at `0x45CAB3`, sets the title-active byte
`+0x26024` at `0x45CAD6`, and seeds `+0x25FF8` at `0x45CB39` and `0x45CBB2`.
The title/music update `0x45CC09` reads those fields at `0x45CE7C..0x45CEE6`.
The frame loop separately calls level update `0x4423D2` at `0x45DBE5`, which
dispatches the hatch update through `0x442BDC` and vtable entry `0x4B0F64`.
Level start resets the level clock at `0x4450E0` and schedules its 1 Hz event
with countdown action `0x4458B0`. [Loader and frame instructions](../../build/level-start-timing-001/static/) ·
[Retained timer trace](gate-cadence-research-001.md)

Hatch update `0x425A95` calls the model update `0x451121` through `0x432383`.
The model retains time past the end of the clip at `0x451175..0x4511EA`.
The hatch tests clip completion at `0x425B17`, reads the remainder at
`0x425B3B`, loads the 1.3 s seed from `0x4B0F48` at `0x425B44..0x425B56`,
then subtracts the remainder at `0x425B8A`. The hatch branch uses clip state,
its armed flag and remaining population. It has no title test. The recurring
interval is added at `0x425D3C..0x425D4B`. [Hatch instructions](../../build/level-start-timing-001/static/revision-read_function_instructions-0x425A95.json) ·
[Hatch pseudocode](../../build/level-start-timing-001/static/revision-procedure_pseudo_code-0x425A95.json) ·
[Model instructions](../../build/level-start-timing-001/static/revision-read_function_instructions-0x451121.json)

`runApplicationLeg` passes the opening offset from `LevelStartSequence` through
`RuntimeSimState::advanceElapsed` into `World::advanceElapsedMsWithReleaseGate`.
The segment containing the opening arms `World::stepRelease` and credits only
that segment's remainder. In the measured 30 ms opening frame this gives the
initial countdown about 20 ms of credit. The accepted door clip, hatch order,
release rates, recurring countdown, title renderer and music trigger retain
their existing paths. The title and doors still follow frame time, and actors
and HUD events follow admitted World time. The visible event time depends on
frame sampling. Exact clock agreement during stalls or fast-forward remains
unverified. [Source diff](../../build/level-start-timing-001/revision-source.patch)

## Checks

- Built `smoke_tests`, `test_cameraentry_after` and `test_hatch_timing_after`
  with the registered UCRT64 toolchain. Six focused smoke checks passed
  (`sharedtiming`, `runtimehud`, `entrystartcue`, `frontendwindow`,
  `frontendentry`, `levelreturn`). The legacy camera check now compares its
  fresh direct frame to the independent full-clip fixture, replacing its old
  arm-normalized expectation. The camera pose and pixel checks remain. The
  legacy hatch units cover the 30 ms regression. Both legacy executables were
  built for this revision, with execution reserved for sign-off. [Build log](../../build/level-start-timing-001/revision-build.log) ·
  [Focused smoke log](../../build/level-start-timing-001/revision-focused-smoke.log)
- All 32 promoted recordings reproduced through `webolution --replay`. The
  ledger gives the result and step for each. This includes 31 wins and the
  playtest 29 Lone Ranger return to the map at step 12893. The promoted files
  exercise version 1. Source review confirms that version 2 also keeps its
  historical Boolean arming. A new version 3 direct recording stored the
  opening offset at step 93 (clock 2820 ms), first released at step 136 (clock
  4110 ms) and replayed through step 207.
  [Promoted replay ledger](../../build/level-start-timing-001/revision-replays.csv) ·
  [New recording summary](../../build/level-start-timing-001/revision-direct-record-summary.txt) ·
  [New playback log](../../build/level-start-timing-001/revision-direct-replay.log)

Handoff — The door-opening update now credits its remainder on map entry, Retry and fresh windowed direct play.  
Next — Supervisor smoke profile pending, followed by independent review and human playtest.

**What to check** — Enter **Just Climb** from the map. At about 4.1 s, **Out**
should reach 1 while **Let's Go!** is visible and the HUD shows 04:56. Choose
**Try again** and expect the title to clear at about 2.5 s and the first
hatchling at about 4.1 s from the start of the level timer.
