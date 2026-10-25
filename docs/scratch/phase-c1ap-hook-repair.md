# C1ap: the count hook moves off the shift

Queue `c1ap-hook-repair`. Offline. No stock start. Worker [Repair alpha-count hook](dcbb693e-98ca-492b-8295-47ac24aa8fd9). REA was used. Dispatched 23:03 MDT.

## Result

No control-flow target falls strictly inside the bytes a hook at `0x473897` overwrites. The copied probe attaches the count read at `0x47383C`, after both shift counts are stored and before the per-pixel loop. The dry check exits 0. The early exit of pid 20612 stays unnamed. No stock start.

## Facilitator check

Re-disassembled `0x4737C0` through `0x47395A`. `0x473897` is `d3 f8` (`sar eax, cl`, 2 bytes). `0x473899` is `mov ecx, [ebp-0xad8]` (6 bytes). `0x473955` is `jmp 0x473866`. `0x47381B` is `jne 0x47383C`. `0x4737F4` stores `[ebp-0xae8]`. `0x473806` stores 0 into `[ebp-0xad8]`. `0x4737E6` jumps to `0x47395A` when `[object+0x24] & 0x18` is 0, which skips this site and the `sar` together.

An independent Capstone sweep of every relative branch in `.text` found no target in `0x473898`..`0x47389E` and none in `0x47383D`..`0x473841`. A dword search of those addresses found none. The store encoding `89 8d 18 f5 ff ff` occurs once, at `0x4737F4`.

Re-read `0x47220E`: `push [ebp+8]`, `lea ecx, [ebp-0x348]`, `call 0x49c750`. `0x4721E5` stores 0 at `[object]`. File strings: `0x4C0E00` is `^tab`, `0x4C0E08` is `^tab_selected`, `0x4C24E8` is `Lo`, `0x4C24EC` is `Hi`.

`python build/tmp/c1ap-hook-repair/dry_check.py` exited 0. Changing only `ptr('0x47383C')` to `ptr('0x47383D')` exited 1 with `missing attach ptr('0x47383C')`. Restored; SHA-256 `9f4ea08963bc33c9bad6350ac3d9a0a5d31bb5d9ffb6e4c0841a6cc9d7936e51` matches the pre-mutation file; the same command exited 0. No Lemmings or Frida process. No source change, so the suite was not rebuilt.

Narrowing: `0x4737EC` is `mov ecx, 8`, before either store. The account that a branch into the old patch killed pid 20612 is false on these bytes. That process's exit code was not captured. `0x47383C` itself is `mov eax, [ebp-0xacc]`; the hook reads the counts from the frame there, once per call. The call at `0x47241A` was not re-derived.
