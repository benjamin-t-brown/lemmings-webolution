# C1v: the ten button plates are drawn by `0x45B66A`, and a zero count skips the text (directive part 2, round 1 of 2)

Queue `c1v-plate-draw-read`. Offline disassembly read. No stock start. Dispatched 2026-10-24 14:02 MDT. `prediction.md` 14:03:25, before the path was opened. REA was used for structure; the bytes below were re-read with `read_steppers.py`.

## Result

`0x45B639` draws ten quads, `i < 10` (`0x45B654`), `imul 0x1B0`, `lea [this+i*0x1B0+0x30]`, `0x45B66A call 0x414920`. `this` is the skill-panel object `0x4C91D8`, so the quads are `0x4C9208 + i*0x1B0`. The icon call `0x45B784` is later and is not this draw.

A second call, `0x45B6B5 call 0x414920` on `this+0x25A0`, runs only when `[this+0x2750] != 0`. The note names that the selection sheet `^tab_selected`. That name was not re-read here.

The ten boxes are written at `0x45A795 call 0x414870`: left `0x12` (18), right `0x5A` (90), top starts at `0x0C` (`0x45A6B2`) and steps by `0x28` (`0x45A83F`), bottom is top + `0x20`. Button `i` is x `[18, 90]`, y `[12+40*i, 12+40*i+32]`. The bytes are an axis-aligned box. They do not say the texels are an oval.

The handle loaded at `0x45A708` is `[0x4E3348]`. The store is `0x45E909`, and `0x45E8F7` pushes `0x4C0E00`. Those file bytes are `5E 74 61 62 00` (`^tab`). What that name opens, and the pixels, are UNREAD. `0x45FE10` was not read.

A zero count skips the text submit. `0x45B6CC` loops `i < 9`. `0x45B6E4 cmp [this+i*4], 0` / `je 0x45B6FF` skips `0x45B6FA call 0x44E150`. The same dword is what `0x45A7D6` divides by 10. The digit builder still runs for `i != 9` (`0x45A79E`). The plate call is outside that branch.

Colour key `0x47218A` and blend `0x46D5F3` stay unread.

## Facilitator check

Re-read `0x45B639` through `0x45B70B`: the ten-quad call, the gated `0x45B6B5` call, and the zero-count skip match the note, including `imul 0x1B0` and `cmp ..., 9`. Re-read `0x45A6B2` through `0x45A842`: top `0x0C`, `push 0x12`, `push 0x5A`, bottom `top+0x20`, `call 0x414870`, then `add eax, 0x28`. Re-read `0x45E8F7 push 0x4C0E00`, `call 0x45FE10`, `mov [ecx+0x1C940], eax`, and the five bytes at `0x4C0E00`.

Not re-derived: the `G` pointer chain from `0x450A4E` to `0x45A5FB`, the body of `0x45FE10`, the UV pairs, the vertex bases inside `0x414920`, and the `^tab_selected` string. No source change, so no mutation. No game process. `ctest` was 34/34 on the unchanged tree before this landing.
