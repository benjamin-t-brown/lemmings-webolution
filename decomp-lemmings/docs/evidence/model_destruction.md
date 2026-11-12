# Inline actor model destruction

Authority: installed executable SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.

| Routine | VA / file offset | Bytes | SHA-256 |
| --- | --- | ---: | --- |
| Actor model destruction | `0x43CE88` / `0x3CE88` | 87 | `73e58fbb82c29c7895e026f9ee8aad7e654f3bc29131ee57e723e86268c2ba41` |
| Animation destruction | `0x46820D` / `0x6820D` | 72 | `6661abce883b6dcac51ead5b08874ff3d122b4cb1d8b5db60ee2fa3a167f744d` |
| Model base destruction | `0x4658B7` / `0x658B7` | 31 | `80e21f525f3b672e459c0312a260e3215576d838aeb474f304f07381369c8211` |
| Animation resource release | `0x468255` / `0x68255` | 81 | `d5b6f8f9aedd65687bdf9ef315a880060ac8ae7923b17df0257718fa925810c4` |
| Animation base destruction | `0x440BF0` / `0x40BF0` | 19 | `758ba2e9bd1e02647438bd10c15567ba9d6a86806423ceb4ffc77e9f637a941f` |
| Quad destruction | `0x440C10` / `0x40C10` | 32 | `201ae284b6133141ba31e708af8af89a3def5234e7ef0139c68d8f3feb5cf57d` |
| CRT vector destruction | `0x49C655` / `0x9C655` | 104 | `e7a5b601ea8f3f9000f5f6c522c22bfc271fe8c8645a1ea1a270ef4b5a7ae5f7` |
| CRT success cleanup funclet | `0x49C6BD` / `0x9C6BD` | 24 | `2c7fc6efbbb1db82eb61aeff1e738b410fed12f9b658526a8357e8fde26cce90` |
| Empty component destructor | `0x41A4B0` / `0x1A4B0` | 11 | `012a7b6ce927f083ba98a546e3d3f750b2a1d82f350ca27efbd30a9c3c7718a5` |

REA procedure/pseudocode evidence, corroborated by local disassembly:

- `43CE88`: `ev_df16c87ec9795694f391403641696c6e247941f9a2e985f69bc532b682819ad3`
- `46820D`: `ev_1f9ec9fba2a55c5c7bfb9befa303846242b5ac13f1c6f2d39d5c0eb8ece168b9`
- `4658B7`: `ev_d32a50f0c09017a5f604b2f251adefadd93daf2a91ec4b5f1490f525877b3086`
- `468255`: `ev_c8e26866e0f8886056b9e87f66e5c9bf284857d37ed37fee5b94a2ab2f24d7f9`
- `440BF0`: `ev_0cb5d8d0a43a0174b43b3dc62910b3b7f67862842e745536ffaaeb353765224d`
- `440C10`: `ev_e3cdf75f8295eaf229361d4219d37b455c83b52d18fce417a5b8f782209690bb`
- `49C655`: `ev_14541173f457f2eb8be64f14e7f3fa3872aae77c6fe188e6530908ae024e31ca`
- `49C6BD`: `ev_d29c07e925a0a0b37089b15c09ae57cb06d837cc63a065654f204e6bfbbc8879`
- `41A4B0`: `ev_337b5d320b111186940747ee0442cf3e472428b348a4fdd750342ac5ea9d66c8`

Original instruction bytes: [model_destruction.asm](model_destruction.asm).
Empty corner destructor `420DA0` is pinned in the model-construction evidence
and loaded unchanged by this oracle as well.

## Recovered behavior

`destroy_actor_model` (`43CE88`) sets vtable `4B14A8`, destroys the animation
object at model `+B0` through `46820D`, and then destroys the model base through
`4658B7`. Base destruction sets vtable `4B1858` and calls the empty destructor
`41A4B0` on component `+8`. It leaves that component's bytes untouched.

Animation destruction calls resource-release helper `468255`, followed by
base/quad destruction `440BF0`/`440C10`. The quad destructor invokes the original
CRT vector-destructor helper `49C655` for four corner records at `+8`, stride
`0x68`, in reverse order. Each corner destructor `420DA0` is empty. Normal CRT
completion sets its success flag and invokes funclet `49C6BD`, which returns
without further work. The recovered source therefore has no corresponding
memory writes for these empty destructors.

`release_animation_model_resource` (`468255`) preserves these branches:

| Initial resource ID | Initial pointer | Behavior |
| --- | --- | --- |
| `-1` | Any value | Return; preserve both fields |
| Other ID | Null | No release call; set ID to `-1`, pointer to null |
| Other ID | Nonnull | Call manager `430050(this=58D23C,id)`, then set ID to `-1`, pointer to null |

The reset happens **after** the release callback, overwriting changes to those
two fields made by the callback. Other callback changes persist. In the full
actor-model destructor, the final base-vtable write also overwrites any vtable
change made during resource release. This differs from resource binding, whose
existing API is not substituted for this distinct release path.

## Boundaries

The code reuses the typed constructor views and `AnimationResourceHooks`.
Resource-manager release is the sole unresolved effect boundary. The release
hook must be present on a path that calls it and must return normally. No
deallocation of the enclosing model object is performed here: these recovered
entry points are destructor bodies, not deleting-destructor wrappers.

The source claims normal-return destructor behavior. Exceptional SEH/CRT unwind,
resource-manager internals, and original game rendering are outside this scope.
Original reverse corner traversal is executed by the native oracle; the source
omits it because the concrete corner destructors have no observable effects.

## Tests and native comparison

Units pass on x64/x86 at O0/O2. They cover sentinel and nonsentinel resource IDs,
null/non-null pointers, callback arguments, post-callback resets, vtable order,
preserved component bytes, base-only destruction, and retained unrelated
callback mutations.

The runner verifies the complete executable identity and all ten loaded routine
spans before execution. Native code runs unchanged except resource-manager entry
`430050`, which is redirected to a controlled release adapter. The full nested
destructor chain and CRT reverse-array helper/success funclet execute original
bytes; original SEH setup/restoration executes without exceptional unwind.

**20,000 cases pass at both O0 and O2**: 5,000 direct calls each to `468255`,
`46820D`, `4658B7`, and `43CE88`. Each starts with a random `0x2C8`-byte model
buffer and exercises resource ID `-1`, zero, signed-int extrema, or random IDs;
pointer absence/presence; and four callback mutation scripts. They compare
resource-manager object/ID arguments, callback-visible ID/pointer/vtable/mode,
and every final model byte. Unwritten fields and padding retain their original
bits, while mutations outside the mandatory reset fields remain visible.

This is controlled original-byte teardown comparison with a synthetic resource
manager, not an observation of a full game's resource lifetime.

```powershell
python tools/run_native_model_destruction_oracle.py --exe 'C:\progs\Lemmings Revolution\Lemmings Revolution.exe' --program build/x86/native_model_destruction_oracle.exe
```
