# actor-light-lookup-001

**needs_review.** Playable v1, One Way Ticket (node 22, `MM_ALL_8_001`). The opening
walk near the reverse-bash zone showed a brightness cutoff in [Playtest 13 C4](../playtest/2026-11-17-playtest-13.md).
The production draw sampled the projected cylinder position, so it could miss
the level's authored bands. This is a routine rendering correction, with Sonnet
recommended and Codex assigned.

## Evidence and implementation

Retained instructions from the registered `Lemmings Revolution.exe`
(SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`)
show `0x42E632/0x42E63E` loading `actor+0x10/+0x08` for `_ftol`. The call
at `0x42E64C` uses `0x403F40` on world `0x4C6A10`. It masks X to 8 bits,
halves it, and uses `0x403FC0` to add `128 * trunc(fileRowY)`. The map byte
selects an RGB triple copied by `0x403820`. [Instructions](../../decomp-lemmings/docs/evidence/actor_finalization.asm)
and [A4o/A4p findings](../lessons/actor-rendering.md) establish the input contract.
This worker read the reference hash and existing evidence.

- `main.cpp` builds the actor samples through `injectedWalkersFromWorld`.
  `terrainSampleFromLemming` takes terrain X and `height - 1 - simY`, and the
  existing motion tracker supplies fractional terrain coordinates for live
  rendering. `TubeScene::drawInjectedWalkers` sends each clip through
  `prepareInjectedWalker`, which now samples `plus8.x/y` for the sprite and
  its carried brick. Picking and highlight preparation reuse the same path.
  Inverted sprites use the same coordinates with their existing placement and
  V flip. Submerged figures keep their separate fluid-brightness rule.
- `World::stepMiner` signals a steel refusal, `emitMinerAxe` stores the terrain
  origin, and `drawDeathParticles` now looks up the thrown axe at its current
  terrain sample after X wrapping. The gravity-dependent flight supplies the
  position. Palette conversion, RGB modulation and alpha rules are unchanged.
- The offline `actorfixture` now takes explicit terrain coordinates for tint
  alongside its model-position arguments for projection. Its scorer decodes
  the retained `actor_source_position_bytes`. The two affected smoke assertions
  now use the recorded terrain pair `(59.68486785888672, 32)`, index `4125`.
  The prior index `4196` used the model pair. Both cells select entry 0 and
  grey 152 on `MM_CLIMB_002`, which masked the earlier mistake. Signed X,
  truncation, nonzero-cell, palette and alpha checks remain covered.

Changed `src/app/TubeScene.cpp`, `src/assets/LightMap.h/.cpp`,
`src/tools/actorfixture.cpp`, `tools/harness/verify_actor_sprite_a4f.py`,
`src/__test__/test_actorsprite.cpp`, `src/__test__/test_collisiongrid.cpp` and
this report.

## Checks and limits

- One Way Ticket's [stored map near the one-way zone](../../build/actor-light-lookup-001/level-light-near-zone.log)
  gives grey 107 at terrain X 66, file row 51, and grey 152 at X 68 on the same
  row. The former model pair selected index 6627, entry 0, at both. A temporary
  [in-memory draw check](../../build/actor-light-lookup-001/inspect-light.cpp)
  matched the installed map in 48 body draws (walk, bash, mine, dig, both
  gravities, three yaws) and four axe draws (both cells and gravities).
  The body check isolates tint with an existing visibility override, and the
  axe check uses empty collision. Human confirmation of the opening route and
  exact native Axe lighting remain open. [Results](../../build/actor-light-lookup-001/inspect-light.log)
- Registered UCRT64 build passed. Focused CTest passed 4/4 in 3.16 s
  (`actorsprite`, `actorcompose`, `collisiongrid`, `miner_axe`). Both retained
  A4f pairs passed the rebuilt fixture check at index 4125 and grey 152.
  The Python pixel scorer could not run because the registered Python lacks
  Pillow. Its syntax and the fixture's command arguments were checked directly.
  Older A4f reports lacking source coordinates can still use the no-light
  diagnostic. [Build](../../build/actor-light-lookup-001/build.log) ·
  [CTest](../../build/actor-light-lookup-001/focused.log) ·
  [Fixture checks](../../build/actor-light-lookup-001/fixture-direct.log)
- Every promoted recording ran sequentially through the rebuilt clone with
  `--game-dir "Lemmings Revolution" --replay <file>`. All 20 exited 0 with
  `PASS`, `won=1`, `completed=1). The checks used a private LocalAppData
  directory, and replay startup bypassed save loading. [Exact filenames and results](../../build/actor-light-lookup-001/replays.log)

| Level | Final step | Saved/needed |
| --- | ---: | ---: |
| MM_BASH_002 | 1844 | 10/10 |
| MM_BASH_MINE_WLR1 | 2272 | 20/10 |
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
| M06_KF_1 | 1544 | 91/50 |
| MM_E1_4SKILLS | 2506 | 20/10 |
| MM_E2_4SKILLS | 2142 | 14/10 |
| MM_BASH_DIG_001_A | 3241 | 20/10 |
| MM_BUILD_BLOCK_A | 2482 | 15/10 |
| E19_KF_2 | 6034 | 24/20 |
| E20_MG_1 | 12810 | 7/5 |
| MM_ALL_8_001 | 13812 | 46/40 |

## Handoff

- Terrain lighting is implemented for the body and axe. All promoted replays and focused checks passed.
- Mandatory smoke is pending supervisor execution after worker exit, followed by independent review.

**What to check** — Open **One Way Ticket** and watch the opening walkers near
the reverse-bash zone. They should darken in the authored bands under the
ledges and brighten as they leave. Right-drag around the cylinder and check
that the shade at a fixed terrain position stays the same as the camera turns.
