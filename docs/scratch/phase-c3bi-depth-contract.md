# C3bi — selected-record depth contract, no correction

2026-10-26. **Accepted bounded comparison; C3 remains failed.** Cumulative
attempt7, one disassembly-worker dispatch. Directive SHA-256
`da9fa952b300b28df1cbcf745e34a330d8d2e07b4bb7f198baa2eb5a8c312291`.
Packets: `build/tmp/c3bi-depth-contract/round-note.md` and
`build/tmp/c3-depth-pass/independent-comparison.json`.

## Result and decision

The three C3bh records use coherent **physical view-depth in our software
buffer**. Independent reconstruction from their source triangles exactly
reproduces every retained accepted depth assignment:

| Stored record | Kind/material | Source corners | Exact accepted-write replays |
|---|---|---:|---:|
|`0x63877`|5 / sand_1|3|9,393|
|`0x6b72b`|5 / sand_1|3|14,146|
|`0x7047c`|7 / wood1|4|5,709|

Total29,248. This is SOURCE arithmetic, **not** native admission, coverage,
effective device state, or a diagnosis of the C3bh depth-sensitive pixels.
No demonstrated factor-of-640/0.3 or billboard-conversion error was found in
this comparison. That negative is bounded by the matched, unrounded,
common-positive-scale model below, not a general native-depth equivalence claim.

`BackdropScene` passes physical Z and `.25/Z` unchanged through
`adaptSceneMeshVertex`; `SceneRaster` interpolates Z affinely with screen
barycentrics, compares Less, and writes the same shared buffer used by `Raster`.
RHW participates in attribute interpolation, not that depth interpolation.
For matched native pre-bias Z=`s*Zview`, a common positive scale cancels in
XY ratios and `sum(w*s*Z)/s = sum(w*Z)`. With native draw bias b, the comparable
value instead includes `-b/s`. **b and effective comparison state are unknown.**
C3am's distinct billboard slot/k conversion remains scoped and unchanged;
applying its divisor to already-physical mesh Z is not supported.

The known C3ak **final-f32-pack adoption gap is confirmed**, not solved:
the original writes f32 XY/Z, reloads that stored Z, then writes f32 RHW from
`.25/Zf`; our mesh adapter retains doubles. In a same-input RN/zero-bias
illustration, all10 corner comparison depths change after packing in the
normalized domain and converting back. Maximum difference is
`4.9692752327246126e-05` physical units. This is **not a native error bound or
screenshot prediction**. Four source corners are outside the stored screen
bounds; none of these10 is certified as an actual native post-clip pack input.

**No production correction is selected.** Quantizing physical Z directly would
not implement native-domain quantization. Choosing a scale/bias/clip input or
effective state to force an adoption now would fill the very missing inputs
this comparison exposes. The approved conditional correction allowance is not
an instruction to change precision automatically or to choose by pixel score.

## Exact unresolved boundary

- Actual per-record repair4 transform stack/projection operands, including
  animated ancestors; retained A1/C3n context is not that draw's state.
- Per-draw camera+`0x770` bias, signed-i32 centers+`0x754/+0x758`, projected-cache
  flag+`0x58`, and pre-/post-clip intermediates feeding the final pack.
- Runtime x87 PC/RC. Destination f32 width is read; RN numerical examples do
  not measure the native rounding mode or upstream precision.
- Successful effective ZENABLE (including W-buffer possibility), ZWRITEENABLE,
  ZFUNC/ZBIAS, viewport/range, buffer format/quantization and clear values across
  selected submissions/later consumers. C3q's requests/caches are not those values.
- Actual admission/submission and Device3/wrapper route success. No defaults
  or C3bh removal result substitutes for these inputs.

These are stop inputs, not newly authorized observation tasks. Observer refusal,
C3bd/C3ao ordering, material corrections and legacy-unattributed Entrance/Exit
remain intact. No expanded census, suppression sweep, fit, or observer work ran.

## Facilitator check

1. Rebuilt/re-executed the scratch production-input extractor into an independent
   output path and reviewed its actual Camera/SceneMeshSource/adapter call chain.
   Separate `verify-comparison.py`, importing no worker arithmetic, authenticates
   stored LVL records/positions/ancestor matrices, independently transforms raw
   positions, computes all10 corner packing examples, and replays the29,248
   original diagnostic CSV depths exactly. Sequential versus composed transform
   checking uses a3e-13 software-operation bound, not a native precision tolerance.
