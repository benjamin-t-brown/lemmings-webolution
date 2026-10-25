# C1ay: +6 reads a GetObjectA word

Queue `c1ay-kept-format`. Offline round 1 of 3 on `kept-format`. No stock start. Worker [Apply kept format](de50f1fd-f708-439e-99ca-81176414cb9e). `prediction.md` 01:44:54, before the disassembly. `round-note.md` 01:49:08. REA bounded the functions. Dispatched 01:43 MDT.

## Result

Context `+6` is unread. `0x47302D` takes the address of `[ebp-0x1a4]`, `0x473034` pushes `0x18`, `0x47303C` loads `[eax+4]`, and `0x473040` calls `[0x4B005C]`. That slot names `GetObjectA`. `[ebp-0x192]` is byte `+0x12` of the `0x18`-byte buffer. `0x473179` masks that dword with `0xFFFF`, compares it with 8, and stores the `setle` at `[ebp-0xe]`.

No instruction in `0x472F99` writes that displacement. The numeric word is whatever `GetObjectA` writes. The c1as log was not applied. `0x472518` is `mov cx, [eax+0xe]` in another function. It does not store `[ebp-0x192]`.

## Facilitator check

Re-read `0x47302D` for 25 bytes through `call dword ptr [0x4B005C]`, and the next bytes `6a 7c` at `0x473046`. The dword at `0x4B005C` is `0x000BD71E`, and the name at `0x4BD720` is `GetObjectA`. Re-read `0x473179`: `mov ecx, [ebp-0x192]`; `and ecx, 0xffff`; `cmp ecx, 8`; `setle dl`; `mov [ebp-0xe], dl`. Re-read `0x472513`: `mov eax, [ebp-0x2c]`; `xor ecx, ecx`; `mov cx, [eax+0xe]`.

`build/tmp/c1ay-facilitator/check_plus6.py` checks those bytes. The exe SHA-256 matches `docs/LESSONS.md`. `-0x1a4 + 0x12` is `-0x192`, and `-0x1a4 + 0x18` is `-0x18c`, so the word sits in the buffer and the c1ax fill starts at the buffer's end. The bytes `6e fe ff ff` occur once in `0x472F99..0x473B1E`, at `0x47317B`. Expecting the name `GetObjectB` failed; the file was not modified, and the restored check passed.

`read_steppers.py` checked the exe digest. No Lemmings or Frida process. No source change, so the suite was not rebuilt.

Narrowing: the call's `eax` is not tested. This check does not re-derive where `[eax+4]` was stored, or the `CreateDIBSection` call the worker reported in the other function. The kept record stays unread.

## Stop

Offline round 1 of the kept-format question. The compare's word is `GetObjectA`'s. Its number is not. The gate is not scored.
