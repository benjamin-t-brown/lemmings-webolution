# Resource cache loading and list insertion

Authority: installed `Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.

| Routine | Address | Bytes |
| --- | --- | ---: |
| Load or retain cached resource | `0x00430660` | 132 |
| Initialize payload | `0x00430920` | 33 |
| Prepend wrapper | `0x00430950` | 25 |
| Allocate and prepend node | `0x00430970` | 147 |
| Construct list node | `0x00430A10` | 64 |
| Factory by entry name | `0x00485590` | 47 |

Exact original bytes, PE offsets and SHA-256 digests are retained in
[resource_cache_load.asm](resource_cache_load.asm) and the native runner.
[REA metadata](resource_cache_load_metadata.json) preserves complete dossiers,
including instruction listings and static pseudocode. Key evidence IDs are
`ev_76cd25c5b0e0ddd57b35c349de487057742839106a20be21340bbaf47d43c9af`
(load), `ev_21cdeafc7e98f51e83664d6d282114901de8128d9a552beb7815e9bed2d2e87e`
(prepend), and
`ev_824edb7f6f702842687f8a5eda96b69ed689ccc668c784c41b0881a841ecf465`
(node construction).

The name bridge dossier is retained [separately](resource_name_factory_metadata.json).

## Static reading

The loaded-resource list starts at manager `+04`. It contains inline begin/end
sentinels and 20-byte x86 nodes: resource pointer `+00`, resource ID `+04`,
32-bit reference count `+08`, next `+0C`, previous `+10`. Lookup is concrete;
see [cache search](resource_cache_find.md). The manager word `+2C` counts load
operations on this path and wraps modulo 2^32. Its broader lifetime meaning
is not inferred here. The source passes this live word separately from the
lookup manager view.

On a hit, `430660` increments the found node's count and returns its resource,
without calling the factory or incrementing the manager count. On a miss it
initializes a temporary payload, records the input ID, invokes virtual factory
slot `+0C`, and stores the returned pointer even when null. It allocates and
prepends a node containing that payload, rereads the current first node,
increments the live manager count, increments that node's count and returns
its resource. It does not repeat the search after the factory.

`485590` resolves the current indexed entry and passes its name pointer directly
to virtual slot `+14`, returning that method's register result. The bridge
neither copies the name nor searches for a duplicate resource name.

Prepend calls allocation before reading the first-node pointer. Thus factory
and allocator mutations of list links and the manager count are observable.
The new node points to the current first and the inline begin sentinel; the
old first's previous link is replaced before the first link is replaced.

Payload initialization clears only resource and references, preserving the ID
word. The node constructor initializes first, then copies resource, ID and
references in that order before setting links. If its source is its own payload,
resource and references consequently become zero. A null allocation in the
prepend routine writes null to the old first's previous link and the begin
sentinel's next link. The loader subsequently dereferences its first node;
normal load completion therefore requires successful allocation. No fallback
or additional null guard is introduced.

## Verification

Unit checks pass on x64 and x86. They cover preserved ID bits, self-aliasing
construction, null allocation in prepend, no factory work on a hit, null
factory results, links, counter wrapping and notification suppression in
balanced access.

At both O0 and O2, the x86 oracle passes **25,000** comparisons across load,
prepend, node construction and payload initialization. It checks return values,
callback traces, every node field, sentinel links, the source payload and the
manager counter. Cases include empty and populated lists, duplicate IDs,
negative IDs, reference/count wrap, callback changes to the current first,
and counter changes during factory/allocation calls. The only adapters are
virtual factory slot `+0C` and `operator new`; all claimed helper bodies and
the original list search execute unchanged.

A further **20,000** comparisons at each optimization level execute complete
original acquire/load/find/list-insert/release/balanced chains. They compare
entry cache pointers, names, unknown entry words, list state, counts and
notification traces against the composed recovered implementations. Existing
cache claims and byte pins are documented [here](resource_cache.md).
Some of these composed chains also execute the unchanged name bridge. An
additional **10,000** direct bridge comparisons at each optimization level
check the exact name pointer, empty through 127-byte names, callback traces
and resulting manager/list state.

These are offline native instruction comparisons, not a running game session.
The resource factory's internal decoding, allocation failure exceptions, SEH
unwinding and invalid/nonterminating lists remain outside the claim. The host
lookup-list constructor creates fixture scaffolding; it is not claimed to be
the recovered original manager constructor. Original data are read in place.

Build the native oracle for x86 with image base `0x10000000`, then run:

```powershell
python tools/run_native_resource_cache_load_oracle.py --exe 'C:/progs/Lemmings Revolution/Lemmings Revolution.exe' --program build/native_resource_cache_load_oracle.exe
```
