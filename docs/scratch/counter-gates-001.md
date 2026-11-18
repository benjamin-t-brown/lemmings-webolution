# counter-gates-001

Status: **needs_review**. Playable v1: restore missing gate puzzle rules, first
encountered at **node 25 / MM_ALL_8_003 / The Iron Curtain**. Difficulty difficult;
recommendation Opus; assigned Codex override used. **Mandatory smoke verification
is pending supervisor execution.** This worker did not run the verifier profile.

Before: the accepted [asset census](asset-census-001.md) traced 231 entities to
`RetailLevel::linkLevelGates`, where they became `Other` with no update. The mesh
builder also received `{}` instead of their movable-vertex records, producing
missing/incorrect gate faces. Connecting state, collision, messages and that
existing mesh path is the next playable-v1 dependency. The cited playtest ledgers
establish the milestone; this particular omission is census/source evidence.

**Acceptance wording correction:** The Iron Curtain contains one **type-0,
switch-only** gate, id 4, rectangle X `[30,45)`, Y `[18,87)`, initial request 2.
It has **no authored counter**: quota is zero and its three inactive counter
fields are `0xCDCDCDCD`. The first quota gate is **node 43 / M16_MG_1 / Lock In**,
with quota 10. This implementation preserves that distinction instead of
inventing a counter on The Iron Curtain. Across the installed levels there are
**209 switch-only gates and 22 quota gates on 11 levels**, totaling **231 gates
on 52 levels**. The complete [52-level count list](../../build/counter-gates-001/gate-counts.csv)
and [231 stored gate records](../../build/counter-gates-001/gate-entities.csv)
retain every level/node and gate count, including levels without quota counters.

The production load path is `loadRetailLevelReport -> linkLevelGates ->
CounterGate::initialise -> Terrain::setGateCollision`; the existing base-id
registry preserves construction order and later-id replacement. Lever load-face
fan-out is unchanged. Lever, pressure-pad and tagged type-2 occupancy-sensor lists
reach `dispatchLeverMessages -> CounterGate::request`. Sensor fan-out was the
small missing prerequisite for the requested gate message path; sensor rendering
and additional registry target classes remain outside this task.

The runtime path is `RuntimeSimState::advanceElapsed -> World::updateSegment ->
actor movement -> stepGadgetContacts -> stepCounterGates -> CounterGate::animate /
paintCollision`. Fluid receivers keep their accepted path and segment dt. The
legacy `tick()` also advances the new gates. `runApplicationLeg::prepareScene ->
TubeScene::syncTerrain/syncCounterGates -> buildCellMeshes/rebuildCellMeshCells`
keeps the gate topology and moves its shared pool vertices between inner and
outer cylinder positions. `counterGateRevision` invalidates the tube cache during
animation. The secondary draw calls `renderCounterGates` with live count state.
Rumble travels through `World::takeSoundEvents -> CuePlayback::drain/playName -> Mixer`.

Static authority: repository-local `Lemmings Revolution/Lemmings Revolution.exe`,
SHA-256 **32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f**;
Ghidra **12.1.4**, profile
`3c06738e45e144d5f073edbd3244c6bff46222f389525fe659bc92dca2ccdece`.
Used the reverse-engineer-anything skill and closed both static sessions without
snapshot arguments. [Instruction cross-checks and exact packet index](../../build/counter-gates-001/static/crosscheck.md)
distinguish retained census pseudocode from newly checked instructions:

- Reader/init `424187/42421D/42727C`: decode the gate fields and bind existing
  `(x,y,index)` records through `43A1C6`, creating missing ones when necessary.
- Requests `423719/423731`: type 4 opens, type 5 closes; the type-0 handler is a
  no-op. Update/count `423749/423858`: open commits reset count; mode 1 counts
  vertical entries, mode 2 horizontal entries; direction controls increment or
  decrement. Quota schedules closure after a clearance delay.
- Animation `4270DC/426FEC`: progress moves at 3/sec; opening uses p-squared,
  closing uses `1-(1-p)^2`. Collision changes on strict endpoint overshoot:
  `427554 OR 403`, `4275D4 AND FFFFFBFC`. Supported unrelated flag planes survive;
  material landability uses the clone's residual authored-word projection.
- Counter `423E69`: installed `^LIMIT_PANEL` and `_LIMIT0..9`, with traced sizes
  and positions. Rumble `423823` plays on a changed request commit whose prior
  state is nonzero. Closing-contact `4270DC -> actor+5D -> 42E340 -> 416BF0/416C49`
  removes the actor on its next update.
- Sensors `43725A/4339FC/4372AF`: preserve closed-AABB contact, cooldown and list
  order, including empty then occupied sends in one zero-cooldown update.

