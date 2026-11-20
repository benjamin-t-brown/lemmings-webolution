# rat-hit-sound-003

Playtest 21 L1 reports the wrong rat-hit sound in **Escape to Victory**
(`E11_KF_1`, node 28). The user identified `SOUNDS\THUNK.WAV` by ear from the
installed samples. The base archive member at `lemmings.box +0x2bf44fb`
decodes as mono PCM at 5,563 Hz with 1,288 frames. The retained level and rat
sprite scan found no record linking that sample to a rat hit.
[Playtest ledger](../playtest/2026-11-19-playtest-21.md) · [Sample inventory](../../build/rat-hit-sound-002/sample-inventory.log)

**Static original evidence.** On the registered x86 EXE (SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`),
the rat writes the forced-death flag at `0x4264F2`. The actor update reads it
at `0x42E340`, loads handle `0x4E32CC` at `0x42E349`, and calls `0x44535B`
with the victim ID at `0x42E360`. `0x45E5B8..0x45E5CA` registers `Ohno`
from `0x4C0CAC` into that handle. The closing gate sets the same flag at
`0x427266`, so the traced original gate crush also requests `Ohno`. Ghidra
12.1.4 pseudocode was checked against the instructions, xrefs and name bytes.
REA closed without a snapshot. Original audible output still needs comparison.
[REA results](../../build/rat-hit-sound-003/static/rea-evidence.json) · [Retained bank and asset cross-check](../../build/rat-hit-sound-002/static/crosscheck.md)

**Clone change.** `World::stepRats -> cueSample -> SoundEvent ->
CuePlayback::drain -> Mixer::play` now requests `Thunk` at the victim's feet
immediately before the forced-death flight notification. This is the shared rat
hit path across installed levels. The code labels it as a provisional
user-identified override of the static `Ohno` read. The clone's
bomber still requests `Ohno` at fuse end under the existing Nuke guard.
`World::consumeGateCrush` still flings the victim silently. Counter-gate motion
still has its separate `Rumble` cue. The existing legacy rat assertion now
expects `Thunk` in place of `Ohno`, with the same count, position and completion
checks. [Source diff](../../build/rat-hit-sound-003/task-source.diff) · [Prior cue change](../../build/rat-hit-sound-003/accepted-rat-change.git-show)

- The registered Windows GCC release build passed. The installed-level clone
  trace used 7,200 updates and SDL dummy audio. It recorded 25 flings across
  both rats and all four punch variants, 25 matching `Thunk` requests with the
  exact victim position on the same update, and zero `Ohno` requests. The
  trace records events and coordinates sent to the position resolver.
  Camera panning and audible output still need a human playtest.
  [Build log](../../build/rat-hit-sound-003/native-build.log) · [Sound-event trace](../../build/rat-hit-sound-003/escape-sound-events.log)
- All 25 promoted replays passed with `webolution --game-dir "Lemmings Revolution"
  --replay <file>` and a private LocalAppData directory. Escape to Victory passed
  at step 28,060 with 43/40 saved, The Iron Curtain at step 18,207 with 21/20,
  and Bomb at step 954 with 17/10. The manifest lists every recording and log.
  [Replay results](../../build/rat-hit-sound-003/replay-results.tsv) · [Escape replay](../../build/rat-hit-sound-003/replays/E11_KF_1-1795115359121043200-1.replay.log)

- **Handoff** `needs_review`. Supervisor smoke is pending.
- **Next action** Review the cue change and trace, then confirm the sound in play.

**What to check** In Escape to Victory, let a lemming walk into either rat.
Expect one Thunk as the victim is flung, then silence from that hit as the
punch finishes and the flying figure expires.
