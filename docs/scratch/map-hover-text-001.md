# map-hover-text-001: the three-line map hover block

2026-11-15. **needs_review; full registered main/recovery verification is pending
the supervisor.** Playable v1 step 8, [playtest 4 T3](../playtest/2026-11-13-playtest-4.md).
Difficulty: routine to difficult; recommended model: Sonnet-class, as assigned.
This invocation used the explicit Codex provider override.

The previous accepted P14 implementation displayed only a title and drew red
over black. The user's [Just Climb reference](../playtest/reference/leveltext.png)
and [Bash and Mine reference](../playtest/reference/original-2026-11-14/map-0140.jpg)
show counts, name and a clock/time row, with black text over a red shadow.
Completing that block is the next dependency for T3; map navigation and launch
rules are unchanged. This is a scoped visual correction, not a historical fidelity
gate pass or evidence that all levels are human-playable.

## Production path and behavior

`runApplication` -> `runFrontendWindow` -> public `SDL_MOUSEMOTION` ->
`FrontendSession::setPointer` -> `composeFrame` -> unchanged
`MapScene::pickNode` -> new `MapScene::drawHover` -> recovered
`shell::drawHud` / `fontCells` and existing `compositeIconRegion` -> frontend
cursor -> ordinary SDL texture presentation. The regression driver reaches this
path through New Game confirmation and Play Current Game, not a private hover
setter or direct framebuffer injection.

`MapScene::build` already joined `LEVELS\\LEVELS.MAP` node launch IDs with
`COMPILED\\LEVELMAP.DAT` display names. It now caches all three formatted strings
from the same `LevelInfo` row and crops the clock from the installed map atlas.
`FrontendSession` draws them only for the existing eligible state2/3 mask hit.
Each frame starts from the cached clean map, so moving off-node removes the
entire block. Pointer picking, unlock state, launch IDs, footer, cursor and saves
retain their existing paths. The baseline and final driver both still launch
`MM_CLIMB_002` after the hover route.

| Node / public pointer | Counts | Name | Time |
| --- | --- | --- | --- |
| 0 / (209,203), fresh | 10 Lemmings : Save 1 | Just Climb | 5:00 |
| 1 / (209,236), fresh | 10 Lemmings : Save 1 | Just Float | 5:00 |
| 2 / (209,269), fresh | 20 Lemmings : Save 10 | Just Bomb | 5:00 |
| 10 / (275,269), private all-progress fixture | 20 Lemmings : Save 10 | Bash and Mine | 10:00 |

All 102 cached triples are asserted against independently formatted installed
metadata. Exact three-string assertions cover both supplied original references.
For all eligible public motion frames, an independent literal glyph-alpha and
atlas/key pixel oracle checks `composeFrame` byte for byte. The former two-pass
title oracle changed specifically because T3 demonstrates its missing rows and
reversed foreground/shadow; no assertion or timeout was weakened.

## Static evidence and assets

