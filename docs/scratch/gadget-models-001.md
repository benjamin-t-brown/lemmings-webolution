# gadget-models-001

Status: **needs_review; smoke pending supervisor execution.** Playable v1,
asset-census-001 rank 3; difficult / recommended Opus / assigned Codex override.
Before: the accepted [census](asset-census-001.md) identified missing render owners
for working teleports and speed pads. The [playtest-8 ledger](../playtest/2026-11-16-playtest-8.md)
establishes the broader omitted-gadget problem, not a human teleport encounter.
Admitting these landmarks and contact feedback is the next authorized dependency.

Production path: `runApplicationLeg -> BackdropScene::build` now admits the
`entities_4347ec` base and both FX owners, and tagged type-4 speed-pad roots.
`gadgetPresentation` binds **54 base + 108 FX + 16 speed models = 178 clips**.
`prepareScene -> animateGadgets -> renderStage(LegacyUnattributed)` applies their
authored rotation/UV tracks through the existing textured mesh/depth path. Base
rings loop; FX remain hidden until use; the arrow panel is visible before contact.
The stored software projection bias is retained for the newly admitted models.

`World::step -> teleportAt / speedPadAt` and `stepTeleporting` notify a read-only
presentation observer at existing departure, speed activation and destination
registration points. `triggerGadget` selects the owner/FX side/sample; clocks sample
the existing `levelClockMs`, freezing on pause and following fast-forward. Retry
constructs fresh presentation. Movement, AABBs, pairing, latches, speed effect,
teleport countdown and rescue accounting are unchanged. Rats use the same contacts.
Named samples reach `CuePlayback::playGadgetSample -> playName -> Mixer`, with the
existing camera/positional-gain adapter and SkipPlaying overlap policy.

Two corrections to the assigned scope, established from installed data:

- **The Lone Ranger has four teleporters**, not two, at stored logical positions
  `(219,30,.5)`, `(241,64,.5)`, `(102,38,.5)`, `(79,61,.5)`. All four are admitted.
- **Only one installed level has nonempty teleport sample names:** node 46,
  `show_kf3`, Water way to go: `teleport` on departure, `teleport2` on arrival.
  These are two samples, not two levels. The remaining 17 teleport levels retain
  silence for empty names. No universal teleport sound was invented.

All **22 affected installed levels** (T = teleport bases, S = speed pads):

| Node | Installed ID | T | S |
| --- | --- | --- | --- |
| 38 | MM_TELE_003 | 4 | 0 |
| 43 | M16_mg_1 | 2 | 0 |
| 44 | M17_MT_1 | 4 | 0 |
| 46 | show_kf3 | 2 | 0 |
| 48 | M18_MT_1 | 4 | 0 |
| 49 | Show_MT3 | 2 | 1 |
| 51 | M21_KF_1 | 2 | 1 |
| 52 | M19_MT_1 | 4 | 0 |
| 55 | M12_MT_1 | 2 | 0 |
| 56 | H17_mg_1 | 0 | 3 |
| 59 | H06_MT_1 | 4 | 0 |
| 67 | H10_MT_1 | 2 | 1 |
| 69 | H15_MT_1 | 0 | 3 |
| 80 | H08_MT_1 | 4 | 1 |
| 82 | VH06_MT | 0 | 3 |
| 84 | VH08_MT | 2 | 0 |
| 86 | VH10_MT | 2 | 0 |
| 88 | VH12_KF1 | 4 | 0 |
| 90 | VH13_MT | 2 | 0 |
| 93 | VH16_MT | 0 | 1 |
| 95 | VH18_JC | 4 | 0 |
| 97 | VH21_JC | 4 | 2 |

The [level index](../../build/gadget-models-001/levels.json) also retains titles.
There are **18 teleport levels and 9 speed-pad levels**, with five overlapping.

