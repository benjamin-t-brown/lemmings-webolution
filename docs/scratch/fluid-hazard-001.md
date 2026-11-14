# fluid-hazard-001: species-aware live fluid hazards

Status: **needs_review**. Playable v1, human playtest 4 T4. Difficulty:
difficult; assigned recommendation Sonnet-class, executed with the explicit
Codex override. Full registered main/recovery verification belongs to the
supervisor after worker exit; the focused checks below are not that verification.
Latest review revision: [recorded gadget win replay](#review-revision-recorded-gadget-win-replay).

## Failure, installed route and production dependency

The reported "Block and Build" is **map node 14, `MM_BUILD_BLOCK_A`, installed
title "Build and Block"**. The registered demonstration asserts both map
lookups using `MapScene`, following `map-level-names-001.md`. It releases the
installed hatch naturally, with no skills or actor injection. The second natural
route is node 8, `MM_W_012`, "Introducing - Water Lemmings".

Before: the first ordinary actor at **12,200 ms, (176,66), sheet 0** has feet at
original Y=29, just above water receiver stream index 8 (id 8, kind 0,
X=[176,184), bottom Y=4, height 24.5, mean surface Y=28.5). Its ground probe is
row 28 and has **no 0x40 hazard bit**. At **12,240 ms** it becomes a Walker at
**(176,67), feet Y=28**, inside the water; ground row 27 has the water-zone bit,
still no 0x40. It then walks across the pool, alive.
`before/MM_BUILD_BLOCK_A-world.csv` retains each position/state
sample, and `*-level-data.txt` records installed hatch and receiver bounds.

This is a missing fluid-object hazard, rather than a missed static 0x40 cell.
`loadRetailLevel -> Terrain::fromCollisionGrid` separates the 0x40 hazard plane
from 0x200 water/0x4000 acid zones. `CollisionGrid::solid` admits nonzero,
landable cells; the clone's existing walking/landing path can therefore stand
on the water cells. The old `World::step` / `stepFaller` only attached Drown
for `Terrain::waterHazard` (0x40). Neither evaluated the live receivers against
the hatch species. The accepted fluid rendering/lever work already carries
those receivers, so adding their missing hazard rule is the next dependency.

## Static rule and its limits

Reference: repository-local `Lemmings Revolution/Lemmings Revolution.exe`, x86,
SHA-256 **32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f**.
The interrupted attempt retained Ghidra static evidence under
`build/fluid-hazard-001/static/`; this resume reused it for the species rule.
One new static Ghidra instruction lookup corroborated the retained sinking
pseudocode. The session was closed without snapshot arguments. No original
execution, original-byte oracle, debugger, hooks, capture or install tooling ran.

| Fluid kind | Ordinary, sheet 0 | Water, sheet 1 | Acid, sheet 2 |
| --- | --- | --- | --- |
| 0 water | drown | survive | drown |
| 1 lava | drown | drown | drown |
| 2 acid | drown | drown | survive |

Instruction cross-check: **0x41C321..0x41C36F** reads lemming+0x54 (current
fluid object), fluid+0x128 (kind), and lemming+0x4C (species). Kind 1 branches
unconditionally to mismatch; sheet 1/kind 0 and sheet 2/kind 2 branch to the
matching continuation. Mismatch constructs state type **0x23** at
**0x41C3BA -> 0x4041FD**, attaches at **0x41C3F1 -> 0x42E696**.
`asm-0x41BFFC.json`, evidence
`ev_b6cf5025afe449a5f3731b4bdf9f406c8b067cbc461f961868fe71ac7298f41e`,
corroborates `lookup-1.json` pseudocode,
`ev_845c208db337203d9c23cf957c5624dec390273263fe8d01e8c0c10de2cb79ba`.
**0x404261** sets the "drown" animation; both `lookup-2.json` and
`asm-0x404261.json` retain that entry routine.

Species provenance is already supported by `phase-d1-release-trigger.md`:
the **0x425A95** hatch step pushes hatch+0xB0 to the constructor
**0x42DE51**. Retained `asm-0x42DE51.json` explicitly shows **0x42E05F**
copying the constructor type argument to lemming+0x4C (evidence
`ev_9b64edaf57979b5f6f0a40deff5b968973ef3152c3a3f72f29ef5fd74dea9b81`).
The sheet names are supported by `phase-c1m-sheet-file-read.md` and
`decomp-lemmings/docs/evidence/actor_resource_selection.md` (table 0x4BFFC8).
Clone provenance: `LevelObjects::readHatchRecords` reads serialized `value_b0`
into `HatchRecord::type`; `RetailLevel.cpp` copies it to `Entrance::sheetType`;
`World::spawn` copies it to `Lemming::sheetType`. No model-name guess or new
species field is used. The new installed-data tests assert ordinary/water
hatches on the two natural routes and a sheet-2 hatch on `M20_MT_1`.

**Provisional contact and presentation:** the writer that selects original
lemming+0x54 and its exact immersion threshold have not been traced. This fix
uses the visible feet inside the live mean volume: original row
`terrainHeight - 1 - y`, vertically `[minY,minY+height]`,
horizontally `[minX,maxX)` with cylinder wrapping; height <= 0 is dry. It
ignores visual waves. This is a player-facing rule, not an original depth pass.
The original fluid state is distinct from static-bit drowning (type 0x22):
`pseudo-0x404372.json`, evidence
`ev_bc0f70e00a99ca2e4396ca3b25046db72203789fa1c22a25da621349b10cef9b`,
shows sinking toward the receiver bottom, fade and effects. This clone reuses
the existing Drown animation, 2-second duration and loss counter, without
retuning them. Sinking, bubble/lava effects and their lifetime are not ported.
A future authorized original observation varying feet/ground depth around a
surface (including a bridge and inverted actor), followed through disappearance,
would settle contact depth and that presentation/lifetime difference.

The resume's full `resume-asm-0x404372.json` (evidence
`ev_a1a7a896a480ee5844853a0ef45f54521982dc5c2ecba85d356c412d3d7ff261`)
corroborates the position minus fluid-bottom computation at **0x4045F8..0x4045FE**,
vertical-direction/dt movement write at **0x404655..0x404675**, and return-2
branch at **0x404912..0x40492C**. This is instruction evidence, not an observed
original lifetime or justification to retune the clone timer.

