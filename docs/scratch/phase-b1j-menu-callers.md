# B1j: FUN_004404fa's callers do not draw the main menu

Queue `b1j-menu-text-caller`. Offline disassembly read, round 1 of the B1
main-menu render question. No stock start. Worker note
`build/tmp/b1j-menu-text-caller/round-note.md`.

## Result

There are 17 direct `E8` calls to `FUN_004404fa` (`0x4404FA`), in 12
procedures. None of them passes UK string indices 125, 126, 120, 3, or 43,
and none is stored in the boot vtables `0x4B1E88` or `0x4B2528`. The five
main-menu labels are not drawn on this path.

`push 126` does not occur in `.text`. The only `push 120` is at `0x487511`,
and that call is allocator `0x49C1A8`, not the glyph builder. The font operand
at the 17 calls is the BSS pointer `[0x4CBF78]` or `[0x4CBF7C]`. Constructor
`0x440330` writes vtable `0x4B15C8` and `[this+8] = -1`. It does not write a
glyph table or a sheet name. The call encodings do not contain `0x4BF968` or
`"^Font"`.

The function that does draw "Start New Game", "Play Current Game",
"Language Selection", "Options", and "Quit" is unread. B1's full-frame gate
was not scored.

## Facilitator check

Independent scan of raw `.text` (`SizeOfRawData` `0xAF000`, not the harness
virtual size) found those same 17 call sites and no others. SHA-256 of the
stock exe matches the authority hash in `docs/LESSONS.md`. `read_steppers.py`
re-read `0x4404FA`'s prologue, `0x440330` (`mov [eax+8], 0xffffffff` and
`mov [ecx], 0x4b15c8`), and `0x487511` (`push 0x78` / `call 0x49c1a8`).
A file search for the 12 procedure addresses as dwords finds stores only at
`0x4B0994` (`0x413F15`) and `0x4B1780` (`0x457B70`). `push 125` occurs six
times in `.text`, none within `0x40` of a glyph call; `0x457C81` is after
`ret` at `0x457BFF`.

Narrowed: "the operand is not the `^Font` table" is true of these call
encodings and of `0x440330`. It does not say the object at `[0x4CBF78]` is
never registered as `^Font`. That registration is a different read.

No production file changed, so there was nothing to mutate. `cmake --build
--preset dev-debug` had no work. `ctest --preset dev-debug` passed 30/30.
No Lemmings process was running.
