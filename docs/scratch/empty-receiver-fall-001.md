# empty-receiver-fall-001 — dry receivers no longer provide a lid

Status: **needs_review**; full main/recovery verification and acceptance are
pending supervisor execution. Playable v1, playtest 5 U3. Difficulty: difficult;
assigned Sonnet-class recommendation, explicit Codex implementation override.
No original execution, hooks, debugger, capture, original-byte oracle, installation
change, subagent, control edit or commit was performed.

## Failure and production path

Installed route: `MM_W_012`, “Introducing - Water Lemmings”, ordinary hatch
release with no input. The authentic task-entry clone wins **20/20, zero dead**.
Actor 0 reaches the first lower basin at X=222. At **23,120 ms** it becomes a
Walker at **(222,67)**, visible feet original row 28, ground probe row 27.
That raw cell is **0x200**, converted to `Material::Earth`, although the loaded
live water-zone bit is **false** and that receiver's height is **zero**.
It later crosses the other dry groups too, including X=159 at 46,120 ms.

`loadRetailLevelReport -> Terrain::fromCollisionGrid -> CollisionGrid::solid`
admitted every nonzero landable word as material. Then
`linkLevelGates -> FluidGate::initialise` correctly cleared the dry receiver's
water-zone plane, but left the previously constructed Earth material behind.
`runApplication -> RuntimeSimState::advanceElapsed -> World::advanceElapsedMs ->
updateSegment -> stepElapsed/step -> stepFaller/stepWalker -> solidAt` therefore
landed and walked on a permanent baked fluid volume. This was a simulation
support defect, not a wrong initial fluid state or a missing lever message.

Retained evidence under `build/empty-receiver-fall-001/`:

- `before/natural.csv`, `before/cells.csv`, `before/natural-summary.txt`: complete
  natural actor trace, raw/loaded cell flags and outcome. X=152..167 and
  216..223 have raw 0x200 cells at original rows 4..27; the real floor at row 3
  is **0x3**, not a fluid-only word or static 0x40 drowning hazard.
- [Before application image](../../build/empty-receiver-fall-001/before/first-lower-pit.png),
  `before/app.csv`: production application at the first dry-pit crossing.
- `entry/` preserves task-entry files. `baseline-provenance.json` proves the
  frozen World.cpp, World.h, RetailLevel.cpp and old gadget replay match this
  worker's supervisor entry manifest, run `20261115T012330-45884-36de11d1`.
  `empty_receiver_identity` pins these sources. No resumed implementation was
  substituted for a pre-task baseline; the application/rendering sources are
  unchanged by this task.

## Original evidence and provisional choices

Reference: repository-local `Lemmings Revolution/Lemmings Revolution.exe`, x86,
SHA-256 **32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f**.
Targeted Ghidra 12.1.4 static inspection used the project profile and was closed
without snapshot arguments. Complete returned evidence is in this task's
`static/` directory, including pseudocode and instruction cross-checks:

- `walk-pseudo.json`, evidence `ev_2404271ac1872fef0f6f409367b188d143e8b10bae96fa6c1aaf82f385aa042a`,
  and `walk-asm.json`, `ev_1e3f30e2c7dccedb693292e1298223a7717f6c09b40c4dc6f7a56c1cdec1e454`.
  **0x41C321..0x41C36F** checks lemming+0x54, receiver kind+0x128 and species+0x4C;
  mismatches attach the drowning state at **0x41C3BA / 0x41C3F1**. A match sets
  byte+0x4A at **0x41C426..0x41C429**. Terrain wall probing separately tests
  bit 1 at **0x41C47A..0x41C482**. The continuation compares ground words with
  zero at **0x41C6EF / 0x41C732 / 0x41C774**, constructs fall via **0x41C7A0**,
  and attaches it at **0x41C7D7**. The match flag suppresses the ordinary
  terrain Y snap at **0x41C80E..0x41C833**. This is not a
  claim that every nonzero cell is an immutable solid wall.
