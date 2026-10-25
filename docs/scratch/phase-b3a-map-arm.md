# B3a: screen id 5 is not a compare

Queue `b3a-map-arm`. Offline round 1 of 3 on `map-draw`. No stock start. Worker [Find the map draw call](bbb9b902-1749-4fb5-ba12-0a28f828f98f). `prediction.md` 02:40:41, before the search. `round-note.md` 02:46:39. REA listed the references. Dispatched 02:40 MDT.

## Result

No instruction selects the value 5. `0x4C416C` is used three times, all in `0x4882C0`. `0x488359` loads the dword into `ecx` and `0x48835F` stores it at `[eax+0x2c]`. `0x488362` loads it again, `0x488368` adds 1, and `0x48836B` writes it back. `0x488377` is `ret`. There is no `call` between the load and the return. The file dword at `0x4C416C` is 1. The map draw call was not reached.

## Facilitator check

Re-read `0x488359` for 24 bytes through the store-back, then `mov eax, [ebp-4]`, `mov esp, ebp`, `pop ebp`, `ret` at `0x488377`. `build/tmp/b3a-facilitator/check_id.py` searches the file for `6C 41 4C 00`. The exe SHA-256 matches `docs/LESSONS.md`. The three hits are the immediates at `0x48835B`, `0x488364`, and `0x48836D`. The stored dword is 1. Expecting the add immediate `2` failed; the file byte is `1`. The checker was restored and passed. No Lemmings or Frida process. No source change, so the suite was not rebuilt.

Narrowing: this check does not re-scan every `cmp` with 5 in `.text`. It shows the address itself is not an operand of a compare. The object that receives the pre-increment value 5, and any draw it makes, stays unread.

## Stop

Offline round 1 of the map-draw question. The id is a creation counter. The draw call is the next round. The gate is not scored.
