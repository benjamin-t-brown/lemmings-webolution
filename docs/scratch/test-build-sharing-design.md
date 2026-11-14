# Test-build sharing: design (lead, 2026-11-14, read-only analysis)

Measured on the verification build `build/agent-native/main` (tree at antigrav-presentation-001
attempt 1) with `build/lead/compile_census.py` and `compile_groups.py`.

## What is wasted
- 1,867 compile steps for 367 distinct source files across 173 targets: 1,500 steps are repeats.
- The repeats are the application layer (`WEBOLUTION_GFX_SOURCES` plus the four `Scene*.cpp`
  files, about 41 sources). `webolution` compiles it once, and about 59 test executables and
  five near-identical `*_support` object libraries each compile it again. `main.cpp` is built
  59 times, `BackdropScene.cpp` 48, `TubeScene.cpp` 44.
- Every one of those command lines is unique, so nothing is shared today, but the differences
  are only per-test macros (`TP_SOURCE_DIR`, `HATCH_TIMING_BEFORE`, `PANEL_ICON_BEFORE`, ...).
  Checked by grep over `src/` outside `src/__test__/`: the only macro that reaches a production
  source is `WEBOLUTION_APP_NO_MAIN`, and only in `src/app/main.cpp`. The rest are read only by
  the test's own `.cpp`.

## Change
1. One object library, `webolution_app` : the application layer without `main.cpp`, compiled
   once with `WEBOLUTION_HAVE_SDL2=1`, linked by `webolution` and by every test that uses the
   current application sources.
2. One object library, `webolution_app_nomain` : `main.cpp` compiled once with
   `WEBOLUTION_APP_NO_MAIN=1`. `webolution` keeps its own `main.cpp` compile without it.
3. Each test executable keeps only its own test source(s); its private macros stay on the test
   target, where they now affect only that source.
4. The five `*_support` libraries (`fastforward_support`, `hatch_exit_support`,
   `actor_occlusion_support`, `fluid_kinds_support`, `scene_demo_support`) become the shared
   library plus whatever single file each one swaps or removes.
5. Frozen "before" targets that compile a retained source copy under `build/<task>/` keep
   compiling that copy: those are different sources and are evidence. Where a frozen target
   replaces only a few files, it takes the rest from the shared library only if the frozen
   copy of those headers is identical to the current ones; otherwise it stays as it is.
6. `cmake/*.cmake` task files follow the same pattern, and the standard task footer tells
   future workers to link the shared libraries instead of listing application sources.

## Expected effect (estimate, not measured)
About 1,867 steps down to roughly 450-600, depending on how many frozen targets can share.
An edit to a widely included application header now recompiles on the order of 1,000 steps;
afterwards on the order of 50-100. Peak compiler memory per build falls with it.

## What must not change
Registered test names and count, every assertion and timeout, and the frozen baselines'
sources. Check: `ctest -N` lists are identical before and after, the full suite passes, and
the compile-step count is reported from `compile_commands.json` before and after.

## Risk
A test that silently depended on its private macro leaking into application sources would
change behavior; the grep above says none does today, and the worker re-checks it. One-definition
problems are possible where a test currently swaps one application file for a variant; those
targets keep a private copy of that file and must not also link the shared object for it.
