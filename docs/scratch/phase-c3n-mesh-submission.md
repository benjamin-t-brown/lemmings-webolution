# C3n — conditional mesh source fields, binding and packing

2026-10-25. Offline READ + DERIVED; no production edit or stock start.
Packet: `build/tmp/c3n-mesh-submission/round-note.md`; full source/candidate
table `commands.json`, index `command-index.csv`, gaps `input-requirements.json`.
Reproduce with ordinary Python + Capstone:
`python -B build/tmp/c3n-mesh-submission/replay.py`.

## Accepted result

Four parsed Object trees contain **1177 polygon records in 19 mesh nodes**:
kind5=465, kind6=16, kind7=696. Kinds0..4 are absent in those trees. Source
corner count is **4243**; **24** records repeat a vertex index. Emitters assign
attributes to the indexed shared intermediate before drawing, so the last
attribute store for an index wins for every occurrence, not one independent
attribute record per corner. **304** records bind null via empty material names;
the other logical names and per-record indices are in the table. This is not a
census of final visible submissions or coverage of unread trailers.

New READ chain:

- `0x46A810..A949` uses three indexed vertices and copies UV/RGB/alpha to
  intermediate+28/+38/+50. Kind5 supplies its subrecord here. `0x4599C0..9AE8`
  uses four indexed UV corners with one shared RGB/alpha; `0x46ACB0..ADE9`
  uses four full-attribute corners. No per-corner permutation is added.
- `0x469D71` selects a **node material slot**, not a global handle. Loader
  `0x46B4E5..B585` resolves its exact string, stores the following u32 as the
  blend selector, and initializes UV offsets to zero. Empty string stores -1.
- `0x43BD80..BDE8` copies material+8/+10 into global UV offsets and resolves
  the handle through manager `0x58CB48`, or binds null. `0x472640..2656` sends
  texture-object+0 to stage0; texture-object+24 flags and API success remain
  distinct runtime conditions. File material flags are not texture-object flags.
- `0x46D372` projects pointers then clips. At `0x46DDF0..DE66`, flag+58 avoids
  reprojecting: X=cx+x/z, Y=cy-y/z, then z loses camera+770 bias; flag becomes1.
  Clip/projection origins are i32, not f32. Candidate source-pose defaults do
  not claim animated matrices, draw-specific bias or a measured FPU mode.

Final pack loop `0x46D4F6..D5BC` writes f32 screen X/Y/Z; f32 RHW is
**0.25 / already-stored-f32-Z**, reloading it at `0x46D51C`, `fdivr` at
`0x46D51F`. Raw double `0x4B19D8` is `000000000000D03F` =0.25.
Diffuse is truncated alpha<<24 OR channel0<<16 OR channel1<<8 OR channel2,
without clamping or per-byte masking. UV is f32(source+material offset).
**Specular+14 is unwritten, not zero.** The 32-byte layout/device+70 call
with primitive6/FVF1C4 maps to TL triangle fan under the documented Device3
route (SDK header identities in `sdk-check.json`); wrapper success is conditional.

## Facilitator check

`python -B build/tmp/c3n-facilitator-check.py` imports no worker parser or
transform/clipping implementation. A separate raw LVL parser reproduces all
record counts, source indices, vector fields, material names/selectors and four
tree ends `9176E/98D41/9DB2B/AAA43`. Sequential point transforms agree with all
source world/view triples within **2.2737367544323206e-13**, independently of the
worker's composed-matrix arithmetic. It authenticates all **84** raw PE windows
and writes fresh disassembly under `build/tmp/c3n-facilitator-check/`.
Emitter copy offsets/counts, material loader/bind, projection and complete pack
loop were semantically re-read against these bytes.

For the candidate's **2077 constructed post-clip intermediates**, separate pack
arithmetic matches every known output byte; a nonzero specular canary is retained.
Changing **only alpha by -1** through the candidate packer changes all2077
diffuse words and no other output bytes. Inputs are private buffers, discarded;
LVL/exe originals were not written. Worker's additional instruction-interpreter
2049 synthetic packing checks and SDK checker rerun pass; those supplement the
independent formula/source check, not replace it.

**Narrowing:** 2077 is a candidate table count, **not independently certified
native clipping/submission count**. General near/screen clipping, exact PC/RC,
animation, all-kind normal predicates, scene reachability and effective device
state remain unaccepted. The packet's clip implementation is a candidate to
test during production integration, not a measured original draw stream.
No uploaded texel words or actual texture+24 values are accepted here.

No live process/attach/write was owned. Install hashes were independently
checked with C3o's immutable ledger this pass. Production is unchanged; a fresh
wrapper build and **34/34 CTest passed** before landing. Original exe/LVL digests
match the packet, including `.data` raw-backed end0x4C6000.

## Next

Scene device-state requests and actual texture upload/flags are the next values;
derive independently, then integrate renderer and score **repair4**, not A6d3.
C3 baseline stays0.2050523 against0.02, with floor ring included. No pixel pass.

Dispatch13:37:26; worker finished14:02:45; accepted14:15:57 MDT
(**38m31s** dispatch-to-acceptance, including the resolved sync interruption).
