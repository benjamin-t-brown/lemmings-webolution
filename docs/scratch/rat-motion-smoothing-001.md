# rat-motion-smoothing-001

**Needs review. Supervisor smoke pending.**

[Playtest 19 J5](../playtest/2026-11-19-playtest-19.md) reports jitter in
**Escape to Victory**, installed node 28 / `E11_KF_1`. The rat loop in
`injectedWalkersFromWorld` sent integer feet directly to the renderer, so each
cell step appeared in a single frame. Lemmings already sampled a buffered path
between confirmed positions.

`runApplicationLeg` calls `LemmingMotionTracker::observe` before advancing the
world and after each `RuntimeSimState::advanceElapsed` update segment, including
each fast-forward pass. The tracker now keeps a path for each placed rat using
the same X/Y interpolation, cylinder wrapping and discontinuity checks as the
lemmings. `injectedWalkersFromWorld -> drawInjectedWalkers ->
prepareInjectedWalker -> actorSpritePlacement / drawActorSprite` uses the
sampled rat pose. Turns, gravity changes, teleport entry and arrival, and large
jumps snap X/Y. A punch pins the pose to the confirmed contact point, a
provisional presentation choice because the sim keeps its movement state during
the punch. Pausing freezes the level clock and thus the sampled pose. Rescue
and exit-tail draws still use their existing position and lane calculations.
The rat draw entries stay after the live rescue and exit-tail entries. Picking
iterates the live rescue prefix. Simulation, attack contact and counts are
untouched.

Static Ghidra 12.1.4 evidence from `Lemmings Revolution/Lemmings Revolution.exe`
(SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`)
shows that the rat constructor at `0x448630` calls the common constructor,
which enables the lane flag at `0x42DDD5`. The shared walker calls
`0x42F977` from `0x41BF53`. That helper uses facing to approach depth 0.25
or 0.75 by 0.02 per call, with a crossing clamp. The clone keeps the existing
rat lane rule from [evil-rats-001](evil-rats-001.md). Escape's rats start at
their stored 0.5 depth and walking approaches the facing lane at the provisional
rate of one slab unit per level second. The tracker reads that lane directly.
The X/Y buffer is a provisional clone policy, and the original lane cadence
remains unmeasured. [Static evidence and instruction
records](../../build/rat-motion-smoothing-001/static/rea.json).

## Clone observations

The native clone ran Escape to Victory through `runApplication` with a fixed
1/60 s frame input and the SDL dummy driver. The session recorded 480 consecutive
640×480 framebuffers with a private save path and no skill assignments. The camera
began at yaw 155/256 and rotated to 195/256 after the pause to expose the lower
walkway. [Build log](../../build/rat-motion-smoothing-001/observer-turn-view-build.log) ·
[Run log](../../build/rat-motion-smoothing-001/escape-turn-view/run.log) ·
[SDL inputs](../../build/rat-motion-smoothing-001/escape-turn-view/inputs.tsv)

- **Normal speed.** Both rats advanced across the upper walkway in small
  increments. Across frames 174–181, their glove markers moved about 4.0 and
  2.5 pixels right while their confirmed X cells held at 169 and 187.
  [Consecutive frames](../../build/rat-motion-smoothing-001/escape-turn-view/normal-detail.png) ·
  [Animation](../../build/rat-motion-smoothing-001/escape-turn-view/normal-speed.png)
- **Fast-forward and turn.** Both rats kept advancing between cell changes
  on frames 373–380 with the plate held. The hold advanced level time 5,600 ms
  over 48 frame intervals. The second rat faced right on frame 399 and faced
  left on frame 400 at its lower-walkway reversal. The wall briefly clipped
  the sprite, then it emerged heading left. Its depth eased from 0.75 before
  the turn to 0.25 by frame 405. The session exercised a turn and ordinary
  falls. Teleport and large-jump snapping are supported by the source trace.
  [Turn frames](../../build/rat-motion-smoothing-001/escape-turn-view/turn-detail.png) ·
  [Animation](../../build/rat-motion-smoothing-001/escape-turn-view/fast-forward-and-turn.png) ·
  [Pixel measurements](../../build/rat-motion-smoothing-001/escape-turn-view/rat-pixels.tsv)
- **Pause and lemmings.** During frames 313–342 the clock held at 5,216 ms
  and the crop containing both rats was pixel-identical to frame 312. In the
  same session the first lemming held its drawn pose and walk frame while
  paused. After resume, its sampled X advanced from 75.245 to 75.995 between
  frames 343 and 352 with its confirmed X still 76. It visibly walked out
  from behind the column as the rats resumed. The fixed-camera session also
  shows the lemming walking during fast-forward.
  [Pause animation](../../build/rat-motion-smoothing-001/escape-turn-view/pause-and-resume.png) ·
  [Lemming frames](../../build/rat-motion-smoothing-001/escape-turn-view/lemming-detail.png) ·
  [Frame measurements](../../build/rat-motion-smoothing-001/escape-turn-view/frame-summary.txt) ·
  [Fixed-camera frames](../../build/rat-motion-smoothing-001/escape-observation/scene-overview.png) ·
  [Fixed-camera run](../../build/rat-motion-smoothing-001/escape-observation/run.log)

## Checks

The previous MinGW-w64 `webolution.exe` build passed. The prior supervisor
smoke passed 95 main tests, including the 102-level sweep, and 84 recovery
tests. The new supervisor smoke is pending. The 23 promoted wins all passed
in the retained incoming desktop run. The replay selection found zero on the
17 installed rat levels, so this revision had zero applicable replay runs.
The fixed-cadence clone observations leave desktop wall-clock presentation
for the user's playtest.
[Clone build](../../build/rat-motion-smoothing-001/build-webolution.log) ·
[Prior smoke](../../build/agent-runs/20261119T165054-57312-475d4659/verification.log) ·
[Incoming replays](../../build/escape-seams-001/after-desktop-replays/results.tsv) ·
[Replay selection](../../build/rat-motion-smoothing-001/replay-selection.json)

The implementation changes are in `src/app/LemmingMotion.cpp`,
`src/app/LemmingMotion.h` and `src/app/TubeScene.cpp`. This revision adds the
observations to this report and archives the supporting files under
`build/rat-motion-smoothing-001/`. The scoped whitespace check passed.

- **Handoff** `needs_review`. Supervisor smoke is pending.
- **Next action** Review the clone frames and smoke result, then confirm J5 in desktop play.

**What to check**

In **Escape to Victory** (node 28 / `E11_KF_1`), watch both
rats on the upper walkway at normal speed and in fast-forward. Pause mid-stride
and resume. The rats should glide between cell positions, turn without trailing
across the old path, and hold still while paused. Lemmings should move as before.
