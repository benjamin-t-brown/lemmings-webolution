# A1 outer-route check: one walker, two passes, and a sprite-path correction

This follows the [projection/cache replay](phase-a1-projection-cache.md). The facilitator
independently re-read the cited bytes and call graph. **A1 remains open.** No camera,
threshold, ROI, or production renderer changed.

## Verdict

- **The outer tube skin is not `0x47D0C0` and not a third emitter.** It is drawn by the same
  per-frame world surface walker `0x43A4EC` that draws the selected cell, through the sibling
  routes `0x462404`, `0x4623A5`, `0x462465`. The selected inline-cell route is `0x4622FD`.
- **All four routes funnel through the same projection chain:**
  `0x462590 -> 0x4599A0 -> 0x4599C0 -> 0x46D372 -> 0x46DDF0 -> 0x46D3C1 -> 0x46D460`.
  The facilitator re-read every direct callee edge in that chain.
- **`0x47D0C0` is the sprite/billboard path.** It is slot +4 of the four-entry table at
  `0x4B1AD8` (`10 b6 47 00 | c0 d0 47 00 | 3f b7 46 00 | 4c b4 47 00`), has no direct call
  site (`xrefs` returns only `0x4B1ADC`), builds four corners from half-extents
  `+0x320/+0x328/+0x330`, projects with the billboard matrix `0x58D438`, averages the four
  screen coordinates, and calls `0x47A565 -> 0x440C50` with eight coordinates. It never
  touches `0x58D2B0` or `0x46DDF0`.
- **One projection formula, two per-pass bias values.** `0x43A6C0` subtracts
  `0x4B1460 = 0.0003` before the INN pass; `0x43A760` adds it back before the three OUT
  passes. Outer routes therefore submit `SZ = preZ`; the inner route submits
  `SZ = preZ + 0.0003`; `RHW = f32(0.25 / SZ)`. Projected XY uses `1/preZ` and is unaffected.

The facilitator verified the bias bytes directly:

```text
0x43A6C0  dd 05 b0 d2 58 00 dc 25 60 14 4b 00 dd 1d b0 d2   ; FLD [58D2B0]; FSUB [4B1460]; FSTP [58D2B0]
0x43A760  dd 05 b0 d2 58 00 dc 05 60 14 4b 00 dd 1d b0 d2   ; FLD [58D2B0]; FADD [4B1460]; FSTP [58D2B0]
0x4B1460  61 32 55 30 2a a9 33 3f                              ; f64 0.0003
```

## Correction

The feasibility note's tentative "outer-skin route `0x47D0C0`" is withdrawn. It is the
sprite/billboard projector, not tube geometry. The outer skin is the sibling-route branch of
the world walker.

## Limits

- No live outer-pass capture was run; the outer-pass bias `0` is derived from the subtract/
  restore bracket and the retained `-0.0003` context value, not observed at a breakpoint.
- The radius split (402 vertices at ~50, 495 at ~60) is measured from the LVL by
  `build/tmp/a1-outer-route/check_shells.py`; the assignment of the outer vectors to radius 60
  is by elimination, not a live dump of the group vectors.
- Effective device depth/clipping state and physical near/far planes remain unsettled.
- Whether the bias choice contributes to the failed left view is a pixel question; the
  [sourced candidate](phase-a1-sourced-candidate.md) subsequently found the bias mutations
  pixel-invisible in that candidate.

## Reproduction

```text
python build/tmp/a1-outer-route/read_windows.py
python build/tmp/a1-outer-route/check_shells.py
python build/tmp/a1-outer-route/callers.py
```

Artifacts: `build/tmp/a1-outer-route/round-note.md`, `citations.json`, the disassembly
windows, and the re-runnable readers above.