## Connected implementation and demonstrations

`runApplication -> RuntimeSimState::advanceElapsed -> World::advanceElapsedMs ->
updateSegment -> stepElapsed/step -> attachFluidHazard` now tests species against
`setup_.gates`. Walking/falling substeps also check after moving, so contact
cannot be skipped inside their existing continuous movement budget. A fatal
contact clears a Blocker's field, sets `State::Drown`, resets its frame and
movement remainder, and follows the existing `stepDrown -> Dead -> dead_` path.
`LemmingAnimTracker -> injectedWalkersFromWorld -> TubeScene` uses the existing
"drown" SPR clip. The public application test asserts that clip and an advancing
temporal entry. Teleport transit and actors already dying/removed are excluded.

The current receiver `height` is read on every check, never the initial height,
stale `maxY`, or a residual zone-mask cell. Existing update order stays
lemmings -> levers -> receivers; a height changed at the end of that update is
seen by the next actor update. No new clock, release-rate change, skill retuning,
gate/mask rewrite or renderer repair was made. The accepted kind-2 height-rate
limitation remains; hazards follow its actual current height too.

After: the same natural ordinary actor enters **Drown at 12,240 ms, (176,67)**,
then **Dead with dead=1 at 14,200 ms** through the unchanged drowning clock.
Its route summary over the bounded
60-second component observation reports 13 lost and no saves. `MM_W_012`'s
natural sheet-1 actors still cross water as Walkers, with no drowning/deaths
(three saved during that component window). These are scripted clone
observations, not human victories or original comparisons.

Evidence root: `build/fluid-hazard-001/{before,after}/`.

- `MM_BUILD_BLOCK_A-world.csv`, `MM_W_012-world.csv`: 20-ms World traces;
  `*-world-summary.txt` and `*-level-data.txt`: outcomes and decoded data.
- `*-app.csv`: ordinary production application state, species, loss count,
  rendered clip and temporal entry at each 100-ms frame.
- `MM_BUILD_BLOCK_A-first-contact.png`, `MM_BUILD_BLOCK_A-drown.png` (after),
  and matched `MM_BUILD_BLOCK_A-frame-{130,138,160,200}.png`: natural basin
  contact/crossing and death presentation. `MM_W_012-first-contact.png` and
  the corresponding frame sequence retain the living water-lemming control.
- `after/contact-fixtures.csv`: placed contacts in real installed water
  (`MM_BUILD_BLOCK_A`), acid (`M20_MT_1`) and lava (`H17_mg_1`) receivers;
  all three species in Walker/Faller/Builder/Blocker states. Fatal cases check
  delayed loss and exactly one count. Supplementary MM_W_012 receiver-id-5
  fixtures send real type-4/type-5 lever messages and check dry, rising,
  full and drained heights, plus clearance one row above each surface.

