# C3i: type-1 draw 0x46c55d read, kinds 5/7 and type-2 load not reached (Q1 round 2 of 2, offline)

2026-10-23. Directive 943cd7e Q1. Worker note `build/tmp/c3i-node-draw-read/round-note.md`; `draw.py` / `draw.out`, `disasm.out`, `prediction.md` (written after the worker had seen the first 0x200 bytes of the draw; disclosed there). REA not used; no game process, no `src/` change. Bodies 9 (two partial) against a cap of 10.

## Result

| Value | Tag | Address |
| --- | --- | --- |
| draw skips the node and its children when byte `[this+0x110] == 0` | READ | `0x46c56b..0x46c573` |
| `0x43bc10(this+8)` composes the head matrix with the stack top `0x58d5a0`; `0x46c440` saves `[0x58d290]` to `this+0x1d4`, sets `[0x58d290] = this`, copies the stack top (12 f64) to `this+0x148` | READ | `0x46c580`, `0x46c440..0x46c49b` |
| so `this+0x148` is the composed model matrix and children compose onto the parent through the stack | DECODED | |
| cull: `tmp = 0x40eb50(&tmp, this+0x148, 0x58d498)`; node popped if `tmp.z + [this+0x100] <= 0.0` | READ | `0x46c598..0x46c5e4` |
| vertex loop over the list at `this+0x118`, stride `0x18`, calls `0x40eae0(out, v, this+0x148)`; vertices in file order, no permutation; `0x40eae0` = `0x40eb50` (rotation) then adds the translation at `M+0x48` | READ | `0x46c604..0x46c659`, `0x40eae0` |
| `tmp.z - [this+0x100] <= 0` runs `0x469eaa` per block (table `0x4c2230`), else `0x469d71` (table `0x4c2210`); each block binds texture `[blk+8]` from `this+0x138` (`0x43bd80`) then dispatches by kind | READ | `0x46c65b..`, `0x469d71`, `0x469eaa` |
| children at `this+0x68` via vcall +4, then pop `0x43bbe0` and restore `[0x58d290]` (`0x46c50f`) | READ | |
| `0x40eb50` (row or column, axis order), `0x43bc10` past its 11th instruction, the per-kind draws from `0x46a280`, the `0x43bd80` bind, meaning of `[this+0x100]`, door state | UNREAD | |
| runtime inputs the draw depends on: stack bottom `0x58d5a0` element 0 and the object at `0x58d498` (cull) | NAMED, not read | |
| kinds 5 and 7 readers `0x46af80`, `0x46b3d0`; type-2 load `0x47b320`; so kinds 5/7 strides stay EMPIRICAL and the scenery parse still stops at the type-2 child `0x967a2` | UNREAD (not reached) | |

## Question (d)

Not recomputed: the chain still ends in UNREAD terms, so c3g's identity-bottom boxes stand as a hypothesis (Entrance `cube8` 302.30-338.28 x 234.58-260.92 against hatch [303,235,338,276]; Exit `cyl2` 47.64-113.02 x 165.76-274.77 against balloon [50,167,92,235]). Nothing fitted.

## Q1 status

Round 2 of 2 used. The load layout (c3g) and the draw skeleton are READ. The projection (`0x40eb50`), the per-kind draws, kinds 5/7 and the type-2 load are not, so the render does not run (directive Q3 condition not met).

## Facilitator check

Re-read on mouv: `0x46c55d` head (gate `[eax+0x110]`, `call 0x43bc10`, `call 0x46c440`, cull with `0x58d498`, `fadd [ecx+0x100]` against `[0x4b0338]` = 0.0, pop `0x43bbe0`), `0x46c440` (save and set `[0x58d290]`, copy to `+0x148`), `0x46c5e9..0x46c65b` (vertex loop at `+0x118`, `call 0x40eae0`, `+0x148` as the matrix), `0x40eae0` (`call 0x40eb50`, then add `[eax]` from `0x40ebf0`). All agree with the note. Narrowed: the table-driven kind dispatch (`0x469d71`, `0x469eaa`, tables `0x4c2210`/`0x4c2230`) and `0x469c9f` were not re-read by me; the worker's prediction was written after seeing the first 0x200 bytes of the draw, so it is not an independent prediction. No mutation applies: no code changed.
