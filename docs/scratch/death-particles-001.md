# death-particles-001: explosion debris and splat pieces

Status: **needs_review**. Implementation complete; the supervisor's mandatory
**smoke profile is pending**. This report does not claim smoke acceptance or
human playtest acceptance. Playable v1, implementation-plan work-order step 3,
human [playtest 6 V1/V2](../playtest/2026-11-15-playtest-6.md). Difficulty:
difficult; recommended model Opus for the cross-system presentation/evidence
work; executed with the assigned Codex override.

## Failure, scope and production path

Before: the human reported no exploding-lemming/block particles and no pieces
after fatal falls. Terrain clearing and loss accounting already worked. The
Splatter sprite adapter displayed an intact fallback pose. These missing visual
consequences are the next playable-v1 dependency. No simulation retuning,
original execution, fidelity gate change or broader timing work is included.

Production path:

1. `runApplicationLeg` installs `World::setDeathObserver` before diagnostic
   startup ticks or ordinary play. `World::explode` snapshots occupied cells in
   the existing 7x7 blast window, calls the unchanged `Terrain::clearExplosion`,
   performs the existing death/cue operations, then reports only cells actually
   changed to Empty. Steel, blocker-protected cells and mask-excluded cells are
   therefore excluded without duplicating the stencil or changing its rules.
2. Both fatal-fall branches of `World::stepFaller` notify after their existing
   Splatter/loss/cue transition. `TubeScene::emitDeathParticles` receives const
   lemming/cell views and samples installed artwork/terrain colours. It does
   not write to World, consume game RNG, add collision or change timers.
3. `RuntimeSimState::advanceElapsed`'s existing after-update callback advances
   effect presentation with `World::levelClockMs()`, including fast-forward
   passes. Pause freezes that clock. A fresh level owns a fresh scene/effect
   list. Expiry is independent of the existing Splatter-to-Dead transition.
4. `runApplicationLeg::renderFrame` completes `scheduleSceneFrame` (scenery,
   edited tube geometry, fluid and secondary scenery), draws ordinary actors,
   then calls `TubeScene::drawDeathParticles`. Sprite pieces use the existing
   cylinder pose, actor projection, alpha test and scene-depth clipping. Chips
   use the same physical view-Z convention and strict less-than scene test.
   Both retain the traced normalized 0.0003 bias; airborne pieces omit the
   provisional actor supporting-wall contact allowance. Neither writes depth.
5. `injectedWalkersFromWorld` disables the intact Splatter's render flag,
   matching static `40B4AF`. Its vector slot remains present, preserving all
   subsequent screen-box/hover/assignment indices. Simulation alive/active
   predicates and the existing Splatter frame lifetime are unchanged.

There is no per-frame terrain or actor scan for particle detection. With no
particles, advance/draw each return at the empty-vector check: no allocations,
texture reads, depth-buffer copies or pixel loops. Memory is bounded at 4096
entries by evicting oldest presentation particles before a new complete burst.
This is an implementation bound, not a measured frame-time benchmark.

## Evidence and provisional contracts

Registered reference SHA-256:
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
All analysis used the repository-local `Lemmings Revolution/Lemmings Revolution.exe`.
Ghidra 12.1.4 static analysis only; `close_binary({})` completed without snapshot
arguments. No original game, original-byte oracle, debugger, hook or capture ran.

Retained accepted recovery, reread first:

- [Actor explosion](../../decomp-lemmings/docs/evidence/actor_explosion.md),
  `408227`, and its assembly: six actor children, pair (4,0), mode 6.
- [Terrain blast](../../decomp-lemmings/docs/evidence/terrain_blast.md),
  `43F510`, and its assembly: four fragments at eligible aligned debris sites;
  final count 100, RGB (255,32,32), pair (2,0.5), mode 7. These recovered
  callbacks explicitly leave renderer/physics semantics unresolved.
- [FallSplat investigation](../../old/docs/scratch/phase3-round28-die.md)
  distinguishes the type-1A fall death from fluid death and identifies `40B3EC`.
- [Actor depth policy](actor-depth-001.md) documents the clone depth units,
  alpha test and bias; this task preserves that policy's fidelity limits.

New targeted static results are retained in
[rea.json](../../build/death-particles-001/static/rea.json) and
[pieces.json](../../build/death-particles-001/static/pieces.json):

