# A2 tube addressing, routing, and omitted scenery — facilitator check

Authority: stock 640x480 `Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Branch `decompile`. This finishes the interrupted `a2-tube-render` round from
its on-disk artifacts; the facilitator did not redispatch the original brief.
An optional read-only explore agent supplied scenery leads, not verification.

## Verdict and scope

**The declared A2 fixed-ROI tube checks and scenery inventory are complete.**
All nine retained camera views pass the unchanged gate, and an additional exposed
inner-wall region passes. The fixture now uses the executable's two OUT mesh
banks and center filter, source-derived texture rows, and INN-before-OUT passes.
This is a captured-level Python fixture; production implementation remains A4.

**Full-scene fidelity is not accepted.** The additional changed-pixel diagnostic
fails at `0.022030839` against `0.02`. The cavity and upper-rim diagnostic
rectangles also fail. Those failures are retained below, not explained away or
used to relax a threshold. A4 must carry them forward while adding the omitted
scenery, objects and sprites. A2 does not establish all-level geometry generation,
device-state equivalence, or full-frame pixel agreement.

## Reproduction

```text
python tools/harness/verify_tube_a2.py --bytes-only
python tools/harness/verify_tube_a2.py --render
python tools/harness/verify_tube_a2.py --live build/tmp/a2-tube/live/durable
python tools/harness/render_tube_fixture.py --case initial --label example
```

Normal Windows Python, with Pillow and Capstone; do not prepend UCRT64 bin.
All payloads and generated PNGs stay under ignored `build/tmp/`. The fixture
requires the retained A1 cell capture, true mesh arena, decoded texture banks,
camera fixture, and zoom/elevation captures listed in its source. It fails on
missing/mismatched data rather than substituting guessed geometry.

The live replay tool is `tools/harness/probe_tube_a2.py --pid <owned-pid>
--output build/tmp/<new-directory>`, run from a hidden worker with the stock game
already paused on the retained MM_CLIMB_002 camera. It leaves that process alive
and paused; the A2 launch driver owns process termination and wrapper restoration.

## READ: texture addressing and the brief's correction

For angular bucket `g` and vertical band `b` with six bands:

```text
col = g / 4                  (integer division)
row = (6 - 1 - b) / 2        (integer division)
OUT descriptor = col + row*8
INN descriptor = OUT descriptor + 24
```

The original brief's `(b//2)*8 + g//4` formula is wrong. Its middle two bands
agree by coincidence. The independent scan reads the actual cell `+0xC8` words:
the flipped formula agrees on **192/192** cells; the unflipped one disagrees on
**128/192**. The renderer derives the material index and checks it against the
captured word, rather than treating its own formula as an expected answer.

Selected initialized bytes, all independently re-read by `verify_tube_a2.py`:

| Address | Bytes | READ |
| --- | --- | --- |
| `0x43A43A` | `c1 e1 03` | loader argument is bucket*8 |
| `0x4608C5` | `8b4508 99 83e207 03c2 c1f803 99 83e203 03c2 c1f802` | signed /8 then /4 |
| `0x460A10` | `83e801 2b45fc 99 2bc2 d1f8` | (bandCount-1-band)/2 |
| `0x460A1B` | `8b4df8 8d34c1` | add row*8 |
| `0x460A2D` | `89 b0 c8 00 00 00` | store material at cell+0xC8 |
| `0x43AB3D` | `81 c1 4c 04 00 00` | INN name at world+0x44C |
| `0x43AB68` | `8d 44 ca 18` | INN descriptor col+row*8+24 |
| `0x43AB9F` | `81 c1 50 05 00 00` | OUT name at world+0x550 |
| `0x43ABCA` | `8d 04 ca` | OUT descriptor col+row*8 |

The name association is also retained in the earlier bounded live read at
`tools/harness/fixtures/cylinder_draw/live-left-read.json`: `+0x44C` is
`INN_MM_CLIMB_002`, `+0x550` is `OUT_MM_CLIMB_002`. This round re-read the
registration instructions and all cell indices, not every texture object's name
in a new process. All file citations use PE raw sizes; initialized `.data` ends
at `0x4C6000`. The audit rejects BSS addresses.

## READ: pass order and actual OUT routing

The source walker is `0x43A4EC`. Its call order is:

1. INN `0x43A754 e8 a4 7b 02 00` -> `0x4622FD`, with material `cell+0xC8 + 24`.
2. OUT right side `0x43A7F4 e8 0b 7c 02 00` -> `0x462404`, using cell pointer `+4`.
3. OUT left side `0x43A87C e8 24 7b 02 00` -> `0x4623A5`, using cell pointer `+0`.
4. OUT center `0x43A8FA e8 66 7b 02 00` -> `0x462465`, filtering both banks.

The INN bias bracket is independently asserted: `0x43A6C0` subtracts the
binary64 `0.0003` at `0x4B1460`, and `0x43A760` adds it back before OUT.
This establishes the order by **READ**; reversing the passes survives the
pixel test because the raster's depth test hides the order change.

The OUT side routes traverse their arrays backward. At the center bucket,
signed low shorts from each 4-byte child record are compared against normalized
camera center*8: `+0` keeps `<`, `+4` keeps `>=` (`0x4624B4 0fbf00`,
`0x4624C5 7408`, `0x462528 0fbf10`, `0x462539 7508`). Mesh objects start their
quads at `+0x10` (`0x41364A 83c010`) with stride `0x80`
(`0x4136CA 0580000000`). Both banks are decoded from the true live arena,
not the old one-allocation-shifted windows. The arena SHA remains
`f96bcf7ff21a4ef6f28f0e0f9502444006de4cb5e924f09465f549de20d2ad0e`.

The initial fixture submits 259 nondegenerate OUT quads and 44 INN quads, dropping
none. The old all-mesh0 candidate rendered many unselected quads. Its fixed-ROI
pixels are identical, so that region cannot independently pin the OUT route
choice. The corrected routing is source-derived and the limitation stays explicit.

REA was used for the world walker and scene call structure, then corroborated
with raw instruction reads. Walker Evidence ID:
`ev_952cead401d54d8c2fb563c1f0386bf4a656c616f9075f4bc09c0a7b9a99fdf5`;
scene Evidence ID:
`ev_8c1fab6641da71bfa7ed7758c9b30b2f868e3af7397919869af201efb48e5adc`.
No generated pseudocode was copied into the implementation. The REA session was
closed after the investigation.

## Pixel checks and isolated survivors

Unchanged gate: 640x480, ROI `[100,80,280,410]`, normalized mean absolute RGB
error `<= 0.02`. Predictions are in `build/tmp/a2-tube/facilitator-prediction.md`,
written before the new renders and before the live isolation.

| View | Source-walker default | INN disabled | Unflipped texture rows |
| --- | ---: | ---: | ---: |
| initial | 0.015574261 | 0.016950243 | 0.023557822 |
| grabbed mouse | 0.016047116 | 0.016047116 | 0.022929205 |
| right | 0.014810480 | 0.014810480 | 0.020366937 |
| left | 0.015662727 | 0.029356902 | 0.032037191 |
| canonical | 0.015576704 | 0.016952686 | 0.023555402 |
| zoom 1 | 0.011949253 | 0.070724742 | 0.014237033 |
| elevation up | 0.011645672 | 0.039570608 | 0.071148742 |
| elevation down | 0.012860831 | 0.018475386 | 0.061576198 |
| zoom-0 elevation input | 0.015859752 | 0.017238991 | 0.023778504 |

The separately declared exposed inner-wall rectangle `[205,80,275,300]` at zoom 1
passes at `0.011068330`. The retained view already exposes INN; a new camera view
was not necessary to test that part of the brief.

- Bank swap fails all nine views (`0.128219405..0.153384102`).
- Removing INN fails left, zoom1 and elevation-up; six views survive.
- Reverting the texture-row flip fails eight views; zoom1 survives.
- `all` vs `selected` INN and all-mesh0 vs source-walker OUT survive the fixed
  ROI in all nine. They change pixels elsewhere; do not label them protected by
  that ROI. OUT all0 is full-frame identical at elevation-down only.
- OUT-first vs INN-first is full-frame identical in all nine software renders.
  The order is accepted as a byte read, not a discriminated pixel parameter.
- After each of six isolated options at each of nine views, a fresh default render
  matches its baseline PNG byte-for-byte: **54/54 restores**. The verifier checks
  the saved restore hashes again when rescoring without `--render`.

## MEASURED: the upper gold band is outside the tube walker

Owned stock process PID 17228, `Compiled\MM_CLIMB_002.lvl` read back, paused
(`ui_state 1`), camera byte-equal to the retained reference. The durable replay
temporarily changed `55` (PUSH EBP) to `C3` (RET), first only at `0x43A4EC`, then
at the four cell route entries as a second structural experiment. Every byte was
logged before application, restored, instruction-cache-flushed and re-read.

The region `[100,0,540,12]` remains **pixel-identical** with the whole walker
disabled. The tube ROI changes by `0.341672740`; suppressing the four cell routes
changes it by `0.338286217`. Both ROI and upper band restore to error `0.0`.
This corroborates a separate scene contribution for the upper band in this frame;
it does **not** identify the exact model, texture, or function drawing it.

Full restored screenshots are **not** byte-identical: the difference is bounded
to `[303,235,339,277]`, over the animated hatch/count area. Do not describe this
as full-frame restoration. Code bytes and camera bytes restore exactly.

The first launch attempt failed at menu-highlight acquisition before any camera
or code write. It was stopped and the wrapper restored. The successful driver
moved the frontend cursor away from the menu and reacquired actual focus before
input batches. All attempts and the earlier successful isolation are preserved
under `build/tmp/a2-tube/live/`; only the durable capture above supplies the final
headline numbers.

## Omitted-scene inventory and retained failed diagnostics

| Visible omission | Evidence / disposition |
| --- | --- |
| upper gold band | survives complete walker suppression; exact resource unknown |
| sand ground, sky/grass, large green hose and base surround | visible in original `live/durable/no-walker.png`; model texture references `Models\bg_sand_medium\sand_1`, `sky`, `hose` are read from the LVL, but texture-to-pixel assignment is unproven |
| hatch model, its animated door/count; exit balloon/basket | visible in retained originals; counted `Entrance.a3d` and `Exit.a3d` strings in the level; not in this tube-only fixture |
| lemmings | visible actors omitted; no billboard was used to infer actor coordinates |
| skill panel, counters, cursor, pause banner, timer | original HUD/UI omitted; Phase C scope |

The wooden platforms, carved stair faces and the gray stone inside the cutaway
are already in the captured tube render. Do not file them as absent scenery.
The inventory does not claim a complete parser for the background model.

Additional regions declared before scoring retain these failures:

- Initial cavity `[280,80,430,290]`: **0.030728416**, fails.
- Initial upper rim `[100,0,540,35]`: **0.400295646**, fails. Part of it is the
  separately observed gold band; no claim that this explains every residual pixel.
- Deterministic mask of all **179765** pixels changed by the original's four-cell-
  route suppression: source-walker candidate **0.022030839**, fails. The older
  all0 geometry scores **0.024106054**, also fails. Missing objects and sampling
  differences are possible contributions, not an independently measured complete
  explanation. This wider diagnostic remains open for integration.

## Cleanup and facilitator verification

The facilitator independently parsed PE raw sizes, checked 28 instruction pins
and five constants plus the format strings, read all 192 retained material words,
implemented the source-derived route replay, rendered the nine-by-six isolation
matrix, recomputed original-image errors, and ran the live suppression/restore
experiment. The read-only explore map supplied no accepted measurement.

All live camera/code writes and the level-table entry swap were restored.
The input shim was released. The owned game was stopped; no debugger or hardware
breakpoints were used. Executable, save and wrapper hashes match the starting
snapshot (`32675bdb...`, `f7f86039...`, `d639e220...`). The full hashes, writes and
readbacks are in `live/live-isolation.json`; durable probe results and PNG hashes
are in `live/durable/isolation.json`.

Build succeeds; **14/14 CTest tests pass**, 58.25 seconds. All three new Python
harness scripts compile, the byte/pixel/live replay verifier passes its declared
gates, and `git diff --check` is clean. Additional diagnostic failures above
remain failures despite those green checks.

**At this A2 checkpoint, next was A3 sprite-view selection**, then A4 production integration.
The [later A3 check](phase-a3-sprite-view.md) completes bounded selection verification
and hands off to A4. Carry the
failed broader diagnostics and omitted-scene inventory into A4; the fixed-ROI
passes do not replace those outstanding checks.