**Provisional choices/limits:** the original dt entry strips are mapped to swept
integer clone positions, once per entry; stationary actors and teleport jumps do
not consume quota. Vertical clearance speed comes from actual clone movement;
horizontal clearance retains the traced 5/sec divisor. Cross-class update priority
uses the order above. Counter display clamps to 0..99; decals use a slight outward
slab offset and the clone's ordinary depth/blend path. Closing death reuses clone
particles rather than the original flying figure/death audio. Residual material
projection is not a new raw-mask fidelity claim. Native timing/depth, exact pixels
and natural gameplay remain unverified; parked timing/clipping work is unchanged.

Registered MinGW-w64 GCC built the clone and the new **legacy** unit. The final
[targeted build](../../build/counter-gates-001/final-relevant-build-2.log) and
[checks](../../build/counter-gates-001/final-relevant-tests-2.log) passed **10/10**
in 3.71 seconds: the new gate unit plus sharedtiming, levergates, leverinitface,
actorcompose, backdropscene, terrain, world, retaillevel and clicktoassign. The
new unit takes 1.04 seconds and checks crossings, seam/direction/vertical cases,
collision/other flags, delay, Rumble, reset, sensor/pad dispatch, crush removal,
installed counts, movable mesh indices/cache and changing counter pixels. The
first render-target build exposed a missing SceneRaster link dependency; adding
it to actorcompose and clicktoassign fixed the build. No existing test assertion,
timeout, recording or registration was weakened or removed.

The final [102-level sweep](../../build/counter-gates-001/sweep-final/matrix.csv)
completed **102/102**, with no crash, hang, load failure or blank view; [run log](../../build/counter-gates-001/sweep-final-report.log).
The comparator uses the existing matrix copied at worker entry under
`build/counter-gates-001/entry-backup/`, not a freshly run pre-task baseline.
An earlier development sweep also completed, but preceded the vertex-binding
correction and is retained separately under `sweep/`; it is not final evidence.
The final idle results changed on these eight levels (saved totals unchanged):

| Node / installed ID | Changed result |
| --- | --- |
| 43 M16_MG_1 | dead 0 -> 1; active 14 -> 13 |
| 44 M17_MT_1 | dead 0 -> 2; active 60 -> 58 |
| 65 H12_MG_1 | dead 0 -> 1; active 10 -> 9 |
| 69 H15_MT_1 | dead 46 -> 0; active 4 -> 50 |
| 80 H08_MT_1 | walkers 2 -> faller 1 + walker 1 |
| 87 VH11_MT | dead 0 -> 43; active 53 -> 10 |
| 88 VH12_KF1 | walkers 15 -> fallers 2 + walkers 13 |
| 98 VH22_KF1 | dead 44 -> 40; active 6 -> 10 |

[Exact state differences](../../build/counter-gates-001/sweep-changes.json) retain
teleporting/splattering states too. These are no-input clone observations, not
wins or evidence that a person can solve the levels. Visual inspection of the
ordinary initial clone draw confirms the striped barrier on
[The Iron Curtain](../../build/counter-gates-001/iron-curtain-initial.png) and the
visible red **10** on [Lock In](../../build/counter-gates-001/lock-in-initial.png).
These two direct-entry `--sim --shot` outputs do not establish a played switch
interaction or original pixel fidelity.

[Promoted recording selection](../../build/counter-gates-001/affected-replays.json):
**none affected**, since every promoted human win precedes node 25. Therefore no
replay run or divergence/retirement is reported. Recordings were left untouched.
All clone commands used private `build/counter-gates-001/private-appdata`; direct
shots and the sweep have no save store. Installed and normal user saves were not
read/imported/reset/written. No original execution, debugger, hooks, capture,
oracle, installation, detached work, subagent, commit or push occurred.

Changed: `World.h/.cpp`, `Terrain.h/.cpp`, `RetailLevel.cpp`, `TubeScene.h/.cpp`,
`main.cpp`, the new `test_counter_gates.cpp`, CMake dependencies/legacy registration,
and this report. Entry backups/evidence remain under ignored
`build/counter-gates-001/`. Existing queue/status/hook-log edits were preserved;
no protected controls or shared instructions were edited.

Handoff: gate implementation and final 102-level sweep are ready for independent review; status needs_review.
Next: supervisor smoke, review the type-0 acceptance correction, then the user's playtest; no fidelity gate is closed.

**What to check:** **node 25 / The Iron Curtain**: the striped barrier starts
closed; let a lemming operate its switch and confirm that it retracts and permits
passage. There is no counter on this level. **Node 43 / Lock In**: watch the
counter start at 10 and decrease as lemmings enter the quota gate; at zero the
barrier closes after clearance, with matching collision. A linked open request
resets the quota for another opening.
