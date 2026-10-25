# C1ba: 0x45015A does not write the bit-count word

Queue `c1ba-bitcount-source`. Offline round 3 of 3 on `kept-format`. No stock start. Worker [Read bit-count source](b11db5cc-2ae0-408f-8e3f-0ac0f7bb933b). `prediction.md` 02:04:51, before the disassembly. `round-note.md` 02:11:54. REA bounded the function. Dispatched 02:04 MDT.

## Result

`0x45015A` does not write offset `0x1C` of the buffer copied onto `0x58D5B8`. A walk of its 217 instructions finds no memory displacement `0x1C`.

`0x450218` calls `0x43B5B0`. `0x450233` loads the first byte of the pointer at `[edx+4]` and compares it with `0x42`. `0x450245` compares the next byte with `0x4D`. Both matches jump to `0x450497` and leave that pointer, which `0x4501FC` stored from the return of `0x49C1A8`. A mismatch goes to `0x450264`. On that path `0x4503B3` calls `0x49D010` and `0x4503FD` calls `0x467DE8`. `0x450478` stores a replacement pointer at `[edx+4]`. Those three callees were not opened.

The format string at `0x4C2500` is `Textures\%s.bmp`. The `%s` is `[ebp+8]` of `0x47218A`, pushed at `0x4722D4`. That name was not read. `+6` stays unread. The question is at its cap.

## Facilitator check

Re-read `0x4501FC` `mov [edx+4], eax`, `0x450218` `call 0x43B5B0`, `0x450233` `mov cl, [eax]`, `0x450235` `cmp ecx, 0x42`, `0x450245` `mov cl, [eax+1]`, `0x450248` `cmp ecx, 0x4D`, and `0x45025F` `jmp 0x450497`. Re-read `0x4503B3` `call 0x49D010`, `0x4503FD` `call 0x467DE8`, and `0x450478` `mov [edx+4], eax`. The bytes at `0x4C2500` are `Textures\%s.bmp`.

`build/tmp/c1ba-facilitator/check_bitcount.py` walks the function from `0x45015A`. The exe SHA-256 matches `docs/LESSONS.md`. The walk has 217 instructions and no displacement `0x1C`. Expecting the compare immediate `0x41` failed; the file byte is `0x42`. The checker was restored and passed. No Lemmings or Frida process. No source change, so the suite was not rebuilt.

Narrowing: the `42 4D` path does not replace the pointer. The word at offset `0x1C` of that buffer is whatever `0x49C1A8` or `0x43B5B0` put there. This check does not show that buffer is `^TAB.BMP`, and it does not show `GetObjectA` still holds the handle from `0x47255B`.

## Stop

Offline round 3 of 3. The number is unread. The question is parked. The gate is not scored.
