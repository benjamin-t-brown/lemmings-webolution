# C2l: `_curExit` is a registrar push

Queue `c2l-cursor-exit`. Offline round 1 of 3 on the cursor clause. No stock start. Worker [Find _curExit reference](bdcf8f24-084b-4419-961e-e6aa8920fd94). `prediction.md` 02:15:55, before the search. `round-note.md` 02:19:17. REA found the string and the xref. Dispatched 02:15 MDT.

## Result

`_curExit` is at `0x4C0DD0`, bytes `5F 63 75 72 45 78 69 74 00`. The dword `D0 0D 4C 00` occurs once in the image, as the immediate of `0x45E89B` `push 0x4C0DD0`. The next instructions are `mov ecx, 0x58CB48` and `call 0x45FE10`. The push does not name `_curLem` (`0x4C0D84`) or `_curNoLem` (`0x4C0D78`). It is a registrar push, not an exit-hover test.

## Facilitator check

Re-read `0x45E89B` for 15 bytes: `68 d0 0d 4c 00 b9 48 cb 58 00 e8 66 15 00 00`. The call's next address plus `0x1566` is `0x45FE10`. The nine bytes at `0x4C0DD0` are `_curExit` and a NUL. `0x4C0D78` is `_curNoLem` and `0x4C0D84` is `_curLem`.

`build/tmp/c2l-facilitator/check_exit.py` searches the file for `D0 0D 4C 00`. The exe SHA-256 matches `docs/LESSONS.md`. The only hit is the push immediate at `0x45E89C`. Expecting the `x` byte to be `0x79` failed; the file byte is `0x78`. The checker was restored and passed. No Lemmings or Frida process. No source change, so the suite was not rebuilt.

Narrowing: a later choice that uses a handle and never stores this address was not read. The body of `0x45FE10` was not re-read. Nothing was drawn.

## Stop

Offline round 1 of the cursor clause. The string has one registrar reference. The hover rule is the next round. The gate is not scored.
