# Walking surface tracking and depth approach

Authority: installed executable SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
The native runner checks that identity, each code hash, and each constant before
loading original bytes into an owned process. No original assets are stored.

| Routine | VA | File offset | Bytes | SHA-256 |
|---|---|---|---|---|
| Surface tracking | `0x41C875` | `0x1C875` | 666 | `ee05836e53dc30772ff5830269c690acbf673158ca4493a914996d836bafa5c2` |
| Depth approach | `0x42F977` | `0x2F977` | 352 | `788c5b7df46a173591d45b52df0cd6041967fa65ab23ca21cd320e45d9755ae2` |

REA evidence:
`ev_711cdc5b41a17a20cf10a529d474e9ae82463cae97106b675b91b7aafb649ba1`
and `ev_97e8cea7d12cf99fc4bdc9dec9fbcc9138fb469dc51df1456eab524a0abca3a9`.
Local byte disassembly resolves x87 unordered branches and rounding boundaries.

## Static recovery

Surface processing samples position plus `pair_360`, clears byte `+4A`, moves
the sample one X unit according to whether direction `+40` equals one, subtracts
direction `+44` from Y, wraps X and clamps Y. The grid probes use truncated X
and Y, then truncated Y plus direction with 32-bit wrap. Surface bits are
`0x0200 | 0x2000 | 0x4000` across either cell. With neither cell tagged, only
current gadget `+54` is cleared; last gadget `+58` remains intact.

When tagged and no current gadget exists, the routine scans the global list in
order at the first point, then at Y plus direction. The first containment hit
sets both `+54` and `+58`. Existing gadgets receive up to two containment checks;
two misses clear only `+54`. The second containment point adds direction to
the already clamped floating-point Y and is not reclamped. Count and element
queries remain explicit callbacks, preserving their order and repeated count
reads. The opaque handles retain the original object identity.

Depth processing runs only when byte `+5E` is nonzero. Direction exactly minus
one selects global `4C5B30`; every other direction selects `4C5B20`. It approaches
that target by the exact binary64 constant at `4B10E0`
(`0x3F947AE147AE147B`, approximately 0.02), then clamps a crossing. Each arithmetic
result is stored to binary64 before the crossing comparison. An unordered first
comparison follows the addition path, including when the target is NaN.

## Tests and native comparison

The x64 unit test checks the second surface scan, low-byte truth, retention of
`+58` when clearing `+54`, direction target selection, crossing clamps, exact
constant cancellation and NaN-target addition.

The native oracle executes both recovered routines and original sample,
coordinate, grid, integer-conversion and ordinary absolute-value helpers
unchanged. It adapts only list count `41CB90`, list entry `41CB70` and gadget
containment `430BA1`. It compares 15,280 surface cases with full callback traces,
exact sample-point bits, gadget fields, cleared flag and absence of other writes.
The cases include every surface mask separately, no masks, unrelated bits,
existing and missing gadgets, first/second/no hits, empty lists, randomized
grids, both directions, fractional samples and coordinate wrapping/clamping.

Depth requires no callbacks or patched code. It matches in 10,616 cases with
exact output bits and absence of other writes, including both signed zeros,
NaN, infinities, overshoot, disabled state and randomized positions/targets.
Both unoptimized and optimized x86 runs pass. Surface native inputs have finite
X and Y; the CRT nonfinite-X absolute-value exception path is outside this
oracle's mapped helper set. These tests establish surface selection against
scripted containment answers, not gadget containment geometry itself.
