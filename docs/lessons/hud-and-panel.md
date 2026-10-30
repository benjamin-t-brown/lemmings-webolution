# HUD and skill-panel findings

> Dated historical research excerpts, preserved in their original wording and order within each source range.
> Earlier conclusions may be superseded by later entries; this is not current project status or
> authorization. See [STATUS](../STATUS.md) for current scope and [LESSONS](../LESSONS.md)
> for essential startup guidance. Some excerpts refer to findings that appeared earlier in the
> former combined file; use their linked evidence and the source-range comments below.

<!-- Source lines 1075-1087 in the pre-split docs/LESSONS.md; excerpt preserved, relative links rebased. -->
## Pause text at 0x44CD06

[C2a](../scratch/phase-c2a-pause-timer.md): `0x44CD06` fills `H+0x758` with
string index `0x44` at (520, 15), `H+0x77C` with index `0x45` at (520, 45),
and `H+0x224` with `[0x4CBAAC] / 60` at (560, 430). The below-10 format is
`"%02d:0%d"`; otherwise `"%02d:%02d"`. The font pointer `[0x4CBF78]` is
registered through `0x4403A7` as `"^Font"` / table `0x4BF968`. The index
characters are a runtime lookup, not the file string `"Unknown String"`
except on the out-of-range path. The file language slot is 2 (`Uk`).
[C2b](../scratch/phase-c2b-pause-index-text.md): both `LANGUAGE\UK.LTX` copies
store `Out` at index `0x44` and `Saved` at index `0x45`. These lists are the
counters, not the word `Paused`. The C2 gate was not attempted.


<!-- Source lines 1344-1413 in the pre-split docs/LESSONS.md; excerpt preserved, relative links rebased. -->
## Useful strings the engine keeps

Cue and UI element names, packed null-terminated in `.data` from `0x004C0C7C` (inside
rawsize, so readable from the image):

`Letsgo`, `Yippee`, `Chink`, `Explode`, `Bubble`, `Scrape`, `Ohno`, `Changeop`, `Splat`,
`ForceF`, `Die`, `FallSplat`, `Lever`, `Mine`, `MineMetal`, `Rumble`, `AntiGrav`, `SpeedPad`,
`Action`, `Balloon`, `Fire`, `Witch`, `Bits`, `GreyIcons`, `BlackIcons`, `AdditiveIcons`,
`Axe`, `Smoke`, then cursors `_curNoLem`, `_curLem`, `_curNoLemLeft`, `_curLemLeft`,
`_curNoLemRight`, `_curLemRight`, `_curEnt`, `_curExit`, `^curGrab`, `_curMseDwn`,
`+shadow_new`, `^tab`, `^tab_selected`, `^out`, `^saved`, `^time`.

`^saved` / `^out` / `^time` are the HUD counters, and `_curExit` means the engine has its own
exit hover test — both matter for Phase C.

**Every cue in that list has now been chased down** (rounds 18–28), and most were clean
negatives. Do not re-investigate them; see the gadget table below.

**`GreyIcons`/`BlackIcons`/`AdditiveIcons` are chased (C1b, round 1): each sheet decodes to
exactly one sprite, not ten icons.** `SPRITES\GREYICONS.SPR`→`bomb` (18×25, one frame),
`SPRITES\BLACKICONS.SPR`→`questionmark` (16 near-duplicate frames, max 20×19),
`SPRITES\ADDITIVEICONS.SPR`→`armageddon` (13 frames, max 15×18) — independently re-decoded
twice, from the production reader and from a from-scratch parser. All three, plus `Bits`,
`Axe` and `Smoke`, are registered by the **same** function, `FUN_0045e51f`, through the
**same** generic sheet-loader `FUN_00446040` round 23 already named for `Axe`; that
function only loads handles, it does not draw. These three read as per-button **badges**
(questionmark→fast-forward's multiplier cycle, armageddon→the detonation animation,
bomb→plausibly a greyed/disabled overlay), not the ten base icons' strip — do not reuse
them as the panel's base art. `SPRITES\BITS.SPR` is the lemming's death-gib body parts, a
sibling clean negative. The ten base icons' actual sheet/draw call is still unlocated
([C1b](../scratch/phase-c1-panel-icon-render.md)): the hover/selection chain
(`FUN_00454d6c`→`FUN_004532e2`→`0x45cff9`) contains no draw call; the real draw sits behind
an untraced per-skill vtable. Do not repeat the archive/PE-resource search — it was
exhaustive.

**C1c (round 2): the per-skill vtables don't draw either, and `LemmingsLO` is unconfirmable.**
All nine skill-constructor vtables' ~50 virtual-method bodies are simulation physics,
eligibility, cell-flag writers, or (several skills) a call into `FUN_0042fd25` that sets the
**lemming's own in-level action-pose animation name** — the input to the actor-rendering
pipeline (A4r/A4t), not the panel. `SPRITES\LEMMINGSLO.SPR`'s name string does not exist
anywhere in the exe's string table, so that lead cannot even be xref-searched. `panel_boxes`
(`0x4c9208`) has exactly 9 xrefs, all inside the hit-test function `0x454d6c`-`0x455656` — no
second, draw-side reader exists anywhere in the binary; do not re-check this. The per-level
main loop `FUN_0045cff9` (sole caller `0x450696`) calls `FUN_00414920` (confirmed real draw
primitive) via `FUN_00454a85`, but only for one resource-indexed overlay quad/text label per
frame, never a ten-slot array. One concrete unresolved lead remains for whoever resumes this:
`FUN_00454a85`'s closing indirect dispatch `(**(code**)(**(int**)(levelScreen+0x3c)+4))()` on
an unidentified object — REA's own "unresolved indirect call" limitation, not yet a target.

**C1d (round 3): the ten-widget array is found; full dispatch and the icon index are
still open.** `levelScreen`'s constructor (`0x452bc2`) builds exactly ten identical
sub-objects at stride `0x238` (`+0x70, +0x2a8, +0x4e0, +0x718, +0x950, +0xb88, +0xdc0,
+0xff8, +0x1230, +0x1468`), one shared constructor `FUN_004576b0` (used nowhere else),
one shared six-slot vtable `0x4b1774`. Slot+4 (`FUN_00457868`) is a genuine icon draw:
resource lookup by the widget's own `+8` index via `FUN_00414a10`, UV rect from the
widget's own `+0x21c..+0x228` divided by the resource's `+0x14/+0x18`, drawn via
`FUN_00414920`. `levelScreen+0x3c` is fixed to widget 0 at construction; the dispatch
`FUN_00454a85` reaches only that one widget per frame. No resolved caller reaches the
other nine, and no writer of any widget's `+8` was found. Do not re-derive any of this;
resume only on the two named open threads (a stride-`0x238` literal search for a possible
unresolved-indirect dispatch loop; a field-store search for `+8`).

**C1e (round 4, cap exception): both threads are clean negatives — parked.** An
exhaustive `search_bytes` scan for the `0x238` stride literal found exactly 7 matches in
all of `.text`, none a dispatch loop and none near `levelScreen`'s constructor. A second
exhaustive scan (all ten widgets' absolute `+8` offsets, plus a full re-read of the five
most-likely candidate functions) found no writer of any widget's icon index anywhere.
Both independently re-verified exactly. **Parked in PLAN.md Phase E** — closing this
needs a live memory read (not more static analysis); do not repeat either search.


<!-- Source lines 1463-1470 in the pre-split docs/LESSONS.md; excerpt preserved, relative links rebased. -->
## HUD counters and timer (C2c, 2026-10-23, READ only)