- `fluid-step-pseudo.json`, `ev_de36c609c78d14612b0e0b2ff3147eb6f157db6bf9c4de6abd004693bf2d66ef`,
  and `fluid-step-asm.json`, `ev_2ae680649aeb679063787aa74c121319033de622238cba8a5591c95165c1ffc2`:
  **0x42B24D** updates height+0x1A8 and maxY+0x68 and calls **0x43127E** to
  clear the previous volume during drain, then **0x4311D0** for the shortened
  positive volume. Raise also repaints the live volume. The accepted height,
  request, mask ordering and wave tail are unchanged.
- Accepted [lever-render-001](lever-render-001.md) retains reader
  **0x42ACAC**, low-height copy **0x42AEFF..0x42AF1D**, dry water-mask clear
  **0x42AF32..0x42AF3D -> 0x43127E**, and the initial-face fan-out. Accepted
  [fluid-hazard-001](fluid-hazard-001.md) establishes the species mismatch rule.

The human [original session 2](../playtest/2026-11-14-original-session.md)
and reviewed [pit contact sheet](../playtest/reference/original-2026-11-14/s2-pit-1.jpg)
show lemmings falling inside an empty lower container and all ten being lost,
with zero saved. Session 1's `pit-a.jpg` shows water lemmings traveling on the
filled surface after a lever pull. These are retained human observations;
this worker did not run the original. They supersede the earlier report's
unobserved-empty-pit limitation, without changing its correct initial-height finding.

**Provisional:** the exact writer selecting original lemming+0x54, fractional
surface/contact threshold, wave interaction and moving-surface rider update
have not been reconstructed. The clone uses the live mean surface, with feet
at `floor(minY + height)` in original rows and support one row below. Positive
height provides support; zero/negative height does not. Matching upright
walk/fall actors inside the volume ride its live mean height; a retreating
surface allows ordinary step-down/fall behavior. Waves remain visual. An
authorized original per-frame feet/contact trace during fill and drain would
settle rounding, rider attachment and wave effects. Inverted surface riding
and submerged stationary skill states are not newly established here.

The original contact sheets cannot distinguish splat death from a special
container-floor death. This implementation uses ordinary clone fall/splat
rules and adds no special floor kill. Original fill duration, release counts,
timing gates and full-frame fidelity remain outside this fix.

## Implementation and after behavior

`RetailLevel.cpp::linkLevelGates` removes material only for **fluid-only raw
words within registered receiver bounds/high height**. It retains words with
actual terrain or other flags, all real floors/walls, and runtime-built bridges.
The raw CollisionGrid parser and general Terrain conversion remain unchanged.

`World::groundAt` separates current surface support from solid terrain used for
walls and skill material. Walk/fall landing and Blocker ground/assignment
checks use it. The latter preserves Blocker support after the bogus material
is removed. `attachFluidHazard` retains the accepted species hazard rule and
snaps only matching upright walk/fall movers that are actually inside fluid
to its mean surface. It does not pull actors down from real bridges above it.
The header change adds a private method only, with no field/layout change.

After, the same natural actor remains a Faller at **(222,67), 23,120 ms**,
continues through the dry receiver to **(222,91), 24,320 ms**, and becomes a
Splatter with **fallDistance=50**. The real floor is original row 3. The
unchanged `Rules::splatDistance=30` causes the loss. The no-input route ends
**0 saved / 20 dead**. This uses the clone's existing installed release setup;
it is not a claim that its count/timing matches the original's observed ten.

The registered `empty_receiver_after` also tests real lower receiver IDs 5/6:

- A walker starting at the nearby low ledge falls to Y=91 in an empty basin.
  Its shorter 25-cell fall survives, as ordinary fall rules require.
- After real type-4 messages and the unchanged height ramp, a sheet-1 actor
  crosses the filled [152,168) surface, while a sheet-0 actor enters Drown and
  counts exactly one loss through the existing timer.
- Live type-4/type-5 transitions move a water actor with rising/draining height,
  within one cell of the mean surface. Slow drain can use normal step-downs;
  a separate dry case proves the floor fall. A supplementary constructed
  bridge remains supporting material across an empty receiver.

Reviewed current images (640×480):

