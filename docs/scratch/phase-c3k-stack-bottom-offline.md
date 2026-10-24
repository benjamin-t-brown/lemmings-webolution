# C3k: stack bottom `[0x58D3D8]` and the `0x40eb50` convention, offline (directive Q1 round 1 of 2)

Worker note: `build/tmp/c3k-stack-bottom-offline/{prediction.md,stackbottom.md,round-note.md,c3k_check.py,c3k_check.out}`. Offline; no game, no debugger, no `src/` change.

**Body cap breached.** About 12 bodies against the hard cap of ten. Recorded, not waived.

## (a) Writer of `[0x58D3D8]`

| Term | Tag | Address |
| --- | --- | --- |
| `0x58D3D8 = 0x58CB40 + 0x898`, a field of the camera object built by `0x468e22` (`mov ecx, 0x58cb40; call 0x468e22` at `0x45da5a/5f`) | READ | `0x45da5f` |
| The only store into `this+0x898` is `add edi, 0x898` then `mov ecx, 0x18; rep movsd`, source the return of `0x43bca0` | READ | `0x469113`, `0x46911e` |
| `.text` has 17 dword references to `0x58D3D8..0x58D42F`, all `push 0x58d3d8` (address as argument), and one `add reg, 0x898` | READ (sweep) | worker `writer_check.out`; facilitator sweep agrees |
| The builder has one rel32 caller (`0x45da5f`); every path from the frame entry `0x45cff9`, in all six `[this+0x572c]` arms including the pause arm, reaches the push `0x45dd7b` through it | READ (CFG walk) | `c3k_check.py` C |
| Between the push `0x45dd7b` and the scene call `0x45e0e6` the only call is `0x45fd90`, which copies into the stack's heap array | READ | `frame_dd7b.out` |
| So element 0 of `0x58D5A0` at draw is the builder's value in the same frame | READ for the window; DECODED for "at draw" (the walk below `0x437ba3` is unread) | |
| Value: the 12 doubles at `0x58D3D8` from `camera_a1/reference-state.json`, the same camera block as the A6f repair 4 pause; the A1 audit reproduces them (binary32 residual 0.0) | MEASURED-FROM-REFERENCE | `audit_a1.out` |
| The scene call runs in the paused frame: needs `[game+0] == 2` (`cmp` at `0x45dd96`) and byte `[game+0x5139] != 0` (`0x441853`); neither was read in the pause. The tube draws in the pause behind the same gate, so it very likely runs | DECODED | `gate5139.out` |
| Callees of `mov ecx, 0x58cb40` not read, so a whole-object copy through one is not excluded; none sits in the push-to-scene window | UNREAD (stated limit) | |

## (b) `0x40eb50`

`out[j] = mat[j]*vec[0] + mat[3+j]*vec[1] + mat[6+j]*vec[2]`, a row vector times a row-major 3x3, args `[ebp+8]` out, `[ebp+0xc]` vec3, `[ebp+0x10]` mat. READ at `0x40eb59/5b`, `0x40eb63/66`, `0x40eb71/74`, stores `0x40eb7c`, `0x40eba8`, `0x40ebd5`. `0x40eae0` = `0x40eb50` then adds the translation at `M+0x48`. c3f, c3i and LESSONS A6d15 agree.

**Disagreement found:** `tools/harness/measure_d2_pick.py:149-156` swaps the roles. The widget projection at `0x456360..0x456377` pushes `0x58d3d8` (the matrix, `[ebp+0x10]`), then the widget vector from `0x412c20`, then the out pointer, and calls `0x40eae0`. The script treats the widget's nine doubles as the matrix and `0x58D3D8` as the vector. Neither run nor edited here. It belongs to the C2 cursor clause, which this directive holds out of scope; recorded in Phase E.

## Facilitator check

Re-read on mouv: `0x40eb50` head (the `fld/fmul` offsets `0`, `0x18`, `0x30` and the store `0x40eb7c`), `0x469103..0x46911e` (`add edi, 0x898`, `rep movsd`), `0x45da5a/5f`, `0x45dd70..0x45dd80`, `0x456357..0x456377` (the push order that shows the role swap). A separate `.text` dword sweep for `0x58D3D8..0x58D42F` and for `add reg, 0x898` finds the same 17 sites and the single `0x469113` writer. `c3k_check.py` rerun is byte-identical. No code changed, so no mutation applies. Narrowed: the CFG walk, the `0x45fd90` chain and the gate bytes were not re-derived by me; the pause value of `[game+0]` and `[game+0x5139]` is unread.
