# D2a: the original's in-level pick rule, read and ported

Queue id `d2a-pick-rule`. Round 1 of 3. Offline disassembly read plus unit tests; no live
game, no stock start, no debugger. Full worker evidence:
`build/tmp/d2a-pick-rule/{prediction.md,reads.txt,round-note.md}`.

## Question

Exactly which lemming does the original choose when the cursor box overlaps zero, one or
several lemming boxes — comparison operators, cursor box origin/extent, iteration order
and tie-break, and which lemmings are skipped.

## Answer

`src/app/Pick.h`/`Pick.cpp` ports `pickBoxOverlaps` (`0x00454CB9`) and
`pickHoveredLemming` (the accept/skip/tie-break body of `0x00454D6C`, `0x004550A3`-
`0x00455236`).

- **Box test** (`0x00454CB9`): all four edges inclusive —
  `lemming.left <= cursor.right-4`, `lemming.right >= cursor.left+4`,
  `lemming.top <= cursor.bottom+4`, `lemming.bottom >= cursor.top-4`. Each box field is a
  double truncated to int32 by the standard MSVC round-toward-zero idiom at `0x0049C5AC`.
  `lemming+0x128` is the box's own base (not a pointer stored there); the cursor sprite
  box is a plain 32x32 at `screen+0x70`.
- **Tie-break, falsifying the brief's own "last-in-list-wins" hypothesis**: priority
  (desc, from a per-state category read at `state+0x14` through a `.data` table at
  `0x004C05D0`) wins outright; only an exact tie on priority falls through to squared
  distance (asc, strict `<`) between the lemming's own position (`lemming+8`) and a
  reference vector at `screen+0x58`; an exact tie on *both* keeps the earlier-encountered
  candidate. List order is otherwise irrelevant.
- **Skip gates modelled**: not yet "emerged" (`lemming+0x29`), an eligibility byte
  (`lemming+0x370`), and the box test itself.
- **Skip gates read but deliberately not modelled**, with reasons: a facing-filter UI pair
  (`screen+0x16b1`/`+0x16b2` against `lemming+0x40`) and the skill-stock validity gate
  (`0x0045A0E0`) — both depend on runtime UI/skill-panel state, not lemming-vs-cursor
  geometry, which is `Pick.h`'s whole scope.
- **Not modelled, out of D2(a)'s scope**: two upstream widget-hit-test loops
  (`0x4C88F8`, `0x4C8908`) that run before the lemming loop and can suppress the pick
  entirely if some UI panel's box is hit first. Flagged for the live D2(a) capture: keep
  the 20 recorded cursor positions clear of any panel/UI widget box.
- **Genuine gaps, reported not guessed**: the priority-category table's contents
  (`0x004C05D0`) and what `screen+0x58` holds (camera position? cursor ray? unestablished
  by static reads alone).

## Tests

`test_pick.cpp` (CTest `pick`): one hit; one-pixel miss on each of the four edges (each
preceded by the exact-boundary hit, confirming inclusivity); closer/farther distance
tie-break; exact tie (keeps the earlier candidate); priority beats distance; a skipped
(ineligible) candidate; an all-ineligible list (picks nothing).

## Facilitator check (2026-10-21)

Independently re-read `0x00454CB9` (the whole box-overlap function) and
`0x004551E4`-`0x00455203` (the priority/distance tie-break) directly against the installed
exe — byte-for-byte match to `reads.txt` in both cases. Independently confirmed the
`kTailAt`/offset reasoning is unrelated to this round (that is D3a's hatch words); Pick.cpp
needed no address re-derivation beyond the two spans above since every other cited address
in `reads.txt` (the four field accessors, the truncation helper, the state-category and
eligibility-byte accessors, the distance vector math) is a short, self-contained
one-line-shape function whose bytes were spot-checked during the mutate-one-alone pass
below rather than independently transcribed in full.

Mutate-one-alone: changed `pickBoxOverlaps`'s left-edge comparison from `<=` to `<`
(inclusive to strict), rebuilt, ran `test_pick` — caught exactly as expected
(`FAIL: left edge: cursor.right-4 == lemming.left is still a hit`). Restored to the
worker-delivered SHA-256, rebuilt, full suite **26/26** (including the new `pick` test and
the concurrently-landed `retaillevel` test). No Lemmings Revolution process was started by
this round or by this check.

**Landed.** D2(a)'s live cursor-capture recheck (D2b) is a separate, later round; this
round does not claim the live gate passes.
