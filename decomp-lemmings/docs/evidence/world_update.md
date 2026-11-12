# Complete world update orchestration

Authority: installed `Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.

| Routine | VA | PE file offset | Length | SHA-256 |
| --- | --- | --- | ---: | --- |
| World update | `4423D2` | `423D2` | 3,569 | `e1b02be8cfe9fb35345494b637c8a07db697ff288f9e52914372293637628fbf` |

REA evidence: `ev_36e58cc18fa573df10c5e4935cc6397d3f747c0c8d94ea5139992151311d9c8b`.
The complete original instruction bytes are in
[world_entity_pass.asm](world_entity_pass.asm). Additional accessor bytes are
in [world_update.asm](world_update.asm); helper REA IDs, metadata and
pseudocode are retained in [world_update_helpers.json](world_update_helpers.json).

## Static recovery

`update_world` implements the complete routine's normal control flow and
composes `update_world_entity_collections` directly. Known world/entity fields
have explicit offset names. Opaque animation and auxiliary records retain their
original 40-byte and 376-byte strides. Countdown records retain the 384-byte
stride, paused byte `+168`, double `+170`, and uninterpreted surrounding bytes.

1. If world byte `+5139` is zero, write status `+00=1` and return. Otherwise
   store elapsed at `+5120` and update inline `+5140` through `437BDE`.
2. In order, update optional objects `+50B8`, `+50C0`, `+50B4`, `+50BC`,
   `+50C8` through `4911E9`, `4457E0`, `49185C`, `49259C`, `4457F0`.
   Every call receives the original elapsed double; each pointer is read when
   its branch is reached. Some decompiler signatures omit these arguments.
3. Only when animation count `+538C` equals zero, look up the first resource
   ID at array `+170` through global `58CB48`, use resource payload `+1C`,
   construct two temporary animation objects, configure inclusive ranges
   `F0..F7` and `F8..FF` with interval 0.06, append both, then destroy the
   temporaries in reverse order. Update entries 0 and 1 unconditionally.
4. If `+5138` is nonzero, perform the sequence logic described below.
5. Subtract elapsed from every unpaused countdown record. Invoke `43B419`
   on inline world `+08`, then update every inline auxiliary record at
   collection `+1FD0` through `41F92E`. Counts are signed and read live.
6. Execute the recovered collection pass, then call `45AF0E` on `+27D0`,
   `44D22C` on `+2018`, `43B150` on `+08`, and `445498` on world itself.
   All except `43B150` receive the original elapsed double.

### Sequence control

When 16-bit count `+5308` is zero, if sequence pointer `+539C` is null and
byte `+50D3` is zero, clear `+53A0`, create `413D40(5,5)`, and call its virtual
slot `+04` with null actor and a local positive-zero time. Reset cursor
`+5304` to the actor list's last node (`end.previous`) regardless of creation.

Query global `4C6A08` through `445436`; any nonzero integer invokes `444A34`
and then clears word `+5098`. An existing sequence gets `414132(320,240,1)`
and virtual `+04(null,&remaining)` with a fresh copy of original elapsed.
Only return value **1** completes it: call virtual `+0C(null)`, reread the
sequence pointer, destroy it with flag 1 if still nonnull, clear the pointer,
and set bytes `+53A0` and `+50D3` to 1. Callback changes to the pointer are
observed at the original read boundaries.

If `+53A0` is nonzero, cursor is not the actor list's begin sentinel, and
`!(timer >= 0)`, schedule one `416910` behavior through actor `42E696`.
The unordered comparison is intentional: original `4428C8..4428D9` tests only
x87 C0, so **NaN also enters this branch**. Capture the actor and its old
primary `+340` before allocation; destroy the old behavior only when it is
nonnull and differs from the actor's primary after installation. Advance
through the **live** cursor's previous pointer, increment the 16-bit counter
with wrapping, and add 0.05 to the live timer. Finally subtract original
elapsed from the timer whenever the sequence section was enabled. There is
no catch-up loop: at most one actor is scheduled per update.

Double constants use their exact binary64 patterns: 0.06 is
`3FAEB851EB851EB8`, while 0.05 at VA `4B15E8` is `3FA999999999999A`.
Zero at `4B0338` is eight zero bytes. Timer arithmetic uses extended
intermediates with explicit binary64 stores between addition and subtraction.

## Verification

**Unit tests:** pass on x64 and x86. Directed checks cover the disabled early
return, initialization ranges, NaN/negative/signed-zero sequence gates,
16-bit wrapping, reverse cursor movement and one transition per update.
10,000 deterministic fixtures exercise enabled/disabled subsystems, signed
array counts, sequence completion/noncompletion, allocation returning null for
an actor behavior, callback mutations and arbitrary binary64 time values.

**Native comparison:** 10,000 complete ordered trace and final-state comparisons
pass at O0 and O2. The entire original 3,569-byte routine runs **unchanged**;
there are no internal block-skipping jumps. It runs all collection traversal
instructions with nonempty, mutating fixtures, plus 32 unchanged field/list/
array accessor bodies. The runner verifies the executable hash, all 33 code
spans, PE mappings, and the two loaded floating constants before execution.

A single ordered trace includes setup, optional updates, sequence calls,
collection callbacks and the tail. Comparisons cover the original elapsed
arguments, local sequence time arguments, state transitions, sequence pointer
and reverse cursor, signed counts, countdown doubles, array contents and every
actor primary pointer. Quiet NaNs are compared as a class for arithmetic
results; signed zeros and finite/infinite values are compared by bits.
Tests include pointer replacement during sequence positioning, sequence
clearing during leave, live timer mutation during behavior installation,
stored-elapsed mutation, live auxiliary count changes, and the collection
mutation cases documented in [world_entity_pass.md](world_entity_pass.md).
Native adapters check inline receivers, resource payload, global manager
receivers, fixed sequence arguments and destructor flags.

## Explicit boundaries

Callbacks represent optional subsystem methods, resource lookup, animation
construction/configuration/append/destruction/update, sequence object methods,
allocation plus the two constructors, behavior installation/destruction,
world actions, auxiliary update, and tail methods. Their internals are not
claimed recovered by this module. The collection pass retains its documented
seven external effect adapters and entity virtual method boundaries.

Allocation and exception handling are intentionally explicit boundaries:
constructors are combined with their allocation in factory callbacks. A null
actor-behavior allocation is supported and passed to `42E696`. Creating a
null sequence before the original unconditional virtual call would fault;
valid sequence creation is a precondition. This comparison covers normal
control flow, not allocation exceptions, SEH dispatch or a live game session.
The full world update is reconstructed orchestration, not a claim that every
called subsystem or the complete game has been recovered.

Build the x86 oracle from `tools/native_world_update_oracle.cpp`,
`src/world_update.cpp` and `src/world_entity_pass.cpp`, with image base
`0x10000000`, then run:

```powershell
python tools/run_native_world_update_oracle.py --exe 'C:/progs/Lemmings Revolution/Lemmings Revolution.exe' --program build/native_world_update_oracle.exe
```
