# C3d: scene node placement chain, partial read (Q1 round 2 of 3, offline, blocked at the body cap)

2026-10-23. Directive ac03584 Q1. Worker note `build/tmp/c3d-scene-transform-read/round-note.md`; `c3d_chain.py` (stdlib), `c3d_chain.out` (two long lines truncated at 230 characters in the saved file; a rerun is the same content). No start, no debugger, no `src/` change. REA not used; capstone via a throwaway venv. About 10 bodies touched against a cap of 8 (trivial thunks counted); the substantive draw-time callee was never reached.

## Result

| Value | Tag | Address |
| --- | --- | --- |
| `0x437ba3`: `add ecx,0x10; call 0x47496d` | READ | `0x437baa..0x437bb0` |
| `0x47496d`: `push 0x58d3d8; call 0x45fdb0` (matrix-stack push); `[this+0x30] = [0x58ce24]`; `[0x58ce24] = this+0x34`; `call [[this+4]+4]`; pop `0x43bbe0`; restore `[0x58ce24]` | READ | `0x474974..0x4749b5` |
| `[this+4]` built at load by `0x4749bf` through factory `0x46bca4` (u32 type 0..3; allocs 0x118 / 0x168 / 0x1d8 / 0x358; vtable `0x4b19a0` +4 = `0x46b6cc`) | READ | |
| which type the root `thenull` stream selects; whether `0x46b6cc` is the callee at `0x4749a7` | UNREAD (inference) | |
| what the callee does with matrix stack `0x58D5A0` (`0x43be80`, `0x43be30`, `0x43be10`), whether root scale 10 reaches children | UNREAD | |
| file chain: `thenull` (diag 10, t 0) -> `sand7` (R=I, t=(35.5122,-1.8937,-81.1852)) -> `base` (R=I, t=(0,1.3325,0)); each next node's matrix starts 4 bytes before the parser's end (cause UNREAD) | READ values / DECODED walk | |
| no second matrix after a node's vertices; a node's transform is its own f64[12] | DECODED (layout fit, consumer UNREAD) | |
| Exit `Top` matrix has no scale, so a node-local scale does not explain the Exit width gap | READ | `0xa0ce1` |
| projected bounds with "own f64[12] applied once, no parent": Entrance `Top` x 302.3-338.28 y 234.58-260.92, Exit `Top` x 72.21-87.14 y 157.67-236.06 (both equal c3c); root `thenull` implausible (252 of 933 vertices in front, x -372795..106693) | DECODED hypothesis | |

The root result does not show scale 10 is misapplied; it shows the hypothesis is incomplete. Live-only: `[0x58ce24]` before and during `0x4749a7`.

## Facilitator check

Re-read on mouv: `0x47496d..0x4749a7` (every instruction in the table), `0x4B19A0` vtable bytes `20 bf 46 00 cc b6 46 00` (+0 `0x46bf20`, +4 `0x46b6cc`), `0x45fdb0` prologue. `c3d_chain.py` rerun: identical except two lines the saved file truncates at 230 characters. Narrowed: the body cap was exceeded (about 10 against 8), counted by the worker; the round is partial and states so; the "-4 byte" node offset is unexplained and may be a parser overshoot. No `src/` diff, no game process.
