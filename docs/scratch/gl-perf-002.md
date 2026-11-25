# gl-perf-002: ordinary GL play on normal GPU drawing (attempt 2, Claude)

Status: **incomplete, not ready to accept.** Acceptance criteria 2 and 3 (the 108-workload matrix and the
image comparison) have no new evidence from this attempt, and criterion 1 and the second half of criterion 4
are only read from source, not run. The cause is the environment (below), not a finding about the renderer.

## Attempt 3 (Claude): blocked again, no work done
Bash allows only `git show` and `verify.py`. A `cmake --build build/release --target webolution -j 8` call was
denied with "no approval surface". Per the task rules I did not retry it and did not run `verify.py` either,
because the supervisor runs the smoke gate. The tree is unchanged from attempt 2. The Release build, matrix, image
comparison, replays, Emscripten build and probe-binary deletion all need a session that can run native commands
(the interactive lead session, or a worker whose permission set includes `cmake`, `python build/gl-perf-002/matrix.py`
and the `webolution` launches). More attempts under the current permission set will fail the same way.

## Blocker (attempt 2)
This worker session had no approval surface for Bash. Only `python tools/agent_control/verify.py` was
permitted. A direct `cmake --build` of the release `webolution` target was denied, and so were all native
launches, `matrix.py`, replays and the image comparisons. I could not build, run, measure or delete the
probe executables and objects under `build/gl-perf-002/` (`*.exe`, `*.obj`; no delete tool was available).
The lead should delete them.

## What the uncommitted attempt-1 tree contains (read, not run)
Source-traced call path of ordinary GL level play:

- `src/app/main.cpp:1083` creates `HardwareLevelScene` only when the native GL presenter is active and neither
  `--gl-exact` (`main.cpp:244`) nor a GL comparison option is given. Otherwise `createOpaqueGpu` builds the exact
  path of phases 3 to 7 (`main.cpp:1094`). `--gl-exact` is the explicit opt-in.
- `src/app/HardwareLevelScene.cpp`: one mesh program (`HardwareSceneShaders.h`), vertex buffers cached per pass
  by the prepared-stage key (tube and primary are static, the rest stream), batches grouped by material,
  `glDepthFunc`/`glDepthMask`/`glBlendFuncSeparate` for depth and blending, one draw per batch. No
  texture barrier, no destination region copy and no readback in this file. Sprites are one fullscreen-triangle
  draw per actor into a viewport box. Fluid depth uses two depth blits per frame only when a drowning is on screen.
- `NativeCompositor::compose(..., hardware=true)` uses a `HARDWARE_COMPOSITION` shader and ordinary blend
  state instead of the barrier/copy path (`NativeCompositor.h:148,179,213`).
- Pick, hover and highlights for a hardware frame use `nativeLevelQueries` on the displayed frame's prepared
  CPU data (`main.cpp:2459,2474,2535`), not GPU pixels.

## Changes this attempt
- `src/app/HardwareLevelScene.cpp`: `status()` stops calling `glGetError` after 16 checks (`checked_`,
  `kCheckedFrames`). A per-frame `glGetError` synchronizes with a threaded driver. This is the only concrete
  candidate for the remaining swap wait that I could find in the source. It is **unmeasured**.
- `src/app/NativeCompositor.h`: `compose()` skips its final `glGetError` for the hardware path after 16 frames
  (`hardwareComposed_`). The exact path still checks every frame.

## Not done (needs a session that can build and run)
- Build the Release `webolution`; the changed files are compiled only by the native target, so they are
  **not compile-checked**. The smoke run below does not prove they compile.
- Run `build/gl-perf-002/matrix.py` (108 workloads x 3 runs x 2 backends) and read `summary.tsv`. Check p95 <= 1.5
  ms, max <= 6.9 ms, mean <= half of software, GPU p95 <= 0.5 ms, mean swap <= 0.3 ms, first present <= software.
  The swap stalls and the roughly 1 s cold first present are not yet attributed; likely suspects are shader
  compile and link at startup (the sprite, sky and visibility programs are lazy, so only the mesh program is
  built up front) and driver work on first `BufferData`/`TexImage2D`.
- Frame comparison (menu, map, Just Climb at rest and rotating, Just Dig after a dig, Kriss Kross drowning, Just
  Bomb after a blast, hovered actor): 99.5% of pixels within two bytes. Probes and baselines from attempt 1 exist
  under `build/gl-perf-002/`; none was run on the final tree.
- Recorded-win replays under `docs/playtest/replays/`: none run. No recording was touched.
- Emscripten software build: not built. The edits are inside `#ifndef __EMSCRIPTEN__` (`NativeCompositor.h`) or in
  a native-only file, and `HardwareLevelScene.cpp` is not in the web build, but this is not verified.

## Checks
- `python tools/agent_control/verify.py --profile smoke`: **ran once in this session**, 95 smoke tests and 84
  recovery tests passed (full output: `C:\Users\bbrown\.claude\projects\C--progs-lemmings-webolution\71fad927-e19b-4a0f-907c-7ab4e7226e5c\tool-results\bemf3i9zm.txt`).
  The supervisor's own smoke gate is still pending. This only builds `smoke_tests`, not the native GL target.

## Handoff
Attempt 2 added only the two `glGetError` reductions and found the rest of the attempt-1 tree wired as described. It has no
measurement, frame comparison, replay or release-build evidence.
Next: a session with build and run permission should build Release, run `matrix.py`, then attribute swap time and first present.

What to check (once built): `webolution --game-dir "Lemmings Revolution" --direct-level --native --level MM_CLIMB_002
--renderer gl --window-scale 2 --profile-motion rotate --frames 240` with `WEBOLUTION_FRAME_PROFILE` set. Expect the
active frame cost p95 <= 1.5 ms and mean swap <= 0.3 ms, the same picture as `--renderer software`, and hover on an
actor, a hatch and a switch matching software.
