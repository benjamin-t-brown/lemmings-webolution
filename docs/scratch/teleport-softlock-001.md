# teleport-softlock-001: destination pad re-entry

Status: **needs_review**. Playable v1, sweep S1, nodes 38/43/44. Implemented and
targeted checks passed; mandatory full main/recovery verification and independent
acceptance remain for the supervisor. No original execution, debugger, hooks,
capture, oracle, subagents or background helpers were used.

## Observed cause and production path

The unchanged clone confirms the sweep's inference. Installed `MM_TELE_003`,
natural first lemming, no skills or injected actors:

| Tick / level time | Position | State / frame |
| --- | --- | --- |
| 72 / 14.4 s | 85,34 | Walker / 27 |
| 73 / 14.6 s | 85,34 | Teleporting / 0 |
| 74 / 14.8 s | 85,34 | Teleporting / 2 |
| 75 / 15.0 s | 85,34 | Teleporting / 4 |
| 76 / 15.2 s | 241,31 | Walker / 1 |
| 77 / 15.4 s | 241,31 | Teleporting / 0 |
| 80 / 16.0 s | 79,34 | Walker / 1 |
| 81 / 16.2 s | 79,34 | Teleporting / 0 |

It makes 15 arrivals in the next 12 seconds without walking out. Facing stays
left. This is observed clone behavior, not merely a source inference.
[Before trace](../../build/teleport-softlock-001/before/trace/trace.csv),
[corrected trace result](../../build/teleport-softlock-001/before/trace/trace-result.txt).

Production call path: `runApplication --level --sim` -> `loadRetailLevel` ->
`findLevelObjects` serialized id pairing -> `LevelSetup::teleports` ->
`RuntimeSimState::advanceElapsed` -> `World::advanceElapsedMs` ->
`updateSegment` -> `stepElapsed` -> `step` -> `teleportAt` /
`stepTeleporting`. The arrival snapped to the partner cell as a Walker;
the next pad check happened before walking and attached Teleporting again.
The per-tick driver uses the same loader and elapsed-time World entry; the three
application runs also exercise the renderer and application clock adapter.

Afterward tick 76 still arrives at `(241,31)`; ticks 77..83 walk through
`(240,31)` to `(234,31)`, outside the closed destination box. Only one arrival
occurs in this trace window, with no immediate re-entry.
[After trace](../../build/teleport-softlock-001/after/trace/trace.csv).

## Static arrival contract and implementation