Visually inspected final production framebuffers (640x480):

| Case | Before | After |
| --- | --- | --- |
| Ordinary first contact, app frame 122 / 12.3 s | [Walker](../../build/fluid-hazard-001/before/MM_BUILD_BLOCK_A-first-contact.png) | [Drown](../../build/fluid-hazard-001/after/MM_BUILD_BLOCK_A-first-contact.png) |
| Same clock, frame 130 / 13.1 s | [walking across pool](../../build/fluid-hazard-001/before/MM_BUILD_BLOCK_A-frame-130.png) | [drowning at entry](../../build/fluid-hazard-001/after/MM_BUILD_BLOCK_A-frame-130.png) |
| Water-lemming contact | [before](../../build/fluid-hazard-001/before/MM_W_012-first-contact.png) | [after](../../build/fluid-hazard-001/after/MM_W_012-first-contact.png) |

The final camera is yaw 176 / eye height 40 for Build and Block, yaw 128 /
eye height 96 for MM_W_012. At frame 130 the ordinary baseline actor walks
to X=180 with clip `walk`, temporal entry 3; after it stays X=176 with clip
`drown`, temporal entry 15. Its visible box is approximately
(315,69)..(332,86), beside the blue pool surface. The retained 138/160/200
frames continue the sequence. These small sprites are clone pixels and clip
observations, not original-frame or animation-timing comparisons.

The resumed ground-probe proposal was tightened before final verification:
feet Y=29 must remain safe above a surface at Y=28.5, while feet Y=28 are wet.
Using the ground cell would prematurely drown an actor standing above the
surface. The final placed-contact cases check that half-cell clearance too.
The earlier ground-probe observations remain under
`resume-entry/inherited-evidence/`, with their original logs.

Fixtures supplement the two unmodified natural hatch routes; they do not prove
natural acid play, every receiver geometry, or all-level playability.

## Preservation, checks and handoff

Attempt 1 was interrupted by the host memory failure, not rejected gameplay.
Its baseline/after logs and static evidence already existed at resume, despite
the scope note saying they had not run. Its regression build ended interrupted;
it is not a completed verification result. Earlier compile errors and their
logs are retained. `resume-entry/` backs up all five resumed task files.

The frozen task-entry `World.cpp` equals HEAD byte-for-byte (SHA-256
`419b9192a3824b6325eea0279222018231967db47ad2c9abb5bb5900f0703b73`).
`fluid_hazard_before` compiles its frozen `before-source/src/sim/World.cpp`
and header, with current unchanged production support/rendering. Registered
`fluid_hazard_identity` pins both frozen simulation files, and the current
`fluid_hazard_after` always registers even without an ignored baseline folder.
The existing assertions and timeouts were not changed.

Each demonstration uses its own build-directory FileSaveSink and compares the
installed/normal clone save's existence and complete bytes before/after.
It never imports or writes either normal save. Original binaries/assets remain
untracked. Pre-existing queue/status/hook-log edits were left intact; no queue,
verifier, controller, registry, permissions, shared instructions, commits or
subagents were used. The parent supervisor holds RepositoryLease; no manual
protected control edit was made by this worker.

Development checks: `resume-build.log` built the focused targets with registered
GCC/UCRT64, `-O2 -g`, assertions kept, four compile jobs to avoid the previous
host memory pressure. `resume-focused-ctest.log` passed **29/29** serial checks
in **814.70 s**, including the then-current ground-probe proposal. It is retained
as development history, not the final feet-contact result. The first added
gadget-control build missed the frozen header's Terrain include directory;
`final-build-1-failed.log` retains that error. The include path was corrected;
`final-build.log` then built it. `final-feet-build.log` builds the final hazard
rule and all selected route executables. No existing assertion/timeout was
weakened to address either issue.

The expansion gadget driver is wall-clock/public-input based and has historical
WIN/LOSS variation. The first development run was **6/20 LOSS, 0 dead** on
`MM_W_012` versus **11/20 WIN, 0 dead** in the latest pre-task expansion run
`20261114-062255`. Earlier pre-task run `20261114-045546` was **4/20 LOSS,
0 dead**. It is not a route dependent on walking fluid: every hatch is sheet 1
and every receiver kind 0, so the new mismatch helper performs no mutation
there, including for assigned Blockers. The task registers a frozen-World
control using the unchanged expansion driver and a pinned copy of the earned
private clone save (`gadget-seed/identity.json`), plus exact comparisons of both
fixed-input `MM_W_012` World/app traces. These diagnose variation without
altering the existing driver or claiming its integrity pass implies a win.
Exact final results and remaining outcome limits follow below.

