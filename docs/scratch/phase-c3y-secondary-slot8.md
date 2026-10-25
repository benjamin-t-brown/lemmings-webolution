# C3y — secondary root sorts children, then calls their slot4

2026-10-25. Offline READ + source-pose DERIVED, no runtime trace.
Packet: `build/tmp/c3y-secondary-slot8/round-note.md`.

## Accepted operation

Factory/constructor/vtable bytes give type0/1/2 slot8 target **46B73F**;
their slot4 targets are46B6CC/46C55D/47D0C0. Secondary holder47491B invokes
root slot8, which is **not the root's slot4 renderer**.

Complete46B73F..46BADF pushes root matrix, temporarily composes each child matrix,
takes its composed translation, rotates by the first9 doubles of58D498 (no
translation term) and adds **f64 child+108**. It pops each temporary child matrix,
builds descending-key index order, then calls each sorted child's **slot4** at
46BAD1 with ECX=child and no explicit stack arguments. Root matrix remains on
stack until46BAD6 pop. No own root geometry or own visibility+110 test occurs
inside this complete body. This does not override the child's slot4 gates.

For the retained secondary root there are29 leaf children:12 meshes and17type2.
The source +108 value matters (grass1=25802.048391002703); it is not zero.
Source/reference inputs yield global-node order:

`6,5,10,7,9,8,11,12,13,19,24,16,15,18,22,17,14,23,25,20,21,29,30,28,27,26,31,32,33`.

The child's settled slot4 geometry receives the same source matrix/root/camera
composition as C3n/o, but **order and inherited caches/buffer state differ**.
Sorting is nonstable for ties; the four-equal-key fixture gives0,3,2,1. Distinct
finite retained keys avoid that ambiguity; NaN/empty/corrupt arms are not certified
by a Python stable-sort analogy. Scratch keys/indices58D528/58D518 and registration/
allocation success are conditional, not BSS file values.

## Facilitator check

`python -B build/tmp/c3y-facilitator-check.py` separately maps original PE sections,
authenticates24 windows/anchors and all three vtable slot pairs, and saves fresh
disassembly. Re-read full929-byte sort/dispatch body, index insertion helper,
push/pop/key/compare arguments and virtual call offsets. Reuses accepted C3k/n
matrix/rotation facts rather than repeating their research.

Separate raw LVL head/name/child-count reads and sequential point transforms
reproduce all29 keys/ranks; minimum adjacent key gap exceeds9e-5. Every sorted
node's reused geometry/material value list matches accepted C3n/o table exactly,
covering82 secondary mesh records. Candidate native_order is only system under
test: changing **one private source f64+108** at980FA to50000 moves node26 from
rank25 to0, while per-node geometry/material values remain unchanged. Restoring
the eight private bytes restores the complete LVL buffer and table. Original
install/captures are unchanged.

The packet additionally flattens24 mesh/5type2 candidate draws and119 vertices.
**Mesh clipping/counts are inherited C3n candidates, not accepted native counts**;
only source ordering is newly verified here. Original precision, animated matrices,
historical gates, actual child visibility, effective state/API success and unwritten
specular remain conditional. No new native visible-pixel/floor identity acceptance.

Fresh wrapper build and **35/35 CTest passed**. No Lemmings process or live
resources; no source edit or new score. C3 baseline0.2050523 remains above0.02.
Next verify returned Zstatic provenance and park unresolved observation at its
cap, while continuing explicit-input mesh source/raster implementation.

Dispatch15:27:50; accepted15:47:59 MDT, **20m09s** dispatch-to-acceptance.
