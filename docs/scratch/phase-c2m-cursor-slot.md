# C2m: `_curExit` is stored at 0x4E333C

Queue `c2m-cursor-slot`. Offline round 2 of 3 on the cursor clause. No stock start. Worker [Read cursor register slot](a4525208-4a9d-4f2d-99d8-c8aae20721c8). `prediction.md` 02:22:25, before the disassembly of `0x45FE10`. `round-note.md` 02:27:16. REA bounded the caller. Dispatched 02:21 MDT.

## Result

`0x45E8A5` calls `0x45FE10`. `0x45E8AA` reloads `edx` from `[ebp-8]`, and `0x45E8AD` stores `eax` at `[edx+0x1C934]`. `[ebp-8]` is the `ecx` saved at `0x45E525`. The one direct call, `0x450895`, sets that `ecx` to `0x4C6A08`. The slot is `0x4C6A08 + 0x1C934` = `0x4E333C`. `.data` file bytes end at `0x4C6000`, so the runtime dword is not in the image. No reader of the slot was opened.

## Facilitator check

Re-read `0x45E8A5` for 14 bytes: `e8 66 15 00 00 8b 55 f8 89 82 34 c9 01 00`. Re-read `0x45E525` `mov [ebp-8], ecx` and `0x450890` `mov ecx, 0x4C6A08` / `call 0x45E51F`. Re-read `0x45FF35` `mov eax, [ebp-0x10]` and `0x45FF45` `ret 4`.

`build/tmp/c2m-facilitator/check_slot.py` checks those bytes. The exe SHA-256 matches `docs/LESSONS.md`. A scan of direct `E8` calls finds one target of `0x45E51F`, at `0x450895`. A disassembly of `0x45E51F` through `0x45ED80` finds one write of `[ebp-8]`, at `0x45E525`. `0x4E333C` is past `0x4C6000`. Expecting displacement byte `0x35` failed; the file byte is `0x34`. The checker was restored and passed. No Lemmings or Frida process. No source change, so the suite was not rebuilt.

Narrowing: the prediction that the base was still `0x58CB48` is rejected by the reload of `[ebp-8]`. Indirect calls of `0x45E51F` were not scanned. The earlier writes of the returned `[ebp-0x10]` were not re-counted. A reader of `0x4E333C` was not opened, so this is not the hover rule.

## Stop

Offline round 2 of the cursor clause. The registration slot is `0x4E333C`. The hover rule is the next round. The gate is not scored.
