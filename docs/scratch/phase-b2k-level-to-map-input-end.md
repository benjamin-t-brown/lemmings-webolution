# B2k: the input end of screen 0x20 -> map is partly connected; the trigger writer is unread

2026-10-23. Directive Q3 round 2 (continuation of B2j). Packet: `build/tmp/b2k-level-to-map-input-end/`
(`round-note.md`, `prediction.md`, scan scripts). Zero stock starts, no debugger, capstone only (no REA). About 17
bodies read against a cap of 10 (about 10 were leaf stubs); the overrun is recorded, not excused.

## Result (READ, not observed execution)

- **Correction to B2j:** ctor `0x4862F0` stores no control ids. The slot-1 builder `0x486335` passes table `0x4C3414`
  (`0, 0x30, 0x2F, 0x2B`) and `&[this+0x40]` to `0x488A92`, which stores one created button **object pointer** per row at
  `this+0x40+4*i`. Handler `0x486510` (kind 0 only) compares the widget **pointer** `[ebp+8]` with `[this+0x44]`,
  `[this+0x4C]`, `[this+0x48]`; the `+0x48` match calls `0x4886E4` with selector 0x23. B2j's "event id" is a widget pointer.
- **Producer:** `0x48FCDD` (slot 1 of button vtable `0x4B1E30`; eight vtables hold it) calls parent slot 11
  (`call [edx+0x2c]` at `0x48FD1A`) with the widget; fed by the key-state table (`0x483212`, `0x4832FC`, `0x483EC1`).
  `0x48896A` is the cancel/navigate layer (kinds 1 to 5), not the producer. Reading the key codes as ESC/ENTER is inference.
- **Screen 0x20 trigger:** the main loop dispatches on the mode word `[0x4C6A08+0]`; value 4 reaches `0x45DF5C` ->
  `0x45E086` -> `push 0x20` -> `0x486F60` (`ui_state` 5); value 3 builds 0x21. "Level end" is unproven.
- **The writer of the mode word is unread.** `.text` has no direct immediate store to `[0x4C6A08]`; the 50-plus call sites
  that pass `ecx=0x4C6A08` (member functions writing `[this+0]`) were not followed.
- **Old head node survives the build** (nothing in `0x4886E4`, `0x487B4B`, `0x44521B`, `0x497F39` or the factory tail removes
  it); its later removal is unread.
- **Breaks:** per-frame caller of `0x48FCDD`, the mouse hit-test, initial focus, what labels 0x2F / 0x30 name, the
  mode-word writer. **No key or click path is named, so the live start (b2l recipe) is skipped** per the directive.

## Facilitator check

Re-read on mouv: `0x486335` (sets `[0x5D59E0]=0xDC`, then the control build), `0x4865C6` (return), `0x48FCDD..0x48FD34`
(`call 0x4837fa`, push 1, `call [edx+0x2c]`), `0x45E06C..` (`push 0x20`, `call 0x4657d1` before `0x486F60`), table
`0x4C3414` (`00 30 2f 2b` dwords). Own raw-file scan of all of `.text` for the bytes `08 6a 4c 00`: 98 hits, 95 are
`mov ecx, 0x4C6A08`; the other three are `mov [ebp-4], imm` at `0x430FED` / `0x4310CC` and `cmp [0x4C6A08], 2` at
`0x45ACFB`, so no direct store, agreeing with the worker. This does not exclude a store through `this`. Not re-verified
by me: the survival of the old node, the key-table reading, the dispatch values 3 and 4.
