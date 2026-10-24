# C3e: scene walk callee and matrix stack (Q1 round 3 of 3, offline, blocked at the body cap)

2026-10-23. Directive ac03584 Q1. Worker note `build/tmp/c3e-scene-callee-read/round-note.md`; `c3e_chain.py` (run with `/tmp/c3dvenv/bin/python -I`, needs capstone; reuses `c3d_chain.py`), `c3e_chain.out`. No start, no debugger, no `src/` change. REA not used. 8 substantive bodies by the worker's count, three of them not on the brief's order.

## Result

| Value | Tag | Address |
| --- | --- | --- |
| the u32 after each of the four `Object 09` tags is 0 (`0x5963b`, `0x91779`, `0x9ccc8`, `0xa0cd4`); factory `0x46bca4` reads a 4-byte value and takes case 0: `new 0x118`, ctor `0x46bed0`, vtable `0x4b19a0` | READ for the factory; "that u32 is the type" DECODED (nothing else seen between tag and u32) | |
| the callee at `0x4749a7` for type 0 is `0x46b6cc`; if byte `[this+0x110]` is non-zero it calls `0x43bc10(this+8)`: `top = stack[count-1]` (stack `0x58D5A0`), `r = 0x482210(copy of this+8, top)`, push r through `0x43be80`; then vcall `+4` on each child of the list at `this+0x68`; then pop `0x43bbe0` | READ | `0x46b6cc`, `0x43bc10..0x43bc4e` |
| so a node matrix is composed with the stack top once and nested through its children | READ | |
| operand order inside `0x482210` (callees `0x4128e0`, `0x40eb50`, `0x402680`); that `this+8` holds the f64[12]; the ctor `0x46b590` and per-type load vcall `+0xc`; whether `sand7`, `base`, Entrance `Top` and Exit `Top` are children of the root or siblings (hence whether scale 10 reaches them); the stack bottom at draw (`0x58D5A0`) | UNREAD | |
| projected bounds, hypothesis "own matrix once, no parent": `sand7` x 396.0-437.0 y 308.0-322.0; `base` x 297.0-341.0 y 368.1-386.6; Entrance and Exit `Top` equal c3c; root implausible. "Child of root" bounds (both operand orders) span x 97.6-784.9, so the two orders are not separable offline | DECODED hypotheses | |

Live-only: the stack bottom at draw and `[0x58ce24]` (nothing in the touched path reads it).

## Facilitator check

Re-read on mouv: `0x46B6CC..` (the `[this+0x110]` test, `add edx,8`, `call 0x43bc10`, the child loop through `0x431cd0` at `this+0x68`) and `0x43BC10..0x43BC50` (stack `0x58d5a0`, `call 0x43bca0`, `call 0x43bc60`, push via `0x43bcf0`). `c3e_chain.py` rerun: output identical (32 lines). Narrowed: "composed once" is the shape of `0x43bc10`, with the multiply's operand order unread; the type link is decoded not confirmed; the body cap is met only on the worker's count (three bodies off the brief's order). No `src/` diff, no game process.
