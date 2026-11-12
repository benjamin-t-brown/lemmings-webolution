# native-015: consume structured cylinder secondary triples

2026-10-30. Implementation prepared for `needs_review`. Full verification and
queue transitions belong to the active supervisor; independent acceptance is
still required. No original executable, copied original instructions, oracle,
hooks, debugger, capture or launcher ran. Installed assets were read only.

## Production change and evidence

Changed files: `src/assets/Cylinder.cpp`, comments in `Cylinder.h`, focused
assertions in `src/__test__/test_surface.cpp`, and this note. Authentic frozen
sources, drivers, snapshots, inventories and reports are ignored under
`build/native-015/`. No recovery source/header, CMake, renderer, geometry helper,
other loader, simulation, queue or control file changed.

The traced application path remains `src/app/main.cpp:301`
`runApplicationLeg` -> selected-level `AssetLibrary::read` ->
`findCylinderMesh` at line 422. Failure returns before `TubeScene::build` at
line 429. `src/app/TubeScene.cpp:48` accepts the mesh, copies it at line 57,
loads both existing slot vectors at lines 85-86, and builds surface faces at
line 101. This change permits the previously rejected supported serialized
input to reach scene construction when the other scene prerequisites hold.
It does not demonstrate a repaired retail frame.

Reference: repository-local `Lemmings Revolution/Lemmings Revolution.exe`,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`,
checked by reading bytes only. Retained static evidence is
`decomp-lemmings/docs/evidence/004432c1.md` and `world_geometry_prefix.md`.
The sequence `0x004432C1` -> `0x00439FD3` -> `0x00469578` reaches the
class-one reader after the ring/radius blocks. `0x00469578` calls
`0x00431390` twice, each reading a u32 count and triples through
`0x00480BB0`, before the named-reference count. The retained body identities
include `0x00469578` SHA-256
`a29be6ee24128d14dae022d962ec6888e43b041f75053f1831ff1a4e4690f2ac`,
`0x00431390` SHA-256
`e8062c35d6ab10dc04eb4254758489d8c94d96fe4ff11e8a8821f0fe525e522d`,
and `0x00480BB0` SHA-256
`657dbef004ae2a8d230c1d82eb021ca4dfaedfcf59599a6e9c8d666594c0f454`.
These are retained evidence, not new execution/comparison results.

The unchanged `decomp-lemmings/src/level_name_tables.cpp` represents the second
array as `following_class_one.second_triples`; `read_triples` bounds its count
against the remaining bytes divided by 24. The unchanged
`world_geometry_prefix.cpp` preserves the ordered enclosing stream reads.
Production uses their existing linkage and portable interface. No new native
calling convention, allocation layout or field-offset overlay is introduced.
Serialized widths are separate from original object storage and ABI evidence.

## Exact boundary contract

Before this task, successful recovered world decoding was followed by a
compatibility-tail check requiring a zero word after surface triples. Complete
worlds containing one or more secondary triples failed directly with
`cylinder: expected a zero word before the tile texture list`. This was a
source-derived loading failure. Native-014 deliberately asserted that rejection;
its note, original frozen reports and timeout records remain unchanged.

After this task, the structured call passes the already-decoded array length
as an internal optional parameter to `readMeshTail`. At that position it
consumes the validated four-byte count and exactly `count * 24` payload bytes,
then resumes the existing tile count and both complete slot-block loops. The
payload is opaque: no value is decoded again, evaluated, shaded, added to the
surface or exposed as a public field. There is no string/count scan. Cursor
checks use subtraction after proving the cursor is in range, and bound the
secondary count by remaining bytes / 24 before multiplication.

The legacy locator supplies no secondary length. It retains its zero-word
policy, diagnostics and scan verbatim. Only `invalid_argument` from the two
existing recovered reads enables fallback; decoded metadata and tail failures
return directly. Allocation failures and unrelated exceptions are not caught.
Ring/thickness policies, radius-count coupling, all bit transfers, metadata,
public fields/helpers, tile limits and name policies remain. `offset` is the
actual world-section start; `endOffset` remains the completed compatibility
tile boundary, rather than the recovered reader's final cursor.

## Executed source regressions

`build/native-015/freeze.py` copied authentic task-entry `Cylinder.cpp` and
`Cylinder.h` before production edits. Entry-state hashes, staged/unstaged diffs
and status are retained. `prepare_driver.py` / `compare_loader.cpp` and
`run_checks.py` build separate frozen/current public-loader executables with
registered `C:/progs/msys2/ucrt64/bin/g++.exe`, C++23 and existing source-built
libraries. No production file is overwritten to exercise the frozen version,
and no stream source is duplicated in build registration.

The zero-array baseline independently specifies both rings, 128 triples, 64
radii, three surface triples, every metadata value, both complete slot vectors,
world offset 76 and tile boundary 3806. Expected data comes from explicit
little-endian fixture values, never from a second recovered invocation.

| Public driver input | Frozen | Current | Current offset / endOffset |
| --- | --- | --- | --- |
| Zero secondary triples | PASS, exit 0 | PASS, exit 0 | 76 / 3806 |
| One secondary triple | Expected rejection, exit 2 | PASS, exit 0 | 76 / 3830 |
| Three secondary triples | Expected rejection, exit 2 | PASS, exit 0 | 76 / 3878 |

`pre-change-fixture-1.log` and `pre-change-fixture-3.log` retain the authentic
zero-word rejection. Current fixture logs assert every existing mesh field,
all binary64 component bits, full arrays/slot strings, unchanged input and an
empty structured diagnostic, both before and after a usable legacy fragment
is appended as trailing input. The boundary advances by exactly 24 bytes per
secondary triple. Frozen zero-array loading passes as a control.

The extended public tests use independent zero-array expectations for zero,
one and three entries. Opaque payloads contain negative-zero, quiet-NaN and
name/count-like bit patterns. Tests cover every byte cut within the secondary
count and payload, including cuts between entries, retaining the recovered
cause and legacy failure together. A legacy fragment with a nonzero word at
the same position still rejects with its combined diagnostic. Existing
radius-mismatch, ring/thickness validation with later usable legacy fragments,
prefix/world/truncated-tail, bit-retention and legacy hard-tail assertions
remain. Only the intentional nonempty-secondary rejection assertion is
superseded by successful complete-output checks.

Fresh asset-free `test_surface.exe`: **3,794 checks, zero failures** (0.054611 s
process duration). The registered focused targets built successfully. CTest
`^(surface|tileposition|actorpose|corpus)$`: **4/4 PASS, 64.05 s**, with surface
32.63 s and corpus 29.86 s; `focused.log` and `focused.xml` retain results.
No tileposition, actorpose or corpus test source changed.

## Installed overlay comparison

Both authentic frozen/current loaders read every effective installed level
through production `AssetLibrary` base/patch overlay and `levelFileNames`.
Newly measured: **102 effective levels / 105 physical archive entries**.
Each run had **zero failures, 102 structured successes, zero fallbacks**.
Separate inventory-only recovered reads found **zero secondary triples,
zero populated secondary arrays, zero inventory failures**; distribution
`{0: 102}`. Those inventory reads provide no expected mesh values.

| Driver | Public loader calls | Complete corpus wall time | Process duration |
| --- | ---: | ---: | ---: |
| Frozen | 1.32932 s | 2.77151 s | 2.782646 s |
| Current | 1.29308 s | 2.73036 s | 2.740455 s |

The complete **4,130,189-byte** snapshots match byte for byte: level identities,
all pre-existing `CylinderMesh` fields including `meshFallbackReason`, each
binary64 bit, every array element and slot string, and both offsets. Input
preservation is checked per level. **No corpus discrepancies** were found;
comparison would stop on any mismatch. Corpus time includes overlay reading,
input copies, serialization and the inventory-only read; these source-driver
durations are not game timing or a performance guarantee. Older 102/105
observations remain attributed to their original notes.

## Preservation, verification limit and next step

`audit.py` / `audit.json` check the 1,650 entry-state file hashes and authentic
frozen sources. Only the three intended source/test files changed among those
files; this note is newly added. Staged diff is byte-identical. The exact
entry-state surface test is reconstructed under ignored storage and its hash
verified before saving `task-surface.diff`. Every prior direct `check(...)`
statement survives, as do all non-superseded validation calls. The legacy scan,
geometry helpers, radius/surface readers and tile loops are unchanged; public
header code is identical after comments are removed. Task text uses LF and
`git diff --check` passes.

Read-only inventories retain **70 main / 84 recovery registrations**. Recovery
has empty `LEMMINGS_REFERENCE_EXE` and `native_int_array` as its sole
`native_`-prefixed source test. No original-byte oracle is registered or run by
these checks. Other queue entries, native-006 deferral, STATUS gates, recovery
PROGRESS, the 372 / 6,186 historical baseline and D1/D3/digger parks remain.

The worker has **not** run or claimed a pass for the full verifier. As explicitly
assigned, the active supervisor runs
`python tools/agent_control/verify.py --profile all` after worker exit using
registered tools. Native-014's retained 240.03 s and 240.07 s `levelreturn`
timeouts remain failures in their historical records; this focused pass does
not reinterpret them. Next: supervisor full verification, then separate
read-only review of every native-015 acceptance criterion before `done`.
The worker leaves queue transitions to the controller.

Remaining compatibility limits: the two slot blocks still use the existing
name/value-compatible layout; arbitrary reference values and generic tile
semantics are outside scope. Named references are not substituted for slots.
Secondary field meanings, original allocations/object construction, ABI layout,
x87 precision/conversion, radial-layer ordering, rendering topology, timing and
frame fidelity remain unresolved here. Further rendering/resource integration
requires its own bounded evidence-backed runtime task; no such extra task was
started, and no retail-frame repair or original-game fidelity is claimed.
