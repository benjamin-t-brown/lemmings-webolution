# Playtest 27 S3 and S4 · Explosion flecks and skill debris

**Needs review. Supervisor smoke profile pending.** The playtest reported a
blood-like explosion cloud and short throws from digging and mining. The
promoted wins for **Just Bomb** (MM_BOMB, node 2), **Just Dig** (MM_DIG_000,
node 7) and **Just Mine** (MM_MINE, node 5) supply the matched clone frames.
[Playtest 27](../playtest/2026-11-21-playtest-27.md#requests)

## Production changes

- **S3.** Bomber and nuke both enter `World::stepExploder → explode`.
  `runApplicationLeg` forwards the death observer to
  `TubeScene::emitDeathParticles`. The clone previously emitted 100 tumbling
  flecks with red 96–255 and green/blue 16–32 from a burst-local RNG. The
  **user-directed deviation of 2026-11-21** uses **20 brown flecks**. Evenly
  spaced samples of the actual removed cells use `terrainDebrisMaterial` and
  the cell-centre texel to shade `kTerrainChunkFallback` (128, 104, 72) by
  0.75–1.25. A missing sample uses that brown directly. The existing six body
  pieces, textured crater chunks, decal and ten pink fall-splat flecks are
  preserved. [Emitter](../../src/app/TubeScene.cpp#L1152)
- **S4.** `World::stepDigger/stepMiner → clearSkillBox → TerrainCutObserver`
  feeds `TubeScene::emitTerrainDebris`. For chunk index `i = 0..3`, the
  Digger's tangential speed changes from `(i − 1.5) × 3` to
  `(i − 1.5) × 10`. The Miner goes from `−facing × (3 + i)` to
  `−facing × (10 + 2i)`. Both rise speeds change from 9–13.5 to 11–16.25
  terrain units/s, gravity from 16 to 20 units/s² and life from 1.6–1.9 to
  1.7–2.0 s. Rise and gravity mirror for inverted actors. The analytic arcs
  put the centres 7.5–10.2 cells past launch height in the direction of
  gravity by expiry. This is provisional clone tuning with no debris
  collision. Basher retains its existing launch and lifetime.
  [Emitter](../../src/app/TubeScene.cpp#L1107)

Both observers read completed simulation changes. The render pass and its
4096-particle bound stay in place. Four skill chunks still spawn per removed
cell. Sampled Bomber frames peak at 288 particle triangles, down from 1248.
The skill sequences show up to 36 extra triangle submissions in individual
late frames during overlapping cuts.
Wall-clock frame cost was unmeasured. [Task-only patch](../../build/particle-tuning-001/task-only.patch)

## Static and clone evidence

Registered x86 reference SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Ghidra 12.1.4 shows RGB arguments (255, 32, 32) at
`0x43F8D2–0x43F8ED` and `PUSH 0x64` at `0x43F901` before the count setter.
The `0x492E33` pseudocode shows primitive type 1 at the indirect call
confirmed at `0x492F66`, consistent with the retained point-list reading.
The `0x40C960` xrefs include Dig `0x415BE8` and Mine `0x418B69`.
The constructor's 12 slots and height-based life come from the static
reading. Their effective motion in clone terrain units remains unresolved.
REA closed without a snapshot. [Static packets](../../build/particle-tuning-001/static/rea.json) ·
[Retained blast](explosion-pieces-3d-001.md) · [Retained debris](debris-3d-001.md)

The clone probe took eight frame pairs per level from the existing recordings.
The table links complete sequences and shows one measured frame per level.
All pairs share a view and exact level clock. [Frame measurements](../../build/particle-tuning-001/frame-comparison.tsv) ·
[Probe source](../../build/particle-tuning-001/frame-probe.cpp)

| Route | Matched sample | Visible change | Sequences |
| --- | --- | --- | --- |
| Just Bomb | 14,135 ms, yaw 132, zoom 1 | 5,448 strongly red effect pixels → 0. Brown flecks, body pieces and crater chunks are visible. | [Before](../../build/particle-tuning-001/before/MM_BOMB/) · [After](../../build/particle-tuning-001/after/MM_BOMB/) |
| Just Dig | 7,562 ms, yaw 83, zoom 1 | Visible debris spans 148 → 265 px across the cut. | [Before](../../build/particle-tuning-001/before/MM_DIG_000/) · [After](../../build/particle-tuning-001/after/MM_DIG_000/) |
| Just Mine | 8,366 ms, yaw 227, zoom 1 | The farthest visible chunk moves from x=384 to x=494. At 1,418 ms of age none of the first eight chunks are visible, and all eight are still live. | [Before](../../build/particle-tuning-001/before/MM_MINE/) · [After](../../build/particle-tuning-001/after/MM_MINE/) |

## Checks

The GCC `smoke_tests` target built. Four focused smoke-labelled tests passed
`actorcompose`, `backdropscene`, `terrain` and `world`. The production
`webolution --game-dir "Lemmings Revolution" --replay <file>` CLI reproduced
**all 29 promoted wins**, with `won=1 completed=1` and no divergences. Just
Bomb saved 17/10 at step 954, Just Dig 19/10 at step 1014 and Just Mine 20/10
at step 1137. All runs used private LocalAppData and the registered assets
in place. The original was inspected statically. [Build log](../../build/particle-tuning-001/smoke-target-build.log) ·
[Focused smoke log](../../build/particle-tuning-001/focused-smoke.log) ·
[Every replay result and log path](../../build/particle-tuning-001/replays.tsv)

Changed `src/app/TubeScene.cpp` and this report. The task build folder retains
the probe source, logs, and frames. The recorded views cover upright Bomber,
Digger and Miner. Shared nuke routing and gravity reversal were checked in source.

Handoff **needs_review**. The supervisor smoke and recovery gate is pending.  
Next action is independent review and human playtest.

**What to check.** In **Just Bomb**, assign Bomber on a wooden ledge and look
for a sparse brown scatter with body pieces and crater chunks. In **Just Dig**
and **Just Mine**, assign the corresponding skill on the starting ledge and
watch the debris clear the cut and descend.
