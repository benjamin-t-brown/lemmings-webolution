# native-016: independently counted cylinder double blocks

2026-10-30. Implementation prepared for `needs_review`; the full-verification
gate remains unmet after repeated `levelreturn` timeouts (see retry below).
Queue transitions, full verification and independent acceptance belong to the
active supervisor. The worker did not
run the original executable, copied original instructions, an oracle, hooks,
debugger, capture or launcher. Installation files were read only.

## Runtime change and files

Before this change, `findCylinderMesh` decoded a complete recovered world,
then reread its second-array count in `readMeshTail` and required equality with
the first-array length (`ringCount`). Complete supported worlds with two first
blocks and zero, one or three second blocks failed with
`cylinder: the radius array's ring count does not match the mesh`. Native-014
and native-015 deliberately retained that assertion. Their notes and reports
remain unchanged. This is a source-derived serialized-loading limitation, not
an observed retail-frame failure.

After this change, structured loading flattens the independently decoded second
blocks into the existing `radii` field and resumes the compatibility tail after
their actual payload. Empty second arrays produce empty `radii`. The legacy
locator retains its count equality and complete radius-reading stage.

Changed files:

- `src/assets/Cylinder.cpp`: split the shared surface/secondary/tile tail from
  the legacy radius stage; project decoded second blocks and check the cursor.
- `src/assets/Cylinder.h`: comments distinguish retail count/vertex observations
  from the independent serialized arrays. Public code and field names remain.
- `src/__test__/test_surface.cpp`: complete public-loader expectations, combined
  boundaries, special bits, malformed/truncated second arrays and legacy policy.
- This note; source-only drivers, frozen sources and reports are ignored under
  `build/native-016/`.

The traced production route is `src/app/main.cpp`'s `runApplicationLeg`:
`AssetLibrary::read -> findCylinderMesh` (line 422), return on failure, then
`TubeScene::build` (line 429). In `src/app/TubeScene.cpp`, the scene copies the
mesh (line 57), loads both skin slot vectors (85-86), builds optional surface
faces (101), and copies surface/cylinder data into actor-pose storage (164-169).
Skin drawing uses `CylinderMesh::vertex` (504-507); actor-pose preparation retains
its surface-index and cylinder-template path. The executed public-loader change
removes the previous early return for the supported unequal-count fixtures.
Other scene prerequisites remain conditional. No renderer or simulation
consumer gains a use for `radii`, and no geometry helper changes.

## Retained evidence and exact boundary

Reference identity was checked by reading repository-local
`Lemmings Revolution/Lemmings Revolution.exe` bytes only: SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Retained evidence: `decomp-lemmings/docs/evidence/004432c1.md` and
`world_geometry_prefix.md`. After the fifth resource table and bounded string,
`0x004432C1 -> 0x00439FD3 -> 0x0043D0F0` reads a u32, binary64, then separate
counted arrays through `0x0043E5A0` and `0x0043E500`. Their record readers
`0x0043D09A` and `0x0043D060` consume 64 triples / 1,536 bytes and 32 doubles /
256 bytes respectively. Retained evidence pins addresses, lengths and body
hashes; no fresh disassembly or original execution was performed here.

The unchanged `world_geometry_prefix.cpp` reads separate `first_count` and
`second_count` values without an equality test. Production reuses those decoded
arrays through existing linkage. Each second-block component is transferred by
`bit_cast<double>`, in block/component order, without floating-point arithmetic,
normalization, a new public field or a geometric interpretation.

The shared surface-tail cursor is exactly:

```text
worldStart + 16 + first_blocks.size() * 1536
           + 4 + second_blocks.size() * 256
```

The adapter starts at the saved prefix cursor. Before each advance it proves
the cursor is within the input and the count is at most remaining bytes divided
by the nonzero serialized width; multiplication and addition follow that check.
Compile-time shape assertions retain both block widths. The second-array byte
bound also bounds the component-count multiplication used by `radii.reserve`.
There is no additional parser, tail scan or dependence on the recovered final
cursor. `offset` remains `worldStart`; `endOffset` remains the completed
compatibility tile boundary.

The shared tail's executable body from the surface header through both tile
loops is unchanged: it writes `one`, consumes surface triples, then the already
decoded secondary-triple count and opaque 24-byte entries before both slot
blocks. Both paths retain existing limits, field writes and diagnostics. Only
`invalid_argument` from the existing recovered reads enables fallback. Decoded
ring/thickness and compatibility-tail errors return directly; allocation
failures and unrelated exceptions are not caught. The legacy-only route keeps
its exact locator, radius reading/equality, zero-secondary-word policy and
error messages.

