# explosion-decal-001 — installed explosion surface mark

**needs_review; supervisor smoke pending.** Playable v1 presentation step,
asset-census-001 rank 9, Codex override (difficult / recommended Opus).
Installed route: **node 2 / MM_BOMB / Just Bomb**, with all levels potentially
affected through Nuke. The census positively traces a missing surface mark;
playtest 1 P2 supplies the earlier missing explosion/terrain feedback context.
P2's terrain fix and playtest 7 W4's withdrawn particle complaint remain closed.
The mark adds surface feedback to the existing crater and particles.

Production: `World::stepExploder -> explode -> deathObserver` (after the
unchanged crater edit) -> `runApplication`'s existing observer ->
`TubeScene::emitDeathParticles -> emitExplosionDecal`. The observer passes
`World::nukeRequested()` at the time of the blast. Only explosion events
emit marks, including blasts that remove no earth. The scene loads the first
fifth-table texture once and caches at most four 8×16-sector patches per event
with fixed-size seam splits. `advanceDeathParticles(levelClockMs)` ages and expires marks
even after particles are gone. `renderFrame -> drawExplosionDecals` draws after
completed scenery and before actors/particles, outside the opaque tube cache.
Pause holds the existing clock, fast-forward advances it, and Retry replaces
the scene. Terrain and crater outcomes still follow the existing simulation.

Static authority: repository-local **Lemmings Revolution.exe**, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`,
Ghidra **12.1.4**, analysis profile
`3c06738e45e144d5f073edbd3244c6bff46222f389525fe659bc92dca2ccdece`.
Retained evidence comes from the [census revision](../../build/asset-census-001/revision-evidence.md)
and the [task's previous static pass](../../build/explosion-decal-001/static/).
The static session was closed by the previous worker. The resumed worker
checked the registered hash and used the retained instructions and pseudocode.

- `4082B6 -> 43B31E(0, position)` selects **~splat**.
  The census's `revision-instructions-00408227.json` cross-checks the zero
  selector and (+1,+1.5) position adjustment. `pseudo0.json` / `ins0.json`
  (`ev_d401982ab700e3dd…`) cross-check the rectangle
  (-5.5,-4.5)..(+2.5,+3.5), mode 2, and the literal 20/3-second branches.
  Net rectangle relative to the adapted actor is X -4.5..+3.5, Y -3..+5.
- `441E26 -> 441CB2 -> 43FCAA` splits at cylinder sectors, maps corners
  through **43D56C with Z=0**, then builds two triangles per patch.
  `ins2.json`, `more0.json`, `end3.json` and `ins3.json` retain the split,
  lifetime stores at record +170/+178, corner interpolation and triangle
  indices. `43FB14` binds the texture (`more1.json`).
- `43B419` deletes records with remaining life <=0 and supplies both
  triangles RGB **32 × remaining / initial** (`pseudo1.json`, `ins1.json`,
  `ev_afc959d347772dd2…`). `43B384..43B39E` selects **20 seconds** by default
  and **3 seconds** when `4CBB40` is set. The earlier task draft left this
  flag unidentified. The retained Nuke trace identifies the field:
  Nuke's `444AB1` stores to level + `5138`, which resolves to `4CBB40`.
  The replay Nuke handler also sets the absolute address at `457017`.
  The clone samples the existing Nuke flag on each death event, so marks
  made before Nuke keep their original lifetime. [Nuke trace](letsgo-text-nuke-ohno-001.md),
  [Nuke instructions](../../build/letsgo-text-nuke-ohno-001/static/final-static-2.json),
  [replay dispatch](../../build/letsgo-text-nuke-ohno-001/static/dispatch-inst.json).
- `43FA93 -> 44026F -> 414D80(2)` selects **ONE/ONE additive blending**;
  `~` flags at `47261F` disable alpha testing. The installed green artwork
  uses the traced low RGB intensity.
  `ins4.json`, `more3.json`, `last0.json`, `end2.json`, `extra2.json` retain
  those operations. `43FA93` temporarily adds software bias **0.0012**;
  `last3.json` retains the eight bytes at **4B15B8**.

Provisional clone choices: **Less/no depth write**, clamp texture addressing,
0.0012 separation in clone view-depth units, the existing sim-to-Y-up adapter,
and a **256 whole-mark cap**, evicting oldest marks. Native `441B17` instead
shares a 128-segment pool with other effects (`extra3.json`). Marks lie on the
traced inner cylinder surface exposed by a crater; foreground terrain/scenery
occludes them. Original inherited device depth, precise software-bias conversion,
sector submission order and raster edge coverage remain unproved. No fidelity
gate is closed, and no original execution/comparison was performed.

The registered UCRT64 GCC build rebuilt the clone and legacy `explosion_decal`
test using two jobs to limit memory use after the interrupted attempt. The
focused case **passed in 0.14 s**. It covers splat exclusion, fade after
particles expire, pause, both expiry boundaries, simultaneous lifetimes,
occlusion and depth preservation, a synthetic crater in Just Bomb terrain,
the storage cap and fresh-scene reset. [Build log](../../build/explosion-decal-001/20261118T025649-55108-f3ec0c55/build.log),
[CTest log](../../build/explosion-decal-001/20261118T025649-55108-f3ec0c55/focused-ctest.log).

The synthetic 101-mark draw took **2.768 ms** on the test camera. Whole-frame
cost and visibility during an ordinary Nuke still need a human playtest.
The previous pass's synthetic crater image shows the installed green color
on Just Bomb terrain. [JUnit](../../build/explosion-decal-001/20261118T025649-55108-f3ec0c55/focused-ctest.xml),
[previous synthetic render](../../build/explosion-decal-001/installed-crater.png).

All **19 promoted human-win replays PASS**, exit 0, `won=1`, `completed=1`.
Running build `exe-fnv1a64:212c3668f782e1ac`, GCC 16.1.0.
[Exact files, commands and individual outputs](../../build/explosion-decal-001/20261118T025649-55108-f3ec0c55/replays.json).

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

Mandatory smoke is **pending supervisor execution after worker exit**.

Task files are `src/app/TubeScene.h/.cpp`, `src/app/main.cpp`,
`src/__test__/test_explosion_decal.cpp`, the retained legacy registration in
`CMakeLists.txt`, and this report. Scoped whitespace and LF checks passed.
The tests and replays ran with private AppData under the resumed evidence
directory. The supervisor owns the repository lease and queue transition.
[Previous task diff](../../build/explosion-decal-001/task-only.diff),
[resume entry backups](../../build/explosion-decal-001/20261118T025649-55108-f3ec0c55/entry/).

Handoff — installed ~splat mark, bounded rendering and both traced lifetimes implemented.
Next — supervisor smoke and independent review, then the human Bomber and Nuke check.

**What to check:** **Just Bomb / node 2 / MM_BOMB**: assign Bomber beside
destructible terrain. At the explosion, the installed green surface mark should
remain at the crater, fade continuously, and disappear after 20 level-seconds.
Rotate/use both zooms: terrain and scenery should hide it appropriately and
lemmings should draw in front. Pause freezes its fade, fast-forward advances it,
and Retry clears it. Nuke several released lemmings. Each explosion should
leave a mark that fades over three level-seconds, and the scene should stay
responsive.
