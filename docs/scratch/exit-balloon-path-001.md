**exit-balloon-path-001 — needs_review.** Playable v1, playtest 3 R4:
[human failure](../playtest/2026-11-13-playtest-3.md). The installed route is
`MM_CLIMB_002` / Just Climb, ordinary New Game -> map node 0 -> public Climber
clicks -> natural 10/10 win -> Results. The previous presentation moved the
balloon vertically. This change restores the omitted outward/around-cylinder
components. Full registered main/recovery verification is **pending supervisor
execution**, followed by independent review. No original execution occurred.

**Evidence and interpretation.** Reference:
`C:/progs/lemmings-webolution/Lemmings Revolution/Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
The project static REA profile used Ghidra 12.1.4; both analysis sessions were
closed without snapshot arguments. [Complete returned evidence](../../build/exit-balloon-path-001/static/)
is retained, including provider limitations, pseudocode and instruction reads.
Pseudocode is decompiler output, not recovered original source.

Retained accepted work: [hatch/exit presentation](hatch-exit-presentation-001.md),
[end transition](level-end-transition.md), and
[coordinate recovery](../../decomp-lemmings/docs/evidence/world_coordinates.md).
The lead's [original observations](../playtest/2026-11-14-original-session.md)
and both retained sheets, [balloon-a](../playtest/reference/original-2026-11-14/balloon-a.jpg)
and [s2-balloon](../playtest/reference/original-2026-11-14/s2-balloon.jpg), establish
the visible sideways departure, a still camera and approximate timing. Their
single viewing direction does not alone determine the 3D path. Static evidence
now resolves the missing coordinate semantics:

| Native evidence | Meaning used here |
| --- | --- |
| `0x426A0C..0x426A95`, full update `0x42688D` | Logical X adds `0.5*dt`, logical Y adds `5*dt`, radial blend adds `0.5*dt`. Constants at `0x4B03A8` / `0x4B03F0` read back as 0.5 / 5.0. |
| `0x4231DE`, pseudocode checked against complete instructions | X wraps in the 256-unit circumference. |
| Model vtable `0x4B1688`, slot +8 -> `0x451668` -> `0x4658D6` -> `0x43D56C` | The updated logical triple is mapped through the level's coordinate lattice before it becomes the model translation. Model-position forwarding/copy was checked against instructions. |
| `0x43D56C`, pseudocode checked against complete instructions | Interpolate radial layers first, then angular sector, then height row. The third component is a dimensionless blend, **not a world-space radius increment**. |
| `0x43D09A`, complete 86-byte instruction body | Read alternating file triples into runtime offsets `sector*0x18` and `sector*0x18+0x300`, 32 times. |

Key instruction records include
`ev_8cd06a437b64c0efd45a20ad6d5e720be29db31e52d8ef116f9cb342bd110ac7`
(exit update), `ev_43906d08245c159b17a784320ea01f257694b5cacf85eceef5106cc1ba1558e7`
(model positioning), and
`ev_c2956b05e4668464af300c8952040ec3a4b9955986001cb594bc59d0077e2511`
(coordinate interpolation). The additional loader records are
`static/loader_pseudo.json` and `static/loader_instructions.json`.

The existing `map_world_coordinates` arithmetic is reused unchanged. Its
`world_coordinate_mesh(rawWorld)` convenience view does not deinterleave the
serialized blocks, despite the retained recovery prose describing those blocks
as runtime layers. The new **presentation-only adapter** explicitly separates
the file pairs, as the native loader and the existing ActorPose template accessor
do. No recovery code or historical comparison record was changed. Independent
checks against all **111 serialized exit translations on 102 installed levels**
pass after this adaptation; direct use of raw file blocks failed that check.

**Production behavior.** `runApplicationLeg` loads the installed level and builds
`BackdropScene`, then calls `entranceExitPresentation` once. That factory binds
each actual exit root to its serialized logical position and owns the unpacked
coordinate lattice. The existing frame loop samples `7 - end.timerSeconds()`
during `LevelEndPhase::Pending` and calls `setEntranceExitPresentation`.
The setter samples the three traced coordinate rates absolutely, maps the
start/current triples, and applies their displacement to the stored root matrix.
The ordinary LegacyUnattributed scene stage draws the basket, ropes and balloon
together. Repeated samples cannot accumulate drift; zero restores the exact
stored pose. Neither `main.cpp` nor `LevelEndTransition` changed.

On Just Climb, the logical triple moves from `(221,40,1.5)` to `(224,70,4.5)`
over six seconds. The mapped radius grows from **64.81459 to 95.27087** world
units, while the angular position advances roughly 4.2 degrees and height rises
30 units. This is a shallow outward spiral, not a complete revolution or an
invented radial speed. [Half-second coordinates](../../build/exit-balloon-path-001/after/model-path.csv)
retain the exact doubles. All three rates come from static instructions.

The existing results rule is preserved: Pending starts at timer 7, the trigger
call does not subtract dt, and strict timer <1 publishes the original snapshot.
This route retains 64 Pending frames, including three paused frames, with its
last visible departure sample at 6.0 seconds. The camera remains player-controlled
and does not automatically follow. A public right-drag after all ten Climbers
are assigned exposes the right-edge perch; it stays still throughout Pending.
Integer mouse motion reaches yaw 173.4167, within 0.21 unit of the requested
diagnostic view. The original's exact camera parameters were not retained.

**Before/after images and preservation.** The authentic incoming `main.cpp` and
`BackdropScene.cpp` were frozen before the production path changed:
[source copies](../../build/exit-balloon-path-001/before-source/),
[entry hashes](../../build/exit-balloon-path-001/before-sha256.json).
Their CTest identity check prevents substituting modified files. Both executables
use the same current driver/common support; the before target compiles the frozen
application and scene implementation. Incoming main changes remain byte-identical.

- [Side-by-side overview](../../build/exit-balloon-path-001/comparison.svg) and
  [half-second sequence](../../build/exit-balloon-path-001/comparison-full.svg)
  embed both original sheets beside the before/after clone frames. Original
  alignment is approximately 13.5 s / 83.5 s, clone alignment is Pending t=0.
- [Inspected reduced montage](../../build/exit-balloon-path-001/inspection-montage.png)
  shows the vertical baseline and diagonal outward replacement at t=0/2/4/5.
  The original frames and clone both visibly rise; screen-height constancy is
  not claimed from the lead's qualitative “little height change” description.
- Actual clone images: before [perch](../../build/exit-balloon-path-001/before/pending-0.png),
  [2 s](../../build/exit-balloon-path-001/before/pending-2000.png),
  [5 s](../../build/exit-balloon-path-001/before/pending-5000.png);
  after [perch](../../build/exit-balloon-path-001/after/pending-0.png),
  [2 s](../../build/exit-balloon-path-001/after/pending-2000.png),
  [4.5 s](../../build/exit-balloon-path-001/after/pending-4500.png),
  [Results](../../build/exit-balloon-path-001/after/results.png).

At two seconds the blue-cloth centroid moves **1.55 px before / 43.87 px after**
sideways. At 4.5 seconds the after image retains only 566 blue-cloth pixels,
versus 3,572 before; at 5.5 seconds it retains zero versus 3,368 before. This
camera/framing leaves a sliver longer than the scope's rough 4–4.5 s departure
estimate; the path was not retuned to fit that estimate. Most of the balloon
has gone at 4.5 s and it leaves before Results. The comparison is visually
consistent with the reference's diagonal departure, **not an exact onset,
camera or disappearance-time match**. The full-frame differences between
paired clone images are confined to the balloon region. Initial frames are
PNG-byte-identical.

[Comparison manifest](../../build/exit-balloon-path-001/comparison.json) retains
image/reference hashes, pixel bounds and summaries. Every paired gameplay frame
(stock, actors, motion budgets, counts, terrain version, end phase/timer and
pause) is identical; the private saved progress is byte-identical. Both routes
win 10/10. This is a scripted regression, not human playability or fidelity proof.
[Unchanged source hashes](../../build/exit-balloon-path-001/unchanged-paths.json)
cover incoming main and the existing hatch/exit tests; the
[task-only patch](../../build/exit-balloon-path-001/task-only.patch) separates the
edits from incoming work. Normal and installed saves are checked unchanged by
both routes; each run seeds only its own temporary task save.

**Checks and remaining limits.** Registered UCRT64 GCC 16.1 / CMake / CTest,
existing `-O2 -g` with assertions, builds capped at eight jobs. Final focused
[run](../../build/exit-balloon-path-001/final-regressions.log): **8/8 passed,
137.40 s**. It covers baseline identity, paired production win/pause/results,
all installed exit data, literal-image comparisons, existing `levelend`, the
frozen-header `fastforward_toggle_baseline`, and existing `hatch_exit_end_after`.
Earlier [model/assets checks](../../build/exit-balloon-path-001/after-tests.log)
passed both existing checks; its new route check initially failed two guessed
pixel thresholds. Initial driver failures and their logs remain under
`driver-1-failed/` and `development-1-after/`: raw file layout and an unsuitable
gameplay camera were corrected; the new pixel checks now exclude sky pixels
and distinguish the actual vertical/outward paths. No existing assertion,
timeout, route classification or control was changed. No full suite was run
by this worker; all other scripted classifications await supervisor verification.

Provisional choices, labelled in production comments: rotate the stored model
with its radial heading while preserving its scale/tilt, retain the accepted
global Pending trigger, and retain the existing results handoff. Native tangent
heading, phase-dependent 0.02 bob/roll, per-type early departure and height-based
native removal are not reproduced. The latter two were already outside the
accepted presentation path. The first-level exit does not reach the native
height-removal threshold during Pending. Existing 20 fps footage extracted at
matched camera/counter frames would settle onset alignment and body wobble;
a matched opposite-angle recording and per-type/timeout examples would settle
the remaining visual/trigger differences. No new original run is requested.
Historical timing/depth gates and completion criteria remain unchanged.

Changed files: `src/app/BackdropScene.cpp/.h`, `cmake/webolution_sources.cmake`,
`CMakeLists.txt`, `src/__test__/test_exit_balloon.cpp`,
`cmake/exit_balloon_identity.cmake`, `tools/compare_exit_balloon.py`, this report.
The supervisor's existing RepositoryLease owns this worker; no manual protected
control edits, queue/status edits, commits or original-file writes were made.

Handoff: R4 outward/around-cylinder departure implemented from static coordinate rates; **needs_review**.
Next: supervisor full registered main/recovery verification and independent acceptance review, then human confirmation.
