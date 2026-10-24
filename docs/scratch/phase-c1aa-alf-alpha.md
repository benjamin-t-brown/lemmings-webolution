# C1aa: `^tab` `.alf` bytes are shifted into a 16-bit pixel

Queue `c1aa-alf-alpha`. Offline. No stock start. Worker [Read ^tab alpha rule](5497ec2c-4de7-4528-a2b3-a826d834a276). `prediction.md` expected a double. The instruction is a pair of shifts. REA was used for bounds. Dispatched 16:27 MDT.

## Result

`0x47388F` loads one byte. `0x473897` is `sar eax, cl`. `0x47389F` is `shl eax, cl`. `0x473943` stores the 16-bit word after the colour channels are ORed in. The sar count is `8 - [ebp-8]`, written to `[ebp-0xae8]` at `0x4737EC`. The shl count is the trailing-zero count of `[ebp-0x128]`, the loop at `0x473810`. A zero dword at `[ebp-0x128]` branches at `0x4736B6` to `0x473B18` and does not reach the `sar`.

`0x473B1E` counts set bits in the dword at `[format+0x1c]` and stores that count at `[context+0xc]`. The upload pushes `lea [ebp-0x14]` and this callback to `call [vtable+0x20]` at `0x4731E3` and `0x473222`. `[ebp-0x14+0xc]` is the upload's `[ebp-8]`. The method body is not in the exe.

`^tab_selected` is the same registration. `0x45E90F` pushes `0x4C0E08` and `0x45E919` calls `0x45FE10`. `TEXTURES\^TAB_SELECTED.ALF` is 2304 bytes, min 0, max 248, 751 nonzero, SHA-256 `166cc4ce6cb85ac13671265264fff73bf76d90ff65d4f55f70dfaa8b7271ab9e`. The BMP is 72 by 32, 24 bpp, pixel offset 54, 6912 nonzero colour bytes, SHA-256 `4b6a24e32b11c0c40ac5bcc3667ec4adbf281f92a01c2de446c584532fca6fe6`.

## Facilitator check

Re-disassembled the upload from `0x472F99` through the `sar`, the callback prologue at `0x473B1E`, the popcount loop `0x473BD4..0x473BF1`, the store at `0x473CAA`, and the registration `0x45E8F7..0x45E924`. Re-ran `python build/tmp/c1aa-alf-alpha/rule.py` (exit 0, same hashes and the same `sar` line). An independent parse of the saved BMP and ALF matched those counts. Changed the checker's expected `0x473897` byte from `d3 f8` to `d3 f0`; the command exited 1. Restored; the file matches the original bytes. No source change. The suite on this tree was already 34/34. No game process.

Narrowing: this pass did not find an instruction in `0x472F99` that writes `[ebp-0x128]`. The shl count consumes that dword, and the zero test is real. Whether `[ebp-0x128]` and the mask whose popcount fills `[ebp-8]` are the same value is unread. The COM method at `[vtable+0x20]` is not in the exe. `0x49DA82`, `0x472490` and `0x472A23` were not re-opened. No constant shift is in these bytes. Do not render from `alf/255`.

## Stop

Round 1 of 3. The rule instruction is `0x473897`. The next body is the writer of `[ebp-0x128]`.
