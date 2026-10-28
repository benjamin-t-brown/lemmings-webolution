# First-level opening camera move

The [connected native comparison](frontend-native-comparison-2026-10-28.md) showed
a different view roughly2seconds after entering `MM_CLIMB_002`. Static tracing
found a missing **opening animation**, not evidence that the base yaw32 is wrong.
No further original process was started; no screenshot was used to fit a value.

## Native findings: READ / DERIVED

Complete raw-backed recipe/checker/REA records: `build/tmp/enacter-camera-start/`.
Main independently reran `check_evidence.py` successfully.

- Successful level entry45C9AB calls level start444D1E. Its camera seed comes
  from the selected hatch's base Vec3 first double (`hatch+8`), **not** its spawn
  word. For the authenticated `MM_CLIMB_002.LVL`, selected index0 and serialized
  source yaw32 are at offsetsA0C71 and9CC69. The existing parsed cylinder has96
  rows. Zoom resets0; fresh-controller elevation is0.
- Base yaw32, height96 and distance377.516 describe the **settled** view.
- 45C9AB copies the intro-enabled flag and zeroes an independent intro clock.
  Retained pose builder45CC09 applies, for active clock `t`:
  - `q=t/2`; if `q<1`, subtract `350-(2-q)*q*350` from localZ **after** its
    ordinary zoom division. Thus initial distance is727.516, not377.516.
  - `u=(t-1)/2`; for positive `u`, add `2*u*u` turns if `u<0.5`, otherwise
    `2*((2-u)*u-0.5)` turns. Wrap while phase>1, not >=1.
  - Compute this frame's pose using the **old clock**, then advance only at
    UI state0. Disable when the new clock is **greater than3**, not equal3.
- At t0/1.5/2/3, equivalent unwrapped pose yaw is32/64/160/288. At3 it is back
  to the base orientation. Controller yaw itself need not change.

The selected-hatch model callback and general serialized model/list extents
remain partial. This is not a universal all-level camera metadata parser, nor
new runtime observation of the source flag or clock in the earlier screenshot.

## Production integration

`CameraIntro` owns the animation clock; `CameraParams` carries a disabled-by-
default pose modifier. `buildCamera` applies the recovered equations only while
active. Canonical fixture arithmetic and direct `--level`/`--sim` commands remain
static as before.

Natural `--frontend` entry into **MM_CLIMB_002**, without an explicit `--camera`
override, now arms the opening move.
The [loss-results follow-up](loss-results-quit.md) gates this on next-load intro
preference: Retry disables it; map selection restores it. Other levels retain their prior diagnostic
startup behavior until their selected-hatch yaw is loaded correctly. Frontend
camera height uses the loaded terrain/cylinder row count, not a universal96.
Pause freezes the intro; picking uses the last displayed camera basis instead of
the unanimated controller basis. Existing simulation timing and camera-input
update order are otherwise unchanged and are not promoted to native fidelity.

## Checks

- Asset-free `cameraintro` checks the seven half-second poses from0 through3,
  unscaled zoom offset, elevation, strict phase/end boundaries, pause/reset and
  disabled-pose equality. Invalid elapsed input handling is SOURCE hygiene.
- Actual `frontendentry` still exercises production menu→map→level. Its30th
  frame equals an **independent static-pose equivalent** at29/144seconds, supplied
  through the existing camera-file path. The expected path never invokes the
  intro branch. One simulation tick and the diagnostic tick-reset checks remain.
- Five isolated mutations fail: omitted pose animation; no frontend arming;
  increment before rendering; advancing while paused; zoom-scaling the extra350.
  The three edited production files were restored against recorded SHA256 values.

Restored full build/CTest: **56/56 PASS**,98.28seconds; `frontendentry`18.28seconds
(`build/tmp/enacter-camera-start/final-tests.log`). Fresh C1 zoom1/elev_down
renders are byte-identical to the preceding passing fixtures (`c1-regression.log`),
so the new disabled-by-default path does not alter those accepted images.

Native visual acceptance remains pending a **clock/pose/tick-matched** capture.
The earlier entry screenshot had none of those values, so we do not assign it an
intro time, tune the clock to it, or claim this correction explains every pixel.
Clouds, HUD/cursor presentation and other renderer gaps remain independently open.
