# Resource manager construction

Authority: installed `Lemmings Revolution.exe` SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
[REA records](resource_manager_construction_metadata.json) retain the six
procedure identities, evidence IDs, complete reported body ranges, pseudocode,
callees and limitations. [Original assembly](resource_manager_construction.asm)
records the pinned instruction bytes; all claimed writes and calls were checked
against local disassembly of the digest-verified executable.

| Routine | VA / file offset | Bytes | SHA-256 |
| --- | --- | ---: | --- |
| Sprite manager | `0x484E40` / `0x84E40` | 31 | `abc654c126fc53484988c6d82c1b27bd769cfac8fe24f09aac83e92549a14ee1` |
| Named manager | `0x485120` / `0x85120` | 87 | `e2c88a9d6d75312e95dfe18ca4f3458937bf4f21db6f2591f9d729e5ad7899dd` |
| Base manager | `0x4852D0` / `0x852D0` | 44 | `f96b6ded611f158614ece584fd9815320401773e881c7483472a6d0f6d90ec02` |
| Sentinel list | `0x4855F0` / `0x855F0` | 119 | `637f730d8f47f10710df0a739f47f5dcff74504268cd8f24f1b2458afb229b81` |
| Empty sentinel node | `0x485730` / `0x85730` | 22 | `7153939bf56a7e8321d61a27f4b79a78b95b23606a048e8b3faa8027df24a98e` |
| Registration vector | `0x485470` / `0x85470` | 53 | `1f0030291fa3a68771361e044671fd4c4ed373926b81c41092ccd2071eedc704` |

Existing helper `430920` (file offset `0x30920`, 33 bytes) has SHA-256
`65eb72c5dcc4eef9e79634224883eed9512f5f5d518e120100eaca3f5333f080`.
Its source is reused from the resource cache loading slice. It clears the
payload resource pointer at `+00` and reference count at `+08`, preserving the
ID word at `+04`.

## Static behavior recovered

The complete normal constructor chain is:

```text
484E40 sprite manager
  485120 named manager
    4852D0 base manager
      4855F0 sentinel list
        485730 begin sentinel -> 430920 payload
        485730 end sentinel   -> 430920 payload
    485470 registration vector
```

`485730` invokes the payload initializer and returns its original node pointer.
It does not touch either link or the node's ID. `4855F0` initializes the begin
payload first, then the end payload; afterward it writes links in this order:

1. `begin.next = &end`.
2. `begin.previous = nullptr`.
3. `end.next = nullptr`.
4. `end.previous = &begin`.

The two sentinels occupy `0x14` bytes each in the original. Both ID words remain
untouched even when the enclosing manager is fully constructed. The list
constructor returns its original list pointer.

`4852D0` constructs this list at manager `+04`, writes vtable pointer `4B1BD8`
at manager `+00`, then clears loaded-resource count `+2C`. It leaves the named
manager's vector at `+30..3F` untouched. `485120` invokes that base constructor,
initializes the vector through `485470`, then replaces the vtable with
`4B1BC0`. `484E40` invokes the named constructor, then replaces the vtable with
`4B1BA8`. Every manager constructor returns its original manager pointer.

The vector initializer writes count zero, capacity zero, growth one, and
backing pointer null, in that order. There is no allocation, cleanup, resource
lookup, factory invocation, or other external effect in these normal paths.
Calling these routines on an already populated object overwrites its fields;
it does not release the old resources.

## Projection and boundaries

The source reuses `ResourceCacheLookupManager`, `ResourceCacheLookupList`,
`ResourceCacheLookupNode`, and `ResourceRegistration`. The loaded count remains
an explicit separate reference corresponding to manager `+2C`. The registration
view references `ResourceCache` backing storage and provides the vector's count,
capacity and growth words. These views must identify distinct logical manager
fields; no 64-bit ABI equivalence to the original is implied.

The established vtable value is written as a little-endian 32-bit token into
`ResourceCacheLookupManager::unknown_00`. It does not install a host C++ vtable.
This avoids adding invented virtual classes or dispatch behavior while retaining
all constructor bytes. C++ default member initialization and the existing
portable list scaffolding do not replace the recovered constructors: tests
explicitly dirty all corresponding original bytes before invoking them.

The implementation claims normal completion only. Native `485120` and `4855F0`
execute their original SEH frame setup/restoration, but exceptional unwind and
compiler-generated cleanup handlers are not reconstructed or exercised. There
are no adapted effect calls in this native test.

## Unit and native evidence

Units pass at x64 O0/O2 and x86 O2. They check each final vtable token, sentinel
resource/reference clearing, preservation of both IDs, all four list links,
loaded count and vector defaults. They also ensure the base constructor leaves
the vector untouched, the node helper preserves every byte except its two
payload words, and the vector helper preserves the rest of the manager.

At **both x86 O0 and O2**, **35,000 full 64-byte comparisons pass**, comprising
5,000 independently dirtied manager buffers for each of the seven entrypoints:
`484E40`, `485120`, `4852D0`, `4855F0`, `485730`, `485470`, and `430920`.
The original functions and all nested helpers execute unchanged, with **no
adapters**. Tests compare return pointer identity and every original byte,
including unknown/preserved IDs and portions outside the selected subobject.
The typed output normalizes its two inline sentinel addresses to the original
buffer address before the byte comparison; no other differences are excluded.

The runner checks the full executable identity and every routine pin before
loading bytes. This establishes constructor behavior, including its empty-list
invariants; it does not establish virtual resource loading, allocator behavior,
or a complete resource manager lifecycle.

```powershell
python tools/run_native_resource_manager_construction_oracle.py --exe 'C:\progs\Lemmings Revolution\Lemmings Revolution.exe' --program build/x86/native_resource_manager_construction_oracle.exe
python tools/run_native_resource_manager_construction_oracle.py --exe 'C:\progs\Lemmings Revolution\Lemmings Revolution.exe' --program build/x86/native_resource_manager_construction_oracle_o2.exe
```
