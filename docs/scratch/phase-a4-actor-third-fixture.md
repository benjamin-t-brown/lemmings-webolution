# A4z: third half-scale fixture passes; matched-camera full-scale fixture fails

Queue `a4z-actor-third-fixture`. Worker session
`ses_ea5f634d4ffekymdW1TlwgwdEx`, one owned session-1 game start, followed by an
offline-only same-session repair. Candidate packet:
`build/tmp/a4z-actor-third-fixture/round-note.md`. No production source changed.

## Verdict and fixed criterion

The different-scale acceptance is **not met**. The new half-scale sample supports
the existing bounded result, but the full-scale sample fails after correcting
the candidate camera. Do not call the whole round a pixel pass, or infer that
the integer-centre convention alone is the cause of failure.

The preregistered criterion remains zero native-only and candidate-only pixels,
with support MAE <= `0.015155820782419248`. Support MAE sums RGB absolute error
on native-versus-background support and divides by support count * 765;
candidate-only error is separately retained. The old absent-actor gate is not
a substitute for this stricter criterion.

| case / crop | native-only count/error | candidate-only count/error | both count/error | support MAE |
| --- | --- | --- | --- | --- |
| third, half scale, `[365,278,429,342]` | 0 / 0 | 0 / 0 | 142 / 150 | 0.0013808340237503453 |
| zoom1, matched camera, original clipped `[473,370,537,434]` | 0 / 0 | 27 / 4267 | 515 / 11095 | 0.028161685386128562 |
| zoom1, matched camera, expanded `[488,362,527,407]` | 0 / 0 | 27 / 4267 | 531 / 11558 | 0.028452912866339254 |

The expanded crop comes from floor(left/top)-4 and ceil(right/bottom)+4 using
the recorded vertices. The worker prediction's literal expanded bounds were
off by one, but its stated formula gives the executed crop above. Both original
and expanded scores fail, so this discrepancy cannot convert failure to pass.
Native versus restored images agree exactly in all scored crops; full frames
do not, because other regions changed. No full-frame restoration is claimed.

## Fixture identity and measured scale

The wrapper selects actor `0x27D3150` through the actor-list accessor, not by
matching a free billboard. It temporarily changes that walker's graphics to
action 0 (`bash`), temporal 0, and the live chooser settles on direction 2,
mirror 1. This is a controlled graphics fixture, not evidence that the walker
performed a bash skill. Both pairs use that same actor and record.

Raw direction-record bytes decode to offsets `(16,1)`, source width/height
`30 x 36`. Native vertex bytes independently decode to:

| case | left | top | right | bottom | measured width/height scale |
| --- | --- | --- | --- | --- | --- |
| third | 390.6588439941406 | 291.763671875 | 405.6588439941406 | 309.763671875 | 0.5 / 0.5 |
| zoom1 | 492.0828857421875 | 366.29913330078125 | 522.0828857421875 | 402.29913330078125 | 1.0 / 1.0 |

Fractional edges differ from the original A4f samples. Scale is proved by
emitted size divided by source size, not by the SPACE key or a between-draw
global alone. Both native pairs are retained under `live-run1/`.

## Camera repair and limits

The original scorer used stock actorfixture's fixed unzoomed camera for both
pairs. Its zoom1 result has no crop overlap (515 native-only, support MAE
0.1900856653340948) and is retained as a mismatched-input diagnostic, not the
matched-camera verdict.

Repair1 builds a scratch copy of actorfixture with explicit camera parameters,
linking unchanged production Camera/ActorSprite and asset functions. It reuses
the accepted A1 `params-zoom1.json` (`zoom_shift=1`, `elevation=18.5`), not a
screenshot-fitted offset. This is an A1 source-derived parameter replay, not a
new measurement of every camera operand. The freshly rebuilt adapter reproduces
the worker overlay pixel-for-pixel. Predicted native corner errors in binary32
ULPs are **[2,1,1,1]**, correcting the worker's “~1 ULP” summary.

Do not promote the worker's failed raw-matrix replay as proof that `0x4C6B88`
is stale: its interpretation/scaling was not established by that failure. Nor
do close corners exclude every placement/precision effect or prove a specific
sampling cause. These remain separate unknowns.

