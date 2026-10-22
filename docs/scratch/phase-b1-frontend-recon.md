# B1 recon: where frontend assets live, and the screen-id global's real shape

Queue id `b1-frontend-recon` (explore, read-only; no live game, no edits). This is a map
for the facilitator's own brief-writing, not a landed measurement in the usual sense — it
is not independently re-verified line-for-line the way a measured round is, but its two
load-bearing structural claims were spot-checked against the executable before use.

## Asset loading is already implemented and tested; only app-side wiring is missing

- The box archive (`lemmings.box` + `patch.box`, `src/assets/Box.{h,cpp}`) stores
  background/button/font art as plain `.BMP` entries under `TEXTURES\`, LRZ-compressed,
  decoded through the same `lrzDecode` → `decodeBmp`/`loadTexture` path already used for
  in-level textures (`src/app/TubeScene.cpp`). No frontend-specific resource kind exists.
  Confirmed directly (grep of the raw archive bytes, not a pre-existing dump): `GERMAN.LTX`,
  `MAIN_LOGO.BMP` and `_FONT.BMP` each occur exactly once in `lemmings.box`; `GERMAN.LTX`
  also occurs in `patch.box` (a language override), `MAIN_LOGO.BMP`/`_FONT.BMP` do not.
  Relevant entries: backgrounds/logos (`LOGO.BMP`, `BACK_LOGO.BMP`, `MAIN_LOGO.BMP`,
  `SMALL_BACK.BMP`, `LEVEL SELECT BACKGROUND.BMP`, `^SPLASH`, `^TAB`/`^TAB_SELECTED`,
  `~TAB`/`~TAB_HIGHLIGHT`/`~TAB_SELECTED`), fonts (`^FONT`, `_FONT.BMP`, `_FONTFE.BMP`,
  `_FONTSMALL.BMP`), and `_BTN_*` (controller-button prompt icons — circle/square/
  triangle/cross/d-pad — not main-menu buttons). No dedicated "menu button" sprite sheet
  exists; the 19 `.SPR` sheets in the archive are all lemmings/HUD-icon/hazard art.
  **Open gap**: no glyph-metrics table (width/kerning) was found anywhere in the archive —
  font layout is either hard-coded in the exe or derivable from `.BMP`/`.ALF` geometry; not
  yet located.
- `.LTX` localised text is **already fully implemented and tested**: `src/assets/GameData.{h,cpp}`
  parses `LANGUAGE\*.LTX` (`u32 count`, `count × u32` offsets, NUL-terminated strings,
  per-language Windows codepage via `src/assets/codepages.inc`). `test_gamedata.cpp`'s
  `testRealData()` already loads the real archive and asserts ≥6 languages with >100
  strings each. `GERMAN.LTX` exists in both archives, alongside `FRANCE/ITALY/SPAIN/UK/
  POLISH` (base) and `CZECH/HEBREW` (patch only).
- Nothing in `src/app/` currently calls `loadTexture` or `loadStrings` for anything
  frontend — the existing, tested readers are simply unwired to any frontend code path yet.

## The screen-id global `0x004C416C` is a shared base-class instance counter, not a hand-picked enum

Independently re-read directly against the installed exe (not trusted from the report):
the constructor at `0x004882c0` (confirmed byte-for-byte: zeroes `+4,+8,+0xc,+0x10,+0x18,
+0x1c,+0x20,+0x24,+0x28,+0x30,+0x34,+0x3c`, byte `+0x14=0`, byte `+0x15=1`, writes vtable
`0x004b1da8` into `[this]` at `0x00488350`) then at `0x00488359`/`0x0048835f` **reads the
current value of `0x4c416c` into the new object's own `+0x2c`**, and at `0x00488362`-
`0x0048836b` **increments `0x4c416c` and writes it back**. So `0x4c416c` is a monotonic
object-creation counter shared by a common frontend-screen base class, and each screen
captures the pre-increment value as "its own id" — explaining, not just restating, why
menu/confirm/map read 3/4/5 (construction order in the boot sequence, not semantically
chosen constants). This base ctor has 20 call sites in `.text` (listed in the explore
agent's full report, not reproduced here), each immediately overwriting `[this]` with its
own 16-slot vtable — roughly 20 distinct frontend screen/dialog subclasses sharing one
small polymorphic interface (dtor/step/draw/input/etc., many default slots identical
across subclasses). Which specific vtable is the main menu, confirm or map screen, and
where its own "draw" slot leads, is not yet traced — this lives at `0x00485xxx`-`0x0049bxxx`,
a different address range from the in-level Phase A/C draw path, suggesting (not yet
proving) a separate frontend-only draw path.

## No usable reference screengrab exists yet

`tools/harness/screengrab.py` (desktop BitBlt) is a working, documented capture path.
`tools/harness/frontend_screens.py`'s "both screen-capture paths are black" note predates
and is superseded by that working path (same commit, `979316a`, before the LESSONS.md
note recording `screengrab.py` as working). Leftover images under `build/live_dump/`
(`crop_menu.jpg`, `m0.bmp`, etc., dated 2025-10-13) are 2560×1440 — from the separate
widescreen-patched exe, not the authority 640×480 `Lemmings Revolution.exe` — and carry no
state metadata. **None usable for B1's ≤0.02 comparison.**

## Scope conclusion

B1 splits into at least three rounds: (1) wiring the already-implemented, already-tested
texture/`.LTX` readers into an app-side load (cheap, offline); (2) capturing a fresh
640×480 "menu, highlight 0" reference screengrab against the authority exe (cheap, one
live stock start); (3) tracing the frontend draw dispatch and font layout and actually
rendering the menu to the gate's threshold (the real reverse-engineering work, likely
multi-round). (1) and (2) are independent and file-disjoint — queued next, concurrently.
