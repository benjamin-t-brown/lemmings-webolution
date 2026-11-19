# rat-attack-sound-001

Playtest 20 K1 questioned the hit sound in **Escape to Victory**, node 28
(`E11_KF_1`). The incoming clone played `Die` (`SOUNDS\DIE.WAV`, bank
11) once at rat contact. The original's victim update plays `Ohno`
(`SOUNDS\OHNO.WAV`, bank 7) as the hit victim enters forced death.
[Playtest ledger](../playtest/2026-11-19-playtest-20.md)

| Moment | Original sample and bank index | Static trigger |
| --- | --- | --- |
| Rat swing, `punch0` through `punch3` | — | `42657A..4265B6 -> 42FD25 -> 43CF53` selects a sprite animation. `408EEB -> 43CF30` advances the strip. |
| Victim hit and flung | `Ohno`, `SOUNDS\OHNO.WAV`, **7** | `4264F2` flags the victim. Its update consumes the flag at `42E340`, reads handle `4E32CC` at `42E349` and plays it with the victim ID at `42E360 -> 44535B`, once per hit. |
| Flying figure launch, flight and expiry | — | `416BF0` is constructed after the cue. `416C49` creates the figure. `4173E0` advances it and `432383 -> 4172A0` checks expiry. |
| Punch finishes, victim removed | — | `408F0A` checks animation completion, `408F39..408F67` removes the victim and `408F7C` restores walking. |
| Rat patrol or idle squeak | — | No dedicated cue in the traced `4265E8` initialization or `426414 -> 42E2BF -> 41BDD4` dry patrol. |

Static target is the registered x86 EXE, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Ghidra 12.1.4 connects `45E5B8..45E5CA` to the installed name and bank.
The bank order agrees with the retained index measurement. REA closed
without a snapshot. [Instructions, pseudocode and bank cross-check](../../build/rat-attack-sound-001/static/crosscheck.md)

`World::stepRats -> cueSample -> SoundEvent -> runApplicationLeg::drainCues
-> CuePlayback::drain -> Mixer::play` now requests `Ohno` from the victim's
feet using the existing spatial gains. The named sample path emits once per
victim independently of the ordinary bomber cue's Nuke guard and
coalescing. The call is immediately before the existing forced-death flight
notification. The original queues the cry when the victim consumes its
forced-death flag. The clone does both at contact, so the original
within-update offset and device playback onset remain unmeasured.

- An installed-level clone trace with SDL dummy audio logged **25 contacts
  and 25 Ohno requests on the matching frames** across both rats and all
  four punch variants. First contact was at 69,466 ms. That victim was
  removed at 69,950 ms. All 25 removals had zero additional cues, and the
  installed Ohno WAV decoded from `lemmings.box`. The trace observes
  routing and simulation, with human listening still pending.
  [Escape sound-event trace](../../build/rat-attack-sound-001/escape-sound-events.log)
- All **24 promoted human wins** passed via `webolution --game-dir
  "Lemmings Revolution" --replay <file>`. Escape to Victory finished at
  step 28,060 with 43/40 saved. The manifest gives every recording, result
  and exact log path. The trace used a private LocalAppData directory and
  no save store. The replay path is save-free.
  [All replay results](../../build/rat-attack-sound-001/replay-results.tsv) ·
  [Escape win](../../build/rat-attack-sound-001/replays/E11_KF_1-1795115359121043200-1.replay.log)
- Registered GCC release build and legacy test syntax check passed.
  The existing assertion drained the queue after completion and expected `Die`.
  It now checks one positioned `Ohno` at contact and an empty queue after completion.
  The release build has no legacy test target, so the initial target
  selection failed and the legacy test received a syntax check only.
  [Initial target selection](../../build/rat-attack-sound-001/initial-target-selection.log) ·
  [Clone build](../../build/rat-attack-sound-001/native-build.log) ·
  [Legacy syntax check](../../build/rat-attack-sound-001/legacy-compile.log)

Changed `src/sim/World.cpp`, `src/__test__/test_evil_rats.cpp` and this report.
[Task diff](../../build/rat-attack-sound-001/task-source.diff)

- **Handoff** `needs_review`. Supervisor smoke pending.
- **Next action** Review the cue path and trace, then confirm the sound in play.

**What to check** In Escape to Victory, let a lemming walk into either rat.
Expect one short “Oh no” as the victim is flung, with silence from the
attack as the figure falls and the rat finishes its punch.
