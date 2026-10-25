# C1ax: the fill writes the +7 dword as 0

Queue `c1ax-fill-coverage`. Offline round 1 of 3 on `fill-covers-0x140`. No stock start. Worker [Read fill coverage](f74225c9-cfb8-4b72-8d87-043f33b04373). `prediction.md` 01:26:48, before the disassembly. `round-note.md` 01:36:58. REA CLI bounded the functions. Dispatched 01:26 MDT.

## Result

`0x473046` is `push 0x7c`. `0x473048` is `push 0`. `0x47304A` is `lea edx, [ebp-0x18c]`. `0x473051` calls `0x49D760`, which writes that many bytes of the pushed fill value. The interval is `[ebp-0x18c, ebp-0x110)`. `[ebp-0x140]` is byte offset `0x4c`, so the dword is inside the fill and the fill byte is 0.

No instruction from the return at `0x473056` through the byte before `0x473146` stores that dword. The span has no call. The read is not reachable from `0x472F99` without executing `0x473051`. `0x473157` stores the `sete` of `([ebp-0x140] & 4) == 4`, which is 0. That is the context `+7` from c1au. The c1au sentence that the fill does not cover the slot is rejected. The worker's withdrawn "always 0" holds for every path that reaches the read.

Whether a `^tab` upload reaches `0x472F99` is unread. The keep rule was not applied.

## Facilitator check

Re-read `0x473046` for 16 bytes: `6a 7c 6a 00 8d 95 74 fe ff ff 52 e8 0a a7 02 00`. The relative call at `0x473051` targets `0x473056 + 0x2A70A = 0x49D760`. Re-read the callee from `0x49D775`: `edx` is the count, `al` is the fill byte, and `rep stosd` at `0x49D7A1` plus the head and tail byte loops write that count. Re-read `0x473146`: `mov edx, [ebp-0x140]`; `and edx, 4`; `cmp edx, 4`; `sete al`; `mov [ebp-0xd], al` at `0x473157` is `88 45 f3`. Re-read `0x473059`: it stores `0x7c` at `[ebp-0x18c]`, and `0x473063` stores `0x20` at `[ebp-0x144]`, which ends at the first byte of the slot.

`build/tmp/c1ax-facilitator/check_fill.py` walks `0x472F99..0x473B1E` from the entry. The exe SHA-256 matches `docs/LESSONS.md`. The walk found 712 instructions and no indirect jump. Avoiding `0x473051` leaves `0x473146` unreachable. The 44 instructions after the call and before the read contain 0 calls and 0 writes whose interval meets `[ebp-0x140, ebp-0x13c)`. The bytes `c0 fe ff ff` occur once in the function, at `0x473148`. Expecting count `0x7b` failed on the byte at `0x473047`; the file was not modified, and the restored check passed.

`read_steppers.py` checked the exe digest before those listings. No Lemmings or Frida process. No source change, so the suite was not rebuilt.

Narrowing: later stores do overwrite other bytes of the filled record, including `[ebp-0x18c]` and `[ebp-0x144]`. They do not overwrite `[ebp-0x140]`. The loaded 0 is a read of the image on every path that reaches `0x473146`, not a sample of a running `^tab` upload. The `eax == 4` jump at `0x472FBC` goes to `ret 4` at `0x473B1B` and does not compute `+7`.

## Stop

Offline round 1 of the fill-coverage question. `+7` at this read is 0. The kept mask is the next round. The gate is not scored.
