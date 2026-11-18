# evil-rats-001

Status: **needs_review; mandatory smoke pending supervisor execution.** Playable v1,
asset-census-001 rank 2; difficulty difficult, recommendation Opus, assigned Codex
override. Before: the accepted [census](asset-census-001.md) traced 45 placed rats
to `entities.subclass_actors`, which had no production simulation consumer. The
playtest ledgers establish the broader missing-hazard problem; they do not record
a human rat encounter. Adding this missing lethal population is the authorized
next dependency, first on **node 28 / E11_KF_1 / Escape to victory**.

`loadRetailLevelReport -> read_compiled_level -> entities.subclass_actors ->
LevelSetup::rats -> World::tick/updateSegment -> stepRats -> shared actor steppers`
now loads each stored position, facing, gravity, climb/float capabilities, render
flag and initial flip. The two Escape rats start at **(155,75,.5)** and
**(173,75,.5)** in file coordinates, falling. All 45 installed rats are upright
fallers with Floater capability, no Climber capability, and no initial flip.
The stored base flag +38 is zero throughout and is retained, not interpreted as
an update-enable flag; neither traced rat/common update checks it.

Rats occupy a separate population. Birth/release, active rescue count, saved,
dead, completion and Nuke scan only rescue actors; rat death paths suppress rescue
loss increments. Exit's existing type-3 exclusion remains. Rats reuse walking,
falling/floating, wall turns, climbing/hoisting if capable, fluids, gravity pads,
speed pads and teleport movement. They also participate in the existing tagged
contacts, gate crossings/crushing and named/crab hazards. Named hazards use stable
negative rat identities, so a later hatch birth cannot change a retained victim.

`stepRats -> groundAt -> closed +/-1 X/Y contact -> pendingVictim -> punch ->
finishRatAttack` claims one non-rat rescue actor. The victim stops moving, cannot
take skills or trigger gadgets, and becomes invisible with provisional death
debris. Completion removes it and counts one loss, then restores the rat's walk
state. A second rat cannot claim the same victim. Surviving rats never prevent
level completion. A rat removed mid-punch resolves its retained victim on its next
update, preventing a permanently captive rescue actor.

Presentation is `injectedWalkersFromWorld -> TubeScene::drawInjectedWalkers ->
prepareInjectedWalker -> actorSpritePlacement/drawActorSprite`: rats append after
the live rescue prefix and exit tails, so they cannot become skill picks. Sheet
type **3** selects **SPRITES\\EVILRATHI.SPR**, with the same camera-dependent facing,
light tint, zoom scale, body/scene occlusion and actor depth as lemmings. Installed
walk, fall, climb, flip, drown, umbopen/float, cartwheel and punch0..3 clips are
selected as applicable. Punch completion uses the installed eight-entry/rate-1
strip duration and recovered **.06 s** controller step, rather than an arbitrary
cooldown. Sprite clocks use bounded cycle phase, not a growing per-render loop.
Victim `Die` and common rat loss sounds use `World::cueSample/cue -> SoundEvent ->
takeSoundEvents -> TubeScene::soundWorldPosition -> CuePlayback/Mixer`, retaining
the accepted positional-audio policy and its .75 radial event adapter.

Static authority: repository-local `Lemmings Revolution/Lemmings Revolution.exe`,
SHA-256 **32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f**;
Ghidra **12.1.4**, profile
`3c06738e45e144d5f073edbd3244c6bff46222f389525fe659bc92dca2ccdece`.
Used reverse-engineer-anything with the project static profile; closed analysis
without snapshot arguments. Complete returned Evidence and limitations are retained
under [static/](../../build/evil-rats-001/static/), alongside the accepted census's
[instruction cross-check](../../build/asset-census-001/static/crosscheck.md).

- **448550 -> 448630 / vtable 4B0F6C:** placed subclass construction;
  **4265E8 -> 42FB5F:** common field reads, fall-state construction, optional
  **40BAC0 -> 40BB26 -> 42F5E3** gravity flip. Retained capability evidence:
  [Climber](phase-d2-skill-assign-capture.md), [Floater](phase-d2h-skill-assign-floater.md).
- **426414**, instruction packet `attack-instructions.json`: **426449** tests
  support; **426477** requires no pending victim; **4264A0** excludes type 3;
  **4264E9/4264F2** retain the victim ID and set victim +5D; **42657A..4265B6**
  choose one of four `punch%d` clips and call **42FD25**. New composed Evidence
  `ev_df9d3d3bf0c1ac7b5cc85cc3c0983d0f896d90616046aa3a2e0ab7aa56b5678d`.
- **42FAD7**, `contact-instructions.json`: closed +/-1 X/Y comparisons;
  no facing, gravity or Z test. **42E2BF** consumes forced +5D through
  **416BF0**, plays the death handle and creates the **416C49** flying figure.
- **408ECC**, `completion-instructions.json`: **408F0A** tests controller
  completion, **408F39..408F67** removes the retained victim, **408F6F** clears
  +3A0, **408F7C -> 42F8B9 -> 41BD60** restores walking. The latter shares
  the existing **5 units/s** walker. Gate **423858** scans the actor list without
  a species filter; the accepted gate adapter now includes rats too.