Positions 520,15 / 520,45 / 560,430 are absolute pixels (`0x44DF40`). The count text (`%d` at `0x4C01F4` / `0x4C01F8`, Saved
from word `0x4CBAA4`) goes into the same lists as the labels (`0x44D316`, caller `0x444B98`); `0x44D22C` (caller `0x44318D`)
re-places them with right end near x 620, Out at y 15, Saved at y 50. Saved is white below word `0x4CBAB0` and (255,63,63) at
or above (R,G,B order unread). Open: replace versus append, the Out source, default glyph colour. Evidence:
`docs/scratch/phase-c2c-counters-timer-read.md`.


<!-- Source lines 1478-1491 in the pre-split docs/LESSONS.md; excerpt preserved, relative links rebased. -->
## HUD counter digits replace the words (C2d, 2026-10-23, READ only)

`0x4404FA` zeroes the list count before it builds, so `0x44D316` leaves only digits in the Out and Saved lists. Out is the sum of
words at `this+0x5070 + idx*2` over the list at `this+0x1FB0` (`0x444ABC`); `0x4CBAB0` has no absolute-address writer. The Saved colour
object holds 255, 63, 63 (at or above target) or 255, 255, 255 in memory order; R,G,B naming and the default glyph colour are
unread. Evidence: `docs/scratch/phase-c2d-counters-open-reads.md`.

## Pause caption inputs (C2e, 2026-10-23, READ only; corrects G)

`G` in the A7 chain is the static `0x4C6A08`; the level screen `0x4CBFB0` is the pointer at `G+0x55A8`. The `0x47AD8C` rect is
gated by `[G+0x26024]`, which is cleared once `[G+0x25FF8]`, decremented only while `ui_state == 0`, drops below -1.0 (about 2.5 s),
so after a 14 s walk the rect is predicted off in the paused frame and A7k's 1020 was an intro value. The caption edge probably comes
from `G+0x25C68` (placement via `0x47ACF9` / `0x47AC50`, unread). Evidence: `docs/scratch/phase-c2e-caption-preflight.md`.


<!-- Source lines 1499-1660 in the pre-split docs/LESSONS.md; excerpt preserved, relative links rebased. -->
## HUD counters and timer drawn (C2g, 2026-10-23, render; reads partial)

Out, Saved and the timer are drawn from the C2c/C2d reads: digits replace the words, right end at 620 (`0x44D288`), Out y 15, Saved y 50, timer
`MM:SS` at 560,430. The amended A7 gate still passes on all ten views (numerator lower on three, equal on seven, since the HUD boxes hold no
gate-mask pixels there); the region residual drops in 27 of 27 cells. Right end versus centre, the Out definition, the Saved target word and the
default glyph colour stay UNREAD. Evidence: `docs/scratch/phase-c2g-counters-timer-render.md`.

## Pause caption panel formula (C2f, 2026-10-23, READ; corrects C2e)

The caption panel is not placed by `G+0x25C68`: `0x44E150` is a generic cell-list draw and `+0x14/+0x18` place the glyph text. The panel is
a quad at `G+0x25C90` written by `0x45D35A call 0x44DC50`: `[half(P40)-half(W)-8, P44-2H, half(P40)+half(W)+8, P44-H+3)` with `P40/P44 =
[0x58CB40/44]` and W/H the returns of `0x47ACF9` / `0x47AC50`. P40 = 640, P44 = 480, H = 29, W 70..71 reproduce the A7 target; those values
are measured-from-reference, not read. Evidence: `docs/scratch/phase-c2f-caption-placement-read.md`.

## Pause caption panel read live (C2h, 2026-10-23, MEASURED)

In the paused frame (`ui_state == 1`) the quad at `G+0x25C90` holds 277 / 422 / 363 / 454 as exact integers in all five snapshots, with
`[0x58CB40]` 640, `[0x58CB44]` 480 and `G+0x25C68+0x14/+0x18` 285 / 422 (so `half(W)` 35 and `H` 29, derived). The C2f formula reproduces the
A7 reference rect from read inputs. Whether the doubles are final pixels (no scale before raster) and the panel colour are unread. Evidence:
`docs/scratch/phase-c2h-caption-live-read.md`.

## Pause caption panel drawn, gate not met (C2i, 2026-10-23, render)

The panel (rectangle MEASURED, fill black and alpha 128/255 HYPOTHESIS from the setter immediates, blend ours) lowers the caption-region
residual on 10 of 10 A7 views, but with the caption exclusion removed the amended gate still fails on `initial`, `canonical` and `MM_DIG_000`
(initial 2879334 against 2742219). The "Paused" glyphs (key `STR_PAUSED`, consumer unread), the blend in `0x46D460` and the tube rows under the
panel are the remainder. Evidence: `docs/scratch/phase-c2i-caption-render.md`.

**C1g (Q1 round 1, offline, [note](../scratch/phase-c1g-base-icon-draw-read.md)): the base icons are drawn by the skill-panel object `0x004C91D8`, not the `0x4b1774` widgets.** READ: frame arm `[G]==2` -> `0x45E0E6` -> `0x441845` -> `0x45B639`; each icon through `0x47A75D(icon, x, y)` at `0x45B784`; selected by animation name from `0x4C0694` (climb ... armageddon), frame `e8*nframes/ec` from `0x4C06E8`, not `widget_index+8`. Sheet: `0x4E3318/14/10` for armageddon/questionmark/bomb, else first non -1 of `0x4E335C[0..3]` (`sprintf("%s%s")` of `Lemmings`/`LemmingsWater`/`LemmingsAcid`/`EvilRat` + `Lo`/`Hi`). Counts: two digits, font `[0x4CBF7C]`, anchor (66, box top + 8). UNREAD: `0x47A75D` middle (x adjust, UV), loaded variant, count font file and colour. C1c's "`LemmingsLO` ruled out" is narrowed to unproven (the name is built by `sprintf`, so the missing literal proves nothing).

**C1h (Q1 round 2, offline, [note](../scratch/phase-c1h-icon-placement-count-read.md)): the icon scale mode is READ, its inputs are not.** `0x47A75D` scales the frame offsets by `S = [0x58D53C] - [0x58CB38]` (halved at -1, native at 0, doubled at >= 1); `[0x58D53C] = [[0x4CBFB0]+0x10]` (`0x44186B`), `[0x58CB38]` is 0 for icons 2/8/9 and 1 otherwise. The sheet suffix is `Hi` unless a size-versus-budget loop (`0x443602..0x443733`) steps prefixes down to `Lo`. The count font object `[0x4CBF7C]` is not loaded by any `^Font` / `_FontSmall` call (those serve `[0x4CBF78]` and `[G+0x5574]`). UNREAD: `[[0x4CBFB0]+0x10]`, the sheet frame values, the budget, the count font file and colour. Q1 is at its cap of two rounds.

**C1i (Q2, the one stock start, [note](../scratch/phase-c1i-icon-live-read.md)): entry failed again, nothing read, parked.** With the c3m retry and logging patch applied, `find_menu_index` still returned None (`no menu highlight`, 01:35:47, about 44 s after launch) and the game was foreground. The per-press diagnostics went to stdout and were lost, so the entry failure (c3j, c1i) is still not named. Install hashes restored, no game-memory write, process closed. Every C1 live item (`[levelScreen+0x10]`, icon fields, `0x4E335C..`, `[0x4CBF7C]`, hook) stays UNREAD; do not spend another start before the entry is understood offline.

