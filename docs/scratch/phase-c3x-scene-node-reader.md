# C3x — strict source-node reader, repaired allocation bounds

2026-10-25. Production **asset parsing only**, not app scene rendering.
Packet `build/tmp/c3x-scene-node-reader/round-note.md`. Original rejected
allocation behavior and original source/test/note hashes retained under
`before-repair/`; positive file-field checks were not withdrawn.

`readSceneNode(bytes, explicitStart)` returns one source tree and exact consumed
end, or an expected error. New `SceneNode.h/.cpp` preserve positions/normals,
node matrices/names/head doubles, material logical name + distinct blend selector,
block allocation word/kind/material slot, leading normal and ordered corner
indices/UV/RGBA, source offsets, children and branch0 type2 source fields.
Kind6 shared attributes expand to its four corners; no alias, transform, cull,
texture-object flag or visibility decision is applied by this reader.

Supported: node0/1/2 with type2 branch0; mesh kinds5/6/7. Other node/mesh variants
and nonzero type2 fail closed. No root scan, trailer inference, app route change,
or rewrite of legacy diagnostic PolyList. One source entry and one CTest target.
Retail tests follow optional WEBOLUTION_GAME_DIR convention, no build/tmp runtime
dependency. Source offsets are caller-selected, not a complete LVL root directory.

## Facilitator check

Read full header/parser/test/build diff. All native layout facts reuse C3g/l/n;
no new original-body claim or live read is introduced. Independent C++ verifier
`build/tmp/c3x-facilitator-check.cpp` calls the actual reader and **re-encodes every
field** with independently written layout logic, checking every stored source
offset along the way. Complete slices byte-match raw LVL:

| Node start | Exclusive end | Bytes |
| --- | --- | ---: |
|59644|9176E|229674|
|91782|98D41|30143|
|9CCD1|9DB2B|3674|
|A0CDD|AAA43|40294|

This protects all fields across40 nodes/19 meshes/1177 records/4243 corners,
not just aggregate counts. Accepted LVL hash remains95f84f86…4c567. Synthetic
tests separately check asymmetric scalar values, hierarchy, explicit nonzero
start, per-corner vs shared attributes, unsupported cases and invalid indices.

Initial independent negative: a124-byte root with9999 promised children attempted
**4559544 bytes** of eager reserve before truncation. A verifier-owned1MiB guard
converted that to bad_alloc. Returned repair instead of accepting green tests.
Repaired reader checks minimum remaining bytes before children/material/block
allocation and removes input-controlled eager reserves. Logical vector accounting
is limited to256KiB and aggregate estimate32MiB; these are **implementation
budgets**, not original-game limits or exact allocator-capacity bounds. Normal
vector growth may reserve extra capacity. Node/depth/count/overflow guards remain;
the supported retained trees fit. No whole-corpus completeness claim.

Recompiled the independent checker: all four source slices still match and the
same malformed input now returns `child count 9999 cannot fit in remaining input`
without oversized allocation. Permanent tests guard that exact case, nested
impossible promises, material/block/record counts and every truncated prefix of
the asymmetric supported tree.

Facilitator isolated mutation incremented only parsed normalIndex; focused
`scenenode` test failed8 assertions, including retail field/census checks.
Restored exact source SHA256
`9e053b932ef7f38e24bf5f5939c6d29ec36209b724cd61ad7a6ca1f1e7b40485`.
The first restoration retained one extra indentation space; hash caught it,
space restored and the full build/suite rerun before acceptance. Mutation log:
`build/tmp/c3x-facilitator-normal-mutation.log`.

No original executable, capture or install write; no Lemmings process/live owner.
Fresh wrapper build and **35/35 CTest passed** after exact restoration. App output and C3
baseline0.2050523>0.02 are unchanged: this module is not yet called by app render.

## Next

Explicit-matrix mesh source adapter, then independently validated submission/
raster/app composition. Secondary slot8 and observation provenance are separate
in-flight reads. Runtime gates/state/native format and exact floor visibility
remain unresolved, not implementation defaults.

Dispatch15:00:21; repair15:25:42; worker repair complete15:37:57; accepted15:43:48
MDT. Total **43m27s**, repair **18m06s** dispatch-to-acceptance.
