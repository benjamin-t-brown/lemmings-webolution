# C3bh — three recorded contributions' software final-image influence

2026-10-26. **Accepted bounded sensitivity result, not C3 acceptance or a correction.**
Approved delegated continuation, directive SHA-256
`0aa356b8ae21413f555043e988e101e13a68e21352bdd9f31f4bbbed0bb6ac47`.
Cumulative attempts **5 and repair6**; no third dispatch. C3bg's accepted logger,
identities, fixed mask and original evidence were reused, not rebuilt.

## Selection and exact intervention

`build/tmp/c3-influence-pass/selection.json` was written **before any variants**.
Select three records by descending existing baseline RGB L1 error on their
C3bg accepted masked support, source-offset ascending for ties. No removal
penalty or variant score selected a record or parameter.

| Stored LVL record | Kind/material under `Models\bg_sand_medium` | Accepted masked pixels | Baseline error on accepted support | Later primary color assignments |
|---|---|---:|---:|---:|
| `0x63877` /407671 |5, `sand_1` |9,393|2,060,922|3,567|
| `0x6b72b` /440107 |5, `sand_1` |14,146|1,376,837|0|
| `0x7047c` /459900 |7, `wood1` |5,709|1,044,586|378|

All three are in stored node366279, blend selector0. Their overlapping accepted
support union is26,804pixels carrying3,998,968 baseline error units:26.7770% of
the unchanged full-frame error,38.5825% of the C3be primary-changed-rest error.
These are **support statistics**, not additive error ownership. Other132 accepted
records,788 zero-write records, other masks/frames/levels and native identities
are unmeasured by these interventions; the result does not generalize to them.

For each record, three variants suppress **only its accepted assignments inside
the pre-existing100,172pixel mask**: RGBA only (`color`), depth only (`depth`),
or both (`both`). This is not geometry/source removal everywhere. Other sources
and unmasked assignments remain. Depth suppression can admit previously rejected
later fragments, so the variants are nonlinear counterfactuals, not candidate fixes.

Worker repeat, from repository root on mouv with normal Windows Python/Pillow:

```powershell
python build/tmp/c3bh-final-influence/run_repair.py
```

The guarded script temporarily wraps `BackdropScene`'s existing masked callback,
forwards original logging, then restores selected before-RGBA and/or before-depth.
Its event rows therefore describe **pre-intervention snapshots**, not retained
post-intervention writes. Script, original/patched source, nine final PNGs,
baseline-before/after, commands, hashes and logs are preserved in that directory.
The temporary change is exactly restored in `finally` and ordinary code rebuilt.
No permanent source or accepted diagnostic change is landed.

## Final sensitivities versus existing error

Every entry below uses the **variant's own final-RGB changed support**. Baseline
error is against the unchanged saved reference on that support; variant error is
separate. Positive delta means damage, negative delta means counterfactual error
reduction. Neither sign selects a correction or diagnoses native correctness.

| Record | Suppression | Changed pixels | Baseline error there | Variant error there | Error delta |
|---|---|---:|---:|---:|---:|
|`0x63877`|color|5,826|1,281,686|1,741,362|+459,676|
| |depth|3,779|812,818|224,060|−588,758|
| |both|5,826|1,281,686|743,176|−538,510|
|`0x6b72b`|color|14,146|1,376,837|6,169,145|+4,792,308|
| |depth|651|109,266|148,845|+39,579|
| |both|14,146|1,376,837|6,134,718|+4,757,881|
|`0x7047c`|color|5,328|966,468|1,638,028|+671,560|
| |depth|3,982|815,600|814,675|−925|
| |both|5,315|962,734|1,138,565|+175,831|

Color/depth support intersections are3,779/651/3,979pixels respectively. Joint
RGB deltas differ from the sum of isolated deltas on3,779/651/4,002pixels.
For wood1,16pixels in the union of isolated supports are unchanged under joint
suppression. Do not add the modes' errors or assign ownership from their supports.
Across all nine variants, the affected union is25,303pixels carrying3,625,825
baseline error units (24.2785% of full-frame error), again not a causal partition.

## Strongest SOURCE-level finding and limits

The three sampled records have **both final color-path and depth-mediated
influence** in our software frame. They are not merely records counted upstream.

- Color-only suppression preserves every primary depth/admission tuple. Original
  raw chains independently show3,567/0/378 sampled pixels later assigned by another
  primary record. Each such last primary writer has blend selector0, so it
  overwrites the tested earlier color dependence; none is in that record's final
  color-sensitive support. This is software assignment/overwrite evidence, not
  native visibility.
- On the remaining last-primary support, nonzero primary-end RGB perturbations
  survive to final RGB on5,826/14,146/5,328pixels. Three additional wood1 pixels
  have a nonzero last-primary color perturbation but no final color sensitivity:
  that **tested perturbation is erased downstream**. Those three are nevertheless
  depth-sensitive. Color-inert is not influence-free. The precise later overwriter
  stage/record, or replacement versus quantization/blending mechanism, is unmeasured.
