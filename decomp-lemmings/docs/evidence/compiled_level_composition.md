# Compiled-level tooling composition

## Reference and claim

The reference context is the repository-local x86
`Lemmings Revolution/Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
The native-004 worker checked this digest by reading bytes only. Adjacent
`lemmings.box` and `patch.box` supply the optional read-only retail inputs.
Paths are resolved from the repository root.

[read_compiled_level and load_compiled_level](../../include/lemmings/compiled_level.hpp)
are new tooling APIs implemented in
[compiled_level.cpp](../../src/compiled_level.cpp). They compose recovered
serialized readers; neither is a port of a complete original function. They
have no assigned original address, recovered calling convention or original
runtime object layout. The original loader address in the retained notes is
evidence for the component read sequence, not an address for these APIs.
The ledger's addressless composition row adds no recovered-function coverage
and does not change the historical `372 / 6,186` baseline.

## Retained original-reader evidence and source contracts

The following existing notes record original-reader addresses, reference byte
pins, static observations and their own verification limits. Their historical
105-entry and x86/x64 fixture observations are retained observations, not new
measurements made by this task. Older incomplete-recovery wording in the
prefix note describes that note's boundary at the time.

| Order / tooling section | Source reader | Retained evidence |
| --- | --- | --- |
| 1. `prefix` | [read_compiled_level_prefix](../../src/level_name_tables.cpp), using [read_compiled_level_header](../../src/compiled_level_header.cpp) | [004432c1.md](004432c1.md): marker, six fields, flags, name tables and initial object |
| 2. `world geometry` | [read_world_geometry_prefix](../../src/world_geometry_prefix.cpp) | [world_geometry_prefix.md](world_geometry_prefix.md): blocks, required class-one record, arrays, fixed bytes, names and tail lists |
| 3. `paired objects` | [read_object_base_prefix](../../src/object_base_prefix.cpp) | [object_base_prefix.md](object_base_prefix.md): object pair, recursive children and auxiliary payloads |
| 4. `entity tables` | [read_level_entity_prefix](../../src/level_entity_prefix.cpp) | [level_entity_prefix.md](level_entity_prefix.md): registry size, groups, name pairs and initial actor lists |
| 5. `gadget records` | [read_level_gadget_records](../../src/level_gadget_records.cpp) | [level_entity_tail.md](level_entity_tail.md): remaining entities, tagged/named records and final fields |

The composition creates one `LevelByteStream` at cursor zero over the caller's
byte span and passes that same stream through the five readers in this order.
After the last reader, `consumed_bytes` is the stream cursor: the serialized
boundary relative to the beginning of the supplied span. No section is parsed
again to calculate that boundary. Raw binary64 values remain integer bit
patterns. This preserves serialized payloads; it does not establish x87
precision, arithmetic, exceptional-value handling or conversion behavior.

The marker comparison remains the component reader's contract. As recorded
in [the loader prefix evidence](004432c1.md), the original caller continues
after the marker helper reports a mismatch. The tooling likewise retains
`marker_matches == false` and continues parsing supported subsequent data.

## Tooling EOF, diagnostics and archive adapter

`require_eof` defaults to true. After successful section reads, strict mode
requires `consumed_bytes == bytes.size()`; otherwise it throws
`trailing level data at byte N (R bytes remain)`. Lenient mode returns the
same serialized boundary and leaves the suffix unconsumed. This EOF policy
is a tooling check, not recovered original-loader behavior.

While reading sections, `read_compiled_level` catches `std::invalid_argument`
and prepends `SECTION at byte CURSOR: ` to the underlying message. The cursor
is where the component stopped when it threw, which can be after some reads
inside that section. It is not generally the section start or the location of
the malformed field. The five start-boundary truncations below deliberately
fail before the first read advances. Other exception types are not decorated
by this catch; bounds/count/depth safeguards and C++ exceptions do not model
the original loader's malformed-input or crash behavior.

[load_compiled_level](../../src/compiled_level.cpp) delegates selection to
`find_box_entry`, reads the selected entry through `BoxArchiveIndex::read_entry`
and uses strict parsing. Its result carries the selected archive path, stored
entry name, declared entry size and composed level. A missing lookup throws
`level not found in archives: NAME`. Parser `std::invalid_argument` failures
are further prefixed with the stored entry name. Lookup and entry reading
occur outside that parser catch, so archive I/O failures are not promised
the same decoration.

[box_archive.md](box_archive.md) supplies the separate retained archive-index
and lookup evidence: first matching archive/entry, ASCII name normalization,
and patch-before-base registration in the original's observed setup. The
tooling receives caller-supplied archive indexes and preserves their order;
it neither discovers archives nor automatically sorts them patch-first.
Callers wanting patch precedence must pass `patch.box` before `lemmings.box`.
The test's CLI takes base then optional patch, and constructs that vector in
patch-first order. The portable filesystem/index/byte-span adapters do not
reconstruct original file handles, CRT allocation, drive/CD discovery or
locale-specific uppercase behavior. The selected entry is read in place,
without extracting assets or modifying the installation.

The retained retail test obtains its expected location by calling
`find_box_entry` itself, then compares the composed loader's selected path,
entry name and size/consumed boundary. This checks composition with that
dependency; it does not independently prove archive lookup correctness.
No new archive implementation or filesystem fixture is introduced here.

## Asset-free composition checks

[test_compiled_level.cpp](../../tests/test_compiled_level.cpp) now builds a
bounded synthetic Level 23 using explicit little-endian integer and string
primitives. It runs before the existing `argc == 1` return. All variable-length
collections are empty; the required class-one base and both class-zero paired
objects are present, with empty child/auxiliary lists. No recursive child is
created. Distinct literal header, world, paired-object, registry and gadget
sentinels check that every section contributes to the result.

Expected values and offsets are literal assertions derived from the serialized
shape, independent of production-reader execution. No component is parsed
separately, and no second production parse supplies the expected result.

| Section | Start | Bytes | Independent size accounting |
| --- | ---: | ---: | --- |
| Prefix | 0 | 74 | 12 marker + 24 fields + 4 flags + 24 counts + 10 short string |
| World geometry | 74 | 6478 | 24 leading fields/counts + 141 empty class-one base + 12 following fields + 16 array counts + 6144 fixed bytes + 13 names + 128 tail-list counts |
| Paired objects | 6552 | 298 | 149 first object including second-present byte + 149 second object |
| Entity tables | 6850 | 20 | 4 registry size + 16 empty table/list counts |
| Gadget records | 6870 | 115 | 48 list counts + 4 intervening value + 44 fixed integers + 4 integer-array count + 5 flags + 2 short + 4 value + 4 final count |
| End / consumed boundary | 6985 | | Sum of the five serialized sections |

The checks assert representative values from all sections, every empty
collection boundary, both paired objects, exact consumption and unchanged
input bytes. The world values include raw `0x8000000000000000` (negative zero)
and `0x7FF80000000023AB` (NaN payload); neither builder nor expectation converts
them to floating point. Additional finite binary64 patterns check the world
base and both objects.

Strict EOF succeeds on exactly 6985 bytes. Appending `AB CD EF` must produce
exactly `trailing level data at byte 6985 (3 bytes remain)`; lenient mode must
retain consumption of 6985 bytes. Replacing only the final marker character
with `4` produces same-length `Level 24`, a false `marker_matches`, and the
same remaining sentinel values and consumed boundary. Input snapshots also
remain unchanged after strict rejection, lenient parsing and marker mismatch.
Truncation at each of the five literal starts checks the exact decorated
diagnostic, section label and cursor. The earlier empty-input and missing-level
rejection checks remain.

All prior retail iteration, marker, collision mapping, archive selection,
trailing-input and final-byte truncation assertions remain. The retail loop
visits every `.LVL` entry in each supplied archive, including overridden base
entries; its count is entry visits, not unique patch-overlaid level names.
It does not assert all decoded fields of the retail levels. Other component
fixture comparisons remain separate evidence in their notes.

## Verification and limits

Native-004 focused verification on 2026-10-30 UTC used the registered UCRT64
MinGW-w64 GCC toolchain and existing Debug recovery configuration. The build
of `test_compiled_level` passed. CTest passed `compiled_level` and
`installed_compiled_levels` (2/2); the latter printed
`Parsed 105 installed level entries through the composed API`. This is a
current source/asset composition observation in the focused log, separate
from the older 105-entry observations above. The log, JUnit and inventory are
retained under ignored `build/native-004-worker/` as
`focused-verification.log`, `focused-results.xml` and `focused-inventory.json`.

The inspected recovery cache has empty `LEMMINGS_REFERENCE_EXE`. Its inventory
has 84 tests and only one `native_`-prefixed test, the ordinary recovered-source
`native_int_array`; it registers no original-byte oracle. The full
`python tools/agent_control/verify.py --profile all` run is delegated to the
supervisor after worker exit, as directed. The required main 70/70 and recovery
84/84 results remain pending for this change until that log exists; focused
2/2 is not a substitute. The task report under `build/native-004-worker/` and
the supervisor's `build/agent-runs/` record retain the verification handoff.

No original game or copied original instructions were executed; no hooks,
debugger, capture, launcher or installation changes were used. Existing oracle
code and retained component comparison results establish no original comparison
for either tooling API. Source tests validate the adapter contracts and
composition within the tested inputs only. Complete serialized parsing does
not construct original runtime objects, resolve resources, perform original
allocation/registration side effects or establish playable state. Production
integration, original runtime behavior and unsupported variants remain outside
this task. Independent acceptance review follows supervisor verification.
