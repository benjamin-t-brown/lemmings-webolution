# emscripten-build-001 · Local browser build

The November 18 web-platform request builds the playable-v1 clone with the registered Emscripten 5.0.3 SDK. The lead’s trial failed at SDL includes and at the native-only `lrtool` constexpr math. SDL2 now reaches game compiles and the link, and the web preset builds only the game.

## Build and serve

Run these commands from the repository root in MSYS2 UCRT64 Bash.

```sh
source /c/progs/emsdk/emsdk_env.sh
export EM_CONFIG=C:/progs/emsdk/.emscripten EM_FROZEN_CACHE=1
/ucrt64/bin/cmake.exe --preset emscripten-release -DCMAKE_MAKE_PROGRAM=C:/progs/msys2/ucrt64/bin/ninja.exe
/ucrt64/bin/cmake.exe --build --preset emscripten-release --parallel 8
"/c/Program Files/Python313/python.exe" scripts/serve-web.py
```

Open [the local game](http://127.0.0.1:8000/). The three distribution files are in `build/cmake/emscripten-release/`. The loopback server fetches nothing externally. It serves `lemmings.box` and `patch.box` directly from the local installation so a rebuild needs no asset copies or packages. Startup loads both archives into browser memory (316 MiB in total). The installed save and the rest of the installation have no HTTP route.

## Production paths

- **Frames and sound.** After storage and archives are ready, **Start game** calls `main → runApplication` inside the click or keyboard gesture. The existing `CuePlayback → Mixer → SDL_OpenAudioDevice` path starts audio there. Asyncify yields to `requestAnimationFrame` in the frontend, map and level loops. The same C++ session, clocks and SDL window survive each yield and level transition.
- **Raster.** On Wasm, one caller drains every `RasterCommandBatch::flush` band. It uses the existing row boundaries, triangle order, pixel operations and error reduction. Windows keeps its worker pool. Browser rendering speed and visible output await the playtest.
- **Saves.** The page holds one Web Lock per origin and populates IDBFS before it enables Start. `FrontendSession → SaveStore → FileSaveSink` uses the existing 1,504-byte codec. Accepted sound settings use the same sink for the existing three-byte sidecar. Both live under `/store/LemmingsWebolution/`. File replacement waits for the IndexedDB transaction and restores the prior in-memory file on failure. Storage initialization or decode errors stop browser startup with a message. Use the same browser profile and URL on reload.
- **Static evidence.** On the registered x86 EXE, SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`, Ghidra lists xrefs to the save writer at `0x00485E65` and `0x0049BCFE`. Instructions in `0x0045F94D..0x0045FADD` confirm the decompiler’s loop bounds of 5 groups × 25 slots, three writes per slot after the leading flag. The browser uses the recovered codec. All original evidence here is static. [REA evidence](../../build/emscripten-build-001/rea-save.json)

## Verification

The registered SDK linked the web game from its frozen cache. GCC rebuilt the Windows game target. A PowerShell policy blocked the optional wrapper before configuration, so the successful builds used the registered UCRT64 Bash directly. [Configure](../../build/emscripten-build-001/configure.log) · [Full web compile](../../build/emscripten-build-001/web-build.log) · [Final web link](../../build/emscripten-build-001/web-distribution.log) · [Native build](../../build/emscripten-build-001/native-build-final.log)

The loopback handler served the game page, Wasm MIME type, both archives and COOP/COEP headers. A Node/Wasm storage probe ran two asynchronous frame yields, round-tripped nonzero save fields and settings, injected a sync error, checked the prior bytes and retried successfully. The probe used private MEMFS and a simulated sync callback. Real IndexedDB reload, audible output, input and frame pacing require the desktop-browser playtest. [HTTP results](../../build/emscripten-build-001/server.log) · [Storage probe](../../build/emscripten-build-001/store-probe.log)

All 23 promoted recordings under `docs/playtest/replays/` returned zero, `PASS won=1 completed=1` and the same final step on both targets. Each command used `--game-dir "Lemmings Revolution" --replay <recording>`. Windows ran `build/cmake/gcc-release/webolution.exe`. The registered `C:/progs/emsdk/node/22.16.0_64bit/bin/node.exe` ran the browser build’s `build/cmake/emscripten-release/webolution.js` with the working directory mounted through NODEFS. Replay exits before opening a save. These are clone regression results. [Per-recording paths, exit codes and results](../../build/emscripten-build-001/replays.tsv)

| Playtest | Installed level | Final step on both | Windows saved/required | Wasm saved/required |
| --- | --- | ---: | ---: | ---: |
| 7 | `MM_BASH_002` | 1844 | 10/10 | 10/10 |
| 7 | `MM_BASH_MINE_WLR1` | 2272 | 20/10 | 20/10 |
| 7 | `MM_BLOCK_001_A` | 1619 | 24/15 | 24/15 |
| 7 | `MM_BLOCK_BOMB_A` | 1546 | 12/10 | 12/10 |
| 7 | `MM_BOMB` | 954 | 17/10 | 17/10 |
| 7 | `MM_BUILD` | 1685 | 20/10 | 20/10 |
| 7 | `MM_CLIMB_002` | 1252 | 10/1 | 10/1 |
| 7 | `MM_DIG_000` | 1014 | 19/10 | 19/10 |
| 7 | `MM_FLOAT` | 978 | 10/1 | 10/1 |
| 7 | `MM_MINE` | 1137 | 20/10 | 20/10 |
| 7 | `MM_W_012` | 1742 | 16/10 | 16/10 |
| 8 | `MM_BASH_DIG_001_A` | 3241 | 20/10 | 20/10 |
| 8 | `MM_BUILD_BLOCK_A` | 2482 | 15/10 | 15/10 |
| 10 | `MM_E1_3_SKILLS` | 2095 | 10/5 | 10/5 |
| 11 | `M06_KF_1` | 1544 | 91/50 | 91/50 |
| 11 | `MM_E1_4SKILLS` | 2506 | 20/10 | 20/10 |
| 11 | `MM_E2_4SKILLS` | 2142 | 14/10 | 14/10 |
| 12 | `E19_KF_2` | 6034 | 24/20 | 24/20 |
| 12 | `E20_MG_1` | 12810 | 7/5 | 7/5 |
| 13 | `MM_ALL_8_001` | 13812 | 46/40 | 46/40 |
| 14 | `MM_E1_5_SKILLS` | 12922 | 16/16 | 16/16 |
| 15 | `MM_ALL_8_002` | 18659 | 19/10 | 19/10 |
| 15 | `MM_ALL_8_003` | 18207 | 21/20 | 21/20 |

The first supervisor run passed all 95 main smoke tests and all 84 recovery tests. Review then found that `*.js` hid `src/web/node-mount.js`, a required `--pre-js` link input. A file-specific `.gitignore` exception now makes the source visible to Git. The generated JavaScript, Wasm and both installed archives still match ignore rules. This revision changed no build or runtime code. [Previous smoke run](../../build/agent-runs/20261118T220800-50424-e6c25e58/verification.log) · [Source inclusion check](../../build/emscripten-build-001/source-inclusion.txt)

- **Handoff** — `needs_review`. The supervisor’s `verify.py --profile smoke` gate for this revision is pending.
- **Next action** — Review the source inclusion check and the new smoke result, then run the human browser playtest.

**What to check** — Build and serve with the commands above. Open [the local game](http://127.0.0.1:8000/), choose **Start game**, change and accept a sound setting, then play and win **Just Climb** (`MM_CLIMB_002`). Listen for music and assignment sounds. After results, reload the same URL and select **Play Current Game**. Expect the map to retain the win and Options to show the saved volume.
