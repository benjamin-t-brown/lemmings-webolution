# Web camera rotation, music and console output

Playtest 16 F3/F4 affected the level-entry turn and player rotation in the desktop browser. On the registered Node/Wasm build, turning the camera invalidates the exact-pose caches. Backdrop and tube rasterization consumed 24.6–27.6 ms per frame before the change. The level renderer also wrote one changing backdrop statistics line to stderr on every draw. [Human playtest](../playtest/2026-11-18-playtest-16.md)

## Rotation measurements

Times are milliseconds per 640×480 scene render. Both builds used Emscripten 5.0.3 Release and the registered Node 22.16.0 on the same 32-logical-CPU host.

| Installed level | Camera | Mean before → after | p95 before → after |
| --- | --- | ---: | ---: |
| Just Climb (`MM_CLIMB_002`) | Entry turn | 25.63 → 5.71 | 27.42 → 6.22 |
| Just Climb | Right-drag | 26.74 → 5.94 | 28.15 → 6.65 |
| Make Mine a Large One (`E18_KF_2`) | Entry turn | 29.32 → 7.17 | 30.59 → 7.72 |
| Make Mine a Large One | Right-drag | 29.31 → 7.28 | 30.78 → 7.80 |

The measurement harness called `runApplicationLeg → renderFrame → scheduleSceneFrame` for 180 poses per run. Entry used the selected hatch, the installed level height and `CameraIntro` at 1/60 s. The table covers its rotating portion, frames 61–179. Drag used `CameraController` with four logical pixels per frame and a world seeded at tick 180. The table covers frames 30–179. Across all 720 final draws, only the first draw of each process exceeded 16.67 ms (43.2–52.7 ms). Browser presentation, HUD, live simulation, input latency and display pacing are outside these scene times. [Exact CSV and captured log paths](../../build/web-perf-001/measurements.tsv) · [Before stage means](../../build/web-perf-001/before-summary.json) · [Final stage means](../../build/web-perf-001/final-summary.json) · [Harness](../../build/web-perf-001/measure.inc)

## Production changes and checks

- **Raster.** The browser preloads up to seven pthread workers before Start. `RasterCommandBatch::flush → RasterRows::run` distributes the existing disjoint bands across those workers and the caller. Each band keeps its triangle order, sampling, depth tests and pixel writes. The caller reduces counts and errors after the barrier. Windows keeps its 16-participant limit. Pixel preservation follows from this call path. [Web build](../../build/web-perf-001/distribution-build.log) · [Native build](../../build/web-perf-001/distribution-native-build.log)
- **Console.** The captured `backdrop: enabled=… pixels-depth-written=…` line occurred 720 times before and zero times after. Remaining captured lines described startup, first sprite draw or profiler shutdown. The web shell deduplicates stdout and stderr with a shared 32-message session limit. Save errors still reach the status display after the limit. A check against the actual page script emitted one copy of a warning repeated 1,000 times. [Console check](../../build/web-perf-001/console-check.log)
- **Audio.** `CuePlayback → Mixer::open → SDL_OpenAudioDevice` requests 4,096 web device frames. At 48 kHz the SDL Web Audio callback has an 85.33 ms block, up from 21.33 ms. In Node, the real mixer and SDL resampler fed a simulated stereo device with a looping bed and a one-shot. Both signals were present throughout consecutive output blocks across a 50.37 ms stall and refill. The buffer adds cue latency and its duration depends on device rate. A stall longer than the available audio can still underrun. Audible playback awaits the browser playtest. [Audio result](../../build/web-perf-001/device-audio.log) · [Device model](../../build/web-perf-001/audio-device.cjs)
- **Replay.** All 23 promoted recordings returned zero with `PASS won=1 completed=1` and matching final steps and saved counts on Windows and Wasm. Each ran with `--game-dir "Lemmings Revolution" --replay <file>` and private save locations. [Every recording and both results](../../build/web-perf-001/replays.tsv)

Static Ghidra evidence on the registered x86 EXE, SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`, places the entry initializer at `0x00444D1E` with a caller at `0x0045CAB3`. Its pseudocode and the two immediate stores at `0x00444FA2/0x00444FA9` agree on fallback yaw 128.0. This corroborates the retained entry-camera evidence. The worker count and audio buffer are browser tuning choices. [Static evidence](../../build/web-perf-001/rea-entry.json)

## Run the browser build

From the repository root, `python build/web-perf-001/serve-reviewed.py` serves the staged release at `http://127.0.0.1:8000/` through the project's HTTP handler. The worker could not inspect the existing shared build's JavaScript ACL, so the staged bundle uses a separate directory with inherited read access. The handler served all three distribution files and both local archives with cross-origin isolation headers. [HTTP checks](../../build/web-perf-001/server.log)

To rebuild the standard distribution in MSYS2 UCRT64 Bash, use the installed SDK and cached SDL sources. A writable project-local SDK cache holds the threaded libraries.

```sh
source /c/progs/emsdk/emsdk_env.sh
export EM_CONFIG=C:/progs/emsdk/.emscripten
export EM_PORTS=C:/progs/emsdk/upstream/emscripten/cache/ports
export EM_CACHE="$(cygpath -am build/web-sdk-cache)"
unset EM_FROZEN_CACHE
/c/progs/emsdk/upstream/emscripten/embuilder.bat build sysroot sdl2-mt
/ucrt64/bin/cmake.exe --preset emscripten-release -DCMAKE_MAKE_PROGRAM=C:/progs/msys2/ucrt64/bin/ninja.exe
/ucrt64/bin/cmake.exe --build --preset emscripten-release --parallel 8
"/c/Program Files/Python313/python.exe" scripts/serve-web.py
```

- **Handoff** — `needs_review`. Supervisor `verify.py --profile smoke` pending.
- **Next action** — Review the production diff, measured timings and supervisor result.

**What to check** — Enter Just Climb and Make Mine a Large One with music on. Watch each entry turn, then hold the right mouse button and rotate repeatedly while assigning a skill. Expect smooth rotation, continuous music, audible cues and a console that stays quiet during rotation.
