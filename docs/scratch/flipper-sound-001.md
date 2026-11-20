# flipper-sound-001

Playtest 21 L4 reports silent gravity flips in **2 sides to every story**
(`M08_KF_1`, node 30). The clone's `World::stepWalker` reversed gravity and
entered `Faller` without submitting a sound event.
[Playtest ledger](../playtest/2026-11-19-playtest-21.md)

**Static evidence.** On the registered x86 EXE (SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`),
the gadget reader registers `AntiGrav` at `0x41D76F` and stores its index at
`0x41D777`. The update at `0x41D384` sets its contact flag at `0x41D450`
for any lemming hit. It resolves the sample at `0x41D4BF` and checks current
playback at `0x41D4E0`. With contact, a resolved sample and a clear playing
bit, `0x41D590` calls `0x44535B` for one positional `ANTIGRAV.WAV` at the
gadget. The contact flag is local to the gadget update. The gravity match at
`0x41D475` guards a separate call. Volume and stop at `0x41D67C` and
`0x41D6E1` reuse the same sample. The inspected flip-back, animation and
inverted-fall paths show no separate sound start. `AGRISE.WAV` and `AGFALL.WAV`
are present in the archive, with no use established by the inspected paths
or retained cue census. Original audible behavior remains unobserved.
[Instruction cross-check](../../build/flipper-sound-001/static-crosscheck.md) ·
[Revision REA evidence](../../build/flipper-sound-001/revision/rea-crosscheck.json)

**Clone change.** `World::stepWalker::flipIfAntiGrav` calls
`cueSample("AntiGrav", l)` after reversing gravity. The existing transition
to `Faller` and immediate return give one `SoundEvent` per actual reversal
in either direction. `CuePlayback::drain` uses the existing actor-foot
position policy and sends the installed sample to `Mixer::play` as
`AddWithoutEviction`. The initial shared-pool route could cut the oldest
lemming or gadget sound at 32 additive voices. The desktop mixer and Web
Audio now mix flipper one-shots additively outside that pool. The reversal
trigger, actor position, overlap and one-shot lifetime are provisional clone
choices. The original path starts at gadget contact and uses a
playing-status guard and fade logic.
[Source diff](../../build/flipper-sound-001/revision/task-source.diff)

- The M08 human win reproduced at step 33,240 with 42/40 saved. The dummy SDL
  trace matched five flips into inverted gravity at `(171,93,0.75)` and two
  return flips at `(151,3,0.75)` to seven positioned `AntiGrav` requests
  and seven mixer voices. The returns were actors 6 and 8 at steps 15,552
  and 16,465. The trace found zero mismatches or extra cues during falls.
  [Sound-event trace](../../build/flipper-sound-001/revision/m08-sound-events.log)
- A native saturation probe held 32 installed `FallSplat` and `Lever` voices.
  A flip started its own positioned voice and preserved every existing clip
  and playhead. The next ordinary request still evicted the oldest capped
  voice. The Web Audio scheduling probe passed with a mock audio context.
  Added a PCM capacity regression to the legacy positional-sound test and
  checked its syntax.
  [Native probe](../../build/flipper-sound-001/revision/m08-sound-events.log) ·
  [Web probe](../../build/flipper-sound-001/revision/web-voice-probe.log) ·
  [Test compile check](../../build/flipper-sound-001/revision/legacy-test-syntax.log)
- The private MinGW GCC release build succeeded. All 25 promoted replays and
  the additional unpromoted M08 win passed through `webolution --replay`
  with private LocalAppData. Audible output remains for live playtest.
  [Build](../../build/flipper-sound-001/revision/native-build.log) ·
  [Per-recording results](../../build/flipper-sound-001/revision/replay-results.tsv)

- **Handoff** `needs_review`. Supervisor smoke is pending.
- **Next action** Review the cue and both reversal directions after smoke, then confirm the sound in play.

**What to check** In **2 sides to every story**, guide a lemming onto each
flipper. Expect one short sound at each gravity reversal, including the
return to upright, with no new sound during the ensuing fall.
