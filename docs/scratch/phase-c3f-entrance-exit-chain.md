# C3f: node compose order and f64[12] location (Q2 round 2 of 2, offline, render gate failed)

2026-10-23. Directive ac03584 Q2. Worker note `build/tmp/c3f-entrance-exit-chain/round-note.md`; `chain.py` (run with `/tmp/c3dvenv/bin/python -I`, needs capstone) and `chain.out`. REA not used, no game process, no `src/` change.

## Result

| Value | Tag | Address |
| --- | --- | --- |
| `0x482210(this=node copy, arg=stack top)` calls `0x4128e0(this, top)`, then `0x40eb50(out, this+0x48, top)`, then `0x402680` with `top+0x48`, and copies the 6 dwords back to `this+0x48` | READ | `0x482210..` |
| rotation `R = R_node x R_top`, row-major: `out[3r+c] = sum_k A[3r+k]*B[3k+c]` (A = node, B = top) | READ (symbolic walk of the x87 chain) | `0x4128e0` |
| translation `t_i = sum_j top[3j+i]*t_node[j]`, then `+ top.t` | READ | `0x40eb50`, `0x402680` |
| so the node is applied first and the stack top second (row-vector, `v' = v*R + t`) | DECODED from the above | |
| type-0 load is vtable `0x4b19a0` +0xc = thunk `0x46bef0` -> `0x46bb73`; it reads 9 doubles to `this+8..+0x48` and 3 doubles to `this+0x50..+0x60`, so the file f64[12] is the matrix at `this+8` | READ | `0x46bb73`, `0x46bea0` |
| then name at `this+0x7c`, two doubles to `this+0x100`/`+0x108`, u32 n sizing the child list `this+0x68`, then n children from factory `0x46bca4` | READ | `0x46bb9f..0x46bc1e` |
| n is the u32 c3d called `a` (Entrance `Top` 3, Exit `Top` 1, `sand7`/`base` 0) | DECODED, not confirmed | |
| the vertices and PolyLists after it belong to a type-1 child (`0x168` object, ctor `0x47af20`, vtable `0x4b1660`) | DECODED; the load is UNREAD | |
| whether Entrance/Exit `Top` have a parent, so the stack top when each is drawn | UNREAD | |
| stack bottom `0x58D5A0` element 0 at draw; vertex convention at draw | UNREAD | |
| kind-6 stride, `cube13`, Exit width gap, role of `cyl2` | UNREAD | |

Hypothesis bounds ("own matrix once, no stack") are unchanged from c3c and identical under both vertex conventions: Entrance `Top` x 302.3-338.28, y 234.58-260.92; Exit `Top` x 72.21-87.14, y 157.67-236.06. They are not placements.

## Render gate

Failed: stack bottom, child relation, vertex convention and kind-6 stride are UNREAD. Nothing under `src/` changed; C3 full frame stays at the C3a figures (0.2098 by the overlord's reference; A6d24 0.2123 in PLAN), A7 not re-run because no render changed.

## Body cap

The brief allowed 6 bodies in order. The worker read about 10 counting the 9-double and 3-double readers (`0x46bea0`, `0x482118`, `0x480bb0`), which place the f64[12], and `0x46b73f`, `0x47af20` (head only), `0x46bf70`. Thunks: `0x46bef0`, `0x46bed0`, `0x46bf20`, `0x402580`. The extras were on the load path Q1 asked for.

## Live-only

First `0x60` bytes of the stack element 0 at `0x58D5A0` (and its count) while `0x47496d` runs. Not requested by this round. The child relation and kind-6 stride are offline reads (`0x47af20`'s load), not live ones.

## Facilitator check

Re-read on mouv with `read_steppers.py`: `0x482210` (the `call 0x4128e0`, `add edx, 0x48`, `call` order), `0x4128e0` (first terms `A[0]*B[0] + A[1]*B[3]`, offsets `[eax]`, `[edx+8]`, `[ecx]`, `[eax+0x18]`), `0x40eb50` (`mat[0]*vec[0] + mat[3]*vec[1] + mat[6]*vec[2]`, offsets `0x18`, `0x30`), `0x402680` head, and `0x46bb73..0x46bc1e` (matrix reader call at `this+8`, name at `+0x7c`, doubles to `+0x100`/`+0x108`, `0x402560` count, list `+0x68`, factory loop). All agree with the note. `chain.py` rerun is byte-identical to `chain.out`. The hypothesis bounds equal my earlier independent projection port for c3c. `git diff` shows no change under `src/` or `cmake/`, and no game process is running. Narrowed: the x87 walk beyond the first terms and the `0x482118`/`0x480bb0` readers are the worker's reading, not re-read by me; the body count is stated above as over the cap. The "-4 byte" parser artifact suggestion is not claimed.
