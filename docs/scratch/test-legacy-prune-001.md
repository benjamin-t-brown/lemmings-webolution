# test-legacy-prune-001: remove historical replay scaffolding

Status: **needs_review**; mandatory smoke verification and independent review are
pending supervisor execution. Playable-v1 plan step 4, explicitly authorized
build/test-only work. Difficulty: routine; recommendation: Sonnet, because this
removes established registrations without changing gameplay. This worker used
the explicit Codex provider override.

The dependency was the accumulated frozen-source replay workload described in
the implementation plan. There is no new gameplay defect or production change.
The accepted frontend-window-quit-001 report and test-parallel-isolation-001
resource audit supplied the retained before/current registration context.

| Main CTest inventory | Before | After |
| --- | ---: | ---: |
| All registered tests | 323 | 233 |
| Smoke | 95 | 95 |
| Legacy | 228 | 138 |

Removed **90 tests** and **41 unused files**. The complete removed-test,
deleted-file and modified-file lists are in
[the inventory](test-legacy-prune-001-inventory.json). No test was added or renamed.
The exact `ctest -N` listings are
[before](../../build/test-legacy-prune-001/ctest-before.txt) and
[after](../../build/test-legacy-prune-001/ctest-after.txt); corresponding full command
and property inventories are `build/test-legacy-prune-001/inventory-{before,after}.json`.

Changed `CMakeLists.txt` and 15 included CMake modules to retain only current-side
targets, remove before/baseline dependencies, and disable optional paired-file
inputs. Deleted the unused frozen identity/freezing scripts, comparison scripts,
three old baseline drivers and the frozen raster-equivalence driver. Optional
render-skip entry-source targets were removed too. Existing current-side targets
whose tests had already been retired remain as requested.

Seven legacy drivers also needed their automatic before-file comparisons removed:
scene integration/admission/branch/nested, hatch timing/exit and exit balloon.
The shared gadget-store diagnostic header retains its current-render checks and
mask output, with its unused historical comparison helper removed. The scene
drivers preserve their rendering partitions: the same ten composed levels and
the same eighteen nested-level resource checks, now explicit installed-level
sets instead of reading a prior run's admission list. The eighteen names come
from retained `build/scene-nested-impl-001/before/levels.txt`; this is preservation
of existing coverage, not a new replay fixture.

Production call path remains unchanged: CMake registers the current driver,
which links shared current application/simulation objects. The scene drivers
open the installed archives, call `BackdropScene::build` / `TubeScene::build`,
render through the production stage scheduler, and exercise `runApplication`.
Examples retained in the four executed drivers include `MM_CLIMB_002`,
`MM_BASH_002`, `MM_W_012`, `MM_DIG_000` and `E11_KF_1`. Before, those drivers read
frozen-run frames, hashes or admission files; after, their independent loader,
rendering, resource and application checks run without those reads.

Three standalone static reference-byte checks remain: `actor_depth_reference`,
`hover_feedback_reference`, `pick_nearest_reference`. They read reference bytes
independently and do not pair before/after runs. Likewise, `playablebaseline_*`
is the accepted current-clone observation driver, not a frozen-source baseline.
The optional compiled-out profiling target generates wrappers around current
sources; it contains no retained pre-task implementation.

Checks completed:

- Full main default ALL build, registered MinGW GCC, `-O2 -g` with assertions,
  eight build jobs: passed. Exact logs:
  [initial full build](../../build/test-legacy-prune-001/build-all.log) and
  [final build](../../build/test-legacy-prune-001/build-all-final.log).
- Existing current-side scene tests `sceneintegration_after`,
  `sceneadmission_after`, `scenebranchimpl_after`, `scenenestedimpl_after`:
  **4/4 passed**, 42.04 seconds serial. This checked the edited drivers after
  removing their historical inputs; it was not a full-suite run. Exact log:
  [current-scene-drivers.log](../../build/test-legacy-prune-001/current-scene-drivers.log).
  SDL dummy drivers and task-private LocalAppData/XDG paths were used.
- Inventory audit: all 95 smoke names, commands and properties are identical;
  `cmake/test_tiers.cmake` and smoke sources are unchanged. Remaining legacy
  timeouts and non-dependency properties are unchanged. Compiler inputs contain
  no frozen sources. The only build-directory inputs are the current profiling
  wrappers described above.
- Task-entry hashes confirm no production, recovery, queue, verifier, controller,
  registry or shared-instruction edits. Pre-existing edits were preserved. Backups,
  entry diff/status, task-only diff and audit are under
  `build/test-legacy-prune-001/`; see
  [audit.json](../../build/test-legacy-prune-001/audit.json).
- Scoped `git diff --check` passed:
  [diff-check.log](../../build/test-legacy-prune-001/diff-check.log).

Limitations: the smoke gate and the remaining legacy tests have not been run by
this worker. No original execution, original comparison, new gameplay assessment
or fidelity claim was made; historical gates and parks remain unchanged. The
normal and installed saves were not written.

Handoff: supervisor runs `verify.py --profile smoke`, then independent review.
Handoff: review the inventory and task-only diff; accept or return this task only.

What to check: level `MM_CLIMB_002`, ordinary frontend entry then return/Quit;
expected result: existing clone behavior is unchanged, and the unchanged smoke
gate passes without building or executing historical before/baseline targets.
