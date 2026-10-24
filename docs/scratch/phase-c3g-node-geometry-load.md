# C3g: type-1 child load, node tree, kind-6 stride (Q1 round 1 of 2, offline)

2026-10-23. Directive 943cd7e Q1. Worker note `build/tmp/c3g-node-geometry-load/round-note.md`; `parse.py` / `parse.out`, `check_stride.py` / `check_stride.out`. REA not used; no game process, no `src/` change. Bodies: about 17 against a cap of 10, all on the requested load path (named in the worker note).

## Result

| Value | Tag | Address |
| --- | --- | --- |
| factory jump table `0x46be3d`: type 0 = `0x118`, type 1 = `0x1d8` (ctor `0x413290`), type 2 = `0x358` (ctor `0x47b320`), type 3 = `0x168` (ctor `0x47af20`); c3f had 1 and 3 swapped | READ | `0x46bceb`, `0x46bd7d`, `0x46bdc3`, `0x46bd34` |
| type-1 load `0x413450` -> `0x469578`: u32 nv + nv x 3 f64; u32 nn + nn x 3 f64; u32 ntex + ntex x (string, u32); u32 nblocks, each u32 size + `PolyList 09` + kind + count + u2 + records; then the node head | READ | `0x469578`, `0x431390`, `0x4697c0`, `0x46b4e5`, `0x469baf` |
| c3d's "node thenull, 933 vertices" was `sand7`'s mesh read before its head; the "one" u32 is the first child's type; the "4 byte" offset is the next child's type u32 | DECODED from the above | |
| kind-6 record is 116 bytes: u32 + 3 f64 + f64 + 4 x (u32 + 2 f64) | READ | `0x46a190`, `0x46b250`, `0x47a004`, `0x47a0c0` |
| kinds 5 and 7 are 160 and 212 bytes | EMPIRICAL only (readers `0x46af80`, `0x46b3d0` unread) | `check_stride.out` |
| `a` is the child count n; `thenull` has `sand7`, `base`, `top`; Entrance `Top` has `cube8`, `cube9`, `cube13`; Exit `Top` has `cyl2` | READ | `parse.out` |
| Entrance `Top` and Exit `Top` are roots of their own Object 09 record, so the scenery root's scale 10 does not reach them | READ | `parse.out` |
| `Top` nodes carry no mesh; `cube13` is a door mesh (mirror of `cube9`); `cyl2` is basket and rope, 137 vertices, diag(4.391, 1.371, 4.391), y -9.391 | READ | `parse.out` |
| type-1 draw `0x46c55d` (vtable `0x4b0948` +4), vertex convention at draw, stack bottom `0x58D5A0` element 0, per-child visibility, door state, type-2 load | UNREAD | |

## Re-projection (HYPOTHESIS: identity stack bottom; column and row convention agree)

| Mesh | Projected x0,x1,y0,y1 | Reference (MEASURED-FROM-REFERENCE) |
| --- | --- | --- |
| Entrance `cube8` | 302.30, 338.28, 234.58, 260.92 | hatch [303,235,338,276] |
| Exit `cyl2` | 47.64, 113.02, 165.76, 274.77 | balloon [50,167,92,235] |

Entrance matches at the top and sides; the bottom 260.9 against 276 is unexplained. Exit overshoots (65 px wide, 109 tall projected against about 42 and 68), so the width gap does **not** close; the reference is a coarse blue test clipped at x=48. Nothing fitted.

## Prediction miss

Counts do not land on the next `Object` tag: the trees close 7, 12709 and 170 bytes before the next tag or end of file. Every declared child count and `PolyList 09` tag lands where the reader puts it. Stride guess (~184) and texture-entry guess (string, string) were wrong.

## Render gate

Failed; nothing under `src/` changed. Draw method, vertex convention, stack bottom and visibility are UNREAD.

## Facilitator check

Re-read on mouv: jump-table targets `0x46bceb`/`0x46bd7d`/`0x46bdc3`/`0x46bd34` (push `0x118`/`0x1d8`/`0x358`/`0x168` and ctor calls `0x46bed0`/`0x413290`/`0x47b320`/`0x47af20`), `0x46a190` loop calling `0x46b250`, `0x47a004` (`0x480bb0`, f64 at `+0x18`, four `0x47a0c0` calls at stride `0x18`), `0x47a0c0` (u32, `0x4821de`, one more call), `0x46b250` (u32 then `0x46b280`). 4 + 24 + 8 + 4 x 20 = 116 agrees. A fresh 30-line parser (not `parse.py`) reproduces both trees: Entrance `Top` -> `cube8`, `cube9`, `cube13` ending `0x9db2b`; Exit `Top` -> `cyl2` ending `0xaaa43`. Narrowed: the strides of kinds 5 and 7 are the worker's empirical landing, not a read (my parser used the same numbers, so it is not independent for those); the `0x4b0948` draw and the projection code were not re-read; no mutation applies because no code changed.