| Case | Evidence |
| --- | --- |
| Natural dry-pit fall | [inside](../../build/empty-receiver-fall-001/after/inside-empty-pit.png), [floor splat](../../build/empty-receiver-fall-001/after/pit-floor-splat.png) |
| Matched natural route frames | `before/` and `after/frame-{232,238,243,280}.png`, with `app.csv` |
| Filled water, sheet 1 | [walks at the surface](../../build/empty-receiver-fall-001/after/fixture-phase-1-sheet-1.png) |
| Filled water, sheet 0 | [Drown at entry](../../build/empty-receiver-fall-001/after/fixture-phase-1-sheet-0.png) |

The fixture images use the production scene/actor renderer, actual World state
and the state's clip at entry zero; they do not establish natural gameplay or
animation lifetime. The dry-pit images come from `runApplication` with natural
hatch releases, animation and movement presentation. The remaining U2 drowning
look and U4 species sprites are separate tasks, not repaired here.

## Regression corrections, checks and preservation

The historical `fluid_hazard_gadget_replay` WIN depended on dry-pit support.
The same public inputs still attach actor 11 and 12 Blockers at ticks **324 / 387**.
The current route instead reaches **0 saved / 11 dead**, with dry-floor Splatter
observed and all lower receivers still empty. Fewer than ten can survive, so
it closes through SDL_QUIT and asserts the private progress remains unchanged.
The frozen original driver and entry simulation/loader remain registered as
`empty_receiver_gadget_before`, retaining the original WIN assertion.

Two earlier assertions encoded this defect and are explicitly corrected:

- The water-hazard test still requires species survival in water. It now
  separately requires dry-pit deaths, rather than requiring zero deaths on the
  whole MM_W_012 route. Its exact World/app before/after comparison is retained
  until the first dry receiver, including every spawned actor in the World
  prefix; both nonempty prefixes match byte-for-byte.
  Full later dry/filled outcomes are asserted by this task. The earlier World
  baseline is paired with the authenticated pre-task loader so its phantom
  material is not silently changed underneath it.
- The lever-render no-input application route now asserts zero lever throws,
  empty lower receivers and losses before reaching the lever. Both renderer
  versions retain exact actor/receiver trace equality. Its separate real-lever
  activation/render fixtures remain intact; the old no-input route is no longer
  represented as a valid ordinary lever solution.

Existing test timeouts, static identities, save codec, timing and controller
settings remain unchanged. The initial development replay timed out at its
unchanged 300-second CTest limit while awaiting the former win. The revised
loss check stops only after both original placements and a mathematically
unwinnable loss count. `focused-1.log` and `focused-2.log` retain this development
history, including the corrected fixture assumption that a gradual one-cell
drain must always enter Faller (ordinary step-down can follow it instead).

`final-focused-ctest.log` passes **21/21 in 238.23 seconds**, including the
three authenticated identities, both receiver demonstrations, both historical
gadget outcomes, water controls, real World/retail units and fluid/lever render
checks. This precedes a formatting/wrapping cleanup and test-comment cleanup;
`final-build.log` rebuilds the final production code and route executables.
Final registered route results are recorded below at handoff; full
main/recovery verification remains the supervisor's next action.

`route-regressions.log` passes **30/30 in 351.79 seconds** on final production
code: all registered baseline, expansion, targeted and fluid-lever play/restart
cases, plus current receiver, gadget and World/retail checks. These assessment
drivers report gameplay outcomes separately from their integrity exit status:

