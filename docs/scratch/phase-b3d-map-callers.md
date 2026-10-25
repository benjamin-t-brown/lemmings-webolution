# B3d: both callers pass their own argument

Queue `b3d-map-callers`. Offline round 4 of `map-draw`. No stock start. Worker [Read the screen-index callers](6f33b250-0e69-44f5-8fee-6f6a1065f23a). `prediction.md` was written before the caller disassembly. REA named the functions. Dispatched 03:01 MDT.

## Result

The two direct calls of `0x48726F` are `0x486FF7` and `0x488713`. Each one's last push is the caller's `[ebp+0xC]`. Neither push is `0x4C416C` or a constant. `0x4886EB` compares that same dword with 3 and skips the call when it is equal. The object stamped with screen id 5 stays unread. The question is parked.

## Facilitator check

Re-read `0x486FE9` through `add esp, 0x14`: the last push is `mov edx, [ebp+0xC]` / `push edx`, then `call 0x48726F`. Re-read `0x4886EB` `cmp [ebp+0xC], 3` / `je 0x48871B`, and `0x48870F` `mov eax, [ebp+0xC]` / `push eax` / `call 0x48726F`. `build/tmp/b3d-facilitator/check_callers.py` scans `.text` for `E8` targets of `0x48726F`. The exe SHA-256 matches `docs/LESSONS.md`. The two calls are `0x486FF7` and `0x488713`. Expecting the displacement `0x08` failed; the file byte is `0x0C`. The checker was restored and passed. No Lemmings or Frida process. No source change, so the suite was not rebuilt.

Narrowing: the runtime dword in either `[ebp+0xC]` was not read. This fourth round was the image source of the jump index, and it is another argument. The creation order that stamps 5 is not in these bytes.

## Stop

Offline round 4 of the map-draw question. The callers do not pass the counter. The question is parked. The gate is not scored.