- `40B3EC`, evidence `ev_84ba5be12ae98de2bd29a0dc4d3909f98950b0bdc5f50c749fdd14a8ba1a7a27`:
  hide intact actor, six pieces with pair (4,0.5)/mode 5, ten particles and
  RGB doubles (255,127,127). Instruction packet
  `ev_ccb536aff5cfebef115106c328dce0c8869ab539889579fec92d445477e553b4`
  cross-checks `40B4AF`, push 6/call at `40B534..539`, pair/mode pushes,
  push 10/call at `40B8A7..8AC`, and the three colour double-word pairs.
- `401C50`, evidence `ev_77ada9a5e52d2fa828f563ad6f49d5ac5b453098f1c6d164a566f5b8165b9f69`:
  six children share resource global `4E3308`; piece indices 0,1,2,2,3,3.
  Its instruction packet confirms the six `+278` stores at `401CA7..401D0B`.
  Binding that global to BITS.SPR remains an inference, labelled provisional.
- `40C960`, evidence `ev_4d5165e8f5824b9c47d901a5167e0eab33767f7648e0d36a0d73e3acddaffc99`:
  debris construction calls `40DBF0(...,4)`, confirmed by instruction packet
  `ev_94c1cee39c29b853cbd395cedd68ea6179f26489b69740fec56e6f4955f21fd9`.
  `4018E0` constructor and instructions are retained too; its native acceleration
  fields are not transplanted into the clone's terrain-coordinate physics.

Installed-asset inspection used the existing `lrtool extract`, source-defined
SPR serialization, and a bounded offline parser; outputs are under
[static/](../../build/death-particles-001/static/). BITS.SPR contains head,
body, arm, leg (16/8/8/8 temporal entries), then four rat clips. Stored rates
are 1.0 and these clips reference `Sheets\$LemBits0`; colours come from the
installed keyed bitmap. No original assets are added to version control.

| Effect | Source and count | Clone motion and lifetime |
| --- | --- | --- |
| Bomber actor | Installed BITS.SPR head/body/two arms/two legs; six is static evidence. Resource binding, directional/temporal sampling and anchoring are provisional. | Provisional: horizontal fan speed 15 terrain units/s; upward speed 17 with +/-35% fan variation; gravity 32 units/s squared; radial velocity sin(3*angle)*0.10; lifetime 1.6 s. |
| Explosion small burst | Static count 100 and RGB (255,32,32); provisional plain coloured chips. | Provisional: horizontal fan 8..18, upward 12 with +/-35% variation, same gravity/radial rule; size 0.45 units, lifetime 0.55..0.97 s. |
| Destroyed block | Four chips per actually removed 4x4-aligned grid site, following retained counts/site eligibility. Sample the rendered cell's own OUT-bank material/texture at cell centre, including reversed tile-row addressing. Model, colour sampling and a four-chip fallback when removal misses every aligned site are provisional. | Provisional: horizontal speeds 9..12, upward 12 with +/-35% variation; size 1.4 units, life 1.2/1.3/1.4/1.5 s; same gravity/radial rule. |
| Fatal-fall pieces | Static count six and indices 0,1,2,2,3,3; same installed BITS artwork and provisional binding/sampling as bomber. | Provisional: horizontal fan 12, upward 10 with +/-35% variation, same gravity/radial rule, life 1.6 s. |
| Fatal-fall droplets | Static count ten and RGB (255,127,127); provisional coloured chips. | Provisional: horizontal fan 8..17, upward 5 with +/-35% variation, size 0.45 units, life 0.55..0.97 s; same gravity/radial rule. |

All motion is analytic `origin + velocity*t + gravity*t*t/2` in the clone's
terrain coordinates. Gravity and rise reverse for inverted lemmings. Chips
are small opaque squares, projected sizes clamped to 0.6..5 pixel half-widths.
No bounce, fragment collision, original randomized emission, exact child
models, blend state or original lifetime is claimed. Missing tile samples fall
back to provisional earth brown; missing piece assets fall back to plain chips.
Original observation of bomber and fatal-fall sequences at a fixed camera,
including special species/inverted deaths, plus tracing the effect update/draw
callbacks and resource registration would settle these remaining choices.

## Clone observations and checks

Before evidence is the human ledger above. No pre-task frozen build, replay
fixture, new scripted route or identity test was created. Existing legacy
drivers were reused unchanged to answer concrete rendering/assignment questions.
Their `legacy`/`plain` labels mean `--no-actor-anim`, not a pre-task binary.

- Final registered-toolchain build passed:
  [final-build.log](../../build/death-particles-001/final-build.log).
