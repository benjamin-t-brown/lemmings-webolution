# C3aa — explicit-matrix source mesh preparation

2026-10-25. Production preparation, not submitted scene geometry or pixels.
Packet `build/tmp/c3aa-scene-mesh-source/round-note.md`. This task was in flight
before render-first steering884798a; no further reader is added by its landing.

`buildSceneMeshSource` takes parsed tree plus REQUIRED explicit parentWorld/view
matrices. It preserves source identity, local/world/view coordinates, material
logical name/selector and raw local normal/index. Each corner retains its own raw
UV/RGBA and a separate **per-record last-index-store** attribute tuple. Empty
material name remains empty, not a texture object. Type2 yields no mesh candidate
but traversal continues; this does not mean type2 is not drawn. No admission,
CPU predicate, clip, runtime matrix, GPU state or resource quantizer is selected.

## Facilitator check

Read full source/header/test and separate wiring hunks. Native layout/matrix/
attribute facts reuse C3n; no new native body or live read was needed. The
independent C++ dump `build/tmp/c3aa-facilitator-dump.cpp` calls the actual adapter
on all4retained roots with explicit identity parent and accepted reference view.
Separate Python checker compares every record/corner to C3n source table and
re-reads raw source fields/normals from LVL. **1177 records/4243 corners**, including
**24 aliased records**, match identity/material/attribute values; max transformed
coordinate discrepancy **2.842170943040401e-14** (<frozen1e-9). Own first check
looked for a nonexistent normals_offset table key; corrected to independent raw
node position-count/normal-pool byte calculation, then all fields passed.

Single-field regression: compiled a private source copy changing only hierarchy
composition `compose(node.matrix,parent)` to its reverse. Same independent
expected table fails at first634B7 world coordinates, error730.6670379638672.
Restored private copy byte-for-byte to production SHA256
`f146f56aaea0300cc7488acbea32f370da7ff2a47fd810df30d4211f3cf129b8`;
production was never modified by this independent mutation. Logs/JSONL in
`build/tmp/c3aa-facilitator-*`. Worker's permanent asymmetric/alias tests and
matrix-order regression supplement this full retained-source comparison.

Fresh build and **38/38 CTest passed** under shared C3 mutex. Optional
retail permanent test was also manually run by worker with explicit install
environment; its new CTest target does not yet join the configured environment
property list, so a generic CTest pass is not claimed to exercise that retail arm.
The independent full4tree comparison above does. App integration should add the
new retail-target environment wiring; do not mistake skip for coverage.

Only this round's one test/source-entry wiring hunks are staged; sibling raster/
billboard edits stay unaccepted and uncommitted. No original process/install
write or owned live resource. App still omits scene; baseline0.2050523>0.02 is
unchanged. Rendering integration and explicit repair4 scoring are next under
steering, not another source/contract-only round.

Dispatch15:57:12; worker16:09:34; accepted16:20:46 MDT, **23m34s**.
