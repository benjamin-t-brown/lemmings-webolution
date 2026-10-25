# C1av: the skip path stores masked pixels

Queue `c1av-skip-write`. Offline round 3 of 3 on the alpha-count question, one of three parallel rounds. No stock start. Worker [Read skip-path stores](a6a833ee-ee2f-43f3-b34b-857c82bc6cca). `prediction.md` 00:16:33, before the disassembly. REA bounded `0x472F99`. Dispatched 00:15 MDT.

## Result

From `0x47395A` through `ret 4` at `0x473B1B` (span `0x1C4`), the path stores 16-bit pixels through `[ebp-0xad4]`. It does not enter `0x473866`..`0x473955` and does not execute the shifted store at `0x473943`.

`0x473955` is `jmp 0x473866`. `0x473881` leaves that loop with `jae 0x47395A`. `0x4737E6` is `je 0x47395A` when `[object+0x24] & 0x18` is 0. `0x473AF6` jumps to the row step at `0x473792`.

Bits of `[object+0x24]` gate the stores. Bit 0 ANDs the pixel with `[ebp-0xad0]` at `0x4739A6` and, when the word is nonzero, ORs the low word of `[ebp-0x128]` at `0x4739CE`. Bit 1 does the same AND at `0x473A32` and ORs at `0x473A70` unless the word is `0x1CE7` or `0x2108`. Bit 2 has no AND; `0x473AE2` ORs the mask word when the pixel word is nonzero. Each loop then adds 2 to the pointer.

## Facilitator check

Re-disassembled the entry, the three store windows, the row step and the epilogue branch with `read_steppers.py`. `0x473955` is `e9 0c ff ff ff`, `0x473881` is `jae 0x47395A`, `0x4737E6` is `je 0x47395A`, and `0x473943` is `66 89 11`, which this landing skips. `0x473960` is `and ecx, 1`, `0x4739E8` is `and edx, 2`, and `0x473A8D` is `and edx, 4`. The AND immediate displacements are `[ebp-0xad0]`; the OR displacements are `[ebp-0x128]`. `0x473A40` compares `0x1CE7` and `0x473A52` compares `0x2108`. `0x473AF6` is `jmp 0x473792`. `0x4737B3` compares `[ebp-0x138]` with `0x10` and `0x4737BA` jumps to `0x473AF6` when it differs; the equal arm stores `[ebp-0x168] + [ebp-0xacc] * [ebp-0x17c]` at `[ebp-0xad4]`. `0x4737AD` is `jae 0x473AFB`. `0x473B1B` is `ret 4`.

A separate read of the exe file checked 18 sites and found no branch from `0x47395A`..`0x473B1E` into `0x473866`..`0x473955`. Expecting `66 89 11` at `0x4739A6` failed; the file byte is `66 89 10`. Restored, the same check passed. `prediction.md` is timestamped before `round-note.md`. No Lemmings or Frida process. No source change, so the suite was not rebuilt.

Narrowing: `[ebp-0x128]` is the mask dword C1ae named. This round does not read its value, and a zero word still takes bit 1's OR. The shifted sample stays on the other side of `0x47395A`. The c1as startup values 1 and 2 select bit 0 and bit 1 for those other uploads; they are not a `^tab` measurement. The numeric mask and the live store counts stay unread.

The worker's queue update opened `build\docs\tasks.json` because the helper used `parents[1]`. The helper now uses the repo root. It is gitignored and was not part of the claim.

## Stop

Offline round 3 of the alpha-count question, the first of the three parallel rounds to land. The skip path's stores are read. The gate is not scored.
