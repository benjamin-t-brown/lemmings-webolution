# World collection update block

Authority: installed `Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.

The recovered scope is the contiguous **1,632-byte block** at
`0x442B00` through `0x44315F`, inside `0x4423D2`. Its PE file offset is
`0x42B00`, and its SHA-256 is
`ad4924ec9e84a4c270624a31618c06ef16d136287bd529988d687dbb17f3f711`.
The enclosing routine is 3,569 bytes; its unrelated preceding/following work
is outside this implementation. REA full-function evidence is
`ev_36e58cc18fa573df10c5e4935cc6397d3f747c0c8d94ea5139992151311d9c8b`.

[world_entity_pass.asm](world_entity_pass.asm) records original instruction
bytes for the full enclosing routine and all 24 executed accessors.
[world_entity_pass_accessors.json](world_entity_pass_accessors.json) retains
REA metadata and evidence IDs for the 20 newly inspected accessors. The runner
pins all 25 loaded code spans by address, PE file offset, length and SHA-256.

## Static recovery

The pass visits these collections in this order:

| World offset | Selection and unsuccessful update handling |
| --- | --- |
| `1EA0` list | Update all; zero result queues entity and unlinks node. |
| `1EB8` array | Update all; zero queues cached entity and erases index; decrement index. |
| `1EC8` actor list | Update all; zero queues entity, reports signed identifier and unlinks node. |
| `1EF0` array | Update enabled entities; ignore result. |
| `1F00` array | Update enabled entities; zero queues live entity and erases live entry; no decrement. |
| `1F50`, `1F60`, `1F70` arrays | Update enabled entities; ignore result. |
| `1F90`, `1F20` arrays | Update all; ignore result. |
| `1FA0` array | Update enabled entities; zero deactivates live entity then registers live entity. |

Enabled means the byte at entity `+38` is nonzero. Update's low return byte
alone determines success. The original identifier getter returns signed
16-bit `+3A`. Time is forwarded verbatim, with no positive/finite gate.
Array counts are signed and read on each loop check. Negative and zero counts
perform no visits. The source preserves the cached linked successor, cached
`1EB8` entity, and live slot/data rereads after callbacks in `1F00` and `1FA0`.

The portable structures expose only these known fields and callbacks; their
host layout is not presented as the complete original object layout. Disposal,
list unlinking, array erasure, logging, registration and entity virtual methods
remain explicit external effects. The original logger also receives global
`4CBF88` and a fixed format string; this port represents its observed semantic
boundary as `actor_removed_notice(identifier)` and does not reconstruct logging.

## Unit verification

x64 and x86 unit tests pass. Directed assertions cover collection removal
rules, disabled entities, nonzero `0x80` return values, negative counts,
cached successor traversal, and cached-versus-live disposal pointers.
An additional 10,000 deterministic fixtures exercise all eight callback
mutation modes and elapsed values including signed zero, negative time,
infinities and NaN.

## Native comparison

The x86 harness passes **10,000 ordered callback-trace and final-array-state
comparisons at both O0 and O2**. It maps the hash-verified enclosing routine
and executes the original collection instructions plus unchanged accessor
bodies. Two explicit adapter jumps skip unclaimed surrounding world work:

- `4423F6 -> 442B00`, after the original prologue saved `this`.
- `443160 -> 4431B3`, to the original SEH-restoring epilogue.

No instruction in `[442B00,443160)` is patched. Virtual methods and the seven
external effect targets are adapters: `4090B0`, `445950`, `408FC0`, `40F860`,
`4465D0`, `446C70`, `43B536`. The native/portable adapters apply matching
synthetic effects and compare the resulting call order and arguments.
Mutations replace current entries during update/enqueue/deactivation, alter
linked next during update, shrink/grow dynamic counts, and replace an array's
storage during enqueue. Array erasure shifts entries, revealing the original
difference in index adjustment. The fixture's unlink adapter records its call
without destroying nodes; valid lifetimes are maintained for cached successors.

This is isolated native comparison of the claimed block. It does not verify
the seven effect implementations, exception dispatch, preceding/following
world work, or a running game session.

Run with an x86 build of `tools/native_world_entity_pass_oracle.cpp` plus
`src/world_entity_pass.cpp`, linked at image base `0x10000000`, then:

```powershell
python tools/run_native_world_entity_pass_oracle.py --exe 'C:/progs/Lemmings Revolution/Lemmings Revolution.exe' --program build/native_world_entity_pass_oracle.exe
```
