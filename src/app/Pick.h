#pragma once

#include <span>

namespace webo::app {

// A pure port of the original's in-level cursor-to-lemming pick rule.
//
// Read at `0x00454D33` (builds one candidate's box and dispatches to the
// overlap test) and `0x00454CB9` (the overlap test itself), reached from the
// per-frame hover scan at `0x00454D6C`-`0x00455236`. Every rule here cites
// its address and the instruction bytes read there in
// `build/tmp/d2a-pick-rule/reads.txt`; re-read those addresses before
// trusting a claim below rather than this comment.
//
// A box is in render-pixel screen coordinates, left<=right, top<=bottom.
// Which way y runs does not matter to this file: both boxes below come from
// the same accessor shape (`+0x20` left, `+0x28` top, `+0xF0` right, `+0x90`
// bottom, each a truncated double -- `0x00414770`/`0x00414790`/`0x0044D860`/
// `0x0044DFF0` through `0x0049C5AC`), so whatever convention the original
// uses, both sides of every compare use it the same way.
struct PickBox {
  int left = 0;
  int top = 0;
  int right = 0;
  int bottom = 0;
};

// Whether `box` (a lemming's own rectangle, `lemming+0x128`) is hit by
// `cursorBox` (the cursor sprite's rectangle, `[screen+0x3c]+0x10`), exactly
// as `0x00454CB9` computes it. The cursor box is shrunk 4 px in x and grown
// 4 px in y before the compare (the sprite itself is 32x32, set at
// `0x0045578C`), and all four edges are inclusive (<=, >=, read off the
// jg/jl/jg/jl branches at `0x00454CD6`/`0x00454CEC`/`0x00454D02`/
// `0x00454D18`): touching the adjusted edge counts as a hit.
bool pickBoxOverlaps(const PickBox& box, const PickBox& cursorBox);

// One lemming's candidacy for the hover slot `screen+0x16D8`.
//
// `box` is its own rectangle (`lemming+0x128`, confirmed this round to be an
// *embedded* sub-object rather than a pointer stored there -- the caller at
// `0x004550D9` computes `lemming+0x128` as a plain address and passes it
// straight through as the accessor's `this`, with no extra dereference).
//
// `eligible` folds the two purely lemming-side skip tests the original
// performs before it even reaches the box compare for a given candidate: the
// hatch-emergence byte at `lemming+0x29` (`0x00455104`, skip when zero) and
// the byte at `lemming+0x370` (`0x00455159`, via the one-line accessor at
// `0x00455F20`, skip when zero; its exact meaning beyond "the original tests
// it and skips on zero" is not established by this round's reads). `false`
// means the original would never select this lemming this frame regardless
// of its box.
//
// `priority` and `distanceSquared` are the two tie-break keys the original
// computes for every candidate that is eligible: `priority` from a per-state
// category read at `[state+0x14]` (`0x00430A70`) through an unenumerated
// table at `0x004C05D0`, and `distanceSquared` the squared distance between
// the lemming's own position (`lemming+8`, a 3-double vector) and a
// reference point at `screen+0x58` (also a 3-double vector) -- read at
// `0x004551AB`-`0x004551EB` as two calls to a vector-subtract helper
// (`0x00401830`) feeding a dot-product helper (`0x00418350`), i.e.
// `dot(a-b, a-b) = |a-b|^2`.
//
// This function does not compute either tie-break key itself: the state
// category table and what `screen+0x58` actually holds (camera position?
// the unprojected cursor ray? something else) are not settled from static
// bytes alone -- see `reads.txt` and `round-note.md`. The caller supplies
// both. Passing 0 for every candidate's `priority` and `distanceSquared`
// reproduces "first eligible, overlapping candidate in list order wins",
// which is only correct when every real candidate is genuinely tied on both
// keys (the original's own sentinel init -- priority `-1`, distance `1e8`,
// both at `0x00454D7D`/`0x00455069` -- gives the same answer for a single
// candidate, which is what every `test_pick` case except the tie-break one
// exercises).
struct PickCandidate {
  void* id = nullptr;  // identifies the lemming to the caller; opaque here
  PickBox box;
  bool eligible = true;
  int priority = 0;
  double distanceSquared = 0.0;
};

// Which candidate (if any) the original would write to `screen+0x16D8` this
// frame, given `cursorBox` and the per-lemming candidates in the same
// (actor-list) order the original iterates them. Returns `nullptr` for "no
// lemming", matching the clear-then-maybe-set shape of `0x00454D6C`'s
// prologue (`+0x16D8` is unconditionally zeroed at `0x00454DC2` before any
// candidate is considered).
//
// Decision, read from `0x004550A3`-`0x00455236`: skip ineligible candidates
// and candidates whose box does not overlap `cursorBox` (`pickBoxOverlaps`);
// among the rest, the highest `priority` wins outright over any
// lower-priority candidate seen so far (`jg` at `0x004551F4`, unconditional,
// no distance compare), and among candidates tied on `priority`, the
// smallest `distanceSquared` wins -- strictly smaller (`fcomp` + `test ah,1`
// at `0x004551F6`-`0x004551FE` reads the FPU's C0 flag, i.e. "is the new
// distance less than the stored one", not "less or equal"), so an exact tie
// on both keys keeps the earlier-encountered candidate rather than the
// later one. This is NOT "last overlapping lemming in list order wins" (the
// brief's own unverified hypothesis): that hypothesis is falsified by this
// read.
//
// Not modelled here, because they are not lemming-vs-cursor geometry: two
// UI-state facing filters (`screen+0x16b1`/`+0x16b2` against `lemming+0x40`,
// `0x0045511B`-`0x00455153`) and a skill-stock validity gate
// (`0x0045A0E0(0x004C91D8, lemming)`, `0x00455194`-`0x004551A5`) that the
// real routine also applies before writing `+0x16D8`. A live caller that
// needs a byte-exact `+0x16D8` in a scenario where those can differ (cursor
// over a facing-restricted lemming with no stock, for instance) must apply
// them itself; see `round-note.md`.
const PickCandidate* pickHoveredLemming(std::span<const PickCandidate> candidates,
                                         const PickBox& cursorBox);

}  // namespace webo::app