| Route | Current outcome | Comparison / retained result |
| --- | --- | --- |
| Baseline Climber / Floater / Bomber | WIN 10/10, WIN 4/10, WIN 15/20; restarts verified | `build/playable-baseline-001/route-{1,2,3}/play-result.txt`; accepted outcomes retained |
| Expansion Climber | WIN 10/10 | `build/playable-expansion-001/runs/20261115-014932/step-1-earn-node0/` |
| Expansion Blocker / Builder | LOSS (3 then 0 saved) / LOSS 0/20 | Same LOSS outcomes as latest pre-task `20261115-005922`; wall-clock input sampling/counts vary |
| Expansion gadget | Blocked by an unearned selectable prerequisite | Same pre-task block, not a newly tested gadget win |
| Targeted Blocker / lever | LOSS 0/30 / blocked by missing node-8 unlock | `build/targeted-play-001/runs/20261115-015102/`; same outcomes as pre-task `20261115-010038` |
| Both ordinary/stable fluid-lever assessments | **LOSS 0/20**, no lever toggle; results, reload and restart observed | `build/fluid-lever-assessment-001/{stable-camera,ordinary-camera}/result.txt`; former no-input wins depended on walking over dry receivers |
| Recorded gadget placements, authentic frozen simulation | **WIN 11/20**, zero dead, private completion persisted | `build/empty-receiver-fall-001/gadget-before/summary.txt`; both placements at ticks 324/387 |
| Same recorded gadget placements, current simulation | **0 saved / 11 dead**, no longer winnable | `build/fluid-hazard-001/gadget-replay/summary.txt`; same placement ticks, dry splats, no drowning, private progress preserved |

The two now-losing fluid assessments are explicitly justified changes to the
same unsupported no-input dry-container route, not preserved wins or new
human-play conclusions. The separate filled-receiver tests and images retain
water travel and ordinary drowning coverage. Earlier historical wins and
reports remain unchanged. No unrelated route repair was attempted.

`final-assertions-ctest.log` passes **4/4 in 41.71 seconds**, regenerating the
authentic/current receiver evidence after adding the exact fifty-cell natural
floor-splat assertion. Installed and normal saves remain unchanged on both
sides. `preservation-audit-initial.json` retains an initial broad-manifest audit
flag for the parent's dispatch update to `agent/tasks.json` (mtime 01:23:31 UTC,
task status in_progress, this run ID); the classified final audit distinguishes
that pre-worker control update from source changes. No worker control write occurred.
`final-water-prefix-ctest.log` passes **3/3 in 62.97 seconds** after retaining
every spawned actor in the strict pre-dry World comparison; both prefixes are
nonempty and byte-identical. The corresponding builds are retained as
`final-assertions-build.log` and `final-water-prefix-build.log`.
Builds use registered UCRT64 GCC 16.1.0, C++23, `-O2 -g`, assertions enabled,
with at most eight build jobs. No full verifier profile was run by this worker.

All new application demonstrations use private saves below this task's build
directory; existing drivers retain their private paths. Complete installed and
normal clone save bytes/existence are checked and unchanged. No user save was
imported/reset. The parent supervisor owns RepositoryLease; only production,
test/build registration and this report were edited. Pre-existing edits are
preserved. `final-source-hashes.json` and the task-scoped diff/line-ending check
record final content; original binaries/assets remain untracked.

Task files: `src/sim/World.{h,cpp}`, `src/sim/RetailLevel.cpp`, new
`src/__test__/test_empty_receiver.cpp`, `test_fluid_hazard.cpp`,
`test_fluid_hazard_gadget_replay.cpp`, `test_lever_render.cpp`, new
`cmake/empty_receiver{,_identity}.cmake`, `cmake/fluid_hazard.cmake`,
`cmake/fluid_hazard_water_control.cmake`, one include in `CMakeLists.txt`, and
this report. The verification retry below also changes `test_levergates.cpp`
and `test_hatch_timing.cpp`. Existing changes to CMake/application render files are not this
task's changes and were retained.

## Verification retry, 20261115T022549-45884-0a63ad1f

The preceding supervisor run
[`20261115T012330-45884-36de11d1/verification.log`](../../build/agent-runs/20261115T012330-45884-36de11d1/verification.log)
passed **261/264 main tests** and failed `levergates` and both
`hatch_timing_routes` variants. This supersedes the earlier worker's pending
verification statement for that content; it is a failed full run, not an
acceptance. Recovery verification and full verification of this retry remain
the supervisor's responsibility. No full profile was repeated by the worker.

The production fix and its header are unchanged in this retry. Incoming source
hashes, backups, the three failed-route outputs and the new focused build/log
are retained in `build/empty-receiver-fall-001/retry-2/`. The existing parent
RepositoryLease belongs to PID 45884. `Get-Process` showed no live clone,
compiler or CTest at retry entry; CIM inspection was denied. No protected control,
original installation or normal save was edited. Retained static instructions
were rechecked at the cited addresses without opening another analysis session;
the selected instructions are in `retained-instruction-crosscheck.txt`.

