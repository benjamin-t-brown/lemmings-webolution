# C1b: the ten skill-panel icons' sheet was not located (clean negative); compositing primitive built, tested, not wired

Queue `c1b-panel-icon-render`, round 1 of 3 on this question. Offline-only, as
required: zero stock starts, no live game process at any point. Worker packet
`build/tmp/c1b-panel-icon-render/round-note.md`.

## Result

**The ten base skill-panel icons were not located.** This round read the three
sprite-sheet strings `GreyIcons`/`BlackIcons`/`AdditiveIcons` LESSONS.md already
named as unchased, decoded their real archive `.SPR` files, and searched every
other `.SPR`, every `TEXTURES\SHEETS\*` and bare `TEXTURES\*` entry, and the
stock `.exe`'s PE resource table. None is a ten-frame, 72x32, panel-index-
ordered icon strip. The native draw call that actually paints the ten buttons'
base artwork was traced as far as the hover/selection update chain
(`FUN_00454d6c` -> `FUN_004532e2` -> `0x45cff9`, confirmed to contain no
texture/sprite/blit call) and stops at an untraced per-skill vtable — new,
open-ended native work beyond "wire one additive HUD draw to an identified
sheet," which this round's brief scoped it to stop short of.

Because no confirmed base-icon sheet exists, nothing was wired into
`main.cpp`/`TubeScene`. What was delivered: a tested compositing primitive
(`src/app/SkillPanel.h`/`.cpp`, additive, not yet called from production code)
and the acceptance score computed exactly as specified — both fail.

## What was read (REA, Ghidra, read-only; archive bytes via two independent decoders)

- `GreyIcons` (`0x4c0d44`), `BlackIcons` (`0x4c0d50`), `AdditiveIcons`
  (`0x4c0d5c`): exactly one xref each (`0x45e77b`, `0x45e793`, `0x45e7ab`),
  independently re-read by the facilitator and matching the worker's claim
  exactly.
- All three xrefs resolve into one function, `FUN_0045e51f`
  (`0x45e51f`-`0x45ed80`), independently re-decompiled by the facilitator: it
  registers 22 audio-cue handles via `FUN_0041d7d0`, then calls a *different*
  registrar, `FUN_00446040` — the same function round 23 already identified
  for `Axe` — for exactly six strings (`Bits`, `GreyIcons`, `BlackIcons`,
  `AdditiveIcons`, `Axe`, `Smoke`), storing the returned handles at
  `this+0x1c900`..`this+0x1c914`, then registers the cursor/HUD-counter set via
  a third registrar, `FUN_0045fe10`. No draw call anywhere in this function —
  it only loads handles into the UI-state struct. This independently confirms
  the worker's central claim and corroborates, rather than contradicts, round
  23's `Axe` finding.
- The climber skill constructor `0x413c70` (already located by C1a): sets a
  vtable pointer and two small fields only, no inline icon reference. Any icon
  identity reachable from a skill object would be through that vtable's
  virtual dispatch, not traced here.

## What was independently re-decoded (facilitator, from scratch, bypassing the project's own `.SPR` reader)

A standalone Python parser, following only `src/assets/Sprite.h`'s own
documented field-order comment (not importing any project code), decoded the
worker's saved raw extracts (`build/tmp/c1b-panel-icon-render/extract/*.spr`)
and reproduced every cited number exactly, including that each file's byte
count is consumed exactly to the palette field (no misalignment):

| File | Bytes | Sprite name | Animations/frames | Max frame |
| --- | ---: | --- | --- | --- |
| `greyicons.spr` | 118 | `bomb` | 1 anim, 1 frame | 18x25 |
| `blackicons.spr` | 1492 | `questionmark` | 16 anims, 1 frame each | 20x19 |
| `additiveicons.spr` | 1191 | `armageddon` | 13 anims, 1 frame each | 15x18 |

None is ten frames, none is 72x32, none is panel-index-ordered. This falsifies
the brief's own pre-registered hypothesis ("three sheets for three visual
states, matching the three distinct strings") from two independent decoders
now, not one.

## Facilitator check

- Fresh rebuild from a touched/clean `ninja` state (not "no work to do"):
  `cmake --build build/cmake/dev-debug` compiles `SkillPanel.cpp`,
  `test_skillpanel.cpp` and relinks `webolution.exe`/`test_skillpanel.exe`
  cleanly. (An editor-only clangd pass flagged missing-include/unknown-type
  diagnostics on these new files; the actual CMake/ninja build — the real
  compiler, with the project's real include paths — has none. The diagnostics
  were a language-server include-path artifact, not a build defect.)