- New fast legacy unit passed, 0.03 seconds:
  [ctest-unit.log](../../build/death-particles-001/ctest-unit.log). It checks
  exactly-once blast/splat notifications, actual removed-cell reporting,
  preserved steel/loss totals and analytic upright/inverted gravity. It does
  not establish natural gameplay or original fidelity. Smoke membership stays
  at 95; no existing assertions/timeouts or tests were changed or removed.
- Existing Bomber driver passed on **MM_BOMB**, ordinary panel assignment:
  [bomber.log](../../build/death-particles-001/bomber.log). Skill stock 20->19,
  click frame 17, Exploder frame 42, blast/removal frame 48; all existing
  countdown/animation/outcome assertions passed. Inspected
  [blast frame](../../build/death-particles-001/bomber/bomb-blast-frame.png)
  shows separated head/body pieces above the new crater. The existing driver's
  other image names refer to animation diagnostics, not this task's baseline.
- Existing connected Nuke driver passed on **MM_CLIMB_002**:
  [nuke-final.log](../../build/death-particles-001/nuke-final.log),
  [summary](../../build/death-particles-001/nuke-final/summary.txt),
  [trace](../../build/death-particles-001/nuke-final/trace.csv).
  Five released lemmings exploded, 0 saved, results reached, 71 changed terrain
  cells, no failures. Inspected [first crater](../../build/death-particles-001/nuke-final/crater-1.png)
  and [third crater](../../build/death-particles-001/nuke-final/crater-3.png)
  show installed lemming pieces, red burst and terrain-coloured chips. Earlier
  `nuke-live/` was before correcting chip texture-row sampling; use `nuke-final/`.
- Existing CLI natural hatch/fall path on **MM_FLOAT**, no skills or injected
  actor/terrain state: tick 103 has the first lemming falling at (139,86),
  tick 104 has it splatted at (139,88), lost count 1; tick 106 has it Dead,
  with remaining pieces still visible. Inspected
  [fall before contact](../../build/death-particles-001/float-103.png),
  [pieces at 104](../../build/death-particles-001/float-104.png), and
  [pieces at 106](../../build/death-particles-001/float-106.png).
  The first image uses the default camera; the latter two use
  [float-camera.json](../../build/death-particles-001/float-camera.json), yaw
  139. The same splatted state is not visible from the far side in the
  [default-camera view at 104](../../build/death-particles-001/float-104-hidden.png).
  Camera rotation also changes the native submission gate, so this pair does
  not isolate the per-pixel depth test. The strict completed-scene comparison
  is established by the production call path above.
  These are clone observations, not an original comparison or a camera-matched
  pre/post implementation test. Exact state outputs are in the corresponding
  `float-*.log` files. In the diagnostic tick path, clock advancement at tick
  end makes a just-emitted effect 0.2 s old in the saved image; ordinary explicit
  updates retain their existing clock-before-step order.

Every development execution used a private task LocalAppData root under
`build/death-particles-001/appdata`; the existing Nuke driver additionally owns
its private save sink and preservation checks. No normal or installed save was
imported/reset/written. No supervisor, queue, verifier, tool registry, permission,
shared instruction, completion criterion or historical gate was edited.
Pre-existing edits remain preserved. Entry source backups and the incremental
task diff are under [before-source/](../../build/death-particles-001/before-source/)
and [task.diff](../../build/death-particles-001/task.diff); the CMake preservation
copy is explicitly reconstructed by subtracting only this task's registration.

Changed files: `src/sim/World.{h,cpp}`, `src/app/TubeScene.{h,cpp}`,
`src/app/main.cpp`, new `src/app/DeathParticles.h`, new
`src/__test__/test_death_particles.cpp`, four CMake registration lines, and this
report. No tests were added to the smoke tier. Human confirmation, the mandatory
supervisor smoke result, broad occlusion coverage and original effect fidelity
remain unknown/pending.

Handoff: implementation is ready for supervisor `verify.py --profile smoke` and independent diff/call-path review.
Handoff: preserve all timing/fidelity parks; acceptance of V1/V2 is the user's next playtest.

What to check: **MM_BOMB** (or playtest's **MM_BLOCK_BOMB_A**), assign Bomber on a
solid ledge: the lemming bursts into pieces and the removed ledge throws chips,
then both disappear without changing the crater/loss. **MM_FLOAT**, let the first
lemming fall to the bottom without Floater: head/body/limb pieces and small pink
droplets replace the intact figure; foreground walls hide the effect. Pause
holds particles and fast-forward ages them with gameplay.
