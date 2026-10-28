# First-level picking: submission visibility and terrain support

Offline continuation of [overlap selection](first-level-overlap-selection.md).
No original-game start/attach, desktop input, or installed save/config writes.
The raw static reads and SOURCE tests below are not a new native comparison.

## Shared draw/pick visibility gate

`ActorVisibility` factors out the tube pass bounds already ported in
`TubeScene::render` (43A4EC..43A615). Bounds use the camera selector, the level's
stored cylinder extent, truncation toward zero, and shifts of32. There is no new
estimated camera-facing cone or screen-depth heuristic.

The raw actor branch43A94D..43AA47 establishes:

- Actor+18 (shell Z) **>=1**: actor+29 copies enabled byte+28, without angular
  rejection. The X bucket is calculated but does not gate drawing in this arm.
- Z<1: wrap raw actor X with40DEC0, truncate, divide by8 toward zero; when
  upper bound d>=32 and bucket<a, add32. Require **a<=bucket<=d** and actor+28.
- Both arms draw only if the resulting byte is nonzero. The hover loop later
  tests that same byte at455104.

This is **submission visibility**, not a proof that each sprite pixel passes a
terrain-depth test. In particular outer-shell actors retain the native bypass.
The native X helper snaps tiny magnitude to0, adds256 only once for negative X,
and repeatedly subtracts256 for nonnegative X>=256; tests preserve these quirks.

`prepareInjectedWalker` applies this gate before preparing the sprite. Both
`drawInjectedWalkers` and `injectedWalkerScreenBoxes` use it, so a culled actor
does not draw and cannot enter the production candidate list. Slot order is
unchanged: hidden slots are null, not erased and accidentally remapped to actors.
The tube route uses the same factored bounds calculation as the actors.

`InjectedNativeWalker::renderEnabled` is an explicit actor+28 input. It defaults
on for injected/source alive actors; this does **not** reconstruct all native
lifecycle writers. `composeSkipActorVisibility` is a diagnostic bypass used to
prove that an otherwise-projectable hidden actor would change the image and
be picked under the old behavior. It is false in normal application/CLI use;
no historical image fixtures were silently switched to the bypass.

## Bounded first-level Climber support

The ground predicate is42E913, reached through45A0E0 ->40C061 ->4183D3.
The previous omission was not filled with a generic guessed `canAssign()`.

New producer reads close the ordinary Walker coordinate chain:

- Spawn zeroes actor+360/+368. Facing-turn42F66B sets horizontal bias to-1
  for left and0 for right. 42E913 adds1 for left Walkers, cancelling that bias.
  **Do not probe source x+1 for a left-facing Walker.**
- Upright gravity has vertical bias0 and42E913 subtracts1 from native Y-up.
  Under the existing SOURCE `H-1-y` conversion this becomes source `y+1`.
- 40DF50/40DE60 clamp to actual terrain rows. Below-bottom SOURCE synthetic
  Steel is not the native support sample.
- Native support tests the whole word: nonzero, no0x80/no0x1000; exact0x200
  accepts sheet1, exact0x4000 accepts sheet2, exact0x2000 rejects; other words
  pass. The fluid comparisons are equality, **not** bit tests.

All24576 authored `MM_CLIMB_002` words are enumerated:0 (8973),1 (9339),3
(6224),0x82 (40). On exactly that alphabet the native predicate agrees with
the imported material solidity. Thus `firstLevelClimberSupport` is an explicitly
bounded DERIVED/SOURCE adapter for selected Climber + upright Walker:
`solid(x,clamp(y+1,0,H-1)) && !trampoline(x,clamp(y+1,0,H-1))`.
The represented trampoline marker preserves a native rejection but does not
make this a universal raw-word port.

The application enables this support rule only on its canonical
`MM_CLIMB_002` route, and feeds it into eligibility **before** ranking overlaps.
Other skill/state/gravity combinations return unknown rather than claiming
native equivalence. Existing fallback behavior remains for unknown cases.

The sim does not preserve complete raw terrain words. A general implementation
must retain equivalent information through terrain mutations; reconstructing
words by OR-ing marker booleans would lose exact-zone equality and omitted bits.
Native inverted actor anchoring is also unresolved: the native +368 bias is3,
so a literal existing pose conversion would yield y-4, not a guessed y-1.
Continuous pose, parallel-state/lifecycle completeness and facing filters remain
open. This is not full native eligibility acceptance.

## Evidence and checks

Re-run `python tools/harness/read_runtime_pick.py`; optionally supply
`--first-level build/lvl_dump/levels/COMPILED/MM_CLIMB_002.LVL` for the pinned
level alphabet and49152 upright coordinate/word cases. This reads only an
already-extracted file and never runs the target or extracts/writes grid files.
Authority SHA256 and raw-backed extents are checked. Raw instruction packets:
`build/tmp/enacter-pick-visibility/`, `build/tmp/enacter-pick-support/`.
The initial visibility dump started mid-instruction at43A8C0; the retained
`actor-gate-aligned.txt` starts at43A901 and is the decode used here.

Tests:

- `actorvisibility`: native interval endpoints, exact d32 seam, a32 retention,
  shell-Z bypass, explicit disabled byte, and native X wrapping quirks.
- `levelvisibility`: actual app, SDL dummy, retail first level prepared at20
  ticks, opposite yaw160. Uses the observed World to locate the hidden actor's
  diagnostic projected box. Production rendering with that actor is byte-equal
  to the actor-absent scene; diagnostic bypass produces different pixels. A click
  at the hidden projection does not assign, spend stock, or resume pause.
- `runtimepick`: closer unsupported Walker loses to farther supported Walker;
  left/right bias cancellation; below-feet versus feet sample; bottom clamp,
  Steel and trampoline cases; unsupported state/skill/gravity remain unknown.
- Existing actual-app overlap and assignment tests remain, as does actorcompose.

Isolated mutations fail: omit shared visibility gate (hidden actor is rendered
and assigned); ignore support during candidate construction; use the feet cell;
add an uncancelled left-facing X offset. Production hashes were restored against
`build/tmp/enacter-pick-visibility/pre-mutation.sha256` before final tests.

Restored full build/CTest: **61/61 PASS**,111.05seconds; `levelvisibility`
passed in1.65seconds (`full-suite.log` in the same packet). No new live-native
acceptance is implied. The source-only review's remaining test gaps are direct
application discrimination of unsupported terrain, full draw-path disabled-byte
coverage, and broader selector/extent sweeps; the supported helper cases and
actual hidden-actor click/render path are covered above.
