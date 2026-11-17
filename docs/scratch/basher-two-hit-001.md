# basher-two-hit-001

**needs_review; supervisor smoke pending.** Playable v1, playtest-ledger work
order, [playtest 11 A9](../playtest/2026-11-16-playtest-11.md): the user saw two
punches per wall piece on **Sergeant Bash, node 20 / MM_E1_4SKILLS**, bottom
first, with sound and debris on both. The clone cleared all four rows and
advanced on its first punch. Restoring the intermediate collision/rendered
shape is the next dependency. Difficulty difficult; recommendation Opus;
assigned Codex override used.

## Static evidence

Started from accepted [basher](basher-fix-001.md),
[terrain rendering](terrain-render-001.md), [debris](bash-particles-001.md),
[audio](gameplay-sounds-001.md) and [work audio](work-sounds-001.md), plus
retained [round 5](../../old/docs/scratch/phase3-round5-live.md).
Used reverse-engineer-anything and static REA/Ghidra **12.1.4** on only
`Lemmings Revolution/Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Complete returned pseudocode/instruction packets, Evidence IDs and profile
digest `3c06738e45e144d5f073edbd3244c6bff46222f389525fe659bc92dca2ccdece`
are retained under [static/](../../build/basher-two-hit-001/static/).

| Static observation | Instruction cross-check |
| --- | --- |
| `40BBA0` registers the two punches in one animation cycle: rows 0/1 at 12/16%, then 3/2 at 60/64%. | Loop index shifted left two at `40BCD2`, `40BD38`, `40BE0A`, `40BE70`; callback `40C357` and arguments 0/1/3/2 stored at `40BCDE..40BCE8`, `40BD44..40BD4E`, `40BE16..40BE20`, `40BE7C..40BE86`. |
| `40C357` addresses an ordinary leading grid cell at base row + argument times gravity. | Multiply/add at `40C3E8..40C3F4`, lookup `40C5E2`, cell helper call `40C61A`. |
| `40C0AF` clears the **whole addressed row cell**, with no half-cell shape word in this path. The observed wall piece spans four such rows. | Refusal masks 2, facing 4/8, blocker 800 at `40C0F8/40C104/40C110`; zero store `40C16D`; rebuild `40C18C`; subpasses 0/3 select debris at `40C191..40C1C5` and Mine at `40C317..40C32F`. |
| `40C6E8` commits one column after both punches, at the registered 92% event. | Record index 23, shift/store/callback `40BF3F..40BF55`; facing addition, wrap/truncate, actor X store `40C70F..40C73C`. |

These are static observations, not an original run/comparison. The earlier
four-cells-per-cycle evidence remains valid; the removed approximation was
doing all four at the first record. REA was closed without snapshot arguments.

## Production change and limits

`main.cpp` skill input -> `World::assign(Basher)` resets frame/cycle count ->
`World::step -> stepBasher -> clearSkillBox -> Terrain::clearBox/set/version`.
The first punch removes feet rows **0/1**; the second removes **2/3**. Both
successful cuts notify `TerrainCutObserver -> TubeScene::emitTerrainDebris`
and queue Bash/Work -> `CuePlayback -> Mine`. The surviving upper rows stay
solid for every collision consumer. At 92% the lemming advances one column;
the existing grace count and post-advance ground/fall probe still apply.

Rendering follows the existing `runApplicationLeg -> TubeScene::syncTerrain ->
rebuildCellMeshCells -> CellMesh Builder::partLists/mesh0/mesh4 -> render`
path **after each punch**. The row writes already regenerate the partial wall
rectangle, so Terrain/TubeScene/CellMesh need no additional production change
or invented subcell shape format. The installed-data legacy check confirms
two changed collision rows and new mesh faces after each punch on **Just Bash,
MM_BASH_002**, wall column 152, clone rows 52..55 (file rows 43..40).

**Timing:** the cycle stays **1440 ms**, one column per cycle, with two punches
instead of one. Steady crossing time per column therefore does **not double**.
The upper half clears at 60% rather than 12% (nominally 691.2 ms later), and
advance moves from 12% to 92% (nominally 1152 ms later). At the default sampled
5 Hz start, the two cuts are ticks 0/4 and advance tick 6; successive columns
still average 7.2 ticks. This changes intermediate simulation state even when
a recording's later checkpoints still agree.

**Provisional:** each punch's close row writes (12/16 and 60/64) are coalesced
at its first record on the existing 5 Hz boundary; no native sub-frame clock
or 1/13-cell nudges were transplanted. The original's upper-row order 3/2 is
not separately visible in this adapter. Sound/debris require an actual cut;
empty grace swings stay silent. Both punches retain the existing whole-box
steel preflight, open-air/grace rule, upright tunnel footprint and support
probe. The existing overhang difference remains: the air-stop preflight sees
all four rows, whereas native arg 0 sees the feet row. No one-way retuning:
native facing refusal bits are established above, but the existing Terrain
importer does not retain bash/mine 4/8 flags; that omission is unchanged.
Inverted bashing and original animation alignment remain unproven. This is
a player-visible clone fix, not timing/render fidelity acceptance.

## Checks and preservation

Registered GCC 16.1.0/UCRT64 CMake/Ninja built `webolution`, `test_world`,
`test_basher_two_hit`, `test_gameplay_sounds`, `test_terrain_debris`, eight jobs:
[build.log](../../build/basher-two-hit-001/build.log).
Focused checks **4/4 PASS, 0.25 s wall**:
[focused.log](../../build/basher-two-hit-001/focused.log),
[JUnit](../../build/basher-two-hit-001/focused.xml).
Existing World checks include steel refusal, open-air stop and grace tunnelling.
The new **legacy** unit (0.09 s) guards two punches on one column, per-punch
sound/debris, both seam directions, surviving collision rows, delayed advance
and installed mesh propagation; it is not a gameplay route or human acceptance.

The smoke `testBashFootprintAndCadence` expectation of four removed cells at
each event encoded A9 itself. Under the explicit playtest-defect exception it
now requires two cells per punch, twelve punches instead of six whole bites,
the same five-cycle/36-tick cadence, bottom-first survival and delayed advance.
Its steel, grace, air-stop, tunnel-boundary and other tests are unchanged.
No assertion/timeout was weakened, no test removed, and no replay/check edited.

Ran `webolution --game-dir "Lemmings Revolution" --replay <file>` sequentially
on **all 17 active promoted human recordings**: **17 PASS, exit 0, won=1,
completed=1**. Exact filenames, commands, assignments and logs:
[replays.json](../../build/basher-two-hit-001/replays.json).
No divergences or retirements. Retired files were not run or changed.

| Level | Final step | Saved/required |
| --- | ---: | ---: |
| MM_BASH_002 | 1844 | 10/10 |
| MM_BASH_MINE_WLR1 | 2272 | 20/10 |
| MM_BASH_DIG_001_A | 3241 | 20/10 |
| MM_E1_4SKILLS | 2506 | 20/10 |
| MM_BLOCK_001_A | 1619 | 24/15 |
| MM_BLOCK_BOMB_A | 1546 | 12/10 |
| MM_BOMB | 954 | 17/10 |
| MM_BUILD | 1685 | 20/10 |
| MM_CLIMB_002 | 1252 | 10/1 |
| MM_DIG_000 | 1014 | 19/10 |
| MM_FLOAT | 978 | 10/1 |
| MM_MINE | 1137 | 20/10 |
| MM_W_012 | 1742 | 16/10 |
| MM_E1_3_SKILLS | 2095 | 10/5 |
| M06_KF_1 (playtest 11) | 1544 | 91/50 |
| MM_E2_4SKILLS (playtest 11) | 2142 | 14/10 |
| MM_BUILD_BLOCK_A | 2482 | 15/10 |

The first four use Bashers (accepted assignment steps **653/1478**,
**1071/1637**, **298/367/1281/1407/1828/1946/2061/2699/2865/3068**,
**1090/1448**, respectively). The other thirteen have no accepted Basher
assignment and still reproduce. Replay checks do not observe every cut.

Changed: `src/sim/World.cpp`, `World.h`, `src/__test__/test_world.cpp`, new
`test_basher_two_hit.cpp`, one legacy registration in `CMakeLists.txt`, this
report. Incoming source/CMake edits are preserved under
[baseline/](../../build/basher-two-hit-001/baseline/);
[task-only.patch](../../build/basher-two-hit-001/task-only.patch) isolates the
implementation/test changes. LF checks and scoped `git diff --check` passed.
Supervisor PID **15712** holds RepositoryLease. CIM process metadata was denied;
Get-Process showed no clone/build runner at entry. No protected controls or
shared instructions were edited. No original, oracle, debugger, hooks, capture,
install, subagent, concurrent writer, commit or push ran. Commands used private
`build/basher-two-hit-001/appdata`; replays open no save, and tests use no save.

**Smoke/sweep/recovery verification is pending supervisor execution**, not
claimed from an earlier task. No full/profile gate was duplicated. Independent
review follows that gate; acceptance of A9 is the user's next playtest. All
historical fidelity gates, parks and full-project criteria remain unchanged.

Handoff: two bottom-first punches now update collision, mesh, sound and debris before advance; 4 focused checks and all 17 promoted wins pass; needs_review.
Next: supervisor smoke and independent review, then human confirmation of A9.

What to check: **Just Bash, node 6 / MM_BASH_002**, assign Basher at either earth
pillar: each punch makes Mine sound and chips, first removing the bottom half,
then the top half, before advancing. The four-row tunnel and floor stay intact;
steel still refuses and sustained open air ends the bash. Repeat on **Sergeant
Bash, node 20 / MM_E1_4SKILLS**; two punches should be visible per wall slice.
