# C3bg — bounded primary source-to-accepted-write diagnostic

2026-10-26. **Accepted diagnostic, not C3 acceptance or causal attribution.**
Approved delegated recovery; directive SHA-256
`de6f270670a1e85c5f312cf6fdd616ef77d3f318e5593319af0c9efb8e64a7e8`.
Cumulative attribution attempt **4** (C3be initial/repair1 and C3bf remain counted).
One implementation dispatch; no repair needed. No live/native work.

## Delivered executable contract

`SceneRasterObserver` is an optional **software** diagnostic sink, unrelated to the
parked native observer. `BackdropScene::renderStage` supplies candidate record,
node and block file offsets before texture/predicate rejection, then tags each
primary fan triangle. Only the primary scheduler callback receives the sink.
`SceneRaster` checks depth/alpha admission and mask membership before populating
and collecting a write payload; the callback follows actual RGBA and optional
depth assignments. Assignments count even when RGBA is unchanged. Ordinary raster
arithmetic, stage order, materials and depth policy are not corrected or tuned.

The repeat command is:

```powershell
powershell -NoProfile -File scripts/Invoke-Ucrt64.ps1 "cmake --build --preset dev-debug"
python tools/harness/c3_write_diagnostic.py
```

Run from `C:\progs\lemmings-webolution` on mouv, with normal Windows Python/Pillow,
the existing UCRT64 build and saved repair4 camera/actors/reference/support files.
The harness pins repair4 and the saved C3be primary-changed-rest mask, converts it
to raw bytes and renders diagnostic off/on. CLI paths are `--c3-write-mask` and
`--c3-write-events`; no diagnostic flags are used for ordinary rendering. The
supported measurement is this fixed masked invocation, not arbitrary masks,
levels, repeated interactive frames or a full-frame capture. No full-frame event
stream was collected. Image invariance is measured, not a zero-overhead claim.

Worker packet/raw CSV/per-source totals/images/commands:
`build/tmp/c3bg-write-diagnostic/`. Raw `masked-events.csv` SHA-256:
`1cb392df26fee79b6955b5758b802322a4b28901213c0c533c4975f55ed53c1d`.
There is no event cap or truncation in this packet. Candidate rows are a census,
not accepted triangles, native draws or visible objects. Primary billboards are
independently counted as **zero**, not assumed absent by exclusion.

| Fixed support result | Independently checked total |
|---|---:|
| Primary mesh candidate records | 923 |
| Records with no accepted masked assignments | 788 |
| Color assignments | 134,518 |
| Depth assignments | 134,518 |
| Unique accepted pixels | 100,172 |
| Assignments after an earlier write to that pixel | 34,346 |
| Assignments with unchanged RGBA | 124 |

Per-record node/block/fan identity, accepted triangle-index sets, assignment totals
and unique pixels are retained in `masked-totals.json`. This joins **our source
records to accepted writes only**. It does not determine surviving final influence,
ownership of baseline error, wrong native objects/state or a correction.

## Facilitator check

Independent checks and logs are under `build/tmp/c3-write-pass/`:

- `verify-support.py` reconstructs the mask from retained baseline/primary RGB and
  the pre-existing rest rectangles, without worker imports. All 100,172 pixels
  match the saved mask (baseline error there10,364,721); no ranking is repeated.
- `verify-source-census.py` separately walks stored LVL bytes at root file offset
  `0x59644` through `0x9176e`: 923 records, four nodes, zero billboards. This is a
  **stored-file/SOURCE** check, not executable research or native admission.
- `verify-events.py` scans every raw event and candidate without worker code. All
  923 record/node/block/fan identities match that stored-file census; all events
  are inside the independent mask. Every before-RGBA/depth tuple matches that
  pixel's preceding accepted event, starting from the actual primary clear
  `(0,0,0,255)/infinity`; every accepted depth is positive/finite and strictly
  nearer. It independently reproduces every per-record and whole total above,
  including zero-write records, repeats and unchanged assignments.
