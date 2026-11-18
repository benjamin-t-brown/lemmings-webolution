# Make Mine a Large One — waterfall occlusion

**Needs review.** At node 26 (`E18_KF_2`), the back of a waterfall wrote depth over the opening hatch and hid every pixel of the first lemming. The four human recordings each reached a release before the player quit. The fix restores visibility from the selected hatch while preserving foreground occlusion. [Playtest 15 E1](../playtest/2026-11-18-playtest-15.md) · [Four attempts](../../build/large-one-scenery-001/human-attempts.tsv)

## Clone observations

The framebuffer probe used the installed level, `readSelectedHatchCamera` yaw 70, zoom 0, 96 cylinder rows, the production `CameraIntro`, door and cue gates, and 10 ms simulation frames. At intro time zero, three kind-7 records in the secondary `null2` holder at `0xA001B` owned 190,974 depth pixels. All use `Models\bg_mine01\^WaterFall01` with blend selector 1. At first release (5.810 s, settled), only the first obscured the play area. [Before](../../build/large-one-scenery-001/baseline/E18_KF_2.txt) · [After](../../build/large-one-scenery-001/corrected/E18_KF_2.txt)

| Node | Node / record offset in LVL | Intro pixels before | Facing dot at release | Final depth pixels at release, before → after |
| --- | --- | ---: | ---: | ---: |
| `transWaterFall_01_` | `0xA009C / 0xA016C` | 52,916 | −4.93678 | 299,844 → 0 |
| `transWaterFall_02_` | `0xA02CA / 0xA039A` | 45,024 | −2.21793 | 0 → 0 |
| `transWaterFall_03_` | `0xA4E36 / 0xA4F06` | 93,034 | +5.48518 | 0 → 0 |

At pixel (320,183) the first actor's origin depth was 324.349. Before the fix the waterfall stored 58.7234. Afterward the scene depth was 331.445 and all 97 previously hidden body pixels drew. The landing and walking samples restored 102 and 110 pixels. At 11.010 s two lemmings contributed 121 and 106 pixels. Turning to yaw 6 put nearer geometry in front of the first actor, hiding all 119 potential pixels and leaving 128 of 131 on the second. At yaw 198 the front of waterfall 02 still drew 402 pixels. [First release before](../../build/large-one-scenery-001/baseline/E18_KF_2-release.png) · [First release after](../../build/large-one-scenery-001/corrected/E18_KF_2-release.png) · [Second release and occlusion measurements](../../build/large-one-scenery-001/continued/second-release.txt)

## Static evidence and production change

Ghidra 12.1.4 opened the registered x86 `Lemmings Revolution/Lemmings Revolution.exe`, SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`. Dispatchers `0x469D71 / 0x469EAA` index the ordinary and clipped tables. Kind-7 slots `0x4C222C / 0x4C224C` lead through `0x469D53 / 0x469E8C` and `0x46A3A0 / 0x46A520` to `0x46B2A0 / 0x46B350`. Instructions confirm that `0x46B320` selects the first indexed vertex, `0x401830` subtracts it from the node-local eye at +0x1A8, and `0x418350` dots the result with the record's indexed normal. Both branches compare with zero and call their polygon emitter only for a positive result. Ghidra closed without a snapshot. [Static instructions, pseudocode, table bytes and xrefs](../../build/large-one-scenery-001/rea/) · [Retained eye-transform evidence](phase-c3ai-predicate-context.md)

`runApplicationLeg → renderFrame → scheduleSceneFrame → BackdropScene::renderStage(Secondary) → drawMesh` previously applied the facing test only to kinds 4/5. It now uses the same node-local test on the mine waterfall quads before either fan triangle can write color or depth. The kind-7 change is limited to that authored material across levels. Camera, actor composition, gate timing and the existing provisional depth policy were preserved. Original frame output and human playability await separate observation. [Renderer](../../src/app/BackdropScene.cpp) · [Prior scenery change](../../build/large-one-scenery-001/prior-scenery-git-show.txt)

## Checks

The four frame-rate levels (Just Climb, Sergeant Bash, Kriss Kross and The Crowded House) and One Way Ticket each matched their prior scene RGB and depth at two intro poses, release, landing, later walking, rotation, zoom and reverse. These are deterministic pre-HUD scene samples. A separate Release application run drew the target level through its normal window renderer with HUD at the settled pose. All runs used private LocalAppData. [40 comparisons and target deltas](../../build/large-one-scenery-001/view-comparisons.tsv) · [Window run](../../build/large-one-scenery-001/corrected/release-window.log) · [Window frame](../../build/large-one-scenery-001/corrected/release-window.png)

The registered UCRT64 Release target built. A new legacy test isolates waterfall 01 and checks that its rear is rejected and its front draws. It passed in 0.08 s. Changed files are `src/app/BackdropScene.cpp`, `src/__test__/test_waterfall_facing.cpp`, `CMakeLists.txt` and this report. [Build](../../build/large-one-scenery-001/build-final.log) · [Final target](../../build/large-one-scenery-001/final-target.log) · [Focused CTest](../../build/large-one-scenery-001/test-waterfall-facing.log)

All 23 promoted recordings returned exit 0, PASS, won=1 and completed=1 on the Release executable. Values below are saved/needed. [Exact recording paths, steps, results and invocation logs](../../build/large-one-scenery-001/replays.tsv)

| Recording | Result | Recording | Result | Recording | Result |
| --- | ---: | --- | ---: | --- | ---: |
| MM_BASH_002 | 10/10 | MM_FLOAT | 10/1 | MM_BUILD_BLOCK_A | 15/10 |
| MM_BASH_MINE_WLR1 | 20/10 | MM_MINE | 20/10 | E19_KF_2 | 24/20 |
| MM_BLOCK_001_A | 24/15 | MM_W_012 | 16/10 | E20_MG_1 | 7/5 |
| MM_BLOCK_BOMB_A | 12/10 | MM_E1_3_SKILLS | 10/5 | MM_ALL_8_001 | 46/40 |
| MM_BOMB | 17/10 | M06_KF_1 | 91/50 | MM_E1_5_SKILLS | 16/16 |
| MM_BUILD | 20/10 | MM_E1_4SKILLS | 20/10 | MM_ALL_8_002 | 19/10 |
| MM_CLIMB_002 | 10/1 | MM_E2_4SKILLS | 14/10 | MM_ALL_8_003 | 21/20 |
| MM_DIG_000 | 19/10 | MM_BASH_DIG_001_A | 20/10 | | |

- Handoff — `needs_review`. Supervisor smoke and recovery are pending.
- Next action — independent review after the gate, then the user's playtest.

**What to check** — Enter Make Mine a Large One from node 26. Watch the first lemming fall from the hatch and walk along the steel ledge. Rotate and zoom through the next release. Both lemmings should be visible on the near side and disappear behind solid scenery as the tube turns.
