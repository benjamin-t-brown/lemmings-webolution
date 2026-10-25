# C1ar: the exit code is read inside the detach callback

Queue `c1ar-exit-capture`. Offline. No stock start. Worker [Repair exit-code capture](be109f46-d4a4-46db-a956-3394e41498d9). REA was not used. Dispatched 23:24 MDT.

## Result

The copied probe calls `read_exit_code` as the first statement of the first `on_detached`. The mask hook logs `skip_input`, the dword at `[[ebp+8]+0x24]`. The four attaches are unchanged. The dry check exits 0. No stock start. Pid 25964's exit code stays unread.

## Facilitator check

Re-disassembled `0x472F99` through `0x4737E6`. `0x472F99` is `push ebp` and `0x472F9A` is `mov ebp, esp`. Every `[ebp+8]` operand in that span is a load. `0x47315E` loads `[ebp+8]`, `0x473161` loads `[ecx+0x24]`, `0x473164` masks `0x18`, and `0x47316D` stores that boolean byte at `[ebp-0x10]`. `0x4737DB` loads `[ebp+8]` again, `0x4737DE` loads `[edx+0x24]`, `0x4737E1` masks `0x18`, and `0x4737E6` jumps to `0x47395A` when the result is 0. The byte at `0x4737DB` is `8B`.

`python build/tmp/c1ar-exit-capture/dry_check.py` exited 0. Removing only `read_exit_code(STATE.get("pid"))` from `on_detached` exited 1 with `exit-code read is not inside the detach callback`. Restored; SHA-256 `b79882a5fdc2146f9a732ac5313d6158e01c129d224eaf2c019595b540b59f75` matches; the same command exited 0. No Lemmings or Frida process. No source change, so the suite was not rebuilt.

Narrowing: the hook logs the full dword. The branch tests that dword AND `0x18`. The branch reloads the object later, and this check does not show the field is unchanged between `0x4736B6` and `0x4737DE`. The callback read has not run against a live process.
