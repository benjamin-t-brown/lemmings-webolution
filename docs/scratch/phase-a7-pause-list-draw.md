# A7h — G+0x25C68 draw has no constant in band B

2026-10-20. **READ.** The pause handler draws that object without a fixed
pixel in y `432..447`. The wider-mask score is unchanged.

## Arguments

G is `[ebp-0x94]` in `0x45CFF9`. The object is G+`0x25C68`.

`0x45D2D9` calls `0x44DF40` with `this` = G+`0x25C68`. The two stack values
are formulas. Push order:

| Push | Value | Callee slot | Stored |
| --- | --- | --- | --- |
| `0x45D2B5` `push edx` first | `[0x58CB44] - 2*[ebp-0x34]` | `[ebp+0xC]` | `[this+0x18]` |
| `0x45D2CC` `push ecx` second | `sar([0x58CB40],1) - sar([ebp-0x30],1)` | `[ebp+8]` | `[this+0x14]` |

`[ebp-0x34]` is the return of `0x47AC50` and `[ebp-0x30]` is the return of
`0x47ACF9`, both called on G+`0x25C68`. `0x58CB40` and `0x58CB44` are past
raw `.data` (`0x4C6000`), so their values are runtime. `0x44DF46` saves
`ecx` to `[ebp-0xC]`. `0x44DFCF` writes `[ebp+8]` to `[this+0x14]`.
`0x44DFD8` writes `[ebp+0xC]` to `[this+0x18]`. `ret 8` pops both arguments.

The H arm at `0x44D1F1` pushes `432` (`0x1B0`) first, so that constant is
`[ebp+0xC]` there, and pushes `0x140 - sar(...)` second. The G+`0x25C68`
arm has neither `432` nor `320`.

`0x45D2F4` calls `0x44E010` on the same object after `push 0x3F613404` then
`push 0xEA4A8C15`. As float32 the first dword is about `0.8797`. The second
is not a pixel. `0x45E342` calls `0x44E150` on G+`0x25C68` with no stack
arguments (`0x44E1AD` is `ret`). That call runs only when `[G+0x572C] == 1`
(`0x45E31C`, `jne 0x45E347`). The same test also calls `0x44DE60` on
G+`0x25C90`.

## A different object in the same function

`0x45E124` stores `[ebp-0x58]=160`, `[ebp-0x54]=480`, `[ebp-0x4C]=180`,
`[ebp-0x50]=260`. Those dwords are loaded as doubles and passed to
`0x47AD8C` at `0x45E2A4` with `this` = G+`0x26000`. `0x45E2B5` then calls
`0x44E150` on that object. They are not arguments of the G+`0x25C68` calls.
`480` is the bottom of a 640×480 frame and can overlap band B only once the
edge order is known. It is not known here.

## Facilitator check

`build/tmp/a7h-facilitator/verify.py` re-decoded the push order, the
`+0x14`/`+0x18` stores, both immediates, the state test, `0x44E150`'s
bare `ret`, the H `432`/`320` pair, and the four rectangle stores.
Expecting `add ecx, 0x2018` at `0x45E33C` failed; the file byte is
`81c1685c0200`. Restored hash
`11cf4ee1cc7e01efdd096cd9924e0156e4dbfed30e5d4175a809af3f31395604`.

The packet had `[ebp+8]` and `[ebp+0xC]` swapped on the G+`0x25C68`
placement call. Build had no work. CTest 24/24. No game process.

## Left open

No constant on the G+`0x25C68` draw sits in band B. The `160/480/180/260`
values on G+`0x26000` are the next geometry to read. Index `0x54` and
`[ebp-0x10]` stay unread. The gate remains
`3031085/137520225 = 0.0220410125`.
