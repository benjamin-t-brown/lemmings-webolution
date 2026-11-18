# Right-drag yaw reversal

**Needs review.** Playable v1, ordinary frontend to level map to **One Way Ticket**
(node 22, `MM_ALL_8_001`). Playtest 13 C2 reported that right-drag turned the
cylinder the wrong way. The user chose to reverse horizontal drag. For a
12-unit rightward input, the camera previously subtracted 5 yaw units and now
adds 5. The leftward input has the opposite effect at the same gain.
[Playtest 13](../playtest/2026-11-17-playtest-13.md)

## Production path

`runApplicationLeg` enters relative mouse mode on a gameplay camera grab and
queues scaled SDL motion in `PendingCameraEvents`. The fixed-step loop passes
the deltas and held inputs to `CameraController::update`. The grabbed branch
now adds `(10.0 / 24.0) * mouseDx` to yaw. Vertical drag and NUMPAD4/6 keep
their existing calculations. `pickReference_[0]` still follows the actual
yaw delta using its existing wrap rule. The render builds `displayedCamera`
from the updated pose. After release, hover and assignment use that displayed
camera and scene depth to place and rank actor hit boxes at the pointer.
[SDL input](../../src/app/main.cpp#L1305) · [Fixed update](../../src/app/main.cpp#L1430) ·
[Camera input](../../src/app/CameraInput.cpp#L68) · [Displayed pose](../../src/app/main.cpp#L1531) ·
[Assignment picking](../../src/app/main.cpp#L1339)

This is provisional clone behavior based on the user's playtest. The retained
A1 trace measured negative grabbed horizontal gain on the registered reference
EXE (SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`),
with gain and divisor operands at `0x004C0398` and `0x004C0388`. The clone now
uses positive gain. A direction option is tracked for after 1.0.
[A1 evidence](phase-a1-control-mapping.md) · [Post-1.0 list](../POST_1_0.md)

## Checks

- Rebuilt `webolution` and `test_camerainput` with the registered UCRT64 tools.
  The focused `camerainput` smoke test passed. For the same positive delta,
  its yaw assertion moves from 25 to 35 (starting at 30), and its zoomed
  pick-reference assertion moves from 27 to 37 (starting at 32). The combined
  flip-and-drag case uses `mouseDx = -144` to preserve its −60-unit drag and
  the existing exact-zero wrap-order assertion. Subsequent assertions that
  use the first drag's yaw now start from 35. The existing vertical and key
  assertions still pass. [Build log](../../build/drag-yaw-reverse-001/build.log) ·
  [CTest log](../../build/drag-yaw-reverse-001/camerainput.log)
- Ran all 20 promoted recordings sequentially through the rebuilt clone with
  `--game-dir "Lemmings Revolution" --replay <file>`. Each returned exit 0,
  `PASS`, `won=1` and `completed=1`. The private `LOCALAPPDATA` stayed empty.
  Replay consumes resolved simulation commands and bypasses both the save store
  and the interactive camera and picker. Live drag feel and picking after a drag
  await human confirmation. [Exact recordings and output](../../build/drag-yaw-reverse-001/replays.log)

| Level | Final step | Saved / needed |
| --- | ---: | ---: |
| `MM_BASH_002` | 1844 | 10/10 |
| `MM_BASH_MINE_WLR1` | 2272 | 20/10 |
| `MM_BLOCK_001_A` | 1619 | 24/15 |
| `MM_BLOCK_BOMB_A` | 1546 | 12/10 |
| `MM_BOMB` | 954 | 17/10 |
| `MM_BUILD` | 1685 | 20/10 |
| `MM_CLIMB_002` | 1252 | 10/1 |
| `MM_DIG_000` | 1014 | 19/10 |
| `MM_FLOAT` | 978 | 10/1 |
| `MM_MINE` | 1137 | 20/10 |
| `MM_W_012` | 1742 | 16/10 |
| `MM_E1_3_SKILLS` | 2095 | 10/5 |
| `M06_KF_1` | 1544 | 91/50 |
| `MM_E1_4SKILLS` | 2506 | 20/10 |
| `MM_E2_4SKILLS` | 2142 | 14/10 |
| `MM_BASH_DIG_001_A` | 3241 | 20/10 |
| `MM_BUILD_BLOCK_A` | 2482 | 15/10 |
| `E19_KF_2` | 6034 | 24/20 |
| `E20_MG_1` | 12810 | 7/5 |
| `MM_ALL_8_001` | 13812 | 46/40 |

- Handoff — Horizontal right-drag reversed. The focused camera test and all 20 promoted wins passed.
- Next — Supervisor smoke after worker exit, then independent review and human playtest.

**What to check** — Open **One Way Ticket**, let the opening camera settle,
and hold the right mouse button over empty terrain. Drag right and left to
check that the cylinder turns opposite to the previous build at the same
speed. Drag up and down at both zoom levels to check the familiar tilt
response. While still grabbed, press NUMPAD4 and NUMPAD6 to check their usual
directions. Release the button, select an available skill, and click an
eligible lemming. The lemming under the pointer should receive the skill.
