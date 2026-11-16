# moving-hazards-001 — named hazards

Status: **needs_review; supervisor smoke pending**. Playable v1, following
[playtest 8's lead data finding](../playtest/2026-11-16-playtest-8.md), not a
human encounter with these four kinds. Before: parsed models had no render or
simulation consumer on 20 installed levels. This implements that missing
dependency. Difficulty: difficult; recommended Opus, assigned Codex override.

## Evidence and behavior

Static only: repository-local `Lemmings Revolution/Lemmings Revolution.exe`,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`,
Ghidra 12.1.4. Complete returned evidence/limitations are retained under
`build/moving-hazards-001/static/`; pseudocode was cross-checked against
instructions. Analysis was closed without snapshot arguments. No original game,
copied code/oracle, debugger, hook, capture or installation tool ran.

The recovered [named-record layout](../../decomp-lemmings/docs/evidence/level_entity_tail.md)
and installed `build/moving-hazards-001/census.txt` establish all **29 models**:
22 deathrollers on the assigned 14 levels, five robins on four levels, one
scorpion and one buzzsaw. **18 rollers are DYNA; four are STAT** (nodes 24, 60,
and two on 68). Every model, contact volume and named clip resolves.

| Kind / installed example | Implemented behavior and authored data |
| --- | --- |
| Deathroller, node 20 `MM_E1_4SKILLS` (Sergeant Bash) | A spinning spiked drum follows its serialized loop. Path segments are `(214,29)+(0,10)t` for 2 s, `(214,49)+(-10,0)t` for 11 s, `(104,49)+(0,-10)t` for 2 s, `(104,29)+(10,0)t` for 11 s. Stored segment 1 / phase 5.7 samples `(157,49)` on first update; the pre-update placement is `(157,53,0.5)`. The drum spins at stored 2.2 turns/s. Continuous contact removes every victim with body pieces/pink particles. DYNA rollers cut ordinary terrain along the leading edge and reverse at steel. STAT rollers spin and kill without patrolling or cutting. |
| Buzzsaw, node 81 `VH05_JC1` (Don't Let them out!) | Fixed placement `(180,10,0.5)`; its authored `attack` transform clip loops every 1.2 s. Continuous contact removes all victims. Selector 0 has no particle-state constructor, so victims disappear without the roller's debris. Closed kill box: X 173..188, original Y 2..31. |
| Scorpion, node 63 `M07_mg_1` (Topsy Turvy) | Fixed placement `(188,27,0.5)`; `rest` 1.2 s, one-shot `attack` about 0.766667 s. Retains one contact victim until current clip completion, removes it with particles, then attacks and returns to rest. Closed kill box is X 171..175, original Y 27..31, from the relative volume, not the broad model bounds. |
| Robin, node 37 `M02_MG_1` (What a Fantastic Pair of Birds) | Both stored birds render and animate in place, at `(108,4,0.5)` and `(145,4,0.5)`. Same single-victim contract as the scorpion; `rest` 0.6 s and `attack` 1 s. First bird's closed box is X 106..110, original Y 4..12. |

Static call chains (files below are in the retained `static/` directory):

- `0x42576F` reads path indices, current path/segment, travel, speed multiplier,
  reverse flag and cut offsets. `0x42478F` calls contact/update `0x432E3F`, cuts
  the old leading edge, advances travel by `dt * speed` with reverse sign,
  samples `0x451997`, then recomputes boxes and model position. Open paths
  reflect; loops wrap; positions use stored start plus velocity times travel.
  `0x0042478f{,-asm}.json`, evidence
  `ev_9c5b036e1e1f0938dc155c54de9cf4ce5470b4e1df38f95a8f9fefe2aa18dc84`;
  `0x00451997{,-asm}.json` and `0x0042576F-asm.json` corroborate the operands.
- `0x424583` accepts route 0/1/toggle; `0x4245F1` and `0x425047` defer a
  change until a stored junction, preserving remaining travel. Enabled/disabled
  messages trace through `0x43645E/0x436471 -> 0x41DC00/0x41DC20`. The clone
  registers named targets before load-time lever fan-out and preserves stream
  overwrite order. Corresponding pseudocode/instruction files are retained.
- `0x4333D2` selects named rest/attack clips. `0x432E3F` uses +0x1D1 to collect
  all contacts or retain the first victim ID until model completion; attack
  +0x1D4 suppresses another kill until it completes. `0x430F4D -> 0x430C9C`
  adds relative boxes to broad-box minima and compares closed actor points.
  `0x00432E3F{,-asm}.json`, evidence
  `ev_7521707687641480f865a73b4b8637969cc149d26f48c788055c8d844403ebe9`.
  `0x4331D0` maps selectors 7, 8 and 14 to `0x40A6A0`; selector 0 has no state.
- Roller spin is the third auxiliary array, not a missing transform clip:
  `0x476277/0x47CDE7` reads node/axis/rate; `0x4764C4 -> 0x476EE0 -> 0x481F3C`
  writes Z rotation from absolute clip time. `0x426D90/0x426DB0` multiply turns
  by two and the stored pi at `0x4B0F88`. These instructions and pi bytes are
  retained. All installed roller tracks target node 1, axis 2, rate 2.2.

## Production path and limitations

`loadRetailLevelReport` constructs `LevelSetup::namedHazards` from structured
records and then links lever/pad targets. `RuntimeSimState::advanceElapsed ->
World::advanceElapsedMs -> updateSegment -> stepNamedHazards` runs after current
triggers/receivers; diagnostic `World::tick` also calls it. `advancePatrol`
updates terrain through its versioned material API. Victims leave Blocker fields,
become Dead once and update existing HUD/results. The existing death observer
reaches `TubeScene::emitDeathParticles`. Pending victim indices stay stable.

`BackdropScene::build` admits the actual serialized trees as `named-hazard`
roots. `main.cpp::prepareScene -> setNamedHazardStates` samples model clips and
spin from World's state, and maps moving positions through the accepted
FILE-to-runtime cylinder lattice adapter. It restores base matrices before each
absolute sample. Rendering does not advance the simulation. Pause freezes state;
fast-forward uses admitted time; retry reconstructs state and load-time messages.

**Provisional adapters:** integer feet/radial lane 0 for contacts; existing hybrid
clone timing for clip completion/contact scheduling; cross-class overlap order;
normalized quaternion interpolation; radial-heading preservation of authored
tilt/scale for moving models; existing mesh/fluid depth and particle presentation.
Terrain cutting edits materials and retains the clone's separate cell flag planes;
the original zeroes its packed cell word. Terrain debris and the stored
`DEATHROLL`/`birdcall` sounds are not implemented. `SHOW_KF3` contains an authored
infinite-duration zero-velocity endpoint: it is preserved, with a labelled numeric
guard keeping zero displacement finite if reverse wrapping adds infinity to
travel. This guard and large-dt endpoint normalization are clone safeguards, not
original arithmetic equivalence. No kind is drawn-but-inert. No original-fidelity
pass, natural hazard encounter, human win or full-project completion is claimed.

## Checks and preservation

- Registered UCRT64 `-O2 -g` build with assertions: `build/moving-hazards-001/final-build.log`.
  The PowerShell wrapper was execution-policy denied; direct registered tool paths
  with process-local runtime PATH succeeded, as in the existing verifier.
- New **legacy** `namedhazards`: PASS, **0.40 s**, exact run
  `build/moving-hazards-001/final-logic-ctest.log`. Covers open/closed paths,
  endpoints, steel/cutting order, junction requests, infinite waits, enabled state,
  closed contacts, loss/particle accounting, retained victims, attack rearming,
  and all 29 production model/clip bindings. Earlier failed development checks are
  retained, including the rejection of the authored infinity before correction.
- Inspected production clone images: `MM_E1_4SKILLS.png`,
  `MM_E1_4SKILLS-tick1.png`, `MM_E1_4SKILLS-tick6.png`, `roller-side.png`,
  `M02_MG_1.png`, `M07_mg_1.png`, `VH05_JC1.png` under the task build directory,
  each with its camera JSON/render log. The roller is narrow head-on; the rotated
  view shows its long spiked drum. Later samples show translation/spin. Birds,
  scorpion and saw are visible at authored positions. These are clone render
  observations, not original comparisons or scripted wins.
- All **15 promoted recordings** were run unchanged with the requested replay
  CLI; every result is exit 0, PASS, won=1, completed=1. Exact final-binary
  filenames, steps and results: `build/moving-hazards-001/final-replay-results.tsv`
  and `final-*.log`. No promoted level contains these four hazards, so these
  regressions do not establish a hazard level's playability.

| Replay level | Saved / required | Result |
| --- | ---: | --- |
| M06_KF_1 | 93/50 | PASS |
| MM_BASH_002 | 10/10 | PASS |
| MM_BASH_MINE_WLR1 | 20/10 | PASS |
| MM_BLOCK_001_A | 24/15 | PASS |
| MM_BLOCK_BOMB_A | 12/10 | PASS |
| MM_BOMB | 17/10 | PASS |
| MM_BUILD | 20/10 | PASS |
| MM_CLIMB_002 | 10/1 | PASS |
| MM_DIG_000 | 19/10 | PASS |
| MM_FLOAT | 10/1 | PASS |
| MM_MINE | 20/10 | PASS |
| MM_W_012 | 16/10 | PASS |
| MM_BASH_DIG_001_A | 20/10 | PASS |
| MM_BUILD_BLOCK_A | 15/10 | PASS |
| MM_FLOAT_CLIMB2 | 10/6 | PASS |

Changed: `src/sim/{World.h,World.cpp,RetailLevel.cpp}`,
`src/app/{BackdropScene.h,BackdropScene.cpp,main.cpp}`,
`src/__test__/test_namedhazards.cpp`, `CMakeLists.txt`, this report. Pre-edit
backups/task-only diff are under `build/moving-hazards-001/`. All earlier edits
are preserved. Supervisor PID 26408 owns RepositoryLease; no protected control
was edited. CIM process metadata was denied; Get-Process confirmed no running
clone/build before edits. Source is LF and scoped diff whitespace checks pass.
Replay/unit runs do not open saves; image runs redirected LocalAppData to private
`build/moving-hazards-001/appdata`. Normal and installed saves were untouched.
No assertion/timeout/replay file was modified; no gameplay fixture, frozen build,
new gameplay route, commit or background worker was added. The **smoke gate is
pending supervisor execution after worker exit**; no full suite ran here.

Handoff: all four kinds render and simulate; 29 installed bindings, the legacy logic check and 15 human-win replays pass.
Next: supervisor smoke and independent review, then the user's hazard-level playtest for acceptance.

**What to check:** Sergeant Bash (node 20): rotate to the roller near column 157,
watch it spin and traverse the rectangular patrol, then allow a lemming into its
path; it should die once with debris. Bridges on the leading edge should be cut;
steel should reverse the roller. What a Fantastic Pair of Birds (node 37): walk
past either bird; it should animate, take one victim per attack and return to rest.
Topsy Turvy (node 63): approach the scorpion's left contact at X 171..175,
original Y 27..31; it should attack and remove one victim. Don't Let them out!
(node 81): the saw should loop its authored animation and remove contacts in
X 173..188, original Y 2..31. Pause freezes all four; retry restores initial state.
