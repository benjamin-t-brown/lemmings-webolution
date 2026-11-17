# fall-out-death-001

Status: **needs_review**; supervisor smoke and independent review pending.
Playable v1, [playtest 10 Z5](../playtest/2026-11-16-playtest-10.md),
**The Abyss**, node 18, **MM_E2_4SKILLS**. Difficulty difficult; recommendation
Opus; assigned Codex override used. Next dependency: remove the artificial
landing that lets a Floater survive this demonstrated open pit.

## Recorded failure and production change

The winning recording `MM_E2_4SKILLS-1794876758706780100-3.replay` identifies
**actor 0**: at step **1133**, clock **39,898 ms**, it walks left from `(100,63)`
into the hole at **(99,63)**. It becomes a Floater at `(99,74)`, step 1149.
Before this task, step **1195**, clock **42,098 ms**, changes it into a Walker
at **(99,95)**, fall distance reset to zero, **dead=0**. It walks along the
bottom until the final Nuke. There is no lower platform or vertical wrap.

The retry `MM_E2_4SKILLS-1794876708170960000-2.replay` instead includes ordinary
fallers 1/2 entering at **(75,43)**, steps 1173/1279. They already splat at
`(75,95)`, steps 1243/1353, after 52 cells; final **0 saved, 2 dead** is unchanged.
The missed death is the winning attempt's Floater, exempt from distance splats.
[Entry traces](../../build/fall-out-death-001/before/) retain both recordings.

Production path: `main.cpp -> replayLevel` (headless) or
`runApplicationLeg -> RuntimeSimState::advanceElapsed -> World::advanceElapsedMs
-> updateSegment -> stepElapsed/step -> stepFaller -> groundAt -> Terrain::at`.
`Terrain::at(y >= height)` supplies a Steel sentinel, previously accepted as
landing support. The existing bounds-loss branch covered inverted fallers only.

`World::stepFaller` now applies that loss branch to upright actors at the last
row, **before** the out-of-grid support probe. It resets frame/budget, enters
Splatter, increments `dead_` once, emits Splat and calls the existing death
observer. `main.cpp` connects that observer to `TubeScene::emitDeathParticles`;
the normal render path hides the intact Splatter and draws the pieces. Real
support on row 95 lands feet on row 94, so that landing remains safe. Terrain's
sentinel, landing exemptions, fall speed, distance threshold and clocks stay as
they were; its comment now documents the boundary responsibility.

After, the recorded actor enters **Splatter at (99,95), step 1195**, distance
32, **dead=1**; it retires to Dead at step 1243. A diagnostic continuation
retains every recorded input and assignment check, logging count differences
instead of stopping at the first one: it still saves **12/10** and reaches
Results at step 2193. The next recorded update is refused because completion
is earlier. This continuation is **not a strict replay PASS**.
[Trace and continuation log](../../build/fall-out-death-001/continued-win.log).

## Level data, static evidence and provisional boundary

