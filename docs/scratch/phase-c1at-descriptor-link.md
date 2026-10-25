# C1at: plate pointer and upload object are different loads

Queue `c1at-descriptor-link`. Offline round 1 of 3 on the alpha-count question, parallel with c1au and c1av. No stock start. Worker [Link plate descriptor](1afb5306-7fca-4c84-99f8-0ff628dec533). `prediction.md` 00:16:49, before the disassembly. REA bounded the functions. Dispatched 00:15 MDT.

## Result

`[ebp-4]` at `0x414944` and `[ebp+8]` in `0x472F99` are loaded from different stores. Both paths use offset `+0x24`. No instruction on the paths read copies one pointer into the other.

The plate side calls `0x414A10` and stores the return at `[ebp-4]` (`0x41493E`), then passes it to `0x47261F`. The upload side allocates `0x28` bytes (`0x47448E` `call 0x49C1A8`) and `0x47220B` writes `[object+0x24]` from the name-prefix flag. A first byte of `0x5E` stores 8 at `0x474471`. `0x47241A` passes that allocation to `0x472F99`, and `0x472435` returns it.

## Facilitator check

Re-disassembled `0x414920` through the call, `0x4721A8`, `0x47220B`, `0x47241A`, `0x472435`, and `0x47446C` through `0x4744B1`. `0x414944` is `e8 d6 dc 05 00` (`call 0x47261F`). `0x41493E` stores the lookup return at `[ebp-4]`. `0x47220B` is `mov [ecx+0x24], edx` with `edx` from `[ebp+0xc]`. `0x47446C` compares the first byte with `0x5E`, and `0x474471` stores 8. `0x47448E` is `call 0x49C1A8` after `push 0x28`. `0x4744B1` is `call 0x47218A`. `0x47241A` is `call 0x472F99`. `0x472435` loads `[ebp-0x588]` into `eax` and `0x472448` is `ret 8`.

A separate read of the exe file checked those 10 sites. Expecting a zero immediate at `0x474471` failed; the file byte is 8. Restored, the same check passed. `prediction.md` is timestamped before `round-note.md`. No Lemmings or Frida process. No source change, so the suite was not rebuilt.

Narrowing: this is "same field, different load" on the paths read. It does not close the object identity. `0x414FA0`, `0x414CC0`, and the factory call at `0x414E98` were not disassembled, so an insert of the `0x28`-byte pointer into the pool node is unread. The pool-stride formula and the A4t material name were not re-derived. c1ad's live flags value 8 is a separate paused-start measurement; the immediate 8 here is the `^` prefix on the upload object.

## Stop

Offline round 1 of the alpha-count question. The two pointers are different loads. The gate is not scored.