- Fresh `ctest --test-dir build/cmake/dev-debug` (with `WEBOLUTION_GAME_DIR`
  set): **30/30 passing**, including the new `skillpanel` target run verbosely
  (34/34 checks, reproducing the exact sheet/frame-count table above against
  the real archive through the production `webo::loadSpriteSheet` reader).
- **Mutate-one-alone**: swapped the one blend-formula line in
  `compositeIconRegion` (`s[c]*a + d[c]*(255-a)` -> `s[c]*(255-a) + d[c]*a`).
  Exactly one check failed (`partial blend: R channel is 128, was 127`); every
  other check, including the archive-negative ones, was unaffected. Reverted;
  `sha256sum src/app/SkillPanel.cpp` is byte-identical before and after
  (`690a8b3a...`). Rebuilt and reran the full suite: 30/30 again.
- **Independent REA session** against the installed exe (SHA-256
  `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`, matching
  the authority constant): re-read all three string addresses/xrefs and
  re-decompiled `FUN_0045e51f` directly — see "What was read" above. Read-only;
  session closed cleanly, no write.
- **Independent score reproduction**: reran `python tools/harness/score_c1_panel.py`
  fresh. Got exactly `initial` skill_numerator=15157, skill_pixels=184, MAE
  0.10767973856209151 (fail); `a7q_mm_dig_000` skill_numerator=3646,
  skill_pixels=86, MAE 0.0554187566499468 (fail) — identical to the worker's
  own table. The `a7q` figures (86 pixels, 3646 error units) also match
  **docs/scratch/phase-a7-second-level-mask.md`'s own already-landed text**
  exactly, an independent cross-check from a separate round's separate script.
  `initial`'s `wider_numerator`/`wider_denominator` (2616762/135529695) match
  `docs/scratch/phase-a7-unpause-view-masks.md`'s existing `initial` row
  exactly, confirming this round's fresh `--shot` re-render reproduces the
  unchanged production output (no accidental regression from the `CMakeLists.txt`/
  source-list additions).
- `git status --short`: only `CMakeLists.txt`, `cmake/webolution_sources.cmake`
  (both purely additive — reviewed the diff directly, confirmed no existing
  target/test/source entry removed or reordered beyond insertion), the four
  new files the brief's allowlist permits, and `docs/tasks.json`'s two
  permitted fields (`status`, `worker_output`). No `src/app/main.cpp` or
  `src/app/TubeScene.cpp` change — confirmed the worker's claim that nothing
  is wired yet.
- No `Lemmings Revolution.exe`/`webolution.exe` process found running, before
  or after this check. No stock save/config file touched. No REA write
  operation issued.

Round accepted as reported; nothing narrowed. This is a genuine, well-evidenced
clean negative (two independent decoders, one independent REA re-read, one
independent score reproduction cross-checked against a different round's
already-landed numbers) plus a real, tested, additive deliverable — not a
failure to try. C1's rendering clause is **not** accepted; the skill-only MAE
fails by a wide margin on both fixtures, for a measured reason (no icon pixels
exist in the panel region yet at all, not a wrong icon).

## Recorded, not yet investigated (leads for a later round)

- `SPRITES\LEMMINGSLO.SPR` (the lemming's own in-level action-pose sheet) has
  8 animation names matching 8 of the ten skill names (`bash`/`block`/`build`/
  `climb`/`dig`/`explode`/`mine`/`float`). Not confirmed: not traced to any
  draw call, and these frames are 23-39px wide (the full-body in-level pose),
  not measured against the 72x32 button cell. Do not wire this without tracing
  an actual draw reference — a name match alone is exactly the kind of guess
  `match-the-original.md` forbids.
- `SPRITES\BITS.SPR` decodes to the lemming's death-gib body parts
  (`head`/`body`/`arm`/`leg`/`rathead`/`ratbody`/`arm`/`ratleg`) — unrelated to
  icons, recorded so it is not re-chased.
- `TEXTURES\_LIMIT0.BMP`..`_LIMIT9.BMP` (ten tiny digit glyphs, 134-246 B each)
  are a plausible skill-count digit source for C2/a later C1 round — not used
  or investigated here, per this round's own digit/font stop rule.
- `GreyIcons`->`bomb` (one frame), `BlackIcons`->`questionmark` (16 frames, a
  cycling badge), `AdditiveIcons`->`armageddon` (13 frames, a cycling badge)
  read as state-change **badges** for specific buttons (bomber greyed-out?
  fast-forward's multiplier cycle; Armageddon's detonation animation) rather
  than the ten buttons' base icon strip — consistent with, and extending, C1a's
  existing index8=`questionmark`/index9=Armageddon facts. Not traced to a
  draw call; not acted on.

## Next

The remaining open question for this gate clause is **where the ten buttons'
base icon pixels actually come from** — through the untraced per-skill vtable
dispatch (the most direct lead: decompile the vtable slot each of the ten
skill constructors C1a already located points at, looking for a draw/blit
call), or confirmed via the `LemmingsLO` lead above. This is round 1 of 3 on
this question (`docs/ORCHESTRATION.md#milestones-gates-and-round-caps`); two
rounds remain before parking in Phase E.

