# A4d — the original tube route closes the nine-view gate

2026-10-19, branch `decompile`. **A4d checkpoint: tube-only fixed-ROI pass, not full-scene
fidelity.** Routing the A4c per-cell mesh through the `0x43A4EC` walker's real passes and the
`HI` cylinder tiles takes the nine retained views from **0/9 to 9/9** under the unchanged
gate. The A2 wider/cavity/rim and A3 sensitivity failures remain failed.

Authority: stock 640x480 `Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
No live game process was started or driven. REA was opened read-only and closed.

## Verdict

The route is the `0x43A4EC` walker:

| # | pass | call | buckets | bank | material |
| --- | --- | --- | --- | --- | --- |
| 1 | INN | `0x4622FD` | `c` down to `b` | `entry+8` whole-block layer-0 quad | `+0xC8 + 0x18` |
| 2 | OUT right | `0x462404` | `d-1` down to `mid+1` | `entry+4` (mesh4), reversed | `+0xC8` |
| 3 | OUT left | `0x4623A5` | `a` up to `mid-1` | `entry+0` (mesh0), reversed | `+0xC8` |
| 4 | OUT centre | `0x462465` | `mid` | mesh0 forward `coord < centre*8`, mesh4 forward `coord >= centre*8` | `+0xC8` |

Bounds use the stored extent `E` (the `.LVL` `f64 0.36`) and `0.7`, chopped toward zero:
`a = centre-E*32`, `b = centre-0.7E*32`, `mid = centre`, `c = centre+0.7E*32`,
`d = centre+E*32`, then shifted into `[0,32]`. The constant lattice wall is **not** part of
this walker and is replaced; keeping it alongside the route drops to 0/9.

Two new derivations were needed and are now produced from stored data:

- **INN inline quad:** `entry+8`, gated by `entry+0xF8` (the same flag as the A4c `wall`
  flag), is the block's layer-0 rectangle `(x0,y0) (x0,y0+16) (x0+8,y0+16) (x0+8,y0)` with
  the open-top UVs and white colour. 98/98 match the retained arena; no captured bytes are
  production input.
- **Child coordinate:** the two 4-byte child records per quad hold the solid rectangle's
  `x0`; all **776** values match the retained arena. The centre filter partitions mesh0 and
  mesh4 on that coordinate.

## Texture variant

The archive stores plain and `HI` tiles for every cylinder cell. At identical geometry the
plain variant is **0/9** (`0.028511432..0.035698774`); the `HI` variant is **9/9**
(`0.011692744..0.015679210`). The retained originals were captured with `HI`, so production
loads `HI` by default. **The original loader's own selection convention is inferred, not
byte-read:** the pixel discriminator is decisive at this gate, but no instruction carrying a
`HI` suffix was found.

## Nine-view scores (unchanged gate)

640x480, ROI `[100,80,280,410]`, normalized MAE `<= 0.02`. The facilitator re-rendered all
nine from the current build and recomputed every value from the PNGs and retained originals:

| view | route (HI) |
| --- | ---: |
| initial | 0.015584142 PASS |
| mouse | 0.016047116 PASS |
| right | 0.014810480 PASS |
| left | 0.015679210 PASS |
| canonical | 0.015586607 PASS |
| zoom 1 | 0.012003521 PASS |
| elevation up | 0.011692744 PASS |
| elevation down | 0.012866662 PASS |
| zoom-0 elevation input | 0.015869215 PASS |

`wall low` and `cellmesh low` still reproduce the A4a/A4c tables exactly, so the rest of the
renderer is unchanged. The production route frame matches the A2 verifier's default candidate
(`build/tmp/a2-tube/verify/sourced-candidate-default-initial.png`) at **2.6037e-5** full-frame
MAE.

A2 diagnostics: cavity `0.030728457` (A2 `0.030728416`), upper rim `0.400295646` (identical),
changed-pixel mask 179,765 px at `0.022041013` (A2 `0.022030839`). **All three still fail**
against `0.02`; the 9/9 is the fixed ROI only.

## Independent verification

- `cmake --build --preset dev-debug` clean; **18/18 CTest pass**.
- `python tools/harness/verify_cellmesh_route.py`: **24 route instructions + 5 stored
  doubles** re-read; 98 inline quads, 776 child coordinates, 0 flag/inner mismatches; the
  initial ROI/cavity/rim/mask report matches the numbers above.
- One-term route mutations at the same build: swapping mesh0/mesh4 drops to **6/9**;
  disabling INN drops to **6/9** (initial `0.016960124`); plain tiles and keeping the
  constant wall drop to **0/9** (initial `0.028511432` / `0.031523668`). Re-rendering the
  default after the mutations reproduces the baseline PNG hash.
- **Honest negative:** inverting both centre filters leaves the ROI score identical on all
  nine views (frames differ outside the ROI). The centre filter is READ and its child-coordinate
  input is MEASURED, but this gate does not discriminate it.
- Rebuilt after a comment-only cleanup; no live process remains; the installed exe hash is
  unchanged; `git diff --check` is clean.

## Limits

- **`MM_CLIMB_002` only.** No all-level sweep.
- The wall is replaced on this route. The stored constant lattice remains only for the
  diagnostic `wall`/`cellmesh` paths; it is not part of the original walker.
- `world+0x408 == stored 0.36` and the `0x43C990` selector = camera turn fraction are
  **inferred**; both are corroborated by the nine-view pass.
- The material descriptor's UV offset is `0,0` on this level; non-zero offsets are untested.
- The door path (`0x400` cells, `R x 12`) and the `entry+0xB8` sub-object path are
  implemented/read but unexercised here (`R == 0`, all `+0xB8` zero).
- Omitted scene remains: background/sky/sand/hose, upper gold band, entrance, exit, lemmings,
  HUD. The A2 omitted-scene inventory stands.
- No full-frame fidelity, all-level, or sprite/actor claim.

## Next

The tube submission now matches the original at the fixed ROI. The next A4 work is the SDL
window/event loop plus the verified A1 revolve/zoom controls, then the preserved simulation +
A3 sprite selection/actor rendering, then the omitted scene. The door and `+0xB8` paths, the
centre filter, and the all-level sweep stay open.

## Reproduction

```text
python tools/harness/verify_cellmesh_route.py
python tools/harness/generate_cellmesh.py --compare
.\scripts\Invoke-Ucrt64.ps1 "cmake --build --preset dev-debug"
.\scripts\Invoke-Ucrt64.ps1 "ctest --preset dev-debug"
python tools/harness/score_a4_views.py --label route-hi --geom route --render
python tools/harness/score_a4_views.py --label route-low --geom route --tile-detail low --render
```
