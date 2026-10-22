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
