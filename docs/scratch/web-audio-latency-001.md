# Browser assignment audio latency

Playtest 20 K3 reported a Climber click arriving roughly 100–300 ms after assignment. The probe used Just Climb (`MM_CLIMB_002`) because K3 gives no level. Playtest 17 confirmed that the earlier 4,096-frame SDL web buffer fixed the clipping reported in playtest 16 F3. That buffer lasts 85.33 ms at 48 kHz. [K3](../playtest/2026-11-19-playtest-20.md) · [F3](../playtest/2026-11-18-playtest-16.md) · [Confirmation](../playtest/2026-11-18-playtest-17.md) · [Earlier stall model](web-perf-001.md)

## Production path

A left press on the browser canvas can wake the level's SDL loop before the next animation frame. The loop uses the displayed scene for coordinate translation, hatch and panel routing, and `hoveredActor`. A successful `World::assign` queues `Assign`. The handler drains it through `CuePlayback::playName → Mixer::play → weboAudioPlay → AudioBufferSourceNode.start`. The pending animation frame drives simulation and rendering. Looping music and one-shots use an interactive Web Audio context with browser-managed buffering. Playback adds no pthread calls. Desktop audio retains the 1,024-frame SDL request and existing voice policies. [Level input](../../src/app/main.cpp#L1369) · [Browser wait](../../src/web/Browser.cpp#L12) · [Web mixer](../../src/web/WebMixer.cpp) · [Audio graph](../../src/web/web-audio.js)

The installed Action clip crosses 256 S16 counts at 53.70 ms. The web mixer provisionally trims its first 53 ms if the discarded samples peak at 128 counts or less. The installed clip loses 1,168 frames at 22,050 Hz, with a peak of 117 counts in that prefix and −84 at the first retained sample. The attack stays intact. [PCM analysis](../../build/web-audio-latency-001/action-trim-53.json)

Retained Ghidra pseudocode for `0x00455657` agrees with instructions at `0x004556A2` (failed assignment bypasses playback), the Action handle load at `0x004556AA` (xref to `0x004E32FC`), and the call at `0x004556B5` to `0x0044521B`. This supports cue routing. The registered x86 EXE SHA-256 is `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`. The retained REA session closed without a snapshot. [Static evidence](../../build/web-audio-latency-001/rea-assignment.json)

## Measurement

Emscripten 5.0.3 and Node 22.16.0 ran the clone's SDL web mouse callbacks and level event loop. The probe emitted coordinate-bearing canvas events at varied phases of a modeled 60 Hz clock, with CSS scaling and a 2× window. A frame observer supplied screen coordinates from the rendered scene and checked the actor's Climber capability after the game picked and assigned it. Each Web Audio variant succeeded on 21 clicks. The old SDL comparison combines these measured input stages with the retained 32-trial mixer and SDL resampler run. [Probe](../../build/web-audio-latency-001/picking-probe.cpp) · [Node driver](../../build/web-audio-latency-001/picking-node.cjs) · [Retained SDL transport](../../build/web-audio-latency-001/before-node.json) · [Per-click stages and totals](../../build/web-audio-latency-001/picking-latency-summary.json)

| Stage, ms median / max | 4,096-frame SDL | Web Audio, frame polling | Final Web Audio, click wake |
| --- | ---: | ---: | ---: |
| DOM callback through SDL enqueue | 0.03 / 0.12 | 0.02 / 0.12 | 0.03 / 0.15 |
| SDL queue to poll | 9.92 / 16.71 | 9.82 / 16.71 | 0.06 / 0.26 |
| Poll through coordinates, routing, pick and World cue | 0.07 / 0.25 | 0.07 / 0.25 | 0.06 / 0.23 |
| World cue to Action route | 0.01 / 0.13 | 0.01 / 0.13 | 0.01 / 0.14 |
| Route to mixer admission or source start | <0.01 / 0.02 | 0.07 / 0.17 | 0.06 / 0.22 |
| Mixer admission to first PCM callback carrying Action | 100.33 / 213.33 | — | — |
| PCM callback to SDL output block carrying Action | 85.33 / 85.33 | — | — |
| ScriptProcessor playback lead | 85.33 / 85.33 | — | — |
| Onset inside ScriptProcessor block | 45.52 / 83.85 | — | — |
| Source start to next graph quantum | — | 1.33 / 1.90 | 1.53 / 2.65 |
| Graph quantum to 256-count attack | — | 3.69 / 3.69 | 0.71 / 0.71 |
| Click to modeled graph onset | 290.79 / 376.69 | 11.75 / 19.69 | 1.92 / 2.98 |
| **Click to estimated output onset** | **332.79 / 418.69** | **53.75 / 61.69** | **43.92 / 44.98** |
| Click to estimated output attack | — | 56.40 / 64.33 | 44.63 / 45.69 |

