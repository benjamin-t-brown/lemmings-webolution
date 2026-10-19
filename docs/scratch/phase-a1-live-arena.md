# A1 live mesh arena: the pixel gate passes

This follows the [diffuse/decode check](phase-a1-diffuse-decode.md). The fixed criterion is
unchanged: 640x480, ROI `[100,80,280,410]`, normalized mean absolute RGB error `<= 0.02`.

## Verdict

A candidate using the **true live mesh0 arrays** from the arena at `0x4ECB00` passes every
retained view. The round's prewritten prediction (that it would improve but still fail
`initial` and `left`) was falsified.

| Candidate | initial | mouse | right | left | Pass |
| --- | ---: | ---: | ---: | ---: | --- |
| Retained best `m0min-diff` (reproduced bit-exactly) | 0.021467353 | 0.016047116 | 0.014810480 | 0.023751700 | nYYn |
| **`live0-diff` (true live mesh0 + diffuse)** | **0.015574261** | 0.016047116 | 0.014810480 | **0.015662727** | **YYYY** |
| `live0` (true live mesh0, diffuse off) | 0.017450078 | 0.016047116 | 0.014810480 | 0.018642900 | YYYY |

The facilitator independently recomputed all eight ROI errors from the candidate PNGs and the
retained originals; they match exactly. The retained `m0min-diff` PNG hashes and errors also
reproduce bit-for-bit.

## What was captured

- One owned stock process (PID 39284), module base `0x400000` at its preferred base, level
  `Compiled\MM_CLIMB_002.lvl`, `ui_state 1`, camera byte-equal to the retained reference.
- Read-only arena dump `0x4ECB00..0x506400`, **104704 bytes**, SHA-256
  `f96bcf7ff21a4ef6f28f0e0f9502444006de4cb5e924f09465f549de20d2ad0e`, re-read identical.
- 192 entries, **389 mesh0 / 387 mesh4 quads**. Every nonzero object has `kind == 2` and
  `end == obj + 0x10 + count*0x80`; every array hashes to the manifest value.
- **163/163** retained mesh0 windows have their first 16 bytes equal to the live mesh4 header,
  reconfirming the one-allocation shift. 38 of 163 cells have a genuinely different mesh0
  array from the retained window's payload.

## Isolated effects

- **Array source only** (`m0min-diff` -> `live0-diff`): initial `-0.005893`, mouse `0`,
  right `0`, left `-0.008089`. This is the correction that clears the gate.
- **Diffuse only** (`live0` -> `live0-diff`): initial `-0.001876`, mouse `0`, right `0`,
  left `-0.002980`.
- The live decode draws 387 quads / 774 triangles and drops none; the retained window decode
  drew 363 / 726 and dropped 26.
- `live-both` is pixel-identical to `live0` inside the ROI (mesh4 is depth-occluded there).

## Limits — A1 is not fully closed

- This is a **bounded offline raster candidate**, not the production renderer or a runnable
  executable. The entry-point debt (A4) remains.
- Near/far planes, guard band, scissor, blend/alpha state, and the per-bucket route selection
  are still unmodelled; the pass does not prove they are absent or irrelevant in general.
- The pass covers the four retained static frames. Other zoom/elevation branches and the
  complete input-to-camera mapping remain outside this pixel acceptance.
- The live arrays are from one level instance; no claim is made that the arena layout is
  identical in every level without a fresh read.

## Process and installation state

No `Lemmings Revolution.exe` or debugger remains. The installed executable is unchanged
(`32675bdb…`); `SavedGame.dat` (`f7f86039…`) and `dgVoodoo.conf` (`d639e220…`) match the
recorded hashes. The temporary windowed wrapper setting was restored. All injected input was
released; the camera block was read-only; the only game-memory write was the established
`select_level` slot swap, restored in `finally`.

## Reproduction

```text
python build/tmp/a1-live-arena/capture_arena.py --pid <owned-pid> --output build/tmp/a1-live-arena
python build/tmp/a1-live-arena/render_live.py --case initial --label live0-diff
python build/tmp/a1-live-arena/score.py --dir build/tmp/a1-live-arena/candidates --label live0-diff
```

Artifacts: `build/tmp/a1-live-arena/round-note.md`, `pixel-report.json`, `arena.bin`,
`live-arena-manifest.json`, `render_live.py`, `score.py`, and the candidate PNGs.

**Next:** A2, the tube — INN/OUT addressing, draw order, inner-before-outer, and the scenery
the original draws that the candidate still omits. Then A3, sprite-view selection.