Retained starting points:
[native-023](native-023-teleport-links.md),
[lessons](../lessons/data-and-simulation.md),
[round 16](../../old/docs/scratch/phase3-round16-teleport.md).
New targeted static REA/Ghidra 12.1.4 reads used the repository reference only:
`Lemmings Revolution/Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Analysis was closed without a snapshot. Full tool Evidence and limitations are in
[static/](../../build/teleport-softlock-001/static/).

| Shipped instructions / decompilation | Finding |
| --- | --- |
| `0x434B0F`, `0x4354DD..0x43550D` | Pad obtains the actor id, queries its `+0x248` list via `0x435CE0`, and skips an already listed actor. |
| `0x4350E1..0x4350F9` | Source obtains the actor id and calls `0x434A90` on its `+0x260` partner before copying the destination pose. |
| `0x434A97..0x434AA4` | `0x434A90` passes the id to the pad's `+0x248` list insertion. `0x435D00` -> `0x435DF0` allocates/links a list node. |
| `0x435F90` | Membership is an equality search over the stored ids (decompilation). |
| `0x435454..0x435478` | Exclusion entry is erased if actor lookup fails or its point is outside the pad AABB; it is retained while inside. |
| `0x43503C..0x43508D` | Separate facing-dependent exit motion, Y+2 and depth offset exist; these were not needed to establish the exclusion rule. |

Pseudocode was checked against the saved pad routine, registration and insertion
instructions, particularly the membership branch, destination registration and
outside-only erase. Evidence: `static/decompile.json`, `instructions.json`,
`suppression.json`, `register-instructions.json`, `list.json`,
`insert-instructions.json`. Interior selectors `0x43503C` and `0x435400` resolve
to canonical function `0x434B0F`, not separate procedures.

`World.h` now stores each actor's excluded pad indices, transposing the original
per-pad actor-id lists. `World.cpp` registers source and arrival contacts, skips
excluded pads, and removes contacts only outside their closed wrapped AABBs.
Cleanup also runs during non-Walker and sub-tick updates. Thus departure rearms
the pad, and different lemmings remain independent. Pairing, snap coordinates,
facing, release rates, skills and timing are unchanged.

One explicitly provisional policy remains: custom overlapping pads all become
excluded when they contain the arrival point. Every one of the 54 installed
links has exactly one containing destination, measured by the regression, so
this policy is equivalent there. Exact overlapping-pad behavior would require
a suitable original observation or carrying explicit partner identity through
the clone's projected data. No such original observation ran.

The existing common frame increment plus `stepTeleporting`'s increment produces
600 ms transits at this clock, despite the nominal four-count rule. That behavior
is preserved; this task does not establish original timing or reconstruct the
original's animated exit trajectory/Y+2 pose.

## Same idle measurement before and after

All 18 installed teleport levels were loaded, advanced with 1-second elapsed
passes, and stopped five seconds after the last release (or level end/time cap),
matching `level-sweep-001`. These three natural routes encounter pads; the other
15 do not encounter one during their idle windows and have **identical complete
idle census rows** before and after.

| Level / node | Horizon | Teleporting before -> after | Completed arrivals / departures after | Maximum transit / immediate re-entries |
| --- | --- | --- | --- | --- |
| MM_TELE_003 / 38 | 167 s | 94 -> 0 | 179 / 177 | 600 ms / 0 |
| M16_MG_1 / 43 | 28 s | 6 -> 1 | 21 / 18 | 600 ms / 0 |
| M17_MT_1 / 44 | 124 s | 44 -> 5 | 420 / 411 | 600 ms / 0 |

The remaining Teleporting actors are bounded transits. Leavers can walk around
and deliberately encounter a pad again: later visits are allowed. Departure
totals exclude arrivals still within a pad at the sampling horizon.
The no-input MM_TELE_003 run proceeds into hazards: afterward 78 dead, 1 Faller,
21 Walkers, zero saved, versus 100 alive/zero saved before. The other two still
have no saved or dead. This fix removes forced bouncing; it is not a win or a
claim of player-confirmed playability.

Measurements: [before census](../../build/teleport-softlock-001/before/sweep/idle.csv),
[after census](../../build/teleport-softlock-001/after/sweep/idle.csv),
[per-visit audit](../../build/teleport-softlock-001/after/sweep/visits.csv),
[natural transition events](../../build/teleport-softlock-001/after/sweep/events.csv).
The three `runApplication` idle runs reproduce these census rows exactly.

Retained actual clone frames, visually inspected in this worker:

| Level | Before final | After final | After departure |
| --- | --- | --- | --- |
| MM_TELE_003 | [frame](../../build/teleport-softlock-001/before/MM_TELE_003/MM_TELE_003-final.png) | [frame](../../build/teleport-softlock-001/after/MM_TELE_003/MM_TELE_003-final.png) | [frame](../../build/teleport-softlock-001/after/MM_TELE_003/MM_TELE_003-departure.png) |
| M16_MG_1 | [frame](../../build/teleport-softlock-001/before/M16_MG_1/M16_MG_1-final.png) | [frame](../../build/teleport-softlock-001/after/M16_MG_1/M16_MG_1-final.png) | [frame](../../build/teleport-softlock-001/after/M16_MG_1/M16_MG_1-departure.png) |
| M17_MT_1 | [frame](../../build/teleport-softlock-001/before/M17_MT_1/M17_MT_1-final.png) | [frame](../../build/teleport-softlock-001/after/M17_MT_1/M17_MT_1-final.png) | [frame](../../build/teleport-softlock-001/after/M17_MT_1/M17_MT_1-departure.png) |

Images show lemmings distributed along the walkways after the fix. They are
scripted source-built clone observations, not original captures or proof of
full-frame fidelity. The event audit establishes individual departures more
precisely than these one-second sampled images.

## Checks, provenance and handoff

Changed: `src/sim/World.cpp`, `src/sim/World.h`, new
`src/__test__/test_teleportsoftlock.cpp`, an additive `CMakeLists.txt` block, and
this report. Pre-existing animation, terrain, queue and installation-log edits
were preserved. No protected controls were edited by this worker.

Five new CTests register the natural trace, 18-level census plus 108 directed
real-pad visits (54 pads, both facings), and three production application runs.
The directed visits assert serialized destination/facing, departure without
re-entry, one containing destination, rearming after departure and independent
actors. These seeded visits supplement the natural routes, not replace them.
`--observe` is only for recording the failing baseline; registered tests never
use it. Final sweep: 577 checks; trace: 3; each app: 8, all passed.

Targeted final run: [targeted-final.log](../../build/teleport-softlock-001/targeted-final.log),
8/8 passed (five new tests, `world`, `teleportlinks`, `teleportdemo_after`).
Builds used registered UCRT64 GCC/CMake/Ninja and existing `-O2 -g` assertions.
Full registered main/recovery verification is **pending supervisor execution**;
no worker full-suite pass is claimed.

Each final test compares existence and complete bytes of the installed and
normal clone saves before/after; both remain unchanged (1504 bytes each).
`after/*/save-proof.csv` retains the results. App tests inject a private store
under their own output directories; no default store or save import is used.

Baseline `World.cpp`, `World.h` and `CMakeLists.txt` copies match the supervisor
task-entry hashes exactly. The pre-change source-built executable and driver
are retained under `build/teleport-softlock-001/baseline/`; provenance and final
tested source hashes are in [source-provenance.json](../../build/teleport-softlock-001/source-provenance.json).
The queue's hash differs because the supervisor writes the dispatch's
`ready` -> `in_progress` status after taking its entry manifest; the worker
did not write it. Other recorded protected-file hashes match entry.

Reporting corrections: an early trace helper incorrectly called a bounce back
to the source a departure. Its condition was corrected and the unchanged
baseline rerun before freezing the driver; the final baseline says
`arrivals=15 left=0 reentered=1`. Before-image files named `*-departure.png`
likewise depict a bounce return and are **not** departure evidence. Only the
before final frames and after departure frames are cited above. The first MM
application baseline preceded this trace-only helper correction; its application
routine and production sources were unchanged.

Handoff: teleport re-entry is fixed; five registered regressions and retained clone evidence are ready for review.
Next: supervisor runs full main/recovery verification, then independent acceptance; human playtests and historical fidelity gates remain open.
