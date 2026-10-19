# A1 sourced candidate: negative pixel result and a mesh-decode correction

This follows the [outer-route check](phase-a1-outer-route.md) and the
[projection/cache replay](phase-a1-projection-cache.md). **A1 remains open.** The fixed
criterion is unchanged: 640x480, ROI `[100,80,280,410]`, normalized mean absolute RGB error
`<= 0.02`.

## Verdict

The sourced candidate — actual captured radius-60 outer shell, per-pass submitted depth —
**does not pass**. Two retained views exceed the criterion.

| View | Sourced candidate | Pass? | Retained smooth-cylinder baseline |
| --- | ---: | --- | ---: |
| initial | **0.024898923879316037** | **no** | 0.01846389824167602 (pass) |
| mouse | 0.019481987632314429 | yes | 0.016010321075680553 (pass) |
| right | 0.017784665830417466 | yes | 0.01477834994828459 (pass) |
| left | **0.030445368719878525** | **no** | 0.033010761206839635 (fail) |

The facilitator independently recomputed all four ROI errors from the candidate PNGs and the
retained originals; the values match exactly. The retained baseline reproduction also matches
the retained PNG hashes and errors bit-for-bit.

## Isolated mutations

Each change was rendered alone, and the default was re-rendered byte-identically before the
next mutation.

| Mutation | initial | mouse | right | left | Pixels changed? |
| --- | ---: | ---: | ---: | ---: | --- |
| outer shell -> smooth cylinder | 0.018479 | 0.016094 | 0.014810 | **0.033014** (worse) | yes |
| outer context `0.0` -> `-0.0003` | 0.024899 | 0.019482 | 0.017785 | 0.030445 | **no, byte-identical** |
| inner context `-0.0003` -> `0.0` | 0.024899 | 0.019482 | 0.017785 | 0.030445 | **no, byte-identical** |

The per-pass bias is real in the executable (`0x43A6C0` subtracts `0x4B1460 = 0.0003` before
the INN pass; `0x43A760` adds it back before the OUT passes), but **pixels cannot see it in
this candidate**: the two shells sit at radii ~50 and ~60, so their `preZ` values never come
within `0.0003` of a depth tie. The depth test itself is load-bearing: disabling it gives
0.097375 / 0.071745 / 0.088432 / 0.095290. This is a null result for the bias, not evidence
against the mapping.

## Correction: the retained mesh decode was misaligned

`tools/harness/audit_a1_live_meshes.py` decoded every `*-mesh-quads.bin` from file offset 0.
The captured array actually begins at **file offset 16** (a 16-byte prefix, then
`count * 128`-byte records; the file is `count * 128` bytes, so the final 16 bytes were never
captured).

The facilitator independently re-ran the checks:

- **163/163** mesh0 files have their first 16 bytes byte-identical to the sibling mesh4
  vector header recorded in `facilitator-extra-meshes/manifest.json` (`header_u32`).
- Degenerate quads: **731/776** at the old base 0 versus **171/776** at base 16.
- At the old alignment almost every submitted triangle has zero projected area and is
  skipped. The retained "passing" cell candidate was therefore **the smooth template cylinder
  alone**, not evidence that the captured cell geometry matched the original.

This does not invalidate the earlier route, bias, or emission reads; it invalidates the
geometry replay's claim to have drawn the captured quads. The decode is still not fully
settled: three structurally motivated decodes were rendered, and the structural one is closest
but still fails (`initial` 0.024043, `left` 0.025862).

## Residual

- **Not a pose error:** a +/-10 px shift scan improves left only from 0.030445 to 0.029926
  (dx=1); the residual is content, not a small camera offset.
- **Not the per-pass bias:** the bias mutations are pixel-invisible above.
- **Not missing depth testing:** disabling it is far worse.
- **Most likely next lead:** the captured outer quads carry a per-quad diffuse/shading colour
  (4x f64 at quad `+0x00`, e.g. `255` or `216.8`) that the original emitter consumes at
  `0x46D460`; the candidate deliberately ignores it. This is untested, not proven.
- **Still unimplemented:** clipping, near/far planes, guard-band, scissor, blend state, draw
  order, and whether the walker draws a full 360-degree shell or a ~236-degree arc.

## Reproduction

```text
python build/tmp/a1-sourced-candidate/render_sourced.py --case initial --label sourced
python build/tmp/a1-sourced-candidate/render_sourced.py --case initial --label mut-outer-smooth --outer smooth
python build/tmp/a1-sourced-candidate/render_sourced.py --case initial --label mut-outer-ctx-neg --outer-context -0.0003
python build/tmp/a1-sourced-candidate/render_sourced.py --case initial --label mut-inner-ctx-zero --inner-context 0.0
python build/tmp/a1-sourced-candidate/render_sourced.py --case initial --label sourced-restored
python build/tmp/a1-sourced-candidate/score.py --dir build/tmp/a1-sourced-candidate/candidates --label sourced
```

Artifacts: `build/tmp/a1-sourced-candidate/round-note.md`, `pixel-report.json`,
`render_sourced.py`, `score.py`, and the candidate PNGs. Raw retail payloads remain under
ignored `build/tmp/`.

**Next question:** apply the captured per-quad diffuse/shading to the sourced candidate and
resolve which mesh decode is real, then re-run every retained view and the isolated mutation
gate under the unchanged criterion. Do not soften the threshold or claim A1 closed.
