# C1w: ten plate boxes joined to the footprint, saved renders rescored

Queue `c1w-plate-footprint`. Offline. No stock start. Dispatched 2026-10-24 14:17 MDT. Worker [Plate boxes](3092565d-db9b-46da-bcfd-0997859714b6). `prediction.md` 14:19:05, fixture 14:20:37.

## Result

`tools/harness/fixtures/c1-footprint/footprint.json` now has the ten C1v boxes, x `[18, 90]`, y `[12+40*i, 12+40*i+32]`. Icon and glyph arrays are the C1u rectangles. SHA-256 `e96840e02c3046705c5d1dd19e947a47d7ea4aa65edbe34e31cccafd44b76892`.

The pixel rule is unchanged. The union is 23166 pixels.

| View | Image | Numerator | Pixels | Denominator | MAE |
| --- | --- | ---: | ---: | ---: | ---: |
| `initial` | `render.png` | 5171522 | 23166 | 17721990 | 0.29181384257636983 |
| `initial` | `no-panel.png` | 5593804 | 23166 | 17721990 | 0.315641979258537 |
| `a7q_mm_dig_000` | both | — | 23166 | — | reference-unnamed |

Both initial figures are above 0.02. Not tuned. These are the with-plates baseline. The C1u pre-plate numbers stay in that round's note. `^tab` pixels are still unread, so the scored rectangle is the box, not a proven oval.

`tools/harness/score_c1_footprint.py` now refuses an empty plate list or a list whose length is not 10, and writes `build/tmp/c1w-plate-footprint/score-table.json`.

## Facilitator check

Icons and glyphs compare equal to the C1u fixture at `HEAD` before this landing. All ten plate edges match `18, 12+40*i, 90, 12+40*i+32`. Icon-0 is still 35, 18, 46.5, 33. An overlap loop that does not import the scorer reproduced 23166, 5171522 and 5593804.

One mutation: plate-0 `right` 90 changed to 80 and nothing else. The initial render MAE moved to 0.29001710229161554. Fixture and score table restored; the fixture hash matches the value above.

No Lemmings process. The saved PNGs were not regenerated. No C++ change. `ctest --preset dev-debug`: 34/34 passed.
