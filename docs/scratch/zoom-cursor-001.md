# zoom-cursor-001

**needs_review.** Supervisor smoke is pending for this revision. Playable v1,
ordinary frontend → level map → **One Way Ticket (MM_ALL_8_001)**. Playtest 13 C1
reported that Space zoomed toward an apparently random point. The previous
controller used a virtual cursor accumulated at 2.0×/2.4× relative motion.
Its row set elevation and its horizontal position drove a yaw nudge of up to
30 units. The nudge could move the intended feature away from the drawn
pointer. [Playtest 13](../playtest/2026-11-17-playtest-13.md)

## Production path

`runApplicationLeg` translates SDL events to logical game pixels.
Space or either Ctrl key calls `zoomAtPointer` with the same `pointerX/Y` used
for hover and clicks. At the pixel centre it reads visible actor depth, falling
back to the displayed scenery depth. It refreshes a deferred scene through
the same render path as picking. `CameraBasis::worldRay` reverses the existing
view projection, also used by actor occlusion, to reconstruct the point.
`cursorZoomPose` solves lateral and vertical eye offsets at the existing
zoomed forward distance. With a settled camera, this moves the eye along the
same sightline through terrain openings. [Application input](../../src/app/main.cpp#L1184) · [Camera projection](../../src/app/Camera.cpp#L243)

`PendingCameraEvents::zoom` carries that pose to the next fixed camera step.
`CameraController::update` applies it on zoom-in, and `buildCamera` uses the
offsets at zoom 1. Zoom-out still toggles the scale and retains the controller
yaw. Right-drag keeps its yaw and elevation gains. The previous attempt
shifted the entire vertical range by the anchor's overshoot. For example,
an anchor at 67.5 moved the lower limit from −10.5 to 9.5. The revision lets
the camera start outside the range and move inward at the usual rate. An
outward drag beyond either limit holds the current height. After re-entry,
the original limits of −10.5 and 47.5 both apply. Camera input is presentation-only.
`ReplayRecorder` records resolved actor assignments and other simulation
commands, so replay files need no camera event. [Camera input](../../src/app/CameraInput.cpp#L38) · [Replay recorder](../../src/app/Replay.hpp#L97) · [Previous review](../../build/agent-runs/loop-a86e20b263664cd7bb21ceea87e7f193/0002-review-1e54c6/decision.json)

**Provisional clone behavior.** The historical A1 input mapping measured the
virtual-cursor nudge on the registered reference EXE (SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`).
Retained pose instructions at `0x45CC61–0x45CCC2` establish eye-distance
scaling and `0x45CF30–0x45CF48` establish zoom-only elevation. The new eye
offsets implement the user's playtest request. [A1 controls](phase-a1-control-mapping.md) · [A1 zoom arithmetic](phase-a1-zoom-elevation.md)

Blank pixels, the skill panel, and targets behind the zoomed eye use neutral
zoom. Scenery depth uses the renderer's linear interpolation and comparison
bias, so it approximates physical depth. Moving actors and the opening camera
animation can move a feature between frames. Human confirmation is pending.

Across both attempts the task changes `src/app/main.cpp`, `Camera.cpp/.h`, `CameraInput.cpp/.h`,
`ActorDepth.h`, `ActorOcclusion.h`, and `src/__test__/test_camerainput.cpp`.
The smoke unit expectations for the old row formula and yaw thresholds now
check the supplied offsets, independence from the virtual cursor, and the
neutral fallback. This revision changes `CameraInput.cpp/.h` and this report.
The existing camera tests ran without further edits. [Source backups](../../build/zoom-cursor-001/backup/) · [Pre-task main.cpp](../../build/zoom-cursor-001/main.HEAD.cpp) · [Incoming revision sources](../../build/zoom-cursor-001/review-revision/)

## Checks

- Registered UCRT64 GCC/CMake/Ninja rebuilt the clone and camera tests for this
  revision. `camerainput` and `cameraintro` passed. A temporary controller probe
  tested anchors at 67.5 and −30.5. It checked horizontal drag, gradual inward
  motion, reversal before re-entry, crossing the range boundary at the usual
  gain, reaching both original endpoints and immediate reversal at saturation.
  [Build log](../../build/zoom-cursor-001/review-revision/build.log) · [CTest log](../../build/zoom-cursor-001/review-revision/focused.log) · [Probe source](../../build/zoom-cursor-001/review-revision/drag-probe.cpp) · [Probe results](../../build/zoom-cursor-001/review-revision/drag-probe.log)
- Previous-attempt offscreen source inspection used One Way Ticket and **MM_CLIMB_002** terrain.
  All 54 left/centre/right and top/middle/bottom samples reprojected within
  0.001 px, and the zoomed render's depth at each pixel was within 0.109 world
  units of the sampled point. The discarded yaw prototype put a nearer wall
  in front of five of the same samples. Eight visible injected lemming poses
  on those two levels had a visible actor pixel within 1 px after zoom.
  [Terrain probe](../../build/zoom-cursor-001/probe.cpp) · [Terrain results](../../build/zoom-cursor-001/probe.log) · [Yaw prototype results](../../build/zoom-cursor-001/orbit-prototype.log) · [Actor probe](../../build/zoom-cursor-001/actor-probe.cpp) · [Actor results](../../build/zoom-cursor-001/actor-probe.log)
- All 20 promoted recordings returned **PASS**, exit 0, `won=1` and
  `completed=1` with `webolution --game-dir "Lemmings Revolution" --replay <file>`.
  The revised clone used private `LOCALAPPDATA` for these headless runs. The
  replay path bypasses the save store. All outcomes matched the previous attempt.
  [Replay filenames and output](../../build/zoom-cursor-001/review-revision/replays.log) · [Previous replay results](../../build/zoom-cursor-001/replays.log)
- The previous supervisor run passed 95 main smoke tests and 84 recovery tests.
  The current smoke gate is pending supervisor execution after worker exit.
  [Previous supervisor verification](../../build/agent-runs/20261118T062133-49048-d9d552a2/verification.log)

| Level | Final step | Saved / needed |
| --- | ---: | ---: |
| MM_BASH_002 | 1844 | 10/10 |
| MM_BASH_MINE_WLR1 | 2272 | 20/10 |
| MM_BLOCK_001_A | 1619 | 24/15 |
| MM_BLOCK_BOMB_A | 1546 | 12/10 |
| MM_BOMB | 954 | 17/10 |
| MM_BUILD | 1685 | 20/10 |
| MM_CLIMB_002 | 1252 | 10/1 |
| MM_DIG_000 | 1014 | 19/10 |
| MM_FLOAT | 978 | 10/1 |
| MM_MINE | 1137 | 20/10 |
| MM_W_012 | 1742 | 16/10 |
| MM_E1_3_SKILLS | 2095 | 10/5 |
| M06_KF_1 | 1544 | 91/50 |
| MM_E1_4SKILLS | 2506 | 20/10 |
| MM_E2_4SKILLS | 2142 | 14/10 |
| MM_BASH_DIG_001_A | 3241 | 20/10 |
| MM_BUILD_BLOCK_A | 2482 | 15/10 |
| E19_KF_2 | 6034 | 24/20 |
| E20_MG_1 | 12810 | 7/5 |
| MM_ALL_8_001 | 13812 | 46/40 |

- Handoff — Cursor-anchored zoom preserves both original drag limits after re-entry. Both camera units and all 20 promoted wins pass.
- Next — Supervisor smoke after worker exit, then independent review and human playtest.

**What to check** — Open **One Way Ticket (MM_ALL_8_001)** and let the opening
camera settle. Park the pointer over a lemming or a terrain detail at the left,
centre, and right of the tube, including near the top and near the bottom above
the panel. Press Space or Ctrl. The detail should enlarge and stay at or very
near the pointer. After zooming on a high or low detail, right-drag horizontally
and check that the camera keeps its height. Drag vertically toward the middle
and check for smooth movement through to the opposite limit. Reverse and check
that it stops at the same limits as a centre zoom. Press Space or Ctrl again to
return to the wider view.
