# A6d24: retained matched-pause full-frame gate fails

2026-10-20. Offline score audit, no production edits or stock-game operation.
OpenCode targeted `round-agent` with configured Luna/high per [roles](../ROLES.md),
Task `ses_ea4699bbaffe6tExXNcex61qMy`; actual execution model is not independently
exposed by Task return. Brief `build/tmp/a6d24-full-frame-gate-brief.md`; packet
`build/tmp/a6d24-full-frame-gate/round-note.md`.

## Fixed gate and result

The [A6 gate](../../PLAN.md) applies the A4d normalized RGB MAE formula to the
**entire 640x480 frame**, not its passing tube ROI:

`sum(abs(referenceRGB-candidateRGB)) / (255 * 3 * 640 * 480)`.

No crop, mask, alignment, resize, alpha contribution or HUD exclusion. The
denominator is **235,008,000**. The <=0.02 criterion permits an integer numerator
of at most **4,700,160**, fixed before scoring.

| Candidate | Integer RGB error numerator | Normalized display | Clause 1 |
| --- | ---: | ---: | --- |
| Current actors-on | 49,881,782 | 0.21225567640250545 | **FAIL** |
| Tube-only diagnostic | 49,888,588 | 0.21228463711873638 | **FAIL** |

Exact decisions use rational/integer comparisons, not rounded displays. Current
actors-on improves the global numerator by 6,806 relative to tube-only. This is
only their measured difference; no named cause or region of the remaining error
was established. In particular, do not claim HUD, sky, objects or scenery account
for a measured proportion without a separate measurement.

The [A6d3](phase-a6-matched-state-bbox.md) half-open placement boxes remain fixed:

| Actor | Box xyxy | Tube-only error / denominator | Actors-on error / denominator | Improves? |
| --- | --- | ---: | ---: | --- |
| 0 | `[467,282,480,300)` | 20,268 / 179,010 | 14,165 / 179,010 | yes |
| 1 | `[397,292,408,310)` | 16,001 / 151,470 | 13,546 / 151,470 | yes |
| 2 | `[315,283,326,301)` | 15,283 / 151,470 | 17,035 / 151,470 | **no** |

Actor 2 still worsens **0.10089786756453423 -> 0.11246451442529874**.
**Both clauses fail for this retained candidate comparison. A6 is not accepted.**
The actor boxes are the original overlay-derived placement boxes, not newly
measured native controller rectangles.

## Input identities and reproduction

Reference: `build/tmp/a6d3-matched-state-bbox/original-live.png`, SHA-256
`603f05f6b5da51dca6e0372db8885a486a5b53aa658a3ec4a31acee0296d0e03`.
Camera parameters: same directory `live-camera-params.json`, SHA-256
`c7389849bb0b048da4199b7694a37f2ae5d0010ab10b6831c7aedbf8d95a44fd`.
Production executable hash at scoring:
`a94206259be18968073a13a6752d675477bf81926c51eee15bdca5788a52fa25`.

Fresh actors-on PNG:
`c4e6cf477e7f803d92329e5b223fe296b014801abb0552132d65133fb1a09ebf`.
Fresh tube-only PNG:
`66a8ea3b44df2216b64debf9d8f417d2ed923841fc440e99c028b013010d3651`.
Both **byte-match** the same-mode A6d3 renderer PNGs. This is not a new renderer
regression; it is a newly scored global failure of the same retained outputs.

```powershell
$env:WEBOLUTION_GAME_DIR = 'C:\progs\Lemmings Revolution'
.\scripts\Invoke-Ucrt64.ps1 "build/cmake/dev-debug/webolution.exe --native --geom route --level MM_CLIMB_002 --camera build/tmp/a6d3-matched-state-bbox/live-camera-params.json --shot build/tmp/a6d24-full-frame-gate/actors-on.png"
.\scripts\Invoke-Ucrt64.ps1 "build/cmake/dev-debug/webolution.exe --native --geom route --level MM_CLIMB_002 --camera build/tmp/a6d3-matched-state-bbox/live-camera-params.json --shot build/tmp/a6d24-full-frame-gate/actors-off.png --no-injected-walkers"
python build/tmp/a6d24-full-frame-gate/score_full_frame.py
python build/tmp/a6d24-full-frame-gate/facilitator_verify.py
```

The command's `--native` selects the production native-asset mode; it does not
launch the stock game. All output paths above belong to this round, not prior
evidence directories.

## Facilitator check

Independently recomputed every full-frame and box numerator using a separate
Pillow ImageChops/histogram checker with exact `Fraction` comparisons, not the
worker's per-pixel sum. Matched all reported integers and hashes; checked prior
PNG byte identity, frozen-box provenance, camera/list/executable identities.
Read actual `src/app/main.cpp:223-225` default injection and the scene option
fields: these are the retained A6d3 positions with walk temporal 0 and phase 0,
not live World integration. No original-code claim required another exe read.

Independently changed just pixel `(0,0)`'s red channel by +1 in a scratch image:
numerator delta is **-1**, denominator unchanged. Reopening the untouched PNG
reproduces baseline exactly. This is a scorer inclusion/normalization check,
not a production-source or behavioral regression test. No source mutation was
performed or implied. Separate verification record:
`build/tmp/a6d24-full-frame-gate/facilitator-verified.json`.

Full build reported no work; **24/24 CTest pass**. No production source, tracked
harness, old captures, install/config/save or camera/list input edits. A6d23
facilitator changes were preserved. Own process and `LemWeb` scheduled-task checks
returned no rows. No commit authorization assumed.

## Limits and next priority

- Native full animation/controller state is absent from the original A6d3
  capture. This is the frozen current-candidate score, not a complete native
  state-pairing claim. [A6d23](phase-a6-native-camera-pair.md)'s different pause
  has no screenshot and was not substituted.
- Worker manifest field `normalized_exact_decimal` actually contains a binary64
  quotient's string; the exact values are the integer numerator/denominator above.
- Its aggregate `overall_a6_pass` implementation also includes tube-only in an
  `all()` despite tube-only being diagnostic. That makes no difference to this
  failure: **actors-on itself fails clause 1**, and actor 2 fails clause 2.
  Do not reuse that field as a general passing A6 verifier or ignore incomplete
  controller-state pairing if numerical gates ever pass.
- No scene-region decomposition or causal attribution was performed. The next
  useful planning input is localization of the dominant full-frame error against
  this same immutable reference; not another fitted actor offset, gate relaxation,
  or revival of the parked projection question. A7 scene work may be relevant,
  but its contribution is still a hypothesis. Keep both A6 thresholds unchanged.
