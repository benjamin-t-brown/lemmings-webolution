# steel-blast-dig-tilt-001

**needs_review; supervisor smoke pending.** Playable v1,
[playtest 11 A1/A10](../playtest/2026-11-16-playtest-11.md).
Difficulty routine; recommendation Sonnet; assigned Codex override used.

## Cause and production change

A1 reports exploding lemmings destroying metal; A10 reports metal tilting
beside snow digs on **The Crowded House, node 21, E20_MG_1**. The three retained
attempts contain accepted upper-shelf Digger assignments at steps 346 (112,13),
132 (119,13), and 172 (129,13). Attempts were read, not replayed or promoted.

Bomber: `World::assign -> fuse expiry in step -> stepExploder -> explode ->
Terrain::clearExplosion`. Nuke: `nuke -> stepNuke -> nukeExploding primary
Exploder -> stepExploder -> explode`, sharing that crater. **Collision already
skips Steel**, as does `clearBox`; no additional simulation check was needed.
Collision is a cell mask, with no neighbour tilt writer.

Visible reshaping: `Terrain::set/version -> runApplicationLeg ->
TubeScene::syncTerrain -> cylinderTemplateVertex -> rebuildCellMeshCells ->
Builder::partLists/mesh0/mesh4 -> TubeScene::render`. This shared path handles
explosions, bash, mine, dig and building. It merged steel/ordinary faces and
created new corners using collision height **96**. This collapses new rows
80..96 to Y=80, while stored corners still reach Y=96.

Runtime rebuilding now partitions runs/merges at steel boundaries. New corners
touching steel use serialized coordinate height **112**, preserving metal's
plane; ordinary corners retain the existing provisional shaping. Shared steel
edges use protected coordinates. Full-block wall flags and load-time meshing
retain their behavior. This selective partition is **provisional playable-v1
policy**, not recovered original material-specific deformation.

Concrete installed example: steel **file cell (141,81), clone (141,14)**,
beside snow column 140. Its newly exposed corner must be Y=81, previously
Y=80. [Asset calculation](../../build/steel-blast-dig-tilt-001/vertices.txt)
and [audit](../../build/steel-blast-dig-tilt-001/vertex-audit.txt) confirm all
397 stored vertices incident to steel on E20_MG_1 equal the height-112 template.

## Static evidence and limits

Registered reference: `Lemmings Revolution/Lemmings Revolution.exe`,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Used reverse-engineer-anything, static REA/Ghidra 12.1.4; returned packets retain
the exact identity/profile/Evidence IDs under
[static/](../../build/steel-blast-dig-tilt-001/static/).
[Instruction cross-check](../../build/steel-blast-dig-tilt-001/static/crosscheck.txt):

- Blast `43F510`: `43F61B/43F620` skips steel bit 2;
  `43F62B/43F633` skips **0x200 water zone**;
  `43F63E/43F645` skips **0x800 blocker field** before zeroing at
  `43F7F8` (the older F7F5 citation names the preceding pointer load).
- Dig `415A43`: `415B1B` tests steel; `415B2A` tests the upright/inverted
  refusal mask 0x10/0x20. These flags are not another material.
- Dig `415D62` and the blast tail reach `43B0A1 -> 43AE6C`, queuing
  neighbouring blocks. Missing keys reach `43A31D -> 43D2B8` at
  `43A3AE`; `43D2F9/43D334` use world+28.
  [Accepted coordinate-height explanation](basher-occlusion-001.md)
  identifies 112 separately from the 96 collision rows.

No additional material exemption was established in this bounded trace.
The water-zone blast exemption remains an existing clone omission in
`clearExplosion`; reporting it does not expand this steel fix. Blocker
protection remains. Static analysis was closed without snapshot arguments.
No original execution/comparison occurred. Human confirmation, original
deformation fidelity and historical timing/render gates remain unresolved.

## Checks

Registered GCC/CMake/Ninja build passed:
[build-final.log](../../build/steel-blast-dig-tilt-001/build-final.log).
**3/3 focused checks passed**:
[log](../../build/steel-blast-dig-tilt-001/focused-final.log),
[JUnit](../../build/steel-blast-dig-tilt-001/focused-final.xml).
`terrain` and `cellmesh` preserve existing checks. New **legacy**
`steel_terrain` (0.12 s) covers Bomber/Nuke collision, mixed-block flags,
installed dig/blast steel faces, and the dig's protected side/ordinary shaping.
These direct-edit invariants are not gameplay routes or human acceptance.
An initial unit-setup error used `fillBox` to replace occupied earth; corrected
to `set`, since filling only writes empty cells.

Ran every active promoted recording with
`webolution --game-dir "Lemmings Revolution" --replay <file>`.
All **17 PASS, exit 0, won=1, completed=1**. Exact filenames, commands, steps
and individual logs: [replays.json](../../build/steel-blast-dig-tilt-001/replays.json).

| Level | Saved/required |
| --- | --- |
| MM_BASH_002 | 10/10 |
| MM_BASH_MINE_WLR1 | 20/10 |
| MM_BLOCK_001_A | 24/15 |
| MM_BLOCK_BOMB_A | 12/10 |
| MM_BOMB | 17/10 |
| MM_BUILD | 20/10 |
| MM_CLIMB_002 | 10/1 |
| MM_DIG_000 | 19/10 |
| MM_FLOAT | 10/1 |
| MM_MINE | 20/10 |
| MM_W_012 | 16/10 |
| MM_E1_3_SKILLS | 10/5 |
| M06_KF_1 (playtest 11) | 91/50 |
| MM_E1_4SKILLS | 20/10 |
| MM_E2_4SKILLS (playtest 11) | 14/10 |
| MM_BASH_DIG_001_A | 20/10 |
| MM_BUILD_BLOCK_A | 15/10 |

**No divergence/retirement request.** Recordings and replay checks were
unchanged. Replay validates simulation, not the rendered fix.

Changed: `src/app/TubeScene.{cpp,h}`, `src/assets/CellMesh.{cpp,h}`,
`src/__test__/test_steel_terrain.cpp`, `CMakeLists.txt`, this report.
Incoming main/audio, hooks-log, queue/status and report edits were preserved.
Live processes were checked; supervisor PID 15712 owns RepositoryLease.
No manual protected edits. [Backups and HEAD main](../../build/steel-blast-dig-tilt-001/before/)
are retained. Final checks/replays use private task `appdata`; units/replay
open no save. Installed/normal saves were not written. No original/oracle,
hooks, debugger, capture, installs, subagents, background jobs, commits or
pushes. Scoped diff/LF checks pass. No full verifier profile was duplicated.

Handoff: steel collision and runtime mesh checks pass; all 17 promoted wins reproduce.
Handoff: needs_review; supervisor smoke and independent review next, followed by human visual confirmation.

What to check: **The Crowded House (node 21, E20_MG_1)**, dig the upper snow
shelf immediately left of the steel pillar at columns 141..143 (file row 81,
clone row 14): snow may tilt into the cut; metal stays straight and solid.
Use Bomber beside metal, or Nuke beside/on that pillar: ordinary terrain
craters while every metal cell remains visible and supports/collides as before.