Final production checks: [final-focused-ctest.log](../../build/fluid-hazard-001/final-focused-ctest.log)
passed **32/32 in 334.03 s**, with four test jobs using the accepted resource
isolation. This includes the final feet-contact production code, World units,
all registered baseline/expansion/targeted play routes and their restart
checks, both lever variants and the new hazard/identity/gadget controls.
The demonstration-only camera and screen-box trace columns were refined
afterward; [final-image-build.log](../../build/fluid-hazard-001/final-image-build.log)
and [final-images-ctest.log](../../build/fluid-hazard-001/final-images-ctest.log)
then passed **4/4 in 85.40 s** on the final task sources, regenerating both
image sets, all fixtures and the water control. The latter compares every
actor's position/state/frame/motion budget throughout the 60-second World
route, plus the 300-frame production app trace: **byte-identical for MM_W_012**.
Both final demonstration summaries assert installed/normal saves unchanged.
`final-source-hashes.json` pins the eight source/build files; LF checks and
task-scoped `git diff --check` pass.

| Registered route | Final observed outcome | Comparison / retained result |
| --- | --- | --- |
| Baseline Climber / Floater / Bomber | WIN 10/10, WIN 4/10, WIN 15/20; restart verified | `route-regressions/final-baseline/` (winning outcome retained) |
| Both fluid-lever variants | WIN 20/20, reload/restart verified | `route-regressions/final-fluidlever/` |
| Expansion, run `20261114-075826` | Climber WIN; Blocker WIN 23/30; Builder LOSS 0/20 | Blocker WIN and Builder LOSS match pre-task run `20261114-062255` |
| Targeted, run `20261114-075939` | Blocker LOSS; lever stage blocked by missing unlock | Same outcomes in latest pre-task run `20261114-062411`; final drowning sites shifted one cell with the corrected contact depth |
| Expansion gadget, run `20261114-075826` | **LOSS 8/20, 0 dead** | Latest pre-task run was WIN 11/20; earlier pre-task LOSS 4/20. This historical win was **not re-established** here. |
| Frozen task-entry gadget control | **LOSS 9/20, 0 dead** | `gadget-before/runs/seed/step-4-gadget/play-result.txt`; same earned private input save, unchanged public-input driver, authentic pre-task World |

The gadget control and exact fixed-input water comparisons establish that a
loss is reproducible without the hazard implementation; they do not establish
identical real-time SDL actions or repeat the historical win. This remains an
explicit limit on the strict route-outcome acceptance criterion for independent
review, rather than being labelled a fluid-dependent changed outcome or hidden
by CTest's driver-integrity pass. No route/controller/timing repair was made to
obtain a win. The successful dedicated lever routes retain their scoped evidence.

Full registered main/recovery verification and independent review are **pending
supervisor execution**. These development checks are not a substitute. No
historical timing, rendering-fidelity or full-project completion gate is changed.

Handoff: live fluid hazards and installed-route regression evidence are ready for needs_review; original contact/lifetime fidelity remains explicitly provisional.
Next: supervisor runs full registered main/recovery verification on final content, then independent acceptance review; a human rechecks T4 on the ordinary clone.

## Review revision: recorded gadget win replay

The independent review of run `20261114T073603-33172-9b04b526` supported
criteria 1 and 2. Its [supervisor verification](../../build/agent-runs/20261114T073603-33172-9b04b526/verification.log)
passed **248 main / 84 recovery tests**, but criterion 3 remained unmet:
the supervisor's `playable-expansion-001/runs/20261114-081058/step-4-gadget/`
lost **0/20**, unlike the earlier recorded **11/20 WIN**. This supersedes the
earlier worker's pending-verification statement for that historical content;
the rejection and all earlier losing outcomes remain evidence.

This revision leaves the hazard implementation, original assessment driver,
existing assertions/timeouts and production timing unchanged. Entry backups are
under `build/fluid-hazard-001/review-resume/entry/`. Comparing all **1,666**
entries of the prior verification source manifest finds no changed production
source or missing file; differences are the replay's CMake registration and
the parent's queue/generated status updates. The new test is an added file.
[Comparison](../../build/fluid-hazard-001/review-resume/prior-verification-source-comparison.json).
The supervisor owns the lease; no protected control was edited by this worker.

