# Actor effect terrain segments

Authority: installed `Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.

| Routine | VA | PE file offset | Bytes | SHA-256 |
| --- | --- | --- | ---: | --- |
| Segment builder | `0x00459557` | `0x59557` (365911) | 998 | `fb8c4b16cb86d0cf1f14a8b550791262138a1d7bfeffe951bd79389cf706cd27` |

Full instruction bytes: [actor_shadow_segments.asm](actor_shadow_segments.asm).
REA evidence: `ev_2c95f562a5a2f88dba00f87b9d821cfa9508a33b958ad0844310668090d9fff6`.

## Static behavior

The routine snapshots effect position `+3B0`, truncates its Y before clamping
to grid bounds, then truncates again. It forms signed bounds `trunc(X)-1` and
`trunc(X)+1` with wrapping 32-bit arithmetic. The loop compares these bounds as
signed integers, wraps each visited column through `0x40DEC0`, then truncates
that wrapped value. Mask comparisons use the wrapped column, while endpoint
and center comparisons retain the unwrapped bounds/original X.

The selected point starts at `(left, clamped Y)`. For each visited column:

- If current cell bit one is set and the clamped row above lacks bit one,
  select left or right according to the column/original-X comparison and Y+1.
- If current/above both have bit one, preserve the previous selected pair,
  except at the unwrapped last-column equality, which selects `(right,Y)`.
- If current bit one is clear and the row below is empty, inspect two rows
  below. An empty cell selects `(wrapped column+1,Y)`; a nonempty cell selects
  the left/right endpoint at Y-1.
- Otherwise a nonempty row below selects Y and either the left endpoint,
  right endpoint or integer wrapped column, in that equality-test order.

The rows used for queries are clamped; the selected output Y+1/Y-1 is not
clamped again. The current/above predicates test bit one, whereas the two
lower predicates test the entire cell against zero. Bit `0x80` alone therefore
counts as nonempty in the lower predicates.

Every iteration resizes the caller's list, constructs `(selected X, selected
Y,0)`, obtains the mutable entry, and assigns it. The local write index starts
at zero without reading existing list length. When the wrapped column equals
truncated original X and the last unwrapped bound is less than the right
endpoint, an additional point `(selected X+1,selected Y,0)` is inserted in
the same resize/get/assign order. The x87 comparison also inserts for an
unordered right endpoint. Unordered original X selects the left side in the
left/right comparisons. A wrapped first bound greater than last skips all
list operations and preserves existing contents.

Integer conversion retains the original signed64 `FISTP` low-32-bit projection
and masked-invalid result. No endpoint seam repair, duplicate removal or
iteration limit is added. In particular a selected X can be exactly 256.

## Verification

Units pass on x64 O2 and x86 O0, covering empty terrain, insertion ordering,
nonzero lower-row masks, carried endpoints, lower terrain, signed-bound wrap,
the zero seam and an unordered right endpoint.

O0 and O2 native comparisons each pass **22,723 list/grid mutation traces**:
16,507 three-point results, 6,126 four-point results and 90 no-iteration cases
that retain five preexisting points. The unchanged 998-byte routine and
original vector assignment, coordinate, conversion, grid and array helpers
run natively. Only list resize and mutable-entry lookup are adapted.

There are 2,048 exhaustive cases crossing four current/above/below/deep masks
with eight callback mutation scripts, 675 numerical/seam cases, and 20,000
seeded mixed-terrain cases. Traces compare each list call and its preceding
point contents; final list values, grid contents and effect storage also
match. Callback scripts change the original effect position and mutate grid
cells during resize/lookup, proving the snapshot and access order. Finite
values compare bit for bit; NaNs compare by classification.

## Limits and composition

The caller supplies a positive grid with valid storage, masked FP exceptions,
valid nonthrowing list callbacks, and coordinates whose wrapping and signed
loop terminate. Counter overflow can create a nonterminating original loop;
the source does not invent a guard. Very large positive wrapped columns can
also take a long time in the original repeated-subtraction wrapper. Exceptional
CRT behavior, floating-point status flags and concurrent mutations are not
claimed.

`build_actor_shadow_segments(shadow.position_3b0,left,right,grid,list)` supplies
the implementation for `ActorShadowCallbacks::build_segments_459557`. The list
adapter preserves original allocation/access boundaries. Full allocation and
renderer behavior remain separate from this geometric recovery.

The [effect update oracle](actor_shadow.md) also executes the complete unchanged
`0x458186 -> 0x459557` chain for 12,000 additional updates at O0 and O2, comparing
it with the composed recovered implementations. List allocation/access,
state-membership and renderer resource callbacks remain explicit; the segment
construction itself runs concretely on both sides.