- Own fresh renderer command with the **independently reconstructed raw mask**
  reproduces the entire CSV byte-for-byte, not just its summary. Repeated again
  after mutation restoration with the same exact result. `independent-events.json`
  retains raw examples: LVL record`0x635f7`, pixel(638,79), black ->(241,209,108,255),
  depth infinity ->1222.9960315086298; overlapping record`0x63877`, pixel(593,102),
  depth1179.9116235749507 ->1033.489881991896; unchanged-color record`0x7846e`,
  pixel(583,338), depth478.0764056589519 ->444.50414664526238. These are event
  snapshots, not independently reconstructed real-scene texture/shading values.
- A separate compiled `sample-oracle.cpp` plus `verify-sample-oracle.py` checks raw
  payload arithmetic against independent literal inputs: blended RGBA
  `(10,20,30,255)` ->`(55,23,35,191)` with depthWrite=false, then
  `(78,24,38,159)` with depthWrite=true. IDs/pixel/before/after/depth flags match;
  this tests the software logger, not native fidelity.
- Independently changed **only** the mask guard to `observer != nullptr`.
  Successful mutant build; `test_sceneraster` exits1 with
  `FAIL: outside-mask accepted writes are not collected`. Restored source SHA-256
  exactly`d18daf55c3ccd1f687d79c55618cd2ffe1923d4b8da6f90d5aee40bc0609457a`;
  rebuilt full project and restored diagnostic test passes. `mutation-result.json`
  records the single edit, hashes and exits. Synthetic controls also separate
  depth/alpha rejection, repeated writes, optional depth assignment and zero mask.
- Own restored **44/44 CTest pass**,66.21s; framework tests **32/32 pass**.

## Image invariance and unchanged gates

Repair4 diagnostic disabled, enabled, own independently masked run, and restored
enabled run all have identical PNG bytes to C3be/C3bf:
`9a5baba7264e3b2672ada76b7b26ba4e6044588c28dcad284e6f3ff5bd48f145`.
Own fresh post-restoration C1 gated PNGs also match retained bytes exactly.
`verify-gates.py` uses separate Pillow/integer arithmetic, no worker scorer.

| Fixed gate | Own fresh error | Verdict |
|---|---|---|
| C3 repair4 full frame, floor included |14,934,330 /235,008,000 =0.06354817708333334| **FAIL**0.02, unchanged |
| C1 zoom1 footprint |285,083 /17,721,990 =0.0160863988750699| PASS, unchanged |
| C1 elev_down footprint |285,700 /17,721,990 =0.016121214378294987| PASS, unchanged |

No gate/reference/mask or exclusion changed. C3bd ordering and C3ao integration
remain; Entrance/Exit stay legacy-unattributed and native observer refusal stays.

## Preservation, timing and stop

The six owned code/test files and new harness are retained in the landing commit;
exact snapshots, hashes and diff are additionally under
`build/tmp/c3-write-pass/source-delivery/`, `source-delivery-sha256.json` and
`source-delivery.patch`. Repetition does not require finishing implementation.
Worker's initial path/PNG-encoding check failures and resolutions remain in its
packet; they were delivery debugging, not infeasibility findings.

All16 non-source fixed/install inputs and113prior evidence files match initial
hashes. Nine initially dirty/untracked files are preserved outside isolated own
PLAN status/parking-line changes and Phase C append; unrelated framework changes
and the parent's directive edit are excluded from the landing. The temporary
verification mutation is exactly restored; the opt-in diagnostic remains by
design. Independent host check: zero Lemming processes and zero LemWeb tasks.
Worker session`ses_e862fe1f9ffeZfIEBxetdX4F3G` succeeded/idle on its configured
pin; no live resources owned. Actual child/parent/profile/hash checks are in the
queue and ownership artifacts. Historical sessions without idle metadata were
not treated as running: `/api/session/active` confirms no competing project owner.

Pass start14:00:05-06:00; dispatch14:03:54; worker idle14:16:07; independent
verification completed14:22:53. Actual landing/end timings are recorded in the
queue. One dispatch/attempt4, no repair5, zero starts/attaches/injection/tasks or
original-executable research. The initial45-minute repair-inclusive deadline was
14:48:54; overall limit16:00:05. Stop at the accepted writes diagnostic. Unused
repair capacity is not follow-on authorization; the directive is consumed despite
its active heading. Final influence, native comparison, correction and observer
work remain parked and require fresh approval; no next question is dispatched.
