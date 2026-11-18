# hud-icons-001

Playtest 13 C6 on node 22, **One Way Ticket** (`MM_ALL_8_001`), showed bare
counter and clock numbers in the clone. The original screenshot shows a
lemming, an exit balloon, and a stopwatch in those positions.
[Playtest 13](../playtest/2026-11-17-playtest-13.md)
[Original screenshot](../playtest/reference/playtest-13/one_way_ticket_orig.jpg)
[Prior clone screenshot](../playtest/reference/playtest-13/one_way_ticket_web.jpg)

The ordinary level leg in `runApplicationLeg` loads `^OUT`, `^SAVED` and
`^TIME` once through `loadTexture`, including their installed ALF planes.
Frame composition uses `compositeIconRegion` before the existing
`drawRuntimeHud` call. The counter and clock values still come from the same
`World` and are formatted by the same HUD text path. Skill-panel composition
and input keep their existing calls. A failed icon load is logged and leaves
that icon empty so the level can still open.
[Asset setup](../../src/app/main.cpp#L1114)
[Frame composition](../../src/app/main.cpp#L1623)

Ghidra ties the three assets to the quads initialized at `0x44CA7B` and
submitted by `0x44CD06`. The recovered pixel rectangles are (557, 15) at
26×32, (560, 53) at 20×32, and (579, 395) at 26×32 in the 640×480 HUD.
The One Way Ticket clone frame shows the icons beside both counters and
above the clock. The clone uses its standard straight-alpha compositor
provisionally. Original sampling, hardware blending and the alternate
stopwatch branch at `0x44CDE3` remain unresolved. The `+shadow_new` preview
remains deferred pending a complete placement trace.
[Reference identity, instructions and cross-check](../../build/hud-icons-001/evidence.md)
[Retry decompilation, xrefs and instructions](../../build/hud-icons-001/retry/static-evidence.json)
[Clone frame](../../build/hud-icons-001/one-way-ticket-window.png)

## Checks

The previous smoke run failed both `frontendentry` image comparisons.
Applying the three installed icon blits to the old expected frame accounts
for all 1,425 changed pixels in each output. The expected window composition
in `HatchEntryFixture.h` now includes the three icons before the existing text
draw. The test still compares entire frames and checks the direct headless
control and tick-0 positive control. The targeted registered UCRT64 build
and `frontendentry` run passed on the retry.
[Prior smoke failure](../../build/agent-runs/20261118T163427-26368-d36d20e1/verification.log)
[Retry build](../../build/hud-icons-001/retry/build.log)
[Focused test](../../build/hud-icons-001/retry/frontendentry.log)

The retained One Way Ticket window run and three texture decodes completed
with exit 0. The retry's PNG comparison matched all 1,004 opaque icon texels
to the installed art at the recovered coordinates. The window run and
focused test used private `LOCALAPPDATA` paths.
[Texture dimensions](../../build/hud-icons-001/texture-sizes.tsv)
[Window invocation](../../build/hud-icons-001/one-way-ticket-window.log)
[Pixel comparison](../../build/hud-icons-001/retry/pixel-comparison.log)

Each of the 20 promoted recordings ran via
`webolution --game-dir "Lemmings Revolution" --replay <file>` and returned
exit 0, `PASS`, `won=1`, and `completed=1`.
[Exact recording paths, results and invocation logs](../../build/hud-icons-001/retry/replays.tsv)

| Recording level | Completion step | Saved / target |
| --- | ---: | ---: |
| MM_BASH_002 | 1844 | 10/10 |
| MM_BASH_MINE_WLR1 | 2272 | 20/10 |
| MM_BLOCK_001_A | 1619 | 24/15 |
| MM_BLOCK_BOMB_A | 1546 | 12/10 |
| MM_BOMB | 954 | 17/10 |
| MM_BUILD | 1685 | 20/10 |
| MM_CLIMB_002 | 1252 | 10/1 |
| MM_DIG_000 | 1014 | 19/10 |
| MM_FLOAT | 978 | 10/1 |
| MM_MINE | 1137 | 20/10 |
| MM_W_012 | 1742 | 16/10 |
| MM_E1_3_SKILLS | 2095 | 10/5 |
| M06_KF_1 | 1544 | 91/50 |
| MM_E1_4SKILLS | 2506 | 20/10 |
| MM_E2_4SKILLS | 2142 | 14/10 |
| MM_BASH_DIG_001_A | 3241 | 20/10 |
| MM_BUILD_BLOCK_A | 2482 | 15/10 |
| E19_KF_2 | 6034 | 24/20 |
| E20_MG_1 | 12810 | 7/5 |
| MM_ALL_8_001 | 13812 | 46/40 |

The task’s production change is in `src/app/main.cpp`. This retry changed
`src/__test__/HatchEntryFixture.h` and this report. Detailed evidence and entry
source records are under `build/hud-icons-001/`.

Handoff to the supervisor with status `needs_review`. The smoke gate is pending.\
Independent review checks the draw path and smoke result. The next human playtest confirms C6.

## What to check

Open **One Way Ticket** and let lemmings release. The lemming and balloon
icons should sit to the left of their counters and the stopwatch above the
clock. Watch the out count and clock update, then save a lemming to check
that the saved count updates beside its icon. Select a skill to confirm the
existing panel responds as before.
