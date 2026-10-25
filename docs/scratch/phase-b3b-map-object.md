# B3b: the screen creations are not in one order

Queue `b3b-map-object`. Offline round 2 of 3 on `map-draw`. No stock start. Worker [Find the id-5 creation](eddd7231-4826-4fb5-8940-b51288844dd9). `prediction.md` 02:49:06, before the call search. `round-note.md` 02:54:13. REA listed the callers. Dispatched 02:48 MDT.

## Result

The direct calls of `0x4882C0` are not one straight-line order, so none is shown to stamp 5. `0x48729D` compares `[ebp-0xb4]` with `0x24` and `0x4872A4` jumps to `0x4879BB` when the index is above that. Otherwise `0x4872B8` is `jmp dword ptr [ecx*4+0x487AB3]`, after loading a selector byte from `[index+0x487B0F]`. There are 20 direct calls. The vtable of the object that receives 5 stays unread.

## Facilitator check

Re-read `0x48729D` for 34 bytes through the indirect jump. The first table dword at `0x487AB3` is `0x4872BF`. `build/tmp/b3b-facilitator/check_table.py` scans `.text` for `E8` targets of `0x4882C0`. The exe SHA-256 matches `docs/LESSONS.md`. The scan finds 20 calls. Expecting the compare immediate `0x25` failed; the file byte is `0x24`. The checker was restored and passed. No Lemmings or Frida process. No source change, so the suite was not rebuilt.

Narrowing: the per-arm caller list in the worker's note was not re-walked. The indirect jump is enough to reject one fall-through order. What writes `[ebp-0xb4]` was not read, so the table index is not shown to be the creation counter.

## Stop

Offline round 2 of the map-draw question. The stamp of 5 is unread. The index writer is the next round. The gate is not scored.
