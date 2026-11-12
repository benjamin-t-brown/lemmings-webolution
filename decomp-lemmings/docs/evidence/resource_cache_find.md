# Loaded resource lookup

Authority: installed `Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.

| Routine | VA | PE file offset | Bytes | SHA-256 |
| --- | --- | --- | ---: | --- |
| Lookup | `4306F0` | `306F0` | 106 | `c9ac76eb0fdf7ee30c39be22275c04a317e44ba8e990c4aa6d4030d792b31b72` |
| First node | `430760` | `30760` | 17 | `a26cd3064709e57ee483cfbbb2753f8da501718ce4ced1e834a5123dd2fcaa3d` |
| End test | `430780` | `30780` | 29 | `279bb61281a416389bf3313e5358634b5d927b0d349db8573597cecd1ad504d0` |

Original instruction bytes are in [resource_cache_find.asm](resource_cache_find.asm).
Selected REA records are retained in
[resource_cache_find_metadata.json](resource_cache_find_metadata.json).
Lookup evidence: `ev_9e639d5c33170c3517c3aa3b8227e9a707cfe22766d7c463e002128e8416b9e2`.
First-node evidence: `ev_f5413304b0c2d6c7b2c7b1fd31fb428ee200d55ea734b11eb5d2ba3553e1e3ff`.
The end-test helper was read directly from the verified original bytes.

## Static recovery and shared layout

The manager contains a loaded-resource list at `+04`. Its first-node pointer
is at list `+0C` (manager `+10`); its inline end sentinel starts at list
`+14` (manager `+18`). Each original node has:

| Node offset | Observed field |
| --- | --- |
| `00` | Resource pointer; returned by the separately recovered cache acquire path. |
| `04` | Resource ID, compared by this lookup. |
| `08` | Reference count, preserved by this lookup. |
| `0C` | Next node pointer. |
| `10` | Previous node pointer, established by constructor `430A10`; ignored by lookup. |

`ResourceCacheLookupNode` embeds the existing `ResourceCacheNode` at offset
zero and adds next/previous pointers. The existing `unknown_04` member is
exposed as a signed ID by `resource_cache_lookup_id`, without changing the
shared header. The native x86 view has the original 20-byte node size; x64
uses native pointer sizes. The manager lookup view ends at original offset
`2C`, leaving the loader's separate count outside this lookup module.
The list constructor in this module only supplies a portable empty-list
setup; recovery of the original list constructor is not claimed.

`430760` returns the begin sentinel's next pointer. `430780` compares the
candidate address with the inline end sentinel address; it does not
read through the candidate. `4306F0` starts at first, checks end before
reading an ID, follows next links until a matching ID or end, then returns
the matching node or null. Duplicate IDs resolve to the first match in link
order. All 32 ID bits participate, including negative values. End-sentinel
payload is never a match. Resource pointer, reference count and backward
links do not affect matching. No memory is written.

The public node-returning function supports loader/cleanup composition. The
manager wrapper returns the embedded `ResourceCacheNode*`, whose address is
identical to the original node address on x86. It performs no acquisition,
reference-count change, registration, loading, or name matching.

## Verification

**Unit tests:** x64 and x86 pass 319,076 generated lookup comparisons plus
directed assertions for empty lists, sentinel IDs, duplicate precedence,
negative IDs, null resource payloads, count preservation and reordered links.
Full fixture object bytes are compared before/after every fixture to detect
writes. Fixtures have 0 through 64 linked nodes in shuffled storage order,
with disconnected nodes and deliberately irrelevant backward links.

**Native comparison:** the complete original lookup and both helpers run
unchanged, with **no adapters**. At both O0 and O2, 319,076 lookup calls and
120,000 helper calls agree with the portable code. The native harness also
checks all fixture object bytes remain unchanged. End tests include null,
begin, end, real-node and arbitrary nonmatching pointer values; they require
no pointer dereference. Fixtures give the end sentinel a matching candidate
ID and an invalid next pointer, proving the traversal stops at the sentinel.
The runner verifies executable identity, PE mappings and all three code hashes
before loading these bodies for execution.

Valid live nodes and a chain reaching end are caller preconditions. The
implementation does not invent null-link checks, cycle detection, bounds
checks or fallback results for invalid lists. Nonterminating or faulting
invalid inputs are not executed in the tests. This is isolated native
comparison, not a live game-session observation.

Build `tools/native_resource_cache_find_oracle.cpp` and
`src/resource_cache_find.cpp` for x86 with image base `0x10000000`, then run:

```powershell
python tools/run_native_resource_cache_find_oracle.py --exe 'C:/progs/Lemmings Revolution/Lemmings Revolution.exe' --program build/native_resource_cache_find_oracle.exe
```