**C1k (part 1b, offline, [note](../scratch/phase-c1k-observable-entry.md)): the menu entry now records its own failure; the cause is still not named.** `find_menu_index` takes a `log` and writes one line per DOWN press (key polls consumed, foreground pid before and after, mod-5 and kept counts); `snapshot_idle` samples screen id, `ui_state`, key polls and foreground for 15 s with no input; the c3m retry and this change are landed in `tools/harness/goto_level.py`. The probe copy (`build/tmp/c1k-observable-entry/probe.py`, output `build/tmp/c1l-icon-live-read/`) reads exactly what the c1i probe read. The dry check is stubbed: it proves the lines reach `run.log`, not that DOWN reaches the game.

**C1j (part 1a, offline, [note](../scratch/phase-c1j-named-writers-read.md)): the C1 live-read list is shorter, and C1h's count-font claim is corrected.** `[[0x4CBFB0]+0x10]` is a runtime mode field fed by messages and flag table `0x5D5850` (writers `0x4450EB`, `0x45D184`, `0x45700F`, `0x453369`), not a constant or a device query; its stock value is UNREAD. The frame record `F` is a `SpriteFrame` stream read by `0x485CE0`, so it is derivable from the `.SPR` once sheet and row are known. `[0x4CBF7C]` = `[G+0x5574]` and **is** loaded: `0x45D03E` -> `0x4403E5` pushes `_FontSmall` (C1h said no call loads it; withdrawn). `0x4E3310/14/18` = GreyIcons / BlackIcons / AdditiveIcons by registration stores `0x45E78D/A5/BD` (READ, no copy). Still live-only: the mode field, the per-icon `+0x1B8/1CC/1F8/1FC`, the `0x4E335C` handles, the font file and colour.

**C1l (part 2, the one stock start, [note](../scratch/phase-c1l-icon-live-read.md)): the entry worked and the icon inputs were read; the hook did not run.** Idle snapshot: the menu was interactive from the first sample (screen 3, `ui_state` 3, key polls advancing, game foreground), the scan found its highlight after three presses, and the cause of the c3j / c1i failures is still not named. MEASURED at the paused start: `[levelScreen+0x10]` = 0 (so generic icons are halved, icons 2/8/9 native, by C1h's formula); `0x4E335C` = {6, -1, -1, -1}; `+0x1FC` is the registrar handle (6 for the seven generic icons, 1 / 2 / 3 for icons 2 / 8 / 9 = Grey / Black / Additive); all ten frame records F; `[0x4CBF7C]` and `[0x4CBF78]` are live font objects (`+8` = 37 and 36). UNREAD: the sheet behind handle 6, the font file and colour, the count text behind the objects' `+0xC` pointers, the hook's x / y. Install restored, one start used.

**C1m (directive 1a, offline, [note](../scratch/phase-c1m-sheet-file-read.md)): handle 6 names `Lemmings` plus `Hi` or `Lo`, read from the loader; Hi versus Lo is a video-memory query.** `0x4432C1..0x4438B8` formats `%s%s` from the prefix table `0x4BFFC8` and suffix table `0x4BFFD8`, registers it through `0x446040` and stores the handle at `0x4E335C + 4*i`; the file is opened lazily by `0x484EB0` as `Sprites\%s.spr`, and `lemmings.box` holds `SPRITES\LEMMINGSHI.SPR` and `...LO.SPR`. Prefix presence is four flag bytes at `.LVL` offsets 36..39 (`MM_CLIMB_002` = 1,0,0,0). For that level `Hi` holds iff `[0x58CD80+0xCC]` (DirectDraw caps query, `0x1000`) is at least 2,678,724 bytes; the value is UNREAD, as is the registry slot-6 name. That handle 6 is the `0x443874` call is INFERRED from registration order. Facilitator re-derived the tables, header flags and threshold from the exe and level bytes; the step-down CFG and lazy-load chain were not re-derived.

**C1n (directive 1b, offline, [note](../scratch/phase-c1n-count-font-read.md)): the count text is two digits built by `idiv 10`; the font is `_FontSmall` by name; its file and colour are only partly read.** `0x45A7D0..0x45A7FB` builds `count/10 + 0x30`, `count%10 + 0x30` (0 draws "00") and `0x4404FA` turns it into 0x1B0-byte glyph quads at `textobj+0xC`. Handle `+8` is the pool index in manager `0x58CB48`, keyed by the name `_FontSmall`, written through `0x440404` -> `0x474519`; it does not touch the sheet registrar. The only archive candidate is ordinal 656 `TEXTURES\_FONTSMALL.BMP` (181 bytes); glyphs '0'..'9' are 6 x 14 at x = 0, 6 ... 54 (`0x4BFC10`). UNREAD: that file's pixels, any tint, and icon 9's text pointer.

**C1o (directive part 2, tooling, [recipe](../agents/MOUV_SSH.md)): `rea instrument-with-frida attach` works from an ssh worker against a session-1 process.** MEASURED on a throwaway `pythonw` target hooking `kernel32!Sleep`: hits reported, clean detach, target alive; the facilitator re-ran it. Not tried on the game, so no hook read of the game has been made.

**C1q (directive 1b round 2, offline, [note](../scratch/phase-c1q-font-bitmap-read.md)): the count font bitmap is 64 x 16, 4 bpp, white digits on black.** The exe's decoder `0x467DE8`, run in an emulator on archive ordinal 656, yields a 630-byte BMP with palette index 1 = white and cells 6 wide at x = 0, 6 ... 54, matching table `0x4BFC10`. Vertex colour defaults to 255.0 (`0x440A10`), packed to `0xFFFFFFFF` by `0x46D460`, so the digits are white unless a later writer changes the quad colour: an inference, with those writers, the colour key (`0x47218A`) and the blend state UNREAD. The facilitator re-parsed the decoded BMP; the emulation of the decoder itself was not independently run.

**C1p (directive 1a round 2, offline, [note](../scratch/phase-c1p-spr-sheet-read.md)): `LemmingsHi.spr` parses offline and equals c1l's live frame records for all seven generic icons.** `0x483EF0` reads 15 records (bash ... cartwheel) of sub-lists of 0x48-byte frames (source rect, four UV doubles, dx, dy, w, h, texture name) and a scale of 1.0 (Hi) or 0.5 (Lo). Indexed by c1l's `+0x1C8` / `+0x1CC` / `+0x1B8`, Hi matches dx, dy, w, h and all four UV doubles 7/7; Lo fails, so the c1l run used Hi (derived; the free-memory value stays unread). The facilitator's fresh parser reproduced 7/7 and 0/7. UNREAD: the sheet BMP encoding (the `$LEMSHEETHI*.BMP` members start `02 da 8d 60`, not `BM`), the archive lookup `0x47E40D`, the 384-byte tail's reader. The render still lacks the sheet pixels and the icon draw x, y.


**C1r (directive part 1, offline, [note](../scratch/phase-c1r-sheet-pixels-decode.md)): the sheet members decode through the font's routine `0x467DE8`, and the seven generic icons' pixels are READ.** The member's leading `02` is a chunk count, not a method tag: per chunk u16 length, u16 output size, u8 type (3 raw, 4 = `0x467DE8`); caller `0x450270..0x450449`, call `0x4503FD`. All 115 `$LEMSHEET*` members decode to a BMP of the declared size (worker's unicorn sweep). The icon frame rectangle is x `[f8, fc)`, y `[f4, f0)`; its UV equals rect / sheet size, and the crops hold art (non-index-0 pixels 679/690 ... 689/690). Icons 2, 8, 9 use `GREYICONS` / `BLACKICONS` / `ADDITIVEICONS` (24, 24, 4 bpp), not the Hi sheets: partly read. The colour key (`0x47218A`), blend (`0x46D5F3`), tint writers and draw x, y stay UNREAD. The facilitator rebuilt the crop counts from the saved BMPs and c1l's live UV and re-read the call site; the decoder emulation was not independently run.

