# C1z: plate blend and texture-stage slots, hook at `0x41494F` (directive part 1b)

Queue `c1z-plate-blend-hook`. Offline. No stock start. Worker [Name the blend hook](e33d1221-bff8-43a9-b39d-b5bc04581b5c). `prediction.md` 15:17:07, `hook.js` 15:17:20, note 15:18:42. Dispatched 15:09 MDT. REA was used for bounds only. The bytes below were re-read with `read_steppers.py`.

## Result

`0x45B66A` calls `0x414920` and returns at `0x45B66F`. From `0x414920` to `0x41494F` there is no branch. `0x414944` calls `0x47261F`. `0x41494C` loads the quad's first dword, `0x41494E` pushes it, and `0x41494F` is `call 0x414D80`. The read-only hook is that call. A plate hit is `[ebp+4] == 0x45B66F`. The selector is `[esp]` before the call. The descriptor `0x414A10` returned is `[ebp-4]`.

`0x414D80` writes slot `+0x58` on `[[0x58CDB8]]`. A selector equal to `[0x4C227C]` makes no call. Selector 0 writes state `0x1B` value 0 and does not write `0x13` or `0x14`. Any other selector writes `0x1B` value 1, then source `0x13` and destination `0x14`: selector 2 writes 2 and 2, selector 3 writes 5 and 2, and any other nonzero selector writes 5 and 6.

`0x47261F` writes slot `+0x98` (texture, stage 0) and slot `+0xA0` (stage 0). A descriptor equal to `[0x5CD5F4]` skips the whole body. Flags `[descriptor+0x24]` equal to `[0x4C24E4]` skip the state calls after the bind. Otherwise the first matching flag row writes alpha-test state `0x0F` through `+0x58` and the alpha stage states through `+0xA0`. Colour stage state 1 value 4, state 2 value 2, state 3 value 0 is written on every row. The low-two-bits row also writes alpha ref `0x18` from `[0x4C24E0]` and alpha func `0x19` value 5.

The MinGW `IDirect3DDevice3` header places `SetRenderState` at `+0x58`, `SetTexture` at `+0x98`, `SetTextureStageState` at `+0xA0`, and `DrawPrimitive` at `+0x70`. `d3dtypes.h` names state `0x1B` blend enable, `0x13` source, `0x14` destination, `0x0F` alpha-test enable, `0x18` alpha ref, `0x19` alpha func. No GUID in the exe was read, so the interface identity is that header layout.

The live selector, flags, alpha ref, and both caches are unread. `^tab` pixels were not decoded.

## Facilitator check

Re-disassembled `0x45B660..0x45B67E`, `0x414920..0x414A06`, `0x414D80..0x414E52`, and `0x47261F..0x472978`. The hook bytes are `e8 2c 04 00 00`. The blend immediates and the stage rows match the note, including the colour writes at `0x4726E5`, `0x4727AC`, and the shared prefix `0x4727FF`. Counted the `IDirect3DDevice3` methods in `C:\progs\msys2\mingw32\include\d3d.h`: index 22 is `+0x58`, 38 is `+0x98`, 40 is `+0xA0`, 28 is `+0x70`. Re-ran `python build/tmp/c1z-plate-blend-hook/dry_check.py` (exit 0, one attach, `node --check` 0). `ctest` 34/34 on the unchanged tree. No game process.

Narrowing: the hook runs after `0x47261F` returns. That function stores the descriptor at `0x472637` and the flags at `0x472671` before the epilogue, so a logged `flag_cache` equal to `flags` does not say whether the stage writes ran. `blend_cache` is read before `0x414D80`, so comparing it with `sel` does say whether the blend writes will run. Not re-derived: `0x414A10`, the four `eax` gates in `0x46D3C1` (still called at `0x4149FD` on this path), and callees of `0x414A60`.

## Stop

Part 1b has its one round. The render still needs the live selector and flags, and the `^tab` pixels. Part 2 is the one Frida start. It was not started here.
