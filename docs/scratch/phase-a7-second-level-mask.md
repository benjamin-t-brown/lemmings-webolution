# A7q: second level passes, no camera input

2026-10-21. One stock start of `MM_DIG_000`, pid 21464. Dispatch: Claude Code
`round-agent`, session model (no pin). Round 1 of 3 on "does the amended
wider-mask gate generalize to a second level." Reused A7p's capture path
(entry, `select_level` over level-table slot `[0][0]`, 14 s unpaused walk,
pause) unedited, substituting the level name.

## Offline preflight

Passed with zero reasons: all files present, launcher keeps stderr/exit,
install hashes match the A7n/A7p authority set, all five prologues read
`0x55`, `MM_DIG_000.LVL` present. The params-derivation self-check
reproduced `build/tmp/a1-zoom-elevation/params-canonical.json` on all 16
renderer-read fields (zero mismatches) from A7p's own saved
`start-check/camera.bin`. This needed two reads beyond the brief's cited
five lines of `render_camera_skin.py`: the camera's own `aspect` and
`angle_turn_parameter` fields, at offsets 288 and 280 of the 360-byte
camera block at `0x4CBFC8`, located by matching the exact values in
`tools/harness/fixtures/camera_a1/reference-state.json` for the same camera
hash. `render_camera_skin.py`'s own fixed-fixture constants for those two
fields are a labelled bounded hypothesis, not the production renderer's
input contract (`src/app/Camera.cpp` reads `aspect`/`angle_degrees` from
the JSON directly).

## Live run

Loaded path read back `Compiled\MM_DIG_000.lvl`. The countdown dword at
`0x4CBAAC` read 300 before the walk and 286 after (whole seconds). There is
no established reference camera hash for `MM_DIG_000` (round 1 on this
level); the post-walk hash was read and logged:
`b6be6e3dd82ee95bb425992d6ab00cbba49337df994cbd089e16f91d6e46524a`. The
live row count at `0x004C6E28` read 96, the same value as `MM_CLIMB_002`
(a two-level observation, not yet a rule). With no camera input, the
baseline/suppressed/restored mask pair captured cleanly.

## Score

Mask: pixels where `baseline.png` differs from `no-cell-routes.png`
(175498 raw), minus the caption panel `[277,422,363,454)` (2210 pixels) and
the ten skill-panel bands (86 pixels, 3646 error units) — the same amended
exclusion used for A7p's accepted gate.

| numerator | denominator | N | MAE | pass |
| ---: | ---: | ---: | ---: | --- |
| 2606399 | 132499530 | 173202 | 0.019671005625453916 | yes |

`numerator * 50 = 130319950 <= 132499530`. Mask bbox `[87,24,640,480)`. 4
mask pixels fall inside an informational, non-excluded timer-area box built
from a *placement argument READ* (`docs/scratch/phase-a7-draw-loop-caption-bridge.md`,
operands `560/430`), not a measured rectangle; they stayed in the score, as
the brief required.

## Left open (worker)

- Only the no-camera-input case was tested; camera movement (zoom,
  rotation, elevation) on a second level is unmeasured, and A7p's own
  `zoom1` failure shows the renderer does not always generalize within one
  level either.
- `world_rows == 96` on both levels checked so far is a two-level
  observation, not a rule.
- The timer-area box is informational (a placement-argument read), not a
  candidate exclusion.

## Facilitator check

Independent re-score (`build/tmp/a7-review/a7q/verify.py`, separate code
from both the worker's `capture.py` and from `build/tmp/a7-review/a7p/rescore.py`,
reading only the three saved PNGs) reproduces every number exactly:
numerator 2606399, denominator 132499530, N 173202, MAE
0.019671005625453916, pass, mask bbox `[87,24,640,480)`, raw changed
175498, caption-excluded 2210, skill-excluded 86/3646 error units, and the
same 4 timer-area coordinates. A 6px x-shift of `render.png` alone (no
other change) raises the numerator to 5360684 (MAE 0.0405, fails),
confirming the checker discriminates rather than passing at any value.

Re-read against the executable: SHA-256 of the installed exe is
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`
(matches the authority used throughout A1/A7). The five prologues
(`0x4622FD`, `0x4623A5`, `0x462404`, `0x462465`, `0x43A4EC`) all read `0x55`
in the installed exe, confirmed independently (not just from the worker's
own logs). Camera-block offsets 280 and 288 were independently decoded
against `tools/harness/fixtures/camera_a1/reference-state.json`'s
`raw_hex` for the `7e6e1a8a…` camera and match `angle_turn_parameter`
(0.052861109375953674) and `aspect` (1.3333333730697632) exactly — the
worker's offset claim is confirmed, not merely re-asserted. The operand
addresses in `derive_params.py`'s `OPERANDS` dict are byte-identical to the
already-established addresses in `build/tmp/a1-zoom-elevation/build_params.py`.

Build: `ninja` had no work (research-only round, no `src/**` changes);
fresh `ctest --preset dev-debug` is 24/24. `git status --short` on the
tracked tree shows only `docs/tasks.json` changed. Live side: installed exe
hash matches the authority; no `Lemming`/`webolution` process and no
`LemWeb-*` scheduled task remain (checked independently of the worker's own
`cleanup_check.py`, which agrees).

**A7's amended gate now passes on this independent re-score**: all nine
`MM_CLIMB_002` views (A7p) plus this second level (A7q), caption panel and
ten skill-panel boxes excluded, all `<= 0.02`. This is the standing run
`a7-complete`'s stop condition "the milestone gate passes on the
facilitator's own independent re-score" (`docs/ORCHESTRATION.md#standing-run`).
The standing run stops here; no further A7 rounds are queued. Camera
movement on a second level, and whether `world_rows` is level-independent
in general, are explicitly left open, consistent with the gate's own
wording (one second level, not nine views on it).
