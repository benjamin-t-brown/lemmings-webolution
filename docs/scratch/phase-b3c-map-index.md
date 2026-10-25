# B3c: the jump index is a stack argument

Queue `b3c-map-index`. Offline round 3 of 3 on `map-draw`. No stock start. Worker [Read the table index](a549f4a8-0dcd-43c7-b684-fabab660bd35). `prediction.md` was written before the disassembly. REA was not used. Dispatched 02:56 MDT.

## Result

`0x487294` loads `eax` from `[ebp+8]`, and `0x487297` stores it at `[ebp-0xb4]`. The next instruction is the compare with `0x24`. Nothing in that prologue has the operand `0x4C416C`. The index is the caller's argument. It is not the creation counter. The object that receives screen id 5 stays unread.

## Facilitator check

Re-read `0x48726F` through the compare. The bytes at `0x487294` are `8b 45 08 89 85 4c ff ff ff`. `build/tmp/b3c-facilitator/check_index.py` checks that sequence and that the prologue window contains no `6C 41 4C 00`. The exe SHA-256 matches `docs/LESSONS.md`. Expecting the argument displacement `0x0C` failed; the file byte is `0x08`. The checker was restored and passed. No Lemmings or Frida process. No source change, so the suite was not rebuilt.

Narrowing: callers were not opened, so the argument's value is unread. A later store of `[ebp-0xb4]` on another path was not searched. The jump table was not re-read.

## Stop

Offline round 3 of the map-draw question. The index is `[ebp+8]`. A fourth round reads the two direct callers, because that is the remaining image source of this argument. The gate is not scored.
