# B2j: level-to-map preflight, one handler-to-store path read, no input end named

2026-10-23. Directive Q3 round 1 (preflight). Packet: `build/tmp/b2j-level-to-map-preflight/` (`round-note.md`,
`prediction.md`, scan scripts). Zero stock starts, no debugger, capstone only (no REA). About 14 functions read
against a cap of 12 (surplus was small leaf stubs).

## Result (READ, not observed execution)

- **Path A, handler to construction:** screen 0x20's handler `0x486510` (vtable `0x4B1CB0` slot 11) compares the
  event id with `[screen+0x48]` (`0x486599`), zeroes `[0x5D59E0]` (`0x48659E`) and calls vtable slot 3 `0x4886E4` with
  selector 0x23 (`0x4865AC..0x4865BA`). `0x4886E4` calls the factory `0x48726F`; selector 0x23 is case `0x4878E8`,
  ctor `0x49AA90`, vtable `0x4B3EB8`.
- **0x23 is the map** only by inference: `0x4B3EB8` is the head node B2h observed after the map-node click. No label
  in the exe says so.
- **The 0x1F dialog does not lead to the map.** Its handler `0x48F558` builds 9 / 0x19 or sets flags
  (`0x4CBFA9/AA/AC`), never 0x23.
- **Dead end:** the main-loop `[level+0x55A2]` block (`0x45D606`, builds 0x23) has no setter anywhere in `.text`.
- **Stacking candidate S (READ only):** `0x4886E4` calls the factory without the `0x4884F4` call on the old head that
  `0x486F60` makes; the factory tail links the new node onto the old one. Whether the old node survives
  (`0x487B4B`, `0x44521B`, `0x488378`) is unread.
- **Unread:** the ctor `0x4862F0` control ids, the producer of the event id (`0x48896A`), and the condition that builds
  screen 0x20 (only builder `0x45E0A9`, trigger at `0x45E086`, unread; level end is a guess).
- By the brief's rule neither A nor S is a named key or click path, so the b2k live start was **not** recommended.

## Facilitator check

Re-read on mouv with `read_steppers.py`: `0x486510` (prologue, call to `0x48896A`), `0x486590..0x4865BD`
(`jne 0x4865c6`, `mov [0x5d59e0],0`, `push 0x23`, `call [edx+0xc]`), `0x4886E4` (`cmp [ebp+0xc],3`,
`[this+0x14]=1`, `[this+0x1C]=arg`, push 1 then the factory args), `0x48726F` (`cmp [ebp-0xb4],0x24`), `0x4878E8..`
(`call 0x49aa90`). Own raw-file read of vtable slots: `0x4B1CB0` slot 3 `0x4886E4`, slot 11 `0x486510`; `0x4B26B0` slot
11 `0x48F558`; `0x4B3EB8` slot 3 `0x4886E4`; factory byte table index for 0x23 is 20, jump entry `0x4878E8`. Not
re-verified by me: the `[+0x55A2]` no-setter scan, the leaf stubs, the side hypothesis about the +2 counter step.
Narrowed: "named" is not claimed for A or S.
