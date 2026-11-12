# Walking collision decision at 0x0041BFFC

## Authority and bytes

Installed executable SHA-256:
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
The sibling remake is not evidence for this recovery.

| Address | File offset | Bytes | SHA-256 |
| --- | ---: | ---: | --- |
| `0x0041BFFC` | 114684 | 2169 | `837e048169e5423ee649ff92a802ab611cc029b1802c81451360e9d3aadcb3fd` |

REA pseudocode evidence:
`ev_845c208db337203d9c23cf957c5624dec390273263fe8d01e8c0c10de2cb79ba`.
Original instruction inspection checked the full extent through `0x0041C874`,
including x87 condition flags, integer widths, gadget-field writes and direct
primary-state replacement. The native runner checks the executable identity,
PE address mapping, function hash, all executed helper hashes, and constants.
It does not persist original bytes or game assets.

## Recovered decisions

`resolve_actor_walking_collision` uses the shared logical `WalkingActor` view.
The original `this` state pointer is unused by this routine.

- Return 2 before any mutation when actor Y is nonpositive, unordered, or at
  least the grid height. Both original `TEST AH,0x41` comparisons admit NaN.
- Sample actor position plus `pair_360` and clear actor byte `+4A`. Truncate
  sample X through x87 signed64/low32 before wrapping; truncate sample Y minus
  direction `+44` before clamping to obtain the lower cell; separately truncate
  and clamp sample Y to obtain the current cell. Integer projection uses the
  original low-word semantics, not floor or saturating signed32 conversion.
- If the lower cell has `0x8000` and actor state `0x2B` is absent, visit the
  registered gadget list in order. Recompute the sample for each containment
  query and stop on the first nonzero result. **If no gadget contains the point,
  the last visited gadget is still tested.** Its enabled byte is `+38`, read by
  `0x0041CBA0`. The original dereferences null when this path has an empty list;
  valid nonempty lists are the recovered contract.
- For an enabled selected gadget, allocate/construct state `0x0041AD90` (size
  `0x20`) and attach it. Store the actor in gadget `+A4`, call gadget slot `+20`,
  then create falling state `0x00415FA0` (size `0x28`) and attach it. Return 1.
- If actor gadget `+54` is present, the only compatible category/mode pairs are
  actor category 1 with gadget `+128==0`, or category 2 with mode 2. Other pairs
  select the gadget's `+A8` animation name, clear extra states (`0x0042E78A`),
  create/attach `0x004041FD` (size `0x20`), and return 1. Compatible pairs set
  actor `+4A=1` and continue with terrain decisions.
- A cell carrying `0x800` reverses actor direction `+40`, unless category is 3.
  This check applies to the current cell and each of the two possible cells
  above it. The recovered direction reversal updates both associated pair
  components and preserves signed32 wrap, including `INT32_MIN`.
- If current-cell bit 1 is set, inspect one cell in direction `+44`, then a
  second if bit 1 remains set. An empty step calls `0x0042EE74` with the integer
  target Y. When all three cells carry bit 1, nonzero actor byte `+48` creates
  state `0x00413750` (size `0x20`), **writes primary `+340` directly**, and calls
  that state's enter slot `+8`; no attachment call occurs. Return 1. Without
  `+48`, reverse direction and return 0.
- If current-cell bit 1 is clear and the cached lower cell equals zero, inspect
  up to two cells opposite direction `+44`. If both are zero, create/attach the
  falling state and return 1. Otherwise, when actor `+4A` is zero, call the Y
  movement helper with the found cell Y plus direction. Nonzero `+4A` suppresses
  that movement. All other paths return 0.

The terrain comparisons intentionally distinguish exact zero, bit 1, `0x800`,
and `0x8000`. Signed32 additions/subtractions wrap before Y clamping. Cached
lower/current cell values and subsequent read order follow the original.

## Boundaries

Original sample, coordinate, grid, and reversal behavior is implemented directly
using the recovered helpers. Gadget-list operations, containment/enabled queries,
animation naming/selection, state allocation/construction/attachment/entry,
extra-state clearing, gadget activation, and Y movement remain explicit
callbacks in this slice. Field projection callbacks for gadget mode and actor
association represent direct object accesses, not additional original calls.
The no-op temporary-vector destruction does not need host state.

Allocation/constructor callbacks require successful valid objects. Allocation
failure, invalid gadget references, SEH unwind, and the internals of callback
systems are outside the comparison contract. Coordinates must lie in the
terminating domain of the original wrapper, and the grid must have positive
height with valid backing cells.

## Verification

**Static:** REA plus original bytes establish the routine and the distinctions
above; no live game session or sibling-source semantics were substituted.

**Unit tests:** `test_actor_walking_collision` passes on x64 and x86. Tests cover
bounds/NaN rejection without mutation, unsupported ground, supported/step-down
paths, blockers at each height, category-3 immunity, direct climbing-state
replacement before enter, last-gadget fallback after no containment matches,
gadget actor association before activation, double state attachment, and all
four tested category/mode values including suppression of step-down movement.

**Offline native comparison:** the oracle executes the unchanged 2169-byte
routine plus original geometry/grid/coordinate/conversion/reversal helpers.
Only declared boundaries are redirected. Both unoptimized and `-O2` comparisons
pass 25,000 cases. Results are 18,420 return-zero paths, 6,303 return-one paths,
and 277 return-two paths, with 6,833 state creations and 3,080 gadget-state
queries. The first 4,096 cases enumerate all `8^4` combinations of eight cell
values across four sampled terrain rows. Further cases vary geometry, signed
directions including integer limits, categories, flags, gadget modes and
matches, and NaN/infinite coordinates. Final geometry/state and callback order,
arguments, callback-time state, primary handles, and gadget association are
compared. This is a native test with adapters, not a complete game run.

```powershell
python tools/run_native_actor_walking_collision_oracle.py `
  --exe 'C:\progs\Lemmings Revolution\Lemmings Revolution.exe' `
  --program build/x86/native_actor_walking_collision_oracle.exe
```