Static authority: repository-local `Lemmings Revolution/Lemmings Revolution.exe`,
SHA-256 **32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f**;
Ghidra **12.1.4**, profile
`3c06738e45e144d5f073edbd3244c6bff46222f389525fe659bc92dca2ccdece`.
Used reverse-engineer-anything with the project static profile; closed both
sessions without snapshot arguments. Complete returned Evidence is under
[static/](../../build/gadget-models-001/static/); the
[instruction cross-check](../../build/gadget-models-001/static/crosscheck.md)
retains decisive operands, rather than treating pseudocode as original source.

- `4347EC` loads both FX models, selects clip 0 and looping, and does the same
  for a base with animation. `434AAF` draws the base then enabled FX owners.
- `43540C/435467` are exclusion-list/contact sites **inside `434B0F`**, not
  independent function entries. Its departure/arrival branches select side FX,
  call model slots `+38/+0C/+08` to start/place/orient them, and play serialized
  handles at `+1A8/+1AC`. Native FX enablement also uses per-side actor counts.
- `432742` retains the existing speed-pad AABB, facing and vertical filters;
  `432632` performs the rising-edge FASTER message and SpeedPad cue. The new
  notification is at the clone's existing corresponding latch, not a second test.
- The census's third-array label "events" is structurally misleading here:
  `47CDE7` reads **node, axis selector, turns/sec**. `4764C4 -> 476EE0` samples
  absolute clip time. Selectors **0/1/2 mean Y/X/Z**, through
  `481D10/481E26/481F3C`; matrices preserve translation. `4821B0` confirms row-major
  element addressing. The new gadget sampler uses this mapping.
- `476FB0/476A80` select the UV interval and add its offset plus fraction*delta
  to the stored corner UV. Teleport/FX clips contain 4/8 spin records with
  durations **.5/5 seconds**; speed clips contain two UV tracks, **2/3 second**.

**Provisional limits:** preorder node binding follows the existing app animation
adapter. FX reuse the owning base's placed matrix/scale/tilt, with the traced
half-turn side difference. They loop while enabled and remain enabled for one
authored five-second cycle after the last use; native per-side actor counters
are not reconstructed. Speed pads hold the first UV sample while idle and play
one cycle on each existing speed activation. These finite feedback policies aim
at readable clone behavior, not original playback phase/lifetime fidelity. Base
rings use a shared zero phase. Audio uses existing provisional spatial gains,
radial .75 and per-sample overlap suppression; physical audio was not observed.
The observer is attached on the ordinary window-play path; diagnostic pre-seeded
`--sim-ticks` runs do not retrospectively create contact effects. Native depth,
exact pixels, hybrid timing and human solvability remain unverified. No park or
historical fidelity gate is reopened or passed.