**C1s (directive part 2, the one stock start, [note](../scratch/phase-c1s-frida-icon-hook.md)): the ten skill-panel icons are drawn at x = 39, 37, 38, 39, 40, 39, 41, 37, 38, 38 and y = 40, 92, 121, 158, 197, 238, 279, 317, 364, 410, and the count glyph quads are white.** MEASURED on the stock exe with one Frida hook on `0x47A75D` (`rea instrument-with-frida attach` from ssh onto the session-1 game: first attach to the stock game, clean detach, game alive, install hashes restored): 8,660 hits in 6 s, 4 complete frames of 10 icons, ECX = `0x4C91D8 + 0x1150 + i*0x208`, x and y equal C1g's READ arguments exactly. All 72 count-quad colour blocks read rgb 255.0 and alpha 255.0 (texture handle 37); the packed `0xFFFFFFFF` is derived, `[vertex+0x10]` was not read, and the blend state is UNREAD. `[0x58CE4C]` = 260,118,840 (above the Hi threshold); icon 9 has no text pointer. The facilitator re-derived every number from the saved raw output and checked the live side after the worker returned; the live reads themselves are single-start.

**C1t (directive part 3, offline, [note](../scratch/phase-c1t-q3-panel-render.md)): the ten icons and the two-digit counts are drawn, and the C1 gate fails.** `--skill-panel` composites the C1r sheet pixels at the C1s positions through the C1h scale (`S = -1` for the seven generic icons, `0` for icons 2, 8, 9). The scorer's `skill_mae` is 0.10767973856209151 on `initial` (15157/140760) and 0.0554187566499468 on `MM_DIG_000` (3646/65790), identical to the pre-panel run. The scorer's mask inside the skill bands is only x 86..89 (184 and 86 pixels); the new pixels sit at x 26..76 and meet that mask in 0 pixels, so the masked error cannot move. A full-band compare of the `initial` render to `original-0.png` is 0.29204980255991286 (5147553/17625600), also above 0.02 (0.3146047226761075, 5545097/17625600, with the panel off). The oval button plates, the colour key and the device blend were not drawn. Not tuned.

**C1u (directive part 1, offline, [note](../scratch/phase-c1u-footprint-mask.md)): the icon and glyph footprint is committed, and the saved C1t renders miss 0.02 on `initial`.** Ten icon rectangles use the C1s positions, the C1l frame sizes and the C1t edges (`dy` unscaled; scale 0.5 or 1). Eighteen glyph rectangles are the C1s quad-corner doubles, `[66, 72)` and `[72, 78)` by 14 rows; icon 9 has none. Plates are empty. The raster rule, fixed before the score, counts a pixel whose square intersects the rectangle: 5133 pixels. Against `original-0.png`, `render.png` is 0.22416530739836685 (880240/3926745) and `no-panel.png` is 0.3317052673397432. `MM_DIG_000` has no named stock frame. The facilitator recomputed the rectangles from the saved C1l and C1s files and re-read `0x47A7BB` and the `-1` divide arm; the scale-0 path to `0x47AB2A` was not re-walked.

**C1v (directive part 2 round 1, offline, [note](../scratch/phase-c1v-plate-draw-read.md)): the ten button plates are quads drawn by `0x45B66A`, and a zero count skips the text submit.** `0x45B639` loops `i < 10`, takes `this+i*0x1B0+0x30` (`0x4C9208` when `this` is `0x4C91D8`) and calls `0x414920`. The box written at `0x45A795` is x `[18, 90]`, y `[12+40*i, 12+40*i+32]`. The handle is `[0x4E3348]`, stored from the name at `0x4C0E00`, whose bytes are `^tab`. The file and the pixels are unread, and nothing in these bytes says the texels are an oval. `0x45B6E4` compares `[this+i*4]` with 0 and skips `0x44E150`; that dword is the count `0x45A7D6` divides by 10. The plate draw is outside the branch. Colour key `0x47218A` and blend `0x46D5F3` stay unread. The facilitator re-read the loop, the box pushes and the five name bytes; the registration chain and `0x45FE10` were not re-derived.

**C1w (directive plate join, offline, [note](../scratch/phase-c1w-plate-footprint.md)): the ten plate boxes are in the footprint, and the saved renders still miss 0.02.** The boxes are x `[18, 90]`, y `[12+40*i, 12+40*i+32]`. Icon and glyph edges are the C1u rectangles, unchanged. The union is 23166 pixels. Against `original-0.png`, `render.png` is 0.29181384257636983 (5171522/17721990) and `no-panel.png` is 0.315641979258537. `MM_DIG_000` stays reference-unnamed. The facilitator recomputed the union and both numerators, and a one-edge change of plate-0's right from 90 to 80 moved the render MAE to 0.29001710229161554 before restore. `^tab` pixels stay unread.

**C1x (directive part 2 round 2 of 2, offline, [note](../scratch/phase-c1x-colour-blend-read.md)): `0x47218A` is the texture-loader entry, and `0x46D5F3` is an indirect call.** `0x47218A` is `push ebp`. The plate draw does not call it; `0x4744B1` does. `0x46D5F3` is `call dword ptr [ecx+0x70]`, `ecx = [[0x58CDB8]]`, after `push 0x1C4` and `push 6`. No blend mode is in that instruction. `0x4149FD` calls `0x46D3C1`, and `0x46D457` calls `0x46D460` only when four `test eax,eax` checks are nonzero. Those `eax` values, the method, and the `^tab` pixels are unread. The facilitator re-read those instructions. Part 2 stops here; the render was not started.

**C1z (directive part 1b, offline, [note](../scratch/phase-c1z-plate-blend-hook.md)): the plate quads set blend in `0x414D80` and texture stage in `0x47261F`, and the hook is `0x41494F`.** `0x45B66A` calls `0x414920` with no branch before `0x41494F` (`call 0x414D80`). Slot `+0x58` there is blend enable `0x1B`, source `0x13` and destination `0x14`, skipped when the selector equals `[0x4C227C]`. `0x414944` calls `0x47261F`, which binds a texture through `+0x98` and writes stage-0 colour and alpha through `+0xA0`, plus alpha test through `+0x58`, unless the descriptor or the flags match their caches. Header names for those slots come from MinGW `IDirect3DDevice3`; no GUID was read. The live selector and flags are unread. The hook samples the flag cache after `0x47261F` returns, so that sample does not say whether the stage writes ran. The facilitator re-disassembled all four ranges and re-ran the dry check.

**C1y (directive part 1a, offline, [note](../scratch/phase-c1y-tab-pixels-decode.md)): `^tab` decodes to a 72 by 32 BMP whose colour bytes are zero.** `0x45E8F7` pushes `0x4C0E00` (`^tab`) and `0x45E909` stores the slot. A leading `5E` sets flag 8 (`0x474471`), and `0x47236B` pushes `Textures\%s.alf`. `TEXTURES\^TAB.BMP` is 6966 decoded bytes, SHA-256 `d38db65196988c5b3d018a5a5f6386ba6333c5590e64ad4bfe67a55bac1403ed`, and all 6912 colour bytes are `0x00`. `TEXTURES\^TAB.ALF` is 2304 bytes with 2129 nonzero. The consumer `0x47E40D` and `^tab_selected` (`0x45E90F`) are unread. The facilitator re-ran the decode and re-read the registration, the flag-8 test, and the `.alf` format.