## C1c: both pre-registered leads also falsified; one concrete unresolved thread found (round 2 of 3)

Worker packet `build/tmp/c1c-panel-icon-draw-trace/round-note.md`. Offline REA
only, read-only, zero stock starts, no `src/**` change (read-only round).

### Result

A further-narrowed clean negative, not a located draw call:

- **All nine skill-constructor vtables** (C1a's addresses) were read and every
  one of their ~50 deduplicated virtual-method bodies decompiled. None
  contains a texture/sprite/blit call or references the sheet-loader
  `FUN_00446040`. They are simulation physics/eligibility/cell-flag code, or
  (for several skills) a call into `FUN_0042fd25` that sets the **lemming's
  own in-level action-pose animation name** (`"build"`/`"float"`/etc.) — the
  input to the already-identified actor-rendering pipeline, not the panel.
  This also explains C1b's `LemmingsLO.SPR` name-coincidence lead: those are
  the lemming's own pose names, now confirmed reachable from the skill-tick
  code, but still feeding the in-level actor sprite.
- `SPRITES\LEMMINGSLO.SPR`'s own name string does **not exist anywhere** in
  the exe's string table (`search_strings` for `"LEMMINGSLO"`, zero matches;
  a broader `"Lemmings"` search found nothing matching), so the lead is not
  just unconfirmed, it is unconfirmable by any xref path.
- A new candidate was traced: the per-level main loop `FUN_0045cff9`
  (`0x45cff9`-`0x45e486`, confirmed sole caller `0x450696` — the application's
  own per-level message pump). Its sibling call next to C1b's already-read
  hover chain, `0x453027`, is camera-pan/edge-scroll math, not a draw. Its
  unconditional per-frame call, `FUN_00454a85`, **does** contain a confirmed
  real draw call into `FUN_00414920` (the same primitive A4r/A4t already
  identified for actor rendering) — but for exactly **one** resource-indexed
  overlay quad (field `levelScreen+0x2a`) and one text-glyph list (field
  `levelScreen+0x2c`, drawn via `FUN_0044e150`), never a ten-slot,
  panel-index-ordered array.
- `xrefs` on `panel_boxes` (`0x4c9208`) returns exactly 9 references, all
  inside the already-read hit-test function (`0x454d6c`-`0x455656`) — no
  second, draw-side reader of this table exists anywhere in the binary.
- **One concrete, unresolved thread remains**: `FUN_00454a85` ends with
  `(**(code**)(**(int**)(param_1+0x3c)+4))()` — an indirect virtual
  "draw yourself" dispatch on whatever object lives at `levelScreen+0x3c`,
  which REA reports as an unresolved computed/indirect call (its own stated
  limitation). Its concrete target was not identified this round. Four
  state-transition functions inside `FUN_0045cff9` (win/lose/AVI-only, not
  per-frame) were also left untraced as poor candidates for a continuously-
  visible panel.

### Facilitator check

- Fresh, independent REA session against the installed exe (SHA-256
  `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`, matches
  authority). Re-ran `search_strings` for `"LEMMINGSLO"` (case-insensitive):
  **zero matches**, confirming the worker's claim exactly.
- Re-ran `xrefs` on `0x4c9208`: exactly 9 references
  (`0x45480a,0x454840,0x45487d,0x4548b3,0x4548ea,0x4554e0,0x4554f7,0x45550e,0x455525`),
  all inside the cited hit-test function's range — matches exactly.
- Re-ran `procedure_callers` on `0x45cff9`: exactly one caller, `0x450696` —
  matches exactly.