## Original bytes (independently reread)

Authority SHA-256 matches LESSONS. `build/tmp/a4z-facilitator/bytes.py` records
file-backed pins and full windows in `byte-pins.json` / `bytes.txt`:

| address | bytes | bounded read |
| --- | --- | --- |
| `0x42F076` | `c70538cb580001000000` | set scale operand 0x58CB38 to 1 around this actor draw branch |
| `0x42F088` | `8a918c030000` | actor+0x38C selects the scaled/suppression branch |
| `0x42F0DF` | `c70538cb580000000000` | clear operand after those branches |
| `0x441862..0x44186B` | `8b15b0bf4c008b4210a33cd55800` | control+0x10 copied to 0x58D53C |
| `0x4419A0` | `c7053cd5580000000000` | clear that global later |
| `0x47A7CD..0x47A7D2` | `a13cd558002b0538cb5800` | draw scale delta subtracts the two operands |
| `0x47A766..0x47A76F` | `833d3cd5580000750c6a00` | **nonzero skips** pre-draw argument-0 filter call |
| `0x47A776` | `e8ae67ffff` | call 0x470F29 on the zero arm |
| `0x470F39` / `0x470F65` | `c705e0244c00ef000000` / `c705e0244c00a0000000` | reference EF first, A0 on conditional LINEAR path |

This **narrows A4x/A4y's scope**: POINT/EF was established for the zero-zoom
pre-call path, not every zoomed actor. The nonzero path skips that assignment;
the last preceding filter state, material-cache effects and effective zoom1
device state are not established here. Filtering is a concrete next lead,
not a measured explanation of the residual. No blanket “only scale control”
or exhaustive-computed-writer claim is accepted.

## Facilitator check (2026-10-20)

- Separate `build/tmp/a4z-facilitator/verify.py` independently recomputes native
  vertex/record size, PNG digests, support masks, category counts/errors, MAEs,
  restored-crop equality, source/install hashes and corner ULPs. Results:
  `independent.json`. It does not import the worker scorer/decomposer.
- Own comparison-only raster matches every repaired overlay byte before
  changing one term: UV +0.5 alone worsens full-scale support MAE to
  `0.05560355969129648`; coverage -0.5 alone to `0.029454845340521646`.
  At half scale UV alone worsens to `0.08483844241922121`; coverage alone
  survives unchanged. These are diagnostic perturbations, not production fixes.
- Fresh scratch-adapter compile, separate command replay and overlay comparison
  pass (`replay.py`, `fresh-replay.json`, `zoom1-fresh.png`). Own third scorer
  rerun reproduces the half-scale result in `third-fresh/`.
- Whole build check reports no work required; **20/20 CTest pass** via
  `scripts/Invoke-Ucrt64.ps1 "cmake --build --preset dev-debug && ctest --preset dev-debug"`.
  No new unit-test/source behavior or all-scale acceptance follows.

## Live restoration and remaining accounting gap

Sixteen top-level write events include restores and canonical-backup writes;
their wrapper checks actual reads after each write and records final per-address
restore booleans. The separate `enter_fast` level-slot write is **not** in that
ledger. Its log reports `slot_restored: true`, but original/temporary/restored
slot bytes were not retained. The self-copy explanation is not independent
byte evidence. That historical accounting gap cannot be repaired after process
termination; do not report a fully byte-audited live ledger.

The process was owned and stopped; facilitator's separate fresh PowerShell
check finds zero Lemmings/webolution processes and zero LemWeb tasks. Original
exe, save and config digests equal the retained before/after digests. All nine
source/harness hashes independently match; pre-existing dirty work is preserved.
No new live operation occurred during repair or facilitator verification.

## Next

Land this as a bounded negative at full scale. Before changing raster behavior,
trace the zoom1 actor's encoded filter/alpha state and explicit inherited-state
unknowns in one static-only round. Retain A2/A3's broader failures and A5m's
unwired app/depth/pose/time gaps. No effective-device or whole-scene acceptance.
