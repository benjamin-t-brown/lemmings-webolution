# A7b — lower-band error is stable; native composition remains unresolved

2026-10-20, `decompile`. Offline PNG analysis plus a packet-only repair. No
production/harness edit, stock or replacement-renderer launch, installation
write, new native measurement or gate correction. The worker used retained
images only; facilitator verification also ran the standard asset-backed test
suite. A7 remains open.

## Result

The A7a diagnostic remains **3031085/137520225 = 0.022041012512886742 > 0.02**.
Reference and mask identities are unchanged: durable A2 baseline and its
no-cell-routes contrast, not A1 original-0.

Fixed B is `[0,432,640,448)`. Fixed U is the union of A7a's four strongest
lower-band 16×16 cells: x origins 288, 304, 320, 336 at y432. These cells are
adjacent and exactly tile `[288,432,352,448)`; U is within B. Never sum them.

| Region | Mask N | Candidate error E | 765N | Share of global E | 50E−765N |
| --- | ---: | ---: | ---: | ---: | ---: |
| B | 3,491 | 302,804 | 2,670,615 | 9.98995409234647% | 12,469,585 |
| U | 922 | 200,664 | 705,330 | 6.620203656446454% | 9,327,870 |

If **all** masked B error were removed with every other error unchanged, the
best-case score would be **2728281/137520225 = 0.019839125481361013**. This is
the upper bound on error-removal potential expressed as a best-case residual
score, **not** an achieved correction, attainable native fix, changed mask,
new gate or A7 pass. The other eight view masks and a second-level pair remain
unidentified. This one-view arithmetic cannot qualify the milestone.

## Stability and suppression measurements

Both restorations equal baseline in B and U: **0** changed pixels and **0** RGB
error, null difference bbox. This rules out variability in these particular
restoration pairs as the source of this lower-band contrast, not variability
in every capture.

For both no-cell-routes and no-walker, relative to baseline:

| Region | Changed pixels | RGB error numerator | Difference bbox | Changed ∩ original mask |
| --- | ---: | ---: | --- | ---: |
| B | 3,491 | 586,625 | `[169,432,470,448)` | 3,491 |
| U | 922 | 80,826 | `[288,432,352,448)` | 922 |

The two suppression images themselves are identical within B/U. These broad
contrasts cannot isolate an exact draw, texture, filter, raster convention,
omitted layer or interaction. They do not establish that a simple tube-only
fix exists.

Full-frame restorations are **not** identical to baseline. Both have **823**
changed pixels in `[303,235,339,277)`, but their error numerators differ:
**11,380** for restored-no-cell-routes and **49,411** for restored-no-walker.
Do not infer equal RGB error from a shared changed-pixel count or bbox.

## Visual lead, not native ownership

`lower-band-panels.png` and `U-context-nearest4.png` show the caption **Paused**
in the baseline, suppression and restoration images, whereas the tube-only
candidate has no caption. That observation suggests pause-caption composition
as a native-trace lead. It does **not** establish which caption/background
pixels contribute to the fixed mask, the native coverage/alpha rule, resource
identity, draw order or how much error a correct caption would remove. Exact
baseline/suppression-equal pixels are outside the gate's mask. Do not paste the
original caption onto production or exclude this band from acceptance.

## Facilitator check and repair

One targeted worker session, `ses_ea36317c9ffeSRKYMjjoitFgo1`, completed initial
analysis and a same-session offline repair. Required OpenCode model/high routing
was checked through `debug agent`; local assistant metadata records the expected
provider/model (mapping remains in `docs/ROLES.md`).

The facilitator independently used ImageChops channel maxima/masked histograms,
an explicit four-cell union mask and original-image differences rather than the
worker's scalar loops. `build/tmp/a7b-facilitator/verify.py` checks all six PNG
hashes/dimensions, global and regional integers, all **15** baseline contrasts,
both suppression-pair contrasts, bboxes, unchanged real metrics after repair
and the best-case arithmetic. All pass.

Verification caught two packet defects: the initial synthetic claimed an
unmasked error765 but supplied255 and had an out-of-region error0, so it did
not test region exclusion; and prose copied full-frame E11380 onto both
restorations. The repair preserved the initial packet under `pre-repair/`,
corrected the prose and contiguous-union wording, and tests the actual
`candidate_region()` helper with selected masked error7, selected unmasked
error765 and masked out-of-region error11. Whole-mask E18/N2 and selected E7/N1
pass. The facilitator separately executes isolated **in-memory** mask-bypass
and region-bypass mutations; each raises the intended synthetic assertion.
Analyzer bytes remain unchanged, SHA-256
`00c0fab4b93d1f89e2370eaccf11c9545e1dc6a982905b643b668354bf2ace49`.
No production mutation was appropriate for this image-measurement-only task.

Full build reports no work; fresh **24/24 CTest** pass. No new native address
claim required a byte reread. Existing A2 authority and limits are preserved,
not promoted into fresh native evidence. All prior retractions/failures stand.
Independent final cleanup and diff/lint checks are recorded in the queue.
No commit was authorized or made.

## Artifacts, reproduction and cap

- Worker: `build/tmp/a7b-lower-band-characterize/round-note-repair1.md`, corrected
  `round-note.md`, `results.json`, `analyze.py`, input identities, annotated PNGs,
  preserved pre-repair packet and `synthetic-mutation-results.json`.
- Independent: `build/tmp/a7b-facilitator/verify.py` and `verification.json`.
- Rerun: `python build/tmp/a7b-facilitator/verify.py` (normal Windows Python/Pillow).

This reaches the localization-family **three-dispatch cap**: A7a, A7b and the
packet repair. No fourth pixel-only scoring/crop round. The unresolved cause is
parked in Plan Phase E with resume outline
`build/tmp/a7-lower-band-native-resume-brief.md`. A continuation needs new native
ownership evidence or an explicit fourth-round gate-based justification for a
bounded native pause-caption trace. It must not silently reset the count. No
live allowance is renewed.