Production archive readers find a **256 x 96** grid. At column 99, original
rows **0..31** (simulation rows 95..64) are all **zero**: an open cylinder
bottom, with no floor or encoded kill/fluid zone. Column 75 is also open below
the retry's ledge. There are 25 open bottom cells on The Abyss and 231 supported
ones. [Cells](../../build/fall-out-death-001/abyss-cells.csv) and
[102 installed levels' bottom support counts](../../build/fall-out-death-001/edge-data.csv)
come from resolved map nodes and production collision/terrain readers; 49 levels
have some open bottom cells. These are static asset observations.

Reused accepted [gravity-gadgets-001](gravity-gadgets-001.md) and
[death-particles-001](death-particles-001.md), with retained full returned
evidence copied to [static/](../../build/fall-out-death-001/static/).
Reference: repository-local `Lemmings Revolution/Lemmings Revolution.exe`, x86,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
No new REA session or original execution was needed.

- `416433` pseudocode, `ev_34a2e470c1ee596182b4c7d27791325753b1b2ea3382c17949c2042b0043f691`,
  cross-checked against `static-fall-ins.json`,
  `ev_b879fd6905f9c1bc4d2161f2fb124977043543deb48fdc7179e51e908e553e6d`:
  **4164CE..4164FC** checks raw actor Y against zero and cylinder height;
  **4164FE..41653A** constructs `40B3A0` and attaches it via `42E696`.
  This precedes trampoline, landing, distance and Floater/species exemptions.
- The constructor is type **1A**, vtable `4B0668`; entry **40B3EC** clears
  render flag at **40B4AF** and emits the established loss effects. The later
  landing distance comparison is strictly **>30**, not the open-edge rule.

**Provisional:** native raw actor Y, biased support sample (`42F22E`) and
clamping (`40DF50`) do not map exactly to this integer feet grid. Upright loss
at row `height-1`, and the retained inverted first-outside-cell rule, are clone
adapters. The static original bounds loss is established; exact fractional
boundary placement and original effect duration are not. This is a playable
clone correction, without reopening original timing/fidelity gates.

## Checks and changed sweep outcomes

Registered UCRT64 GCC/CMake built the clone, World unit, sweep and new legacy
boundary unit. [Focused sweep log](../../build/fall-out-death-001/focused-sweep.log):
**19/19 PASS**, 92.86 s, covering World plus 17 chunks and the report, 102 levels.
Compared with the task-entry matrix retained from `build/level-sweep-001/`
(mtime 2026-11-17 01:19:19 UTC), these are all changed gameplay counters/states:

| Node / installed level | Dead before -> after | Active before -> after |
| --- | ---: | ---: |
| 33 / M11_KF_1, Designed with love | 0 -> 94 | 99 -> 5 |
| 51 / M21_KF_1, Walk the plank or join the crew? | 0 -> 48 | 100 -> 52 |
| 69 / H15_MT_1, Rocket Science. | 0 -> 46 | 50 -> 4 |
| 89 / VH13_KF_1, Anger, Love, Hate | 0 -> 41 | 50 -> 9 |
| 101 / MM_H_003, It's Going to Take Time | 0 -> 100 | 100 -> 0 |

All formerly supported bottom walkers now take open-edge losses. Node 101 also
changes `world_finished` from false to true. Saved counts and sweep completion
classifications are unchanged everywhere; The Abyss's idle route is unchanged
(13 dead, 2 walkers at the observation cutoff). Full counter/state differences:
[sweep-changes.txt](../../build/fall-out-death-001/sweep-changes.txt), with both
matrices retained beside it. This idle window is not a human playability test.

[Final units](../../build/fall-out-death-001/final-units.log): **2/2 PASS**, 0.13 s.
New `fall_out` takes 0.04 s, stays **legacy**, and checks upright/inverted,
ordinary/Floater, tick/elapsed movement: one loss/cue/particle notification at an
open edge, ordinary retirement, and survival on real last-row support. It adds
no route, replay fixture, frozen build or identity gate. No existing assertions
or timeouts changed. Inventory remains **95 smoke**, now **238 total**.
Mandatory smoke/recovery verification remains the supervisor's next action.

All **17 promoted win recordings** were run individually through
`webolution --game-dir "Lemmings Revolution" --replay <file>`. Exact files,
commands, exits and logs: [replays.json](../../build/fall-out-death-001/replays.json).

| Level | Strict replay result, saved/needed |
| --- | --- |
| M06_KF_1 | PASS 93/50 |
| MM_BASH_002 | PASS 10/10 |
| MM_BASH_MINE_WLR1 | PASS 20/10 |
| MM_BLOCK_001_A | PASS 24/15 |
| MM_BLOCK_BOMB_A | PASS 12/10 |
| MM_BOMB | PASS 17/10 |
| MM_BUILD | PASS 20/10 |
| MM_CLIMB_002 | PASS 10/1 |
| MM_DIG_000 | PASS 19/10 |
| MM_FLOAT | PASS 10/1 |
| MM_MINE | PASS 20/10 |
| MM_W_012 | PASS 16/10 |
| MM_E1_3_SKILLS | PASS 10/5 |
| MM_E2_4SKILLS | Intended divergence at step 1195: dead 0 -> 1 |
| MM_BASH_DIG_001_A | PASS 20/10 |
| MM_BUILD_BLOCK_A | PASS 15/10 |
| MM_FLOAT_CLIMB2 | PASS 10/6 |

The separate Abyss retry also **PASSes**, 0/10, step 1360. The changed win file
and all other recordings are left untouched. No legacy test was removed.

Changed: `src/sim/World.cpp`, comment in `src/sim/Terrain.cpp`,
`src/__test__/test_fall_out.cpp`, its legacy registration in `CMakeLists.txt`,
and this report. Incoming source/CMake edits were preserved;
[entry backups](../../build/fall-out-death-001/entry/) retain them and HEAD main.
Supervisor PID **4628** owns RepositoryLease; Get-Process found no clone/build
runner at entry (CIM command-line inspection was denied). No protected controls
were edited. Replays open no SaveStore; checks used private LocalAppData
`build/fall-out-death-001/appdata`. Installed/normal user saves were unused.
No original/oracle, debugger, hooks/capture, installs, agents, background worker,
commit or push ran. LF and scoped diff checks pass. Visible presentation remains
for the user's playtest; static/source/replay evidence does not establish fidelity.

Handoff: open-bottom falls now count one normal splat loss, including Floaters; focused sweep/units pass, one intended replay divergence documented.
Handoff: needs_review; supervisor smoke and independent review next, followed by the user's playtest.

What to check: **The Abyss / MM_E2_4SKILLS**, let a Floater walk left off the
lower ledge into the hole at **(99,63)**. Expect it to fall to the open bottom,
die with the ordinary splat pieces/sound and one loss, and never walk along an
invisible floor. A short fall onto an actual lower platform must still survive.