## Executed source regressions

Before production edits, `build/native-016/freeze.py` saved authentic
`Cylinder.cpp` and `Cylinder.h`, entry-state hashes of 1,651 files, Git status
and staged/unstaged diffs. Frozen hashes are checked before driver builds and
by the preservation audit. `prepare_driver.py`, `compare_loader.cpp` and
`run_checks.py` build separate frozen/current public-loader executables with
registered MinGW-w64 GCC 16.1.0, C++23, existing source-built libraries and
production `Box.cpp`/`LevelIndex.cpp`. No production source is overwritten to
exercise frozen behavior. No build registration or recovery source changes.

Little-endian fixture expectations specify every metadata field, all 128 vertex
triples, the complete variable-length double vector, all three surface triples,
both full slot vectors, diagnostics and both offsets. They do not obtain
expected mesh data from another recovered invocation. Second-block payloads
include negative-zero and quiet-NaN bits; checks inspect binary64 bits without
evaluating geometry. The three-block driver also contains three opaque secondary
triples, combining both previously integrated boundaries.

| First / second blocks | Frozen public loader | Current public loader | Current offset / endOffset |
| --- | --- | --- | --- |
| 2 / 0 | Expected rejection, exit 2 | PASS, exit 0 | 76 / 3294 |
| 2 / 1 | Expected rejection, exit 2 | PASS, exit 0 | 76 / 3550 |
| 2 / 2 | PASS control, exit 0 | PASS, exit 0 | 76 / 3806 |
| 2 / 3, plus 3 secondary triples | Expected rejection, exit 2 | PASS, exit 0 | 76 / 4134 |

Authentic rejections are retained in `pre-change-fixture-{0,1,3}.log`.
Each current case checks all fields again after appending a usable legacy
fragment and checks input preservation before/after both calls. End positions
are independently calculated as `3294 + secondCount * 256 + secondaryCount * 24`.

The surface suite tests all four second counts with both zero and three
secondary triples, with and without trailing input. Every byte cut within each
second count/payload is exercised through public `expected`, including block
boundaries. Truncated count errors retain the recovered world cursor 3164;
truncated payloads and an all-ones malformed count retain cursor 3168 and
`invalid world double block count`; all also retain the legacy failure cause.
A complete legacy-only fragment with a mismatched radius count still rejects.
Existing nonzero-secondary legacy rejection, prefix/world failures, ring and
thickness validation with a usable later fragment, ring boundaries, secondary
truncations, special vertices and legacy hard-tail assertions remain. A decoded
unequal-count world with a reference value of 513 additionally proves that the
compatibility slot-length error returns directly despite a later usable legacy
fragment. Only the deliberate structured radius-mismatch rejection is replaced.

Executed checks:

- Separate frozen/current builds and all public-driver regressions: expected
  exits passed; no compiler warnings in driver logs.
- Asset-free `test_surface.exe`: **12,516 checks, zero failures**, 0.159266 s
  process duration; retail environment intentionally absent for this check.
- Registered focused targets built successfully; CTest
  `^(surface|tileposition|actorpose|corpus)$`: **4/4 PASS, 65.22 s** (surface
  32.69 s, corpus 31.02 s). Results are in `focused.log` / `focused.xml`.
  No tileposition, actorpose or corpus assertion/source changed.

## Installed overlay comparison

Fresh production `AssetLibrary` base/patch overlay runs measured **102 effective
levels / 105 physical archive entries**. Both public loaders had **zero failures,
102 structured successes and zero fallbacks**. Separate inventory-only reads
provide counts, never expected mesh values: first-block distribution `{7: 102}`,
second-block distribution `{7: 102}`, joint distribution `{(7,7): 102}`,
secondary-triple distribution `{0: 102}`, zero populated secondary arrays and
zero inventory failures. This comparison covers every effective level, rather
than independently comparing overwritten physical entries. Historical 102/105
and equality observations retain their original scope in earlier notes.

| Driver | Public loader calls | Complete corpus wall time | Process duration |
| --- | ---: | ---: | ---: |
| Frozen | 1.29161 s | 2.85226 s | 2.865034 s |
| Current | 1.32462 s | 2.96986 s | 2.980298 s |

Complete **4,130,189-byte** snapshots match byte for byte: level names, every
pre-existing `CylinderMesh` field, all arrays and binary64 bits, all slot strings,
diagnostics and offsets. Inputs remain unchanged. **No discrepancies** were
found; the comparison stops on any unexpected difference. Wall times include
asset reading, input copies, serialization and inventory-only recovered reads;
they are source-driver measurements, not original-game timing guarantees.

