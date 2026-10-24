# C2k: 0x46D460 is not a blender; the render state is unread; the "Paused" string is READ; render unchanged, MM_DIG_000 still fails by 2,660

2026-10-23. Directive Q1 round 2 of 2 (offline; the last). Worker note `build/tmp/c2k-caption-blend-read/round-note.md`. `PauseCaption.{h,cpp}` gain `captionDiffuse()`;
`test_pausecaption` gains `testDiffuse()`. The blend and text colour are unchanged because no read value differs from the render input. No start, no debugger, no game process.

## READ

| Item | Value | Tag |
| --- | --- | --- |
| `0x46D460` | per vertex packs diffuse `(int(d[+0x50]) << 24) \| (int(c0) << 16) \| (int(c1) << 8) \| int(c2)` (`0x46D52E..0x46D586`); `int()` is `0x49C5AC` (`fnstcw`, `or ah,0xC`, `fistp`: truncation); one device call through vtable `+0x70`, args `(6, 0x1C4, ...)` (`0x46D5DE..0x46D5F3`) | READ |
| Meaning | alpha is the top byte of a D3D diffuse dword, so 128.0 packs to `0x80` and the scale is the byte scale; the per-pixel blend, its rounding and 8-bit versus float are the device's | READ (packing); blend UNREAD |
| Vertex offsets | alpha at vertex `+0x50`, colour at `+0x38` land on quad `+0x58` / `+0x40`, where `0x44DD20` / `0x44DBC0` store 128.0 and the zero colour | INFERRED by arithmetic; the vertex lookup `0x414A40` was not read |
| `0x486847(0x54)` | tail-calls `0x486819(idx, [0x4C3434])`; `[0x4C3434]` = 2 (Uk); index `0x54` in both `LANGUAGE\UK.LTX` copies (lemmings.box, patch.box) is "Paused" | READ |
| Text colour | not in the lookup | MEASURED-FROM-REFERENCE (white) |
| Device render state for this quad (alpha-blend enable, blend factors, texture stage) | | UNREAD |

## Result

The render is identical to C2j (the three views hashed against C2j's PNGs are byte-equal). Nine of ten views pass with only the ten skill boxes excluded; `MM_DIG_000` 2686464 against
2683804 fails by 2,660. **Q1's gate is not met.** The question hit its two-round cap (c2j, c2k). The blend formula, the text colour in the exe and the exact pixel rounding stay UNREAD; the fill and
alpha are now read as byte-scale immediates.

## Facilitator check

Re-read on the exe: `0x46D52E..0x46D58B` (the `shl 0x18/0x10/8`, `or`, the store at `+0x10`), `0x46D5DE..0x46D5F3` (`push 0x1c4; push 6; call [ecx+0x70]`), `0x49C5AC` (`or ah,0xc`, `fistp`),
`0x486847`, and the dword at `0x4C3434` from the file bytes (2). The "Paused" string re-read from the saved base and patch `UK.LTX` extracts at pointers `[0x54]` 1548 and 1956. Mutation alone:
`<< 24` -> `<< 23` in `captionDiffuse` fails 3 `pausecaption` checks; restored, sha1 `9364a685...` equal. Full build and ctest on mouv 34/34 after copying the three changed files (sha1 equal on both).
The worker's script contained a stray no-argument `git checkout` (the brief forbade it); `git status` and the diff show only the three owned files and the queue field changed, no stash. Not verified: that
the vertex lookup `0x414A40` yields the offsets assumed, the worker's other four mutations, and the D3D render state.
