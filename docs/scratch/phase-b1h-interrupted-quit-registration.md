# B1h returned incomplete; standing run stopped by user

2026-10-22. Queue `b1h-atlas-independent-match`. Original OpenAI dispatch ended on
quota; Mac continuation `ses_e9a8c2165ffeq0o5CzvhVt4g1o` was interrupted at the user's
request at about16:17 after approximately65 minutes against a35-minute budget.
**Over budget by about30 minutes. No final worker report. No B1 pass.**
Read-only session API confirms outcome interrupted/idle, last message aborted, and
only this facilitator active locally. The old mouv session is also failed/idle.
No new worker, repair, phase or milestone will be dispatched under this standing run.

## What is retained, and what is not accepted

- Tracked matcher/test edits are retained as an **experimental diagnostic**, not a
  renderer-ready glyph-layout generator. Exact snapshots are under
  `build/tmp/b1h-atlas-independent-match/mac-snapshots/`. Matcher SHA-256
  `7186347bac8e196729f77e8504c7f45e29bee017d68faef15ce345a399217b4d`, test SHA-256
  `bcecd5a4e277f8f5d7cfd3f23264d6e461dc21298ace00314f8991bcc1dfa83a`.
- Original `round-note.md` describes only pre-quota segmentation. It is **not** a
  final report of the Mac work. `continuation-prediction.md`, ten `scored*` directories,
  worker-authored `independent-candidate-check.json`/helper scripts and all previous
  results are preserved. That worker-authored “independent” check does not substitute
  for facilitator verification.
  The shared source snapshot was overwritten during successive trials; saved-pixel
  authentication does not claim every historical search can be rerun byte-for-byte
  from the final optimizer version. Per-trial source provenance is incomplete.
- Latest saved run is `scored-grouped/registration.json`, digest
  `dffe26cb11ae64d7465a698694006c9873c2be8ad17dbcf0e243eed790638f27`.
  It was completed by an already-launched SSH process after the interruption (saved
  artifacts around16:18:55). It is retained, not treated as authorization for more work.
- There are actual source-crop renders and pixel controls now, unlike the original
  segmentation-only pass. Nevertheless, the selected crops are not a verified Q/u/i/t
  identity mapping. The saved images show incorrect/incoherent letters. Distinct source
  indices do not ensure distinct glyph shapes or semantic correctness.
- Final code partitions the three target clusters as2/1/1 and splits the first at its
  midpoint. This constrains target placement without proving the true character boundary.
  Several family outputs overlap neighboring destination boxes. Actual search beam/prune
  limit is96, while emitted prose says160; main uses connected supports, while emitted
  crop-method prose says bounded unions. Search completeness and metadata precision
  cannot be accepted. Six tests exercise helpers/rendering, **not register_family**.
- `_FONT`/`_FONTFE` still tie under black tint because their alpha matches; their RGB
  pixels differ. The data does not reject the retail font families, establish a native
  charmap, or supply all five menu labels/layout. No production menu or B1 full-frame
  render/score was produced. Earlier text-free passing scalars remain explicitly rejected.

## Facilitator verification (saved outputs only, no new search)

Separate checker `build/tmp/b1h-atlas-independent-match/facilitator-saved-output-check.py`
imports **no worker code**. On mouv it authenticates reference/source hashes, scores all
ten saved registration runs against the retained reference Quit ROI `[282,438,359,463)`,
and independently reconstructs saved black-tint pixels directly from original exported
retail RGBA crops, one physical resize and alpha composition. Latest saved renders
reproduce byte-exact RGB pixels and all recorded scale/placement-control numerators.
Verification output: `facilitator-saved-output-verification.json`.

Latest whole-row RGB absolute-error denominator is77*25*3*255 = **1,472,625**:

| Source family | Latest saved numerator | Normalized row error |
| --- | ---: | ---: |
| ^FONT | 31,554 | 0.0214270 |
| ^FONTH | 39,916 | 0.0271053 |
| ^FONTP | 31,723 | 0.0215418 |
| _FONT / _FONTFE | 29,712 | 0.0201762 |
| blank row | 80,390 | 0.0545896 |

These are **row diagnostics**, not B1's640x480 denominator235,008,000 or its gate.
They improve the blank-row scalar but leave substantial residual and wrong letters.
The user's “leaves the blank-row residual” is therefore narrowed: residual remains,
not numerical equality to the blank baseline.

Own isolated control: shift only the first saved `_FONT` placement1px, source crops
unchanged. Numerator29,712 ->36,462; restoring the saved placement reproduces baseline
pixels exactly. Saved scale1 ->1.125 control independently gives61,322. This proves
pixel sensitivity, **not semantic glyph correctness**. Direct montage
`facilitator-montage.png` shows reference first, then all five latest family attempts.
No optimizer/search was rerun, no matching repair or new font hypothesis tried.

Reproduction of the independent check and saved tests:

```text
python build\tmp\b1h-atlas-independent-match\facilitator-saved-output-check.py
python -m unittest discover -s build\tmp\b1h-atlas-independent-match\mac-snapshots -p test_match_b1_menu_glyphs.py -v
```

Local tracked scripts byte-equal the tested snapshots; all6 Pillow tests pass. Fresh
authoritative mouv wrapper build/CTest: **29/29**,62.79s, no production changes. Separate
cleanup JSON confirms no Python/game process or LemWeb task, and exe/config/save hashes
equal prior baselines. No live starts/REA/install writes; no debugger to detach. Only
Mac-created AppleDouble sidecars/temporary cleanupPS1 removed; evidence retained.

## Stop and recommendation

User explicitly ends this standing run after B1h. Task returned incomplete, blocked by
that stop; no further dispatch and no milestone advancement. B1h consumed2 dispatches
(quota-interrupted original and user-interrupted continuation); same glyph question now
totals5 including B1f3. Current standing-run completed/returned dispatch count16; original
starts4 in the run, **zero** for B1h.

Recommended action on a future **explicitly authorized** resumption: review the saved
source crops/target boundaries and define one semantically discriminating glyph test
before further optimization. Do not accept arbitrary shape combinations merely because
they improve L1 or perturbation controls pass. Reuse existing retail exports/reference,
keep the full-frame gate unchanged, and defer the production menu renderer until actual
character/layout evidence exists. Do not repeat the failed native menu-vtable search or
automatically launch another broad glyph round. No follow-up is queued or dispatched now.
