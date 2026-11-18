# Level play at 144 Hz

**All 15 measured means fell below the 6.94 ms budget.** Kriss Kross rotation fell from 17.592 to 6.401 ms. Its p95 was 7.279 ms and its maximum was 7.693 ms. It accounted for all 32 over-budget frames in the 3,000-frame final sample. This addresses the playable-v1 performance request in [playtest 14 D5](../playtest/2026-11-18-playtest-14.md).

## Frame times in milliseconds

| Installed level | Mode | Before mean | Before p95 | Before max | After mean | After p95 | After max |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Just Climb (MM_CLIMB_002) | Rest | 6.298 | 6.831 | 7.282 | 2.304 | 2.522 | 2.804 |
| Just Climb | Rotate | 11.304 | 12.323 | 13.077 | 4.632 | 5.031 | 5.651 |
| Just Climb | Fast-forward | 6.252 | 6.681 | 6.984 | 2.283 | 2.467 | 2.697 |
| Sergeant Bash (MM_E1_4SKILLS) | Rest | 5.811 | 6.269 | 7.082 | 2.317 | 2.524 | 3.019 |
| Sergeant Bash | Rotate | 11.586 | 12.350 | 13.256 | 4.845 | 5.216 | 5.562 |
| Sergeant Bash | Fast-forward | 6.263 | 6.711 | 7.481 | 2.377 | 2.669 | 3.180 |
| Kriss Kross (E19_KF_2) | Rest | 10.225 | 11.107 | 13.557 | 4.428 | 4.913 | 5.331 |
| Kriss Kross | Rotate | 17.592 | 20.434 | 22.263 | 6.401 | 7.279 | 7.693 |
| Kriss Kross | Fast-forward | 9.876 | 11.269 | 11.718 | 4.434 | 5.011 | 5.542 |
| The Crowded House (E20_mg_1) | Rest | 6.692 | 7.101 | 7.637 | 2.787 | 3.096 | 3.611 |
| The Crowded House | Rotate | 12.194 | 13.730 | 14.967 | 5.077 | 5.683 | 6.006 |
| The Crowded House | Fast-forward | 6.812 | 7.443 | 8.032 | 2.755 | 3.008 | 3.298 |
| One Way Ticket (MM_ALL_8_001) | Rest | 12.046 | 12.716 | 14.320 | 3.748 | 4.008 | 4.304 |
| One Way Ticket | Rotate | 15.694 | 19.104 | 21.507 | 5.834 | 6.303 | 6.769 |
| One Way Ticket | Fast-forward | 12.734 | 14.291 | 18.465 | 3.926 | 4.220 | 4.678 |

Both runs used the registered MinGW-w64 GCC 16.1.0 Release build (`-O3 -DNDEBUG`, C++23), a Ryzen 9 9950X3D, a 640×480 CPU raster and a shown 1280×960 SDL window on the 144 Hz display. The retained harness seeded 180 simulation ticks, supplied 1/60 s per frame, disabled audio and drew 240 frames. The table uses frames 30–229 inclusive and the nearest-rank p95. Rotation supplied eight physical drag pixels per frame. Fast-forward held plate 8 across the sample to exercise the current hold-to-speed control. Rest/rotation ended at 40,000 ms of level time and fast-forward at 63,904 ms on both builds. Every run used the task-private LocalAppData directory. [Harness](../../build/frame-rate-144-001/measure.cpp) · [Before build](../../build/frame-rate-144-001/before-build.log) · [Final build](../../build/frame-rate-144-001/review-build.log) · [All 30 CSV and log paths](../../build/frame-rate-144-001/timing-inventory.json)

## Where rotation improved

Kriss Kross (`E19_KF_2`) rebuilds view-dependent scenery while the camera turns. The exclusive mean costs below include worker dispatch and completion. The revised raster bucket also includes counting changed pixels within each band, previously charged to preparation. Simulation, particles and other frame bookkeeping account for the remainder. [Before stage means for all modes](../../build/frame-rate-144-001/before-summary.json) · [Final stage means for all modes](../../build/frame-rate-144-001/review-summary.json)

| Kriss Kross rotation stage | Before ms | After ms |
| --- | ---: | ---: |
| Preparation and texture decode | 5.306 | 1.918 |
| Backdrop, tube and fluid raster | 6.960 | 3.104 |
| Actors, footprint and carried bricks | 3.885 | 0.369 |
| HUD | 0.780 | 0.523 |
| SDL upload, copy and present | 0.566 | 0.409 |

