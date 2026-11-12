# Actor construction orchestration

Authority: installed `Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
REA static evidence:
`ev_3a211ca1e2c46f9e9fc412ee932431d5c3c984cf364c108d5b1af9eaf7062e8a`.
The saved procedure and pseudocode are corroborated by local disassembly.

| Routine | VA / file offset | Bytes | SHA-256 |
| --- | --- | ---: | --- |
| Actor constructor | `0x42DE51` / `0x2DE51` | 811 | `15040d265ccb2da2faf319c56a20ef30547196c03d14c4d95c38169000fd5cfc` |
| Empty component constructor | `0x430A90` / `0x30A90` | 14 | `befca35709395b425403c72a706d2f3c715162ad22664d93df30e0255a4d4e8c` |

Full original instruction bytes: [actor_construction.asm](actor_construction.asm).
The other unchanged native helpers are the previously pinned triple constructor
`4017F0`, triple copy `4066D0`/`406700`, empty `401B80`, empty destructor `403EB0`,
and empty pair constructor `4821D0`.

## Static behavior

`construct_actor` recovers normal-return orchestration of `42DE51`, with typed
known fields and explicit callbacks for nested subsystems. Its input doubles
become a temporary `(x,y,0.5)` passed to base constructor `4230A2`. Inline model
`+78` and extra-list `+374` construction follow. Pair constructors at `+68`,
`+350`, `+360`, `+390`, and component constructor at `+348` do not write storage.
The actor vtable becomes `4B10E8` before the first allocation.

The remaining ordered effects and writes are:

1. Allocate `0x3D0`; construct `457F40` if nonnull; store its returned pointer at
   `+344`. Clear byte `+38C`, then bind the current global resource ID indexed by
   the input category to model resource `+128` through `46836D`.
2. Clear bytes `+60,+5C,+5F,+4A`; set `+5E=1`; clear `+5D`. Zero pair `+68` and
   set both directions `+40,+44` to 1. Increment the category's 16-bit counter
   at `4CBA78`, with wrap, then call refresh `444ABC` on global `4C6A08`.
3. Allocate `0x28`; construct monitor `4151F0` if nonnull; attach through
   `42E696`. This callback sees the old category and primary fields unless
   an earlier subsystem changed them.
4. Write category `+4C`, clear gadget pointer `+54`, set byte `+370=1`, and clear
   primary pointer `+340`. Allocate `0x28`; construct falling state `415FA0`
   if nonnull; attach through `42E696`.
5. Clear climb/umbrella bytes `+48,+49`; write pair `+350=(0,3)` and pair
   `+360=(0,0)`. Model virtual slot `+8` receives a pointer to the actor's current
   position. Register the actor through `43B517` on global `4C6A10`, then write
   mode `+50=1` and return the actor.

Callbacks may change fields and backing global values. The resource ID is read
at binding time, the counter at increment time, and current actor position is
passed by reference at model synchronization. The input category remains the
counter/resource index even if callbacks mutate actor category. Constructor
return pointers, which need not equal allocation pointers, are preserved.

## Boundaries

Base construction, inline model/list construction, allocation, shadow/monitor/
falling construction, resource binding, refresh, state attachment, model virtual
dispatch, and world registration are callbacks. The recovered monitor initializer,
falling behavior, attach logic and shadow projection can be composed by callers;
this routine does not silently substitute incomplete versions of those effects.
The callback for model synchronization must dispatch the current model's slot.

`ActorConstructionState` is a portable field projection, not an ABI-compatible
original object. Unknown component `+348` and pair `+390` are explicit preserved
storage. Default values in the C++ projection are caller conveniences, not claims
about initialization of original memory. Other base/model/list fields are outside
this projection. No initialization of byte `+4B`, for example, is claimed here.

Category must index both supplied global spans. The original has no bounds check.
Callbacks must return normally: original SEH exception cleanup/destructors and
allocation exceptions are not recovered here. Null allocation skips construction
but is still passed to attachment. Our null-tolerant test adapter verifies that
call sequence; it does not establish that the original attachment accepts null.

## Unit and native comparison evidence

Units pass on x64 at O0/O2 and x86 at O0. They check exact positive/signed-zero
initialization, preserved unknown fields, 16-bit counter wrap, exact event order,
category/primary values at both attachments, returned-pointer use, all 64
allocation/constructor-null combinations, and final callback mutations.

The native oracle executes the entire unchanged 811-byte constructor plus the
unchanged helpers listed above. Only named external subsystem entries are
redirected to controlled adapters; model slot `+8` uses an adapter vtable. The
native invocation still installs/restores its original SEH frame, but no exception
handler is invoked. The runner checks executable identity and every loaded code
span before mapping it.

**10,000 cases pass at both O0 and O2.** Comparison includes ordered callback
arguments, intermediate snapshots of all projected actor fields and global
tables, final actor/global state, return identity, and every byte in the synthetic
`0x3A0` actor buffer. Untouched bytes outside the projection retain their dirty
initial patterns, except the explicitly installed model adapter vtable.

Cases include all four synthetic category indices; count boundaries 0, 32767 and
65535; allocation-null and constructor-null combinations; dirty initial fields;
distinct allocation/constructor-return addresses; callbacks that change fields,
resource IDs and counters between reads; signed zero, quiet NaN, infinities,
smallest positive subnormal and largest finite position arguments. Arithmetic
is not performed on input positions by this orchestration. Signaling NaN/FPU
exception behavior is not claimed.

This is synthetic, controlled original-byte comparison of constructor
orchestration, not an observation of a complete actor spawn in the original game.

### Composed inline-model pass

The oracle additionally runs **10,000 cases at O0 and O2** with the actual
`43CE00` inline-model constructor called from unchanged `42DE51`. Its complete
normal nested chain (`465840`, `4680F0`, `4409E0`, `440A10`, empty member
constructors, CRT vector construction and success cleanup) runs unchanged.
The existing 10,000-case model-boundary mutation pass remains intact.

For this second pass only, the source fixture's model callback calls recovered
`initialize_actor_model`. It starts from the same dirty model storage and
compares all inline-model bytes (`actor+78` through `actor+33F`) as part of the
complete `0x3A0` actor-buffer comparison. Original model entry bytes are restored
before the second pass; the model-construction runner supplies every additional
code pin and the checked `4B15C0` constant.

Concrete initialization emits no synthetic model-constructor event; all other
ordered effect arguments, callback mutations, global values, null-allocation
branches and final bytes remain compared. The original model vtable identity
`4B14A8` is installed by the original constructor; its later position-sync slot
`+8` remains the declared adapter. Base/shadow/state construction, resource
binding, registration, allocation and other effects remain boundaries in this
composed pass. This closes the inline-model initialization boundary in the
comparison without claiming full game spawning or model-position mapping.

The fixture's concrete composition also has a unit check for initialized model
identity/mode and preservation of position and buffer trailing bytes. Actor
construction units pass on x64/x86 at O0/O2 after this extension.

Example invocation after building the x86 oracle at an alternate image base:

```powershell
python tools/run_native_actor_construction_oracle.py --exe 'C:\progs\Lemmings Revolution\Lemmings Revolution.exe' --program build/x86/native_actor_construction_oracle.exe
```
