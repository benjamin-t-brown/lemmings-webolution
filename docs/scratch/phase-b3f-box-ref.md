# B3f: three pushes of `LEMMINGS.BOX`

Queue `b3f-box-ref`. Offline round 2 of 3 on `level-pick`. No stock start. Worker [Find lemmings.box reference](d3562828-7a82-46b3-81e2-92a1cb5513b2). `prediction.md` was written before the search. REA CLI found the uppercase bytes. Dispatched 03:12 MDT.

## Result

ASCII and UTF-16LE `lemmings.box` are absent. Three ASCII `LEMMINGS.BOX` strings are at `0x4C292C`, `0x4C2A60`, and `0x4C2A70`. Each address is pushed once: `0x47DF27`, `0x47E298`, and `0x47E2CC`. Those pushes are not a level click. The calls beside them were not opened.

## Facilitator check

`build/tmp/b3f-facilitator/check_box.py` hashes the exe. The SHA-256 matches `docs/LESSONS.md`. Lowercase and UTF-16LE `lemmings.box` are absent. Each of the three addresses is the 13 bytes `LEMMINGS.BOX` and a NUL, and each address dword occurs once, as the immediate of its `push`. Re-read the three pushes with `read_steppers.py`. Expecting the first byte `0x4D` failed; the file byte is `0x4C`. The checker was restored and passed. No Lemmings or Frida process. No source change, so the suite was not rebuilt.

Narrowing: which push runs for a level choice was not read. The calls after the pushes were not opened.

## Stop

Offline round 2 of the level-pick question. The filename has three pushes. The call after each push is the next round. The gate is not scored.
