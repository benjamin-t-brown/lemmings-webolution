# A7a — current wider-mask failure reproduced and localized

2026-10-20, `decompile`. Offline measurement only: no production change,
stock-game start, debugger or installation write. A7 remains open.

## Result and scope

The current production **tube-only** route reproduces A4d's initial-view wider
diagnostic, not a new first measurement:

| Candidate | Absolute RGB error numerator | Denominator | Normalized MAE |
| --- | ---: | ---: | ---: |
| Current production route | 3,031,085 | 137,520,225 | 0.022041012512886742 |
| Retained A2 default | 3,029,686 | 137,520,225 | 0.022030839463795235 |

Both fail the unchanged **0.02** threshold. The mask consists of the **179,765**
pixels where A2's durable `baseline.png` differs from `no-cell-routes.png`.
The error reference is that durable baseline, **not** the separately retained
A1 `original-0.png`. The latter remains the fixed-ROI/cavity/rim reference,
as in the [A2 scorer](../../tools/harness/verify_tube_a2.py).

Production PNG SHA-256:
`66a8ea3b44df2216b64debf9d8f417d2ed923841fc440e99c028b013010d3651`.
Explicit `--no-injected-walkers` avoids the default injected actors; this is not
a current actors-on or A6 acceptance check. Fixed ROI remains
`708159 / 45441000 = 0.015584142074338152`, passing.

## Predeclared disjoint localization

The worker declared a 16×16 grid anchored at image origin before scoring.
All 1,200 cells partition the image; only mask pixels contribute. For cell
error numerator `E` and mask pixel count `N`, signed excess is
`50*E - 765*N`. Negative cells offset positive cells in the global score.
The sum of positive cell excess is a distinct localization diagnostic,
**not** the global excess or a different acceptance gate.

- Global signed excess: **14,034,025**.
- Positive-cell excess: **42,953,165** across **252** cells.
- Top-five positive excess: **11,139,155**, or **25.9332577704111%**.
- The predeclared top-five-below-half falsifier is triggered. Excess is not
  concentrated in five cells under this partition; this says nothing about
  concentration in larger connected regions.

| Cell origin (x,y) | Mask pixels | Error numerator | Signed excess |
| --- | ---: | ---: | ---: |
| (336,432) | 232 | 56,363 | 2,640,670 |
| (304,432) | 222 | 49,829 | 2,321,620 |
| (288,432) | 229 | 47,319 | 2,190,765 |
| (320,432) | 239 | 47,153 | 2,174,815 |
| (320,224) | 171 | 38,842 | 1,811,285 |

Four strongest cells occupy the lower y=432 band, not the predicted cavity/rim.
No native draw, model, texture, HUD or sampling cause has been identified.
Separate cavity/rim rectangle diagnostics remain failed at
`0.030728457308849464` / `0.40029564553093966`; overlapping rectangles are
not added to the partition totals. The [A2 upper-band observation](phase-a2-tube.md)
and its limits stand.

## Gate evidence availability

The worker inventory identifies a scoreable suppression pair only for initial.
The other eight retained views and a second level have **no identified matching
suppression pair** in the checked A2/A4d evidence. Candidate renders for all
nine views do exist. The facilitator's filename search also found earlier
`build/tmp/a2-tube/live/no-cell-routes.png` and a durable restoration image;
these are not additional camera/level gate fixtures. This is a bounded retained-
evidence inventory, not a proof that an arbitrarily named mask cannot exist
elsewhere. Never reuse the initial-view mask on a different camera as acceptance.

A7's nine-view-plus-second-level gate is therefore not fully measurable from
the identified retained masks. No live authorization is renewed by this round.

## Facilitator check

Dispatched the configured OpenCode targeted worker; Task session
`ses_ea370bb13ffe4K7dJYPfV74fmd` completed. `opencode debug agent round-agent`
resolved the required model and `high` variant; the local session message
metadata also records the expected provider/model (see `docs/ROLES.md`).

Independently ran the full build (no work needed), all **24/24 CTest** tests,
and a separate production render. Its PNG is byte-identical to the worker's.
`build/tmp/a7a-facilitator/verify.py` uses ImageChops masked histograms rather
than the worker's scalar pixel loops: it independently reproduces both complete
score numerators, every one of the 1,200 cell rows, ranking and excess sums,
cavity/rim integers and fixed ROI. It checks baseline/suppression hashes against
the retained isolation log and a synthetic masked one-channel delta of 7,
excluding an unmasked 765 error. No production mutation was warranted in this
measurement-only round. No new native address claim was made.

Narrowed the packet's mask-absence wording to identified evidence and separated
top-five-grid concentration from connected-region or causal conclusions.
All earlier failures/retractions remain. Independent final process/task check:
**0** stock/replacement processes, **0** LemWeb scheduled tasks. No source or
framework edits; no commit authorized or made.

## Reproduction and next bounded question

Worker packet: `build/tmp/a7a-wider-mask-localize/round-note.md`, `score.json`,
`regions.json`, `mask-inventory.json`, `prediction.md`, `commands.txt` and
`score_a7a.py`. Independent packet: `build/tmp/a7a-facilitator/`.

```text
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/Invoke-Ucrt64.ps1 "cmake --build --preset dev-debug"
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/Invoke-Ucrt64.ps1 "ctest --preset dev-debug"
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/Invoke-Ucrt64.ps1 "build/cmake/dev-debug/webolution.exe --level MM_CLIMB_002 --native --camera tools/harness/fixtures/camera_a1/camera-fixture-params.json --geom route --no-injected-walkers --shot build/tmp/a7a-facilitator/production-initial.png"
python build/tmp/a7a-facilitator/verify.py
```

This was question round **1**. Next useful offline question: characterize the
lower-band residual against the existing baseline/suppressed/restored images
before choosing an implementation or requesting new stock captures. Do not
infer that it is absent scenery merely because the milestone is named scene
completeness. Remaining gate-mask acquisition needs a separately scoped and
authorized live brief; current start allowance remains zero.