2. Fresh actual PE reads cover projection`0x46DDF0..46DE68`, final
   pack`0x46D4F6..46D5BE`, camera literals`0x468FC9..4690E3`, and1/.25 constants.
   All5 worker windows match the executable. `46DE50 dc a1 70 07 00 00`
   subtracts bias after XY; `46D516 d9 5a 08`, `46D51C d9 40 08` and
   `46D51F dc 3d d8 19 4b 00` establish the f32Z store/reload/reverse divide.
   `.data` raw bytes end at`0x4C6000`; no BSS value is inferred.
3. Three selected corners distinguish the Z reload. For record407671/corner1,
   normalized Z`0.40629171334538516` stores`0.4062917232513428`; RHW becomes
   `0.6153213977813721`, versus`0.6153214573860168` if reload is omitted.
   The standalone independent checker also rejects reciprocal depth interpolation
   at pixel49791: observed affine`1034.6232774549599`, alternative
   `1034.5649664967555`. These are comparison controls, not production regressions.

The worker's first overly strict historical-transform equality failed and is
retained. Its later historical bound is not used as a native tolerance. Retained
evidence was read rather than regenerated in place.

### Closing checks and unchanged gates

Own `mutate-comparison.py` changes **only** the Z-reload operand in a scratch
copy of the delivered calculator (`packedz` to `nz`). The calculator still runs;
the separate checker fails on the407671/corner1 RHW value above. Restoring that
one operand restores passing output and exact calculator SHA-256
`e1be4b6a6b0bd20c0fa147b7891fa6506b52fa1c42eb7b16dad2d375fcadc2f0`.
The delivered worker script and production source remain unchanged. This protects
the comparison, not native fidelity or a newly implemented renderer behavior.

Fresh final full build and **44/44 CTest pass** (63.20s); framework tests **32/32
pass**. Own independent Pillow/integer scorer re-renders the three fixed gates,
before and after verification. Each PNG equals the retained baseline byte-for-byte:

| Gate | Error | Verdict |
|---|---|---|
|C3 repair4 full frame, floor included|14,934,330/235,008,000 =0.06354817708333334|**FAIL**,0.02|
|C1 zoom1 footprint|285,083/17,721,990 =0.0160863988750699|PASS|
|C1 elev_down footprint|285,700/17,721,990 =0.016121214378294987|PASS|

Repair4 PNG remains`9a5baba7264e3b2672ada76b7b26ba4e6044588c28dcad284e6f3ff5bd48f145`.
No source change, mask/reference/camera/exclusion/floor change or gate movement.
All21 fixed/source/install inputs and250 prior evidence files match initial
hashes. Nine initially dirty paths are preserved, with only this pass's isolated
PLAN status/parking lines and Phase C append added; unrelated changes are excluded
from round commits. Independent end inventory finds0Lemming processes/0LemWeb
tasks. Worker REA static target closed; no live resource owned. Zero stock starts,
attaches, injection, scheduled-task creation, package installations or install writes.

## Ownership, times and stop

Owner`ses_e85700de8ffeh9NovTQmub70J5`, parent`ses_e8675c6f3ffeWYENquwRYs5xF4`;
actual metadata confirms configured Astra/high. Prior owner/worker succeeded idle;
own read-only active-set checks show no competing project owner. Worker
`ses_e856c2d78ffehzTGP8LcgRnFkF` used configured disasm-agent Sol/high without
override, returned complete and is confirmed succeeded/idle.

Conservative pass start23:33:36Z; attempt7 dispatch23:37:39.661948Z; worker
completion23:53:48Z (API idle23:53:58.523Z). One of three attempts used, no repairs;
six earlier attempts remain counted. The original00:22:39.661948Z task deadline
and01:33:36Z overall ceiling were not restarted. Actual verification/landing/end
timestamps and elapsed values are in the queue/archive.

Stop at the accepted bounded comparison and exact unresolved inputs. No second
dispatch or automatic correction/observation question. Directive consumed despite
its active heading; remaining capacity is not authorization to continue.