**C1ab (gate run round 1, offline, [note](../scratch/phase-c1ab-prestage-hook.md)): the plate hook samples the descriptor and flag cache at `0x414944`, before `0x47261F` stores them.** `0x414944` is `call 0x47261F`. `onEnter` reads `[ebp-4]`, `[descriptor+0x24]`, `[0x5CD5F4]` and `[0x4C24E4]` on a plate hit. `0x41494F` still reads the blend selector before `call 0x414D80`. `0x47262E` skips the body when the descriptor equals the cache, and `0x472665` skips the flag store when the flags equal the cache, after the bind at `0x472653`. The live values are unread. The facilitator re-disassembled both ranges and re-ran the dry check.

**C1aa (gate run round 1, offline, [note](../scratch/phase-c1aa-alf-alpha.md)): a `^tab` `.alf` byte is `sar` then `shl` into the 16-bit pixel.** `0x473897` is `sar eax, cl` with `cl = 8 - [ebp-8]`. `0x47389F` is `shl eax, cl` with `cl` the trailing-zero count of `[ebp-0x128]`. `0x473943` stores the word. `0x4736B6` skips the `sar` when that dword is 0. `0x473B1E` stores a popcount at `[context+0xc]`, and the upload passes `lea [ebp-0x14]` to `[vtable+0x20]`. `^tab_selected` is the same `0x45FE10` path; its colour bytes are nonzero and its `.alf` max is 248. The writer of `[ebp-0x128]` is unread. No constant shift is in these bytes. The facilitator re-disassembled the shifts and re-ran the rule command.

**C1ad (gate run, one stock start, [note](../scratch/phase-c1ad-plate-blend-live.md)): at paused `MM_CLIMB_002` every logged plate uses blend selector 1.** MEASURED: 8650 plate hits, caller `0x45B66F`; the 40 stored records are four frames of plates 0–9 at `0x4C9208 + i*0x1B0`. Descriptor `0x2627270`, flags 8, blend cache `0x10620C1F`. The flag cache equals the flags on all 40. The descriptor cache differs only on plate 0. `[0x4C24E0]` is `0xA0` and was not shown to be stored, because the flag-cache compare skips those stage writes. Selector 1 is the landed nonzero arm (enable 1, source 5, destination 6) and the cache compare does not skip it. The facilitator re-parsed the hook log and re-hashed the install.

**C1ac (gate run round 2, offline, [note](../scratch/phase-c1ac-alpha-mask-writer.md)): `[ebp-0x128]` is written by a 32-byte `rep movsd`, not by the upload.** The five uses in `0x472F99` are reads. `[ebp-0x128]` is `[ebp-0x144+0x1c]`. `0x473C95` calls `0x49D010` with count `0x20`; `0x49D043` and the overlap copy `0x49D1C4` are `rep movsd`. The popcount reads the source `+0x1c`; the shift reads the destination. `call [edx+0x64]` at `0x4736F9` receives the record before the shift load, and that body is not in the exe, so the shift is not shown to count the copy. No numeric mask. The facilitator re-listed the five reads and re-ran the command.

**C1ae (gate run round 3, offline, [note](../scratch/phase-c1ae-shift-count-hook.md)): a read-only hook at `0x473897` can see the mask and both shift counts.** `onEnter` is before `sar eax, cl`. `0x473891` reloads the sar count into `ecx`. `[ebp-0xad8]` still holds the shl count, and `[ebp-0x128]` is the mask. The span stores neither the mask nor a call. The numeric counts are unread. The facilitator re-ran the dry check and a one-address change made it fail.

**C1af (gate run round 4, one stock start, [note](../scratch/phase-c1af-shift-count-live.md)): the hook at `0x473897` logged 0 hits after pause.** `Popen` 17:15:44, pid 32972. The click, pause, and `ready.json` are 17:16:16, and the attach began then. `C1AE_HOOK` is `hits` 0. The six-second window did not call the `sar`. The upload at the click was not observed. Install hashes match. The facilitator re-read the hook JSON and re-hashed the install.

**C1ag (gate run round 5, offline, [note](../scratch/phase-c1ag-arm-before-click.md)): the probe waits for `hook-armed.flag` before the level click.** The raise on a missing flag is before `aim_at`. `ready.json` is written after `enter_level()` returns. The attach remains `0x473897`. The report timer still starts when the script loads. The facilitator re-ran the dry check and saw it fail after that one `raise` was removed. No stock start.

**C1ah (gate run round 6, the last stock start, [note](../scratch/phase-c1ah-shift-count-armed.md)): `0x473897` did not run between the arm and the report.** `Popen` 17:32:40, pid 42720. The arm wait is 17:33:10, the click is 17:33:39, and `C1AE_HOOK` is `hits` 0. `select_level` is logged before the wait. Install hashes match. The facilitator re-read the hook JSON and re-hashed the install. The shift counts stay unread.

**C1aj (format enumeration round 1, offline, [note](../scratch/phase-c1aj-format-keep.md)): `0x473B1E` copies 32 bytes of the enumerated record and returns 1 on both keep and reject.** The popcount arm is `0x473C95` `call 0x49D010`. It requires `[format+4] & 0xE0000` clear, `[format+0xc] == 0x10`, bit 0 of `[format+4]` set, and the popcount of `[format+0x1c]` greater than 1. `0x473D97` is the only `ret`. The upload does not read `eax` after either `call [eax+0x20]`. The stop is outside the exe. The facilitator re-read the keep and the `ret`. The numeric mask stays unread.

**C1ai (alpha-count round 1 this run, offline, [note](../scratch/phase-c1ai-upload-when.md)): `^tab` and `^tab_selected` are registered from the PE entry point, and the upload's phase is unnamed.** Sixteen calls in `0x45E51F`, including `0x45E901` and `0x45E919`, run from `0x49D518` through `0x49D5E1` and `0x450895`. `0x45FE10` does not call `0x472F99`. The only direct call of the upload is `0x47241A`, from `0x4743DD`, which has no direct caller. `0x4B19FC` holds that function. The facilitator re-scanned `.text`. The shift counts stay unread.

**C1ak (alpha-count round 2 this run, offline, [note](../scratch/phase-c1ak-vtable-call.md)): `0x471746` calls `[edx+0x14]`, and that phase is unnamed.** The instruction is inside `0x471720`, which is vtable `0x4B19E8` slot `+0x0C`. On that vtable the target is `0x4743DD`. On vtable `0x4B1A00` the same instruction calls `0x49C134`. `0x471230` overwrites `0x4B19E8` with `0x4B1A00` at `0x47141F`. `0x474380` leaves `0x4B19E8` and is called from `0x46F16F`. There is no direct caller of `0x471720`. The facilitator re-read the call. The shift counts stay unread.

**C1al (alpha-count round 3 this run, offline, [note](../scratch/phase-c1al-ctor-caller.md)): the constructor that leaves vtable `0x4B19E8` runs at process startup, and it does not call `0x471720`.** `0x49D5AB` inside the PE entry calls the walker `0x49D345` over `[0x4BF000, 0x4BF0DC)`. `0x4BF0B0` holds `0x468D20`, and that chain's last call is `0x46F16F` `call 0x474380`. The upload call stays unnamed. The facilitator re-read the chain. The shift counts stay unread. The question is at its cap.