Checks: registered UCRT64 GCC **16.1.0**, existing **-O2/assertions**, eight jobs.
Production built in [build-5.log](../../build/gadget-models-001/build-5.log); final
legacy-unit build is [build-final.log](../../build/gadget-models-001/build-final.log).
[Final related checks](../../build/gadget-models-001/related-tests-final.log):
**5/5 passed in 1.44 s**: world, backdropscene, namedhazards, positional_sound,
gadget_models. The new **legacy** invariant takes **.52 s** and checks notification
edges/destination, every installed model's association with its simulation pad,
all 178 clips, rotation axes, absolute resampling, sample selection, empty-name
silence and no-audio suppression. It adds no gameplay route or replay fixture.
Initial new-test failures (Earth enum typo, an extra post-arrival movement tick,
and the scope's erroneous two-sampled-level count) remain in the development logs;
existing tests/assertions/timeouts were not changed.

Inspected zero-tick production clone images: [The Lone Ranger](../../build/gadget-models-001/lone-219-initial.png)
and [Take Her to Warp Speed, Captain](../../build/gadget-models-001/speed-210-initial.png).
They show the rings and speed arrow panel at stored places. Their adjacent `.log`
files retain commands' results; both exited 0. The earlier `lone-initial.png` used
default camera values because its task-local camera file lacked JSON keys; the
two linked images use corrected explicit JSON. These are initial views, not an
observed ordinary contact/arrival, human win or original comparison.

[Replay selection](../../build/gadget-models-001/replay-selection.json) found
**zero affected promoted levels**. For regression confirmation all **19 promoted
recordings** were nevertheless run individually with `webolution --game-dir
"Lemmings Revolution" --replay <file>`; **19/19 PASS, won=1, completed=1**.
[Exact files, step numbers, results and per-file logs](../../build/gadget-models-001/replays/results.json):

| Recorded level | Final step | Saved/needed | Result |
| --- | --- | --- | --- |
| MM_BASH_002 | 1844 | 10/10 | PASS |
| MM_BASH_MINE_WLR1 | 2272 | 20/10 | PASS |
| MM_BLOCK_001_A | 1619 | 24/15 | PASS |
| MM_BLOCK_BOMB_A | 1546 | 12/10 | PASS |
| MM_BOMB | 954 | 17/10 | PASS |
| MM_BUILD | 1685 | 20/10 | PASS |
| MM_CLIMB_002 | 1252 | 10/1 | PASS |
| MM_DIG_000 | 1014 | 19/10 | PASS |
| MM_FLOAT | 978 | 10/1 | PASS |
| MM_MINE | 1137 | 20/10 | PASS |
| MM_W_012 | 1742 | 16/10 | PASS |
| MM_E1_3_SKILLS | 2095 | 10/5 | PASS |
| M06_KF_1 | 1544 | 91/50 | PASS |
| MM_E1_4SKILLS | 2506 | 20/10 | PASS |
| MM_E2_4SKILLS | 2142 | 14/10 | PASS |
| MM_BASH_DIG_001_A | 3241 | 20/10 | PASS |
| MM_BUILD_BLOCK_A | 2482 | 15/10 | PASS |
| E19_KF_2 | 6034 | 24/20 | PASS |
| E20_MG_1 | 12810 | 7/5 | PASS |

No divergence or retirement request. Recordings/replay checks are untouched.
Smoke remains **pending supervisor execution**, not claimed passed. No mandatory
suite was duplicated. Commands used private `build/gadget-models-001/private-appdata`;
the direct shots, units and replay routes used have no save store. Installed and
normal user saves were not read/imported/reset/written. Initial CIM process-detail
access was denied; permitted Get-Process checks found no game/clone/build runner.
Static Ghidra analysis executed no target code and was closed. No original game,
copied-code oracle, debugger, hooks, capture, installation, detached work, subagent,
commit, push or protected-control/shared-instruction edit occurred.

Changed: `src/app/BackdropScene.h/.cpp`, `src/app/main.cpp`, `src/sim/World.h/.cpp`
(observer only), `src/audio/CuePlayback.h/.cpp`, `src/__test__/test_gadget_models.cpp`,
its CMake legacy registration and this report. Entry backups, static evidence,
checks and task-only diffs remain under ignored `build/gadget-models-001/`.
Pre-existing accepted-task, queue/status and hook-log changes were preserved.

Handoff: gadget landmarks, authored clips and serialized sample routing are ready for independent review; status needs_review.
Next: supervisor smoke, independent diff/call-path review, then the user's playtest; no fidelity gate is closed.

**What to check:** **node 38 / MM_TELE_003 / The Lone Ranger**: rotate around the
cylinder to see all four stored teleporters; let a lemming enter one and arrive at
its linked partner. Rings should spin and the respective side FX should appear at
departure and arrival, then settle after use. **Node 49 / Show_MT3 / Take Her to
Warp Speed, Captain**: the arrow panel must be visible before contact; a crossing
should animate its arrows with the existing speed effect. Pause must freeze both
animations, fast-forward must advance them, and Retry must reset them. **Node 46 /
show_kf3 / Water way to go**: hear the serialized departure/arrival samples; node 38
must retain its empty-name silence.
