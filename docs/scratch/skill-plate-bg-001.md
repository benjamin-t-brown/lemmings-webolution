# Playtest 13 C8 — skill-panel plates

**Needs review.** On node 22, One Way Ticket (`MM_ALL_8_001`), the current gameplay frame shows eight skill plates, the fast-forward plate and the nuke plate. All ten have dark rounded interiors. The selected Climber has a white rim. This task adds the report only. [Current clone frame](../../build/skill-plate-bg-001/gameplay.png) · [User's original screenshot](../playtest/reference/playtest-13/one_way_ticket_orig.jpg)

The [Playtest 13 clone screenshot](../playtest/reference/playtest-13/one_way_ticket_web.jpg) shows an unselected dark plate around the Bomber where brighter scenery crosses it. Lower entries appear bare against the nearly black water. Black artwork with partial opacity barely changes a black destination. The C5 backdrop correction restored the green water behind those rows. The panel and its runtime adapter have the same source as the reported build `deb8c25`. A second pass would darken interiors twice. [Playtest entry](../playtest/2026-11-17-playtest-13.md) · [C5 backdrop evidence](oneway-backdrop-001.md) · [Playtest-build panel source](../../build/skill-plate-bg-001/playtest13-SkillPanel.cpp)

## Draw path and evidence

- **Installed art.** `TEXTURES\^TAB.BMP` supplies black RGB and `^TAB.ALF` the rounded 72×32 mask, with alpha up to 128. `^TAB_SELECTED.BMP` and its ALF supply the white rim. The current source-built texture tool decoded both. [Base decode](../../build/skill-plate-bg-001/texture-TAB.log) · [Selection decode](../../build/skill-plate-bg-001/texture-TAB_SELECTED.log) · [Retained asset reads](../lessons/hud-and-panel.md)
- **Static original.** Ghidra opened the registered x86 `Lemmings Revolution/Lemmings Revolution.exe`, SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`. The xref at `0x441A71` calls `0x45B639`. Instructions confirm the decompiler's ten unconditional quad submits at `0x45B66A`, followed by the enabled selected quad at `0x45B6B5`, nonzero counts at `0x45B6FA` and icons at `0x45B784`. The retained initializer places each base at x 18, y `12 + 40 × index`, width 72, height 32. Static analysis closed without a snapshot. The original did not run. [Xref, instructions and pseudocode](../../build/skill-plate-bg-001/rea.json) · [Plate initialization](phase-c1v-plate-draw-read.md)
- **Production.** In `runApplicationLeg`, `drawRuntimeSkillPanel` passes counts, selection, fast-forward state and icon animation to `RetailSkillPanel::draw`. It draws the ten bases, selected and held-speed outlines, counts, then icons. The same 72×32 bounds feed hit testing. The existing horizontal sampler remains provisional. [Frame composition](../../src/app/main.cpp) · [Runtime input](../../src/app/RuntimeSkillPanel.cpp) · [Panel drawing](../../src/app/SkillPanel.cpp)
- **Clone observation.** At the settled opening camera, the scene-only pixel at (53,28) was RGB (46,60,28). In the gameplay frame it became (22,29,13) inside the selected plate. Its rim at (53,14) was (228,229,227), and its corner at (18,12) preserved (34,44,20). All ten rows were inspected visually. Windows PowerShell execution policy blocked the optional all-row pixel script before it ran. Hover motion and exact original edge filtering were not measured in this pass. [Gameplay run](../../build/skill-plate-bg-001/gameplay.log) · [Scene-only run](../../build/skill-plate-bg-001/scene.log) · [Earlier icon-animation evidence](panel-icon-anim-001.md)

## Checks

The registered UCRT64 build completed. The focused `skillpanel` and `runtimeskillpanel` tests passed, including selection, stock counts and hit boundaries. Clone runs used private `build/skill-plate-bg-001/private-save` as LocalAppData. [Build](../../build/skill-plate-bg-001/build.log) · [Focused tests](../../build/skill-plate-bg-001/panel-tests.log)

All 20 promoted recordings returned exit 0, `PASS`, `won=1` and `completed=1`. Each result below gives saved/required. [Exact recording paths, commands, final steps and individual logs](../../build/skill-plate-bg-001/replays.tsv)

| Level | Saved/required | Level | Saved/required |
| --- | ---: | --- | ---: |
| MM_BASH_002 | 10/10 | MM_W_012 | 16/10 |
| MM_BASH_MINE_WLR1 | 20/10 | MM_E1_3_SKILLS | 10/5 |
| MM_BLOCK_001_A | 24/15 | M06_KF_1 | 91/50 |
| MM_BLOCK_BOMB_A | 12/10 | MM_E1_4SKILLS | 20/10 |
| MM_BOMB | 17/10 | MM_E2_4SKILLS | 14/10 |
| MM_BUILD | 20/10 | MM_BASH_DIG_001_A | 20/10 |
| MM_CLIMB_002 | 10/1 | MM_BUILD_BLOCK_A | 15/10 |
| MM_DIG_000 | 19/10 | E19_KF_2 | 24/20 |
| MM_FLOAT | 10/1 | E20_MG_1 | 7/5 |
| MM_MINE | 20/10 | MM_ALL_8_001 | 46/40 |

Handoff to the supervisor as `needs_review`. The smoke gate is pending.\
Independent review should compare the current gameplay frame with the user's reference before the next human playtest.

**What to check.** Open One Way Ticket, hover Builder, then select it. Its icon should animate and the white outline should move to Builder. Hold and release fast-forward. Its outline should follow the hold, while all ten dark plates remain visible, including the nuke plate.
