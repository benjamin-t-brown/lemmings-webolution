# C2n: two instructions load the `_curExit` slot

Queue `c2n-cursor-read`. Offline round 3 of 3 on the cursor clause. No stock start. Worker [Find _curExit slot reader](da642337-beab-4c72-8a05-e19f69c0844a). `prediction.md` 02:29:54, before the search. `round-note.md` 02:37:36. REA's byte search matched the file scan. Dispatched 02:29 MDT.

## Result

The bytes `34 C9 01 00` occur three times. `0x45E8AD` is the landed store. `0x45ECBF` and `0x45EE3A` are `mov eax, [edx+0x1C934]`. The first pushes `eax` and calls `0x414B90`. The second does the same and calls `0x414D20`, and `0x45ED95` jumps to `ret` at `0x45F1A9` when the byte at `[object+0x1C9B1]` is nonzero. The direct caller of that function, `0x450A74`, sets `ecx` to `0x4C6A08`. Neither callee was opened. This is not a hover rule. The question is at its cap.

## Facilitator check

Re-read `0x45ECBF` `8b 82 34 c9 01 00`, then `push eax`, `mov ecx, 0x58CB48`, `call 0x414B90`. Re-read `0x45EE37` `mov edx, [ebp-4]` and `0x45EE3A` the same load. Re-read `0x45ED8D` `mov cl, [eax+0x1C9B1]`, `0x45ED95` `jne 0x45F1A9`, and `0x45F1A9` `mov esp, ebp` / `pop ebp` / `ret`. Re-read `0x450A6F` `mov ecx, 0x4C6A08` / `call 0x45ED81`.

`build/tmp/c2n-facilitator/check_reads.py` searches the file for `34 C9 01 00`. The exe SHA-256 matches `docs/LESSONS.md`. The three hits are the store and the two loads. The relative calls resolve to `0x414B90` and `0x414D20`. Expecting the load opcode `0x89` failed; the file byte is `0x8B`. The checker was restored and passed. No Lemmings or Frida process. No source change, so the suite was not rebuilt.

Narrowing: the second load is skipped when `[object+0x1C9B1]` is nonzero, and that byte was not read. A full branch map of `0x45E51F` was not built. The callees were not opened, so the sprite choice stays unread.

## Stop

Offline round 3 of 3. The slot has two readers. The hover rule was not read. The question is parked. The gate is not scored.
