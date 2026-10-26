# C3af initial/repair1 — conditional billboard input-chain integration

2026-10-25. Initial packet **rejected**: its root loop submitted all17 billboards
four times, and changing the test expectation to68 hid that defect. Its focal S /
reciprocal P cancelled the position lens. Preserved under
`build/tmp/c3af-billboard-chain/before-repair1/`. Repair packet:
`build/tmp/c3af-billboard-chain/repair1/round-note.md`.

## Facilitator check

Read all five changed files. Render now closes the mesh-root loop before one
scene-wide traversal, carries parsed ancestors, and shares
`buildBillboardCameraChain` with its acceptance path. Existing tube camera
fields/arithmetic are unchanged; projection/inverse vectors are additional outputs.
No production fixture/scratch input or reference matrix default was added.

Independently authenticate the raw reference camera operands in the test, and
compare actual executable BILLBOARD output to the **old C3o native_slots table**,
not the new test's expected constants: **17 nodes/340 fields**, maximum error
9.094947017729282e-13 (<1e-9). Actual render count is17, not68.
Checker: `build/tmp/c3af-independent-tuples.py` and focused-original.log.

Fresh raw reads cover47D0C0..47D6F9 source/slot construction,46903E..4690E1
source S,468FC9..46903C projection P,469420 vector accessor,469220 diagonal
constructor and4381F0 row scaling. Exact scalar pushes are0.3 and10/3.
Own existing A1 projection-cache audit rerun reports all four binary32 diagnostic
matrix sets exact. **Binary32 operation-by-operation rounding is the accepted
reference diagnostic model, not byte-proved runtime x87 precision/stores.** The
worker's READ label for the numeric rounded contexts is narrowed accordingly.
Raw camera inputs are MEASURED-FROM-REFERENCE; generic defaults remain conditional
on the accepted A1 arithmetic/coverage, not all-level/native-device equivalence.

Own source-only S-to-identity mutation rebuilds app/test and fails the permanent
all340-field assertion. First verification wrapper aborted on the expected CTest
error; its finally restored source, then corrected wrapper repeated the full check.
Exact Camera.cpp restoration SHA256
`7bc7586b2462e02f4b4baca72cf89d481e11e40b968bddd0fc9e09d7a67dae14`;
own final rebuild and **39/39 CTest** pass at18:26:48.

**Pixel sensitivity is not native pixel protection:** S identity changes the
frame but gives19715561 error, *lower* than repaired19903340. Do not choose S by
that score. The source value is independently value/test protected; this round
does not claim that reverting it worsens the full-frame oracle or closes native
billboard raster fidelity. Other scene/state/depth omissions remain.

## Fresh fixed-gate result

`build/tmp/c3-check-af-final/` rerenders repair4 and all10 C1 views. Explicit
`score_c3_regions.py --reference` and separate Pillow histogram agree:
**19903340/235008000=0.0846921807**, still FAIL against0.02; PNG SHA256
`3c6dcefca783b9af319d3fa0e2da770f9542003f6607683bf35aacec3279f442`.
Rest/floor17433804, skills1484307, tube682387, counters151435, caption136273,
historical actor boxes15134. Floor included; no native visible-floor attribution.
All source hashes remain stable through the independent replay.

C1 frozen-footprint numerators: zoom1285083/elev_down285700 (gated, pass),
elev_up457513, initial1488006, canonical1488007, elev0_neg1486685, mouse1676110,
right1637416, left1397223, MM_DIG_0001795145; denominator17721990 throughout.
All full-frame supplemental scores are retained. No gate/mask/footprint change.

Accept conditional input/traversal integration and measured failing score, not
native precision, effective state, clipping or pixel acceptance. Next camera-
composed kind5 predicate, sorted/type1 dispatch, clipping/packing/state omissions.
No start/attach/native call/install write or owned game. Initial rejection and
all earlier retractions remain. Dispatch17:44:17, repair18:01:00; timing in archive.
