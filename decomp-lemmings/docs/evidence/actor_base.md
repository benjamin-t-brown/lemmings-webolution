# Base actor construction

Authority: installed executable SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
REA evidence `ev_823dde9bf186e144fa7f1574cbfc19b5c3b10501a1b72e5aa20a5fbade6c889b`
supplies the full procedure information and pseudocode; local disassembly
corroborates the writes and copy order.

| Routine | VA / file offset | Bytes | SHA-256 |
| --- | --- | ---: | --- |
| Base constructor | `0x4230A2` / `0x230A2` | 202 | `fa818c0a9e84b04f99175d5536e13e8f39096f386c873205b141680b840857d2` |

Original bytes: [actor_base.asm](actor_base.asm). Existing helper pins used by
the native oracle are `4066A0` (34 bytes, zero triple construction) and `4017F0`
(60 bytes, triple construction by DWORD copies).

## Static behavior

`construct_actor_base` recovers the normal-return path of `4230A2`:

1. Construct zero position at `+8` through `4066A0`/`4017F0`.
2. Write vtable `4B0E90`, byte `+38=0`, and byte `+28=1`.
3. Copy six DWORDs from the supplied position to `+8` using `REP MOVSD`.
4. Write positive-zero doubles at `+30` and then `+20`.
5. Copy global 16-bit ID `4C55EC` to actor `+3A`, then increment the global with
   16-bit wrap.
6. Pass a pointer to a stack-local actor pointer to registry `4233A0`, with
   original registry object `4CBB18`. Return the original actor pointer.

The copy preserves position bits, including signaling NaNs: no floating point
load/arithmetic participates in the original constructor. If its position input
is exactly the actor's own position, the initial zero construction makes the
copied input zero. The source uses a bit-preserving copy and supports this alias
case explicitly. Partial overlapping raw input spans are outside its typed
array contract; the original forward `REP MOVSD` could behave differently there.

The source registry hook receives a reference to the local actor pointer. It can
mutate actor fields or the external ID counter; those changes persist. Replacing
that local pointer does not change the constructor's return value. No claim is
made here that the original registry routine performs those mutations; they are
controlled tests of the constructor boundary.

## Projection and boundaries

`ActorBaseState` names only known offsets. Fields `+20` and `+30` retain offset
names because their complete meanings are unknown. Object padding and other
fields are not invented or reset. The constructor's base vtable identity is a
logical value, not a host virtual-function pointer.

The registry insertion implementation is an explicit callback. The actor
constructor at `42DE51` can call this base implementation from its existing base
callback, while retaining any base fields in caller-owned context. No ABI layout
compatibility between the independent typed views is implied. Default C++ member
values are not evidence of original initialization.

The callback must return normally. Original SEH cleanup is not reconstructed;
the native oracle executes the constructor's SEH frame setup/restoration but
does not exercise exceptional unwind. The ID counter represents a separate
global, and inputs must satisfy the distinct-array or exact-self-alias contract.

## Tests and native evidence

Units pass at O0 and O2 on x64 and x86. They check positive zero writes, signed
zero/infinity/signaling-NaN position copies, ID boundaries `0`, `32767`, `65535`,
the incremented counter visible to registration, exact self-aliasing, registry
mutation of fields/counter, and replacement of the local registry argument.

The native runner validates the full executable SHA-256 and every loaded
routine pin. It executes unchanged `4230A2`, `4066A0`, and `4017F0`, adapting only
registry entry `4233A0`. **20,000 cases pass at both O0 and O2.** Cases combine
random position bit patterns, dirty actor storage, all eight registry mutation
scripts, 16-bit counter boundaries, random IDs and exact self-aliasing in half
the trials. They compare callback snapshots, registry object/argument identity,
counter values, constructor return identity and the entire `0x40` actor buffer.
Padding remains identical to the initial dirty bytes.

This verifies original-byte constructor behavior with a synthetic registry,
not a complete original-game spawn. Floating point status flags, exception
unwind and arbitrary partial raw-buffer overlap are not claimed.

```powershell
python tools/run_native_actor_base_oracle.py --exe 'C:\progs\Lemmings Revolution\Lemmings Revolution.exe' --program build/x86/native_actor_base_oracle.exe
```
