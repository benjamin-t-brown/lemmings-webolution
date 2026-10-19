# A4e — interactive SDL window and bounded A1 camera controls

2026-10-19, branch `decompile`. The interrupted prediction-only A4e was resumed
with one round worker. The facilitator independently verified the result.

## Accepted replacement behavior

Normal launch now opens a 640x480 SDL window and presents the accepted tube
`route` by default. Explicit diagnostic geometry remains available. `--shot`
renders once before SDL initialization; `--frames N` provides a finite window
smoke run. The title and `--help` expose right-mouse grab/drag, NUMPAD4/6,
SPACE/CTRL zoom and BACKSPACE flip. LSHIFT triples grabbed key rotation and
is covered by the controller tests.

The SDL-free controller and input queue implement the accepted key/mouse signs,
zoom/elevation mapping and flip curve. Main uses a fixed 144 Hz accumulator,
retains one-shot input until an update consumes it, and preserves held input
through catch-up steps. Focus loss clears pending events and releases grab.
Window dimensions are normalized before constructing the controller.

This is replacement integration based on A1 evidence. No stock game was started,
no live memory or install file was written, and no new stock behavior was measured.

## Narrowed readings from the stock bytes

The authority SHA-256 remains
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
The independent reader maps addresses through PE section raw sizes; `.data`
file bytes end at `0x004C6000`. Operand and instruction reads are retained in
`build/tmp/a4e-window-controls/facilitator-control-bytes.json`.

- **Zoom entry only:** `0x45306A 837a1401` requires pending zoom 1;
  `0x453077 83781000` requires current zoom 0; the following `jne` skips the
  setup on exit. Elevation setup and the edge yaw nudge must not repeat on exit.
- **Nudge deadzone:** `0x45315F e8ccadfbff` calls the magnitude helper;
  `0x40DF3B e8b4e90800` reaches its finite-number sign clear at
  `0x49C98C 25ffffff7f`. `0x453167 dc1d40104b00` compares magnitude with
  `0x4B1040 9a9999999999b93f` (0.1); `0x453172 740e` skips zeroing when
  magnitude is at least that value. Thus `abs((cursorX-320)/90) < 0.1`
  produces no nudge; otherwise the clamped ratio is scaled by 30.
- **Correct gain address:** `0x4531C6 dc0d48174b00` reads double 30 at
  `0x4B1748 0000000000003e40`. The old predictor comment's `0x4C03D8`
  is an unrelated int32 30, not this double operand. The initial independent
  assertion against that wrong address failed; the correction is retained.
- **Incremental flip:** `0x453EC9 dca2c0160000` subtracts the previous curve;
  `0x453ED5 dc88b8160000` scales the difference by the flip amount.
  `0x453EFF dc4158` and `0x453F14 dc4040` add the delta to yaw fields.
  Concurrent grabbed input must survive subsequent flip updates.
- **Yaw and free cursor bounds:** `0x454170 dc25a8074b00` and
  `0x4541A0 dc05a8074b00` subtract/add 256 in the wrap loops. The free-cursor
  branches `0x454010..0x454077` and `0x45407D..0x4540E4` clamp to zero and
  viewport size minus one (`0x45405C 83ea01`, `0x4540C9 83e901`).

These are READ corrections to the bounded implementation brief, not an expanded
live acceptance of all control branches. Not-grabbed key signs remain
NUMPAD4 = -4.8 cursor x/update, NUMPAD6 = +4.8, as in the retained A1 predictor.

## Facilitator check

The facilitator rebuilt and ran the full suite: **19/19 CTest pass**. A separately
generated C++ probe replayed retained stock input samples, including grabbed
keys, mouse and free cursor movement: **54 comparisons pass at 1e-4 tolerance**.
Those are replays of existing measurements, not 54 new live observations.

An independent native-window probe launches only the replacement, with no
`--native` flag. Its actual client is 640x480; the presented initial RGB frame
matches an explicit `--native --geom route --shot` frame exactly (full-frame
MAE 0). A posted SPACE key event changes the presented frame, and WM_CLOSE
exits 0. The separate SDL dummy three-frame run also exits 0. This establishes
that bounded native event/presentation path; it does not establish physical
mouse delivery, grab/focus transitions or every keyboard binding end to end.

The facilitator independently rerendered all nine A4d views, calculated the RGB
errors directly, and compared PNG hashes with the pre-A4e baseline. Every score
and hash is unchanged. Gate: 640x480, ROI `[100,80,280,410]`, normalized MAE
`<=0.02`. Scores in view order:

