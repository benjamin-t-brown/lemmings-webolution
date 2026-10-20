# A4v: actor support residual was a half-pixel convention

Queue `a4v-actor-support-residual`. Offline round, no game process. Worker packet:
`build/tmp/a4v-actor-support-residual/round-note.md` (prediction `prediction.md`, written
before scoring; decomposition `decompose.py`, `decomp-baseline/`, `decomp-after/`;
variants `variants.py`, `variants-2/`; filter scans `scan_filter_states.py`,
`scan_filter_callers.py`; gate `final-gate/`). The first worker died on a model-gateway
error mid-round; a continuation on the same pin finished it, and one repair added a
missing V-axis test.

## Result

- **Code:** `drawActorSprite` now uses an integer pixel centre. A pixel `x` is covered iff
  `left <= x < right` (`ceil(edge)` for all four bounds), and UV is
  `(x - left)/(right - left)`, `(y - top)/(bottom - top)`. It was `ceil(edge - 0.5)` and
  `x + 0.5` / `y + 0.5`. Nearest texel is unchanged.
- **MEASURED (offline, two fixtures):** decomposition into (a) native-only, (b)
  candidate-only, (c) both-with-error pixels, error as summed 8-bit |RGB|:

  | fixture | state | (a) n / err | (b) n / err | (c) n / err | candidate MAE | support MAE |
  | --- | --- | --- | --- | --- | --- | --- |
  | five | before | 17 / 1838 | 12 / 2032 | 112 / 6246 | 0.0032284 | 0.0819 |
  | five | after | 0 / 0 | 0 / 0 | 129 / 141 | 0.0000450 | 0.00143 |
  | eight | before | 22 / 3215 | 24 / 3133 | 116 / 4995 | 0.0036200 | 0.0778 |
  | eight | after | 0 / 0 | 0 / 0 | 138 / 160 | 0.0000511 | 0.00152 |

  Footprint now matches native exactly on both. About 1 unit per pixel of colour error is
  left, unexplored.
- **Variants:** bilinear at the old convention only reaches 0.069 / 0.068; bilinear at the
  integer centre is 0.027 / 0.026. Nearest at the integer centre is 0.0014 / 0.0015. The
  worker's bilinear prediction was falsified in mechanism. The texel-centre (UV) part is
  the larger share; coverage alone is the smaller share.

## Scope limits

- **The rule is a fit, not a byte.** Two fixtures, both at scale delta −1 with fractional
  edges. A third fixture with another fractional offset or scale would test it.
- **Filter state in force is untraced.** See the facilitator narrowing below.
- The gate thresholds are loose: it passes under each single reverted part. The unit
  test, not the gate, is what protects each part.
- Actors are still not drawn in the app.

## Original bytes (READ, worker, rereads by facilitator)

- `0x47A766`: `cmp [0x58D53C],0` ; `jne 0x47A77B` ; `push 0` ; `mov ecx,0x58CD80` ;
  `call 0x470F29`.
- `0x470F29` (`ret 4`): local = 1; `[0x4C24E0]` = `0xEF`; if the argument is nonzero and
  byte `[this+0x3A1]` is nonzero, local = 2 and `[0x4C24E0]` = `0xA0`. Then
  `SetTextureStageState`(0, `0x10`, local) at `0x470F86` and (0, `0x11`, local) at
  `0x470FA3`. In D3D7 numbering those are MAG/MINFILTER, 1 POINT, 2 LINEAR (SDK
  inference).
- Scan of `.text` `0x401000..0x4B0000`: TSS `0x10` and `0x11` have one site each, the two
  above; `0x12` and render states 17/18 none. `0x470F29` has 10 direct callers. Not read:
  `dgVoodoo.conf` forced filtering, and which call is in force at actor draw.

## Facilitator check (2026-10-20)

- **Reread** `0x47A766` and `0x470F29`; bytes match the worker's packet.
- **Narrowed — filter and alpha reference reopened.** `0x470F29` writes `0xEF` first and
  `0xA0` only on its linear branch. A4t read `[0x4C24E0]` = `0xA0` in process memory.
  So if `0x470F29` ran before that read, its last run took the **linear** branch, unless
  another writer of `0x4C24E0` exists (not scanned). The worker's "point filter in force"
  is therefore not established, and the pixel fit favouring nearest over bilinear
  (0.0014 vs 0.027) disagrees with that branch. Either dgVoodoo overrides filtering, the
  actor draw runs under a different call, or the fit is coincidental. A4u's `> 0xA0`
  stands as the A4t-measured value; `0xEF` on a point path remains open.
- **Decomposition reproduced** with an independent script
  (`build/tmp/a4v-actor-support-residual/facilitator/fcheck.py`, log `fcheck.txt`): all
  (a)/(b)/(c) counts and errors above match for both fixtures, before and after.
- **Own gate rerun** reproduces five 0.0000450 / 0.00143 and eight 0.0000511 / 0.00152.
- **Found a hole, repaired:** reverting the y-axis UV alone left `test_actorsprite` green
  (the new 8×1 sheet cannot discriminate V) while the gate worsened 60× (five support
  0.00268, eight 0.00237). Repair 1 added a 1×8 distinct-row sheet drawn 2:1 at a
  fractional top. Own rerun of y-UV alone now fails `test_actorsprite`,
  "integer-centre V selects rows 1,3,5,7". The worker also shows coverage-alone and
  x-UV-alone failing their own assertions.
- **Restoration:** `ActorSprite.cpp` sha256 `977bda40c751c9c8…` after each restore, fresh
  mtime; then fresh full build rc 0 and **20/20 CTest** (`facilitator/fct-final.txt`).
- **Live side:** no Lemmings process and no `LemWeb-*` task; the round never started the
  game.
