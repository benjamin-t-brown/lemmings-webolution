# C2a: 0x44CD06 fills two indexed lists and a separate timer

Queue `c2a-pause-timer-read`. Offline disassembly read, round 1 of the
pause-caption and timer question. C2 groundwork only: the C2 gate was not
scored, and no stock start was used. Worker note
`build/tmp/c2a-pause-timer-read/round-note.md`.

## Result

`0x44CD06` builds three glyph lists through `FUN_004404fa`, all with
`ecx = [0x4CBF78]`.

| Record | Text | Horizontal `+0x14` | Vertical `+0x18` |
| --- | --- | --- | --- |
| `H+0x758` | string index `0x44` | 520 | 15 |
| `H+0x77C` | string index `0x45` | 520 | 45 |
| `H+0x224` | `[0x4CBAAC] / 60` as MM:SS | 560 | 430 |

The timer reloads `[0x4CBAAC]` for both the quotient (`[ebp-8]`) and the
remainder (`[ebp-4]`). Remainder below 10 uses the file string `"%02d:0%d"`
at `0x4C01DC`. Otherwise the format is `"%02d:%02d"` at `0x4C01E8`. A negative
dword stores 0 back to `[0x4CBAAC]` and formats 0:00. The timer list is not
submitted when `[0x4E33C8] == 1`. That dword is past the file end and is not
identified as pause state.

`0x486A9F` loads `[0x4CBF78]`. `0x486C47` calls that loader and `0x486C4E`
calls `0x4403A7`, which binds glyph table `0x4BF968` (`lineHeight` 29) and
the name `"^Font"` at `0x4BFF68`. `0x4403C6` binds the same table with the
name at `0x4BFF70`, also `"^Font"`. Constructor `0x440330` does not write the
table. Whether `0x486C47` runs before the first entry to `0x44CD06` is
unsettled.

Indices `0x44` and `0x45` are a runtime lookup through `0x486819`. The
out-of-range fallback at `0x4C3450` is the file string `"Unknown String"`.
The in-range characters are not in this function. Nothing here says the text
is `Paused`.

## Facilitator check

SHA-256 matches the authority hash. `read_steppers.py` re-read the three
fills. `0x44CD36` is `push 0x44`, `0x44CD41` loads `[0x4CBF78]`, `0x44CD47`
calls `0x4404FA`, then `push 0x0F` and `push 0x208` (15 and 520). The
`H+0x77C` pair is `push 0x45`, then `push 0x2D` and `push 0x208` (45 and 520).
`0x44DFCF` stores `[ebp+8]` (the second push) at `+0x14`. `0x44DFD8` stores
`[ebp+0xC]` (the first push) at `+0x18`. `0x440D00` returns `[record+0x14] +
[record+0x1C]`, which is the cursor the glyph builder advances. File bytes
at `0x4C01DC` and `0x4C01E8` are the two format strings. `0x4BF968` begins
`1d 00 00 00`. `0x4BFF68` and `0x4BFF70` are `"^Font"`.

Narrowed: the `mov eax, [0x4CBF78]` inside `0x486A9C` is at `0x486A9F`, after
the prologue, not at the entry byte. The second `idiv` reloads `[0x4CBAAC]`;
it does not divide the quotient again. The worker's abbreviated listing
skipped those reloads; the quotient and remainder are still of the same
dword.

No production file changed, so there was nothing to mutate. The suite run
with the B1j check was 30/30, and this round also changed no source. No
Lemmings process was running. The C2 gate was not attempted.
