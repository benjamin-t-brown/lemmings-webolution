# C3p — kind-5 predicate, retained-camera derivation

2026-10-25. Offline READ + DERIVED; no production change or pixel acceptance.
Worker packet: `build/tmp/c3p-kind5-culling/round-note.md`; reproduce with
`python -B build/tmp/c3p-kind5-culling/run.py` (normal Windows Python + Capstone).

## Accepted result

For each kind-5 record, the branch is `dot(E - first_vertex, selected_vector) > 0`.
The leading record u32 selects the vector in node+0x128; the first shared element
selects the vertex in node+0x118. Negative, zero and unordered skip the emitter.
This is not a winding test and not a promise of final visible pixels.

READ anchors, re-read against stock SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`:

- `0x418350` multiplies three components and adds left-to-right (`DE C1`).
- `0x46AFB3..AFCC` obtains the first vertex and subtracts it from node+0x1A8;
  `0x46AFDD..AFF5` selects the vector and calls the scalar helper.
- `0x46AFFA`: `DC 1D 38 03 4B 00 DF E0 F6 C4 41 75 0B` compares to
  raw-backed zero, masks C0/C3, and skips. The near route at `0x46B050` has
  the same inputs and test (`0x46B0AA..B0B5`). Emitters differ and remain outside
  this acceptance. Tables `0x4C2210[5]` / `0x4C2230[5]` select the two routes.
- `0x46C49E..C508` composes node matrix M with S at `0x58D498`, inverts its
  rotation and transforms its negated translation: E = -t * inverse(R).
  Accessors `0x412C10/20`, inverse `0x46C750`, negate `0x412760`, and vector
  multiply `0x40EB50` were checked. This is row-vector arithmetic.
- S is the camera builder's +0x958 block, not file-backed runtime data.
  `0x46903E..4690E1` constructs its diagonal from camera+0x150, scales each
  row by double bits `400AAAAAAAAAAAAB`, and explicitly zeroes translation.
  Retained camera values are inputs, not a new live observation or FPU-mode read.

For the four parsed C3l trees at the accepted reference camera:

| Node | Records | Predicate pass | Predicate skip |
| --- | ---: | ---: | ---: |
| thenull/sand7 | 329 | 240 | 89 |
| thenull/base | 64 | 32 | 32 |
| thenull/top | 64 | 28 | 36 |
| Top/cyl2 | 8 | 6 | 2 |
| Total | 465 | 306 | 159 |

Full records/offsets and source fields are in `table.csv`, `tables.json`,
`inputs.json`, and `node-inputs.json` in the packet. Unread trailers, runtime
visibility bytes, clipping, binding and device state are not settled here.
Exact live scalar magnitudes and FPU control word are not accepted.

## Facilitator check

`python -B build/tmp/c3p-facilitator-check.py` independently parses the LVL
bytes, composes the matrices, and solves E by pivoted Gaussian elimination
(not the worker's cofactor evaluator). It reproduces all 465 decisions and the
four tree ends `9176E/98D41/9DB2B/AAA43`; maximum scalar difference against the
worker's precision-53 table is 5.684341886080802e-14. The check authenticates
every listed instruction against the PE across all 35 windows, plus constants
and dispatch-table entries, and saves fresh disassembly in
`build/tmp/c3p-facilitator-check/`. Manifest `sha256` fields hash assembly text,
not raw windows; the independent comparison explicitly accounts for that.

With `--mutated`, it parses the worker's private LVL copy and confirms only
three normal-0 sign bytes differ (`5EE52/5EE5A/5EE62`). Only record `634B7`
changes pass to skip, giving 305/160. Re-running the untouched source restores
306/159 and its original SHA-256 `95f84f86fb53ab29ce2ce3aed794cf69a06e5754d42ef78657f3af1a5db4c567`.
No original file or live memory was mutated. Full build/CTest: **34/34**.
Independent process check finds no Lemmings process; zero starts/attaches.

Narrowing: this accepts conditional retained-camera predicate decisions, not
final submission counts, exact native precision, or the worker's broader
precision-sweep headline as an independently reproduced measurement.

## Gate and next work

C3 remains unimplemented and above 0.02. Correct-reference fresh baseline is
`48188934/235008000 = 0.2050523131127451`, independently rendered/scored by
`build/tmp/c3-facilitator-baseline.py`. `score_c3_regions.py` currently hardcodes
A6d3, not repair4; C3a's historical numbers remain diagnostics for A6d3.
The next scoring implementation must explicitly use repair4 original hash
`39ba74328224a44ab1ae392790dd191303eb399492162d164c104a577fb99053` with its camera
and actor table. No gate change is needed. C3n mesh fields and C3o repaired
type-2 slot pairing remain in flight; device state and scene implementation follow.

Dispatch 13:37:26; worker finished 13:54:46; accepted 14:00:25 MDT
(22m59s dispatch-to-acceptance). No stock start.
