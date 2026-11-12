# Falling actor landing decision

Reference executable SHA-256:
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
REA pseudocode evidence for `0x00416433`:
`ev_34a2e470c1ee596182b4c7d27791325753b1b2ea3382c17949c2042b0043f691`.
The exact local disassembly was checked for x87 unordered branches, integer
conversion order, bit-mask priority, call arguments and state-constructor IDs.

## Byte identity

| VA | File offset | Length | SHA-256 |
|---|---:|---:|---|
| `0x00416433` | 91187 | 706 | `d6342397236ba119cdff7c865fce1aefeb2f10fd27028ea4760b86d339d9945f` |
| `0x0040B3A0` | 45984 | 48 | `4d91ce609376caf9a4d1b52e8178680d6a4317cb979143c19204d8fd30d1fa5f` |
| `0x0040EC7E` | 60542 | 48 | `6eecedeb1303e6e4de0ba7303e979661fb59d1939bb6125cbef3627b5f8f4cc4` |
| `0x0042F8B9` | 194745 | 121 | `7cf669715e700dd603d30fa1310673c55a7e98f22a60cd96b0dcfe27a26e8fcd` |
| `0x0041BD60` | 114016 | 62 | `70f14575bf28f09be3fd872cd47207fcb38fd264370a6aec21ec71fd4e56ff18` |

The three constructor routines are static evidence, not reconstructed
constructors: they set state type IDs `0x1A`, `0x21` and `0x12`. The oracle
adapts their execution. These pins and the decision/selector pins are local to
`run_native_actor_landing_oracle.py`; coordinate/grid helper pins come from the
main verifier. All input ranges are mapped through the PE section table and
checked after verifying the whole executable. No original binary or assets are
stored in the repository.

## Recovered behavior

`resolve_actor_landing` reuses `actor_sample_position_360`, which adds the
actor's pair at `+0x360` to its XY position. The following conversion order is
significant:

1. Subtract signed direction `+0x44` from sampled Y in x87 precision.
2. Convert to signed64, retaining only the low signed32 bits.
3. Clamp that integer-valued Y to the world height, then convert again.
4. Convert sampled X to signed64 and retain low signed32 **before** cylinder
   wrapping, then convert the wrapped result again.
5. Read the collision cell using the recovered grid accessor.

For example, sampled X `-0.75` becomes zero before wrapping. This differs from
wrapping the fractional value first. Masked invalid x87 conversions return
signed64 indefinite (`INT64_MIN`), whose low32 bits are zero; the source
preserves that behavior for NaN, infinity and out-of-range conversions.

The collision lookup occurs before the actor's original Y bounds check.
Positive height and valid collision storage are therefore required even for
an actor that will immediately take the out-of-world transition.

Decision order:

| Condition, in priority order | Action | Result |
|---|---|---|
| Original actor Y is at/below zero, at/above height, or NaN | Create constructor `0x40B3A0` state and attach | true |
| Cell includes `0x1000` | Bounce notice with integer XY; create `0x40EC7E` state; attach | true |
| Cell is zero or includes `0x80` | No transition | false |
| Distance `+0x18` is greater than 30, umbrella byte `+0x49` is zero, and no category exemption applies | Create `0x40B3A0` state and attach | true |
| Remaining solid-cell cases | Call landed-state selector `0x42F8B9` | true |

Category 1 is exempt when the cell includes bit `0x200`; category 2 is exempt
when it includes bit `0x4000`. These are bit tests, unlike the complete-cell
comparisons in `actor_accepts_terrain_value`. A bounce cell takes priority over
the `0x80` flag. Nonzero umbrella bytes include values above 127.

The distance threshold is strictly greater than 30. NaN distance does not
satisfy that comparison and reaches the landed-state selector. NaN actor Y
instead takes the out-of-world branch because the original x87 test accepts
unordered status there; a literal translation of the decompiler's `<=` tests
would miss this behavior.

## Landed-state selector

The separately recovered `select_actor_landed_state` implements `0x0042F8B9`.
It always creates the state with constructor `0x0041BD60` and then attaches it.
The original requests 32 allocation bytes. The constructor establishes type ID
`0x12`, phase byte `+0x10 = 1`, extra-state byte `+0x11 = 0`, and flag `+0x18 = 0`.
Those constructor facts are static evidence only; the creation callback remains
responsible for the actual state object and its unrecovered behavior.

REA selector evidence:
`ev_ee0d640b115aa0a5dd8912fb0e8e44e86a88e22857116b619c7c98979a5d7a68`.
REA constructor evidence:
`ev_a4a5067a31e77e3cc806ac8053625b28b186f57a99cca66258e38e636b928263`.

## Verification

Portable unit tests pass on x64 and x86. They check boundary priority, exact
and next-representable distance thresholds, umbrella bytes, overlapping terrain
masks, category exemptions, NaN/infinite Y, NaN distance and truncation before
X wrapping using a nonuniform grid.

`native_actor_landing_oracle` passed **12,445 decision/trace comparisons** on
2026-10-29 UTC. It executes the complete 706-byte original landing routine
unchanged with original sample-position, conversion, clamp, wrap and collision
accessor helpers. The landed-state selector `0x0042F8B9` also executes unchanged,
eliminating its earlier adapter. The compared outputs are the full integer return value,
ordered transition/notice/selection trace, bounce integer coordinates and
preservation of actor bytes when external callbacks do not modify them.

The corpus includes all 32 combinations of five relevant terrain bits,
categories -1 through 3, umbrella values 0/1/128, five distance cases including
the threshold and NaN, Y boundary/special cases, and 10,000 randomized cases
with nonuniform cell grids, offsets and vertical directions. NaN and infinities
are also supplied as sampled X inputs; they are converted before wrapping.

Explicit adapters:

- `0x49C1A8`: records state creation and provides stable storage; checks the
  original allocation size is `0x18`, `0x30` or `0x20`.
- `0x40B3A0`, `0x40EC7E`, `0x41BD60`: check the allocation identity/size and return it.
- `0x42E696`: records attachment. This routine is independently recovered and
  tested elsewhere, but this oracle isolates the landing decision.
- `0x40F860`: records bounce XY and checks the original format-string pointer.

The source exposes creation, attachment, selection and bounce notice as typed
callbacks. Actual constructor effects, state lifecycle, log formatting and
full running-game effects remain outside this claim. Allocation failure and
unwinding through original MSVC exception frames are not tested.

```powershell
python tools/run_native_actor_landing_oracle.py `
  --exe 'C:\progs\Lemmings Revolution\Lemmings Revolution.exe' `
  --program build/x86/native_actor_landing_oracle.exe
```

```text
native actor landing passed 12445 decision/trace comparisons
```
