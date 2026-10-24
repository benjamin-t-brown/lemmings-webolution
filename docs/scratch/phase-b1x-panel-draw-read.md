# B1x: the panel geometry is read, no renderer value replaced (round 1 of 2)

2026-10-23. Directive Q1 round 1. Worker note `build/tmp/b1x-panel-draw-read/round-note.md`. Offline; no stock start, no debugger.
Only comments in `src/shell/MenuRender.h` changed. Score unchanged, 0.005244472528594771 (1232493 / 235008000, 185447 px).

## READ

- `0x48B1C7` is not a draw: it is the tail of a constructor-like function (`call 0x48A99B`, then `0x48A190(0x6E)` if
  `this[+0x78]`, return 1). The directive's premise that the panel draw follows it was wrong.
- The panel object (vtable `0x4B1E28`, ctor `0x489999`) is two quads: fill at `obj+8`, border at `obj+0x1B0`.
- `0x48A190` passes `0x489B39` the rect `X = [+0x44] + [+0x34] - 0x14`, `Y = [+0x48] + [+0x38]`, `W = [+0x3C] + 0x28`,
  `H = [+0x40]` (`0x4897C0` returns `+0x44`, `0x4897E0` returns `+0x48`).
- `0x489B39`: fill quad `[X+2, X+W-2] x [Y+2, Y+H-2]`; border quad one pixel larger each side, `[X+1, X+W-1] x [Y+1, Y+H-1]`.
- The ctor pushes `(255.0, 0, 0)` for the fill and `(64.0, 64.0, 64.0)` for the border into `0x44DBC0` (arguments only).

## UNREAD (so still MEASURED-FROM-REFERENCE in the renderer)

- Panel-object fields `+0x34..+0x48` (the 19 / 1 / 27 rule is consistent with the geometry but not derived).
- What `0x4017F0` / `0x44DBC0` do with the doubles, and the setter that yields 224,10,0 and 224,128,0 (the ctor fill is 255,0,0).
- The quad draw (edge convention, blend over the panel). Hypothesis only: a tint pass could explain both the fill colours and the
  backdrop being up to 7 levels darker.

Worker read 15 bodies against a cap of 12 (6 one-line accessors); reported, not excused.

## Facilitator check

Re-read on mouv: `0x48B1C7..0x48B1FD`, `0x489B39..0x489BBA`, `0x48A190..0x48A23A`, `0x489A00..0x489A3A` (colour pushes),
`0x4897C0`, `0x4897E0`. All match, including the +2/+1 quad insets and the 255.0 / 64.0 doubles. `git diff` of `MenuRender.h` is
comments only; Mac and mouv copies have the same MD5. Full ctest 32/32 on mouv after a rebuild. No mutation run, correctly:
no value was replaced. Not verified: the panel-object field writers, anything about colour meaning.
