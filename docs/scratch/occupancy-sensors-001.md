# occupancy-sensors-001

Status: **needs_review**. Playable v1; difficulty difficult, recommendation Opus,
assigned Codex override. **Mandatory smoke verification is pending supervisor
execution.** No full verifier profile was run by this worker.

Before evidence: [asset-census-001](asset-census-001.md), rank 4, identified seven
unconsumed tagged type-2 sensors on **node 55 / M12_MT_1 / Booby trap.** and
**node 80 / H08_MT_1 / T is for Teamwork.** The playtest ledgers establish the
milestone, but neither level has a retained human sensor failure/win. This is a
source/data omission, not a newly observed playtest failure.

**Scope correction:** accepted [counter-gates-001](counter-gates-001.md) already
loaded the volumes, cooldowns and lists and sent their messages on the ordinary
elapsed-time path. Its prerequisite work is preserved. Remaining source-traced
gaps were `World::tick` omitting sensors, sensors remaining `Other` in the id
registry, and skipping the native contact cache while cooling down. This task
connects that entry point, retains the stored enable flag, implements sensor
enable/disable messages and follows the first-contact cache semantics. There is
no claim that the ordinary seven links were wholly inactive at worker entry.

## Production path and stored links

`loadRetailLevelReport -> linkLevelGates` decodes tagged type 2 into
`LevelSetup::occupancySensors` and `gadgetContactOrder`. The registry now assigns
`ObjectClass::OccupancySensor` in construction order; later named objects still
replace duplicate ids. Stored state, closed AABB, cooldown and ordered list pairs
are preserved. The normal runtime path is `RuntimeSimState::advanceElapsed ->
World::updateSegment -> actor movement -> stepGadgetContacts ->
stepOccupancySensor -> dispatchLeverMessages -> CounterGate::request ->
stepCounterGates -> animate/paintCollision`. `World::tick`, used by diagnostic
startup/compatibility callers, now also invokes the sensor helper before gates.
The ordinary renderer uses the accepted gate mesh/collision integration.

[Installed data](../../build/occupancy-sensors-001/installed.txt) retains all
tagged records and gate bounds. Every sensor starts enabled, state **0**, with a
**5-second** cooldown and an **empty second list**. Bounds below are original
Y-up cylinder coordinates; all span radial Z `[0,1]`.

| Node | Sensor id | Closed X / Y bounds | Occupied messages |
| --- | --- | --- | --- |
| 55 | 4 | `[170,175] / [12,17]` | gate 8, type 4: open |
| 55 | 14 | `[11,19] / [1,9]` | gate 16, type 4: open |
| 55 | 19 | `[2,10] / [45,53]` | gate 15, type 4: open |
| 55 | 20 | `[246,254] / [45,53]` | gate 17, type 4: open |
| 55 | 21 | `[236,244] / [86,94]` | gate 18, type 4: open |
| 80 | 33 | `[93,97] / [83,87]` | gate 35 then gate 17, type 5: close |
| 80 | 34 | `[141,145] / [83,87]` | gate 35 then gate 17, type 5: close |

End-to-end examples: node 55 sensor **4** selects its occupied list, resolves id
**8** to a terrain gate, and type **4** sets request **1**. The gate commits target
**0**, retracts and clears collision on its existing animation endpoint. Its
rectangle is X `[13,47)`, Y `[48,50)`. Node 80 sensor **33** sends type **5** to
**35**, then **17**, setting both requests to **2** and targets to **1**; the
initially open barriers close and stamp collision. Their rectangles are
`[123,126) x [83,95)` and `[112,115) x [83,95)` respectively. These are installed
data/source chains, checked by isolated contact invariants, not played wins.

Leaving does **not** invert these actions. The empty arm changes state to **2**
after the authored cooldown expires; its empty list sends nothing, keeping gates
latched. Continuous contact can refresh the cooldown without resending an
unchanged state. At zero cooldown, empty precedes occupied in the same update.
No extra clock or timing approximation was introduced.

Only repository-local `Lemmings Revolution/Lemmings Revolution.exe` was opened,
SHA-256 **32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f**;
REA Ghidra **12.1.4**, profile
`3c06738e45e144d5f073edbd3244c6bff46222f389525fe659bc92dca2ccdece`.
Used reverse-engineer-anything; closed analysis without snapshot arguments.
[Static cross-check](../../build/occupancy-sensors-001/static/crosscheck.md)
indexes new packets and retained instructions: **43732D/433CEE** read the
fields; **43725A -> 430C33 -> 430BA1** caches the **first overlapping actor
pointer**, correcting the census/task's occupancy-count wording. **4339FC**
orders cooldown/detection; **437285/43729A -> 4372AF -> 433AC0/433B2E** change
state before list fan-out. **423719/423731** open/close terrain gates. Sensor
vtable **4B12B4** instead maps type 4/5 to **41DC00/41DC20**, setting enable
byte `+38`; **4423D2 -> 41CBA0** gates updates on it. Disabling freezes cooldown.

Provisional adapters: integer clone feet, radial lane 0, wrapped X, removed-actor
filtering, lemmings before rats, and stable cache indices. Legacy tick retains
its dt and lever/pad behavior; only the elapsed path preserves tagged-stream
order. No art added: `PlaceHolder.a3d` has a generic draw wrapper, but a visible
player landmark is unestablished. Natural interaction/solvability and native
timing/pixels remain unverified; parks and fidelity gates are unchanged.

Registered MinGW-w64 GCC built the clone and relevant tests:
[build](../../build/occupancy-sensors-001/build.log),
[related build](../../build/occupancy-sensors-001/related-build.log).
[Focused checks](../../build/occupancy-sensors-001/focused.log) passed **2/2**
(counter_gates and the new **legacy** occupancy_sensors test, 1.15 seconds).
[Related checks](../../build/occupancy-sensors-001/related.log) passed **5/5**
(sharedtiming, levergates, leverinitface, world, retaillevel, 1.70 seconds).
The new legacy test takes 0.06 seconds: cooldown/list/cache/bounds invariants,
enable/disable, tick and all seven links/nine sends. Its synthetic fluid target
is not an installed link. Fixtures do not establish natural gameplay.

[Promoted replay inventory](../../build/occupancy-sensors-001/replays.json):
**19 recordings, zero for either affected level**. No affected replay was
available to run; no divergence or retirement is reported. Recordings, replay
checks, assertions and timeouts were left untouched.

Changed: `src/sim/World.h/.cpp`, `src/sim/RetailLevel.cpp`,
`src/__test__/test_occupancy_sensors.cpp`, its legacy CMake registration and this
report. [Detailed evidence](../../build/occupancy-sensors-001/detailed-report.md),
entry backups and task-only diffs are under ignored `build/occupancy-sensors-001/`.
Pre-existing edits were preserved. Checks open no save; test processes used
private `build/occupancy-sensors-001/private-appdata`. No original execution,
background work, subagents, protected-control edits, commits or pushes occurred.

Handoff: seven authored sensor-to-gate links and the remaining sensor integration are ready for independent review; status needs_review.
Next: supervisor smoke, independent review, then the user's playtest; no fidelity gate is closed.

**What to check:** **node 55 / Booby trap.**: send lemmings through the five
volumes above; the linked barriers open, and walking away leaves them open.
**Node 80 / T is for Teamwork.**: enter either upper sensor volume (ids 33/34);
both barriers at X 112..115 and 123..126 close and block passage. Walking away
does not reopen them. Retry resets sensors and gate state. Pause freezes the
cooldown; fast-forward advances it with the existing simulation updates.
