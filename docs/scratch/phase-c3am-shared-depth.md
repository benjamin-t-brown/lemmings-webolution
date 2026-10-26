# C3am initial/repair1 — conditional shared comparison-depth units

2026-10-25. Accepted software-contract correction, not native GPU state.
Packets: `build/tmp/c3am-shared-depth/repair1-round-note.md`, corrected
`derivation.md`; original rejected offset equation retained in `before-repair1/`.

## Facilitator check

Re-read actual source construction, Camera chain, submission adapter, raster depth
seeding and all changed files. Fresh executable reads retained in
`build/tmp/c3am-independent/exe-{source-stages,slots,ps}.log` cover
47D0F2..D37B,47D492..D6C3,468FC9..4690E1: source S precedes generated offsets,
final P follows them, slots retain their own projected Z; camera literals are
0.3 and10/3. This does not establish effective native Z comparison state.

The repaired relation is **slotZ=k*centerViewZ+finalPz*deltaSource**, where
k=parentPz*Sz*finalPz. Dividing by k yields equivalent physical corner depth
centerViewZ+deltaSource/(parentPz*Sz), NOT centerViewZ+deltaSource. Supported
domain is the production diagonal P/S, zero translation. Binary32 camera context
remains the accepted diagnostic model, not measured x87 precision.

Own separately written C++ oracle passes two destination occlusion pixels and
192 exact asymmetric actual-source-to-submission corners. Permanent test also
exercises both helpers: own slot depths19.5/19 become39/38 against opaque38.5.
Source-only raw-slot mutation fails the independent pixel oracle and permanent
test, and fresh full-frame error rises to16740766 (by1911602). Exact source
restoration hash c33993d0d1c97f2ca9cd5b491756d9a608c558b4aac94566582bf32a507351b4;
own restored build and42/42CTest pass. Scripts/logs: `build/tmp/c3am-independent*`.
Old independent C3o tuple replay still matches17nodes/340fields, max error
9.094947017729282e-13. Source-slot helper/tests unchanged; native XY/UV/RHW kept.

Worker accidentally overwrote `build/tmp/c3-facilitator-c1-report/` before fixing
its output directory. Prior contents/logs were not restored and are NOT relied
on as immutable evidence. Original references and independent `c3-check-*`
outputs are separate. Original packet hashes remain preserved; fresh independent
scoring below replaces reliance on the overwritten scratch directory.

## Fixed gate, fresh independent replay

`build/tmp/c3-check-am-final/` contains own repair4 and all10 C1 renders, explicit
reference scorer and independent Pillow partition agreement. Frame SHA256
463f31fce95b3d58304a76b1c4151b0779294a97eb7baf14ff271893216c1577.
**14829164/235008000=0.0631006774 FAIL**, gate4700160. Rest/floor12397921,
skills1490654,tube674411,counters175496,caption75548,actor boxes15134.
Fresh scene-off exactly matches baseline13b1bd501803c3bf3e210cac651b1968083b26c74a852925e04e5e6f44e3829e.
Floor remains included; no gate, mask, reference or exclusions changed.

C1 footprint numerators /17721990: zoom1285083,elev_down285700 (gated passes),
elev_up280461,initial1494385,canonical1494385,elev0_neg1494005,mouse1697627,
right1721725,left1414525,MM_DIG_0001795145. All full-frame supplemental scores
retained in own c1-scores.json. No conversion chosen by score.

Separate cleanup check: three install hashes equal pre-run manifest; no Lemmings
process or LemWeb scheduled task. Zero starts/attaches/writes. Dispatch19:34:53,
repair19:53:10; exact landing time in archive. Remaining packing adoption,
sorted/type1 dispatch, clipping and effective state still block C3 acceptance.
