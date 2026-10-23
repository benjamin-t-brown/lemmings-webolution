# B1i: font charmap, advance and placement read from the original's text-draw code

Queue `b1i-font-charmap-read`, authorized by `docs/DIRECTIVE.md` question 1 (disassembly
read). Offline-only, as required: zero stock starts, no live game process at any point.
Worker packet `build/tmp/b1i-font-charmap-read/round-note.md` and prediction
`build/tmp/b1i-font-charmap-read/prediction.md`.

## Result

**Read and byte-verified, not fitted.** The per-character build loop is `FUN_004404fa`
(`0x4404FA`–`0x440734`). The character-code-to-cell-index rule:

```
cellRecordPtr = glyphTable + (((charByte & 0xff) - firstChar) & 0xff) * 3 + 4
```

computed by `FUN_004404cf` (`0x4404CF`, the subtraction) and `FUN_00440d50` (`0x440D50`,
the `*3+4` stride). `glyphTable` is a per-font-object blob: a 4-byte `int lineHeight`
header followed by exactly 256 packed 3-byte `{X, Y, width}` records (772 bytes total) — a
full-byte-range table, not a row/column grid. The advance rule (`0x440711`–`0x44071e`):
`X cursor += pbVar10[2]` (the per-glyph width byte from that same table entry),
unconditional per character, no fixed immediate anywhere in the function. Row height
(screen rect height and the UV V1 coordinate) comes from the font object's single shared
`lineHeight`, not a per-glyph value — a proportional-width, fixed-height-per-font
mechanism. CRLF-only newline handling (`0x0D` then `0x0A`, via `FUN_00440d70`/
`FUN_00440d80`), not a general escape system.

The prediction (`build/tmp/b1i-font-charmap-read/prediction.md`, written before reading)
guessed a **fixed-width, raw-byte, no-subtraction** 16-column grid. The result confirms
the `charCode - firstChar` subtraction half and falsifies the fixed-width/grid half — the
real mechanism is `docs/AGENT_SAFETY.md`'s own "expect the wrong shape" warning playing
out again, this time correctly caught and reported rather than rounded toward the guess.

Two real font instances were decoded from raw bytes: `"^Font"` (table at `0x4BF968`,
`lineHeight=29`, proportional: space=8, `!`=7, `M`=19, `i`=6, `W`=20) and `"_FontSmall"`
(table at `0x4BFC10`, `lineHeight=14`, digits `'0'`–`'9'` uniformly width 6 — uniform
because the *data* says so, not because the mechanism enforces it). A genuine quirk: in
`^Font`, character `&` (index 6) is an explicit `(X=74, Y=0, width=0)` table entry —
present, zero-width, invisible, contributing nothing to the cursor advance, not a crash or
fallback. Per `match-the-original.md`, this is a quirk to preserve if `&` is ever drawn.

Font resources are resolved by a short internal **name string** (`"^Font"`/`"_FontSmall"`,
found at `0x4BFF68`/`0x4BFF78`/`0x4BFF70`) through `FUN_00474519`, not by the archive
filename — this refines, and does not contradict, B1c's already-settled negative that
`_FONT.BMP`'s literal filename does not occur in `.text`/`.rdata`. A fresh case-insensitive
`FONT` search over the whole binary found exactly three string hits total: `"^Font"` ×2 and
`"_FontSmall"` ×1 — no `_FONTFE`, `^FONTH`, or `^FONTP` string anywhere. Whether those
three sibling sheets use this same class is therefore a well-supported **inference**, not
an observation: every one of `FUN_004404fa`'s 12 callers routes through this one generic,
fully data-driven draw path, and no second font-drawing mechanism was found.

A retail-string replay (code-only, no live run) of `"Quit"` through the `^Font` table
reproduces exact per-glyph cells/UVs and a final cursor advance of 41 px (15+12+6+8, the
literal sum of the four table widths, no extra inter-character spacing term).

**Incidental, out-of-scope lead** (reported, not landed or claimed as resolved): the
per-character loop's caller `0x44CD06` is the exact function A7d–A7g already traced from
the Paused-caption receiver `H` (`frame 0x441845 → H=G+0x2018 → 0x44CD06`). Its decompiled
body shows `H+0x758`/`H+0x77C` (A7f/A7g's own flagged offsets) each filled once by
`FUN_004404fa(DAT_004CBF78, FUN_00486847(0x44 or 0x45), H+0x758-or-0x77C)`, and `H+0x224`
(A7d's flagged "timer-like" field, placement `(560, 430)` matching A7d's own literal
immediates exactly) filled by an `"%02d:0%d"`/`"%02d:%02d"`-formatted MM:SS string drawn
through the same mechanism. This plausibly closes several of A7's long-parked "who fills
H" threads, but is unverified against pixels or a live read this round, and A7's Phase E
park stands untouched until someone picks it up explicitly.

## Facilitator check

Independently re-disassembled all three cited functions from the raw PE bytes
(`tools/harness/read_steppers.py`, not the worker's own REA/Ghidra session):
`0x4404CF` (`sub eax, edx` index arithmetic), `0x440D50` (`imul eax, eax, 3; lea
eax, [ecx+eax+4]` stride), and the advance update at `0x440711`–`0x44071e`
(`mov cl, byte ptr [eax+2]; add edx, ecx`) — all byte-for-byte match the round's own
citations.

Independently read the raw `^Font` glyph table (`0x4BF968`, 772 bytes, Python struct
unpack, not the worker's own script) directly from the installed exe file: `lineHeight =
29` exactly; `&` (index 6) decodes to `(X=74, Y=0, width=0)` exactly as claimed; `'`
(index 7) starts at the same `X=74`, also exact. Independently replayed the `"Quit"` string
through the rule from these same raw bytes and got the identical per-glyph table entries
the round reported — `Q→(92,60,15)`, `u→(20,120,12)`, `i→(144,90,6)`, `t→(12,120,8)`,
summing to the same 41 px advance. Independently read the `_FontSmall` table (`0x4BFC10`):
`lineHeight=14`, digits `'0'`–`'9'` all width 6, slots past `'9'` all `(0,0,0)` — matches.
Independently re-read the two constructor-wrapper call sites (`0x4403A7` pushing immediate
`0x20` and table/name addresses `0x4BF968`/`0x4BFF68`; `0x4403E5` pushing `0x30` and
`0x4BFC10`/`0x4BFF78`) — both exact. Independently ran a case-insensitive `font` search over
the raw exe bytes: exactly 3 hits, the same three strings, no others — reproducing the
round's sibling-sheet negative exactly.

`git status --short` showed only `docs/tasks.json`'s two permitted fields changed for this
item. No live game process was found running. `ctest --preset dev-debug` (no `src/**`
change expected or found): **30/30 passed**.

Round accepted as reported; nothing narrowed. This is a genuine reading-based answer to the
question the directive posed, independently reproduced from raw bytes rather than trusted
from the worker's own tool output. It is READ, not MEASURED — no live process was used or
needed, consistent with the brief's offline scope. See `PLAN.md` Phase B / Phase C for the
landed disposition and what remains open (sibling-sheet coverage, `FUN_00474519`, and the
unverified A7 "who fills H" lead).
