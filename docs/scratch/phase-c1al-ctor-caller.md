# C1al: the upload vtable is constructed at startup, and the upload call is not on that chain

Queue `c1al-ctor-caller`. Offline. No stock start. Worker [Follow upload vtable constructor](eef380ee-28fe-4452-aeff-96738a211c98). `prediction.md` expected unnamed. REA was used. Dispatched 18:44 MDT.

## Result

No call on the chain from `0x46F16F` reaches `0x471720`. The phase of `0x471746` stays unnamed.

The constructor that leaves vtable `0x4B19E8` does run at process startup. `0x49D5AB` is `e8 95 fd ff ff` (`call 0x49D345`) inside the PE entry `0x49D518`. Both arms of `0x49D57A` reach it, and there is no jump between `0x49D584` and that call. `0x49D345` pushes `[0x4BF000, 0x4BF0DC)` and calls `0x49D44B`, which calls each nonzero dword. `0x4BF0B0` is inside that range and holds `0x468D20`. That function calls `0x468D2F`, then `0x468D37` calls `0x468D5F` with `ecx = 0x58CB40`, and `0x468D7E` calls `0x46F14D`. `0x46F16F` is `add ecx, 8` / `call 0x474380`. None of those calls is `0x471720`.

`0x49D345` has one direct caller, `0x49D5AB`. `0x468D20` has none. `0x474380` has one, `0x46F16F`. The bytes `20 8d 46 00` occur only at `0x4BF0B0`.

## Facilitator check

Re-read the twelve instructions in the chain, the walker at `0x49D44B`, and the entry from `0x49D518` through `0x49D5AB`. `0x4BF0B0` is `0x468D20` and lies in `[0x4BF000, 0x4BF0DC)`. An `E8` scan found one caller of `0x49D345` and one of `0x474380`, and none of `0x468D20`. The calls inside `0x46F14D` and `0x468D20` do not target `0x471720`. Expecting `e8 95 fd ff fe` at `0x49D5AB` failed; the bytes are `e8 95 fd ff ff`. The exe was not written. No source change, so the suite was not rebuilt. No Lemmings process.

Narrowing: startup constructs the object. It does not call the upload. `0x58CB40` is past the file-backed image, so the live vtable was not read. The shift counts stay unread.

## Stop

Alpha-count round 3 of 3 this run. The upload call is still unplaced in time, so no stock start. The question is at its cap.
