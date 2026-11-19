# rat-victim-fling-001

**Needs review. Supervisor smoke pending.** Playtest 20 K2 reported that a rat
hit in **Escape to Victory** (`E11_KF_1`, node 28) made the
victim disappear into debris. `World::stepRats` hid the captive actor and sent a
generic death-particle event at contact. The rat's punch controller already owned
the victim's removal and the single loss increment. [Playtest ledger](../playtest/2026-11-19-playtest-20.md)

`World::stepRats` and `World::consumeGateCrush` now notify the forced-death
observer. `runApplicationLeg` takes the victim's confirmed feet and displayed
lane, then `TubeScene::emitForcedDeathFlight` creates a transient sprite.
`advanceDeathParticles` ages it on the existing level clock. `drawDeathParticles`
projects it after the ordinary actors and tests the same terrain, pipe, scenery and
actor depths. It has no picker entry. The rat's punch completion still removes
the captive and counts the loss. The gate still counts its victim at the existing
crush transition. Other particle producers use the generic death observer.

## Static reference

Registered `Lemmings Revolution/Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Ghidra 12.1.4 analysis closed without a snapshot. Instruction reads, decompiler
output, vtables and xrefs are archived with the [address cross-check](../../build/rat-victim-fling-001/static/crosscheck.md).

- `4264F2` (rat) and `427266` (closing terrain gate) set actor `+5D`. `42E340`
  consumes it, plays handle `4E32CC` through `44535B`, and installs `416BF0`,
  whose vtable slot `4B0A7C` runs `416C49`. Splat `40B3EC` and explosion `408227`
  construct a different piece effect. The checked xrefs identify one constructor
  caller and one effect entry through the vtable.
- `416C49 -> 416ED0/417150 -> 496BB3` selects one victim-type sprite from
  `4E335C`, with `fall` or `cartwheel` chosen at `496BD5`. The launch delta is
  the difference of two `43D56C` cylinder samples with the same X, plus world-up
  velocity 10 via mode 8 (`417230 -> 493473`). `496D5B/496C84` apply a signed
  random view-phase rate `16*U³` and advance the strip. `4178B0` integrates motion
  and then adds `-1400*dt²` to Y velocity. `4177D0` gives nominal 2–6 s of life.
- `432080` owns the transient. `417320/43CC68` draw a projected sprite with
  positive life and world Y below 120. `432383 -> 4172A0` signals expiry
  separately from `408ECC`'s punch completion. Original device occlusion and
  frame cadence remain unverified.

The clone uses the central launch offsets, the existing cartwheel art, two
view turns/s signed by facing, a continuous gravity approximation referenced
to 60 Hz and a fixed four-second life. These choices are provisional.

## Checks

- [Registered GCC release build](../../build/rat-victim-fling-001/build-webolution.log) passed.
- In a no-input clone run, victim 14 was hit at clock 69,466 ms and counted dead
  484 ms later. The close view shows the launch, inversion and outward fall.
  [250 ms](../../build/rat-victim-fling-001/escape-fling/age-0250.png) ·
  [750 ms](../../build/rat-victim-fling-001/escape-fling/age-0750.png) ·
  [1,350 ms](../../build/rat-victim-fling-001/escape-fling/age-1350.png) ·
  [Close-view run](../../build/rat-victim-fling-001/escape-fling.log)
- In a wider view, the falling sprite's rectangle contains 109 nonblack pixels
  at 3,984 ms and zero at 4,000 ms. The death count holds at three across those
  frames. [3,984 ms](../../build/rat-victim-fling-001/escape-expiry/age-3984.png) ·
  [4,000 ms](../../build/rat-victim-fling-001/escape-expiry/age-4000.png) ·
  [Pixel check](../../build/rat-victim-fling-001/expiry-pixels.txt) ·
  [Expiry run](../../build/rat-victim-fling-001/escape-expiry.log)
- Two of 24 promoted recordings are on the installed rat or closing-gate levels.
  Escape to Victory passed at step 28,060 with 43/40 saved. The Iron Curtain
  passed at step 18,207 with 21/20 saved. Both recorded wins completed.
  [Selection](../../build/rat-victim-fling-001/replay-selection.json) ·
  [Escape replay log](../../build/rat-victim-fling-001/E11_KF_1-1795115359121043200-1.replay.log) ·
  [Iron Curtain replay log](../../build/rat-victim-fling-001/MM_ALL_8_003-1795036800055905400-5.replay.log)

Changed `src/sim/World.cpp`, `src/sim/World.h`, `src/app/main.cpp`,
`src/app/TubeScene.cpp`, `src/app/TubeScene.h` and this report. Runs used a private
save path or the save-free replay route. The scoped whitespace check passed.
[Source diff](../../build/rat-victim-fling-001/task-source.diff)

**Handoff** `needs_review`. The supervisor owns the pending smoke gate.
**Next action** Review the source and frame sequence, then confirm K2 in play.

**What to check** In Escape to Victory, let a lemming reach an evil rat and watch
from the side. The intact figure should leave the cylinder, tumble as it falls,
and disappear. The out count should drop by one when the punch completes.
