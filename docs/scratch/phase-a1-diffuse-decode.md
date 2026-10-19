# A1 per-quad diffuse and mesh-decode correction: still short

This follows the [sourced candidate](phase-a1-sourced-candidate.md). **A1 remains open.** The
criterion is unchanged: 640x480, ROI `[100,80,280,410]`, normalized mean absolute RGB error
`<= 0.02`.

## Verdict

The captured per-quad diffuse is real, exactly determined, and now applied — but it is a
small effect. The larger correction is the mesh decode: the retained `*-mesh4.bin` windows
are one allocation off and must not be used. Even with both fixes the best candidate still
fails two views.

| View | Best candidate (`m0min-diff`) | Pass? |
| --- | ---: | --- |
| initial | **0.02146735327127484** | **no** |
| mouse | 0.01604711604058009 | yes |
| right | 0.014810479522897824 | yes |
| left | **0.023751700006601966** | **no** |

The facilitator independently recomputed all four ROI errors from the candidate PNGs and the
retained originals; they match exactly. The `baseline` label also reproduces the previous
candidate bit-for-bit (PNG hashes and errors), and `baseline-restored` is byte-identical after
every mutation.

## Mesh decode, settled from the object headers

The route calls establish the layout, not the pixels:

- `0x413640` is `add eax,0x10; ret`; `0x4136c0` is `add eax,0x80; ret`.
- `0x462404` (mesh4) gates on `record+0x04`, loads `[record+4]`, calls `0x413640`, then
  advances by `0x4136c0`.
- `0x4623a5` and `0x462465` (mesh0) do the same with `[record+0x00]`.
- The 16-byte object is `{kind=2, count, capacity, end=obj+0x10+count*0x80}`; the quad array
  starts at `obj+0x10`, stride `0x80`.
- Measured invariant: for **163/163** double-mesh cells, `mesh4_obj == mesh0_obj + 0x10 +
  count0*0x80`.
- The capture scripts read from the header's `+0x0C` **one-past-end** pointer, so every
  window is shifted by `count*0x80`. The mesh0 window's base-16 payload is mesh4's array;
  the mesh4 window's payload is the next allocation. The two arrays are the same mesh in
  **47 of 49** fully covered equal-count cells.

The previous round's "base 16 is the array base" is therefore only true *within the shifted
window*. Dropping the mesh4 window (`m0only`) improves all four views, the largest single
correction so far: initial `-0.003229`, mouse `-0.003435`, right `-0.002974`, left `-0.006695`.

## Diffuse conversion

Read from `0x46D460` and verified by the retained emission:

- `0x46D52E` loads the alpha double at `[cache+0x50]`; `0x46D531` calls `0x49C5AC`.
- `0x49C5AC` is `FSTCW; OR AH,0x0c; FLDCW; FISTP` — **truncate toward zero**.
- `0x46D538` shifts alpha left 24; `0x46D548`/`0x46D556`/`0x46D56F` read components
  `+0x38/+0x40/+0x48` and shift red 16, green 8, blue 0; `0x46D586` writes the packed
  diffuse to the emitted vertex.
- The retained selected quad `(255,255,255,0)` emits `0x00FFFFFF`, bit-exact.
- All captured quads are grey with alpha 0, so channel order does not matter at this level.

The diffuse changes 747 of 59 400 ROI pixels on `initial` and 276 on `left`; it cannot close
either gap.

## Isolated effects

| Label (one change) | initial | mouse | right | left |
| --- | ---: | ---: | ---: | ---: |
| `baseline` (old decode, diffuse off) | 0.024899 | 0.019482 | 0.017785 | 0.030445 |
| `diff-on` (diffuse only) | 0.023386 | 0.019482 | 0.017785 | 0.027818 |
| `m0only` (decode only) | 0.021670 | 0.016047 | 0.014810 | 0.023751 |
| `m0min-diff` (both) | **0.021467** | 0.016047 | 0.014810 | **0.023752** |
| `smooth` (outer geometry only) | 0.018479 | 0.016094 | 0.014810 | 0.033014 |
| `arena-off` (stitched true offsets) | 0.154132 | 0.177136 | 0.162796 | 0.140519 |

The smooth template still beats the captured shell on initial/mouse/right but is worse on
left. The `arena` result is a **data gap**, not evidence against the decode: only 414 of 776
quads have all 128 bytes covered in the retained capture.

## Residual and next question

- **Not a pose error:** a +/-4 px shift scan bottoms out at `initial` 0.020886 and `left`
  0.023057, still above threshold.
- **Still absent:** near/far planes, guard band, scissor, blend/alpha state, per-bucket route
  selection, and the last 16 bytes of every window's last quad.
- **Next lead:** a bounded live read of the ~100 KB arena from `0x4ECB00` to capture the true
  mesh0 arrays (the retained capture fully covers only 147 of 389 mesh0 quads). Then re-render
  the four views and the isolated gate.

## Reproduction

```text
python build/tmp/a1-per-quad-shading/stitch_arena.py
python build/tmp/a1-per-quad-shading/render_shaded.py --case initial --label m0min-diff
python build/tmp/a1-per-quad-shading/score.py --dir build/tmp/a1-per-quad-shading/candidates --label m0min-diff
```

Artifacts: `build/tmp/a1-per-quad-shading/round-note.md`, `pixel-report.json`,
`render_shaded.py`, `score.py`, `arena.bin`, `arena.json`, and the candidate PNGs.
