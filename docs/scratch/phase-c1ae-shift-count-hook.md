# C1ae: the shift-count hook is dry-checked at `0x473897`

Queue `c1ae-shift-count-hook`. Offline. No stock start. Worker [Dry-check shift-count hook](ab133fcc-d9e9-4372-8fef-734982ac1ffe). REA was not used. Dispatched 16:59 MDT.

## Result

One read-only `Interceptor.attach`, `onEnter` at `0x473897`, which is `d3 f8` (`sar eax, cl`). The previous instruction, `0x473891`, reloads the sar count from `[ebp-0xae8]` into `ecx`. The shl count is in `[ebp-0xad8]`; its reload is the next instruction, `0x473899`. The mask dword is `[ebp-0x128]`, loaded at `0x4737FA`. From `0x4737EC` through the loop back-edge there is no `call` and no store of the mask. The numeric counts were not read.

## Facilitator check

Re-ran `python build/tmp/c1ae-shift-count-hook/dry_check.py` (exit 0, one attach, `node --check` 0). Re-read `0x47388F..0x4738A1`: `mov al, [edx]`, `mov ecx, [ebp-0xae8]`, `sar eax, cl`, `mov ecx, [ebp-0xad8]`, `shl eax, cl`. Changed only the attach from `0x473897` to `0x473898`; the dry check exited 1 with `missing attach ptr('0x473897')`. Restored. No game process. No source change. The suite on this tree was already 34/34.

## Stop

Round 3 of 3 on the alpha rule. The dry check passed. The live counts are unread. A fourth round may start the game, because those counts are not in the exe and the plate render needs them.
