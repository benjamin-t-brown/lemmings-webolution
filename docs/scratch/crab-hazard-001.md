# crab-hazard-001 — the crab in the water

Status: **needs_review**; supervisor smoke verification pending. Playable v1,
playtest 8 X3, [Seafood Sarnie, node 15](../playtest/2026-11-16-playtest-8.md).
Before: the human reported a missing creature and contact death. The parsed
`Crab.a3d` placement had neither a scene nor a simulation consumer. Connecting
that demonstrated defect is this task's production dependency. Difficulty:
difficult; recommended Opus, executed with the assigned Codex override.

## Evidence and behavior

Reference: repository-local `Lemmings Revolution/Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Ghidra 12.1.4, static only; closed without snapshot arguments. Full returned
Evidence records and limitations: `build/crab-hazard-001/static/`. Retained
[serialized-reader research](../../decomp-lemmings/docs/evidence/level_entity_tail.md)
establishes the STAT/DYNA/FALL dispatch and named-record layout. No original
game, copied instructions, debugger, hooks, capture or installation changes ran.

- **Serialized:** `build/crab-hazard-001/census.txt` finds **14 crabs on all 12
  specified levels**, nodes 15, 19, 21, 36, 39, 44, 46, 49, 63, 67, 84, 96.
  Every crab is **STAT**, with no path, speed or turning record. Seafood Sarnie's
  cylinder position is **(116, 9, 0.3)**. Models retain their stored world matrices.
  Both transform clips parse on every model: `rest`, duration **1 s**, and
  `attack`, **1.1666666269302368 s**, each animating 16 model parts. No installed
  placement, motion or animation remains unresolved.
- **Static:** reader `0x4333D2` binds names at +0x148/+0xC8; rest starts looping.
  `0x432E3F` advances the model, restores rest after attack completion, gathers
  contacts, removes victims and starts a nonlooping attack. Installed +0x1D1=1
  collects **all** contacts; +0x1D4 prevents another kill during attack.
  Pseudocode and instructions: `static/432e3f{,_asm}.json`, evidence
  `ev_24b6bdde7f841bca9e469227e04f76c03b316aa2e04be7c5d353573567cf50b2`.
- **Static, instruction checked:** `0x430F4D -> 0x430C9C` tests the actor's point
  against closed relative boxes, adding +0x48/+0x50/+0x58 to both endpoints.
  Both facing branches use identical bounds. Every installed relative box is
  **[0,0,0]..[14,7,1]**. Thus Seafood Sarnie's kill volume is **[110,124] x
  [9,16] x [0,1]**, not its broad-box maximum at column 122. Evidence:
  `static/430c9c{,_asm}.json`,
  `ev_a5ba6db6732e83958f36dc0e8064ce9b1489bd3bc797f99745100a2f50cd80c9`.
- **Static, instruction checked:** installed death selector +0xC0=5 follows
  `0x4331D0 -> 0x40A6A0`, vtable `0x4B0628`, enter `0x40A6EC`. This creates
  six body pieces and ten pink particles; the intact victim leaves the active
  list immediately. `static/4331d0{,_asm}.json` and `40a6ec{,_asm}.json` retain
  the calls/counts. The clone counts one death, clears any Blocker field and
  reuses the accepted death-particle observer. It does not attach Drown or
  invent a crab sound.

## Production path and limits

`loadRetailLevelReport` reads named crab records into `LevelSetup::crabs`,
including relative volumes and model clip durations. Unsupported behavior or
missing structured records reports a load error. `RuntimeSimState::advanceElapsed
-> World::advanceElapsedMs -> updateSegment -> stepCrabs` advances animation and
checks contact after the existing receivers. The diagnostic `World::tick` also
steps crabs. Victims become Dead once; the existing HUD/results use `dead_`.
The existing observer reaches `TubeScene::emitDeathParticles/drawDeathParticles`.

`BackdropScene::build` admits the stored crab model through the ordinary scene
mesh/depth path. `main.cpp::prepareScene -> setCrabStates -> applyHatchAnimation`
samples rest/attack from **World's** state. Drawing never advances the hazard.
There is no wall-clock sampling or RNG in creature updates; pause freezes admitted
time, fast-forward uses the existing passes, and reload reconstructs rest state.

**Provisional:** contact uses clone integer feet and radial coordinate 0; every
installed volume includes that lane. Cross-class overlap priority follows the
clone's existing trigger/receiver ordering, with crab contact afterward. Transform
sampling reuses the accepted normalized-quaternion interpolation adapter. Particle
motion, origin, lifetime and sprite sampling reuse the provisional splat effect;
the six/ten counts and pink colour agree with static death entry. Existing fluid
and actor depth policies remain provisional. No original timing/fidelity pass or
Seafood Sarnie human win is claimed; other creature kinds and pipes remain outside
this task.

## Checks and preservation

Registered UCRT64 build: `build/crab-hazard-001/final-build.log`.
New **legacy** `crab` test: **PASS, 0.28 s**,
`build/crab-hazard-001/final-unit-ctest.log`. It checks closed corners and the
far two columns, outside/removed actors, species/gravity independence, simultaneous
kills, attack recovery, deterministic update sequences, reset, and installed
model/animation bindings. Installed census: `installed-crabs.log` in the same
directory. These are logic/data checks, not a scripted gameplay route.

Inspected clone image: `build/crab-hazard-001/Seafood-Sarnie-crab.png`, camera
`crab-camera.json`, exact run `crab-render.log`. The crab's eyes/body/claws are
visible in the blue pool, and the foreground scenery occludes it. `ready-render.log`
is the initial camera run, where the crab is around the cylinder. Attack movement
is checked through model matrices; no natural contact or attack image is claimed.

All **15 promoted human-win recordings** ran with
`webolution --game-dir "Lemmings Revolution" --replay <file>`, unchanged.
Every result was **PASS, won=1, completed=1, exit 0**; none of these levels has a
crab. Exact final-binary filenames/results:
`build/crab-hazard-001/final-replay-results.tsv`, with one `final-*.log` per
recording in that directory. The earlier `replay-results.tsv` run is retained.

| Level | Saved / required | Result |
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
`src/__test__/test_crab.cpp`, `CMakeLists.txt`, and this report. Pre-edit copies
and the task-only diff are under `build/crab-hazard-001/`. Pre-existing pressure-pad,
queue/status and original-log edits were preserved. The supervising PID 26408
already owns RepositoryLease; no protected controls were edited. CIM metadata
was denied, but Get-Process confirmed no live clone/build before edits. Source
text is LF and scoped `git diff --check` passes. No replay/assertion/timeout was
changed, and no replay fixture, frozen build or per-task gameplay route was added.
App runs used private `build/crab-hazard-001/appdata`; replay/unit runs do not
open saves. Normal user and installed saves were untouched. Mandatory smoke is
**pending supervisor execution after worker exit**; no full suite was run here.

Handoff: crab rendering, rest/attack animation and deterministic contact deaths are implemented; all 15 human-win replays pass.
Next: supervisor smoke and independent review, then the user's Seafood Sarnie playtest for acceptance.

**What to check:** Seafood Sarnie (node 15): rotate to the pool at cylinder column
116 and watch the crab move its parts in place. Drain the pool with the existing
switch, then walk a lemming into the idle crab at original Y 9..16, columns
110..124: it disappears into body pieces/pink particles, the dead count rises
once and the crab attacks, then resumes rest after about 1.17 simulation seconds.
Lemmings outside contact survive; a new contact during attack waits until it
rearms. Pause freezes animation, fast-forward advances it, and retry restores rest.
