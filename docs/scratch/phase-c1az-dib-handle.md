# C1az: the handle is object+4, and the bit count is not in the file

Queue `c1az-dib-handle`. Offline round 2 of 3 on `kept-format`. No stock start. Worker [Read DIB handle bits](5ba8d88c-80ce-43fc-a832-8a29f283be56). `prediction.md` 01:53:33, before the disassembly. `round-note.md` 02:00:00. REA bounded the callers. Dispatched 01:52 MDT.

## Result

`0x47255B` stores `CreateDIBSection`'s return at `[edx+4]`. `edx` is the `ecx` saved on entry to `0x472490`. That `ecx` is the object saved at `[ebp-0x588]` in `0x47218A`, and `0x47241A` passes the same slot to `0x472F99`, which `GetObjectA` reads as `[object+4]`.

The creation call pushes a pointer, not a count. The pointer is `0x58D5B8+0x0E`. The word this function multiplies as the bit count is at that pointer `+0x0E`, address `0x58D5D4`. `.data` file bytes end at `0x4C6000`, so the word is not in the image. `0x4724D6` copies onto `0x58D5B8` from the buffer `0x45015A` returns. That function was not opened. `+6` stays unread.

## Facilitator check

Re-read `0x4721A8` `mov [ebp-0x588], ecx`, `0x47231A` `mov ecx, [ebp-0x588]`, `0x472320` `call 0x472490`, `0x4724AB` `mov [ebp-0x30], ecx`, `0x472552` `call dword ptr [0x4B0054]`, and `0x47255B` `mov [edx+4], eax`. The slot's name is `CreateDIBSection`. Re-read `0x47240D` through `0x47241A` `call 0x472F99`, and `0x472FA2` `mov [ebp-0xb08], ecx` followed by `push ecx`. Re-read `0x4724C4` `call 0x45015A`, the copy `push 0x58D5B8` / `call 0x49D010`, and `0x472518` `mov cx, [eax+0x0E]`.

`build/tmp/c1az-facilitator/check_handle.py` checks those bytes. The exe SHA-256 matches `docs/LESSONS.md`. `0x58D5B8+0x1C` is `0x58D5D4`, past the `.data` raw end `0x4C6000`. Expecting store byte `0x88` failed; the file bytes are `89 42 04`. The checker was restored and passed.

The import at `0x4B0020` is `DirectDrawEnumerateA`, reached by `jmp [0x4B0020]` at `0x4A6EF0`. `0x472FA8` pushes the object, then `0x472FAD` calls that import, then `0x472FB2` is `call eax`. Those two bodies were not read. No Lemmings or Frida process. No source change, so the suite was not rebuilt.

Narrowing: the store and the push use `[object+4]` of the object in `[ebp-0x588]`. The push at `0x472FA8` puts that pointer on the stack before the two calls, so this check does not show that `+4` is still the `CreateDIBSection` return when `GetObjectA` runs. The saved `^TAB.BMP` 24 bpp word is not an operand of this call.

## Stop

Offline round 2 of the kept-format question. The bit-count word's producer is `0x45015A`. The number is not. The gate is not scored.