These are static traces, not observed original execution. Provisional choices:
deterministic cycling through the four punches instead of original RNG; rescue
actors update before rats, with contact sampled before each rat's movement;
integer feet/support and existing hybrid movement/death clocks; stationary punches
spend elapsed time starting on the following update; horizontal seam contact
wraps for the cylinder; already-lost/claimed victims are excluded; removal of a
rat mid-punch finishes its victim on the next update. Rats currently draw confirmed
integer movement positions, without lemmings' delayed motion interpolation. Lane
motion uses the accepted provisional one-slab-unit/second adapter from stored .5.
Death figures reuse existing lemming pieces, not a reconstructed rat-specific
flying figure. Exact original floating-point anchoring, timing, overlapping gadget
priority, death presentation and human solvability are unproven. No shared clock,
skill/release tuning or historical fidelity gate was reopened or passed.

The installed placements, independently checked by the new short legacy invariant:

| Node | Installed ID | Title | Rats |
| --- | --- | --- | --- |
| 28 | E11_KF_1 | Escape to victory | 2 |
| 33 | M11_KF_1 | Designed with love | 2 |
| 44 | M17_MT_1 | Under and Up. | 1 |
| 48 | M18_MT_1 | Watch Out! Evil about! | 3 |
| 51 | M21_KF_1 | Walk the plank or join the crew? | 1 |
| 52 | M19_MT_1 | Bamboo Maze | 6 |
| 55 | M12_MT_1 | Booby trap. | 7 |
| 56 | H17_mg_1 | Hit 'n' Run | 1 |
| 58 | H05_KF_1 | Battle o' Lemmuckburn | 2 |
| 64 | M15_mg_1 | The Legend of Smelly Belly | 3 |
| 67 | H10_MT_1 | Scale that wall.... | 1 |
| 69 | H15_MT_1 | Rocket Science. | 5 |
| 72 | H18_mg_1 | Reduce and Simmer | 1 |
| 80 | H08_MT_1 | T is for Teamwork. | 2 |
| 81 | VH05_JC1 | Don`t Let them out! | 4 |
| 92 | VH15_KF_1 | No clues ...hee hee hee hee! | 2 |
| 93 | VH16_MT | Last one to the top..... | 2 |

Checks: registered MinGW-w64 GCC **16.1.0**, existing **-O2/assertions** build,
eight build jobs. [Final build](../../build/evil-rats-001/build-final.log) passed.
[Final related tests](../../build/evil-rats-001/final-related-tests.log) passed
**6/6 in 3.57 s**: actorcompose, world, crab, namedhazards, counter_gates and
evil_rats. The new **legacy** invariant takes **.96 s**, checking contact edges,
one-victim ownership, pause, completion, skill exclusion, positional cue, rescue
accounting, exits, gate/fall/drown losses, interrupted attacks, stable hazard IDs,
gate crossings and all installed counts/capabilities/positions. Synthetic checks
do not establish natural gameplay. An initial new-test assertion incorrectly
treated +38 as enable; its failed logs remain, and the field interpretation above
is the correction. Existing assertions/timeouts/recordings were not changed.

The production direct-entry, zero-tick [clone image](../../build/evil-rats-001/escape-facing.png)
visibly contains both rats on the upper walkway; [command log](../../build/evil-rats-001/escape-facing.log),
exit 0. This is initial rendering only, not an observed movement/contact or human
win, and preceded only removal of a Drown-specific attack exclusion. That exclusion
is absent from the final source, matching the traced lack of a victim-state filter.
No new scripted gameplay route, replay fixture, frozen baseline or original capture
was created. The final supervisor smoke remains **pending**.

[Promoted replay selection](../../build/evil-rats-001/replay-selection.json):
**19 promoted recordings checked, zero affected levels**. Every promoted human
win is outside these 17 levels; no replay run, divergence or retirement request
applies. Recordings and replay checks remain untouched.

Changed: `src/sim/World.h/.cpp`, `src/sim/RetailLevel.cpp`,
`src/app/TubeScene.h/.cpp`, `src/__test__/test_evil_rats.cpp`, its legacy CMake
registration and this report. Entry backups and task-only diff stay under ignored
`build/evil-rats-001/`. Accepted pre-existing counter-gate and other edits were
preserved. The existing supervisor lease belongs to the dispatching controller;
no protected controls/shared instructions were edited. CIM process-detail access
was denied; permitted Get-Process checks found no game/clone or build runner at
entry, and no clone/build/analysis process remained at the final check. Final checks
and the selected image command used private `build/evil-rats-001/private-appdata`;
the test/direct-entry paths used have no
save store. Installed and normal user saves were not imported, reset or written.
No original execution, copied-code oracle, hooks, debugger, capture, installation,
subagent, detached work, commit or push occurred.

Handoff: the separate rat population, lethal contact and EvilRatHi draw path are ready for independent review; status needs_review.
Next: supervisor smoke, independent call-path/diff review, then the user's playtest; no fidelity gate is closed.

**What to check:** **node 28 / E11_KF_1 / Escape to victory**: rotate to the upper
walkway and watch both rats walk and turn at obstacles. Let a lemming contact one:
the rat should punch, the victim should disappear and count dead once, and the rat
should resume walking. Rats must never increase OUT/SAVED/dead rescue totals or
enter an exit; killing a rat must leave those totals unchanged. Check the same
sprites while rotating and zooming for ordinary actor occlusion and scaling.
