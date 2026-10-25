# C1aj: the format callback copies 32 bytes and returns 1

Queue `c1aj-format-keep`. Offline. No stock start. Worker [Read format callback keep](106b510e-f00f-470b-aeba-ede3e07576e0). `prediction.md` 18:21:43, before the reads. REA was used for bounds and references. Dispatched 18:20 MDT.

## Result

`0x473B1E` is one function, 636 bytes, and it returns only at `0x473D97` `c2 08 00` (`ret 8`). The image contains the immediate `0x473B1E` twice: `push` at `0x4731CF` and `0x47320E`. Both are arguments to `call [eax+0x20]` (`0x4731E3`, `0x473222`).

What it keeps is 32 bytes of the enumerated record. Each keep is `push 0x20` / `push [ebp+8]` / `push [context+0x10]` / `call 0x49D010`. The copy that also stores the popcount at `[context+0xc]` is `0x473C95` `e8 76 93 02 00`. That arm runs when context byte `+6` is 0 and byte `+4` is not 0, and only if `[format+4] & 0xE0000` is clear, `[format+0xc]` equals `[context+0]` (both call sites store `0x10` there), context byte `+7` matches "`[format+8]` is nonzero", bit 0 of `[format+4]` is set, and the set-bit count of `[format+0x1c]` is greater than 1. If context byte `+8` is already set, a later record replaces the copy only when the signed OR of `[format+0x10]`, `+0x14` and `+0x18` is greater than or equal to the same OR of the stored record (`0x473C7C` `jge`).

Two other copies: `0x473B76` when byte `+6` is nonzero and `[format+4] & 0x20` is set, then `0x473B85` `xor eax, eax`; and `0x473D79` when bytes `+6` and `+4` are both 0. The third arm does not read `[format+0x1c]` and does not store a popcount. A null context, a null record, the `0xE0000` bits, and every other reject reach `mov eax, 1`. The popcount keep returns that same `1` at `0x473CAD`.

Context bytes `+4`, `+5`, `+6` and `+7` are computed in `0x473146`..`0x4731C7` from runtime fields. They are not immediates. Field names are not in the exe.

## Facilitator check

Disassembled `0x473B1E` for `0x27C` bytes: 208 instructions, one `ret`. The exe digest is `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`. Re-read `0x473C95` `call 0x49D010`, `0x473D97` `ret 8`, `0x473CAD` and `0x473D8F` `mov eax, 1`, `0x473B85` `xor eax, eax`, and `0x49D043` `rep movsd`. After `0x4731E3` the next instruction is `0x4731E6` `mov edx, [ebp-0xc]`, context byte `+8`, not `eax`. An in-memory compare of those bytes passed. Expecting `c2 08 01` at `0x473D97` failed; the file byte is `c2 08 00`. The exe was not written. No source change, so the suite was not rebuilt. No Lemmings process.

Narrowing: keep and reject share `mov eax, 1`, so the return does not say which happened. The upload's second `call [eax+0x20]` runs only when context `+8` is still 0 and context `+6` was nonzero (`0x4731E6`, `0x4731F3`). Whether `eax == 0` stops the COM method is outside the exe. The numeric mask and the shift counts are still unread.

## Stop

Format-enumeration round 1. The exe-side question is answered. The counts still need the live read, which waits on the upload's phase.