**C1am (launch-attach round 1, offline, [note](../scratch/phase-c1am-launch-hook-dry.md)): the four alpha hooks append on entry, and `python` has no `frida` module.** `0x473B1E` is `push ebp`. `onEnter` reads the record at `[esp+4]` and five context bytes at `+4`. `0x472F99` is `push ebp`; the known call passes `[ebp-0x588]` in `ecx` and on the stack. `0x4736B6` compares `[ebp-0x128]` with 0 and jumps to `0x473B18` when it is zero. `0x473897` is `sar eax, cl`, with the shl count still in `[ebp-0xad8]`. The probe source spawns suspended, loads the script, then resumes, and reads the log after the pause. The facilitator re-read the four sites and a one-address change made the dry check fail. Spawn was not executed. The shift counts stay unread.

**C1an (launch-attach round 2, offline, [note](../scratch/phase-c1an-frida-import.md)): `python` can import `frida` 17.24.3.** `python -m pip install frida` installed that wheel only, into the user site-packages. The c1am hook and probe were not edited. `frida.spawn` was not called. The facilitator re-imported the module and re-ran both dry checks. The shift counts stay unread.

**C1ao (launch-attach round 3, one stock start, [note](../scratch/phase-c1ao-launch-read.md)): the spawned process exited before the level, and the shift site logged 0 hits.** Pid 20612, `frida.resume` at 21:43:53, gone at 21:43:58. `hits.log` has 9 upload lines, 28 format records and 9 mask tests: eight masks are 0 and the last is 32768. `0x473897` has no line. Upload names are null. Install hashes match. The facilitator recounted the log and re-hashed the install. The paused-level shift counts stay unread.

**C1ap (hook-repair round 1, offline, [note](../scratch/phase-c1ap-hook-repair.md)): no branch target falls inside a hook at `0x473897`, and the counts are readable at `0x47383C`.** The `sar` is 2 bytes, so a 5-byte patch also covers `mov ecx, [ebp-0xad8]` through `0x47389E`. A `.text` branch scan and a dword search found no target in `0x473898`..`0x47389E`. `[ebp-0xae8]` is stored at `0x4737F4` and `[ebp-0xad8]` is stored as 0 at `0x473806`, both before `0x47383C`. The per-pixel back-edge is `0x473955` to `0x473866`. `0x4737E6` skips this site and the `sar` together when `[object+0x24] & 0x18` is 0. The upload argument is the object; `0x472212` copies `[ebp+8]` to `[ebp-0x348]`. The early exit stays unnamed. The facilitator re-disassembled the span, re-scanned the branches, and a one-address change failed the dry check. No stock start.

**C1aq (hook-repair round 2, one stock start, [note](../scratch/phase-c1aq-launch-read.md)): the repaired probe exited before the level, and `0x47383C` logged 0 hits.** Pid 25964, resume 23:20:21, Frida `process-terminated` at 23:20:23, `detach_crash` null. The log has 9 uploads, 28 format records and masks eight 0s then 32768. The hook's `texture` fields are `_btn_x`, `_btn_cir`, `_btn_squ`, `_btn_tri`, `_btn_up`, `_btn_down`, `_btn_left`, `_btn_right`, then `Sheets\$LemBits0`. `OpenProcess` failed with 87 three seconds after the detach, so `exit_code` is null. Install hashes match. The facilitator recounted the log and re-hashed the install. The paused-level shift counts stay unread.

**C1ar (hook-repair round 3, offline, [note](../scratch/phase-c1ar-exit-capture.md)): the exit code is read inside the detach callback, and the mask hook logs `[[ebp+8]+0x24]`.** `0x4737DB` reloads `[ebp+8]`, `0x4737DE` loads `[edx+0x24]`, and `0x4737E6` jumps to `0x47395A` when that dword AND `0x18` is 0. Every `[ebp+8]` operand from `0x472F99` through that test is a load. The facilitator removed the callback read and the dry check failed, then restored it. No stock start. Pid 25964's exit code stays unread.

**C1as (hook-repair round 4, one stock start, [note](../scratch/phase-c1as-launch-read.md)): the remaining start exited with `0xC0000005`, and the count site was on the skip path.** Pid 44772, resume 23:38:48, `process-terminated` at 23:38:50. `exit_code` is 3221225477. `detach_crash` is null. The log has 9 uploads, 28 format records, masks eight 0s then 32768, and `skip_input` 1 eight times then 2. Each `skip_input` AND `0x18` is 0, so `0x4737E6` skips `0x47383C`. Install hashes match. The facilitator recounted the log and re-hashed the install. The paused-level shift counts stay unread. The start pool is spent.

**C1av (alpha-count offline round 3, [note](../scratch/phase-c1av-skip-write.md)): the skip path stores masked 16-bit pixels and does not store the shifted sample.** From `0x47395A` through `ret 4` at `0x473B1B`, bits 0, 1 and 2 of `[object+0x24]` gate word stores through `[ebp-0xad4]`. Bit 0 and bit 1 AND the pixel with `[ebp-0xad0]` and OR the low word of `[ebp-0x128]`; bit 1 still ORs a zero word unless the word is `0x1CE7` or `0x2108`. Bit 2 only ORs, and skips a zero word. No branch from that span enters `0x473866`..`0x473955`, so `0x473943` does not run. The facilitator re-disassembled the stores, and expecting `66 89 11` at `0x4739A6` failed. No stock start. The numeric mask stays unread.

**C1at (alpha-count offline round 1, [note](../scratch/phase-c1at-descriptor-link.md)): the plate pointer and the upload object are different loads, and both use `+0x24`.** `0x41493E` stores the return of `0x414A10` at `[ebp-4]` and `0x414944` passes it to `0x47261F`. The upload allocates `0x28` bytes at `0x47448E` and `0x47220B` writes `[object+0x24]` from the name-prefix flag; a leading `0x5E` stores 8 at `0x474471`. `0x47241A` passes that allocation to `0x472F99`. The facilitator re-disassembled those stores, and a zero immediate at `0x474471` failed. No stock start. An insert of that allocation into the pool node is unread (`0x414FA0`, `0x414CC0`, `0x414E98`).

**C1au (alpha-count offline round 2, [note](../scratch/phase-c1au-context-keep.md)): context bytes `+4`..`+7` are computed in `0x473146`..`0x4731C7`. The coverage sentence is corrected by C1ax.** `+4` is 1 when `[object+0x24] & 0x18` is nonzero. `+5` is 1 when that dword's low 3 bits are nonzero. `+6` is 1 when the word at `[ebp-0x192]` is `<= 8`. `+7` is 1 when `[ebp-0x140] & 4` is 4. A `^` name stores flag 8, so `+4` is 1 and `+5` stays 0. C1au read the `0x7c` fill at `[ebp-0x18c]` as ending at `[ebp-0x110]` and not covering `[ebp-0x140]`, and withdrew "always 0" for `+7`. C1ax rejects that coverage sentence. The saved `^TAB.BMP` is 24 bpp. If `+6` is 0 and `+7` is 0, the c1aj popcount arm keeps c1as line 29, mask `0x0000F000`; if `+7` is 1, none of the 20 records pass. The facilitator re-disassembled the stores and re-applied the rule. No stock start. The worker's "always 0" for `+7` is withdrawn.

**C1aw (the one menu-attach start, [note](../scratch/phase-c1aw-menu-context.md)): pid 35216 exited with `0xC0000005`, and `^tab` did not reach `0x473146`.** One `Popen`, session 1, armed at the menu before the click. `hits.log` has one line, `Sheets\$LemSheetHi8`, dword `0`, context `0,1,1,0`, flags `2`. The facilitator recounted the line, re-hashed the exe, `SavedGame.dat`, and `dgVoodoo.conf` (they match the pre-launch copies), and a one-address change of the dry check failed. The in-process level slot was not written back because the process was already gone. Not relaunched. The `^tab` dword stays unread. The gate is not scored.

