# Actor and base destruction orchestration

Authority: installed executable SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.

| Routine | VA / file offset | Bytes | SHA-256 |
| --- | --- | ---: | --- |
| Actor destruction | `0x42E17C` / `0x2E17C` | 216 | `4b4d4cfd33548d7fd76b2bbdf9003672f9337a8965f62629706cac012ccf06ab` |
| Base destruction | `0x42316C` / `0x2316C` | 114 | `0b156b4be9e5b5c74bea7f154e51cf248f96ab1cbfc8d277046bf76b384cdade` |
| Behavior cleanup wrapper | `0x42E70A` / `0x2E70A` | 27 | `93fd8f8d455318447912a3fef1938aa1ab1fef475419d8b8c19f47a5c2cc2c0c` |

Full original instruction bytes: [actor_destruction.asm](actor_destruction.asm).
REA procedure information and pseudocode:

- `42E17C`: `ev_f599ec05cb529f22c794ef96b68cfc0e8f378cdbbdcd8302da68f13fd9295319`
- `42316C`: `ev_13eab67f4a9790283f39f8fef5b3db604fb4682a28411622f0774208530b7de9`
- `42E70A`: `ev_c2bed638d980e0444d2275ac69e9404fd0608249be30e980cf33b3ae39664e3b`

## Static behavior

Actor destructor `42E17C` writes actor vtable `4B10E8`, then calls deleting
shadow destructor `42FEC0(shadow,1)` when pointer `+344` is nonnull. It does not
clear that pointer. Next, `42E70A` invokes extra-state cleanup `42E78A` and then
primary cleanup `42E725`. The recovered implementation composes those existing
concrete helpers, preserving their callback-sensitive traversal and rereads:

- Extra cleanup leaves every extra first, then destroys/releases nodes in a
  second pass and resets the sentinels.
- Primary cleanup leaves the initial primary, destroys the current primary
  if still nonnull, then clears it. Leave can replace or clear primary.
- Primary leave can add extras after the first extra cleanup has completed;
  they are still visible to the subsequent list destructor boundary.

After cleanup, `42E17C` reads the **current** actor category `+4C` to decrement
its 16-bit global counter at `4CBA78`, with wrap. It then calls, in order:

1. UI/world refresh `444ABC` with this pointer `4C6A08`.
2. Extra-list destructor `430110` on actor `+374`.
3. Inline-model destructor `43CE88` on actor `+78`.
4. Base destructor `42316C` on the actor.

Base destruction writes vtable `4B0E90`, sign-extends current actor ID `+3A`,
and obtains that registry slot through `423380` on global `4CBB18`. This helper
loads backing pointer `registry+0C` and indexes it by signed ID. The slot is
zeroed before world unregister `43B536(world=4C6A10,actor)`. The final position
destructor `403EB0` is empty. Actor ID itself is not cleared.

`ActorIdRegistry` supplies a slot span and the signed ID corresponding to its
first entry. This makes the valid backing range explicit without inventing a
registry allocation policy. Every ID used must address a valid supplied slot;
the original does not check bounds. The actor's ID is read after model teardown,
so callback changes to it affect which slot is cleared.

## Boundaries and contracts

`ActorDestructionState` references the existing `ActorBehaviorUpdate` so ID,
primary, and extra state changes remain visible across callbacks. Vtable,
category and shadow are projected known fields; other original bytes are not
reset or assigned invented meanings.

Shadow deletion, refresh, extra-list destruction, model destruction and world
unregister are explicit callbacks. State leave/deleting-destructor callbacks and
node release retain the existing cleanup APIs. Neither actor allocation release
nor a deleting-destructor wrapper is claimed: `42E17C` is the destructor body.

These routines recover normal-return paths. Original SEH cleanup/unwind is not
reconstructed; callbacks must return normally. Primary must be valid when its
leave begins, even if the caller initially has no extras. Callbacks must preserve
the original traversal and lifetime preconditions. Category and current signed
ID must address valid supplied storage at their respective reads. Concurrent
unsynchronized mutation between ordinary instructions is outside the contract.

## Unit and native evidence

Units pass on x64/x86 at O0/O2. They check exact subsystem order, extra-before-
primary cleanup, delete flag 1, retained shadow pointer, 16-bit counter underflow,
cleanup changing category, base vtable identity, signed ID extrema, registry
clearing before unregister, and base-only destruction preserving behavior state.

The native oracle loads unchanged `42E17C`, `42316C`, `42E70A`, `42E78A`,
`42E725`, `4302B0`, `430170`, `430190`, `423380`, and `403EB0`. The executable
and every code span are SHA-256 checked before execution. Adapters replace
shadow/UI/list/model/world effects, state virtual leave/destroy methods, and
node release `42FF50`; the cleanup traversal itself runs original bytes.

**8,000 complete actor destructor calls and 2,000 base-only calls pass at both
O0 and O2.** Synthetic cases cover 0..16 extras, absent/present shadows, counter
underflow and boundaries, negative IDs and both signed-16 extrema, dirty actor
padding, and mutations at subsystem or behavior callbacks. Cleanup scripts
include clearing node behavior pointers, prepending extras, changing traversal,
primary replacement/nulling, and adding extras during primary leave. Category,
ID, counts and vtable can change at callbacks. Each run compares:

- Ordered effect arguments and intermediate actor/count/registry snapshots.
- Behavior leave/destroy/release traces, links, primary identity and type IDs.
- Final registry/counter state and every byte of the synthetic `0x3A8` actor
  buffer, including untouched bytes outside projected fields.

This establishes controlled original-byte teardown equivalence with synthetic
effects. It does not claim a complete original-game actor lifetime or exception
unwind equivalence.

```powershell
python tools/run_native_actor_destruction_oracle.py --exe 'C:\progs\Lemmings Revolution\Lemmings Revolution.exe' --program build/x86/native_actor_destruction_oracle.exe
```
