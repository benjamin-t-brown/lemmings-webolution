# rat-hit-sound-002

**Blocked on the sound identity.** Playtest 21 L1 reported a different hit cue
in the original from the clone's `Ohno` in **Escape to Victory** (`E11_KF_1`,
node 28). The registered executable and installed data resolve the traced hit
event to `SOUNDS\OHNO.WAV`. They supply no evidence for a replacement.
[Playtest ledger](../playtest/2026-11-19-playtest-21.md)

- **Hit and playback.** The rat sets victim `+5D` at `4264F2`. When the victim
  consumes the flag at `42E340`, `42E349` reads the handle at `4E32CC` and passes
  it to `44535B` with the victim ID. `4453C3 -> 404CC0` fetches the bank sample
  and `445421 -> 497F39` requests playback before `416BF0/416C49` launches the
  figure. The traced punch completion, flight and expiry paths dispatch no
  separate cue. `41442C` uses the same handle for the guarded bomber cue.
- **Slot binding.** `45E5B8..45E5CA` registers `Ohno` from `4C0CAC` through
  `41D7D0` in bank `58CB90` and stores the returned index at `4E32CC`. The
  first-free rule is real. `45EA01 -> 406440` holds a startup reference, so
  `445A10 -> 449790` preserves that entry at level load. The loader skips
  duplicate names and fills only inactive slots. A cache miss reaches
  `406A20 -> 499840 -> 4741E0 -> 497957 -> 497A87`. The last function opens
  `Sounds\%s.wav` at `497B5D..497B8C`. The size lookup at `444629` accounts
  for load progress. The patch contains zero WAVs, so the resolved sample
  comes from `lemmings.box`.
- **Installed alternatives.** The level sound table at `+0x2cfb` lists the
  common cues, including `Ohno`, `Die`, `Splat` and `FallSplat`, plus
  `DEATHROLL` at `+0x3e43`. No rat or named hazard record binds `DEATHROLL`
  to a hit in this level. Both installed rat sprites parse through EOF with
  clip, frame and texture data for `punch0` through `punch3`. `MOUSEPRE`,
  `THUD`, `THUNK`, `BANG` and `OING` are present in the archive and yielded no
  matching length-prefixed name in the level or the rat sprites.
  [Asset probe](../../build/rat-hit-sound-002/asset-probe.log) · [Sample inventory](../../build/rat-hit-sound-002/sample-inventory.log)

Static reference SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Ghidra 12.1.4 pseudocode was cross-checked against instructions and vtable
bytes. REA closed without a snapshot. Original audible output remains
unverified. [Address cross-check and REA records](../../build/rat-hit-sound-002/static/crosscheck.md)

The current clone emits the cue in `World::stepRats -> cueSample -> SoundEvent
-> CuePlayback::drain -> Mixer::play`. This report is the only repository edit
for the task.
The recovered asset parser and archive inventory completed successfully. The
worker ran no clone trace or promoted replays for this blocked pass. Supervisor
smoke is pending. [Accepted clone change](../../build/rat-hit-sound-002/accepted-rat-change.git-show)

- **Handoff** `blocked`. Playtest L1 remains open.
- **Next action** Identify the cue from a sample reference or independently observed playback before changing the clone.

**What to check** In Escape to Victory, let a lemming reach either rat. The
traced path requests one hit cue as the lemming is flung and no cue when the
figure expires. The sound heard at launch is the outstanding comparison.
