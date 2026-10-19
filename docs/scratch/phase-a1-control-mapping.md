# A1 control mapping: one model reproduces every tested series

This follows the [depth/clipping check](phase-a1-depth-clipping.md). **A1 is now open only for
the zoom/elevation branches.** The camera was restored to the retained reference block
`7e6e1a8a77c29f860f82dd8cde3f60c9f89ecf3f1352d3c2a6801cc9867d0fff` and re-verified at finish.

## Verdict

One model reproduces **473 live points with 1 mismatch** (a 1-second mouse drag; max
`|residual| = 2.9e-4`). Every other point is within `1e-4` (float32 storage rounding). Writing
the canonical level-start control fields and letting one play/pause settle reproduces the
retained reference camera block **byte-for-byte**.

## Mapping (measured unless marked READ)

- **Bindings:** storage `0x005D58B0 + action*8`; 23 actions. NUMPAD4 (action 16, scan `0x4B`)
  rotates left; NUMPAD6 (action 17, scan `0x4D`) rotates right. LSHIFT (action 5, scan `0x2A`)
  multiplies the key step by 3. BACKSPACE (action 6, scan `0x0E`) runs the 180-degree flip.
  SPACE / LCTRL / RCTRL toggle zoom and apply the one-frame edge nudge.
- **Yaw:** NUMPAD4 `+1.000`/frame, NUMPAD6 `-1.000`/frame, LSHIFT `x3`; mouse dx
  `-0.4166667*dx` (gain `10.0` at `0x004C0398`, divisor `24.0` at `0x004C0388`).
- **Elevation:** mouse dy `-0.500*dy` at zoom 0 (`12.0` at `0x004C03A0`), `-0.125*dy` at
  zoom 1.
- **Not grabbed:** keys move the virtual cursor `+-4.800`/frame; mouse moves it by
  `(2.0*dx, 2.4*dy)`; yaw/elevation unchanged.
- **Grab gate (READ + measured):** `mode(control+8)==2`, right button down (swap 0), and not
  (`control+0x1890 != 0` and `control+0x16B3 == 0`). Over empty scenery it engages on the
  first frame; while an object is attached it stays off and the keys move the cursor.
- **Rates:** key `1 unit/frame` at ~143.9 frames/s (`~144 units/s`, frame-rate dependent);
  mouse `0.4167 units/dx` (`~240.5 units/s` at 580 dx/s, input-rate dependent); flip `128`
  units over `0.384 s`.
- **Clean negatives:** mouse wheel (`FUN_00455E70` returns 0), mode 1, zoom outside `{0,1}`,
  `DAT_0058CDAD` free-cursor mode, attached-object branch — not input-reachable in
  `MM_CLIMB_002`.

## Limits

- The gameplay **write site of `control+0x10`** is not located; the 0<->1 toggle and its
  one-frame lag behind `+0x14` are measured, not read.
- The gate latch across a cursor move onto an object is READ only; the swap=1 branch was not
  exercised (it needs a config-global write).
- The flip animation does not advance while `ui_state == 2` (in-level menu); ui 0/1 animate.
  The suppressed camera path is not isolated.
- Only camera-field restores were written; each is logged. No production code changed.

## Reproduction

```text
python build/tmp/a1-control-mapping/predictor.py
```

Artifacts: `build/tmp/a1-control-mapping/round-note.md`, `mapping-table.json`,
`live-series*.json`, `predictor-residuals.json`, `final-checks.json`, `predictor.py`.

**Next A1 item:** the zoom/elevation branches — read the pose builder's zoom/elevation
scaling, capture the original at zoom 1 and elevation deltas, and accept those frames under
the fixed criterion.
