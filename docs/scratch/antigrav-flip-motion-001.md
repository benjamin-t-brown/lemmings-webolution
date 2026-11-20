# Anti-grav lift motion

**Needs review.** Playtest 23 N1 observed a gravity snap in **Designed with love**
(`M11_KF_1`, node 33). `World::stepWalker` changed the gravity sign and entered
`Faller`. Actor preparation immediately mirrored the sprite. The recorded route
is `build/playtest/replays/M11_KF_1-1795198796378331400-1.replay`.
[Playtest 23](../playtest/2026-11-20-playtest-23.md)

## Retail static trace

On the registered x86 EXE with SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`,
`0x41D590` submits the sound. Walker contact at `0x41BFFC` attaches the
cartwheel effect, type `0x2B` and vtable `0x4B0CA8`, followed by the primary
fall type `0x13`. Cartwheel entry `0x41ADDC` reverses gravity via `0x42F5E3`
and selects `cartwheel`. Fall update `0x4160BD` and speed helper `0x416038`
move along the new gravity at 20 cells/s, or 10 with an open umbrella.
Support probe `0x416433` enters Walker via `0x42F8B9` without changing facing.

Cartwheel update `0x41AE95` advances the sprite controller and counts down
from 1.5 clip lengths, or 0.5 for an already assigned Floater. The installed
ordinary strip lasts 0.48 nominal seconds, yielding timer seeds of 0.72 s
and 0.24 s. The effect ends on Walker or timer expiry, selecting `walk` or
`fall`. Fall slices also advance the sprite controller. `0x42E58C..0x42E5D4`
defers the gravity-based UV flip until the cartwheel effect clears. Assignment
at `0x45A0E0` permits Climber, Floater and Bomber subject to their usual
flags. Ground skills reject primary fall type `0x13` via `0x4183D3`.
[Instruction cross-check](../../build/antigrav-flip-motion-001/static-crosscheck.md) ·
[REA records](../../build/antigrav-flip-motion-001/static-evidence.json)

## Clone change and evidence

`flipIfAntiGrav` enters `AntiGrav` and emits one `AntiGrav` sound event.
`World::stepElapsed` and `World::step` dispatch to `stepAntiGrav`, which
shares fall speed and collision probes while carrying fractional travel.
Landing on support bypasses the distance-based splat rule during the lift.
An open roof still produces a counted loss. `LemmingMotionTracker` and
`injectedWalkersFromWorld` supply the live feet position and turn phase to
`prepareInjectedWalker`. Compositing, hover boxes and click selection use the
same placement. Hover checks the actual transit skill eligibility.

**Provisional presentation.** The clone fits a continuous 180-degree phase
to the distance to live support, samples the nine authored poses in the first
half of `cartwheel`, and moves the sprite centre around the projected feet.
The far-side rendering uses the normal Walker orientation. Retail uses a
separate clip timer. The native base-model sign at `0x41AE15` and its exact
effect on the displayed cartwheel direction remain unverified.
[Installed art sample](../../build/antigrav-flip-motion-001/cartwheel-sheet.png)

In the supplied recording, actor 0 enters at step 8150 and 37,974 ms at
`(46,65)`. It travels through nine cartwheel poses, with fractional feet
positions such as `62.70` and `56.74`, and lands alive at `(46,53)` at step
8356 and 38,574 ms. It then walks right on the ceiling. The framebuffer
trace shows visible body pixels in each sampled pose, a Bomber pick on the
same actor, and one sound request for the crossing. The images use the
production terrain, scenery and actor renderers with a fixed camera selected
for the lift. The montage enlarges identical crops from the full frames.
[Montage](../../build/antigrav-flip-motion-001/designed-with-love/crossing-montage.png) ·
[Full frames and trace](../../build/antigrav-flip-motion-001/designed-with-love/) ·
[Observer log](../../build/antigrav-flip-motion-001/observe-recording-final.log)

## Checks

- Registered native build passed. The existing World assertion now requires
  the lift state where it previously required the old `Faller` transition.
  World passed 411 checks. A build-only logic probe also passed both directions
  through a 50-cell shaft, fractional movement, zero dt, Floater assignment and
  slowdown, ground-skill refusal, one-shot audio, and open-roof loss.
  [Build log](../../build/antigrav-flip-motion-001/clone-build-final.log) ·
  [World log](../../build/antigrav-flip-motion-001/world-unit.log) ·
  [Lift probe](../../build/antigrav-flip-motion-001/lift-audit.log)
- All 28 promoted recordings reproduced. The two lift-level wins pass at
  step 1544 for `M06_KF_1` (91/50) and step 33240 for `M08_KF_1` (42/40).
  `M11_KF_1` reproduces the recorded retry at step 13793 (0/85).
  [Every replay result](../../build/antigrav-flip-motion-001/replays.tsv)
- The supervisor smoke gate is pending. The framebuffer evidence is from
  the clone. The retail EXE was used for static analysis only.

Handoff — antigrav-flip-motion-001 is needs_review with the recorded crossing and replay evidence retained.
Next action — supervisor runs the smoke profile and reviews the production path.

What to check — On **Designed with love**, send a lemming onto the lift reached
by the Playtest 23 bridge. Expect a visible half-turn on the way up, one sound,
and a surviving lemming that walks in the same direction on the ceiling.