The failed lever assertion required the defective baked fluid material to remain
Earth. Its revised expectation independently derives fluid-only cells from the
raw grid and serialized receiver rectangles, and requires **exactly 3600**
material removals: the 3072 lower cells plus 528 upper cells. All actual terrain
and the hazard, acid and anti-gravity planes still compare exactly. The original
**3072** water-zone difference assertion remains unchanged. Its earlier apparent
zero difference was secondary: the old loop stopped at the first material
mismatch before counting the zone differences. The comparison now scans every
cell even after a mismatch, so material and mask failures remain independent.

The hatch replays still use their original placements, owned 200 ms frames,
release offsets, state comparisons, earned map prerequisite, fast-forward and
Results path. Both ordinary variants link the current simulation; their names
`before`/`after` refer only to startup `main.cpp`. Their MM_W_012 two-save
expectation also encoded the lid defect. The correction requires **zero saved**
and specifically actors **0 and 1** splatting on the dry floor at **(222,91)**
after a **50-cell** fall. Each loss is recorded with receiver id, current height
and request in `dry-receiver-losses.csv`, and a production image is retained.
This is a justified change to that route's saved count, not a timing adjustment.

`empty_receiver_hatch_before` independently replays the same three routes with
the authenticated task-entry World/loader and the existing hash-pinned hatch
startup baseline. It preserves the historical **two-save** assertion and
requires those same two actors to exit, with no dry-floor splats. Both source
identity tests precede it. Existing test names and timeouts are unchanged; this
additional regression is registered in `cmake/empty_receiver.cmake` and runs
serially under the same 180-second limit as the existing route tests.

The registered UCRT64 build (at most eight jobs) completed in
[`retry-2/build.log`](../../build/empty-receiver-fall-001/retry-2/build.log).
Its existing signedness and frozen-loader initializer warnings are retained.
[`retry-2/focused.log`](../../build/empty-receiver-fall-001/retry-2/focused.log)
passes **8/8 in 475.06 seconds**: the three formerly failed cases, both
source-identity checks, both receiver demonstrations and the additional frozen
simulation replay. No full verifier profile was run in this retry.

| Matched route | Authenticated entry simulation | Current simulation |
| --- | --- | --- |
| Blocker winning placements | WIN 23/30 | WIN 23/30 |
| Blocker losing placements | LOSS 0/30 | LOSS 0/30 |
| Earned MM_W_012 gadget placements | LOSS 2/20; actors 0/1 exit | LOSS 0/20; actors 0/1 splat |

The public action logs and every birth offset match the authenticated replay
on all three routes. Both corrected startup variants still compare post-birth
gameplay state and final outcomes through the registered paired assertions.
For the gadget route, the dry losses occur at age frames **113 / 127** in
receiver **16**, height **0**, request **1**. The baseline's two actors finish
Exited at **(134,79)**; current actors finish Dead at **(222,91)** with
fallDistance **50**. The reviewed Results images show **You Saved 2** before
and **You Saved 0** after. This establishes the exact cause of the changed
two-save expectation, while the earlier recorded gadget win-to-loss evidence
above remains intact.

All three completed route sets are archived under `retry-2/hatch-{baseline,before,after}/`;
`hatch-comparison.json` condenses their outcomes, actions, releases and two actors.
The newly reviewed floor-splat frame's ordinary camera places the affected
actor on the far side; the earlier task's dedicated natural-route images show
the descent and floor contact from the matching visible side. No unrelated
camera or occlusion repair was made. Installed and normal saves remain unchanged
in every replay and receiver demonstration. `preservation-audit.json` confirms
the three production files and protected controls are unchanged in this retry;
`final-source-hashes.json` records its final task content. Original mean-surface
rounding and rider attachment remain provisional as described above.

Handoff: dry-receiver support, live-height riding and retained before/after evidence are ready for needs_review; rounding and original rider attachment remain provisional.
Next: supervisor runs full registered main/recovery verification, then independent review of U3 evidence and a human playtest of the lever-dependent route.
