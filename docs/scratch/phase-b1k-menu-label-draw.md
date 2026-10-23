# B1k: the five main-menu labels are the table at 0x4C4214

Queue `b1k-menu-label-draw`. Offline disassembly read, round 2 of the B1
render question. No stock start. Worker note
`build/tmp/b1k-menu-label-draw/round-note.md`.

## Result

The five UK indices are the file dwords at `0x4C4214`: 125, 126, 120, 3, 43.
`0x48ECF4` pushes count 5 and `0x48ECF6` pushes `0x4C4214`. That selector is
`FUN_0048ecdf` (`0x48ECDF`), the dword at vtable `0x4B252C`.

`FUN_0048aa5e` (`0x48AA5E`) walks each string. `0x48B003` calls `0x4404CF`
with the font object and the following instruction reads the cell's width
byte. It does not call `FUN_004404fa`. The font slot is `[0x4CBF78]`, bound
by `0x4403A7` to table `0x4BF968` (`lineHeight` 29, `firstChar` `0x20`) and
the name `"^Font"`. No BMP filename is on that path. Which texture inside
`0x58CB48` the name returns was not opened.

`0x488554` returns `[0x5D59E0] + (argument - 1) * lineHeight + 0x64`.
`0x48ECE6` stores `0xDC` at `[0x5D59E0]`, and `0x488AB0` pushes 1 before the
call at `0x488AB2`. Argument 1 is therefore `0xDC + 0x64 = 320`. Later rows
are that function applied to the following arguments, or an add of the same
lineHeight; the five Y values 320, 349, 378, 407, 436 follow if the caller
counts 1..5.

Horizontal placement is a signed halving of the word at `[0x58CB40]`.
`0x455F13` loads that word, `0x488AF2` sign-extends it, `0x488AF5` `cdq`,
`0x488AF6` subtracts that sign mask, and `0x488AF8` shifts right by 1. The
word is BSS. Its pixel value is unread.

## Facilitator check

The five dwords at `0x4C4214` were read from the exe image as 125, 126, 120,
3, 43. The next four are 44, 48, 43, 3, and the count immediate is 5.
`read_steppers.py` re-read `0x455F10` (`mov ax, [0x58CB40]`) and `0x488554`.
File bytes match `6A 05` / `push 0x4C4214`, the vtable dword `0x48ECDF`, the
call at `0x48B003` to `0x4404CF`, the store of `0xDC`, and the `lea` with
`+0x64`. SHA-256 matches. Push 1 is at `0x488AB0`; the call itself is at
`0x488AB2`.

Narrowed: the `sub eax, edx` before the shift is preceded by `cdq`. `edx`
there is the sign mask of the loaded word, not a second coordinate. The
operation is a signed half of `[0x58CB40]`. The numeric X is still unread.
A later blit of the cells `0x48AA5E` builds was not opened.

No production file changed, so there was nothing to mutate. `ctest --preset
dev-debug` passed 30/30 on this unchanged tree. No Lemmings process. The B1
gate was not scored.