A silent WASAPI render probe on this Windows session measured a 480-frame callback period at 48 kHz and a 31.98 ms median / 32.00 ms maximum endpoint queue using Chromium's written-minus-played frame method. The 42 ms output estimate uses the 32 ms maximum plus one 10 ms packet for Chromium's audio service, which normally requests the next packet after reading the current one. A second calculation sweeps all phases of the 480-frame callback and its 0–96 frame carry at a 128-frame Web Audio quantum, using Chromium's Windows graph bypass. It puts the final onset at **48.33 ms median / 54.89 ms max** and the attack at **49.04 ms median / 55.59 ms max**. [Endpoint probe](../../build/web-audio-latency-001/endpoint.cpp) · [Endpoint samples](../../build/web-audio-latency-001/endpoint.json) · [Chromium WASAPI](https://raw.githubusercontent.com/chromium/chromium/main/media/audio/win/audio_low_latency_output_win.cc) · [Chromium audio service](https://raw.githubusercontent.com/chromium/chromium/main/services/audio/output_controller.cc) · [Windows graph default](https://chromium.googlesource.com/chromium/src/third_party/+/5372303cf79204894bed06b2f5321f81cc5adac7/blink/renderer/platform/runtime_enabled_features.json5) · [Graph rendering](https://github.com/chromium/chromium/blob/main/third_party/blink/renderer/platform/audio/audio_destination.cc)

The retained headless Chrome run failed during GPU process startup. Browser clock and acoustic output remain unmeasured. A long main-thread stall before DOM dispatch can still delay a new click. [Retained Chrome log](../../build/web-audio-latency-001/before-audio-only-chrome-stderr.log)

## Checks

- The final music bed stayed at 0.15 with zero drops across standalone 51.02 and 67.00 ms stalls and all 49,648 sample frames of a 60-frame rotation model. The earlier SDL refill test used a 50.37 ms stall. [Final continuity run](../../build/web-audio-latency-001/revision-rotation-node.log) · [Earlier model](../../build/web-perf-001/device-audio.log)
- Windows and Wasm release targets built. All 24 promoted recordings returned `PASS won=1 completed=1` on both targets with matching final steps and saved counts, using private save paths. [Windows build](../../build/web-audio-latency-001/revision-native-build.log) · [Wasm build](../../build/web-audio-latency-001/revision-web-build.log) · [Every recording and result](../../build/web-audio-latency-001/revision-replays.tsv)
- The previous supervisor run passed 95 smoke tests and 84 recovery tests. The smoke gate for this revision is pending. [Previous supervisor run](../../build/agent-runs/20261119T195252-53088-26367d4d/verification.log)

This revision changed `src/app/main.cpp`, `src/web/Browser.cpp`, `src/web/Browser.h`, `src/web/WebMixer.cpp` and this report. [Task evidence](../../build/web-audio-latency-001/)

**Handoff** `needs_review`. Supervisor `verify.py --profile smoke` pending.  
**Next action** Review the dispatch and timing evidence, then check the browser build by ear.

**What to check** Open Just Climb in the browser with music on. Listen through the entry turn, right-drag the camera, then release and assign Climber. Expect continuous music through both turns and a prompt Action click on the successful assignment.
