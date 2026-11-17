# positional-audio-001

Status: **needs_review; supervisor smoke pending**. Playable v1, [playtest 12 B4](../playtest/2026-11-17-playtest-12.md).
Difficulty routine; recommendation Sonnet; assigned Codex override used.
Before: the human heard centred, full-volume effects regardless of their source,
including digging on **The Crowded House, node 21 / E20_MG_1**. The missing
event locations and channel gains are the next dependency. This is audio only;
movement, skills, clocks, contact decisions and save/replay semantics are unchanged.

## Production path and policy

`World::cue/ cueSample -> SoundEvent -> takeSoundEvents -> main.cpp drainCues ->
TubeScene::soundWorldPosition -> actorPoseFromCylinderSample -> spatialSoundGains ->
CuePlayback::playName -> Mixer::play -> Mixer::fill`. Events snapshot terrain feet
coordinates before the actor moves/disappears. The scene uses the same cylinder
interpolation as visible actors; the camera is built from the current controls/intro
pose at drain time, after every simulation update. Audio never feeds simulation.

**Provisional curve:** project the physical origin through the current camera;
`pan = clamp((screenX-cx)/(width/2), -1, 1)`. Let `facing` be the dot product of
the source's horizontal radial direction and the camera's horizontal position.
`gain = .25 + .75*(facing+1)/2`; front gain 1, back gain .25 (about -12 dB).
Positive pan reduces the left channel; negative pan reduces the right. Both
retain full gain at centre. Off-camera X clamps continuously, including the yaw
wrap; a source behind the camera is silent. No extra distance/elevation curve.

Each one-shot keeps these gains for its entire tail. Rotating or zooming changes
future events, with no retuning step in an existing voice. This snapshot policy
is a deliberate clone choice; original tracking can update active sounds. Existing
work/explosion restarts, assignment suppression, exit averaging and voice caps
are retained. The exit bus now applies each voice's stereo gains before averaging.
Several workers still share the accepted single Mine voice; simultaneous work or
blasts can replace tails, and a coalesced blast keeps the first event's location.

| Positioned event | Sample / trigger and limit |
| --- | --- |
| Dig, mine, bash | Mine, existing successful terrain-removal strokes |
| Builder bricks | Chink, existing final-three-brick warnings; earlier bricks remain silent |
| Bomber/Nuke blast | Explode, existing completed blast |
| Ordinary fuse warning | Ohno, existing non-Nuke guard retained |
| Fall death | FallSplat, existing splat transition |
| Exit | Yippee, one existing event per saved actor, including overlapping cheers |
| Water/acid entry | Bubble, **provisional** one-shot at existing Drown attachment; original bubble delay is not reproduced |
| Lava entry | Die, traced original lava entry; existing clone Drown attachment/lifetime retained |
| Lever | Lever, accepted contact/toggle, including the diagnostic tick path |
| Pressure pad | Lever, **provisional** once on latch; original pad activation has no established SFX call |
| Speed pad | SpeedPad, existing first-contact latch |
| Crab and named hazards | Fourth serialized name, if nonempty, on each existing kill; sampled crabs have an empty name and remain silent; installed roller/robin names DEATHROLL/birdcall decode successfully |

Object effects provisionally use the contacting/victim actor's event location
(radial slab .75), rather than the object's original tracked origin. No new crab
sample, Fire cue, ambient loop, teleport cue or music was invented. All eight
existing typed routes remain; supplemental/object samples use the event's sample
payload and load once on first use. Missing samples/devices degrade silently with
a diagnostic; failed dynamic loads are remembered. The legacy `takeCues` diagnostic
sink consumes the same buffer but returns only typed events; production drains all
events. **Action assignment feedback, Changeop UI clicks and Letsgo stay centred.**

## Static authority

Used reverse-engineer-anything with the project static REA profile only.
Reference: `C:/progs/lemmings-webolution/Lemmings Revolution/Lemmings Revolution.exe`,
SHA-256 **32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f**.
Ghidra **12.1.4**, profile `3c06738e45e144d5f073edbd3244c6bff46222f389525fe659bc92dca2ccdece`.
Complete returned packets and limitations: [static/](../../build/positional-audio-001/static/).

- `44535B` records an actor id; `445274` copies a 24-byte position. `4373D9`
  dispatches those forms; `43757F` resolves the actor and calls cylinder transform
  `43D56C`, then `4375F0`. Packets `pseudo-44535B.json`, `pseudo-445274.json`,
  `pseudo-0x4373d9.json`, `pseudo-0x43757f.json`; instruction cross-check
  `sound-wrapper-instructions.json`, evidence `ev_4db09afbe316ae7ea38662fe264efdeaac2823c7e3a7854d1da3a9420d79a009`.
- `4375F0` transforms through `40EAE0` using `58D3D8`. Instructions
  `437616..43761F` retain transformed Z; `43762E..4376FD` clamp it to .15.. .22
  or .06.. .12 according to the camera-mode flag and normalize/invert it.
  `437700..437723` add .5 and cap at 1, giving gain .5..1.
  `43772A..437751` compute `(transformedX/gain + 80)/160` for pan;
  `43776C/4377C2` call volume/pan setters. [Instructions](../../build/positional-audio-001/static/instructions-0x4375f0.json),
  evidence `ev_3c62aca41d34f2d59ccc65dd2690b45cfa959306a0f2de3b4ded84e206c60e8e`.