- `runApplicationLeg → renderFrame → scheduleSceneFrame` retains exact camera and terrain-revision cache hits. On a moving-camera miss it draws opaque tube fragments directly against live scenery and postpones filling the caches until the pose repeats. Strict nearer-depth wins and equal depth preserves the earlier fragment. A primary cache hit replaces the entire target, eliminating the preceding clear.
- `BackdropScene::renderStage` and `TubeScene::render` submit clipped Y extents to `RasterCommandBatch`. Up to 16 participants take eight-row bands, each in triangle submission order. Extreme interpolation inputs retain the earlier eight error ranges. Each worker takes its own before/after snapshot for the existing pixel counters. `buildSceneMeshSource` now finds the last attribute store for each vertex through its validated position index.
- `TubeScene::drawInjectedWalkers` reuses body scratch, clears the sampled rectangle and commits accepted opaque texels in the existing actor order. Drowning bodies and bubbles use the same bounded scratch for additive draws. Carried geometry retains its expanded bounds. `ActorDepth` allocates touched 64-pixel blocks and preserves nearer-depth and owner tie rules.
- The level presenter uploads composed RGBX directly. Its channel masks discard the fourth byte exactly as the old RGB24 packing did. Backends that reject RGBX retain the RGB24 upload. Resolution, effects, animation, simulation, input, audio and frame pacing retain their existing policies.

## Output and replay checks

The retained dense-render benchmark sampled five levels at two zooms with both debris presentations. Its 160 stage records matched for RGB, bitwise scene depth, actor depth and ownership. The entry sample used the existing pre-task `build/agent-native/main` production objects (`-O2 -g`) and composited a separate opaque tube layer. The final sample used the Release objects and drew the tube directly. Both used identical installed meshes, edits, builder poses and scenery-generator inputs. Only the measurement drivers were compiled against the corresponding headers for this comparison. [Entry stage records](../../build/frame-rate-144-001/entry-sample_layers/stages.csv) · [Final stage records](../../build/frame-rate-144-001/review-sample_layers/stages.csv) · [Sampling source](../../build/frame-rate-144-001/sample_layers.cpp)

The application-frame audit inspected frames 81 and 229 in all 15 modes, including a drowning actor on Kriss Kross. It found 18 byte-identical PNGs. The other 12, on Kriss Kross and One Way Ticket, differed only at actual sprite writes from the independently clock-seeded scenery animation. All 75 gameplay, release, frame-state, action and outcome files matched. These are sampled clone comparisons. Audible playback, arbitrary player workloads and original-game fidelity remain unmeasured. [Frame comparison](../../build/frame-rate-144-001/review-composed-frames.json) · [Binary and evidence digests](../../build/frame-rate-144-001/evidence-summary.json)

All 21 promoted recordings returned exit 0, `PASS`, `won=1` and `completed=1` on `build/cmake/gcc-release/webolution.exe --game-dir "Lemmings Revolution" --replay <file>`. The table gives saved/needed. [Every filename, command, final step and log](../../build/frame-rate-144-001/review-replay-results.json)

| Recording | Saved/needed | Recording | Saved/needed |
| --- | ---: | --- | ---: |
| `MM_BASH_002` | 10/10 | `MM_E1_3_SKILLS` | 10/5 |
| `MM_BASH_MINE_WLR1` | 20/10 | `M06_KF_1` | 91/50 |
| `MM_BLOCK_001_A` | 24/15 | `MM_E1_4SKILLS` | 20/10 |
| `MM_BLOCK_BOMB_A` | 12/10 | `MM_E2_4SKILLS` | 14/10 |
| `MM_BOMB` | 17/10 | `MM_BASH_DIG_001_A` | 20/10 |
| `MM_BUILD` | 20/10 | `MM_BUILD_BLOCK_A` | 15/10 |
| `MM_CLIMB_002` | 10/1 | `E19_KF_2` | 24/20 |
| `MM_DIG_000` | 19/10 | `E20_MG_1` | 7/5 |
| `MM_FLOAT` | 10/1 | `MM_ALL_8_001` | 46/40 |
| `MM_MINE` | 20/10 | `MM_E1_5_SKILLS` | 16/16 |
| `MM_W_012` | 16/10 |  |  |

**Pacing recommendation** — Offer display-synchronized presentation as an opt-in setting for the 144 Hz monitor. Kriss Kross can still miss a refresh during rotation. This build keeps the uncapped, vsync-off presenter.

**Changed files** — `src/app/{ActorDepth.h,BackdropScene.cpp,Raster.cpp,RasterRows.h,SceneMeshSource.cpp,SceneRaster.cpp,TubeScene.cpp,main.cpp}` and this report. Incoming edits are preserved. [Task-only diff](../../build/frame-rate-144-001/task-only.patch)

**Handoff** — `frame-rate-144-001` is `needs_review`. All 15 means meet budget and all 21 promoted replays pass.

**Next action** — The supervisor runs the pending smoke and recovery gate after worker exit, followed by independent review.

**What to check** — On Kriss Kross, rotate past the water and hatches, stop, then reverse direction and hold fast-forward. On One Way Ticket, rotate and zoom against the blue sky. Expect responsive motion, stable scenery as the camera stops, correct actor occlusion and an unchanged HUD. Watch for occasional late frames on Kriss Kross.
