# C3bk — selected mesh scale/bias input contract

2026-10-26 local. **Accepted conditional READ/DERIVED contract; no implementation.**
Fresh user approval, cumulative attempt9 only; prior eight attempts retained.
Directive SHA256`0f56d31e9b71c537200cc8b6fbc3ab1ff80a6b5c8de10bc9d1b4c91877ea45b6`.
Worker packet:`build/tmp/c3bk-scale-bias-contract/round-note.md`.
Independent check:`build/tmp/c3-scale-bias-pass/independent-contract.json`.

## Recovered contract

For **matched geometric transforms** on the selected mesh path, let Z be the
physical-coordinate view depth before projection, P the native projection affine,
and N its pre-bias native depth. Then:

```
s = stored Pzz
N = s * Z
biased native depth = N - b
physical-equivalent depth = (N - b) / s = Z - b/s
```

This is supported by the mesh's `Hnode * Hancestors * V * P` chain, **not borrowed
from the billboard inverse-projection conversion**. P is diagonal with zero
translation. Node/ancestor scale is already in H; do not multiply it again.
Pzz comes from the stored lens-vector Z scaled by the native binary64 literal0.3;
the lens builder supplies Z from1/camera-width. Actual execution arithmetic and
stored operands matter: an unconditional exact s=0.3/640 numerical substitution
for every retained/native/production table is **not** established.

Bias is the surrounding draw context's **f64 camera+770 =0x58D2B0**, not a polygon
or material field. The camera builder explicitly resets it to+0. On the ordinary
clean reset→first-primary-prefix path, with no intervening override and matching
source tree, the first `sand7` mesh therefore has **b=0, conditional DERIVED**.
This is **not observed repair4 memory**, a universal holder default, or an
exhaustive alias/hook absence claim. General holder callers supply the inherited
context bias, including explicit enclosing model/entity adjustments.

These relations answer the bounded input-contract question. Historical runtime
observation is not required to accept this **conditional** law; conversely the
law does not certify exact historical repair4 operands or instruction precision.
No implementation or new observation is authorized by accepting it.

## Byte-supported input chain

