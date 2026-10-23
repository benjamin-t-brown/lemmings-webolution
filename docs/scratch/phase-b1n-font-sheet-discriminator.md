# B1n: table predicates do not identify the font sheet

2026-10-23. Third dispatch of the B1 runtime-binding question, authorized by
`docs/DIRECTIVE.md` question 1. Offline only, zero stock starts. Candidate packet:
`build/tmp/b1n-font-sheet-discriminator/round-note.md`, prediction and `matrix.json`.

## Bounded result

The checked `^Font` prefix has **five**, not the worker's claimed six, passing
sheets: `^FONT.BMP`, `^FONTH.BMP`, `^FONTP.BMP`, `_FONT.BMP`, `_FONTFE.BMP`.
`_FONTSMALL.BMP` fails. The exact predicates are rectangle bounds, nonzero
non-key/alpha ink for Q/M/digits, zero-width ampersand and blank space. They
do not recognize letters and cannot distinguish these five candidates.
**No sheet identity is accepted**, even as an inference. No renderer or B1
full-frame score was produced.

This is a diagnostic over the six font images in B1e's effective export, not
the directive's complete both-archive test: physical archive inventory,
origin and patch provenance were not established. The missing inventory
cannot turn these tested predicates into a unique choice among the five.
No filename, optimizer, resized matching, or scalar score was used to choose
a source.

## Table-range correction and limits

B1i's blanket **256 packed records / 772 bytes per real table is not
established**. The asserted `^Font` span from `0x4BF968` overlaps the known
`_FontSmall` header at `0x4BFC10` by 92 bytes. The diagnostic stops after
225 complete records before that header; this is a conservative boundary,
not proof of the actual table's complete character domain. The menu's
ASCII entries and B1i's Quit replay are before it and unchanged.

The checker still reads 256 `_FontSmall` records without establishing that
range's end. Its all-negative result is therefore **not accepted as a
font-resource rejection**. Nor does it require positive-width Q/M for that
digit font. The script's `unique` output on any other inventory is not a
qualified identity claim. Both tables' general extent needs a native read
before treating adjacent bytes as glyphs.

## Facilitator check

My separate `build/tmp/b1n-independent.py` imports no worker code. It reads
the authority PE sections with raw-size bounds, authenticates both raw
byte windows/hashes and independently checks every declared cell predicate
against the saved PNGs. All twelve table/sheet rows and all cell bounds/ink
results reproduce exactly, including **five** passing `^Font` rows. This
uses the already-landed image decoder/export; it is not an independent LRZ
decode or proof of full archive inventory.

I independently reread `0x4403A7` (`push 0x20`, `push 0x4BF968`) and
`0x4403E5` (`push 0x30`, `push 0x4BFC10`) from original bytes. Their table
operands and first-character values match B1i. The range overlap follows
directly from those addresses, not from a decompiler's type annotation.

Three synthetic tests pass. An isolated mutation of `ink` to always false
fails the painted-pixel assertion; restoration returns the source SHA-256
to `fd34e3f14f5238936f612d0bfebc4b194a0f26820fd2836ca6553d8eae1cb8e7`.
A separate actual `_FONT` Q-coordinate input control moves only X to 255,
violates bounds, and restores the original table hash. These discriminate
predicate machinery, not semantic glyph identity or B1 pixels.

Full wrapper build and CTest pass **30/30**. Exe/config/save hashes match
my startup baseline; no Lemmings/webolution process or LemWeb task remains.
No production behavior changed. Earlier retractions and history are kept.

## Disposition

Retain the code as diagnostic only; reject the stronger six-candidate,
exhaustive-inventory and `_FontSmall` rejection claims. The source-binding
question is still open after round 3. The current directive expressly
allows one fourth-round live fallback on ambiguity/negative, **only after
offline byte-based current-process highlight and full source-chain
preflight succeeds**. No start is spent or granted to B1n. Do not render
with a guessed sheet or advance another milestone.