A bounded run of the unchanged assessment executable on its pinned earned
private seed again lost **2/20, zero deaths**:
`review-resume/gadget-final-1/runs/earned-seed/step-4-gadget/`.
Its first Blocker was actor 2 at (214,41), whereas the winning trace first
placed actor 11 there, after eleven actors had passed. This diagnoses the
different input route; a passing assessment exit code still does not assert a win.

The added CTest **`fluid_hazard_gadget_replay`** replays the winning route's
two placements through `runApplication` and public SDL events:
Play Current Game -> map node 8 -> public fast-forward and Blocker selection ->
actor 11 at (214,41), actor 12 at (165,41) -> results -> Continue -> map.
It uses the existing bounded integration-test clock, **20 ms per frame**,
to sample the recorded input sites repeatably. No callback writes World,
terrain, gates, stock, progress or clock state. This is a deterministic
public-input replay of the recorded winning placements, not a claim that the
unchanged adaptive wall-clock assessment now wins every invocation.

The retained final-source replay achieves the recorded **WIN 11/20, zero deaths**:

| Actor | Recorded winning Blocker attachment | Final replay attachment |
| --- | --- | --- |
| 11 | (214,41), tick 324 | (214,41), tick 324 / 64,880 ms |
| 12 | (165,41), tick 387 | (165,41), tick 387 / 77,480 ms |

The literal click samples differ (ticks 323/386 versus 320/383 in the wall-clock
trace), but the attached actor states, sites and tick boundaries match.
The test asserts both exact sites, at least eleven saves, the installed
20-total/10-required result, zero deaths, return to map and successful persistence.
A fresh SaveStore reads the written private file: its entire codec image must
equal the earned seed with only node 8 marked complete. Installed and normal
clone saves remain byte-identical, including their existence state.

Evidence under `build/fluid-hazard-001/review-resume/`:

- [winning-route-comparison.json](../../build/fluid-hazard-001/review-resume/winning-route-comparison.json)
  compares the recorded actor attachments with the final production trace.
- [replay-final/replay.csv](../../build/fluid-hazard-001/review-resume/replay-final/replay.csv),
  [summary.txt](../../build/fluid-hazard-001/review-resume/replay-final/summary.txt),
  placement images, private save and completed map retain the full replay.
- [results.png](../../build/fluid-hazard-001/review-resume/replay-final/results.png)
  visibly reports "You Needed 10 / You Saved 11". It is byte-identical to the
  independently inspected first replay's result image.
- `replay-final/source/` retains the actual final test, production dependencies
  and CMake sources; `source-hashes.json` also identifies the executable.

Development validation: first replay **1/1 passed in 169.50 s**. After adding
exact-site and complete-save-image assertions, the final build succeeded and
[replay-final-ctest.log](../../build/fluid-hazard-001/review-resume/replay-final-ctest.log)
passed **3/3 in 170.22 s**: frozen-source identity, earned-seed identity and the
asserted winning replay. The new target initially omitted the production object
members at link time; its CMake source list was corrected. A shell quoting error
prevented one CTest invocation from starting; that log is retained separately as
`replay-ctest-invocation-failed.log`. Neither changed an existing check or timeout.
No full suite was duplicated. Full registered main/recovery verification on this
revision, including the added test, remains **pending supervisor execution**.

Changed in this revision: `src/__test__/test_fluid_hazard_gadget_replay.cpp`,
`cmake/fluid_hazard.cmake`, this report. The task's retained changes remain
`World.{cpp,h}`, `test_fluid_hazard.cpp`, the CMake include and the three other
`fluid_hazard_*.cmake` helpers. No queue, verifier, controller, tool registry,
permission, shared instruction, model pin or unrelated source was edited;
pre-existing edits remain. No original execution, static REA session, subagent,
background work or commit was performed in this revision.

Unknowns are unchanged: original contact depth and sinking/effects lifetime
remain provisional; the adaptive wall-clock assessment still has variable input
sampling and outcomes. The final replay now supplies the previously missing
winning-route observation on final production source. It is regression evidence,
not a human playtest or a historical-fidelity pass.

Handoff: fluid-hazard-001 remains needs_review; the final-source recorded gadget route now wins 11/20 with exact placement evidence and asserted private persistence.
Next: supervisor runs full main/recovery verification and independent acceptance review; a human rechecks playtest T4 in the ordinary clone.