- `498204` instructions include arithmetic omitted by its pseudocode: user
  volume times positional gain, times **5000**, plus **-5000**, then DirectSound
  slot `+3C` (SetVolume). `4982C1` maps pan to `p*20000-10000`, calling slot
  `+40` (SetPan). Both instruction packets and [constant bytes](../../build/positional-audio-001/static/constants.json)
  retain those values; constant evidence `ev_49fc9620ec1c1dd68e2325de31b219ebab3b9885b71811464ca0a1c57c6044a1`.
  This is camera X/Z arithmetic, not actor facing or recovered inverse-distance audio.
  Original transform scale/mode mapping and DirectSound amplitude equivalence
  are not transplanted into the clone's provisional curve.
- `4333D2` reads the fourth name into sound `+1CC` (`4334DA..43351D`);
  `432E3F` plays it per victim at `43310B..433134`. New pseudocode was checked
  against `named-reader-instructions.json` and `named-kill-instructions.json`,
  evidence `ev_8fe0e8907aac22214977b5f2ed07cf298c70a84eae7041aa20228ed7266d8abd`.
  `431950` pad activation has model/latch/message operations, without an SFX call.
  Retained [lever](../../old/docs/scratch/phase3-round21-lever.md),
  [speed pad](../../old/docs/scratch/phase3-round17-speedpad.md),
  [Bubble](../../old/docs/scratch/phase3-round24-bubble.md),
  [Die](../../old/docs/scratch/phase3-round28-die.md) and
  [accepted work sounds](work-sounds-001.md) establish sample/trigger context.

Analysis closed without snapshot arguments. No original execution/comparison,
debugger, hooks or capture occurred; historical fidelity gates remain unchanged.

## Checks and preservation

Registered UCRT64 GCC/CMake/Ninja build, -O2 -g with assertions, eight jobs:
[production build](../../build/positional-audio-001/build-final-2.log),
[check build](../../build/positional-audio-001/build-checks.log), exit 0.
Existing RetailLevel Lever initializer warnings remain.
Focused **5/5 PASS, 0.79 s**: cueplayback, exit_sound, gameplay_sounds,
work_sound and new **legacy** positional_sound;
[log](../../build/positional-audio-001/focused-5.log),
[JUnit](../../build/positional-audio-001/focused-5.xml).
The new unit checks screen direction against sprite projection across yaw/zoom,
front/back gain, yaw wrap, actual additive/exit-average/restart PCM, stable tails,
typed blast and named hazard event positions after death, centred UI, and installed
sample decoding. This is output-buffer evidence, not human listening or original
mixing fidelity. No existing assertions, timeouts, recordings or test tiers changed.
No new scripted route, replay fixture or frozen-source build. **Supervisor smoke
and independent review remain pending.**

All **19 active promoted replays PASS**, exit 0, won=1, completed=1. Ran the built
`webolution --game-dir "Lemmings Revolution" --replay <file>` on every `.replay`.
Exact filenames, commands, steps and individual logs: [replays.json](../../build/positional-audio-001/replays.json).
No divergence or retirement request. Individual saved/required results:

| Level | PASS saved/required |
| --- | ---: |
| MM_BASH_002 | 10/10 |
| MM_BASH_MINE_WLR1 | 20/10 |
| MM_BLOCK_001_A | 24/15 |
| MM_BLOCK_BOMB_A | 12/10 |
| MM_BOMB | 17/10 |
| MM_BUILD | 20/10 |
| MM_CLIMB_002 | 10/1 |
| MM_DIG_000 | 19/10 |
| MM_FLOAT | 10/1 |
| MM_MINE | 20/10 |
| MM_W_012 | 16/10 |
| MM_E1_3_SKILLS | 10/5 |
| M06_KF_1 | 91/50 |
| MM_E1_4SKILLS | 20/10 |
| MM_E2_4SKILLS | 14/10 |
| MM_BASH_DIG_001_A | 20/10 |
| MM_BUILD_BLOCK_A | 15/10 |
| E19_KF_2 | 24/20 |
| E20_MG_1 | 7/5 |

Changed: `src/audio/{SpatialSound.h,Mixer.h,Mixer.cpp,CuePlayback.h,CuePlayback.cpp,
OverlapMix.h}`, `src/sim/{World.h,World.cpp,RetailLevel.cpp}`,
`src/app/{main.cpp,TubeScene.h,TubeScene.cpp}`, new
`src/__test__/test_positional_sound.cpp`, its legacy CMake registration, this report.
Incoming edits are preserved under [entry/](../../build/positional-audio-001/entry/);
[task-only.patch](../../build/positional-audio-001/task-only.patch) isolates this task.
Supervisor PID **46052** holds RepositoryLease; no protected controls were edited.
Get-Process found no clone/build/test runner at entry; detailed CIM metadata was
denied. Checks used private `build/positional-audio-001/appdata`; replay/unit paths
open no SaveStore. Normal and installed saves were unused. Changed text is LF;
scoped diff whitespace check passes. No subagents, detached workers, installations,
commits, pushes, queue/verifier/controller/tool-registry/shared-instruction edits.

Handoff: positioned one-shots and per-channel mixing implemented; five focused checks and all 19 promoted wins pass.
Next: supervisor smoke and independent review, then human listening confirms B4; task remains needs_review.

**What to check:** **E20_MG_1 / The Crowded House (node 21)**: assign diggers,
rotate so a worker is on the left, then right, then the far side; new strokes
should sound on that side and become quieter at the back, while an already
playing tail stays steady during rotation. On **MM_E1_4SKILLS / Sergeant Bash
(node 20)**, compare miner/blast and roller sounds at different camera angles.
On **MM_W_012**, pull the lever; on **MM_BLOCK_BUILD_BASH / Seafood Sarnie**,
activate the pad: the click should come from the contact's screen position.
UI clicks, assignment feedback and the Let's Go voice should remain centred.