Starting evidence: [accepted map-name research](map-level-names-001.md), its
`build/map-level-names-001/static/` records, [frontend lessons](../lessons/frontend.md),
and the [save-format record layout](save-format-research.md). Existing evidence
was read before targeted static lookups. Complete new REA results and copies of
the relevant retained font/layout records are under
`build/map-hover-text-001/static/`. Registered reference:
`Lemmings Revolution/Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Provider: Ghidra 12.1.4, profile digest
`3c06738e45e144d5f073edbd3244c6bff46222f389525fe659bc92dca2ccdece`.
These are static instruction/decompiler reads; analysis was closed without
snapshot arguments. No original code was executed.

| Question | Evidence / implementation |
| --- | --- |
| Font | Retained `4898EC` -> `48B650` -> `486A9C`, instruction `486A9F` reads `[4CBF78]`, the accepted default `^Font`. Reuse installed `TEXTURES\\^FONT.BMP` plus `.ALF`, the recovered 0x20..0x7E table at `4BF968+4`, line height29, and the existing alpha renderer. Installed font and map-atlas PNG measurements are retained by the registered hover test. |
| Position and alignment | `49AC47` instructions `49AF2D..49AF74`, `49AFB6..49B02A`: black `(30,0)/(30,20)/(30,60)`; red `(32,2)/(32,22)/(32,62)`. Flag0x100 and retained `48A720` establish left alignment. Setup instruction Evidence ID `ev_88a268f69ad07c27dd2c94b48edf27c3f4554503464df8b9dd70773393d6f76e`. |
| Colour / order | Setup passes black `(0,0,0)` and red `(220,50,50)` at `49AFEA..49AFF3`, hence a `(2,2)` shadow offset. Both supplied original images visibly establish black foreground over red shadow, correcting the previous report's provisional black-then-red choice. Draw shadow first and foreground last. |
| Clock | `49B7B0` pseudocode and instructions agree: source map+74 is the installed `TEXTURES\\LEVEL SELECT OBJECTS.BMP`; table `4B3EA0` is `{193,0,28,31,14,15}`. Its source rectangle is `[193,0,221,31)`, destination `(0,60)`, black source colour key (SetColorKey flag8, bounds0..0), fast blit flags0x11. Evidence IDs `ev_8142004e0471efd31202b88153a8e56e4adabbec4edafdba9346f0d8b47fb750` and `ev_84f2676e2dfdbb01be791d3259f21fd8ffcf0ebf6010d8a9ae5e4a6ac8069f74`. |
| Formats | Bytes at `4C44E4`: `%d:%02d`; at `4C44EC`: `%d Lemmings : %s %d`. `49B516..49B530` obtains localized string0x7B (123), installed UK `Save`, total, then target. Time uses signed divide/modulo60 at `49B43E..49B461`, with unpadded minutes. Evidence IDs `ev_56c9d050358040c5760abeba3f4a031296470a026756385b15e7a3a465508af0` and `ev_4ecb0e4d2b84e5a75cf7eff1241a2cedafb5d393a2e2f2926f56d5750958a289`. The clone uses equivalent formatting of the installed supported nonnegative values. |
| Counts/time source | Loader `45F43C` reads `Compiled\\LevelMap.dat` as five groups of25, stride0x118, group stride0x1B58. Within each record it reads id, title, then time to +0x100, save target to +0x108, total to +0x104. Hover `49B2F6` reads title `4E3654`, time `4E36D4`, total `4E36D8`, target `4E36DC`, plus group/slot strides. Instructions `45F554..45F5F7` cross-check the reordered numeric stores; `49BAF0/49BB00` and `49BB10/49BB20` cross-check target/total setters/getters. Loader evidence IDs `ev_71adbbd506994f5661d64c65d1eacde016580e800ae089e7f2895343859c2914` and `ev_fe74bc82777af3f4b3766082362de8d94a745637d263174f0cfe70fb0cdf42fc`. Production `parseLevelMap` already exposes these as literal seconds, target and total. No simulation or release tuning is involved. |

## Retained clone evidence and checks

Authentic task-entry sources and hashes are frozen under
`build/map-hover-text-001/before-source/` and `before-manifest.json`; the
pre-existing working diff/status are also retained. The frozen baseline includes
the already accepted frame-consumption edits to `main.cpp`; they were preserved,
not replaced with HEAD. Its optional CTest target compiles the frozen map/session
headers and sources plus frozen main with the final public-input driver.

- Task-entry existing hover test: `build/map-hover-text-001/task-entry-ctest.log`,
  1/1 passed before production edits.
- Initial targeted build exposed missing text/compositor dependencies in the
  small standalone mapscene target. `CMakeLists.txt` now supplies those dependencies
  to mapscene and savedgame. Corrected GCC16.1 -O2/-g assertion-enabled build,
  parallel8: `build/map-hover-text-001/fixed-targeted-build.log`, passed.
- `build/map-hover-text-001/targeted-ctest.log`: **6/6 passed**: mapscene,
  frontendsession, savedgame, maphover, historical maphover_before, and the new
  title-only maphovertext_before baseline. Original timeouts are unchanged.
- `build/map-hover-text-001/comparison-ctest.log`: **1/1 passed**,
  `maphovertext_compare`, also registered for supervisor verification with
  dependencies on the final and title-only drivers. Its output is
  `build/agent-native/main/map-hover-text-comparison/image-comparison.json`.
- 114 no-hover frame hashes and every cached base frame are unchanged. All109
  eligible hover visits change. 21 retained PNG pairs are pixel-compared; unchanged
  pairs are also byte-identical, and every changed pair is confined to the
  top-left block, within `[0,0,640,91)`. This includes off-node after visits,
  locked-node, footer, edge and focus-loss cases. Ordinary selection still works.
- Each baseline/final summary records selectable_nodes102, level_frames1,
  failures0, installed_save_unchanged1, normal_save_unchanged1. Only unique private
  test-directory saves are used; hover makes no writes. The all-progress fixture
  preserves its existing A/B/flag fields; it does not claim natural wins.
- Task changes pass `git diff --check` and LF checks. Final source identities:
  `build/map-hover-text-001/final-source-manifest.json`.

Exact visual runs:

- Before: `build/agent-native/main/map-hover-text-before-test/run-1794760474115101600/`.
- After: `build/agent-native/main/map-hover-test/run-1794760471402480500/`.

Reviewed side by side, with labeled clone/reference panels:

- [Just Climb: before / after / supplied original](../../build/map-hover-text-001/just-climb-comparison.png).
  Clone crops are explicitly enlarged3x nearest for the enlarged supplied crop.
- [Bash and Mine: before / after / supplied original](../../build/map-hover-text-001/bash-and-mine-comparison.png).
  Native-size640x480 frames; the clone all-progress fixture differs from original
  progress state, so lamp/cursor differences outside the block are expected.
- [Just Float final frame](../../build/agent-native/main/map-hover-test/run-1794760471402480500/fresh-node-1.png)
  and [Just Bomb final frame](../../build/agent-native/main/map-hover-test/run-1794760471402480500/fresh-node-2.png)
  show their own metadata rather than the hovered first node's values.

Visual review finds the expected rounded font, black foreground/red shadow,
three-row placement, counts, clock and time. This does not claim numerical
original-frame equivalence: the supplied images have different capture/scaling
and JPEG characteristics. The sheet assembly used installed System.Drawing
inline commands; the retained `.ps1` helper could not be launched under the
host's script policy, and no execution policy was changed.

## Limits, preservation and handoff

Provisional choices are labelled in production comments: the existing integer
alpha/compositor sampling and hide-on-focus-loss policy. A matched original
native-resolution lossless frame for the same node, plus a focus transition,
would settle exact sampler/quantization/focus behavior. The supplied references
settle visible foreground/shadow order, not every native widget ordering case.
Scope is the installed English/ASCII map; no broader localization or malformed
metadata semantics are claimed. Static pseudocode, instruction checks, installed
asset measurements, clone observations and supplied original images remain
separate evidence types.

Changed task files: `src/app/MapScene.{h,cpp}`, `src/app/FrontendSession.cpp`,
`src/__test__/test_maphover.cpp`, `CMakeLists.txt`, `tools/compare_map_hover.py`,
and this report. The one header change was made early before the first build.
The supervisor holds RepositoryLease (PID37880); no protected controls were edited.
Pre-existing edits, including controller-owned task/status changes, hooks log and
accepted frame-consumption files, were preserved. No original game, oracle,
debugger, hook, capture, installation, subagent, commit or push was run.

Handoff: implementation and scoped checks are ready for needs_review; full registered main/recovery verification remains pending supervisor execution.
Next action: supervisor verification and independent acceptance review of T3, followed by human confirmation on the ordinary clone; no other task was undertaken.
