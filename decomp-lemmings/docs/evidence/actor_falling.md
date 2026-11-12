# Falling state: entry, movement, cell timing, and update

## Identity and source ranges

Authority: installed `Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
The sibling remake was not used as evidence.

| Routine | Address | File offset | Bytes | SHA-256 |
| --- | --- | ---: | ---: | --- |
| Enter | `0x00416004` | 90116 | 52 | `635f366b99cdef3f5e27fc69b03543df8ffc87bca57633fdb55eadcab22c4797` |
| Distance | `0x00416038` | 90168 | 74 | `4e79ab47ece9ce6c65dad17bafa033ae914b7f96c7f1c025c1cf62e78558e316` |
| Update | `0x004160BD` | 90301 | 682 | `b69aeeb62541780fa45716d495a80e0dcfed6ec46671eb863d6e9189b2ac616f` |
| Time to cell | `0x00416367` | 90983 | 204 | `94365b118e003a5e9e84111596d2bddb91420f2ceb2f13de6e54331f94ddaf38` |

REA pseudocode evidence, respectively:

- `ev_fe0a5386b691f712625d1a5884ed26f85277eb0d48b1f77d85a22d7ce52a4491`
- `ev_f46e11b596e023657a862f8455c8e29a12840182fcef7944390180f7067af957`
- `ev_76478f46a3b6344e8e953ce53cf796bc64afea8a21977dcf751a32791a029de2`
- `ev_0e3df2207ba920cdbd031031242dfc8ac967f3709bcda057260d3e7f683533ce`

Local original-byte disassembly independently established the x87 comparisons,
conversion, intermediate stores, calling conventions, and embedded model layout.
`tools/run_native_actor_falling_oracle.py` pins every executed original range,
constant, and string, checks the full executable identity, and validates PE
section mappings before passing bytes to the native test process.

Additional data pins introduced by this slice:

| Address | File offset | Bytes | Content | SHA-256 |
| --- | ---: | ---: | --- | --- |
| `0x004B0A08` | 723464 | 24 | binary64 epsilon, 100000000, 20 | `1a8debfe4ec9f968546bfe4b71cd34ce2b8e7a62bf3194e968b52747f49dd77b` |
| `0x004BF354` | 783188 | 56 | `fall`, debug format, `float`, `umbopen`, padding | `5836a972ff3ba0e8bbbceeeb545e1deb36719b01051b1f00ad746422101039ad` |

The epsilon is binary64 bits `0x3E45798EE0000000`, approximately
`9.99999993922529e-9`, rather than an independently rounded decimal `1e-8`.
The runner also pins the existing zero/one/two/ten constants, `0x0049C5AC`
conversion helper, and original Y clamp plus its height accessor.

## Recovered behavior

The source uses logical `ActorFallingState` and `FallingActor` views. Position
and direction fields reuse `ActorGeometryState`; field suffixes document the
original offsets. The model at actor `+78` is an **inline model object**, whose
vtable pointer is stored at that offset. The portable model handle represents
that object, rather than claiming the original field is a model pointer.

### Entry and distance

Entry queries actor state `0x2B`. If the low return byte is zero, it selects
animation `fall`. It then clears actor byte `+4B`, even if the query returned
nonzero. Entry does not reset the falling state's distance or umbrella flags.

Distance is elapsed multiplied by speed 20 when actor `+4B` is zero, otherwise
10. The helper returns on the x87 stack; the update explicitly stores this
result to binary64 before using it.

### Time to cell

The conversion helper truncates Y to signed 64-bit using x87 `FISTP qword`.
Only its low 32 bits are then interpreted as a signed integer. This is neither
`floor(Y)` nor a direct saturating conversion to signed 32-bit. Invalid masked
conversions yield `INT64_MIN`, whose low 32 bits are zero.

For direction `+44==1`, time is `(Y - projected_integer) / speed`. All other
direction values use `((projected_integer + 1) - Y) / speed`, where the integer
increment wraps at 32 bits. Both paths store the quotient to binary64.

When state byte `+21` is nonzero and the quotient is zero **or unordered**, it
is replaced by binary64 `1/speed`. The unordered behavior comes from
`FCOMP; TEST AH,0x40` at `0x00416410..0x0041641B`.

### Substep update

While remaining time is ordered-positive and no transition has been returned:

1. Compute time to the next cell. Umbrella-completion time defaults to
   100000000. If actor `+49` and state `+20` are nonzero and actor `+4B` is
   zero, query model duration (slot `+4C`), store it to binary64, query current
   model time (slot `+48`, still extended), and compute duration minus current
   time plus epsilon. Store binary64 and issue the original diagnostic callback.
2. Prefer a cell step when cell time is less than umbrella time and no greater
   than remaining time. Both tests also admit unordered. Otherwise use umbrella
   time when it is no greater than remaining time, again admitting unordered;
   otherwise consume remaining time. An exact cell/umbrella tie selects umbrella
   completion, not the cell event.
3. Compute distance, then target Y as current Y minus signed direction times
   distance, using extended intermediates and a binary64 argument store. Query
   depth/height `0x0043DA38(position, targetY)` and store position component `+18`.
   Negative **or unordered** results become positive zero. Recompute target Y
   after the callback and apply recovered `clamp_cylinder_y` to update `+10`.
4. Advance the inline model by the chosen step, then subtract the step from
   remaining time. If this was umbrella completion, select `float` before
   setting actor `+4B=1`.
5. On a cell step, if accumulated distance is ordered-at-least 10, actor `+49`
   is nonzero, and state `+20` is zero, select `umbopen` before setting `+20=1`.
   Then increment distance by one, invoke landing `0x00416433`, and set state
   `+21=1`. On other steps, clear `+21`.

A nonzero landing result is returned with remaining time preserved. A zero
result clears remaining time to positive zero, including initially negative,
zero, or NaN input time. No synthetic iteration cap is imposed on the recovered
loop; its contract requires inputs/callbacks for which the original terminates.

## Boundaries and verification

The state query, animation selection, model virtual methods, diagnostic output,
depth query, and landing check are explicit callbacks. This slice does not claim
to implement their internals. The depth callback can bridge the independently
recovered `world_depth_at_y`; the landing callback can bridge
`resolve_actor_landing`. The existing Y clamp is called directly.

**Static:** REA and original instruction inspection establish the source paths
above, including unordered branches that decompiler comparison syntax omits.

**Unit tests:** x64 and x86 `test_actor_falling` cover entry order, raw flag
values, both speeds/directions, negative Y truncation, low-word conversion and
integer wrap, zero/NaN boundary retry, partial steps, exact umbrella/cell tie,
opening threshold, completion timing epsilon, height clamping, recomputation
after a height callback modifies Y, repeated cell substeps, and preserved time
on transition. Unknown callback internals are not asserted.

**Offline native comparison:** the 32-bit oracle executes all four original
routines unchanged, together with the original conversion helper and Y clamp.
Only declared external call boundaries are redirected. Both unoptimized and
`-O2` builds compare 20,000 update calls, 40,000 helper calls, and 5,000 entries;
the updates traverse 39,496 substeps. The tests compare helper x87 values using
their ten meaningful bytes, callback order/arguments, callback-time state and
remaining time, final binary64 state, and transition result. Cases include
finite and arbitrary floating point bit patterns, infinities, subnormals,
quiet/signaling NaNs, raw non-boolean flags, unusual signed directions,
extended model time values, height-callback mutation, ties, and multi-step
loops terminated by the landing adapter.

This is native comparison with adapters, not a live full-game test. Successful
object access, a valid grid, masked floating point exceptions, and terminating
input/callback combinations are required. Original crash, allocation, animation,
rendering, and sound behavior is outside this recovered slice.

```powershell
python tools/run_native_actor_falling_oracle.py `
  --exe 'C:\progs\Lemmings Revolution\Lemmings Revolution.exe' `
  --program build/x86/native_actor_falling_oracle.exe
```
