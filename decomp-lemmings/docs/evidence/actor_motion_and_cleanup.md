# Actor sample-Y movement and extra-state cleanup

## Authority and byte ranges

Installed `Lemmings Revolution.exe` SHA-256:
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
The sibling remake was not used as evidence.

| Routine | Address | File offset | Bytes | SHA-256 |
| --- | --- | ---: | ---: | --- |
| Move sample Y | `0x0042EE74` | 192116 | 47 | `4df50eff7bce9fd5c64817e772f5a5629c8c855b882dcba4557dd4e3a9b1111b` |
| Leave/clear extras | `0x0042E78A` | 190346 | 105 | `c9d0af90fd6a61c55724341da8e80c9da60674fa546f78b216b4f03adc4f9ef7` |
| Destroy/release list | `0x004302B0` | 197296 | 187 | `f85bf9bf209b039ea65fc25c49f9c2718b8991e2f99f44446c1a77feb3673dd1` |

REA pseudocode evidence, respectively:

- `ev_c313290e7fc5431e2ea6998fd3f0bb788aefb94d17fe3e5e2da10df90b5b5492`
- `ev_e01a06ea64f079401e49cb8281001a7a4d43dcb1f30cb13584f40dd1c002bdf5`
- `ev_3d3ffb557385f7dbd1f059cdb2fce1b067579de6f7b298362571f0e6add885fb`

Local instruction inspection independently checked stores, calling conventions,
both list traversals, and next-pointer reads relative to callbacks. The native
runner verifies the full executable, PE mapping, these byte hashes, all executed
list/clamp helper hashes, and the binary64 zero constant before passing bytes
directly to the test process. Original bytes are not saved in the repository.

## Move sample Y

`move_actor_sample_y` subtracts actor `+368` (the Y component of `pair_360`)
from the supplied sample Y. The x87 subtraction is stored to binary64 as the
argument to `0x0040DF50`. The recovered Y clamp result is stored at actor `+10`.
No X/depth component or other actor field changes.

This routine moves the actor so its offset sample has the requested Y before
clamping. It does not compute world depth or apply an additional terrain test.
The source calls the existing recovered `clamp_cylinder_y` directly; the native
oracle executes the original clamp and height accessor unchanged.

## Extra-state cleanup

`clear_actor_extra_behaviors` recovers `0x0042E78A` and its `0x004302B0` helper
using the existing logical `ActorBehaviorUpdate` and list nodes:

1. Traverse the extra list in forward order, invoking every behavior's leave
   slot `+0xC` with the actor. Read the next node **after** leave returns.
2. Restart from the list's current first node. For each node, invoke its
   nonnull behavior's deleting destructor, slot zero, with flags 1. Read next
   **after** that callback. Release the node through the `0x0042FF50(node,1)`
   boundary, then advance to the cached next node.
3. Reset first to the inline end sentinel, clear begin.previous and end.next,
   and set end.previous to begin. Sentinel behavior payloads remain untouched.

All leaves precede all deleting-destructor calls. This is different from
leaving/destroying/releasing each node in one pass. Nodes are not unlinked
individually before release; the sentinel reset happens only after the cleanup
pass. The actor's primary behavior is unchanged.

Callback changes to traversal are preserved. For example, a leave callback can
prepend a node which is then destroyed by the second pass without receiving a
leave call in the first pass. A leave callback may clear its own payload,
causing that node's destructor to be skipped while its node is still released.
Changing a node's next pointer during its destructor affects which node follows;
changing it during release does not, because next is already cached.

Leave and deleting-destructor operations are the existing behavior interfaces.
`ActorBehaviorClearHooks::release_node` represents the final node deletion. It
must not invalidate another still-required node. The source does not attempt to
recover from invalid pointers, callback exceptions, or nonterminating list
mutation; the original does not enforce those conditions either.

## Verification

**Static:** REA and original bytes establish the numeric stores, two-pass
lifetime order, and exact final sentinel writes.

**Unit tests:** x64 and x86 `test_actor_helpers` check normal movement, offset
subtraction, lower/upper clamping, NaN behavior, preserved coordinates, and
85 cleanup configurations: sizes 0 through 16 crossed with ordinary callbacks,
payload clearing during leave, head insertion during leave, next-pointer
mutation during destruction, and next-pointer mutation during node release.
They assert all leaves precede destruction, flags are 1, traversal changes are
respected, primary remains unchanged, and sentinels reset exactly.

**Offline native comparison:** unchanged `0x0042EE74`, `0x0042E78A`,
`0x004302B0`, list accessors, clamp, and height accessor execute in an isolated
32-bit process. The motion test compares all actor bytes for 20,000 inputs,
including every pair of 14 floating point edge values and arbitrary binary64
patterns. The cleanup test compares callback traces and sentinel state across
the same 85 configurations, with leave/destructor/node-release adapters. Both
unoptimized and `-O2` native builds pass. Tests use valid backing storage and
masked floating point exceptions. They do not exercise a live game or actual
heap deallocation inside the adapters.

```powershell
python tools/run_native_actor_helpers_oracle.py `
  --exe 'C:\progs\Lemmings Revolution\Lemmings Revolution.exe' `
  --program build/x86/native_actor_helpers_oracle.exe
```
