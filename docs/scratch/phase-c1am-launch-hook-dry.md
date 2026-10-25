# C1am: the launch-time alpha hooks dry-check

Queue `c1am-launch-hook-dry`. Offline. No stock start. Worker [Dry-check launch hooks](7d2306d3-207a-4081-9b59-e29e9cfea8ed). REA was not used. `prediction.md` 21:25, `hook.js` 21:29. Dispatched 21:20 MDT.

## Result

One read-only script appends a JSON line from each `onEnter` at `0x473B1E`, `0x472F99`, `0x4736B6` and `0x473897`. The probe source calls `frida.spawn`, loads `hook.js`, then `frida.resume`, and reads `hits.log` after the pause. The dry check exited 0. No stock start. `python` has no `frida` module, so spawn was not executed.

## Facilitator check

Re-ran `python build/tmp/c1am-launch-hook-dry/dry_check.py` (exit 0, `node --check` inside it). Re-read the four sites. `0x473B1E` is `55` `push ebp`, then `8b 45 0c` and `83 7d 08 00`, so the record and context are `[ebp+8]` and `[ebp+0xc]` after the prologue and `[esp+4]` and `[esp+8]` at `onEnter`. `0x473D97` is `c2 08 00`. `0x472F99` is `55` `push ebp`. `0x47241A` is `e8 7a 0b 00 00` `call 0x472F99`, and the push and `ecx` just above it are the same `[ebp-0x588]`. `0x473B1B` is `c2 04 00`. `0x4736B6` is `83 bd d8 fe ff ff 00` `cmp dword ptr [ebp-0x128], 0`, then `0f 84 55 04 00 00` `je 0x473B18`. `0x473891` is `mov ecx, [ebp-0xae8]` and `0x473897` is `d3 f8` `sar eax, cl`. Changed only the `0x473897` attach to `0x473898`; the dry check exited 1 with `missing attach ptr('0x473897')`. Restored; the same command exited 0. No Lemmings process. No source change, so the suite was not rebuilt.

Narrowing: the dry check is the script and the exe bytes. `import frida` raises `ModuleNotFoundError`. `rea` is installed and C1s used `rea instrument-with-frida attach`, which attaches to a process that is already running. Whether `frida.spawn` of this exe stays suspended through `script.load` is unproved. The known upload argument is an object pointer, not a texture name.

## Stop

Round 1 of 3 on the launch-attach read. The dry check passed. The live counts are unread. A start waits until `import frida` works, because this probe cannot spawn without it.