- Depth-only suppression admits4,439/944/1,281 additional **other-record primary
  events**, with no original admission lost and no added selected-record event.
  For `0x63877`,4,086 newly admitted events are wood1 record`0x7047c`; other new
  identities and raw samples are retained, not named as native objects.
  All3,779/651 final depth-sensitive pixels for the sand records also have new
  primary admissions. This overlap does not uniquely assign their final colors.
- For wood1,1,233 final depth-sensitive pixels have new primary admissions;
  **2,749 have identical primary-end RGB but different primary-end depth**, then
  different final RGB. This directly separates a downstream depth-mediated path
  from direct primary color. Its later consumer identity remains unmeasured.

No native counterpart, wrong material/pose/order, effective device state, cause
of error or preferred correction follows. The two sand records' different
counterfactual error signs are not a parameter-selection rule. Supported ordering,
material/sort decisions, legacy-unattributed Entrance/Exit and observer refusal stand.

## Facilitator check

Independent artifacts/scripts are under `build/tmp/c3-influence-pass/`:

1. `verify-source-census.py` reuses the accepted independent stored-file checker,
   writing to this pass only. `verify-selection.py` separately reconstructs raw
   pixel chains/ranking/support error, confirming the preselected sample and union.
2. `verify-repeat.py` uses a **different temporary implementation**: inhibit actual
   RGBA/depth assignments in `SceneRaster`, rather than undoing them in a callback.
   It uses the previously independently reconstructed mask, not the worker mask.
   All nine fresh final PNGs equal the worker PNGs **byte-for-byte**, and its
   no-intervention diagnostic baseline equals the accepted baseline. Source is
   restored exactly and rebuilt. Independent variant events capture this alternate
   implementation's actual after values; its unchanged `depthAssigned` flag still
   reflects the original option, not inhibition, and is not a retained-write count.
3. `verify-images.py` imports no worker scorer. It reproduces every table entry,
   full error, PNG hash, support/overlap/non-additivity count, validates color-only
   depth/admission invariance and checks all final changes lie inside the original
   selected accepted support. It retains support PNGs and per-record raw CSVs with
   pixel, baseline/reference/three variant RGBs, errors and membership flags.
   `verify-reexposure.py` independently compares admitted event keys and primary-end
   RGB/depth, including the2,749pixel downstream distinction above.
4. `mutate-one.py` changes **only** the delivered wrapper's color-mode guard to
   `invalid-color`. The successful mutant build yields the ordinary baseline
   instead of the expected407671-color PNG; the behavioral equality assertion
   fails as required. Source restores exactly to SHA-256
   `07e01577ea3ed238bcebd37b7ebc2c65e1b9ac03743550f0973c60d266fc2c51`.
5. Restored full build and **44/44 CTest pass**,63.73s. Framework tests **32/32 pass**.
   `verify-gates.py` independently renders/re-scores fresh ordinary gate images;
   all three PNGs equal retained bytes exactly:

| Gate | Fresh unchanged error | Verdict |
|---|---|---|
|C3 repair4 full frame, floor included|14,934,330/235,008,000 =0.06354817708333334|**FAIL**0.02|
|C1 zoom1 footprint|285,083/17,721,990 =0.0160863988750699|PASS|
|C1 elev_down footprint|285,700/17,721,990 =0.016121214378294987|PASS|

Repair4 PNG SHA-256 remains
`9a5baba7264e3b2672ada76b7b26ba4e6044588c28dcad284e6f3ff5bd48f145`.
No gate, mask, reference, exclusion or floor policy moved. `verify-preservation.py`
confirms all21 fixed/source/install inputs and209 prior evidence files unchanged;
nine initial dirty paths preserved except isolated own PLAN lines/Phase C append.
All production/accepted-harness diffs are empty. Host check finds zero Lemming
processes and zero LemWeb tasks. No starts/attaches/injection/tasks/native research,
packages or observer work occurred; no live resources owned.

## Attempts, ownership and stop

Actual owner `ses_e86156bc2ffewXXG2clAuR4Z5o`, parent
`ses_e8646168dffeJ0x2Dtz2oTI7yv`; metadata/profile/model/hash and actual-active-set
checks are retained. Prior owners were succeeded/idle; no competing project owner.
Worker `ses_e86113f2dffexNcllQEIQ2UlTd` used the configured round pin, now succeeded/idle.

Pass began20:32:58.995Z. Attempt5 dispatched20:37:32.564Z, returned incomplete
20:39:12.650Z (100.086s); its capacity/time statement was not an external blocker
or infeasibility measurement. Final repair6 dispatched20:40:52.945Z, completed
20:43:19.474Z (146.529s), delivering the missing implementation. Initial note and
repair path failure remain preserved. Facilitator verification and actual
dispatch-to-landing/pass-end timings are in the queue. The conservative original
repair-inclusive deadline21:22:30Z and overall22:32:58Z were not reset.

Stop at this accepted three-record result. No second question, expanded influence
census, native comparison, correction or observer reopening follows. Directive
consumed despite active heading; further work requires fresh approval.