Own reads:`build/tmp/c3-scale-bias-pass/own-pe/`, matching all35 worker windows.
Authority remains`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.

- `45DA4D..45DA63` passes main+55C0 to`468E22`, ECX=`58CB40`.
  `468E39 c7807007000000000000` / `468E43 c7807407000000000000` explicitly zero
  both bias dwords. This is a reset instruction, **not inferred BSS initialization**.
- `47B0C3 dd05d0024b00`, `47B0C9 dcb028010000`, `47B0CF dd5dd8` load raw1.0,
  divide by f64 camera+128 and store qword. The constructed vector is copied to
  camera+138, with Z at+148 (`47B0E7..47B117`); `469400..469412` returns+138.
- `468F99..468FC7` builds/copies a diagonal lens matrix into context+8F8.
  `469220..4692C9` explicitly zeroes cross-axis entries. Immediate halves
  `468FC9 683333d33f` / `468FCE 6833333333` assemble raw binary64
  `333333333333d33f` =0.3. `4381F0..43829C` scales its rows; the camera builder
  explicitly zeroes translation at`46900B..46903C`.
- View construction`468EFF..468F96`, then affine composition`4690E3..46911E`,
  put V*P in context+898=`58D3D8`. The affine helpers, stack push/composition and
  actual XYZ transform were independently re-read. Native qword stores do **not**
  prove the production binary32 diagnostic's intermediate arithmetic.
- Established primary frame call`441879 ->437BA3 ->47496D` precedes tube call
  `441884`. Holder pushes`58D3D8`; root`46B6CC` pushes root+8 and visits children
  in source order. Mesh`46C55D` pushes mesh+8; `46C440 ->46BE80` copies the
  composed stack to mesh+148. `46C641` calls`40EAE0` on local XYZ and that matrix.
  `46C655 c6405800` clears intermediate+58 before polygon rendering.
- On projection cache miss, `46D3A9 b940cb5800` selects the same context;
  `46DE50 dca170070000` subtracts f64+770 **after XY ratios** and `46DE59 dd5a10`
  stores Z. Nonzero+58 instead retains the prior cached result. Fresh mesh cache
  reset is therefore part of the chain, not permission to ignore cache elsewhere.

All three selected records`63877/6B72B/7047C` belong to first primary node`596C7`
(`0/thenull/sand7`), root`59644`, as established in C3n and rechecked on the
selected raw LVL fields. No new census or actual native admission is claimed.

### Bias scopes are not all zero

Fresh original reads corroborate:

| Scope | Bias operation around draw | Limit |
|---|---|---|
|camera setup`468E39..468E4C`|two zero dwords|clean initial+0|
|tube`43A6C0..43A6CC` /`43A760..43A76C`|subtract/add literal0.0003 at`4B1460`|after first primary|
|eligible drawing`43FAA9..43FAB5` /`43FAFE..43FB0A`|add/subtract literal0.0012 at`4B15B8`|object+130 gate|
|model/entity`45186F..45189E`|add object+F8, call`47496D`, subtract object+F8|caller-supplied enclosing bias|

These are arithmetic inverse operations, not arbitrary-input bit-snapshot
restoration. Node+108's secondary-sort key bias is a different field/operation.
The worker's early displacement-only search missed direct global writes and is
not absence evidence; directed references and bytes supplied these scopes.

## Values and numerical boundary

| Operand/relation | Value | Provenance |
|---|---|---|
|native scale literal|0.3, binary64`0x1.3333333333333p-2`|READ immediates|
|ideal source reciprocal-width convention at640|0.00046875|DERIVED convention, not exact stored runtime scalar|
|retained A1 lens Z|0.0015625000232830644|historical raw camera+148; width640|
|literal × retained lens Z|0.0004687500069849193|binary64 display of operand product, not stored Pzz|
|retained A1 stored Pzz|0.0004687500186264515|historical raw`context_transforms_raw[0x58d438]`, diagonal+40|
|current production diagnostic Pzz|0.00046874998952262104|SOURCE Camera/C3bi inputs, not native memory|
|clean first-primary b|+0|conditional DERIVED reset/prefix path above|

Native epsilon conversion is`2e-6/s`, not2e-6 physical units. No new clipping
exercise or production epsilon selection ran. A single scalar does **not** make
the existing rounded C3n reference table equal current production doubles:
their input/rounding chains differ. Preserve that distinction rather than fitting
or widening a native tolerance. Exact PC/RC and operation-by-operation equality
remain unestablished; no claimed native error bound follows from the table.

## Facilitator check and narrowing

Independent PE parser freshly authenticated **35 raw windows** and semantically
re-read the camera/mesh/bias chain. Supplementary reads completed the vector-Z
store/return, affine accessors, and stack count/index/push-copy path where packet
windows stopped early; the stack uses96-byte records and copies24dwords on push.
A separate Fraction/4×4 homogeneous-matrix checker imports no worker arithmetic:
it reads selected local positions and ancestor matrices directly from LVL bytes,
authenticates the raw A1 operands, and proves depth factorization for all10
selected source corners under matched H,V,P. It independently reproduces the
scalar table. These checks establish coordinate algebra, not native FPU replay.

Own one-sign scratch contract mutation changes only`N-b` to`N+b`. For
Z=1234,s=3/6400,b=3/12800 it produces1234.5 instead of1233.5 and the independent
behavioral assertion fails. Exact restoration returns passing output and hash
`c050645550e444e1134972369cfa4f0dc1c5115d51dfc62d0a4337f1d6d3791c`.
No production file changed. Worker's initial JSON-key failure is retained; it
does not invalidate the later raw-operand check or become a native precision claim.

**Remaining unknowns, scoped:** actual repair4 native P,H,V,b and PC/RC for
bit/operation-equivalent historical replay; actual clip bounds/centers/cache and
pre/post-clip records; admission/route success and effective device state.
These do not retract the conditional source/default input law. Historical exact
reconstruction remains parked; any implementation needs fresh authorization.

Fresh full build and **44/44 CTest pass** (66.79s), framework **32/32 pass**.
Own independent fixed-reference integer/Pillow re-renders are fresh this pass:

| Gate | Fresh error / denominator | Verdict |
|---|---|---|
|C3 repair4, floor included|14,934,330 /235,008,000 =0.06354817708333334|**FAIL**,0.02|
|C1 zoom1 footprint|285,083 /17,721,990 =0.0160863988750699|PASS|
|C1 elev_down footprint|285,700 /17,721,990 =0.016121214378294987|PASS|

All three PNGs byte-match retained baselines. **No gate movement**, no
reference/mask/exclusion/camera/floor change. Preservation/resource counts and
actual verification/landing/end times are recorded in queue/archive and
`build/tmp/c3-scale-bias-pass/`. Prior unrelated dirty changes remain isolated.
Worker`ses_e85279121ffe0fAGRHus9gulmT` is independently confirmed succeeded/idle;
REA open=false. Zero starts/attaches/injection/scheduled-task creation/install
writes/packages/live resources; no live restoration owed. Sole attempt9, no
repair dispatch. Stop here under the fresh directive; no automatic follow-on.