## Preservation, limits and next action

`audit.py` / `audit.json` compare task-entry bytes, verify the authentic frozen
sources and save `entry-test_surface.cpp` / `task-surface.diff`. They check that
only the three intended existing files changed, staged work is byte-identical,
every prior direct `check(...)` survives, the single removed validation is the
documented mismatch assertion, the legacy locator/radius stage and shared tail
body are unchanged, and public header code is identical after removing comments.
Task-edited files use LF. Existing unrelated installation-file CRLF warnings
are preserved with their original bytes. Queue, native-006 deferral, STATUS gates,
recovery PROGRESS, the 372 / 6,186 baseline, native-014/native-015 histories and
D1/D3/digger parks remain untouched.

Read-only CTest inventories retain **70 main / 84 recovery registrations**.
Recovery has empty `LEMMINGS_REFERENCE_EXE` and `native_int_array` as its sole
`native_`-prefixed source test. No original-byte oracle is registered or run.

Full verification is supervisor-owned, as assigned:
`python tools/agent_control/verify.py --profile all`, using the registered tools.
The first supervisor result and fresh retry check are recorded below; neither
passed full verification. The focused pass does not erase native-014's
historical `levelreturn` timeouts. A further supervisor verification follows
worker exit, with separate read-only acceptance required before `done`.
The worker does not edit the queue.

The compatibility tail still represents two slot blocks in its existing
name/value-compatible layout; arbitrary reference values and generic tile
semantics remain outside scope. Independently counted serialized acceptance
does not establish arbitrary unequal worlds as playable in the original game.
Original allocation, ABI/object layout, array relationships, coordinate meaning,
x87 precision/conversion, radial layers, topology, timing and frame fidelity
remain unresolved here. No repaired-retail-frame claim is made. Further runtime
integration requires another bounded evidence-backed task; none was started.

## Retry: full-verification gate remains unmet

The first native-016 supervisor run,
`20261030T190827-40868-93cf530b`, ran the registered full verifier. Main CTest
passed **69/70**; `levelreturn` timed out at **240.02 s** (total **391.55 s**).
Recovery verification was not reached. This is a failed full verification.
The preceding native-015 supervisor run,
`20261030T184804-40868-a612efad`, passed 70 main / 84 recovery tests, with
`levelreturn` taking 232.95 s. That historical pass establishes neither this
retry's acceptance nor the timeout's cause.

This retry inspected the retained implementation, frozen sources, driver
expectations, production call path and previous failure log before making any
edits. Production and test bytes remain identical to the first attempt's
recorded source hashes. The loader already implements the assigned change;
no speculative production edit or test relaxation was made in response to a
timing failure.

`build/native-016/retry_check.py` saved a separate retry-entry manifest,
staged diff and source copies under ignored `retry-2/`, without replacing the
authentic pre-change freeze or earlier reports. It read the reference identity
and ran only the registered source-built `levelreturn` test, serially with SDL
dummy video/audio and its unchanged 240-second test property. That isolated
check **timed out at 240.03 s**, CTest exit **8**, process duration **240.068 s**.
`retry-2/levelreturn.log`, `levelreturn.xml` and `levelreturn-result.json`
retain the failure. Test retries stopped at this repeated failure. No new
driver/corpus sweeps or focused-suite reruns are claimed for this retry; their
earlier passing measurements above remain attributed to the first attempt.

`retry_audit.py` / `retry-2/audit.json` freshly check 1,652 retry-entry files,
staged-diff preservation, authentic frozen hashes, the unchanged production/test
hashes, all 77 prior direct checks, LF task text, the reference digest read from
bytes and the retained complete corpus snapshots. Read-only inventories again
show 70 main / 84 recovery registrations, empty recovery `LEMMINGS_REFERENCE_EXE`
and `native_int_array` as the sole native-prefixed recovery test. Before the
note update, every captured file matched retry entry; afterward only this note
differs. Queue, consumers, test limits, build settings, verifier, registry,
recovery and installation files remain unchanged by this retry.

The implementation is retained for review, but the full-verification acceptance
criterion is **unmet**. The source-only comparisons do not establish the
timeout's cause or rule out an application regression. Next: the active
supervisor runs the full registered verifier after worker exit. If the timeout
repeats, retain the blocker and stop for a separately bounded investigation of
the source-built `levelreturn` runtime; do not mark native-016 done, weaken its
criteria or expand this task into renderer/test-runtime changes. Independent
read-only acceptance remains required after verification passes. The serialized,
compatibility-tail and original-fidelity limits above remain unchanged.
