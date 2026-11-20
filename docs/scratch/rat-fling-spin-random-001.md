# rat-fling-spin-random-001

Playtest 21 L2 reported uniform spin when rats punch lemmings off the cylinder
in **Escape to Victory** (`E11_KF_1`, node 28). The clone used two view turns
per second for every victim and took the direction from the victim's facing.
[Playtest ledger](../playtest/2026-11-19-playtest-21.md) ·
[Prior flight work](rat-victim-fling-001.md)

## Static reference

Ghidra 12.1.4 read the registered x86 `Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
`496D5B` draws `U = rand() / 32767` through `4040A0 -> 49C84A`,
giving discrete values from 0 through 1. It stores `16 * U³` and draws
again to pick positive phase below 0.5 and negative phase otherwise.
`496C84` multiplies the signed rate by elapsed seconds, wraps by one
view turn, sends the phase through `4683E9` to the camera-dependent
sprite-view chooser at `46840B`, then advances the animation strip
with the same elapsed time. Instructions and constant bytes corroborate
the pseudocode. REA closed without snapshot arguments.
[Address cross-check and raw evidence](../../build/rat-fling-spin-random-001/static/crosscheck.md)

## Production

`World::stepRats` and `World::consumeGateCrush` emit the existing forced-death
notification. `runApplicationLeg` forwards it to
`TubeScene::emitForcedDeathFlight`, which stores one signed rate per flying
figure in the scene. `drawDeathParticles -> prepareInjectedWalker` now feeds
the wrapped rate times flight age to the existing sprite-view selector.
The stream uses the existing CRT algorithm with the provisional choice
of seed 1 per scene. The clone computes phase from its level clock and
uses its cartwheel strip. Native frame cadence and final appearance still
need comparison. The production diff is confined to spin initialization
and view-phase calculation. [Source diff](../../build/rat-fling-spin-random-001/task-source.diff) ·
[Prior accepted change](../../build/rat-fling-spin-random-001/accepted-rat-fling.git-show)

## Checks

- The Windows GCC release target `webolution` built successfully.
  [Build log](../../build/rat-fling-spin-random-001/native-build.log)
- The installed Escape to Victory clone trace observed 25 rat victims and
  25 distinct rates, with 12 positive and 13 negative. Examples include
  `-0.116`, `+3.203` and `-8.914` turns/s for three right-facing victims.
  The trace records launch and velocity. At 250 ms the first flight drew
  without writing scene depth. Nearer terrain and nearer actor depth each
  hid it. The first punch counted one loss at 484 ms, and its flight expired
  at 4,000 ms. [Per-figure trace](../../build/rat-fling-spin-random-001/escape-spin-trace.log) ·
  [Probe](../../build/rat-fling-spin-random-001/trace.cpp)
- All 25 promoted recordings reproduced with `webolution --game-dir
  "Lemmings Revolution" --replay <file>` and private LocalAppData.
  Escape to Victory passed at step 28,060 with 43/40 saved. The Iron Curtain,
  which uses the shared closing-gate flight, passed at step 18,207 with
  21/20 saved. [Per-recording results](../../build/rat-fling-spin-random-001/replay-results.tsv) ·
  [Escape to Victory](../../build/rat-fling-spin-random-001/replays/E11_KF_1-1795115359121043200-1.replay.log) ·
  [The Iron Curtain](../../build/rat-fling-spin-random-001/replays/MM_ALL_8_003-1795036800055905400-5.replay.log)

- **Handoff** `needs_review`. Supervisor smoke is pending.
- **Next action** Review the source and trace, then confirm L2 in play.

**What to check** In Escape to Victory, let several lemmings reach either rat.
Watch from the side. Expect the flying figures to turn at different speeds
and in both directions as they fall, with one casualty for each completed punch.
