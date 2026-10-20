# A4f — one actor-bound billboard: geometry verified, pixels not accepted

Queue id `a4f-actor-billboard`. Worker note retained at
`build/tmp/a4f-actor-billboard/round-note.md`; every artifact named below is in that
directory. Stock authority SHA-256 `32675bdb…3737a2f`.

## Result

`src/app/ActorSprite.{h,cpp}` places one lemming billboard from actor position, the A1
camera and the A3-selected `LEMMINGSHI.SPR` record, and draws it nearest-sample with the
`$` colour-key alpha. For the two retained fixtures (five = walk action13/temporal8/dir1,
eight = block action1/temporal0/dir7, both mirrored, scale delta −1) the four destination
corners match the native emitted corners within one binary32 ULP.

**The pixel gate fails.** Candidate RGB MAE is worse than an explicit *absent-actor*
render on both fixtures, so this round does not accept actor pixels:

| Fixture | candidate MAE | absent-actor MAE | sprite-pixel MAE |
| --- | --- | --- | --- |
| five | 0.00797079 | 0.00618553 | 0.2228 |
| eight | 0.00934436 | 0.00684647 | 0.2228 |

(64x64 crop `(437,268)..(501,332)`, actor-suppressed native background from
`live-pairs-4` used only as a comparison oracle; `facilitator-pixels/baseline-fresh-final`.)

## The placement rule (READ, then native-corner MEASURED)

Raw bytes `0x47A75D..0x47AB3F`, re-read by the facilitator:

- `0x47A7B0 call 0x47ABC0` returns the selected directional record (stride `0x48`).
- `0x47A7BB fild [rec+0x34]; fsubr [ebp+0x10]` — Y origin minus `unknown1`, **full scale,
  before any branch**. `0x47A7C7 fild [rec+0x3C]` loads height.
- Scale delta `[0x58D53C] − [0x58CB38]` (`0x47A7CD..D8`) is global, not per actor.
- `0x47A7E0 mov dl,[ctrl+0x1B4]; test; je 0x47A996` — mirror byte selects the branch.
- Mirrored (`0x47A864..`): X `+= unknown0·s`, left `= X − width·s`; Y adds back
  `unknown1·s` only for delta −1 (`0x47A8AE fild [+0x34]; fdiv 2.0; fadd`) and subtracts
  it again for delta +1 (`0x47A910`). Net `−unknown1·s`.
- Unmirrored (`0x47AA0C..`): X `−= unknown0·s`, right `= X + width·s`; no further Y
  term, so Y keeps the full `−unknown1` at every scale. This is a real asymmetry; preserve it.
- `[ctrl+0x1B0]` (`0x47A7F1`, `0x47A999`) separately reorders the texture-coordinate
  arguments to `0x440C50`. **Not modelled.** Both fixtures sit in one `0x1B0` state.

Narrowing: the five fixture has `unknown0 == width/2`, so the mirrored and unmirrored X
formulas agree on it; only the eight fixture discriminates the mirrored X anchor. The
unmirrored branch is READ only — no native unmirrored fixture was captured.

## Why the pixels fail (READ + facilitator diagnostic)

- `0x440A10` initialises the four inline `0x68`-byte vertex records with RGB 255.0
  (`push 0x406FE000:00000000` ×3 → `0x4017F0` → `rep movsd` to `object+0x40`, i.e.
  record+0x38). The worker's live read (`material-live-2/report.json`) has **152.0** in
  all four records for both fixtures. So a runtime writer changes the actor's vertex
  colour after construction; it is not found, and no `152.0` literal exists in the file.
- Facilitator diagnostic (`facilitator_tint_flip_diag.py`, comparison-only): scaling the
  candidate sprite by 152/255 drops MAE to 0.00323 (five) and 0.00362 (eight), both
  below absent-actor, and sprite-pixel MAE to ~0.08. Reversing U instead makes both
  worse (0.00924 / 0.01078). The current UV orientation is right for these captures;
  the vertex modulation is the dominant missing term. The ~0.08 residual is unexplained
  (filter/blend state `0x1B`/0 at `0x414D80` is unread).

The captured 152 is **not** a production input. Accepting it would encode one level's
runtime value as a constant.

**Superseded 2026-10-19 by A4q** ([tint-writer note](phase-a4-actor-tint-writer.md)). The
152 is `MM_CLIMB_002`'s light-map palette entry 0 at the fixtures' cells (idx 4195/4196).
With the stored map and palette looked up and modulated, the gate passes. Facilitator
rerun on `live-pairs-4`: five 0.0032284 and eight 0.0036200, against absent-actor 0.0061900
/ 0.0068484. `--no-light-lookup` reproduces the old 0.0079708 / 0.0093424.

## Facilitator check (2026-10-19)

- Build up to date; **20/20 CTest** (19 prior + `actorsprite`).
- All nine A4d/A4e tube views re-rendered with the identical score **and** PNG SHA-256.
- Seven isolated source mutations of `ActorSprite.cpp`, each rebuilt into
  `test_actorsprite` alone and restored byte-exact (`facilitator-mutations.txt`): mirrored
  anchor as unmirrored, unmirrored `unknown1` scaled, no 0.0003 depth bias, no U
  reversal, alpha forced opaque, half scale as full, no 0.3 on Z — **all fail**.
  The 0.3-on-Z case is caught only by the synthetic-camera literal; the native corner
  checks use the semantic-matrix overload and bypass `actorProjectionInput`.
- Temporal/direction axes are indexed in `actorfixture.cpp`, which is not under CTest;
  they rest on the A3 tests. The brief's temporal pixel regression is moot while the
  primary gate fails.
- Install hashes equal `before.json`; no stock or `webolution` process running.
- Depth: the 0.0003 bias is READ and discriminated geometrically; no depth ordering
  against the tube is claimed.

## Next

One static question: who writes the actor vertex RGB after `0x440A10`, and from what
input (level light, actor state, global)? Then a pixel rerun with that rule.