| View | MAE |
| --- | ---: |
| initial | 0.015584142 |
| mouse | 0.016047116 |
| right | 0.014810480 |
| left | 0.015679210 |
| canonical | 0.015586607 |
| zoom1 | 0.012003521 |
| elev_up | 0.011692744 |
| elev_down | 0.012866662 |
| elev0_neg | 0.015869215 |

Fourteen isolated controller/clock/event source regressions were checked:
shift multiplier, mouse yaw gain, both displayed elevation gains, applied
elevation gain, flip duration/curve/additive update, deadzone, zoom-exit nudge,
free-key sign, yaw wrap, clock rate and queued-event replay. Thirteen failed
initially. **The zoom-exit mutation survived** because its test stayed inside
the nudge deadzone. The worker added off-center entry/exit and moved-cursor-Y
checks; the facilitator repeated that one mutation and it now fails with
`zoom exit does not repeat edge nudge`. Original failure and corrective rerun
records are both retained. Every source mutation was restored byte-for-byte,
rebuilt, and followed by a passing focused test.

Restoring only the old default `wall` geometry also fails the fixed pixel gate:
MAE `0.031833014`; restoring `route` passes at `0.015584142` and restores PNG
SHA-256 `66a8ea3b44df2216b64debf9d8f417d2ed923841fc440e99c028b013010d3651`.
The final native smoke was repeated after restoration. The stock executable
hash is unchanged; no stock or replacement process remains. The staged diff
check is clean after removing an older harness's extra EOF blank line; all 12
newly staged Python harness scripts compile. The worker's A4e implementation
changed only main, the new input/controller unit, its test and CMake registration;
facilitator changes are documentation and verification.

## Limits and next bounded task

The fixed 144 Hz rate approximates A1's measured foreground rate of about
143.9 updates/s; elapsed accumulation is capped at 0.25 seconds. This is an
explicit replacement policy, not a bit-exact original scheduling claim.
`--frames` uses synthetic fixed elapsed time for smoke testing. The pure
UI-state gates are tested, but main currently supplies UI state 0. Attached
object/picking gates, cursor artwork and frontend/pause transitions are not
integrated. Held/retriggered BACKSPACE combinations have no new live coverage.

A4 remains open. All-level fidelity, simulation/actor presentation, sprite
placement/rasterization, scenery and HUD remain open. Preserve the A2 wider,
cavity and rim failures and the A3 strict sensitivity failure.

Recommended next round, **not dispatched**: A4f, one actor billboard fixture.
Question: can a replacement draw a retained actor-bound walk/block fixture
with original placement, UV/mirror and depth, using the already accepted A3
selector? Dependencies: accepted A1 camera, A3 temporal/directional axes and
actor captures, A4d tube renderer; inspect sprite path `0x47D0C0` before choosing
geometry. Worker ownership: a sprite presentation unit, focused tests and new
harness scripts; `Raster` changes only if original blend/depth evidence requires
them. No simulation-rule or tube-route edits. Acceptance: predeclare an actor-bound
crop before rendering and require normalized RGB MAE <=0.02 plus a strictly worse
score and nonzero changed pixels for each isolated visual regression; derive
placement from bytes/actor-bound evidence,
pass declared five/eight-direction fixtures, discriminate individual placement,
mirror and temporal-frame regressions, keep the existing 19 tests and all nine tube hashes
unchanged, and restore all live writes if fresh capture is necessary. Do not
combine this with a whole simulation/HUD integration round.

## Local reproduction

```powershell
.\scripts\Invoke-Ucrt64.ps1 "cmake --build --preset dev-debug"
.\scripts\Invoke-Ucrt64.ps1 "ctest --preset dev-debug"
.\scripts\Invoke-Ucrt64.ps1 "SDL_VIDEODRIVER=dummy ./build/cmake/dev-debug/webolution.exe --native --frames 3"
python build/tmp/a4e-window-controls/facilitator_native_smoke.py
```

Evidence under `build/tmp/a4e-window-controls/`: `facilitator-control-bytes.json`,
`facilitator_stock_probe.cpp`, `facilitator-native/result.json`,
`facilitator-renders/results.json`, `facilitator-mutations/results.json`,
`facilitator-mutations/zoom-exit-rerun.json`, `facilitator-mutations/default-route.json`.
The candidate note and earlier prediction are retained separately. Native renderer
children need UCRT64 DLLs; normal Python keeps its normal PATH.
