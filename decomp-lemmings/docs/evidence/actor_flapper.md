# Flapper behavior

## Identity and recovered routines

Reference executable SHA-256:
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
The original executable remains outside the repository. [The byte ledger](actor_flapper.asm)
records original instructions, PE offsets and SHA-256 for the four recovered
routines and concrete native helper chain.

| Entry | Bytes | PE offset | Role |
|---|---:|---:|---|
| `0x416910` | 48 | 92432 | Constructor, state type `0x20` |
| `0x41695C` | 66 | 92508 | Entry |
| `0x41699E` | 531 | 92574 | Update |
| `0x401320` | 13 | 4896 | Leave, no side effects |

Vtable `0x4B0A58` contains `0x416BC0`, `0x41699E`, `0x41695C`,
`0x401320` in its first four slots. The destructor is not recovered here.
The name “Flapper” comes from the original diagnostic strings, not an inferred
visual effect. REA evidence IDs:

- Constructor: `ev_791a38be091996ffb013eebb6bbba9ff0056b32283d3d2109f5024828169105b`.
- Entry: `ev_6afa8e932473573b4d652c044bbfe2a0345f5cd84a22164b3f1bb4dc606e1258`.
- Update: `ev_7cb177e32e541ec623965255029f877ccd7ff2bd38cf84a2246ff191159c927f`.

## Supported behavior

Construction calls base initializer `0x418390`, zeroing elapsed at `+8` and
phase byte `+0x10`; it then installs the flapper vtable, sets type `+0x14`
to `0x20` and clears extra flag `+0x11`. Other state bytes are preserved.
The public initializer establishes the known state values and leaves host
operations/context as explicit caller bindings.

Entry reads the signed 16-bit actor identifier at `+0x3A` through concrete
`0x406A00`, logs it with global `0x4CBF88`, then tests the current category
at `+0x4C`. Category 3 skips animation selection; others select `explode`.
The diagnostic callback can change category before that test.

Update preserves this order:

1. Advance inline model `+0x78`, slot `+0x2C`, using the supplied time budget.
2. Add the **current** budget to state elapsed and store binary64. Changes made
   by model advancement are observed.
3. Sample actor position through `0x42F22E`, including the `+0x360` pair.
   Truncate X with the original signed64 `FISTP` and low-word projection,
   wrap that integer through `0x40DEC0`, then truncate again. Subtract current
   direction `+0x44` from sampled Y in extended precision, truncate, clamp
   through `0x40DF50`, then truncate again.
4. Read the concrete grid cell. **Exactly zero** permits state-0 lookup.
   Any nonzero cell, including `0x80`, skips it. If state 0 is absent,
   calculate `0x416038` fall distance: speed 20 normally, 10 when actor
   `+0x4B` is nonzero. Multiply by direction, store binary64, then subtract
   that rounded distance from raw actor Y and store binary64. No height
   correction or actor movement callback runs here.
5. Clear the budget to positive zero, then call model slot `+0x44`.
   A zero low byte returns 0; callback changes to the budget remain visible.
6. Otherwise allocate/construct state `0x4081B0` (size `0x18`); allocation
   failure produces null. Query model slot `+0x48`, store its extended result
   to the time budget, and log the signed identifier, current global context
   and this budget.
7. Attach the new state through `0x42E696`, including null. Reload raw actor
   Y and direction after attachment; subtract one direction unit and store
   binary64. Return 1.

The low byte, rather than equality with 1, controls state/model query results.
Callbacks can change live state, actor, time and global context. There is no
catch-up loop or early guard for zero/negative time. Leave is a no-op.

## Verification

- **Static:** REA pseudocode cross-checked against original x86 instructions.
  The complete executable and individual spans/constants are pinned by
  `run_native_actor_flapper_oracle.py`. The vtable identity and slot mapping
  are included in its verified constants.
- **Unit:** `test_actor_flapper` passed x64 `-O2` and x86 `-O0`. It checks
  constructor field writes and preserved caller bindings; entry logging and
  category mutation; ordinary falling; any-nonzero terrain blocking; low-byte
  completion; null construction; attachment mutations; and empty leave.
- **Native:** `native_actor_flapper_oracle` passed **16,000 updates at each of
  x86 `-O0` and `-O2`**, each accompanied by entry, leave and constructor
  comparisons. All 128 combinations of callback mutation flags are exercised.
  Cases include directions -2 through 2; zero/nonzero floating flags; nonzero
  query results `0x80`/`0xFF`; factory failure; negative, zero and positive
  budgets; NaN/Inf budgets, coordinates and model results; signed zero;
  wrapped low-word conversion around `2^32`; and invalid signed64 conversion
  at `2^63`. Extended model results are rounded at the original store point.

Native comparison executes unchanged `0x416910`, `0x41695C`, `0x41699E`,
`0x401320` and the concrete coordinate/grid/fall-speed/identifier helpers.
Model virtual slots, animation selection, diagnostic output, state lookup,
allocation plus next-state construction, and attachment are explicit adapters.
The factory adapter combines allocation and construction into the public
callback boundary; the original success/null branches still execute.
At every callback the trace captures elapsed, remaining time, position,
direction, floating flag, identifier, context, category and call arguments.
Final raw state and actor bytes, return values and time/context are compared.
Constructor comparison checks its complete `0x18`-byte state. Arithmetic NaNs
are compared by classification; untouched raw bytes remain exact.

## Limits

This implements the flapper state's decisions and numeric movement. Model
playback, diagnostics, state lookup, next-state construction and attachment
remain explicit boundaries. The oracle uses synthetic actor/grid inputs,
not a playable game. It does not validate FP status flags, NaN payload/sign
selection, callback exceptions, compiler SEH unwinding, or aliases between
the independent state/actor/time objects. The original masked FP-exception
mode and valid grid storage are required.

```powershell
python tools/run_native_actor_flapper_oracle.py `
  --exe 'C:\progs\Lemmings Revolution\Lemmings Revolution.exe' `
  --program build/x86/native_actor_flapper_oracle.exe
```
