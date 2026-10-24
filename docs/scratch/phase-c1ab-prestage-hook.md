# C1ab: the plate hook samples the stage caches before the stores

Queue `c1ab-prestage-hook`. Offline. No stock start. Worker [Dry-check pre-stage hook](a1241aef-628b-46ea-86a5-e56ba906e7b8). `prediction.md` written before `hook.js`. REA was not used. Dispatched 16:27 MDT.

## Result

Two read-only `Interceptor.attach` calls, both `onEnter`, in one script. `0x414944` is `e8 d6 dc 05 00` (`call 0x47261F`). On a plate hit (`[ebp+4] == 0x45B66F`) the script reads the descriptor at `[ebp-4]`, the flags at `[descriptor+0x24]`, and the caches `[0x5CD5F4]` and `[0x4C24E4]`. `0x41494F` is `e8 2c 04 00 00` (`call 0x414D80`). That second `onEnter` reads the blend selector at `[esp]` and `[0x4C227C]`. The script has no `onLeave`. The live values were not observed.

From `0x414920` through `0x414944` the bytes are straight-line code: one call to `0x414A10` that returns to `0x41493E`, then the call at `0x414944`. The first branch in this function is `0x41496E` (`jne 0x414996`), after both attaches.

## Facilitator check

Re-disassembled `0x414920..0x41496E` and `0x47261F..0x47267A` with `read_steppers.py`. The straight line, both calls, and both stores match the note: `0x472637` is `mov [0x5CD5F4], ecx` and `0x472671` is `mov [0x4C24E4], ecx`. Re-ran `python build/tmp/c1ab-prestage-hook/dry_check.py` (exit 0, two attaches, `node --check` 0). Changed only the descriptor attach from `0x414944` to `0x414945`; the dry check exited 1 with `missing attach ptr('0x414944')`. Restored. The file again contains `0x414944`, does not contain `0x414945`, and has no CR. `ctest` 34/34. No source change. No game process.

Narrowing: the callee can skip the stores. `0x47262E` is `je 0x472975` when the descriptor equals `[0x5CD5F4]`, which skips `0x472637` and the rest of the body. The texture bind `call [edx+0x98]` is at `0x472653`, before the flag compare. `0x472665` is `je 0x472975` when `[0x4C24E4]` equals `[descriptor+0x24]`, which skips `0x472671`. The sample is still before both stores, because `onEnter` at `0x414944` runs before the call. A live log can compare the four recorded dwords with those two tests. It does not, by itself, show that the stage writes ran. `0x472975` was not re-read. `[0x5CD5F4]` is past the file image; the address is the store immediate.

## Stop

Round 1 of 3 on this question. The dry check passed. The live selector, flags, and both caches are unread. The stock start was not spent.