**C1ax (fill coverage round 1, [note](../scratch/phase-c1ax-fill-coverage.md)): the fill writes 0 through `[ebp-0x140]`, so `+7` is 0 when the read runs.** `0x473046` pushes `0x7c`, `0x473048` pushes 0, `0x47304A` takes `[ebp-0x18c]`, and `0x473051` calls `0x49D760`. That callee writes the count in bytes of the fill value. The interval is `[ebp-0x18c, ebp-0x110)`, and offset `0x4c` is inside it. Nothing from `0x473056` to `0x473146` stores the dword, and the read is not reachable without the call. `0x473157` stores the `sete` result 0, which is c1au's `+7`. Whether `^tab` reaches `0x472F99` stays unread. The facilitator re-disassembled the call and the callee, and expecting count `0x7b` failed. No stock start. The keep rule was not applied.

**C1ay (kept-format round 1, [note](../scratch/phase-c1ay-kept-format.md)): `+6` reads a `GetObjectA` word, and the number is unread.** `0x473040` calls `[0x4B005C]` with a `0x18`-byte buffer at `[ebp-0x1a4]` and the handle `[eax+4]`. The slot names `GetObjectA`. `[ebp-0x192]` is buffer `+0x12`. `0x473179` compares that word with 8 and stores the `setle`. The displacement occurs only on that read. `0x472518` is `mov cx, [eax+0xe]` and does not store the slot. The facilitator re-read the call, the name, and the compare, and expecting `GetObjectB` failed. No stock start. The c1as log was not applied.

**C1az (kept-format round 2, [note](../scratch/phase-c1az-dib-handle.md)): `GetObjectA`'s handle is `[object+4]`, and the bit-count word is not in the file.** `0x47255B` stores `CreateDIBSection`'s return at `[edx+4]` of the object `0x47218A` saved at `[ebp-0x588]`. `0x47241A` passes that object to the upload. The creation call's bitmap pointer is `0x58D5B8+0x0E`, and the word at `0x58D5D4` is the one the stride multiplies. `.data` file bytes end at `0x4C6000`. `0x45015A` produces the buffer copied onto `0x58D5B8` and was not opened. The facilitator re-read the store and the call, and expecting byte `0x88` at `0x47255B` failed. No stock start. `+6` stays unread.

**C1ba (kept-format round 3 of 3, [note](../scratch/phase-c1ba-bitcount-source.md)): `0x45015A` does not write the bit-count word.** No instruction in the function has displacement `0x1C`. `0x450233` compares the buffer's first byte with `0x42`, and `0x450248` compares the next with `0x4D`. A match leaves the pointer stored at `0x4501FC` from `0x49C1A8`. A mismatch reaches `0x4503B3` and `0x4503FD`, which were not opened. `0x4C2500` is `Textures\%s.bmp`. The facilitator walked the 217 instructions, and expecting compare byte `0x41` failed. No stock start. The question is parked. `+6` stays unread.

**C2l (cursor clause round 1, [note](../scratch/phase-c2l-cursor-exit.md)): `_curExit` has one registrar push.** The string is at `0x4C0DD0`. The dword occurs once, as `push 0x4C0DD0` at `0x45E89B`, then `mov ecx, 0x58CB48` and `call 0x45FE10`. It does not name `_curLem` or `_curNoLem`. The facilitator searched the file for the dword, and expecting the `x` byte `0x79` failed. No stock start. The hover rule was not read.

**C2m (cursor clause round 2, [note](../scratch/phase-c2m-cursor-slot.md)): the `_curExit` registration is stored at `0x4E333C`.** `0x45E8AD` is `mov [edx+0x1C934], eax` after `call 0x45FE10`. `edx` is the `ecx` saved at `0x45E525`, and the one direct call sets that `ecx` to `0x4C6A08`. `.data` file bytes end at `0x4C6000`, so the runtime dword is unread. The facilitator re-read the store and the caller, and expecting displacement byte `0x35` failed. No stock start. No reader of the slot was opened.

**C2n (cursor clause round 3 of 3, [note](../scratch/phase-c2n-cursor-read.md)): two instructions load the `_curExit` slot.** `0x45ECBF` and `0x45EE3A` are `mov eax, [edx+0x1C934]`. The first calls `0x414B90`. The second calls `0x414D20` unless `[object+0x1C9B1]` is nonzero. The file contains the displacement three times, including the store at `0x45E8AD`. The facilitator re-read both loads, and expecting opcode `0x89` failed. No stock start. The callees were not opened. The question is parked.

**B3a (map-draw round 1, [note](../scratch/phase-b3a-map-arm.md)): no instruction selects screen id 5.** `0x4C416C` is used three times in `0x4882C0`: load into `ecx`, store at `[eax+0x2c]`, add 1, store back, then `ret` with no `call`. The file dword is 1. The facilitator found those three immediates and no others, and expecting the add immediate `2` failed. No stock start. The draw call stays unread.

**B3b (map-draw round 2, [note](../scratch/phase-b3b-map-object.md)): the creations are not in one order.** `0x4872B8` is `jmp dword ptr [ecx*4+0x487AB3]` after `cmp [ebp-0xb4], 0x24`. There are 20 direct calls of `0x4882C0`. The facilitator re-read the dispatcher and counted the calls, and expecting the bound `0x25` failed. No stock start. The object that receives 5 stays unread.

**B3c (map-draw round 3, [note](../scratch/phase-b3c-map-index.md)): the jump index is `[ebp+8]`, not the creation counter.** `0x487294` loads that argument and `0x487297` stores it at `[ebp-0xb4]` immediately before the compare. The prologue has no operand `0x4C416C`. The facilitator re-read those bytes, and expecting displacement `0x0C` failed. No stock start. What the callers pass stays unread.

**B3d (map-draw round 4, [note](../scratch/phase-b3d-map-callers.md)): both callers pass their own `[ebp+0xC]`.** `0x486FF7` and `0x488713` are the two direct calls of `0x48726F`. The last push at each site is that argument, not `0x4C416C` and not a constant. `0x4886EB` skips the second call when the dword is 3. The facilitator re-read both sites and counted the calls, and expecting displacement `0x08` failed. No stock start. The question is parked.

**B3e (level-pick round 1, [note](../scratch/phase-b3e-level-name.md)): `MM_CLIMB_002` is not in the executable.** The exe has no ASCII or UTF-16 copy of the name and no `.LVL`. `lemmings.box` at `0x6F77` is the path `COMPILED\MM_CLIMB_002.LVL`. The facilitator re-read those bytes, and expecting `0x4E` failed. No stock start. No code reference.

**B3f (level-pick round 2, [note](../scratch/phase-b3f-box-ref.md)): three instructions push `LEMMINGS.BOX`.** Lowercase and UTF-16LE `lemmings.box` are absent. The strings are at `0x4C292C`, `0x4C2A60`, and `0x4C2A70`, pushed at `0x47DF27`, `0x47E298`, and `0x47E2CC`. The facilitator found each address once, and expecting `0x4D` failed. No stock start. The calls after the pushes were not opened.

**B3g (level-pick round 3 of 3, [note](../scratch/phase-b3g-box-call.md)): each `LEMMINGS.BOX` push calls `0x49C760`.** The other argument is a stack buffer, `[ebp-0x110]` or `[ebp-0x104]`, and each call is followed by `add esp, 8`. The facilitator re-read the three calls, and expecting relative byte `0x29` failed. No stock start. No level name is in those bytes. The question is parked.

