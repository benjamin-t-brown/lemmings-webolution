# B3g: the filename pushes call 0x49C760

Queue `b3g-box-call`. Offline round 3 of 3 on `level-pick`. No stock start. Worker [Read calls after box pushes](c352f73d-9f82-4814-8886-9800e9183823). `prediction.md` was written before the disassembly. REA was not used. Dispatched 03:25 MDT.

## Result

Each `LEMMINGS.BOX` push is followed by a stack-buffer pointer and `call 0x49C760`. `0x47DF33`, `0x47E2A4`, and `0x47E2D8` are those calls. Each is followed by `add esp, 8`. The other argument is `[ebp-0x110]` at the first site and `[ebp-0x104]` at the other two. No level name and no level index are in those bytes. The callee was not entered. The question is parked.

## Facilitator check

Re-read the three windows. Each is `push` of the string, `lea` of a stack slot, `push` that address, `call 0x49C760`, `add esp, 8`. `build/tmp/b3g-facilitator/check_calls.py` checks the three relative calls. The exe SHA-256 matches `docs/LESSONS.md`. Expecting the first relative byte `0x29` failed; the file byte is `0x28`. The checker was restored and passed. No Lemmings or Frida process. No source change, so the suite was not rebuilt.

Narrowing: the body of `0x49C760` was not read, so these bytes do not say it opens a file. Which of the three pushes runs for a level choice stays unread.

## Stop

Offline round 3 of the level-pick question. The calls do not take a level name. The question is parked. The gate is not scored.
