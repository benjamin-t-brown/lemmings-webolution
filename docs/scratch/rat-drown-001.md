# rat-drown-001

**Needs review. Supervisor smoke pending.**

[Playtest 19 J8](../playtest/2026-11-19-playtest-19.md) reports rats lingering
in the water in **Escape to Victory**, node 28 / `E11_KF_1`. The later human
attempt `E11_KF_1-1795105239624473600-1.replay` reproduced a drowning
interruption on the installed level. At 60,391 ms rat index 1 entered water
receiver 5 at X=170, row=19, below its 19.5 surface. The level had zero static
0x40 cells. The rat reached 1,000 ms on its drown timer, then claimed lemming
index 8 at replay step 20,083 and drew `punch1`. That branch skipped the
drown step. At step 20,200 the punch killed the lemming and restored Walker.
At 61,832 ms the same water contact started Drown again at zero. This
recording showed one reset followed by eventual removal.
[Entry clone trace](../../build/rat-drown-001/clone-entry-E11_KF_1-1795105239624473600-1.log)

`World::tick/updateSegment -> stepRats -> step/stepElapsed -> stepDrown`
now excludes a drowning rat from starting a punch. The existing fluid
contact, hybrid 5 Hz death clock and EvilRatHi `drown` clip carry it to
`Dead`. That state removes it from active contacts and
`injectedWalkersFromWorld` while keeping the placed rat's stable index.
The existing sheet-3 accounting guard keeps rat losses out of rescue totals.

## Static evidence

Ghidra 12.1.4 inspected the registered x86
`Lemmings Revolution/Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
The rat update at `426414` calls the common update at `4265D1 -> 42E2BF`.
The shared walker writes a matching receiver at `41CA0A/41CA7E`. The fluid
test at `41C321..41C36F` grants immunity only to matching sheets 1 and 2.
Sheet 3 takes the common 0x23 fluid-death path. Entry `404261` starts
`drown`. Lava can play Die at `4042E5`, and the sinking step can play
Bubble at `4046DC`. Near the receiver bottom `40492C` returns 2,
which the shared updater turns into AL=0. The world removes the actor
at `442C6E..442CAE` from the same list that received the placed rats
at `44498C..4449A3`.

The separate terrain 0x40 check at `415386` can construct the 0x22 Drown
state at `4154D8` with vtable `4B0570`. That state plays `drown`, has
a two-second threshold and has no direct sound in its attach and step.
The traced hatch constructor installs this monitor. The placed-rat
constructor and reader install the common actor and fall state. The native
rat support test at `42E913 -> 42E8C7` rejects pure water and acid zone
support for sheet 3. The clone treats live fluid surfaces as support,
so the explicit Drown attack exclusion is a provisional clone rule.
The clone uses its existing mean-surface contact, drown strip placement,
two-second timer and entry cues. The original fall into this pool and its
exact lifetime remain unmeasured. The next clone playtest checks the
on-screen result. REA closed without a snapshot.
[Instructions and pseudocode cross-check](../../build/rat-drown-001/static/crosscheck.md)

## Checks

- Registered GCC 16.1.0 built `webolution` and compiled the strengthened
  legacy `evil_rats` test. Legacy-suite execution is for milestone sign-off.
  [Native build](../../build/rat-drown-001/native-build.log) ·
  [Final test build](../../build/rat-drown-001/native-build-final.log)
- A source-built probe followed the same recorded attempt until the
  count divergence, then separately advanced its first-contact World
  state without inputs or hatch releases. Rats 1 and 0 reached `Dead`
  at 62,204 and 65,403 ms, respectively. Each showed the advancing
  installed 12-entry `drown` clip and disappeared from the draw list
  at 2,000 timer ms. The rescue totals read the same across each rat removal.
  [Final clone trace](../../build/rat-drown-001/clone-final-E11_KF_1-1795105239624473600-1.log)
- Isolated probe checks passed for terrain 0x40, live water, lava and
  acid. Each rat played its clip and disappeared with zero rescue
  losses. Ordinary lemming drown counted one death in each case, and a
  dry rat still killed its target and resumed walking. The runs used a
  private LocalAppData directory and the headless paths had no save store.
  [Fluid and dry-contact log](../../build/rat-drown-001/fluid-checks.log)
- Of 23 promoted wins, zero use the 17 installed rat levels. Five
  unpromoted Escape attempts ran with the final `webolution --replay`.
  Attempts 8–11 passed at steps 4,961, 2,109, 985 and 810, respectively.
  `E11_KF_1-1795105239624473600-1.replay` diverged at step 20,200,
  61,828 ms. The old result expected 5 deaths and the fix produced 4
  because the drowning rat did not finish a punch. This recording is
  unpromoted. [Replay selection](../../build/rat-drown-001/promoted-selection.json),
  [commands' results and log paths](../../build/rat-drown-001/final-replays.json)

Handoff — `src/sim/World.cpp` and `src/__test__/test_evil_rats.cpp` are ready for independent review.  
Next — Supervisor smoke, then the user's playtest.

**What to check** — Escape to Victory (node 28). Assign a Miner on the lower
walkway around X=167 to open a drop into the pool near X=170. Watch a rat
fall in with a lemming nearby. Expect an uninterrupted drown animation
followed by disappearance. OUT, SAVED and death totals should count only
lemmings.