**C1bb (value-framed round 1, offline, [check](../scratch/phase-c1bb-derived-format.md)): the installed `patch.dll` bypasses native format selection for successful `^tab` alpha uploads.** Exe `0x472FAD` imports `patch.dll!DirectDrawEnumerateA`; argument 1 returns hook RVA `0x1879` in patch SHA `c9a09caa…`. Nonzero `object+0xC` enters its alpha upload, requests DERIVED A8R8G8B8 from DDPIXELFORMAT immediates at preferred `0x6CBC19B7..19F2`, then returns 4; exe `0x472FBC` jumps to `0x473B18`, before native +6/enum/shift. Conditional native +6=0 keeps only c1as line 29 A4R4G4B4, but that is not this install's successful `^tab` path or a measured `^tab` list. No format fit, live execution or pixel acceptance claimed. Independent byte/branch/list checks and 34/34 CTest pass. Next: patch converter/resizer texel words, not native 16-bit shifts.

**C1bc (texel-word round 1, offline, [check](../scratch/phase-c1bc-patch-texels.md)): both plate assets resize 72x32 to 128x32 in the installed patch, full UV [0,1].** Converter flips positive-height DIB rows, not `ALF[y*72+x]`; tab RGB0, selected RGB255. Resizer RVA1254 is endpoint-aligned bilinear with binary32 coordinates and truncating channel stores, no native4-bit alpha reduction. Independent p53-nearest full-stream replay matches both assets. Entry PC/RC remains conditional: nearest selected p24 differs at only (11,24), alpha150 vs151. Completed GDI and returned format/pitch512 are explicit conditions, not observed original output. 34/34 CTest pass. Next is a conditional plate render/score, not another unread-edge-only round.

**C1bd (render plus two repairs, offline, [check](../scratch/phase-c1bd-plate-render.md)): conditional plates now draw in `--skill-panel`; C1 gate fails all three views.** Frozen footprint23166pixels: zoom1 **609151/17721990=0.0343726071**, elev_up **981264/17721990=0.0553698541**, elev_down **610758/17721990=0.0344632854**, all>0.02. All-unselected state and pixel-center nearest raster remain hypotheses, not native facts. Own fresh images, independent nine-view histogram re-score, isolated alpha-only mutant (zoom1 error1936862), exact restoration and34/34CTest pass. Plate-exclusive errors325738/669975/323854 exceed icon/glyph283413/311289/286904; regions, not causes. Full inter-band gap minus footprint has5058pixels; worker's4536 stratified sample is not the historical diagnostic. Next numerical plate filter/alpha state; zero-count text skip is already READ in C1v.

**C1bf (zero-count round 1, offline, [check](../scratch/phase-c1bf-zero-count.md)): a zero count skips the text submit, and the frozen gate still fails.** `0x45B6E4` compares `[edx+ecx*4]` with 0 and `0x45B6E8` jumps over `call 0x44E150`. Own restored renders match the worker PNGs. Footprint 23166 pixels: zoom1 **414292/17721990=0.0233772844**, elev_up **784866/17721990=0.0442876900**, elev_down **415022/17721990=0.0234184761**, all >0.02 and all below C1bd. One guard removal fails the destination test and returns zoom1 to 609151. Restored hash and 34/34 CTest. No stock start. Selection and filter were not drawn.

**C1bg (selected-index round 1, offline, [check](../scratch/phase-c1bg-selected-index.md)): the no-skill init selects index 0 and sets the enable byte.** DERIVED, not a pause-memory read. `0x45A629` writes fallback 7. The loop at `0x45A6B9` starts at 0 and stops before 10; `0x45A85E` `jle` skips a count that is not strictly positive; `0x45A866` stores the first such index and `0x45A898` sets `+0x2750` to 1. `MM_CLIMB_002` trailer dwords are 10,0,0,0,0,0,0,0,0,1 then 8, so index 0 replaces 7. Draw `0x45B67E` skips the extra quad when the byte is 0; otherwise `0x414870` gets 18, `40*i+12`, 90, `40*i+44`. `^tab_selected` is at `0x4C0E08`. The A7p script's primary clicks are map node `(206, 204)`. Next/previous handlers were not re-opened. No stock start and no score.

**C1bh (selected-draw round 1, offline, [check](../scratch/phase-c1bh-selected-draw.md)): the derived selected quad lowers two gated views under 0.02 and leaves elev_up failing.** `TEXTURES\^TAB_SELECTED.BMP` is resized with the conditional plate resizer and drawn at index 0, rectangle 18,12,90,44, after the ten base plates. Footprint 23166 pixels: zoom1 **286491/17721990=0.0161658482**, elev_up **580420/17721990=0.0327514009**, elev_down **287373/17721990=0.0162156169**. Own re-renders match the worker PNGs. Removing only that call fails the destination test and returns zoom1 to the C1bf image, 414292. Restored hash and 34/34 CTest. The pixel-center sampler and straight alpha stay a hypothesis. No stock start.

**C1be (plate-filter round 1, offline, [check](../scratch/phase-c1be-plate-filter.md)): stage-0 mag and min are one local, and it is 2 when the argument and the device byte are both nonzero.** `0x470F32` sets the local to 1. `0x470F4D` keeps 1 when the argument's low byte is 0. `0x470F5C` keeps 1 when `[this+0x3A1]` is 0. `0x470F5E` stores 2 otherwise, then `0x470F73` pushes `0x10` and `0x470F90` pushes `0x11`, with no skip of either call. The ten `0x45B66A` submits run before `0x45B6D9`'s `push 0`. `0x46F234` stores immediate 1, and `0x45F1E8` calls that function with `ecx = 0x58CD80`. MinGW `d3dtypes.h` names 16/17 mag/min and 2 LINEAR. No direct filter call sits between `0x441845` and `0x441A71`; the indirect callees there were not re-opened. No stock start and no score.

**C1bi (linear-sample round 1, offline, [check](../scratch/phase-c1bi-linear-plate.md)): a truncating linear sample of `s = x * 128 / 72` does not pass elev_up.** Weights are `(72-rem)/72` and `rem/72`. Request 2 stays the DERIVED LINEAR name; the kernel is a hypothesis. Footprint 23166 pixels: zoom1 **285083/17721990=0.0160863989**, elev_up **580972/17721990=0.0327825487**, elev_down **285700/17721990=0.0161212144**. Against C1bh, zoom1 and elev_down fall by 1408 and 1673, and elev_up rises by 552. Plate-only elev_up error is 466081; icon-and-glyph error is 114891. Omitting the plates raises elev_up to 2055954. Restoring nearest returns zoom1 to the C1bh image. 34/34 CTest. No stock start.

**C1bj (plate-over-stock round 1, offline, [check](../scratch/phase-c1bj-plate-over-stock.md)): drawing the current plates onto the stock frame raises the plate-only error.** The baseline already contains the original plates. Full numerators over 17721990 are zoom1 1123813, elev_up 1145908, elev_down 1114207. Plate-only numerators on 18033 pixels are 669750, 691289, and 666306, each higher than the C1bi plates-over-our-frame numbers 196523, 466081, and 194526. Production source was not edited. On elev_up the plate interior is 384401 of 466081 with the plates drawn and 1589553 with them omitted. Alpha forced to 255 raises plate-only elev_up to 2519199. The icon-and-glyph share, 114891, cannot bring the 580972 elev_up numerator under 354440. No stock start.

**C3ag (CTest wiring, [check](../scratch/phase-c3ag-retail-env.md)): configured CTest now passes the install environment to both scene-source retail tests.** With the parent variable unset, AC runs443checks and AA31; a single AC ENV removal yields AC48/retail-skip while AA31 remains. Generic green CTest alone was not retail coverage. Exact restore and39/39CTest pass. No new native fact or accepted C3 score change.

