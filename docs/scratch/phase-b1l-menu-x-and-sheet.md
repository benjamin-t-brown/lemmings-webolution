# B1l: the menu X word and the ^Font name have no file store

Queue `b1l-menu-x-and-sheet`. Offline disassembly read, round 3 of 3 on the
B1 render question. No stock start. Worker note
`build/tmp/b1l-menu-x-and-sheet/round-note.md`.

## Result

Both halves are file-negatives.

The address immediate `40 CB 58 00` occurs 63 times, all in `.text`. None
of those instructions writes `[0x58CB40]`. The only word access is the read
at `0x455F13`. The address is past the file-backed end `0x4C6000`, so the
image has no initializer either. The horizontal pixel stays unread.

`"^Font"` occurs twice (`0x4BFF68` and `0x4BFF70`). `^FONT.BMP` and
`font.bmp` do not occur. `0x474519` looks the short name up on the manager
at `0x58CB48` and stores a returned dword. It has no BMP operand.

## Facilitator check

An independent scan of raw `.text` found the same 63 immediates. Grouped
by the opcode in front of each immediate: 34 `mov ecx, imm`, 20 `mov eax,
[imm]`, 3 `push imm`, 1 `mov ax, [imm]`, 1 `mov ecx, [imm]`, 1 `mov edx,
[imm]`, 3 `fild`. No `mov [imm]` store. `read_steppers.py` re-read
`0x474519`: it moves `ecx` to `0x58CB48`, calls `0x449B30`, then either
`0x45FE10` or `0x4607D0`, and stores the result through the out pointer.
A case-insensitive search finds `^Font` twice and `^FONT.BMP` / `font.bmp`
zero times. SHA-256 matches.

The callees that copy the name were not re-opened here. They cannot
introduce a `^FONT.BMP` immediate, because that byte string is absent from
the file. `Textures\%s.bmp` does occur elsewhere; this lookup does not
contain it.

No production file changed, so there was nothing to mutate. The source tree
is the one whose `ctest --preset dev-debug` passed 30/30. No Lemmings
process was started. The B1 gate was not scored. This is the third round
on the question.
