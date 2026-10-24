# C1u: icon and glyph footprint, scored before any new drawing (directive part 1)

Queue `c1u-footprint-mask`. Offline. No stock start. Dispatched 2026-10-24 14:02 MDT. Worker [C1 footprint](d937328a-0504-4e17-a2ac-9a212dced90c). `prediction.md` 14:07:28, score 14:09:58.

## Result

The fixture `tools/harness/fixtures/c1-footprint/footprint.json` holds ten icon rectangles and eighteen count-glyph rectangles. `plates` is `[]`. SHA-256 `ea9d5cff5377c2f39183b9ef1bc10597bffd3b7bd7123b217f78d5ea72de29a3`.

Icon edges use the C1s `0x47A75D` arguments, the C1l frame `dx/dy/width/height`, and the C1t placement: `left = x - dx*scale`, `top = y - dy - height*scale`, `right = left + width*scale`, `bottom = top + height*scale`, with `dy` unscaled. Scale is 0.5 for icons 0, 1, 3, 4, 5, 6, 7 and 1 for icons 2, 8, 9. Glyph edges are the corner doubles in the C1s quad dump: digit 0 is `[66, 72)` by 14 rows and digit 1 is `[72, 78)`, at y = 20, 60, … 340. Icon 9 has no quad.

A pixel is inside when its square `[x, x+1) x [y, y+1)` intersects the rectangle. That rule is in the fixture, written before the score. It is the committed raster convention, not a read of the rasterizer. Union count is 5133.

| View | Image | Numerator | Pixels | Denominator | MAE |
| --- | --- | ---: | ---: | ---: | ---: |
| `initial` | `render.png` | 880240 | 5133 | 3926745 | 0.22416530739836685 |
| `initial` | `no-panel.png` | 1302522 | 5133 | 3926745 | 0.3317052673397432 |
| `a7q_mm_dig_000` | both | — | 5133 | — | reference-unnamed |

`initial` is compared with `tools/harness/fixtures/camera_a1/original-0.png`. Both figures are above 0.02. Not tuned. `MM_DIG_000` has no file this round would call the stock frame. `baseline.png` in the A7q directory was not used.

Plate rectangles are still empty and join this fixture before part 3. These two MAEs are the pre-plate baseline.

## Facilitator check

An overlap loop that does not import `score_c1_footprint.py` reproduced 5133 pixels, 880240 and 1302522. Icon edges recomputed from `build/tmp/c1l-icon-live-read/readings.json` and the four identical frames in `hook-output.txt`: 0 mismatches. Glyph doubles unpacked from `raw-bytes.json` at `+0x20/+0x88`, `+0x28/+0x160`, `+0xF0/+0x158` and `+0x90/+0xF8`; both snapshots match; 0 mismatches against the fixture.

Re-read `0x47A7BB`: `fild [ecx+0x34]`, `fsubr [ebp+0x10]`, before the scale subtract at `0x47A7D2`. The `-1` arm at `0x47AA53` divides `+0x30`, the stored `+0x3c`, and `+0x38` by the 2.0 at `0x4B03F8`. `0x47AB2A` is `fsub [ebp-0x10]`. The scale-0 path from the compare to that subtract was not re-walked.

One mutation: icon-0 `right` 46.5 changed to 40 and nothing else. The initial render MAE moved to 0.22229241736471836. Both files restored; the fixture hash matches the value above.

No Lemmings process. No install write. The renderer was not rebuilt for this score; the C++ sources are unchanged. `ctest --preset dev-debug`: 34/34 passed.