- Independently read 64 bytes (16 dwords) of the climber vtable (`0x4b0978`)
  and decoded it by hand: 14 dword slots, all resolving to addresses inside
  `0x00400000`-`0x00500000` (matching the worker's own "richer base class,
  14-16 slots" claim and its cited slot list), followed by a floating-point
  constant, not a 15th slot.
- Independently re-decompiled `FUN_00454a85` directly: reproduces the quoted
  pseudocode exactly, including both resource-index branches, the
  `FUN_00414920` call, and the unresolved `(**(code**)(**(...)+0x3c)+4))()`
  dispatch verbatim.
- Independently re-decompiled `0x413f15` (climber's cited "position/
  animation-timer update" slot): confirms the description — `this+8`/
  `this+0x38`/`this+0x20` double-precision timer/position math and a
  `FUN_004404fa` text-builder call, no draw/blit primitive.
- No `Lemmings Revolution.exe`/`webolution.exe` process found running, before
  or after this check. `git status --short` showed only `docs/tasks.json`'s
  permitted fields — no `src/**` file touched, confirming the "read-only
  round" claim. REA session closed cleanly, no write.

Round accepted as reported; nothing narrowed. Both of C1b's own pre-registered
leads are now independently confirmed falsified, and one new, concrete,
untraced thread exists (`levelScreen+0x3c`'s vtable+4 slot) that is a stronger
lead than either prior one, because it is a genuine unresolved-by-tool
indirect call on an object this round did not yet identify, sitting directly
on the per-frame draw path that already contains one confirmed real draw
call. This is round 2 of 3; one round remains before parking in Phase E.

## C1d: the ten-widget array and its real icon draw are located; the full
## dispatch to all ten, and the per-widget icon index, remain open (round 3 of 3)

Worker packet `build/tmp/c1d-panel-icon-widget-trace/round-note.md`. Offline REA
only, read-only, zero stock starts, no `src/**` change.

### Result — not a third clean negative

This is the strongest result yet, though still not a closed answer. The level-
screen constructor `0x452bc2` builds **exactly ten** identical sub-objects at a
constant stride of `0x238` bytes (`this+0x70, +0x2a8, +0x4e0, +0x718, +0x950,
+0xb88, +0xdc0, +0xff8, +0x1230, +0x1468` — each delta exactly `0x238`), each via
the same constructor `FUN_004576b0` (used nowhere else in the binary) and the
same six-slot vtable at `0x4b1774`. **Ten identical sibling widgets is exactly
the panel's known physical button count (C1a).** `levelScreen+0x3c` — the field
`FUN_00454a85`'s closing indirect dispatch reads — is set unconditionally, once,
to the *first* widget (`this+0x70`) by this same constructor, and no other
writer of that field was found.

The vtable's slot+4, `FUN_00457868`, is a **genuine, confirmed real icon draw**:
it looks up a resource by the widget's own stored index (`+8`, via
`FUN_00414a10`), computes UV fractions from the widget's own pixel rect
(`+0x21c/+0x220/+0x224/+0x228`) divided by that resource's width/height, writes
them onto an inner quad (`+0x10`), and draws it with `FUN_00414920` — the same
confirmed real draw primitive A4r/A4t/C1c already established. This is exactly
the shape a per-button icon draw needs (resource-index + UV-rect, reused across
ten identical instances).

**What remains open, exactly:** the dispatch `FUN_00454a85` actually reaches
only **one** of the ten widgets (the one fixed at `+0x3c` = widget 0) every
frame; no loop over the ten and no second resolved caller of the slot+4
function or of vtable slot 0 exists anywhere in the binary, per REA's resolved-
call analysis — which, as the worker correctly notes, does **not** prove no
loop exists, only that none was found through *resolved* calls (an indirect
dispatch computed from a runtime loop variable would not appear as a named
caller). Separately, no writer of any widget's own `+8` resource index was
found this round (C1b/C1c already fully read the one function most likely to
hold it, `FUN_004532e2`, and found no `FUN_00414920` call there — consistent
with the actual draw living one level deeper, behind the vtable, as this round
found, but a plain non-call field store to `+8` would not have been visible in
that prior read, since neither prior round was looking for it).

### Facilitator check

Independent REA session (SHA-256 matches authority). Re-decompiled
`FUN_00452bc2` in full: reproduces every cited offset and the ten
`FUN_004576b0` calls at the exact stride-`0x238` sequence, and the
`*(int*)(this+0x3c) = this+0x70` store, verbatim. Re-decompiled `FUN_004576b0`:
matches exactly (vtable store to `0x4b1774`, `+0x218` flag set). Re-decompiled
`FUN_00457868`: matches exactly, including the `FUN_00414a10`/`FUN_00440c50`/
`FUN_0043cda0`/`FUN_00414920` call chain. Independently read 32 bytes of the
vtable at `0x4b1774` and decoded it by hand: `+0x00=0x4579a0, +0x04=0x457868,
+0x08=0x457c00, +0x0c=0x457b70, +0x10=0x457ec0, +0x14=0x457f10`, then two null
dwords — matches the worker's table exactly. `xrefs(0x4b1774)` returns exactly
3 addresses, matching. `procedure_callers(0x457868)` and
`procedure_callers(0x4579a0)` are both empty; `procedure_callers(0x4576b0)` is
exactly `["0x452bc2"]` — all three match exactly. No live process found running
before or after. `git status --short` showed only `docs/tasks.json`'s permitted
fields — confirmed read-only round.

Round accepted as reported; nothing narrowed. This is round 3 of 3 — the
pre-authorized cap for this question is reached, but the result is not a clean
negative that leaves the gate's current state fully characterized; it is a
strong partial positive with two concrete, bounded, nameable open threads
(find the stride-`0x238` dispatch loop, if any; find the `+8` index writer).
See PLAN.md for the facilitator's round-cap-exception decision.

## C1e: both cap-exception sub-questions are clean negatives — question parked (round 4)

Worker packet `build/tmp/c1e-panel-icon-dispatch-index/round-note.md`. Offline REA only,
read-only, zero stock starts, no `src/**` change.

### Result

Both of C1d's two named open threads are now clean, well-evidenced negatives:

1. **No stride-`0x238` dispatch loop exists anywhere in `.text`.** An exhaustive
   `search_bytes` for the dword immediate `0x238` across every initialized segment found
   exactly **7** matches, none inside or near `levelScreen`'s constructor, none a loop
   induction variable, none combined with an indirect call. All 7 are either coincidental
   `CALL rel32` displacements/stack-frame-size constants, or genuine but single-instance,
   non-looped field accesses belonging to one unrelated CSize/path-interpolation class
   (own vtable `0x4b18ac`). The constructor itself uses ten distinct absolute offsets, not
   a repeated `0x238` literal.
2. **No writer of any widget's `+8` resource index exists anywhere searched.** The five
   most-likely candidate functions (`levelScreen`'s constructor, the shared per-widget
   constructor, its own callee, the hover/selection chain, the main per-level loop) were
   each re-read in full for a plain field *store* (not a call) to `+8` — none found,
   confirmed down to the instruction level for the per-widget constructor's callee (every
   field it touches is at or after its own `+0x10`, never backward to `+8`). A second,
   independent method — a literal-immediate scan for all ten `levelScreen`-relative
   absolute `+8` offsets (`0x78, 0x2b0, 0x4e8, 0x720, 0x958, 0xb90, 0xdc8, 0x1000, 0x1238,
   0x1470`) across all of `.text` — found no function containing more than one of the ten,
   and the two apparent hits on functions near the already-known constructor's address
   space are both confirmed unrelated by full decompilation.

Residual, explicitly not excluded by this round (reported honestly, not claimed as
exhausted): a computed dispatch through some *other* stride (e.g. a separately built
pointer array); a per-level screen-setup function assigning `+8` through a widget pointer
directly rather than a `levelScreen`-relative literal; or `+8` simply never being written
(left at its allocation-time value). None of these can be settled by static reading alone;
the most direct next step is a live read of a constructed widget's `+8` field during an
authorized stock start — out of this round's zero-stock-start scope.

### Facilitator check

Independent REA session (SHA-256 matches authority). Re-ran `search_bytes` for the dword
`0x238` (`38 02 00 00`) across `.text`: **exactly the same 7 addresses**
(`0x430564, 0x465b4e, 0x465ff6, 0x46605f, 0x466ac2, 0x468e27, 0x4a2765`), matching exactly.
Spot-checked three of the ten `+8`-offset literal scans independently: `0xb90` and
`0x1470` both return **zero** hits (matching); `0x958` returns **exactly 4** hits, all in
the same already-identified-unrelated function `FUN_00468e22` (matching exactly).
Independently re-decompiled `FUN_004680f0` (the per-widget constructor's own callee,
operating on `widget+0x10`): confirmed every field index it touches is non-negative
relative to its own `param_1` — it never reaches back to `widget+8`. No live process
found running before or after. `git status --short` showed only `docs/tasks.json`'s
permitted fields.

Round accepted as reported; nothing narrowed. Both cap-exception sub-questions are
genuine, well-evidenced negatives, independently reproduced. Per the brief's own
instruction ("no fifth round is pre-authorized regardless of outcome"), **this question is
now parked in Phase E** — see `PLAN.md`. The remaining residual (the other nine widgets'
dispatch mechanism, and the `+8` index assignment) needs a live memory read, not more
static analysis, and is recorded as the resume lead.
