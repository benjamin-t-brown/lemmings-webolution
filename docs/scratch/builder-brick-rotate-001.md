# Builder plank rotation

[Playtest 25 Q11](../playtest/2026-11-21-playtest-25.md#defects) reported that the plank
from the Builder's sack grew and moved in the clone. The original also turned it into
place. The clone's `builderBrickShape` supplied scale and offset to `drawBuilderBrick`,
which projected the same orientation throughout. **Just Build** (`MM_BUILD`, node 4)
provides both facings. **Designed with love** (`M11_KF_1`, node 33) provides an inverted
Builder. [Prior carry trace](builder-brick-anim-001.md) · [Inverted route](inverted-builder-001.md)

## Static evidence

REA/Ghidra 12.1.4 read the registered x86 `Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
The pseudocode at `0x410302` and the instructions at `0x410445..0x41046B` yield
`p = (time - 0.12) / (0.78 - 0.12)`. Growth uses `p`. Instructions at
`0x4105D6..0x41065D` derive `travel = max(0, 2*p - 1)` for **both** rotations.
The angles hold at extraction until **0.45 s** and turn linearly through **0.78 s**.

Normal 0 is local up, the difference between two outer-shell points one row apart
at `0x41173A..0x4117A7`. Normal 4 is local inward, the outer-to-inner shell
difference at `0x41177E..0x41178B`, stored at `0x41183F..0x411854`.
`0x4109C8..0x410B1D` branches on facing at actor `+0x40` and vertical direction
at `+0x44`, with `-1` inverted. `0x4813F2` builds degree-based axis-angle
quaternions. `0x410B67..0x410B94` composes normal 0 first, then normal 4,
then cylinder heading.

| Actor | Up rotation in degrees | Inward rotation in degrees | Angle branch |
| --- | ---: | ---: | --- |
| Right, upright | 270 → 360 | −270 → −360 | `0x410A31..0x410A77` |
| Left, upright | 90 → 0 | −90 → 0 | `0x410AD7..0x410B1D` |
| Right, inverted | 90 → 180 | −270 → −180 | `0x4109DE..0x410A2A` |
| Left, inverted | 90 → 180 | 90 → 0 | `0x410A8A..0x410AD0` |

The instructions, Ghidra pseudocode and transform inference are archived
separately. Both sessions closed without snapshot arguments.
[Instruction cross-check](../../build/builder-brick-rotate-001/static/rotation-crosscheck.md) ·
[REA packets](../../build/builder-brick-rotate-001/static/rea-static.json) ·
[Quaternion constructors](../../build/builder-brick-rotate-001/static/quaternion-constructors.json)

## Production change

`builderPresentationPhase → injectedWalkersFromWorld → drawInjectedWalkers →
prepareInjectedWalker → drawBuilderBrick` uses the existing carry clock. The
renderer rotates the cuboid about world up and then inward radial, in model
units. Expressing both turns relative to the placed pose gives opposite
quarter-turns that reach identity at the target cells. Reversing either
facing or gravity reverses both turns. The position follows the short path
across the cylinder seam. The plank blends from actor light into the placed
terrain face shades as it lands. `World::stepBuilder` supplies the solid cells
on the tick that ends the carry. The existing actor composition supplies
pause and occlusion behavior. [Rotation](../../src/app/BuilderBrick.h#L60) ·
[Brick draw](../../src/app/TubeScene.cpp#L420)

The cuboid makes the turn legible. The red-pack anchor, carry translation, centred
cylinder radial, uniform scale in place of the folded 12-vertex mesh and single
OUT tile are provisional. Tile seams and terrain retessellation can still change
edge pixels. These captures establish clone appearance at the sampled views.

## Clone evidence and checks

Each contact sheet shows eight carry samples and then the placed brick, in
reading order. The headless inspector assigned Builder through `World::assign`
at selected coordinates from the playtest routes and rendered through the
installed level geometry, live terrain sync and actor passes. The last carried
frame is at clone time **799 ms**, with 99.91% scale and each turn about 0.25°
from flat. Both cells appear at **800 ms**.
The overlap column counts visible carried pixels that fall on pixels changed
by the terrain draw. The body moves up or down on the placement tick.
[Inspector source](../../build/builder-brick-rotate-001/inspect.cpp) ·
[Build](../../build/builder-brick-rotate-001/inspect-build.log) ·
[Run](../../build/builder-brick-rotate-001/inspect-run.log)

| View | Built cells (Y down) | Last-frame overlap | Framebuffers |
| --- | --- | ---: | --- |
| Just Build, right | (78,72), (79,72) | 676/681 | [Contact sheet](../../build/builder-brick-rotate-001/frames/MM_BUILD-right/contact-sheet.png) · [Samples](../../build/builder-brick-rotate-001/frames/MM_BUILD-right/samples.tsv) |
| Just Build, left | (162,72), (161,72) | 628/636 | [Contact sheet](../../build/builder-brick-rotate-001/frames/MM_BUILD-left/contact-sheet.png) · [Samples](../../build/builder-brick-rotate-001/frames/MM_BUILD-left/samples.tsv) |
| Designed with love, inverted left | (33,45), (32,45) | 434/448 | [Contact sheet](../../build/builder-brick-rotate-001/frames/M11_KF_1-inverted-left/contact-sheet.png) · [Samples](../../build/builder-brick-rotate-001/frames/M11_KF_1-inverted-left/samples.tsv) |

- The registered GCC 16.1.0 build completed the `smoke_tests` and
  `test_builder_brick` targets. The windowless legacy check passed in **0.31 s**
  and checks extraction, midway, turning and delivery in all four facing/gravity
  combinations. Focused smoke tests `actorpose`, `actorcompose` and `cellmesh`
  all passed. [Build](../../build/builder-brick-rotate-001/smoke-target-light-build.log) ·
  [Builder check](../../build/builder-brick-rotate-001/final-builder-brick.log) ·
  [Focused smoke](../../build/builder-brick-rotate-001/final-focused-smoke.log)
- All **29** promoted recordings returned exit 0, `PASS won=1 completed=1`.
  Just Build finished at step **1685** with **20/10** saved and Designed with
  love finished at step **40836** with **97/85**. The index gives each exact
  recording path, finish step and CLI log. Clone runs used task-private
  LocalAppData. [Replay results](../../build/builder-brick-rotate-001/replays.tsv)

Changed `src/app/BuilderBrick.h`, `src/app/TubeScene.cpp`,
`src/__test__/test_builder_brick.cpp` and this report. Entry sources from
`git show` are under [entry](../../build/builder-brick-rotate-001/entry/).

**Handoff.** `builder-brick-rotate-001` is **needs_review**.<br>
**Next.** Supervisor smoke is pending, followed by independent review and the human playtest.

**What to check.** In **Just Build**, assign Builder in each facing. Watch the
plank come out of the sack, grow, tip forward and settle flush into the new step.
Pause during the turn and resume. In **Designed with love**, assign Builder
after a gravity flip and watch the mirrored turn seat the plank against the ceiling.
