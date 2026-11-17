# bash-particles-001: skill debris

Status: **needs_review**; supervisor smoke verification pending. Playable v1,
implementation-plan work-order step 3, [playtest 9 Y7](../playtest/2026-11-16-playtest-9.md).
Difficulty: routine; recommended model Sonnet (existing particle path and retained
cut evidence); performed with the assigned Codex override.

Before: the human reported no bashing particles. The next dependency is visible
feedback at the work face on **Just Bash, MM_BASH_002 (node 6)**. Miner and Digger
also have statically established debris and now use this presentation path.

## Production path

`runApplicationLeg` installs `World::setTerrainCutObserver` before startup ticks.
`World::step` dispatches Basher/Miner/Digger to their existing bite handlers.
`clearSkillBox` wraps exactly the same `Terrain::clearBox` calls: it snapshots
nonempty, nonsteel cells in the cut footprint only when an observer is installed,
then reports cells actually empty after clearing, before the existing movement.
Refusals return before clearing; empty swings report nothing. Clear ordering,
return counts, terrain writes, cues, skill rules, movement and clocks are unchanged.
Without an observer it directly calls the original clearBox path.

The observer passes const actor/cell views to `TubeScene::emitTerrainDebris`.
Four chips per removed cell originate at that cell's wall/floor face. Basher and
Miner throw them back against facing; Digger fans left/right. The colour sampler
is extracted unchanged from the accepted explosion path and reads installed OUT
tile colours (brown fallback). Chips share the existing `DeathParticle` storage,
4096-particle bound, ballistic evaluator and `advanceDeathParticles` clock.
The runtime's existing after-update callback ages them with the level clock:
pause holds them and fast-forward ages them with gameplay. A new scene resets them.
No simulation/scenery RNG, collisions or state writes are introduced.

`renderFrame` completes scene/edited terrain/fluid and actors, then calls the
existing `drawDeathParticles`. Chips retain its strict completed-scene depth
comparison and traced bias, without writing depth. Empty lists still return
before render allocation. No full terrain/actor scan is added to frame drawing.

## Static evidence and limits

Read accepted [death-particles](death-particles-001.md),
[basher fix](basher-fix-001.md), [gameplay sounds](gameplay-sounds-001.md),
[hatch input](hatch-click-rate-001.md), their queue acceptance records, and retained
recovery terrain-blast/actor-explosion evidence first.
Target: repository-local `Lemmings Revolution/Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
New Ghidra 12.1.4 pseudocode and complete instruction packets, including exact
Evidence IDs and analysis-profile identity, are retained in
[static/evidence.json](../../build/bash-particles-001/static/evidence.json).

| Static finding | Instruction cross-check |
| --- | --- |
| Bash helper `40C0AF` constructs debris only for nonzero destructible cells, subpasses 0/3. | Zero at `40C16D`; subpass tests `40C191..40C19B`; constructor `40C1C5`. |
| Mine helper `4188F6` constructs the same debris on successful rows -1/+1. | Zero at `418B29`; row tests `418B2F..418B39`; constructor `418B69`. |
| Dig bite `415A43` constructs the same debris after its three-cell row succeeds. | Zero at `415B84`; constructor `415BE8`. The native path can emit even on an empty row; clone emission requires an actual removal. |
| Shared constructor `40C960` creates four fragments through `40DBF0`. | `40C9FE PUSH 4`, `40CA03 CALL 40DBF0`, matching the accepted death-debris research. |

These are static observations, not original execution or comparison. REA was
closed without snapshot arguments. The reverse-engineer-anything skill was used
for these bounded lookups; no original, oracle, debugger, hook or capture ran.

**Provisional appearance:** four flat texture-coloured chips per removed hybrid
cell, size 0.65/0.85 terrain units, life 1.6..1.9 s, backward speed 3..6 units/s
(Digger symmetric), upward speed 9..13.5, gravity 16 and slight radial spread.
Lifetimes overlap the existing bite intervals. Per-cell density, cell-centre
anchoring, colour/material selection, inner/outer slab choice and deterministic
arcs are clone choices; original fragment geometry/UVs, motion and exact subpass
density are unresolved. No native timing was transplanted. Later authorized
original observation of a cut would settle those choices. No new image or human
playtest establishes the visible appearance here; the next human playtest is
acceptance. Existing depth-policy fidelity limits remain unchanged.

## Checks and preservation

Registered UCRT64 GCC 16.1.0/CMake/Ninja build of `webolution`,
`test_terrain_debris`, `test_death_particles` passed, eight compile jobs:
[build.log](../../build/bash-particles-001/build.log).
Focused legacy checks **2/2 PASS, 0.07 s wall**:
[focused.log](../../build/bash-particles-001/focused.log),
[JUnit](../../build/bash-particles-001/focused.xml).
The new seconds-long legacy unit guards actual post-clear notifications,
seam-wrapped coordinates in both directions, empty/steel silence, inverted Dig,
four chips per removed cell, overlapping life and expiry. No smoke test,
assertion, timeout or replay was changed; no test was removed.

Ran the built clone as `webolution --game-dir "Lemmings Revolution" --replay <file>`
on **all 15 promoted recordings**. Each returned exit 0, **PASS, won=1,
completed=1**. Exact filenames, commands and individual output logs:
[replays.json](../../build/bash-particles-001/replays.json).

| Level | Saved/required |
| --- | ---: |
| M06_KF_1 | 93/50 |
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
| MM_BASH_DIG_001_A | 20/10 |
| MM_BUILD_BLOCK_A | 15/10 |
| MM_FLOAT_CLIMB2 | 10/6 |

These replays exercise simulation/file compatibility, not rendered particles;
their headless World has no presentation observer. The new unit exercises the
observer. Retired playtest-9 files remain untouched and excluded per the replay
README. No new gameplay route, replay fixture or frozen build was added.
No smoke/full profile was duplicated; the supervisor owns the mandatory gate.

Changed: `src/sim/{World.cpp,World.h}`, `src/app/{TubeScene.cpp,TubeScene.h,main.cpp}`,
new `src/__test__/test_terrain_debris.cpp`, four CMake registration lines, this
report. Incoming edits are preserved in [entry/](../../build/bash-particles-001/entry/);
[task-only.patch](../../build/bash-particles-001/task-only.patch) separates these
changes to existing files. Task text is LF; scoped diff checks pass.
Parent supervisor PID 46016 owns RepositoryLease. CIM process enumeration was
denied; Get-Process found no clone/build/test runner before edits. No protected
control, queue, permission, shared-instruction, commit or push changes. No
subagents, concurrent writers or background jobs. Commands used private
LocalAppData `build/bash-particles-001/appdata`; replay and logic checks do not
construct a SaveStore. Installed and normal user saves were not opened or changed.

Handoff: Basher/Miner/Digger emit terrain-coloured cut debris; 2 focused legacy checks and all 15 promoted replays pass; needs_review.
Next: supervisor smoke, independent review, then human playtest; original appearance/density/motion remain provisional.

What to check: **Just Bash (MM_BASH_002)**, bash through each pillar: chips leave
the wall face on successive successful bites, fall away and disappear; air/steel
creates none. Rotate the cylinder: foreground terrain covers chips. Pause freezes
them; resume/fast-forward advances them. **Just Mine (MM_MINE)** and **Just Dig
(MM_DIG_000)**: cutting the slope/floor also throws terrain-coloured debris.
