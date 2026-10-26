# C3bd initial/repair1 — supported scene stages around the tube

2026-10-26. Implementation packet: `build/tmp/c3bd-stage-order/round-note.md`;
initial packet/source diff preserved in `repair1/before/`. Independent checks,
fresh images, immutable-input manifests and logs: `build/tmp/c3-stage-order-pass/`.
Two dispatched attempts, zero stock starts/attaches/injection/host-task operations.

## Correction and scope

The production `main` frame now calls the shared `scheduleSceneFrame` seam:
**primary -> legacy-unattributed -> tube (including actors) -> secondary**.
BackdropScene stages share the same target and depth buffer with no intervening
clear. Accepted C3ao mixed-secondary dispatch/key/bias arithmetic is unchanged.

Retained C3u associations are reused, not guessed from nonsecondary membership:

| Retained root | Attribution | Source candidates |
|---|---|---|
| `0x59644` | primary holder, root slot+4 before tube | 923 mesh records, 0 billboards |
| `0x91782` | secondary holder, root slot+8 after tube | 82 mesh records, 17 billboards |
| `0x9ccd1` | separate Entrance model holder | 16 mesh records |
| `0xa0cdd` | separate Exit model holder | 156 mesh records |

Entrance/Exit retain their former pre-tube software timing, explicitly labelled
legacy-unattributed, **not primary**. Their conditional native entity/model route
does not establish admission or justify moving them in this pass. Actual repair4
mode/enable/admission/effective-state values remain unknown. This is a supported
conditional ordering correction, not an observed native-frame reconstruction.

## Facilitator check

Read every production/test hunk and actual main callback binding. Independently
authenticated C3u's 46 raw PE windows and re-read frame calls `0x441879`,
`0x441884`, `0x44199b`, holder gates and distinct root virtual slots
`0x4749a7 ff5204` / `0x474955 ff5208`. Three immutable Ghidra byte reads are
retained in `ordering-bytes-evidence.json` (Evidence IDs in the independent
ledger); REA analysis was closed without touching the executable.

The existing separate raw-LVL parser reproduces 1177 records partitioned
923/82/16/156. The secondary root has no own mesh records and exactly 29 **flat
direct** children; all 17 retained billboard offsets belong there. Combined with
review of the actual partition/draw loops, this bounds the no-drop/no-duplicate
claim to this retained layout. Primary and legacy contain no billboards, so the
stage split does not additionally reorder their former mesh/billboard streams.
The prior independent 29-child source-offset/key oracle still matches the
unchanged dispatcher and permanent reference regression.

Own asset-enabled verbose BackdropScene run reports
`STAGE_CENSUS primary=923/0 legacy=172/0 secondary=82/17`, **23 checks, 0 failures**.
**Narrowing:** the added stage counters count partitioned source candidates before
predicate/raster rejection, not actual visible/native submissions. Their permanent
assertion checks the aggregate; exact partition and retained coverage additionally
depend on the independent raw-tree check and loop review. No general hierarchy,
admission, precision or native census is certified. Configured CTest supplies the
retail environment; a direct invocation without it skips retail coverage.

Changed only production `SceneFrame.h` to put secondary before tube. Rebuilt the
test: CTest failed (return8), and the separately compiled native-order callback
oracle failed (return1). Exact original/restored header SHA-256 is
`4aa54668f15b311e69ba35ced6375c8e3fd7b12054798ee5e7d6a8fa882c377a`.
Own restored full rebuild and **44/44 CTest passed**. This discriminates SOURCE
scheduling, not native pixel fidelity. The initial worker's swapped test callbacks
alone were rejected; repair1 supplied a real production mutation and missing
render evidence before acceptance.

## Fresh unchanged gates and preselected diagnostics

Independent renderer launches and direct RGB integer sums import no worker
scoring code. Fixed footprint is still 23166 pixels; references, masks, thresholds,
camera inputs and floor inclusion are unchanged. All 11 independent images
(repair4 plus ten C1 views) are **pixel-identical to their retained baselines**;
the six common worker/facilitator images also match byte-for-byte.

| Fixed gate | Numerator / denominator | Result |
|---|---|---|
| C3 repair4 full frame | 14934330 / 235008000 = 0.06354817708333334 | FAIL, unchanged |
| C1 zoom1 footprint | 285083 / 17721990 = 0.0160863988750699 | PASS, unchanged |
| C1 elev_down footprint | 285700 / 17721990 = 0.016121214378294987 | PASS, unchanged |

Preselected diagnostics below use `--no-injected-walkers --skill-panel`, retained
cameras and their named A7p/A7q baselines. These are reports, not new gates:

| Diagnostic | Full-frame numerator / 235008000 | Footprint numerator / 17721990 |
|---|---|---|
| initial | 15550853 (0.0661715899) | 1499542 (0.0846147639) |
| left | 15058816 (0.0640778867) | 1415145 (0.0798524883) |
| elev_up | 4647470 (0.0197757949) | 280461 (0.0158255929) |
| MM_DIG_000 | 17471415 (0.0743439160) | 1795145 (0.1012947756) |

Other C1 footprint numerators, same denominator: mouse1694516, right1727286,
canonical1499542, elev0_neg1499256. All are unchanged. Worker bare-scene diagnostics
omit the skill panel and therefore have different full-frame numerators; they are
not regressions against the panel-on baselines above. The second level retains
the existing empty-backdrop guard, so it is a non-regression control, not evidence
of this stage mechanism executing on another level.

Repair4 PNG SHA-256 remains
`9a5baba7264e3b2672ada76b7b26ba4e6044588c28dcad284e6f3ff5bd48f145`;
scene-off remains `13b1bd501803c3bf3e210cac651b1968083b26c74a852925e04e5e6f44e3829e`.
Region sums remain exact over all307200pixels, with the unbounded floor included
in rest. No pixel cause is attributed. An initial independent launch failed
before rendering because Windows path separators were consumed by bash; only the
verification command's paths were corrected, then all fresh renders completed.

## Cleanup, preservation and stop

Seven install-file hashes match the before manifest; no game or LemWeb scheduled
task remains. No live write/resource was owned. All26 fixed input/dispatcher hashes
match. All16 initial unrelated dirty files were byte-exact before documentation
landing; only our PLAN status line and appended phase-C entry are layered on those
initial changes, staged separately. Prior stopped run is archived without changing
its contents. Framework checks and25 framework tests pass; no framework edits.

Directive stop point reached: correction verified and scores reported, **not C3
acceptance**. Native admission/effective state, model-holder timing, clipping/type1
dispatch and full packing adoption remain open. Observer installation-safety park
and runtime refusal are unchanged. No further renderer/observer round or native
permission follows automatically. Actual dispatch-to-landing/pass times and commits
are recorded in the queue/archive.
