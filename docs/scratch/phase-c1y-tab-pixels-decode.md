# C1y: `^tab` is a 72 by 32 BMP whose colour bytes are zero (directive part 1a)

Queue `c1y-tab-pixels-decode`. Offline. No stock start. Worker [Decode ^tab](e3efaa94-c60d-4705-bbf5-7b8e26c9f60d). `prediction.md` 15:14:27, before this re-run's `tab.member`. Dispatched 15:09 MDT. REA was used for `0x45FE10` and the loader. The bytes below were re-read with `read_steppers.py`.

## Result

`0x45E8F7` pushes `0x4C0E00` and `0x45E901` calls `0x45FE10`. `0x45E909` stores the slot at `[ecx+0x1C940]`. The bytes at `0x4C0E00` are `5E 74 61 62 00` (`^tab`). The next push, `0x45E90F` of `0x4C0E08`, is `^tab_selected`. That member was not decoded.

A leading `5E` stores flag 8 at `0x474471`. `0x4744B1` calls `0x47218A`. Flag bit `0x18` is set, so `0x472328` does not skip the following format. `0x47236B` pushes `0x4C2510`, whose bytes are `Textures\%s.alf`. `0x4C24F0` and `0x4C2500` are `Textures\%s.bmp`. `0x4C24E8` is `Lo` and `0x4C24EC` is `Hi`.

`TEXTURES\^TAB.BMP` in `lemmings.box` decodes through `0x467DE8` to 6966 bytes: BMP, pixel offset 54, 72 by 32, 24 bpp. All 6912 colour bytes are `0x00`. SHA-256 `d38db65196988c5b3d018a5a5f6386ba6333c5590e64ad4bfe67a55bac1403ed`.

`TEXTURES\^TAB.ALF` is 2304 bytes, SHA-256 `ad79114c0300dac61a9740d7d15d5c428fa8c3a4b1bee41d55a77944d57f1a00`, values 0 through 128, 2129 of them nonzero. 2304 is 72 times 32. What `0x47E40D` does with those bytes was not read.

## Facilitator check

Re-ran `python build/tmp/c1y-tab-pixels-decode/decode.py` and got the same line. A separate parse of the saved `tab.bmp` found offset 54, 72 by 32, 24 bpp, and 0 nonzero colour bytes. The saved `tab.alf` is 2304 bytes, the same SHA-256, max 128, 2129 nonzero. Re-read `0x45E8F7`, `0x45E909`, `0x4C0E00`, `0x47446C` (`cmp ecx, 0x5E`), `0x474471` (flag 8), `0x4744B1` (`call 0x47218A`), `0x472328` (`and edx, 0x18`), and `0x47236B` (`push 0x4C2510`). Re-read the `Lo`, `Hi`, `.bmp`, and `.alf` strings. Not re-derived: the `INN_` / `OUT_` test, `0x41D990`, `0x47E40D`, and `[0x5CD5F2]`. No source change. The suite was already 34/34 on this tree. No game process.

## Stop

Part 1a has its one round. A zero colour BMP is not a plate image. The ALF bytes are in hand and their consumer is unread, and `^tab_selected` was not decoded. The render does not start. The one Frida start was not spent.
