# C1ac: the mask slot is filled by a 32-byte copy

Queue `c1ac-alpha-mask-writer`. Offline. No stock start. Worker [Find alpha mask writer](f28aa15c-58f4-450b-92d2-3e3964c3954b). One new body, `0x49D010`. REA was used for bounds. Dispatched 16:44 MDT.

## Result

The upload at `0x472F99` never stores `[ebp-0x128]`. Its five uses are reads: `cmp` at `0x4736B6`, `mov` at `0x4737FA`, and three `or`s at `0x4739C1`, `0x473A63`, and `0x473AD5`.

`[ebp-0x128]` is `[ebp-0x144+0x1c]`. `0x473176` stores `lea [ebp-0x144]` at `[ebp-4]`. The callback context is `lea [ebp-0x14]`, so `context+0x10` is that pointer. `0x473C95`, on the arm that stores the popcount, calls `0x49D010` with count `0x20`, source `[ebp+8]`, and that destination. `0x473BCE` reads `[source+0x1c]` before the copy.

`0x49D043` is `rep movsd`. `0x49D1C4` is the overlap `rep movsd`, taken when the destination lies inside the source. Both write `dest+0x1c`. Which one runs depends on the source pointer from `call [eax+0x20]`, and that method is not in the exe.

The popcount reads the source dword. The shift reads the destination. They are not the same dword. The next call that receives `lea [ebp-0x18c]` is `call [edx+0x18]` at `0x473246`. Later calls at `0x4732B2`, `0x47331D`, and `0x4736F9` (`[edx+0x64]`) also receive that record. `0x4736F9` is after the zero test and before the load at `0x4737FA`. No numeric mask.

## Facilitator check

Re-ran `python build/tmp/c1ac-alpha-mask-writer/mask_writer.py` (exit 0, the four-line result above). An independent disassembly of `0x472F99` for 2949 bytes found the same five uses and no store of `[ebp-0x128]`. Re-read `0x49D010`: `rep movsd` at `0x49D043`, and the overlap `rep movsd` at `0x49D1C4` after `std`. Re-read `0x4736DB..0x4736F9`: `lea [ebp-0x18c]`, then `call [edx+0x64]`. Changed the checker's expected `0x49D043` bytes from `f3 a5` to `f3 a4`; the command exited 1. Restored. No game process. No source change. The suite on this tree was already 34/34.

Narrowing: the copy is the in-exe writer. Whether `0x473810` still counts that copy is unread, because `0x4736F9` sits between the early read and the shift load and its body is not in the exe.

## Stop

Round 2 of 3 on the alpha rule. The numeric shift counts are not in these bytes.
