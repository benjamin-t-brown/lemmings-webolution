# A7e — builder 0x45D27B does not fill base list H

2026-10-20 project date. User-authorized Cursor continuation of the aborted
OpenCode sixth dispatch. **Negative READ.** Not a caption owner, not a pixel
pass, and not an error reduction.

## Result

Let G be the incoming `this` of `0x45CFF9`. `0x45D017` stores entry `ecx` to
`[ebp-0x94]` before any later use. The same slot is the frame receiver at
`0x45E0E0` / `0x45E0E6` `call 0x441845`. Accepted consumer H remains
`G+0x2018` at `0x441A79` / `0x441A7F` `call 0x44CD06`.

The builder call `0x45D27B` `call 0x4404FA` does **not** pass H. On the path
that reaches it:

| VA | Bytes | READ |
| --- | --- | --- |
| 0x45D226 | `c7822c57000001000000` | `[G+0x572C] = 1` |
| 0x45D25D | `81c2685c0200` | `add edx, 0x25C68`; that pointer stays on the stack |
| 0x45D264 | `6a54` | push index `0x54` |
| 0x45D266 | `e8dc950200` | `call 0x486847`, then `add esp, 4` only |
| 0x45D275 | `8b8870550000` | `ecx = [G+0x5570]` |
| 0x45D27B | `e87a32feff` | `call 0x4404FA` |
| 0x45D286 | `81c1685c0200` | `add ecx, 0x25C68`; `call 0x47ACF9` |

`0x486847` reads `[ebp+8]` and global operand `0x4C3434`, calls `0x486819`,
and returns with `ret` (no argument pop). The caller's `add esp, 4` removes
only the index. `0x4404FA` then sees the returned pointer at `[ebp+8]` and
`G+0x25C68` at `[ebp+0xC]`. `0x25C68 - 0x2018 = 0x23C50`. No instruction on
this arm adds `0x2018` or calls `0x44CD06` / `0x44E150`.

Index `0x54` is the lookup argument. Its runtime string is not read.
`0x4C0BE8` on the same arm is the file bytes `pause\n` pushed to `0x40F860`.
That is a different operand from index `0x54` and from `STR_PAUSED` at
`0x4C3C18`.

## Which arm

Jump at `0x45D1AC` is `jmp dword ptr [eax*4+0x45E487]`. `eax` is
`[G+0x572C]`, copied through `[ebp-0x9C]`, after `cmp ..., 5` / `ja 0x45D7F4`.
Table dwords: `0x45D1B3`, `0x45D459`, `0x45D531`, `0x45D7F4`, `0x45D6BC`,
`0x45D6BC`.

The builder is inside case 0 only on one branch. `0x45D1E2` `je 0x45D3C7`
skips it. `0x45D1FC` `jne 0x45D216` is the branch that continues;
`0x45D211` `jmp 0x45D7F4` leaves. `0x45D21B` `call 0x498D9A` is followed by
`0x45D220`, which reloads G and stores state 1. Case index 1 at `0x45D459`
has no `call 0x4404FA` and no `add 0x2018`. Inside that case, `0x45D49C`
stores 2 and `0x45D508` stores 0. Do not describe the whole case as steady
pause.

## Same arm, still not H

After the builder, the arm reloads `ecx` from `[ebp-0x94]`, adds `0x25C68`
again, and calls `0x44DF40` at `0x45D2D9` and `0x44E010` at `0x45D2F4`.
`0x44DF40` is the placement helper A7d named on H. Here its `this` is
`G+0x25C68`. The following call is `0x44E010`, not accepted draw `0x44E150`.
Two immediates, `0x3F613404` and `0xEA4A8C15`, are pushed before `0x44E010`.
Their meaning is not identified.

## Facilitator check

Independent checker `build/tmp/a7e-facilitator/verify.py` (does not load
`pins.json`) re-decoded the dispatch, both switch arms, the builder window,
the lookup wrapper, the builder prologue, the frame call, and H's `add
0x2018`. Authority SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.

Changing only `LIST_IMM` from `0x25C68` to `0x2018` failed both list adds
(`edx` and `ecx`). Restored hash
`08fed147ca9ee8e6ea6763e02e78c1f1e2cbe22f1fa87d117ab9aeb5a47e5679`, then the
checker passed again. The worker checker also passed 11 pins; its in-memory
expectation `81c118200000` failed pin `builder-list-base-add` and the next
plain run passed. `prediction.md` still states the pre-read claim that this
call writes H. That destination claim is falsified. The shared-receiver part
holds: entry `ecx` and the frame `this` are the same stack slot.

A post-builder scan the worker note stopped before is included above. It
does not turn `G+0x25C68` into H. Full build had no work. CTest 24/24.
No Lemmings, webolution, or Ghidra process. No production, live, or install
writes.

## Left open

- Runtime text for index `0x54`, and the file dword at `0x4C3434`, are not
  the displayed string.
- Nothing here shows `G+0x25C68` is copied into H, or that it is not.
- Actual wider-mask score remains `3031085/137520225 = 0.0220410125`.
- No automatic eighth dispatch on `0x45D27B`. Another builder caller is in
  scope only if its destination operand is `+0x2018` on the frame receiver.
